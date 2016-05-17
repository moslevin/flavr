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
    \file   interrupt_callout.c

    \brief  Module providing functionality allowing emulator extensions to
            be triggered on interrupts.
*/

#include "interrupt_callout.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//---------------------------------------------------------------------------
typedef struct Interrupt_Callout_
{
    struct Interrupt_Callout_ *pstNext; //!< Next interrupt callout
    InterruptCalloutFunc  pfCallout;    //!< Callout function
} Interrupt_Callout_t;

//---------------------------------------------------------------------------
static Interrupt_Callout_t *pstCallouts = 0;

//---------------------------------------------------------------------------
void InterruptCallout_Add( InterruptCalloutFunc pfCallout_ )
{
    Interrupt_Callout_t *pstNewCallout = (Interrupt_Callout_t*)(malloc(sizeof(*pstNewCallout)));

    pstNewCallout->pstNext = pstCallouts;
    pstNewCallout->pfCallout = pfCallout_;

    pstCallouts = pstNewCallout;
}

//---------------------------------------------------------------------------
void InterruptCallout_Run( bool bEntry_, uint8_t u8Vector_ )
{
    Interrupt_Callout_t *pstCallout = pstCallouts;
    while (pstCallout)
    {
        pstCallout->pfCallout( bEntry_, u8Vector_ );
        pstCallout = pstCallout->pstNext;
    }
}
