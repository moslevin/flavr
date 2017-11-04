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
    \file   ka_trace.h

    \brief  Mark3 RTOS Kernel-Aware Trace and Print Functionality
*/

#ifndef __KA_TRACE__
#define __KA_TRACE__


#include "kernel_aware.h"
#include "debug_sym.h"

#include "ka_trace.h"
#include "tlv_file.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

//---------------------------------------------------------------------------
/*!
 * \brief KA_EmitTrace
 *
 * Process a kernel trace event and emit the appropriate record into our TLV
 * stream output
 *
 * \param eCmd_ Type of trace command being emitted.
 */
void KA_EmitTrace( KernelAwareCommand_t eCmd_ );

//---------------------------------------------------------------------------
/*!
 * \brief KA_Print
 *
 * Print a kernel string event to the console and TLV stream.
 */
void KA_Print( void );

//---------------------------------------------------------------------------
/*!
 * \brief KA_Trace_Init
 *
 * Initialize the local TLV buffers, etc. Must be called prior to use
 */
void KA_Trace_Init( void );

#endif

