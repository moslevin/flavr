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
  \file  avr_loader.h

  \brief Functions to load intel hex or elf binaries into a virtual AVR.
*/

#ifndef __AVR_LOADER_H__
#define __AVR_LOADER_H__

#include <stdint.h>
#include "avr_cpu.h"

//---------------------------------------------------------------------------
/*!
 * \brief AVR_Load_HEX Load a hex file, specified by path, into the flash
 *        memory of the CPU object
 *
 * \param szFilePath_ Pointer to the hexfile path
 * \return true if the hex file load operation succeeded, false otherwise
 */
bool AVR_Load_HEX( const char *szFilePath_);

//---------------------------------------------------------------------------
/*!
 * \brief AVR_Load_ELF Load an elf file, specified by path, into the flash
 *        memory of the CPU object.  Will also pre-seed RAM according to the
 *        contents of the ELF, if found.
 *
 * \param szFilePath_ Pointer to the elf-file path
 * \return true if the elf file load operation succes
 */
bool AVR_Load_ELF( const char *szFilePath_);
#endif
