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
#include <stdlib.h>

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

AVR_CPU stCPU;

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
static uint8_t     au8OpSizes[65536] = { 0 };
static uint8_t     au8OpCycles[65536] = { 0 };

#endif

//---------------------------------------------------------------------------
static void CPU_Decode( uint16_t OP_ )
{
#if FEATURE_USE_JUMPTABLES
    astDecoders[OP_]( OP_);
#else
    AVR_Decoder pfOp = AVR_Decoder_Function( OP_ );
    pfOP(  OP_ );
#endif
}

//---------------------------------------------------------------------------
static void CPU_Execute( uint16_t OP_ )
{
#if FEATURE_USE_JUMPTABLES
    astOpcodes[OP_]();
#else
    AVR_Opcode pfOp = AVR_Opcode_Function(OP_);
    pfOP(  OP_ );
#endif
}

//---------------------------------------------------------------------------
uint16_t CPU_Fetch( void )
{
    uint16_t PC = stCPU.u16PC;
    if (PC >= 16384)
    {
        return 0xFFFF;
    }
    return stCPU.pu16ROM[ stCPU.u16PC ];
}

//---------------------------------------------------------------------------
static void CPU_GetOpCycles( uint16_t OP_ )
{
#if FEATURE_USE_JUMPTABLES
    stCPU.u16ExtraCycles = au8OpCycles[ OP_ ];
#else
    stCPU.u16ExtraCycles = AVR_Opcode_Cycles( OP_ );
#endif
}

//---------------------------------------------------------------------------
static void CPU_GetOpSize( uint16_t OP_ )
{
#if FEATURE_USE_JUMPTABLES
    stCPU.u16ExtraPC = au8OpSizes[ OP_ ];
#else
    stCPU.u16ExtraPC = AVR_Opcode_Size( OP_ );
#endif
}

//---------------------------------------------------------------------------
static void CPU_PeripheralCycle( void )
{
    IO_Clock();
}

//---------------------------------------------------------------------------
void CPU_RunCycle( void )
{
    uint16_t OP;

    if (!stCPU.bAsleep)
    {

        OP = CPU_Fetch(  );

        // From the first word fetched, figure out how big this opcode is
        // (either 16 or 32-bit)
        CPU_GetOpSize(  OP );

        // Based on the first word fetched, figure out the minimum number of
        // CPU cycles required to execute the instruction fetched.
        CPU_GetOpCycles(  OP );

        // Decode the instruction, load internal registers with appropriate
        // values.
        CPU_Decode(  OP );

        // Execute the instruction that was just decoded
        CPU_Execute(  OP );

        // Update the PC based on the size of the instruction + whatever
        // modifications occurred during the execution cycle.
        stCPU.u16PC += stCPU.u16ExtraPC;

        // Add CPU clock cycles to the global cycle counter based on
        // the minimum instruction time, plus whatever modifiers are applied
        // during execution of the instruction.
        stCPU.u64CycleCount += stCPU.u16ExtraCycles;

        // Cycle-accurate peripheral clocking -- one iteration for each
        // peripheral for each CPU cycle of the instruction.
        // Note that CPU Interrupts are generated in the peripheral
        // phase of the instruction cycle.
        while (stCPU.u16ExtraCycles--)
        {
            CPU_PeripheralCycle(  );
        }

        // Increment the "total executed instruction counter"
        stCPU.u64InstructionCount++;

    }
    else
    {
        // CPU is asleep, just NOP and wait until we hit an interrupt.
        stCPU.u64CycleCount++;
        CPU_PeripheralCycle(  );
    }

    // Check to see if there are any pending interrupts - if so, vector
    // to the appropriate location.  This has no effect if no interrupts
    // are pending
    AVR_Interrupt(  );
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
        au8OpSizes[i] = AVR_Opcode_Size(i);
    }
}

//---------------------------------------------------------------------------
static void CPU_BuildCycleTable(void)
{
    uint32_t i;
    for (i = 0; i < 65536; i++)
    {
        au8OpCycles[i] = AVR_Opcode_Cycles(i);
    }
}
#endif

//---------------------------------------------------------------------------
void CPU_Init( AVR_CPU_Config_t *pstConfig_ )
{
    memset( &stCPU, 0, sizeof(stCPU));
    pstConfig_->u32RAMSize += 256;

    // Dynamically allocate memory for RAM, ROM, and EEPROM buffers
    stCPU.pu8EEPROM = (uint8_t*)malloc( pstConfig_->u32EESize );
    stCPU.pu16ROM    = (uint16_t*)malloc( pstConfig_->u32ROMSize );
    stCPU.pstRAM    = (AVR_RAM_t*)malloc( pstConfig_->u32RAMSize );

    stCPU.u32ROMSize = pstConfig_->u32ROMSize;
    stCPU.u32RAMSize = pstConfig_->u32RAMSize;
    stCPU.u32EEPROMSize = pstConfig_->u32EESize;

    memset( stCPU.pu8EEPROM, 0, pstConfig_->u32EESize );
    memset( stCPU.pu16ROM, 0, pstConfig_->u32ROMSize );
    memset( stCPU.pstRAM, 0, pstConfig_->u32RAMSize );

    // Set the base stack pointer to top-of-ram.
    uint16_t u16InitialStack = 256 + pstConfig_->u32RAMSize - 1;
    stCPU.pstRAM->stRegisters.SPH.r = (uint8_t)(u16InitialStack >> 8);
    stCPU.pstRAM->stRegisters.SPL.r = (uint8_t)(u16InitialStack & 0xFF);

    // Reset the interrupt priority register
    stCPU.u8IntPriority = 255;

#if FEATURE_USE_JUMPTABLES
    CPU_BuildCycleTable();
    CPU_BuildSizeTable();
    CPU_BuildOpcodeTable();
    CPU_BuildDecodeTable();
#endif
}

//---------------------------------------------------------------------------
void CPU_AddPeriph( AVRPeripheral *pstPeriph_ )
{    
    IO_AddClocker(  pstPeriph_ );

    uint8_t i;
    for (i = pstPeriph_->u8AddrStart; i <= pstPeriph_->u8AddrEnd; i++)
    {
        IO_AddReader(  pstPeriph_, i );
        IO_AddWriter(  pstPeriph_, i );
    }

    if (pstPeriph_->pfInit)
    {
        pstPeriph_->pfInit( pstPeriph_->pvContext );
    }
}

//---------------------------------------------------------------------------
void CPU_RegisterInterruptCallback( InterruptAck pfIntAck_, uint8_t ucVector_ )
{
    if (ucVector_ >= 32)
    {
        return;
    }

    stCPU.apfInterruptCallbacks[ ucVector_ ] = pfIntAck_;
}
