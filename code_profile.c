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
 * (c) Copyright 2014-15, Funkenstein Software Consulting, All rights reserved
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

//---------------------------------------------------------------------------
typedef struct
{
    Debug_Symbol_t *pstSym;         //!< Pointer to the debug symbol being profiled at this address
    uint64_t        u64TotalHit;    //!< Total count of hits at this address
    uint64_t        u64EpochHit;    //!< Count of hits at this address in the current epoch
} Profile_t;

//---------------------------------------------------------------------------
static Profile_t *pstProfile = 0;
static uint32_t  u32ROMSize = 0;

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
    }

    printf( "=====================================================================================\n");
    printf( "Detailed code coverage information:\n");
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
        }
        printf("%60s: %0.3f\n", pstSym->szName, 100.0 * (double)iHits/(double)(iHits + iMisses));
    }
    printf( "\n[Global Code Coverage] : %0.3f\n",
            100.0 * (double)iGlobalHits/(double)(iGlobalHits + iGlobalMisses));
}
