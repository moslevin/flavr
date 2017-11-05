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
  \file  variant.c

  \brief Module containing a table of device variants supported by flavr.
*/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "variant.h"

//---------------------------------------------------------------------------
#define KB  * (1024)

//---------------------------------------------------------------------------
// This vector table works for :
// atMega48pa, 88pa, 168p, 328p
static const AVR_Vector_Map_t stSmallAtMegaVectors = {
    .RESET = 0x00,
    .INT0 = 0x01,
    .INT1 = 0x02,
    .PCINT0 = 0x03,
    .PCINT1 = 0x04,
    .PCINT2 = 0x05,
    .WDT = 0x06,
    .TIMER2_COMPA = 0x07,
    .TIMER2_COMPB = 0x08,
    .TIMER2_OVF = 0x09,
    .TIMER1_CAPT = 0x0A,
    .TIMER1_COMPA = 0x0B,
    .TIMER1_COMPB = 0x0C,
    .TIMER1_OVF = 0x0D,
    .TIMER0_COMPA = 0x0E,
    .TIMER0_COMPB = 0x0F,
    .TIMER0_OVF = 0x10,
    .SPI_STC = 0x11,
    .USART0_RX = 0x12,
    .USART0_UDRE = 0x13,
    .USART0_TX = 0x14,
    .ADC = 0x15,
    .EE_READY = 0x16,
    .ANALOG_COMP = 0x17,
    .TWI = 0x18,
    .SPM_READY = 0x19,
    .INT2 = VECTOR_NOT_SUPPORTED,
    .PCINT3 = VECTOR_NOT_SUPPORTED,
    .USART1_RX = VECTOR_NOT_SUPPORTED,
    .USART1_UDRE = VECTOR_NOT_SUPPORTED,
    .USART1_TX = VECTOR_NOT_SUPPORTED,
    .TIMER3_CAPT = VECTOR_NOT_SUPPORTED,
    .TIMER3_COMPA = VECTOR_NOT_SUPPORTED,
    .TIMER3_COMPB = VECTOR_NOT_SUPPORTED,
    .TIMER3_OVF= VECTOR_NOT_SUPPORTED
};

//---------------------------------------------------------------------------
// This vector table works for :
// atMega644p
static const AVR_Vector_Map_t stMediumAtMegaVectors = {
    .RESET = 0x00,
    .INT0 = 0x01,
    .INT1 = 0x02,
    .INT2 = 0x03,
    .PCINT0 = 0x04,
    .PCINT1 = 0x05,
    .PCINT2 = 0x06,
    .PCINT3 = 0x07,
    .WDT = 0x08,
    .TIMER2_COMPA = 0x09,
    .TIMER2_COMPB = 0x0A,
    .TIMER2_OVF = 0x0B,
    .TIMER1_CAPT = 0x0C,
    .TIMER1_COMPA = 0x0D,
    .TIMER1_COMPB = 0x0E,
    .TIMER1_OVF = 0x0F,
    .TIMER0_COMPA = 0x10,
    .TIMER0_COMPB = 0x11,
    .TIMER0_OVF = 0x12,
    .SPI_STC = 0x13,
    .USART0_RX = 0x14,
    .USART0_UDRE = 0x15,
    .USART0_TX = 0x16,
    .ANALOG_COMP = 0x17,
    .ADC = 0x18,
    .EE_READY = 0x19,
    .TWI = 0x1A,
    .SPM_READY = 0x1B,
    .USART1_RX = VECTOR_NOT_SUPPORTED,
    .USART1_UDRE = VECTOR_NOT_SUPPORTED,
    .USART1_TX = VECTOR_NOT_SUPPORTED,
    .TIMER3_CAPT = VECTOR_NOT_SUPPORTED,
    .TIMER3_COMPA = VECTOR_NOT_SUPPORTED,
    .TIMER3_COMPB = VECTOR_NOT_SUPPORTED,
    .TIMER3_OVF = VECTOR_NOT_SUPPORTED
};

//---------------------------------------------------------------------------
// Map for atMega1284p
static const AVR_Vector_Map_t stLargeAtMegaVectors = {
    .RESET = 0x00,
    .INT0 = 0x01,
    .INT1 = 0x02,
    .INT2 = 0x03,
    .PCINT0 = 0x04,
    .PCINT1 = 0x05,
    .PCINT2 = 0x06,
    .PCINT3 = 0x07,
    .WDT = 0x08,
    .TIMER2_COMPA = 0x09,
    .TIMER2_COMPB = 0x0A,
    .TIMER2_OVF = 0x0B,
    .TIMER1_CAPT = 0x0C,
    .TIMER1_COMPA = 0x0D,
    .TIMER1_COMPB = 0x0E,
    .TIMER1_OVF = 0x0F,
    .TIMER0_COMPA = 0x10,
    .TIMER0_COMPB = 0x11,
    .TIMER0_OVF = 0x12,
    .SPI_STC = 0x13,
    .USART0_RX = 0x14,
    .USART0_UDRE = 0x15,
    .USART0_TX = 0x16,
    .ANALOG_COMP = 0x17,
    .ADC = 0x18,
    .EE_READY = 0x19,
    .TWI = 0x1A,
    .SPM_READY = 0x1B,
    .USART1_RX = 0x1C,
    .USART1_UDRE = 0x1D,
    .USART1_TX = 0x1E,
    .TIMER3_CAPT = 0x1F,
    .TIMER3_COMPA = 0x20,
    .TIMER3_COMPB = 0x21,
    .TIMER3_OVF = 0x22
};

//---------------------------------------------------------------------------
static AVR_Feature_Map_t stSmallAtMegaFeatures = {
    .bHasTimer3 = false,
    .bHasUSART1 = false,
    .bHasInt2 = false,
    .bHasPCInt3 = false
};

//---------------------------------------------------------------------------
static AVR_Feature_Map_t stMediumAtMegaFeatures = {
    .bHasTimer3 = false,
    .bHasUSART1 = false,
    .bHasInt2 = true,
    .bHasPCInt3 = true
};

//---------------------------------------------------------------------------
static AVR_Feature_Map_t stLargeAtMegaFeatures = {
    .bHasTimer3 = true,
    .bHasUSART1 = true,
    .bHasInt2 = true,
    .bHasPCInt3 = true
};

//---------------------------------------------------------------------------
static AVR_Variant_t astVariants[] =
{
    { "atmega1284p", 16 KB,  128 KB, 4 KB, &stLargeAtMegaFeatures, &stLargeAtMegaVectors },
    { "atmega1284",  16 KB,  128 KB, 4 KB, &stLargeAtMegaFeatures, &stLargeAtMegaVectors },
    { "atmega644p",  4 KB,   64 KB,  2 KB, &stMediumAtMegaFeatures, &stMediumAtMegaVectors },
    { "atmega644",   4 KB,   64 KB,  2 KB, &stMediumAtMegaFeatures, &stMediumAtMegaVectors },
    { "atmega328p",  2 KB,   32 KB,  1 KB, &stSmallAtMegaFeatures, &stSmallAtMegaVectors },
    { "atmega328",   2 KB,   32 KB,  1 KB, &stSmallAtMegaFeatures, &stSmallAtMegaVectors },
    { "atmega168pa", 1 KB,   16 KB,  0.5 KB, &stSmallAtMegaFeatures, &stSmallAtMegaVectors },
    { "atmega168",   1 KB,   16 KB,  0.5 KB, &stSmallAtMegaFeatures, &stSmallAtMegaVectors },
    { "atmega88pa",  1 KB,   8 KB,   0.5 KB, &stSmallAtMegaFeatures, &stSmallAtMegaVectors },
    { "atmega88",    1 KB,   8 KB,   0.5 KB, &stSmallAtMegaFeatures, &stSmallAtMegaVectors },
    { "atmega48pa",  0.5 KB, 4 KB,   0.25 KB, &stSmallAtMegaFeatures, &stSmallAtMegaVectors },
    { "atmega48",    0.5 KB, 4 KB,   0.25 KB,  &stSmallAtMegaFeatures, &stSmallAtMegaVectors },
    { 0 }
};

//---------------------------------------------------------------------------
const AVR_Variant_t *Variant_GetByName( const char *szName_ )
{
    AVR_Variant_t *pstVariant = astVariants;
    while (pstVariant->szName)
    {
        if (0 == strcmp(pstVariant->szName, szName_ ) )
        {
            return pstVariant;
        }
        pstVariant++;
    }
    return NULL;
}

