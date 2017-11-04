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
    \file   interrupt_callout.h

    \brief  Module providing functionality allowing emulator extensions to
            be triggered on interrupts.
*/

#ifndef __INTERRUPT_CALLOUT_H__
#define __INTERRUPT_CALLOUT_H__

#include <stdint.h>
#include <stdbool.h>

//---------------------------------------------------------------------------
//! Function type used for interrupt callouts
typedef void (*InterruptCalloutFunc)( bool bEntry_, uint8_t u8Vector_ );

//---------------------------------------------------------------------------
/*!
 * \brief InterruptCallout_Add
 *
 * Add a particular callout function to be executed whenever an interrupt
 * is called (or returned-from).
 *
 * \param pfCallout_ Pointer to an interrupt callout function.
 */
void InterruptCallout_Add( InterruptCalloutFunc pfCallout_ );

//---------------------------------------------------------------------------
/*!
 * \brief InterruptCallout_Run
 *
 * Run all interrupt callouts currently installed.
 *
 * \param bEntry_   true - interrupt entry, false - interrupt exit
 * \param u8Vector_ Interrupt vector # (undefined for interrupt-exit)
 */
void InterruptCallout_Run( bool bEntry_, uint8_t u8Vector_ );


#endif

