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

#ifndef __ELF_PROCESS_H__
#define __ELF_PROCESS_H__

#include "elf_types.h"
#include <stdint.h>

//---------------------------------------------------------------------------
/*!
 * \brief ELF_GetHeaderStringTableOffset
 * \param pau8Buffer_
 * \return
 */
uint32_t ELF_GetHeaderStringTableOffset( const uint8_t *pau8Buffer_ );

//---------------------------------------------------------------------------
/*!
 * \brief ELF_GetSymbolStringTableOffset
 * \param pau8Buffer_
 * \return
 */
uint32_t ELF_GetSymbolStringTableOffset( const uint8_t *pau8Buffer_ );

//---------------------------------------------------------------------------
/*!
 * \brief ELF_GetSymbolTableOffset
 * \param pau8Buffer_
 * \return
 */
uint32_t ELF_GetSymbolTableOffset( const uint8_t *pau8Buffer_ );

//---------------------------------------------------------------------------
/*!
 * \brief ELF_LoadFromFile
 * \param ppau8Buffer_
 * \param szPath_
 * \return
 */
int ELF_LoadFromFile( uint8_t **ppau8Buffer_, const char *szPath_ );

#endif //__ELF_PROCESS_H__
