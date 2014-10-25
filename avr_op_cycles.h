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
  \file  avr_op_cycles.h

  \brief Opcode cycle counting functions.
*/

#ifndef __AVR_OP_CYCLES_H__
#define __AVR_OP_CYCLES_H__

#include <stdint.h>

//---------------------------------------------------------------------------
/*!
 * \brief AVR_Opocde_Cycles
 * \param OP_ Opcode to compute the minimum cycles to execute for
 * \return The minimum number of cycles it will take to execute an opcode
 */
uint8_t AVR_Opcode_Cycles( uint16_t OP_ );

#endif
