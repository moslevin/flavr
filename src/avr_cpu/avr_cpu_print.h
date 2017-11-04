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
  \file  avr_cpu_print.h

  \brief Helper module used to print the contents of a virtual AVR's internal
         registers and memory.
*/


#ifndef __AVR_CPU_PRINT_H__
#define __AVR_CPU_PRINT_H__

#include <stdint.h>
#include "avr_cpu.h"

//---------------------------------------------------------------------------
/*!
 * \brief print_core_regs
 *
 * Display the contents of the CPU's core registers to the console
 */
void print_core_regs( void );

//---------------------------------------------------------------------------
/*!
 * \brief print_io_reg
 *
 * Display a single IO register (addresses 0-255) to the console.
 *
 * \param u8Addr_ Address of the IO register to display
 */
void print_io_reg( uint8_t u8Addr_ );

//---------------------------------------------------------------------------
/*!
 * \brief print_io_reg_with_name
 *
 * Print an IO register to the console, with a "friendly" name attached.
 *
 * \param u8Addr_ Address of the IO register to display
 *
 * \param szName_ "Friendly name" of the register.
 */
void print_io_reg_with_name( uint8_t u8Addr_, const char *szName_ );

//---------------------------------------------------------------------------
/*!
 * \brief print_ram
 *
 * Display a block of RAM on the console.
 *
 * \param u16Start_ Start address
 * \param u16Span_  Number of bytes to display
 */
void print_ram( uint16_t u16Start_, uint16_t u16Span_ );

//---------------------------------------------------------------------------
/*!
 * \brief print_rom
 *
 * Display a block of ROM to the console
 *
 * \param u16Start_ Start address
 * \param u16Span_  Number of instruction words (16-bit) to display
 */
void print_rom( uint16_t u16Start_, uint16_t u16Span_ );

#endif
