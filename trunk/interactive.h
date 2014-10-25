/****************************************************************************
 *     (     (                      (     |
 *    )\ )  )\ )    (              )\ )   |
 *   (()/( (()/(    )\     (   (  (()/(   | -- [ Funkenstein ] -------------
 *    /(_)) /(_))((((_)()\  )\  /(_))  | -- [ Litle ] -------------------
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
  \file  interactive.h

  \brief Interactive debugging support.
*/

#ifndef __INTERACTIVE_H__
#define __INTERACTIVE_H__

#include "emu_config.h"
#include "avr_cpu.h"
#include "trace_buffer.h"

//---------------------------------------------------------------------------
/*!
 * \brief Interactive_CheckAndExecute
 *
 * Wait for feedback and execute if running interactive. Otherwise, continue
 * execution without waiting.
 */
void Interactive_CheckAndExecute( void );

//---------------------------------------------------------------------------
/*!
 * \brief Interactive_Set
 *
 * Enable interactive-debug mode
 */
void Interactive_Set( void );

//---------------------------------------------------------------------------
/*!
 * \brief Interactive_Init
 *
 * Initialize the interactive debugger session for the given CPU struct and
 * associated debug data
 *
 * \param pstCPU_ Pointer to the CPU object to debug interactively
 * \param pstTrace_ Pointer to the tracebuffer object
 */
void Interactive_Init( TraceBuffer_t *pstTrace_);

#endif
