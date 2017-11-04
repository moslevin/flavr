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
  \file  watchpoint.h

  \brief Implements data watchpoints for debugging running programs based on
         reads/writes to a given memory address.
*/

#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include <stdint.h>
#include <stdbool.h>

#include "avr_cpu.h"

//---------------------------------------------------------------------------
typedef struct _WatchPoint
{
    struct _WatchPoint *next;       //!< Pointer to next watchpoint
    struct _WatchPoint *prev;       //!< Pointer to previous watchpoint

    uint16_t    u16Addr;            //!< Address (in RAM) to watch on.
} WatchPoint_t;

//---------------------------------------------------------------------------
/*!
 * \brief WatchPoint_Insert
 *
 * Insert a data watchpoint for a given address.  Has no effect if a watchpoint
 * already exists at the specified address.
 *
 * \param u16Addr_ Address of the watchpoint.
 */
void WatchPoint_Insert( uint16_t u16Addr_ );

//---------------------------------------------------------------------------
/*!
 * \brief WatchPoint_Delete
 *
 * Remove a data watchpoint installed at a specific address.  Has no effect
 * if there isn't a watchpoint at the given address.
 *
 * \param u16Addr_ Address to remove data watchpoints from (if any)
 */
void WatchPoint_Delete( uint16_t u16Addr_ );

//---------------------------------------------------------------------------
/*!
 * \brief WatchPoint_EnabledAtAddress
 *
 * Check to see whether or not a watchpoint is installed at a given address
 *
 * \param u16Addr_ Address to check
 * \return true if watchpoint is installed at the specified adress
 */
bool WatchPoint_EnabledAtAddress( uint16_t u16Addr_ );

#endif

