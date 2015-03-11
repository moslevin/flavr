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
 * (c) Copyright 2014-15, Funkenstein Software Consulting, All rights reserved
 *     See license.txt for details
 ****************************************************************************/
/*!
    \file   kernel_aware.c

    \brief  Mark3 RTOS Kernel-Aware debugger
*/

#include "kernel_aware.h"
#include "debug_sym.h"
#include "write_callout.h"
#include "interrupt_callout.h"

#include "ka_interrupt.h"
#include "ka_profile.h"
#include "ka_thread.h"
#include "ka_trace.h"
#include "ka_graphics.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


//---------------------------------------------------------------------------
static void KA_Command( uint16_t u16Addr_, uint8_t u8Data_ )
{
    switch (u8Data_)
    {
    case KA_COMMAND_PROFILE_INIT:   KA_Command_Profile_Begin();     break;
    case KA_COMMAND_PROFILE_STOP:   KA_Command_Profile_Stop();      break;
    case KA_COMMAND_PROFILE_START:  KA_Command_Profile_Start();     break;
    case KA_COMMAND_PROFILE_REPORT: KA_Command_Profile_Report();    break;
    case KA_COMMAND_TRACE_0:
    case KA_COMMAND_TRACE_1:
    case KA_COMMAND_TRACE_2:        KA_EmitTrace(u8Data_);          break;
    case KA_COMMAND_PRINT:          KA_Print();                     break;
    default:
        break;

    }
}

//---------------------------------------------------------------------------
void KA_Graphics_Init( void )  __attribute__((weak));
void KA_Graphics_Init( void )
{

}

//---------------------------------------------------------------------------
void KernelAware_Init( void )
{
    Debug_Symbol_t *pstSymbol = 0;

    // Add a callout for profiling information (present in Mark3 Unit Tests)
    pstSymbol = Symbol_Find_Obj_By_Name( "g_ucKACommand" );
    if (pstSymbol)
    {
        // Ensure that we actually have the information we need at a valid address
        uint16_t u16CurrPtr = (uint16_t)(pstSymbol->u32StartAddr & 0x0000FFFF);
        printf( "found kernel-aware command @ %04X\n", u16CurrPtr );
        if (u16CurrPtr)
        {
            // Add a callback so that when profiling state changes, we do something.
            WriteCallout_Add( KA_Command , u16CurrPtr );
        }
    }
    else
    {
        printf( "Unable to find g_ucKACommand\n" );
    }

    KA_Interrupt_Init();
    KA_Thread_Init();
    KA_Profile_Init();
    KA_Trace_Init();
    KA_Graphics_Init();
}
