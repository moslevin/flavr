/****************************************************************************
 *     (     (                      (     |
 *    )\ )  )\ )    (              )\ )   |
 *   (()/( (()/(    )\     (   (  (()/(   | -- [ Funkenstein ] -------------
 *    /(_)) /(_))((((_)(   )\  )\  /(_))  | -- [ Litle ] -------------------
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
  \file  avr_op_size.h

  \brief Module providing an interface to lookup the size of an opcode
*/

#ifndef __AVR_OP_SIZE__
#define __AVR_OP_SIZE__

#include <stdint.h>

//---------------------------------------------------------------------------
/*!
 * \brief AVR_Opocde_Size
 * \param OP_
 * \return The number of words in an opcode
 */
uint8_t AVR_Opcode_Size( uint16_t OP_ );

#endif
