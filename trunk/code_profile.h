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
    \file   code_profile.h

    \brief  Code profiling (exeuction and coverage) functionality
*/

#ifndef __CODE_PROFILE_H__
#define __CODE_PROFILE_H__

#include <stdint.h>

//---------------------------------------------------------------------------
/*!
 * \brief Profile_Init
 *
 * Iniitialze the code profiling module
 *
 * \param u32ROMSize_ - Size of the CPU's ROM/FLASH
 */
void Profile_Init( uint32_t u32ROMSize_ );

//---------------------------------------------------------------------------
/*!
 * \brief Profile_Hit
 *
 * Add to profiling counters for the specified address.  This should be called
 * on each ROM/FLASH access (not per cycle)
 *
 * \param u32Addr_ - Address in ROM/FLASH being hit.
 */
void Profile_Hit( uint32_t u32Addr_ );

//---------------------------------------------------------------------------
/*!
 * \brief Profile_Print
 *
 * Display the cumulative profiling stats
 *
 */
void Profile_Print(void);


#endif

