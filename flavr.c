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
  \file  flavr.c

  \brief Main AVR emulator entrypoint, commandline-use with built-in
         interactive debugger.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "emu_config.h"

//---------------------------------------------------------------------------
#include "avr_coreregs.h"
#include "avr_periphregs.h"
#include "avr_op_cycles.h"
#include "avr_op_decode.h"
#include "avr_op_size.h"
#include "avr_cpu_print.h"
#include "avr_cpu.h"
#include "avr_loader.h"

#include "avr_disasm.h"
#include "trace_buffer.h"
#include "options.h"
#include "interactive.h"
#include "breakpoint.h"
#include "watchpoint.h"

//---------------------------------------------------------------------------
#include "mega_uart.h"
#include "mega_eint.h"
#include "mega_timer16.h"

//---------------------------------------------------------------------------
typedef enum
{
    EEPROM_TOO_BIG,
    RAM_TOO_BIG,
    RAM_TOO_SMALL,
    ROM_TOO_BIG,
    INVALID_HEX_FILE
} ErrorReason_t;

//---------------------------------------------------------------------------
static TraceBuffer_t stTraceBuffer;
static AVR_CPU stCPU;

//---------------------------------------------------------------------------
void splash(void)
{
printf(
       "* ---------------------------------------+----------------------------------\n"
       "*     (     (                      (     |\n"
       "*    )\\ )  )\\ )    (              )\\ )   |\n"
       "*   (()/( (()/(    )\\     (   (  (()/(   | -- [ Funkenstein ] -------------\n"
       "*    /(_)) /(_))((((_)(   )\\  )\\  /(_))  | -- [ Litle ] -------------------\n"
       "*   (_))_|(_))   )\\ _ )\\ ((_)((_)(_))    | -- [ AVR ] ---------------------\n"
       "*   | |_  | |    (_)_\\(_)\\ \\ / / | _ \\   | -- [ Virtual ] -----------------\n"
       "*   | __| | |__   / _ \\   \\ V /  |   /   | -- [ Runtime ] -----------------\n"
       "*   |_|   |____| /_/ \\_\\   \\_/   |_|_\\   |\n"
       "*                                        | \"From the makers of Mark3!\"\n"
       "* ---------------------------------------+----------------------------------\n"
       "* (c) Copyright 2014, Funkenstein Software Consulting, All rights reserved\n"
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
        default:
            printf( "Some other reason\n" );
    }
    exit (-1);
}

//---------------------------------------------------------------------------
void emulator_loop(void)
{
    while (1)
    {
        // Check to see if we've hit a breakpoint
        if (BreakPoint_EnabledAtAddress(&stCPU, stCPU.u16PC))
        {
            Interactive_Set();
        }

        // Check to see if we're in interactive debug mode, and thus need to wait for input
        Interactive_CheckAndExecute();

        // Store the current CPU state into the tracebuffer
        TraceBuffer_StoreFromCPU(&stTraceBuffer, &stCPU);

        // Execute a machine cycle
        CPU_RunCycle( &stCPU );
    }
    // doesn't return, except by quitting from debugger, or by signal.
}

//---------------------------------------------------------------------------
void add_plugins(void)
{
    CPU_AddPeriph(&stCPU, &stUART);
    CPU_AddPeriph(&stCPU, &stEINT_a);
    CPU_AddPeriph(&stCPU, &stEINT_b);
    CPU_AddPeriph(&stCPU, &stTimer16);
}


//---------------------------------------------------------------------------
void emulator_init(void)
{
    AVR_CPU_Config_t stConfig;

    // -- Initialize the emulator based on command-line args
    stConfig.u32RAMSize = atoi(Options_GetByName("--ramsize"));
    stConfig.u32ROMSize = atoi(Options_GetByName("--romsize"));
    stConfig.u32EESize = atoi(Options_GetByName("--eesize"));

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

    CPU_Init(&stCPU, &stConfig);
    TraceBuffer_Init( &stTraceBuffer);
    Interactive_Init( &stCPU, &stTraceBuffer );
    BreakPoint_Insert( &stCPU, 0 );

    if (Options_GetByName("--hexfile"))
    {
        if( !AVR_Load_HEX( &stCPU, Options_GetByName("--hexfile") ) ) {
            error_out( INVALID_HEX_FILE );
        }
    }
    // -- Obviously, this is debug code...
    else
    {
        if( !AVR_Load_HEX( &stCPU, Options_GetByName("Markade2.hex") ) ) {
            error_out( INVALID_HEX_FILE );
        }
    }

    add_plugins();
}

//---------------------------------------------------------------------------
int main( int argc, char **argv )
{    
    splash();

    // Initialize all emulator data
    Options_Init(argc, argv);
    emulator_init();

    // Run the emulator/debugger loop.
    emulator_loop();

    return 0;

}
