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
  \file  avr_op_decode.h

  \brief Module providing logic to decode AVR CPU Opcodes
*/

#ifndef __AVR_OP_DECODE_H__
#define __AVR_OP_DECODE_H__

#include <stdint.h>
#include "avr_cpu.h"

//---------------------------------------------------------------------------
// Format decoder function jump table
typedef void (*AVR_Decoder)( uint16_t OP_);

//---------------------------------------------------------------------------
/*!
 * \brief AVR_Decoder_Function
 *
 *  Returns an "instruction decode" function pointer to the caller for a
 *  given opcode.
 *
 * \param OP_ Opcode to return the instruction decode function for
 * \return Pointer to an opcode/instruction decoder routine
 */
AVR_Decoder AVR_Decoder_Function( uint16_t OP_ );

//---------------------------------------------------------------------------
/*!
 * \brief AVR_Decode
 *
 * Decode a specified instruction into the internal registers of the
 * CPU object.  Opcodes must be decoded before they can be executed.
 *
 * \param OP_ Opcode to decode
 */
void AVR_Decode( uint16_t OP_ );

#endif

