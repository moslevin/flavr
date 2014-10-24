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
void AVR_InterruptCandidate( uint8_t u8Vector_ )
{
    // Interrupts are prioritized by index -- lower == higher priority.
    // Candidate is the lowest
    if (u8Vector_ < stCPU.u8IntPriority)
    {
        stCPU.u8IntPriority = u8Vector_;
    }
}

//---------------------------------------------------------------------------
void AVR_Interrupt( void )
{
    // First - check to see if there's an interrupt pending.
    if (stCPU.u8IntPriority == 255)
    {
        return; // no interrupt pending
    }

    // Push the current PC to stack.
    uint16_t u16SP = (((uint16_t)stCPU.pstRAM->stRegisters.SPH.r) << 8) |
                     (((uint16_t)stCPU.pstRAM->stRegisters.SPL.r));

    uint16_t u16StoredPC = stCPU.u16PC;

    stCPU.pstRAM->au8RAM[ u16SP ]     = (uint8_t)(u16StoredPC & 0x00FF);
    stCPU.pstRAM->au8RAM[ u16SP - 1 ] = (uint8_t)(u16StoredPC >> 8);

    // Stack is post-decremented
    u16SP -= 2;

    // Store the new SP.
    stCPU.pstRAM->stRegisters.SPH.r = (u16SP >> 8);
    stCPU.pstRAM->stRegisters.SPL.r = (u16SP & 0x00FF);

    // Read the new PC from the vector table
    uint16_t u16NewPC = (uint16_t)(stCPU.u8IntPriority * 2);

    // Set the new PC
    stCPU.u16PC = u16NewPC;
    stCPU.u16ExtraPC = 0;

    // Clear the "I" (global interrupt enabled) register in the SR
    stCPU.pstRAM->stRegisters.SREG.I = 0;

    // Run the interrupt-acknowledge callback associated with this vector
    if (stCPU.u8IntPriority < 32 && stCPU.apfInterruptCallbacks[ stCPU.u8IntPriority ])
    {
        stCPU.apfInterruptCallbacks[ stCPU.u8IntPriority ]( stCPU.u8IntPriority );
    }

    // Reset the CPU interrupt priority
    stCPU.u8IntPriority = 255;

    // Clear any sleep-mode flags currently set
    stCPU.bAsleep = false;
}