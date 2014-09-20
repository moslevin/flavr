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
  \file  avr_cpu_print.h

  \brief Helper module used to print the contents of a virtual AVR's internal
         registers and memory.
*/


#ifndef __AVR_CPU_PRINT_H__
#define __AVR_CPU_PRINT_H__

#include "avr_cpu.h"

//---------------------------------------------------------------------------
/*!
 * \brief print_core_regs
 * \param pstCPU_
 */
void print_core_regs( AVR_CPU *pstCPU_ );

//---------------------------------------------------------------------------
/*!
 * \brief print_io_reg
 * \param pstCPU_
 * \param u8Addr_
 */
void print_io_reg( AVR_CPU *pstCPU_, uint8_t u8Addr_ );

//---------------------------------------------------------------------------
/*!
 * \brief print_io_reg_with_name
 * \param pstCPU_
 * \param u8Addr_
 * \param szName_
 */
void print_io_reg_with_name( AVR_CPU *pstCPU_, uint8_t u8Addr_, const char *szName_ );

//---------------------------------------------------------------------------
/*!
 * \brief print_ram
 * \param pstCPU_
 * \param u16Start_
 * \param u16Span_
 */
void print_ram( AVR_CPU *pstCPU_, uint16_t u16Start_, uint16_t u16Span_ );

//---------------------------------------------------------------------------
/*!
 * \brief print_rom
 * \param pstCPU_
 * \param u16Start_
 * \param u16Span_
 */
void print_rom( AVR_CPU *pstCPU_, uint16_t u16Start_, uint16_t u16Span_ );

#endif
