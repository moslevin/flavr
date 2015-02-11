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
    \file   write_callout.h

    \brief  Extended emulator functionality allowing for functions to be triggered
            based on RAM-write operations.
*/

#ifndef __WRITE_CALLOUT_H__
#define __WRITE_CALLOUT_H__

#include <stdint.h>

//---------------------------------------------------------------------------
//! Function pointer type for memory-write callout handlers
typedef void (*WriteCalloutFunc)(uint16_t u16Addr_, uint8_t u8Data_);

//---------------------------------------------------------------------------
/*!
 * \brief WriteCallout_Add
 *
 * Registers a specific function to be called whenever a specific address in
 * memory is modified.  Multiple functions can be registered at the same
 * location in memory.
 *
 * \param pfCallout_ - Pointer to the callout function
 * \param u16Addr_   - Address in RAM that triggers the callout when written
 */
void WriteCallout_Add( WriteCalloutFunc pfCallout_, uint16_t u16Addr_ );

//---------------------------------------------------------------------------
/*!
 * \brief WriteCallout_Run
 *
 * Function called by the AVR CPU core whenever a word in memory is written.
 * This searches the list of write callouts and executes any callouts registered
 * at the specific address.
 *
 * \param u16Addr_   - Address in RAM currently being modified
 * \param u8Data_    - Data that will be written to the address
 */
void WriteCallout_Run( uint16_t u16Addr_, uint8_t u8Data_ );


#endif

