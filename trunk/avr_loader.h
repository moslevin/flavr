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
  \file  avr_loader.h

  \brief Functions to load intel-formatted programming files into a virtual AVR.
*/

#ifndef __AVR_LOADER_H__
#define __AVR_LOADER_H__

#include <stdint.h>
#include "avr_cpu.h"

//---------------------------------------------------------------------------
/*!
 * \brief AVR_Load_HEX
 * \param pstCPU_
 * \param szFilePath_
 * \return
 */
bool AVR_Load_HEX( AVR_CPU *pstCPU_, const char *szFilePath_);

#endif
