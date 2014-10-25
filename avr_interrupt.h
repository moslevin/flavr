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
  \file  avr_interrupt.h

  \brief CPU Interrupt management
*/

#ifndef __AVR_INTERRUPT_H__
#define __AVR_INTERRUPT_H__

#include <stdint.h>
#include "emu_config.h"
#include "avr_cpu.h"

//---------------------------------------------------------------------------
/*!
 * \brief AVR_InterruptCandidate
 *
 * Given an existing interrupt candidate, determine if the selected interrupt
 * vector is of highier priority.  If higher priority, update the candidate.
 *
 * \param pstCPU_ Pointer to the CPU object to run the interrupt
 * \param u8Vector_ - Candidate interrupt vector.
 */
void AVR_InterruptCandidate( uint8_t u8Vector_ );

//---------------------------------------------------------------------------
/*!
 * \brief AVR_ClearCandidate
 *
 * \param u8Vector_ Vector to clear pending interrupt for.
 */
void AVR_ClearCandidate( uint8_t u8Vector_ );

//---------------------------------------------------------------------------
/*!
 * \brief AVR_Interrupt
 *
 * Entrypoint for CPU interrupts.  Stop executing the currently-executing
 * code, push the current PC to the stack, disable interrupts, and resume
 * execution at the new location specified in the vector table.
 *
 * \param pstCPU_ Pointer to the CPU object to run the interrupt
 */
void AVR_Interrupt( void );

#endif //__AVR_INTERRUPT_H__
