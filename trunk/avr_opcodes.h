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
  \file  avr_opcodes.h

  \brief AVR CPU - Opcode interface
*/

#ifndef __AVR_OPCODES_H__
#define __AVR_OPCODES_H__

#include <stdint.h>
#include "avr_cpu.h"

//---------------------------------------------------------------------------
// Format opcode function jump table
typedef void (*AVR_Opcode)( AVR_CPU *pstCPU_);

//---------------------------------------------------------------------------
/*!
 * \brief AVR_Opcode_Function
 * \param pstCPU_
 * \return
 */
AVR_Opcode AVR_Opcode_Function( uint16_t OP_ );

//---------------------------------------------------------------------------
/*!
 * \brief AVR_RunOpcode
 * \param pstCPU_
 * \param OP_
 */
void AVR_RunOpcode( AVR_CPU *pstCPU_,  uint16_t OP_ );

#endif
