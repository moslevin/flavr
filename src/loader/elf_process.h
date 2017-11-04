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
    \file elf_process.h

    \brief Functions used to process ELF Binaries.
*/

#ifndef __ELF_PROCESS_H__
#define __ELF_PROCESS_H__

#include "elf_types.h"
#include <stdint.h>

//---------------------------------------------------------------------------
/*!
 * \brief ELF_GetHeaderStringTableOffset
 *
 * Returns an offset (in bytes) from the beginning of a buffer containing an
 * elf file, corresponding to the location of the header string table.
 *
 * \param pau8Buffer_ - Pointer to a buffer containing a loaded elf file
 * \return Offset, or 0 if no table found
 */
uint32_t ELF_GetHeaderStringTableOffset( const uint8_t *pau8Buffer_ );

//---------------------------------------------------------------------------
/*!
 * \brief ELF_GetSymbolStringTableOffset
 *
 * Returns an offset (in bytes) from the beginning of a buffer containing an
 * elf file, corresponding to the location of the symbol-string table.
 *
 * \param pau8Buffer_ - Pointer to a buffer containing a loaded elf file
 * \return Offset, or 0 if no table found
 */
uint32_t ELF_GetSymbolStringTableOffset( const uint8_t *pau8Buffer_ );

//---------------------------------------------------------------------------
/*!
 * \brief ELF_GetSymbolTableOffset
 *
 * Returns an offset (in bytes) from the beginning of a buffer containing an
 * elf file, corresponding to the location of the symbol table.
 *
 * \param pau8Buffer_ - Pointer to a buffer containing a loaded elf file
 * \return Offset, or 0 if no symbol table
 */
uint32_t ELF_GetSymbolTableOffset( const uint8_t *pau8Buffer_ );

//---------------------------------------------------------------------------
/*!
 * \brief ELF_LoadFromFile
 *
 * Read the contents of a specific ELF file from disk into a buffer, allocated
 * to a process-local RAM buffer.
 *
 * \param ppau8Buffer_ - Byte-array pointer, which will point to a newly-allocated
 *                       buffer on successful read (or NULL) on error.
 * \param szPath_      - File path to load
 * \return 0 on success, -1 on error.
 */
int ELF_LoadFromFile( uint8_t **ppau8Buffer_, const char *szPath_ );

#endif //__ELF_PROCESS_H__
