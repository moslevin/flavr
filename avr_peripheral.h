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
  \file  avr_peripheral.h

  \brief Interfaces for creating AVR peripheral plugins
*/

#ifndef __AVR_PERIPHERAL_H__
#define __AVR_PERIPHERAL_H__

#include <stdint.h>

struct _AVR_CPU;
//---------------------------------------------------------------------------
// Peripheral callout functions - used to implement arbitrary peripherals
// which are able to intercept/react to read/write operations to specific
// I/O addresses.
//---------------------------------------------------------------------------

typedef void (*PeriphInit) (void *context_, struct _AVR_CPU *pstCPU_);
typedef void (*PeriphRead) (void *context_, struct _AVR_CPU *pstCPU_, uint8_t ucAddr_, uint8_t *pucValue_ );
typedef void (*PeriphWrite)(void *context_, struct _AVR_CPU *pstCPU_, uint8_t ucAddr_, uint8_t ucValue_ );
typedef void (*PeriphClock)(void *context_, struct _AVR_CPU *pstCPU_);

//---------------------------------------------------------------------------
typedef struct _AVRPeripheral
{
    PeriphInit  pfInit;
    PeriphRead  pfRead;
    PeriphWrite pfWrite;
    PeriphClock pfClock;

    void        *pvContext;

    uint8_t     u8AddrStart;
    uint8_t     u8AddrEnd;
} AVRPeripheral;

#endif //__AVR_PERIPHERAL_H__
