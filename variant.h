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
  \file  variant.h

  \brief Module containing a lookup table of device variants supported by flavr.
*/

#ifndef __VARIANT_H__
#define __VARIANT_H__
//---------------------------------------------------------------------------
#define ADD_CAPABILITY                  0xFE, 0xEF

#define IO_REGISTER_RANGE               0xFD, 0xDF

#define AVR_HAS_RAMP_Z                  0x07
#define AVR_HAS_EIND                    0x08

#define AVR_HAS_UART0                   0x09
#define AVR_HAS_UART1                   0x0A

#define AVR_HAS_TIMER0_8BIT             0x0B
#define AVR_HAS_TIMER0_16BIT            0x0C

#define AVR_HAS_TIMER1_8BIT             0x0D
#define AVR_HAS_TIMER1_16BIT            0x0E

#define AVR_HAS_TIMER2_8BIT             0x0F
#define AVR_HAS_TIMER2_16BIT            0x10

//---------------------------------------------------------------------------
typedef struct
{
    const char  *szName;            // Variant name for the part

    uint32_t    u32RAMSize;
    uint32_t    u32ROMSize;
    uint32_t    u32EESize;

    const uint8_t *u8Descriptors;   // CPU Feature descriptor table

} AVR_Variant_t;

//---------------------------------------------------------------------------
const AVR_Variant_t *Variant_GetByName( const char *szName_ );

#endif
