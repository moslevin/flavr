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
  \file  avr_interrupt.c

  \brief CPU Interrupt management
*/

#include <stdint.h>
#include "emu_config.h"
#include "avr_cpu.h"

//---------------------------------------------------------------------------
void AVR_InterruptCandidate( AVR_CPU *pstCPU_, uint8_t u8Vector_ )
{
    // Interrupts are prioritized by index -- lower == higher priority.
    // Candidate is the lowest
    if (u8Vector_ < pstCPU_->ucIntPriority)
    {
        pstCPU_->ucIntPriority = u8Vector_;
    }
}

//---------------------------------------------------------------------------
void AVR_Interrupt( AVR_CPU *pstCPU_ )
{
    // First - check to see if there's an interrupt pending.
    if (pstCPU_->ucIntPriority == 255)
    {
        return; // no interrupt pending
    }

    // Push the current PC to stack.
    uint16_t u16SP = (((uint16_t)pstCPU_->pstRAM->stRegisters.SPH.r) << 8) |
                     (((uint16_t)pstCPU_->pstRAM->stRegisters.SPL.r));

    uint16_t u16StoredPC = pstCPU_->u16PC;

    pstCPU_->pstRAM->aucRAM[ u16SP ]     = (uint8_t)(u16StoredPC & 0x00FF);
    pstCPU_->pstRAM->aucRAM[ u16SP - 1 ] = (uint8_t)(u16StoredPC >> 8);

    // Stack is post-decremented
    u16SP -= 2;

    // Store the new SP.
    pstCPU_->pstRAM->stRegisters.SPH.r = (u16SP >> 8);
    pstCPU_->pstRAM->stRegisters.SPL.r = (u16SP & 0x00FF);

    // Read the new PC from the vector table
    uint16_t u16NewPC = (uint16_t)(pstCPU_->ucIntPriority * 2);

    // Set the new PC
    pstCPU_->u16PC = u16NewPC;
    pstCPU_->u16ExtraPC = 0;

    // Clear the "I" (global interrupt enabled) register in the SR
    pstCPU_->pstRAM->stRegisters.SREG.I = 0;

    // Run the interrupt-acknowledge callback associated with this vector
    if (pstCPU_->ucIntPriority < 32 && pstCPU_->apfInterruptCallbacks[ pstCPU_->ucIntPriority ])
    {
        pstCPU_->apfInterruptCallbacks[ pstCPU_->ucIntPriority ]( pstCPU_, pstCPU_->ucIntPriority );
    }

    // Reset the CPU interrupt priority
    pstCPU_->ucIntPriority = 255;

    // Clear any sleep-mode flags currently set
    pstCPU_->bAsleep = false;
}
