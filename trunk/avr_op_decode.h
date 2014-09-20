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
  \file  avr_op_decode.h

  \brief Module providing logic to decode AVR CPU Opcodes
*/

#ifndef __AVR_OP_DECODE_H__
#define __AVR_OP_DECODE_H__

#include <stdint.h>
#include "avr_cpu.h"

//---------------------------------------------------------------------------
// Format decoder function jump table
typedef void (*AVR_Decoder)( AVR_CPU *pstCPU_, uint16_t OP_);

//---------------------------------------------------------------------------
/*!
 * \brief AVR_Decoder_Function
 * \param OP_
 * \return
 */
AVR_Decoder AVR_Decoder_Function( uint16_t OP_ );

//---------------------------------------------------------------------------
/*!
 * \brief AVR_Decode
 * \param pstCPU_
 * \param OP_
 */
void AVR_Decode( AVR_CPU *pstCPU_, uint16_t OP_ );

#endif

