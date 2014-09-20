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
  \file  interactive.c

  \brief Interactive debugging support.
*/

#include "emu_config.h"
#include "avr_cpu.h"
#include "avr_cpu_print.h"
#include "watchpoint.h"
#include "breakpoint.h"
#include "avr_disasm.h"
#include "trace_buffer.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//---------------------------------------------------------------------------
typedef bool (*Interactive_Handler)( char *szCommand_ );

//---------------------------------------------------------------------------
typedef struct
{
    const char *szCommand;
    const char *szDescription;
    Interactive_Handler pfHandler;
} Interactive_Command_t;

//---------------------------------------------------------------------------
static bool bIsInteractive;
static bool bRetrigger;
static AVR_CPU *pstCPU = 0;
static TraceBuffer_t *pstTrace = 0;

//---------------------------------------------------------------------------
static bool Interactive_Continue( char *szCommand_ );
static bool Interactive_Step( char *szCommand_ );
static bool Interactive_Break( char *szCommand_ );
static bool Interactive_Watch( char *szCommand_ );
static bool Interactive_ROM( char *szCommand_ );
static bool Interactive_RAM( char *szCommand_ );
static bool Interactive_EE( char *szCommand_ );
static bool Interactive_Registers( char *szCommand_ );
static bool Interactive_Quit( char *szCommand_ );
static bool Interactive_Help( char *szCommand_ );
static bool Interactive_Disasm( char *szCommand_ );
static bool Interactive_Trace( char *szCommand_ );

//---------------------------------------------------------------------------
// Command-handler table
static Interactive_Command_t astCommands[] =
{
    { "registers","Dump registers to console",  Interactive_Registers },
    { "continue", "continue execution", Interactive_Continue },
    { "disasm",   "show disassembly", Interactive_Disasm },
    { "trace",    "Dump tracebuffer to console", Interactive_Trace},
    { "break",    "toggle breakpoint at address",  Interactive_Break },
    { "watch",    "toggle watchpoint at address",  Interactive_Watch },
    { "help",     "List commands", Interactive_Help },
    { "step",     "Step to next instruction", Interactive_Step },
    { "quit",     "Quit emulator", Interactive_Quit },
    { "reg",      "Dump registers to console",  Interactive_Registers },
    { "rom",      "Dump x bytes of ROM to console", Interactive_ROM },
    { "ram",      "Dump x bytes of RAM to console", Interactive_RAM },
    { "ee",       "Dump x bytes of RAM to console", Interactive_EE },
    { "b",        "toggle breakpoint at address",  Interactive_Break },
    { "c",        "continue execution", Interactive_Continue },
    { "d",        "show disassembly", Interactive_Disasm },
    { "w",        "toggle watchpoint at address",  Interactive_Watch },
    { "q",        "Quit emulator", Interactive_Quit },
    { "s",        "Step to next instruction", Interactive_Step },
    { "t",        "Dump tracebuffer to console", Interactive_Trace},
    { "h",        "List commands", Interactive_Help },
    { 0 }
};

//---------------------------------------------------------------------------
static bool Interactive_Execute_i( void )
{
    // Interactive mode - grab a line from standard input.
    char szCmdBuf[256];
    int iCmd = 0;

    printf( "> " );

    // Bail if stdin reaches EOF...
    if (0 == fgets(szCmdBuf, 255, stdin))
    {
        printf("[EOF]\n");
        exit(0);
    }

    iCmd = strlen(szCmdBuf);
    if ( iCmd <= 1 )
    {
        printf("\n");
        iCmd = 0;
    }
    else
    {
        szCmdBuf[ iCmd - 1 ] = 0;
    }

    // Compare command w/elements in the command table
    Interactive_Command_t *pstCommand = astCommands;
    bool bFound = false;
    bool bContinue = false;

    while (pstCommand->szCommand)
    {
        if ( (0 == strncmp(pstCommand->szCommand, szCmdBuf, strlen(pstCommand->szCommand)))
              && ( szCmdBuf[ strlen(pstCommand->szCommand) ] == ' ' ||
                   szCmdBuf[ strlen(pstCommand->szCommand) ] == '\0' ||
                   szCmdBuf[ strlen(pstCommand->szCommand) ] == '\n' ||
                   szCmdBuf[ strlen(pstCommand->szCommand) ] == '\r' ) )
        {

            // printf( "Found match: %s\n", pstCommand->szCommand );
            bFound = true;
            bContinue = pstCommand->pfHandler( szCmdBuf );
            break;
        }
        // Next command
        pstCommand++;
    }

    if (!bFound)
    {
        printf( "Invalid Command\n");
    }

    return bContinue;
}

//---------------------------------------------------------------------------
void Interactive_CheckAndExecute( void )
{
    // If we're in non-interactive mode (i.e. native execution), then return
    // out instantly.
    if (false == bIsInteractive)
    {
        if (false == bRetrigger)
        {
            return;
        }
        bIsInteractive = true;
        bRetrigger = false;
    }
    printf( "Debugging @ Address [0x%X]\n", pstCPU->u16PC );

    // Keep attempting to parse commands until a valid one was encountered
    while (!Interactive_Execute_i()) { /* Do Nothing */ }
}

//---------------------------------------------------------------------------
void Interactive_Set( void )
{
    bIsInteractive = true;
    bRetrigger = false;
}

//---------------------------------------------------------------------------
void Interactive_Init( AVR_CPU *pstCPU_, TraceBuffer_t *pstTrace_ )
{
    pstCPU = pstCPU_;
    pstTrace = pstTrace_;
    bIsInteractive = false;
    bRetrigger = false;
}

//---------------------------------------------------------------------------
static bool Token_ScanNext( char *szCommand_, int iStart_, int *piTokenStart_, int *piTokenLen_)
{
    int i = iStart_;

    // Parse leading whitespace
    while ( (szCommand_[i] == ' ') ||
            (szCommand_[i] == '\t') ||
            (szCommand_[i] == '\r') ||
            (szCommand_[i] == '\n')
            ) { i++; }

    // Check null termination
    if (szCommand_[i] == '\0' )
    {
        return false;
    }

    // Parse token
    *piTokenStart_ = i;
    while ( (szCommand_[i] != ' ') &&
            (szCommand_[i] != '\t') &&
            (szCommand_[i] != '\r') &&
            (szCommand_[i] != '\n') &&
            (szCommand_[i] != '\0')
            ) { i++; }
    *piTokenLen_ = (i - *piTokenStart_);

    // printf( "Start, Len: %d, %d\n", i, *piTokenLen_ );
    return true;
}

//---------------------------------------------------------------------------
static bool Token_DiscardNext( char *szCommand_, int iStart_, int *piNextTokenStart_ )
{
    int iTempStart;
    int iTempLen;
    if (!Token_ScanNext(szCommand_, iStart_, &iTempStart, &iTempLen ))
    {
        return false;
    }
    *piNextTokenStart_ = iTempStart + iTempLen + 1;
    return true;
}

//---------------------------------------------------------------------------
static bool Token_ReadNextHex( char *szCommand_, int iStart_, int *piNextTokenStart_, unsigned int *puiVal_ )
{
    int iTempStart = iStart_;
    int iTempLen;

    if (!Token_ScanNext(szCommand_, iStart_, &iTempStart, &iTempLen ))
    {
        return false;
    }

    szCommand_[iTempStart + iTempLen] = 0;

    if (0 == sscanf( &szCommand_[iTempStart], "%x", puiVal_ ))
    {
        if (0 == sscanf( &szCommand_[iTempStart], "x%x", puiVal_ ))
        {
            if (0 == sscanf( &szCommand_[iTempStart], "0x%x", puiVal_ ))
            {
                printf( "Missing Argument\n" );
                return false;
            }
        }
    }

    *piNextTokenStart_ = iTempStart + iTempLen + 1;
    return true;
}

//---------------------------------------------------------------------------
static bool Interactive_Continue( char *szCommand_ )
{
    bIsInteractive = false;
    bRetrigger = false;
    return true;
}

//---------------------------------------------------------------------------
static bool Interactive_Break( char *szCommand_ )
{
    unsigned int uiAddr;
    int iTokenStart;

    if (!Token_DiscardNext( szCommand_, 0, &iTokenStart))
    {
        return false;
    }

    if (!Token_ReadNextHex( szCommand_, iTokenStart, &iTokenStart, &uiAddr))
    {
        return false;
    }

    if (BreakPoint_EnabledAtAddress( pstCPU, (uint16_t)uiAddr))
    {
        BreakPoint_Delete( pstCPU, (uint16_t)uiAddr);
    }
    else
    {
        BreakPoint_Insert( pstCPU, (uint16_t)uiAddr);
    }

    return false;
}

//---------------------------------------------------------------------------
static bool Interactive_Watch( char *szCommand_ )
{
    unsigned int uiAddr;
    int iTokenStart;

    if (!Token_DiscardNext( szCommand_, 0, &iTokenStart))
    {
        return false;
    }

    if (!Token_ReadNextHex( szCommand_, iTokenStart, &iTokenStart, &uiAddr))
    {
        return false;
    }

    if (WatchPoint_EnabledAtAddress(pstCPU, (uint16_t)uiAddr))
    {
        WatchPoint_Delete( pstCPU, (uint16_t)uiAddr);
    }
    else
    {
        WatchPoint_Insert( pstCPU, (uint16_t)uiAddr);
    }
    return false;
}
//---------------------------------------------------------------------------
static bool Interactive_ROM( char *szCommand_ )
{
    unsigned int uiAddr;
    unsigned int uiLen;
    int iTokenStart;

    if (!Token_DiscardNext( szCommand_, 0, &iTokenStart))
    {
        return false;
    }

    if (!Token_ReadNextHex( szCommand_, iTokenStart, &iTokenStart, &uiAddr))
    {
        return false;
    }

    if (!Token_ReadNextHex( szCommand_, iTokenStart, &iTokenStart, &uiLen))
    {
        return false;
    }

    print_rom( pstCPU, (uint16_t)uiAddr, (uint16_t)uiLen );

    return false;
}

//---------------------------------------------------------------------------
static bool Interactive_RAM( char *szCommand_ )
{
    unsigned int uiAddr;
    unsigned int uiLen;
    int iTokenStart;

    if (!Token_DiscardNext( szCommand_, 0, &iTokenStart))
    {        
        return false;
    }

    if (!Token_ReadNextHex( szCommand_, iTokenStart, &iTokenStart, &uiAddr))
    {
        return false;
    }

    if (!Token_ReadNextHex( szCommand_, iTokenStart, &iTokenStart, &uiLen))
    {
        return false;
    }

    print_ram( pstCPU, (uint16_t)uiAddr, (uint16_t)uiLen );

    return false;
}

//---------------------------------------------------------------------------
static bool Interactive_EE( char *szCommand_ )
{
    unsigned int uiAddr;
    unsigned int uiLen;
    int iTokenStart;

    if (!Token_DiscardNext( szCommand_, 0, &iTokenStart))
    {
        return false;
    }

    if (!Token_ReadNextHex( szCommand_, iTokenStart, &iTokenStart, &uiAddr))
    {
        return false;
    }

    if (!Token_ReadNextHex( szCommand_, iTokenStart, &iTokenStart, &uiLen))
    {
        return false;
    }

    printf( "Dump EEPROM [%x:%x]\n", uiAddr, uiLen );

    return false;
}

//---------------------------------------------------------------------------
static bool Interactive_Registers( char *szCommand_ )
{
    print_core_regs( pstCPU );
    return false;
}

//---------------------------------------------------------------------------
static bool Interactive_Quit( char *szCommand_ )
{
    exit(0);
}

//---------------------------------------------------------------------------
static bool Interactive_Step( char *szCommand_ )
{
    bRetrigger = true; // retrigger debugging on next loop
    return true;
}

//---------------------------------------------------------------------------
static bool Interactive_Help( char *szCommand_ )
{
    Interactive_Command_t *pstCommand_ = astCommands;
    printf( "FLAVR interactive debugger commands:\n");
    while (pstCommand_->szCommand)
    {
        printf( "    %s: %s\n", pstCommand_->szCommand, pstCommand_->szDescription );
        pstCommand_++;
    }
    return false;
}

//---------------------------------------------------------------------------
static bool Interactive_Disasm( char *szCommand_ )
{
    uint16_t OP = pstCPU->pusROM[pstCPU->u16PC];
    printf("0x%04X: [0x%04X] ", pstCPU->u16PC, OP);
    AVR_Decode(pstCPU, OP);
    AVR_Disasm_Function(OP)(pstCPU);
    return false;
}

//---------------------------------------------------------------------------
static bool Interactive_Trace( char *szCommand_ )
{
    TraceBuffer_Print( pstTrace, TRACE_PRINT_COMPACT | TRACE_PRINT_DISASSEMBLY, pstCPU );
    return false;
}
