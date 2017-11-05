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
  \file  variant.h

  \brief Module containing a lookup table of device variants supported by flavr.
*/

#ifndef __VARIANT_H__
#define __VARIANT_H__

#include <stdbool.h>
#include <stdint.h>

//---------------------------------------------------------------------------
#define VECTOR_NOT_SUPPORTED (0xFF) // Signal that a specific vector is not implemented on target device

//---------------------------------------------------------------------------
typedef struct {
    uint8_t RESET;
    uint8_t INT0;
    uint8_t INT1;
    uint8_t INT2;
    uint8_t PCINT0;
    uint8_t PCINT1;
    uint8_t PCINT2;
    uint8_t PCINT3;
    uint8_t WDT;
    uint8_t TIMER2_COMPA;
    uint8_t TIMER2_COMPB;
    uint8_t TIMER2_OVF;
    uint8_t TIMER1_CAPT;
    uint8_t TIMER1_COMPA;
    uint8_t TIMER1_COMPB;
    uint8_t TIMER1_OVF;
    uint8_t TIMER0_COMPA;
    uint8_t TIMER0_COMPB;
    uint8_t TIMER0_OVF;
    uint8_t SPI_STC;
    uint8_t USART0_RX;
    uint8_t USART0_UDRE;
    uint8_t USART0_TX;
    uint8_t ANALOG_COMP;
    uint8_t ADC;
    uint8_t EE_READY;
    uint8_t TWI;
    uint8_t SPM_READY;
    uint8_t USART1_RX;
    uint8_t USART1_UDRE;
    uint8_t USART1_TX;
    uint8_t TIMER3_CAPT;
    uint8_t TIMER3_COMPA;
    uint8_t TIMER3_COMPB;
    uint8_t TIMER3_OVF;
} AVR_Vector_Map_t;

//---------------------------------------------------------------------------
typedef struct {
    bool bHasTimer3;
    bool bHasUSART1;
    bool bHasInt2;
    bool bHasPCInt3;
} AVR_Feature_Map_t;

//---------------------------------------------------------------------------
/*!
    This struct contains the information necessary to effectively describe an
    AVR Microcontroller variant among the rest of the code.
*/
typedef struct
{
    const char  *szName;            //!< Name for the variant, used for identification (i.e. "atmega328p")

    uint32_t    u32RAMSize;         //!< RAM size for this variant
    uint32_t    u32ROMSize;         //!< ROM size (in bytes) for this variant
    uint32_t    u32EESize;          //!< EEPROM size of this variant

    const AVR_Feature_Map_t* pstFeatures;   //!< CPU Feature flags
    const AVR_Vector_Map_t* pstVectors;     //!< Interrupt vector mappings

} AVR_Variant_t;

//---------------------------------------------------------------------------
/*!
 * \brief Variant_GetByName
 *
 * Lookup a processor variant based on its name, and return a pointer to a
 * matching variant string on successful match.
 *
 * \param szName_ String containing a varaint name to check against
 *                (i.e. "atmega328p")
 *
 * \return Pointer to a CPU Variant struct on successful match, NULL on failure.
 */
const AVR_Variant_t *Variant_GetByName( const char *szName_ );

#endif
