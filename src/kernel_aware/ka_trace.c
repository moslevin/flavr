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
    \file   ka_trace.c

    \brief  Mark3 RTOS Kernel-Aware Trace functionality
*/

#include "kernel_aware.h"
#include "debug_sym.h"

#include "ka_trace.h"
#include "tlv_file.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

//---------------------------------------------------------------------------
typedef struct
{
    uint16_t u16File;
    uint16_t u16Line;
    uint16_t u16Code;
    uint16_t u16Arg1;
    uint16_t u16Arg2;
} KernelAwareTrace_t;

//---------------------------------------------------------------------------
static TLV_t *pstTLV = NULL;

//---------------------------------------------------------------------------
void KA_EmitTrace( KernelAwareCommand_t eCmd_ )
{
    Debug_Symbol_t *pstSymbol = Symbol_Find_Obj_By_Name( "g_stKAData" );
    if (!pstSymbol)
    {
        return;
    }

    KernelAwareTrace_t *pstTrace = (KernelAwareTrace_t*)&stCPU.pstRAM->au8RAM[ pstSymbol->u32StartAddr ];
    switch (eCmd_)
    {
    case KA_COMMAND_TRACE_0:
        pstTLV->eTag = KA_COMMAND_TRACE_0;
        pstTLV->u16Len = 6;        
        break;
    case KA_COMMAND_TRACE_1:
        pstTLV->eTag = KA_COMMAND_TRACE_1;
        pstTLV->u16Len = 8;
        break;
    case KA_COMMAND_TRACE_2:
        pstTLV->eTag = KA_COMMAND_TRACE_2;
        pstTLV->u16Len = 10;
        break;
    default:
        return;
    }
    fprintf(stderr, "Trace: %04X, %04X, %04X, %04X, %04x\n", pstTrace->u16File, pstTrace->u16Line,
                    pstTrace->u16Code, pstTrace->u16Arg1, pstTrace->u16Arg2 );

    memcpy( pstTLV->au8Data, pstTrace, pstTLV->u16Len );
    TLV_Write( pstTLV );
}

//---------------------------------------------------------------------------
void KA_Print( void )
{
    Debug_Symbol_t *pstSymbol = Symbol_Find_Obj_By_Name( "g_stKAData" );
    if (!pstSymbol)
    {
        return;
    }

    uint16_t u16NamePtr = *((uint16_t*)&stCPU.pstRAM->au8RAM[ pstSymbol->u32StartAddr ]);
    const char *szString = (const char*)&stCPU.pstRAM->au8RAM[ u16NamePtr ];

    strcpy( pstTLV->au8Data, szString );
    fprintf( stderr, "%s", szString );
}

//---------------------------------------------------------------------------
void KA_Trace_Init(void)
{
    pstTLV = TLV_Alloc(64);
}
