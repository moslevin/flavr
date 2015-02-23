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
    \file   ka_profile.c

    \brief  Mark3 RTOS Kernel-Aware Profilng
*/

#include "kernel_aware.h"
#include "debug_sym.h"
#include "write_callout.h"
#include "interrupt_callout.h"
#include "ka_profile.h"
#include "tlv_file.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

//---------------------------------------------------------------------------
//!! This is all singleton data... could be better hosted in a struct...
//!! Especially if Mark3 ever supports multiple concurrent Profilers
static uint64_t u64ProfileEpochStart = 0;
static uint64_t u64ProfileTotal = 0;
static uint64_t u64ProfileCount = 0;
static char szNameBuffer[32] = {};
static TLV_t *pstTLV = NULL;

//---------------------------------------------------------------------------
typedef struct
{
    uint64_t u64Timestamp;          //!< Timestamp when the profiling print was made
    uint64_t u64ProfileCount;       //!< Count of profiling events
    uint64_t u64ProfileTotalCycles; //!< Total cycles (sum from all profiling events
    char     szName[32];            //!< Profiling name
} Mark3Profile_TLV_t;

//---------------------------------------------------------------------------
static void KA_PrintProfileResults(void)
{
    Mark3Profile_TLV_t stTLV;

    stTLV.u64ProfileCount       = u64ProfileCount;
    stTLV.u64ProfileTotalCycles = u64ProfileTotal;
    stTLV.u64Timestamp          = stCPU.u64CycleCount;

    strcpy( stTLV.szName, szNameBuffer );
    memcpy( pstTLV->au8Data, &stTLV, sizeof(Mark3Profile_TLV_t) );

    TLV_Write( pstTLV );
}

//---------------------------------------------------------------------------
void KA_Command_Profile_Begin(void)
{
    u64ProfileCount = 0;
    u64ProfileTotal = 0;
    u64ProfileEpochStart = 0;

    Debug_Symbol_t *pstSymbol = Symbol_Find_Obj_By_Name( "g_stKAData" );
    if (pstSymbol)
    {
        uint16_t u16NamePtr = *((uint16_t*)&stCPU.pstRAM->au8RAM[ pstSymbol->u32StartAddr ]);
        const char *szName = (const char*)&stCPU.pstRAM->au8RAM[ u16NamePtr ];
        if (szName)
        {
            strcpy( szNameBuffer, szName );
        }
        else
        {
            strcpy( szNameBuffer, "(NONE)" );
        }
    }
}

//---------------------------------------------------------------------------
void KA_Command_Profile_Start(void)
{
    // Profile stop or reset
    u64ProfileTotal += (stCPU.u64CycleCount - u64ProfileEpochStart);
    u64ProfileEpochStart = 0;
    u64ProfileCount++;
}

//---------------------------------------------------------------------------
void KA_Command_Profile_Stop(void)
{
    u64ProfileEpochStart = stCPU.u64CycleCount;
}

//---------------------------------------------------------------------------
void KA_Command_Profile_Report(void)
{
    KA_PrintProfileResults();
    u64ProfileTotal = 0;
    u64ProfileEpochStart = 0;
}

//---------------------------------------------------------------------------
void KA_Profile_Init(void)
{
    pstTLV = TLV_Alloc(sizeof(Mark3Profile_TLV_t));
    pstTLV->eTag = TAG_KERNEL_AWARE_PROFILE;
    pstTLV->u16Len = sizeof(Mark3Profile_TLV_t);
}
