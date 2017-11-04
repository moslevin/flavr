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
  \file  avr_op_size.h

  \brief Module providing an interface to lookup the size of an opcode
*/

#ifndef __AVR_OP_SIZE__
#define __AVR_OP_SIZE__

#include <stdint.h>

//---------------------------------------------------------------------------
/*!
 * \brief AVR_Opocde_Size
 *
 * Return the number of bytes are in a specific opcode based on a 16-bt first
 * opcode word.
 *
 * \param OP_ Opcode word to determine instruction size for
 *
 * \return The number of words in an instruction
 */
uint8_t AVR_Opcode_Size( uint16_t OP_ );

#endif
