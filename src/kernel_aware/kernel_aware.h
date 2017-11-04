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
    \file   kernel_aware.h

    \brief  Kernel-Aware debugger plugin interface.
*/

#ifndef __KERNEL_AWARE_H__
#define __KERNEL_AWARE_H__

#include "elf_process.h"
#include "debug_sym.h"
#include "avr_cpu.h"

#include <stdint.h>

//---------------------------------------------------------------------------
typedef enum
{
    KA_COMMAND_IDLE = 0,
    KA_COMMAND_PROFILE_INIT,
    KA_COMMAND_PROFILE_START,
    KA_COMMAND_PROFILE_STOP,
    KA_COMMAND_PROFILE_REPORT,
    KA_COMMAND_EXIT_SIMULATOR,
    KA_COMMAND_TRACE_0,
    KA_COMMAND_TRACE_1,
    KA_COMMAND_TRACE_2,
    KA_COMMAND_PRINT
} KernelAwareCommand_t;

//---------------------------------------------------------------------------
/*!
 * \brief KernelAware_Init
 *
 * Initialize special RTOS kernel-aware debugger functionality when selected.
 * Currently this is tied to Mark3 RTOS (see kernel_aware.c implementation),
 * but can be abstracted using this simple interface to any other RTOS kernel
 * or environment (but why would you -- Mark3 is awesome!).
 *
 */
void KernelAware_Init(void);

void KA_Graphics_Init( void )  __attribute__((weak));
void KA_Joystick_Init( void )  __attribute__((weak));
#endif
