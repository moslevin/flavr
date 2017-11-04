/****************************************************************************
 *     (     (                      (     |
 *    )\ )  )\ )    (              )\ )   |
 *   (()/( (()/(    )\     (   (  (()/(   | -- [ Funkenstein ] -------------
 *    /(_)) /(_))((((_)()\  )\  /(_))     | -- [ Litle ] -------------------
 *   (_))_|(_))   )\ _ )\ ((_)((_)(_))    | -- [ AVR ] ---------------------
 *   | |_  | |    (_)_\(_)\ \ / / | _ \   | -- [ Virtual ] -----------------
 *   | __| | |__   / _ \   \ V /  |   /   | -- [ Runtime ] -----------------
 *   |_|   |____| /_/ \_\   \_/   |_|_\   |
 *                                        | "Yeah, it does Arduino..."
 * ---------------------------------------+----------------------------------
 * (c) Copyright 2014-17, Funkenstein Software Consulting, All rights reserved
 *     See license.txt for details
 ****************************************************************************/
/*!
    \file   code_profile.c

    \brief  Code profiling (exeuction and coverage) functionality
*/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debug_sym.h"
#include "code_profile.h"
#include "avr_disasm.h"
#include "tlv_file.h"

//---------------------------------------------------------------------------
typedef struct
{
    Debug_Symbol_t *pstSym;         //!< Pointer to the debug symbol being profiled at this address
    uint64_t        u64TotalHit;    //!< Total count of hits at this address
    uint64_t        u64EpochHit;    //!< Count of hits at this address in the current epoch
} Profile_t;

//---------------------------------------------------------------------------
typedef struct
{
    uint64_t u64CyclesTotal;
    uint64_t u64CPUCycles;
    char szSymName[256];
} FunctionProfileTLV_t;

//---------------------------------------------------------------------------
typedef struct
{
    uint32_t u32FunctionSize;
    uint32_t u32AddressesHit;
    char szSymName[256];
} FunctionCoverageTLV_t;

//---------------------------------------------------------------------------
typedef struct
{
    uint32_t u32CodeAddress;
    uint64_t u64Hits;
    char     szDisasmLine[256];   //!< Disassembly for the address in question
} AddressCoverageTLV_t;

//---------------------------------------------------------------------------
static Profile_t *pstProfile = 0;
static uint32_t  u32ROMSize = 0;

//---------------------------------------------------------------------------
static TLV_t *pstFunctionCoverageTLV = NULL;
static TLV_t *pstFunctionProfileTLV = NULL;
static TLV_t *pstAddressCoverageTLV = NULL;

//---------------------------------------------------------------------------
static void Profile_TLVInit(void)
{
    pstFunctionProfileTLV = TLV_Alloc( sizeof(FunctionProfileTLV_t));
    pstFunctionProfileTLV->eTag = TAG_CODE_PROFILE_FUNCTION_GLOBAL;

    pstFunctionCoverageTLV = TLV_Alloc( sizeof(FunctionCoverageTLV_t));
    pstFunctionCoverageTLV->eTag = TAG_CODE_COVERAGE_FUNCTION_GLOBAL;

    pstAddressCoverageTLV = TLV_Alloc( sizeof(AddressCoverageTLV_t));
    pstAddressCoverageTLV->eTag = TAG_CODE_COVERAGE_ADDRESS;
}

//---------------------------------------------------------------------------
static void Profile_FunctionCoverage( const char *szFunc_, uint32_t u32FuncSize_, uint32_t u32HitSize_ )
{
    FunctionCoverageTLV_t *pstData = (FunctionCoverageTLV_t*)(&pstFunctionCoverageTLV->au8Data[0]);

    strcpy(pstData->szSymName, szFunc_);
    pstData->u32FunctionSize = u32FuncSize_;
    pstData->u32AddressesHit = u32HitSize_;
    pstFunctionCoverageTLV->u16Len = strlen(szFunc_) + 8; // Size of the static + variable data

    TLV_Write( pstFunctionCoverageTLV );
}

//---------------------------------------------------------------------------
static void Profile_Function( const char *szFunc_, uint64_t u64Cycles_, uint64_t u64CPUCycles_ )
{
    FunctionProfileTLV_t *pstData = (FunctionProfileTLV_t*)(&pstFunctionProfileTLV->au8Data[0]);

    strcpy(pstData->szSymName, szFunc_);
    pstData->u64CyclesTotal = u64Cycles_;
    pstData->u64CPUCycles = u64CPUCycles_;

    pstFunctionProfileTLV->u16Len = strlen(szFunc_) + 16; // Size of the static + variable data

    TLV_Write( pstFunctionProfileTLV );
}

//---------------------------------------------------------------------------
static void Profile_AddressCoverage( const char *szDisasm_, uint32_t u32Addr_, uint64_t u64Hits_ )
{
    AddressCoverageTLV_t *pstData = (AddressCoverageTLV_t*)(&pstAddressCoverageTLV->au8Data[0]);

    strcpy(pstData->szDisasmLine, szDisasm_);

    pstData->u32CodeAddress = u32Addr_;
    pstData->u64Hits = u64Hits_;

    pstAddressCoverageTLV->u16Len = strlen(szDisasm_) + 12;

    TLV_Write( pstAddressCoverageTLV );
}

//---------------------------------------------------------------------------
void Profile_Hit( uint32_t u32Addr_ )
{
    pstProfile[ u32Addr_ ].u64EpochHit++;
    pstProfile[ u32Addr_ ].u64TotalHit++;

    Debug_Symbol_t *pstSym = pstProfile[ u32Addr_ ].pstSym;
    if (pstSym)
    {
        pstSym->u64EpochRefs++;
        pstSym->u64TotalRefs++;        
    }
}

//---------------------------------------------------------------------------
void Profile_ResetEpoch(void)
{
    // Reset the epoch counters for all addreses
    int i;
    for (i = 0; i < u32ROMSize; i++)
    {
        pstProfile[i].u64EpochHit = 0;
    }

    // Reset the per-symbol epoch counters
    Debug_Symbol_t *pstSym;
    int iSymCount = Symbol_Get_Func_Count();
    for (i = 0; i < iSymCount; i++)
    {
        pstSym = Symbol_Func_At_Index(i);
        pstSym->u64EpochRefs = 0;
    }
}

//---------------------------------------------------------------------------
void Profile_PrintCoverageDissassembly(void)
{
    Debug_Symbol_t *pstSym;
    int iSymCount = Symbol_Get_Func_Count();
    int i;
    int j;

    printf( "=====================================================================================\n");
    printf( "Detailed Code Coverage\n");
    printf( "=====================================================================================\n");
    // Go through all of our symbols and show which instructions have actually
    // been hit.
    for (i = 0; i < iSymCount; i++)
    {
        pstSym = Symbol_Func_At_Index(i);

        if (!pstSym)
        {
            break;
        }

        printf("%s:\n", pstSym->szName);
        j = pstSym->u32StartAddr;
        while (j <= (int)pstSym->u32EndAddr)
        {
            uint16_t OP = stCPU.pu16ROM[j];
            stCPU.u32PC = (uint16_t)j;

            if (pstProfile[j].u64TotalHit)
            {
                printf( "[X]" );
            }
            else
            {
                printf( "[ ]" );
            }
            printf(" 0x%04X: [0x%04X] ", stCPU.u32PC, OP);

            AVR_Decode(OP);

            char szBuf[256];
            AVR_Disasm_Function(OP)(szBuf);
            printf( "%s", szBuf );

            Profile_AddressCoverage( szBuf, stCPU.u32PC, pstProfile[j].u64TotalHit );

            j += AVR_Opcode_Size(OP);
        }
        printf("\n");
    }
}

//---------------------------------------------------------------------------
void Profile_Print(void)
{
    uint64_t u64TotalCycles = 0;

    Debug_Symbol_t *pstSym;
    int iSymCount = Symbol_Get_Func_Count();
    int i;
    for (i = 0; i < iSymCount; i++)
    {
        pstSym = Symbol_Func_At_Index(i);
        u64TotalCycles += pstSym->u64TotalRefs;
    }
    printf("\n\nTotal cycles spent in known functions: %llu\n\n", u64TotalCycles);

    printf( "=====================================================================================\n");
    printf( "%60s: CPU utilization(%%)\n", "Function");
    printf( "=====================================================================================\n");
    for (i = 0; i < iSymCount; i++)
    {
        pstSym = Symbol_Func_At_Index(i);
        printf( "%60s: %0.3f\n",
                pstSym->szName,
                100.0 * (double)(pstSym->u64TotalRefs) / (double)(u64TotalCycles) );
        Profile_Function( pstSym->szName, pstSym->u64TotalRefs, u64TotalCycles );
    }

    printf( "=====================================================================================\n");
    printf( "Code coverage summary:\n");
    printf( "=====================================================================================\n");
    int iGlobalHits = 0;
    int iGlobalMisses = 0;
    for (i = 0; i < iSymCount; i++)
    {
        pstSym = Symbol_Func_At_Index(i);
        int j;
        int iHits = 0;
        int iMisses = 0;

        for (j = pstSym->u32StartAddr; j < pstSym->u32EndAddr; j++)
        {
            if (pstProfile[j].u64TotalHit)
            {
                iHits++;
                iGlobalHits++;
            }
            else
            {
                iMisses++;
                iGlobalMisses++;
            }

            // If this is a 2-opcode instruction, skip the next word, as to not skew the results
            uint16_t OP = stCPU.pu16ROM[j];
            if (2 == AVR_Opcode_Size(OP))
            {
                j++;
            }
        }
        printf("%60s: %0.3f\n", pstSym->szName, 100.0 * (double)iHits/(double)(iHits + iMisses));
        Profile_FunctionCoverage(pstSym->szName, iHits + iMisses, iHits);
    }
    printf( "\n[Global Code Coverage] : %0.3f\n",
            100.0 * (double)iGlobalHits/(double)(iGlobalHits + iGlobalMisses));

}
//---------------------------------------------------------------------------
void Profile_Init( uint32_t u32ROMSize_ )
{
    // Allocate a lookup table, one entry per address in ROM to allow us to
    // gather code-coverage and code-profiling information.
    uint32_t u32BufSize = sizeof(Profile_t) * u32ROMSize_ ;
    u32ROMSize = u32ROMSize_;
    pstProfile = (Profile_t*)malloc( u32BufSize );
    memset( pstProfile, 0, u32BufSize );

    // Go through the list of symbols, and associate each function with its
    // address range in the lookup table.
    int iFuncs = Symbol_Get_Func_Count();
    int i;
    for (i = 0; i < iFuncs; i++)
    {
        Debug_Symbol_t *pstSym = Symbol_Func_At_Index( i );
        int j;
        if (pstSym)
        {
            for (j = pstSym->u32StartAddr; j < pstSym->u32EndAddr; j++)
            {
                pstProfile[j].pstSym = pstSym;
            }
        }
    }

    Profile_TLVInit();

    atexit( Profile_Print );
    atexit( Profile_PrintCoverageDissassembly );
}
