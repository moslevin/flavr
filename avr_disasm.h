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
  \file  avr_disasm.h

  \brief AVR Disassembler Implementation
*/

#ifndef __AVR_DISASM_H__
#define __AVR_DISASM_H__

#include "avr_opcodes.h"

/*!
 * \brief AVR_Disasm_Function
 *
 * Return a function pointer to a disassembly routine corresponding to a
 * given opcode.
 *
 * \param OP_ Opcode to disasemble
 * \return Function pointer that, when called with a valid CPU object and
 *         opcode, will produce a valid disassembly statement to standard
 *         output.
 */
AVR_Opcode AVR_Disasm_Function( uint16_t OP_ );


#endif
