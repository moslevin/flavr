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
    \file   ka_interrupt.c

    \brief  Mark3 RTOS Kernel-Aware Interrupt Logging
*/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "avr_cpu.h"
#include "kernel_aware.h"
#include "ka_interrupt.h"
#include "write_callout.h"
#include "interrupt_callout.h"
#include "tlv_file.h"

//---------------------------------------------------------------------------
static TLV_t *pstTLV = NULL;

//---------------------------------------------------------------------------
typedef struct
{
    uint64_t    u64TimeStamp;
    uint8_t     u8Vector;
    bool        bEntry;

} Mark3Interrupt_TLV_t;

//---------------------------------------------------------------------------
static void KA_Interrupt( bool bEntry_, uint8_t u8Vector_ )
{
    Mark3Interrupt_TLV_t stData;
    stData.u64TimeStamp = stCPU.u64CycleCount;
    stData.u8Vector = u8Vector_;
    stData.bEntry = bEntry_;

    memcpy( &(pstTLV->au8Data[0]), &stData, sizeof(stData) );
    TLV_Write(pstTLV);
}

//---------------------------------------------------------------------------
void KA_Interrupt_Init(void)
{
    pstTLV = TLV_Alloc( sizeof(Mark3Interrupt_TLV_t) );
    if (!pstTLV)
    {
        return;
    }

    pstTLV->eTag = TAG_KERNEL_AWARE_INTERRUPT;
    pstTLV->u16Len = sizeof(Mark3Interrupt_TLV_t);

    InterruptCallout_Add( KA_Interrupt );
}
