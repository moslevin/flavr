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
 * (c) Copyright 2014, Funkenstein Software Consulting, All rights reserved
 *     See license.txt for details
 ****************************************************************************/
/*!
  \file  avr_opcodes.h

  \brief AVR CPU - Opcode interface
*/

#ifndef __AVR_OPCODES_H__
#define __AVR_OPCODES_H__

#include <stdint.h>
#include "avr_cpu.h"

//---------------------------------------------------------------------------
// Format opcode function jump table
typedef void (*AVR_Opcode)();

//---------------------------------------------------------------------------
/*!
 * \brief AVR_Opcode_Function
 *
 * Return a function pointer corresponding to the CPU logic for a given
 * opcode.
 *
 * \param OP_ Opcode to return an "opcode execution" function pointer for
 * \return Opcode execution function pointer corresponding to the given opcode.
 */
AVR_Opcode AVR_Opcode_Function( uint16_t OP_ );

//---------------------------------------------------------------------------
/*!
 * \brief AVR_RunOpcode
 *
 * Execute the instruction corresponding to the provided opcode, on the provided
 * CPU object.  Note that the opcode must have just been decoded on the given
 * CPU object before calling this function.
 * 
 * \param OP_ Opcode to execute
 */
void AVR_RunOpcode(  uint16_t OP_ );

#endif
