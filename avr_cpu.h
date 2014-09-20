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
  \file  avr_cpu.h

  \brief AVR CPU Emulator - virtual AVR structure declarations and functions
         used to drive the emulator (fetch/decode/execute).
*/

#ifndef __AVR_CPU_H__
#define __AVR_CPU_H__

#include <stdint.h>
#include <stdbool.h>

#include "emu_config.h"

#include "avr_peripheral.h"
#include "avr_periphregs.h"
#include "avr_coreregs.h"
#include "avr_registerfile.h"
#include "avr_io.h"

#include "watchpoint.h"
#include "breakpoint.h"

//---------------------------------------------------------------------------
// Compile-time emulator configuration
//---------------------------------------------------------------------------
typedef struct
{
    union
    {
        AVRRegisterFile stRegisters;
        uint8_t aucRAM[ sizeof(AVRRegisterFile) ];
    };
} AVR_RAM_t;    // Note - the true size of this struct is defined at runtime based on the configured RAM size.

//---------------------------------------------------------------------------
struct _AVR_CPU
{
    //---------------------------------------------------------------------------
    // Jump tables for peripheral read/write functions.  This implementaton uses
    // a table with function pointer arrays, enabling multiple peripherals to
    // monitor reads/writes at particular addresses efficiently.
    //---------------------------------------------------------------------------
    IOReaderList *apstPeriphReadTable[CONFIG_IO_ADDRESS_BYTES];
    IOWriterList *apstPeriphWriteTable[CONFIG_IO_ADDRESS_BYTES];
    IOClockList  *pstClockList;

    //---------------------------------------------------------------------------
    // List of data watchpoints
    struct _WatchPoint *pstWatchPoints;

    //---------------------------------------------------------------------------
    // List of instruction breakpoints
    struct _BreakPoint *pstBreakPoints;

    //---------------------------------------------------------------------------
    // Internal CPU Registers (not exposed via IO space)
    uint16_t     u16PC;         // Program counter is not memory mapped, unlike all others

    //---------------------------------------------------------------------------
    // Emulator variables
    uint64_t     u64InstructionCount; // Total Executed instructions
    uint64_t     u64CycleCount; // Cycle Counter
    uint32_t     u32CoreFreq;   // CPU Frequency (Hz)
    uint32_t     u32WDTCount;   // Current watchdog timer count
    uint16_t     u16ExtraPC;    // Offset to add to the PC after executing an instruction
    uint16_t     u16ExtraCycles;// CPU Cycles to add for the current instruction

    bool         bAsleep;       // Whether or not the CPU is sleeping (wake by interrupt)
    //---------------------------------------------------------------------------
    // Temporary registers used for optimizing opcodes - for various addressing modes
    uint16_t    *Rd16;
    uint8_t     *Rd; // Destination register (in some cases, also source)

    uint16_t    *Rr16;
    uint8_t     *Rr; // Source register

    uint16_t    K; // Constant data
    union
    {
        uint32_t    k;   // Constant address
        int32_t     k_s; // Signed, constant address
    };

    uint8_t     A; // IO location address
    uint8_t     b; // Bit in a register file (3-bits wide)
    uint8_t     s; // BIt in the status register (3-bits wide)
    uint8_t     q; // Displacement for direct addressing (6-bits)

    //---------------------------------------------------------------------------
    // Setting up regions of memory for general-purpose RAM (shared with the
    // IO space from 0-0xFF), ROM/FLASH, and EEPROM.
    //---------------------------------------------------------------------------
    uint16_t     *pusROM;
    uint8_t      *pucEEPROM;
    AVR_RAM_t    *pstRAM;

    //---------------------------------------------------------------------------
    uint8_t     ucIntPriority;  // Priority of pending interrupts this cycle
};
typedef struct _AVR_CPU AVR_CPU;

//---------------------------------------------------------------------------
typedef struct
{
    uint32_t u32ROMSize;
    uint32_t u32RAMSize;
    uint32_t u32EESize;
} AVR_CPU_Config_t;

//---------------------------------------------------------------------------
/*!
 * \brief CPU_Init Initialize the CPU object and its associated data
 * \param pstCPU_ Pointer to a valid AVR CPU object
 * \param pstConfig_ Pointer to an initialized AVR_CPU_Config_t struct
 */
void CPU_Init( AVR_CPU *pstCPU_, AVR_CPU_Config_t *pstConfig_ );

//---------------------------------------------------------------------------
/*!
 * \brief CPU_Fetch Fetch the next opcode for the CPU object
 * \param pstCPU_ Pointer to a valid AVR CPU object
 * \return First word of the next opcode
 */
uint16_t CPU_Fetch( AVR_CPU *pstCPU_ );

//---------------------------------------------------------------------------
/*!
 * \brief CPU_RunCycle Run a CPU instruction cycle
 * \param pstCPU_ Pointer to a valid AVR CPU object
 */
void CPU_RunCycle( AVR_CPU *pstCPU_ );

//---------------------------------------------------------------------------
/*!
 * \brief CPU_AddPeriph
 * \param pstCPU_
 * \param pstPeriph_
 */
void CPU_AddPeriph( AVR_CPU *pstCPU_, AVRPeripheral *pstPeriph_ );
#endif
