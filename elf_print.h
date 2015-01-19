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

#ifndef __ELF_PRINT_H__
#define __ELF_PRINT_H__

#include "elf_types.h"
#include <stdint.h>

//---------------------------------------------------------------------------
/*!
 * \brief ELF_PrintHeader
 * \param pau8Buffer_
 */
void ELF_PrintHeader( const uint8_t *pau8Buffer_ );

//---------------------------------------------------------------------------
/*!
 * \brief ELF_PrintSections
 * \param pau8Buffer_
 */
void ELF_PrintSections( const uint8_t *pau8Buffer_ );

//---------------------------------------------------------------------------
/*!
 * \brief ELF_PrintSymbols
 * \param pau8Buffer_
 */
void ELF_PrintSymbols( const uint8_t *pau8Buffer_ );

//---------------------------------------------------------------------------
/*!
 * \brief ELF_PrintProgramHeaders
 * \param pau8Buffer_
 */
void ELF_PrintProgramHeaders( const uint8_t *pau8Buffer_ );

#endif //__ELF_PRINT_H__
