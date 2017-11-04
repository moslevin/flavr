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
    \file   ka_thread.h

    \brief  Mark3 RTOS Kernel-Aware Thread Profiling
*/

#ifndef __KA_THREAD_H__
#define __KA_THREAD_H__

#include <stdint.h>

typedef struct
{
    uint8_t SPH;
    uint8_t SPL;
    uint8_t r[32];
    uint8_t SREG;
    uint16_t PC;
} Mark3_Context_t;

//---------------------------------------------------------------------------
void KA_Thread_Init( void );

int KA_Get_Thread_Priority(int id_);

const char *KA_Get_Thread_State( int id_ );

#endif
