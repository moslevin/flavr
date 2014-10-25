/****************************************************************************
 *     (     (                      (     |
 *    )\ )  )\ )    (              )\ )   |
 *   (()/( (()/(    )\     (   (  (()/(   | -- [ Funkenstein ] -------------
 *    /(_)) /(_))((((_)()\  )\  /(_))  | -- [ Litle ] -------------------
 *   (_))_|(_))   )\ _ )\ ((_)((_)(_))    | -- [ AVR ] ---------------------
 *   | |_  | |    (_)_\(_)\ \ / / | _ \   | -- [ Virtual ] -----------------
 *   | __| | |__   / _ \   \ V /  |   /   | -- [ Runtime ] -----------------
 *   |_|   |____| /_/ \_\   \_/   |_|_\   |
 *                                        | "Yeah, it does Arduino..."
 * ---------------------------------------+----------------------------------
 * (c) Copyright 2014, Funkenstein Software Consulting, All rights reserved
 *     See license.txt for details
 ****************************************************************************/
/*!
  \file  breakpoint.h

  \brief Implements instruction breakpoints for debugging based on code path
*/

#ifndef __BREAKPOINT_H__
#define __BREAKPOINT_H__

#include <stdint.h>
#include <stdbool.h>

#include "avr_cpu.h"

//---------------------------------------------------------------------------
typedef struct _BreakPoint
{
    struct _BreakPoint *next;
    struct _BreakPoint *prev;

    uint16_t    u16Addr;
} BreakPoint_t;

//---------------------------------------------------------------------------
/*!
 * \brief BreakPoint_Insert
 *
 * Insert a CPU breakpoint at a given address.  Has no effect if a breakpoint
 * is already present at the given address.
 *
 * \param pstCPU_ Pointer to the CPU object on which to create a breakpoint.
 * \param u16Addr_ Address of the breakpoint.
 */
void BreakPoint_Insert( uint16_t u16Addr_ );

//---------------------------------------------------------------------------
/*!
 * \brief BreakPoint_Delete
 *
 * Delete a breakpoint at a given address (if it exists).  Has no effect if
 * there isn't a breakpoint installed at the location
 *
 * \param pstCPU_ Pointer to the CPU object on which to delete the breakpoint.
 * \param u16Addr_ Address of the breakpoint to delete.
 */
void BreakPoint_Delete( uint16_t u16Addr_ );

//---------------------------------------------------------------------------
/*!
 * \brief BreakPoint_EnabledAtAddress
 *
 * Check to see whether or not a CPU execution breakpoint has been installed
 * at the given address.
 *
 * \param pstCPU_ Pointer to the CPU object on which to check for breakpoints.
 * \param u16Addr_  Address (in flash) to check for breakpoint on.
 * \return true if a breakpoint has been set on the given address.
 */
bool BreakPoint_EnabledAtAddress( uint16_t u16Addr_ );

#endif

