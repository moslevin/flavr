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
/*!
    \file   elf_print.h

    \brief  Functions to print information from ELF files
*/

#ifndef __ELF_PRINT_H__
#define __ELF_PRINT_H__

#include "elf_types.h"
#include <stdint.h>

//---------------------------------------------------------------------------
/*!
 * \brief ELF_PrintHeader
 *
 * Print the contents of a loaded ELF file's header data to standard output.
 *
 * \param pau8Buffer_ Buffer containing the loaded ELF contents
 */
void ELF_PrintHeader( const uint8_t *pau8Buffer_ );

//---------------------------------------------------------------------------
/*!
 * \brief ELF_PrintSections
 *
 * Print a list of named sections contained in the loaded ELF file.
 *
 * \param pau8Buffer_ Buffer containing the loaded ELF contents
 */
void ELF_PrintSections( const uint8_t *pau8Buffer_ );

//---------------------------------------------------------------------------
/*!
 * \brief ELF_PrintSymbols
 *
 * Print a list of ELF Symbols contained in the loaded ELF file.
 *
 * \param pau8Buffer_ Buffer containing the loaded ELF contents
 */
void ELF_PrintSymbols( const uint8_t *pau8Buffer_ );

//---------------------------------------------------------------------------
/*!
 * \brief ELF_PrintProgramHeaders
 *
 * Print the list of program headers stored in the loaded ELF file .
 *
 * \param pau8Buffer_ Buffer containing the loaded ELF contents
 */
void ELF_PrintProgramHeaders( const uint8_t *pau8Buffer_ );

#endif //__ELF_PRINT_H__
