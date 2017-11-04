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
 * (c) Copyright 2014-17, Funkenstein Software Consulting, All rights reserved
 *     See license.txt for details
 ****************************************************************************/
/*!
  \file  flavr.c

  \brief Main AVR emulator entrypoint, commandline-use with built-in
         interactive debugger.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "emu_config.h"
#include "variant.h"

//---------------------------------------------------------------------------
#include "avr_coreregs.h"
#include "avr_periphregs.h"
#include "avr_op_cycles.h"
#include "avr_op_decode.h"
#include "avr_op_size.h"
#include "avr_cpu_print.h"
#include "avr_cpu.h"
#include "avr_loader.h"

//---------------------------------------------------------------------------
#include "mega_uart.h"
#include "mega_eint.h"
#include "mega_timer16.h"
#include "mega_timer8.h"
#include "mega_eeprom.h"

//---------------------------------------------------------------------------
#include "avr_disasm.h"
#include "trace_buffer.h"
#include "options.h"
#include "interactive.h"
#include "breakpoint.h"
#include "watchpoint.h"
#include "kernel_aware.h"
#include "code_profile.h"
#include "tlv_file.h"
#include "gdb_rsp.h"

//---------------------------------------------------------------------------
typedef enum
{
    EEPROM_TOO_BIG,
    RAM_TOO_BIG,
    RAM_TOO_SMALL,
    ROM_TOO_BIG,
    INVALID_HEX_FILE,
    INVALID_VARIANT,
    INVALID_DEBUG_OPTIONS
} ErrorReason_t;

//---------------------------------------------------------------------------
static TraceBuffer_t stTraceBuffer;

//---------------------------------------------------------------------------
void splash(void)
{
printf(
       "* ---------------------------------------+----------------------------------\n"
       "*     (     (                      (     |\n"
       "*    )\\ )  )\\ )    (              )\\ )   |\n"
       "*   (()/( (()/(    )\\     (   (  (()/(   | -- [ Funkenstein ] -------------\n"
       "*    /(_)) /(_))((((_)()\\  )\\  /(_))     | -- [ Litle ] -------------------\n"
       "*   (_))_|(_))   )\\ _ )\\ ((_)((_)(_))    | -- [ AVR ] ---------------------\n"
       "*   | |_  | |    (_)_\\(_)\\ \\ / / | _ \\   | -- [ Virtual ] -----------------\n"
       "*   | __| | |__   / _ \\   \\ V /  |   /   | -- [ Runtime ] -----------------\n"
       "*   |_|   |____| /_/ \\_\\   \\_/   |_|_\\   |\n"
       "*                                        | \"From the makers of Mark3!\"\n"
       "* ---------------------------------------+----------------------------------\n"
       "* (c) Copyright 2014-17, Funkenstein Software Consulting, All rights reserved\n"
       "*     See license.txt for details\n"
      );
}

//---------------------------------------------------------------------------
void error_out( ErrorReason_t eReason_ )
{
    switch (eReason_)
    {
        case EEPROM_TOO_BIG:
            printf( "EERPOM Size specified is too large\n" );
            break;
        case RAM_TOO_BIG:
            printf( "RAM Size specified is too large\n" );
            break;
        case RAM_TOO_SMALL:
            printf( "RAM Size specified is too small\n" );
            break;
        case ROM_TOO_BIG:
            printf( "ROM Size specified is too large\n" );
            break;
        case INVALID_HEX_FILE:
            printf( "HEX Programming file cannot be loaded\n");
            break;
        case INVALID_VARIANT:
            printf( "Unknown variant not supported\n");
            break;
        case INVALID_DEBUG_OPTIONS:
            printf( "GDB and built-in interactive debugger are mutually exclusive\n");
        default:
            printf( "Some other reason\n" );
    }

    Options_PrintUsage();

    exit (-1);
}

//---------------------------------------------------------------------------
void emulator_loop(void)
{
    bool bUseTrace = false;
    bool bProfile = false;
    bool bUseGDB = false;

    if ( Options_GetByName("--trace") && Options_GetByName("--debug") )
    {
        bUseTrace = true;
    }

    if ( Options_GetByName("--profile"))
    {
        bProfile = true;
    }

    if ( Options_GetByName("--gdb"))
    {
        bUseGDB = true;
    }

    while (1)
    {
        // Check to see if we've hit a breakpoint
        if (BreakPoint_EnabledAtAddress(stCPU.u32PC))
        {
            if (bUseGDB)
            {
                GDB_Set();
            }
            else
            {
                Interactive_Set();
            }
        }

        // Check to see if we're in interactive debug mode, and thus need to wait for input
        if (bUseGDB)
        {
            GDB_CheckAndExecute();
        }
        else
        {
            Interactive_CheckAndExecute();
        }

        // Store the current CPU state into the tracebuffer
        if (bUseTrace)
        {
            TraceBuffer_StoreFromCPU(&stTraceBuffer);
        }

        // Run code profiling logic
        if (bProfile)
        {
            Profile_Hit(stCPU.u32PC);
        }

        // Execute a machine cycle
        CPU_RunCycle();
    }
    // doesn't return, except by quitting from debugger, or by signal.
}

//---------------------------------------------------------------------------
void add_plugins(void)
{
    CPU_AddPeriph(&stUART);
    CPU_AddPeriph(&stEINT_a);
    CPU_AddPeriph(&stEINT_b);
    CPU_AddPeriph(&stTimer16);
    CPU_AddPeriph(&stTimer16a);
    CPU_AddPeriph(&stTimer16b);
    CPU_AddPeriph(&stTimer8);
    CPU_AddPeriph(&stTimer8a);
    CPU_AddPeriph(&stTimer8b);
    CPU_AddPeriph(&stEEPROM);
}

//---------------------------------------------------------------------------
void flavr_disasm(void)
{
    uint32_t u32Size;

    u32Size = stCPU.u32ROMSize / sizeof(uint16_t);
    stCPU.u32PC = 0;

    while (stCPU.u32PC < u32Size)
    {
        uint16_t OP = stCPU.pu16ROM[stCPU.u32PC];
        char szBuf[256];

        printf("0x%04X: [0x%04X] ", stCPU.u32PC, OP);
        AVR_Decode(OP);
        AVR_Disasm_Function(OP)(szBuf);
        printf( "%s", szBuf );
        stCPU.u32PC += AVR_Opcode_Size(OP);
    }
    exit(0);
}

//---------------------------------------------------------------------------
void emulator_init(void)
{
    AVR_CPU_Config_t stConfig;

    // -- Initialize the emulator based on command-line args
    const AVR_Variant_t *pstVariant;

    pstVariant = Variant_GetByName( Options_GetByName("--variant") );
    if (!pstVariant)
    {
        error_out( INVALID_VARIANT );
    }

    if (Options_GetByName("--exitreset"))
    {
        stConfig.bExitOnReset = true;
    }
    else
    {
        stConfig.bExitOnReset = false;
    }

    stConfig.u32EESize  = pstVariant->u32EESize;
    stConfig.u32RAMSize = pstVariant->u32RAMSize;
    stConfig.u32ROMSize = pstVariant->u32ROMSize;

    if (stConfig.u32EESize >= 32768)
    {
        error_out( EEPROM_TOO_BIG );
    }

    if (stConfig.u32RAMSize >= 65535)
    {
        error_out( RAM_TOO_BIG );
    }
    else if (stConfig.u32RAMSize < 256)
    {
        error_out( RAM_TOO_SMALL );
    }

    if (stConfig.u32ROMSize >= (256*1024))
    {
        error_out( ROM_TOO_BIG );
    }

    CPU_Init(&stConfig);

    TraceBuffer_Init( &stTraceBuffer );

    if (Options_GetByName("--hexfile"))
    {
        if (!AVR_Load_HEX( Options_GetByName("--hexfile") ))
        {
            error_out( INVALID_HEX_FILE );
        }
    }
    else if (Options_GetByName("--elffile"))
    {
        if (!AVR_Load_ELF( Options_GetByName("--elffile") ))
        {
            error_out( INVALID_HEX_FILE );
        }
    }
    else
    {
        error_out( INVALID_HEX_FILE );
    }

    if (Options_GetByName("--disasm"))
    {
        // terminates after disassembly is complete
        flavr_disasm();
    }

    if (Options_GetByName("--debug"))
    {
        Interactive_Init( &stTraceBuffer );
    }
    if (Options_GetByName("--gdb"))
    {
        GDB_Init();
    }

    // Only insert a breakpoint/enter interactive debugging mode if specified.
    // Otherwise, start with the emulator running.
    if (Options_GetByName("--debug") && Options_GetByName("--gdb"))
    {
        error_out( INVALID_DEBUG_OPTIONS );
    }
    if (Options_GetByName("--debug"))
    {
        BreakPoint_Insert( 0 );
    }

    add_plugins();

    if (Options_GetByName("--mark3") || Options_GetByName("--profile"))
    {
        // Initialize tag-length-value code if we're running with code
        // profiling or kernel-aware debugging, since they generate a
        // lot of data that's better stored in a binary format for
        // efficiency.
        TLV_WriteInit( "flavr.tlv" );
    }

    if (Options_GetByName("--mark3"))
    {
        // Mark3 kernel-aware mode should only be enabled on-demand
        KernelAware_Init();
    }

    if (Options_GetByName("--profile"))
    {
        Profile_Init( stConfig.u32ROMSize );
        atexit( Profile_Print );
    }
}

//---------------------------------------------------------------------------
int main( int argc, char **argv )
{    

    // Initialize all emulator data
    Options_Init(argc, argv);

    if (!Options_GetByName("--silent"))
    {
        splash();
    }

    emulator_init();

    // Run the emulator/debugger loop.
    emulator_loop();

    return 0;

}
