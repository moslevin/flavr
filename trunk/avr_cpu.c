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
  \file  avr_cpu.c

  \brief AVR CPU emulator logic - this module contains the entrypoints
         required to implement CPU instruction fetch/decode/execute operations,
         using either lookup tables or direct-decoding logic.
*/

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "emu_config.h"

#include "avr_cpu.h"
#include "avr_peripheral.h"
#include "avr_interrupt.h"
#include "avr_io.h"
#include "avr_op_decode.h"
#include "avr_op_size.h"
#include "avr_opcodes.h"
#include "avr_op_cycles.h"

#include "trace_buffer.h"

#if FEATURE_USE_JUMPTABLES
//---------------------------------------------------------------------------
// 2 levels of jump tables are required for AVR.

// The first is to implement addressing mode detection (which we then use to
// seed the appropriate intermediate register pointers in the AVR_CPU struct).

// This greatly reduces opcode function complexity, saves lots of code.
// Second-level is a pure jump-table to opcode function pointers, where the
// CPU register pointers are used w/AVR_CPU struct data to execute the opcode.

//---------------------------------------------------------------------------

static AVR_Decoder astDecoders[65536] = { 0 };
static AVR_Opcode  astOpcodes[65536] = { 0 };
static uint8_t     aucOpSizes[65536] = { 0 };
static uint8_t     aucOpCycles[65536] = { 0 };

#endif

//---------------------------------------------------------------------------
static void CPU_Decode( AVR_CPU *pstCPU_, uint16_t OP_ )
{
#if FEATURE_USE_JUMPTABLES
    astDecoders[OP_](pstCPU_, OP_);
#else
    AVR_Decoder pfOp = AVR_Decoder_Function( OP_ );
    pfOP( pstCPU_, OP_ );
#endif
}

//---------------------------------------------------------------------------
static void CPU_Execute( AVR_CPU *pstCPU_, uint16_t OP_ )
{
#if FEATURE_USE_JUMPTABLES
    astOpcodes[OP_](pstCPU_);
#else
    AVR_Opcode pfOp = AVR_Opcode_Function(OP_);
    pfOP( pstCPU_, OP_ );
#endif
}

//---------------------------------------------------------------------------
uint16_t CPU_Fetch( AVR_CPU *pstCPU_ )
{
    uint16_t PC = pstCPU_->u16PC;
    if (PC >= 16384)
    {
        return 0xFFFF;
    }
    return pstCPU_->pusROM[ pstCPU_->u16PC ];
}

//---------------------------------------------------------------------------
static void CPU_GetOpCycles( AVR_CPU *pstCPU_, uint16_t OP_ )
{
#if FEATURE_USE_JUMPTABLES
    pstCPU_->u16ExtraCycles = aucOpCycles[ OP_ ];
#else
    pstCPU_->u16ExtraCycles = AVR_Opcode_Cycles( OP_ );
#endif
}

//---------------------------------------------------------------------------
static void CPU_GetOpSize( AVR_CPU *pstCPU_, uint16_t OP_ )
{
#if FEATURE_USE_JUMPTABLES
    pstCPU_->u16ExtraPC = aucOpSizes[ OP_ ];
#else
    pstCPU_->u16ExtraPC = AVR_Opcode_Size( OP_ );
#endif
}

//---------------------------------------------------------------------------
static void CPU_PeripheralCycle( AVR_CPU *pstCPU_ )
{
    IO_Clock(pstCPU_);
}

//---------------------------------------------------------------------------
void CPU_RunCycle( AVR_CPU *pstCPU_ )
{
    uint16_t OP;

    if (!pstCPU_->bAsleep)
    {

        OP = CPU_Fetch( pstCPU_ );

        // From the first word fetched, figure out how big this opcode is
        // (either 16 or 32-bit)
        CPU_GetOpSize( pstCPU_, OP );

        // Based on the first word fetched, figure out the minimum number of
        // CPU cycles required to execute the instruction fetched.
        CPU_GetOpCycles( pstCPU_, OP );

        // Decode the instruction, load internal registers with appropriate
        // values.
        CPU_Decode( pstCPU_, OP );

        // Execute the instruction that was just decoded
        CPU_Execute( pstCPU_, OP );

        // Update the PC based on the size of the instruction + whatever
        // modifications occurred during the execution cycle.
        pstCPU_->u16PC += pstCPU_->u16ExtraPC;

        // Add CPU clock cycles to the global cycle counter based on
        // the minimum instruction time, plus whatever modifiers are applied
        // during execution of the instruction.
        pstCPU_->u64CycleCount += pstCPU_->u16ExtraCycles;

        // Cycle-accurate peripheral clocking -- one iteration for each
        // peripheral for each CPU cycle of the instruction.
        // Note that CPU Interrupts are generated in the peripheral
        // phase of the instruction cycle.
        while (pstCPU_->u16ExtraCycles--)
        {
            CPU_PeripheralCycle( pstCPU_ );
        }

        // Increment the "total executed instruction counter"
        pstCPU_->u64InstructionCount++;                

    }
    else
    {
        // CPU is asleep, just NOP and wait until we hit an interrupt.
        pstCPU_->u64CycleCount++;
        CPU_PeripheralCycle( pstCPU_ );                
    }

    // Check to see if there are any pending interrupts - if so, vector
    // to the appropriate location.  This has no effect if no interrupts
    // are pending
    AVR_Interrupt( pstCPU_ );
}


#if FEATURE_USE_JUMPTABLES
//---------------------------------------------------------------------------
static void CPU_BuildDecodeTable(void)
{
    uint32_t i;
    for (i = 0; i < 65536; i++)
    {
        astDecoders[i] = AVR_Decoder_Function(i);
    }
}

//---------------------------------------------------------------------------
static void CPU_BuildOpcodeTable(void)
{
    uint32_t i;
    for (i = 0; i < 65536; i++)
    {
        astOpcodes[i] = AVR_Opcode_Function(i);
    }
}

//---------------------------------------------------------------------------
static void CPU_BuildSizeTable(void)
{
    uint32_t i;
    for (i = 0; i < 65536; i++)
    {
        aucOpSizes[i] = AVR_Opcode_Size(i);
    }
}

//---------------------------------------------------------------------------
static void CPU_BuildCycleTable(void)
{
    uint32_t i;
    for (i = 0; i < 65536; i++)
    {
        aucOpCycles[i] = AVR_Opcode_Cycles(i);
    }
}
#endif

//---------------------------------------------------------------------------
void CPU_Init( AVR_CPU *pstCPU_, AVR_CPU_Config_t *pstConfig_ )
{
    memset(pstCPU_, 0, sizeof(*pstCPU_));
    pstConfig_->u32RAMSize += 256;

    // Dynamically allocate memory for RAM, ROM, and EEPROM buffers
    pstCPU_->pucEEPROM = (uint8_t*)malloc( pstConfig_->u32EESize );
    pstCPU_->pusROM    = (uint16_t*)malloc( pstConfig_->u32ROMSize );
    pstCPU_->pstRAM    = (AVR_RAM_t*)malloc( pstConfig_->u32RAMSize );

    memset( pstCPU_->pucEEPROM, 0, pstConfig_->u32EESize );
    memset( pstCPU_->pusROM, 0, pstConfig_->u32ROMSize );
    memset( pstCPU_->pstRAM, 0, pstConfig_->u32RAMSize );

    // Set the base stack pointer to top-of-ram.
    pstCPU_->pstRAM->stRegisters.SPH.r = 0x08;
    pstCPU_->pstRAM->stRegisters.SPL.r = 0xFF;

    // Reset the interrupt priority register
    pstCPU_->ucIntPriority = 255;

#if FEATURE_USE_JUMPTABLES
    CPU_BuildCycleTable();
    CPU_BuildSizeTable();
    CPU_BuildOpcodeTable();
    CPU_BuildDecodeTable();
#endif
}

//---------------------------------------------------------------------------
void CPU_AddPeriph( AVR_CPU *pstCPU_, AVRPeripheral *pstPeriph_ )
{    
    IO_AddClocker( pstCPU_, pstPeriph_ );

    uint8_t i;
    for (i = pstPeriph_->u8AddrStart; i <= pstPeriph_->u8AddrEnd; i++)
    {
        IO_AddReader( pstCPU_, pstPeriph_, i );
        IO_AddWriter( pstCPU_, pstPeriph_, i );
    }

    if (pstPeriph_->pfInit)
    {
        pstPeriph_->pfInit( pstPeriph_->pvContext, pstCPU_ );
    }
}

//---------------------------------------------------------------------------
void CPU_RegisterInterruptCallback( AVR_CPU *pstCPU_, InterruptAck pfIntAck_, uint8_t ucVector_ )
{
    if (ucVector_ >= 32)
    {
        return;
    }

    pstCPU_->apfInterruptCallbacks[ ucVector_ ] = pfIntAck_;
}
