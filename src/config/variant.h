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

    const uint8_t *u8Descriptors;   //!< A bytestream composed of feature descriptors

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
