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
  \file  avr_io.h

  \brief Interface to connect I/O register updates to their corresponding
         peripheral plugins.
*/

#ifndef __AVR_IO_H__
#define __AVR_IO_H__

#include "avr_peripheral.h"

//---------------------------------------------------------------------------
typedef struct _IOReaderList
{
    struct _IOReaderList *next;
    void *pvContext;
    PeriphRead pfReader;
} IOReaderList;

//---------------------------------------------------------------------------
typedef struct _IOWriterList
{
    struct _IOWriterList *next;
    void *pvContext;
    PeriphWrite pfWriter;
} IOWriterList;

//---------------------------------------------------------------------------
typedef struct _IOClockList
{
    struct _IOClockList *next;
    void *pvContext;
    PeriphClock pfClock;
} IOClockList;

//---------------------------------------------------------------------------
/*!
 * \brief IO_AddReader
 *
 * \param pstPeriph_
 * \param addr_
 */
void IO_AddReader(  AVRPeripheral *pstPeriph_, uint8_t addr_);

//--------------------------------------------------------------------------
/*!
 * \brief IO_AddWriter
 *
 * \param pstPeriph_
 * \param addr_
 */
void IO_AddWriter(  AVRPeripheral *pstPeriph_, uint8_t addr_);

//--------------------------------------------------------------------------
/*!
 * \brief IO_AddClocker
 *
 * \param pstPeriph_
 */
void IO_AddClocker(  AVRPeripheral *pstPeriph_ );

//--------------------------------------------------------------------------
/*!
 * \brief IO_Write
 *
 * \param addr_
 * \param value_
 */
void IO_Write(  uint8_t addr_, uint8_t value_ );

//---------------------------------------------------------------------------
/*!
 * \brief IO_Read
 *
 * \param addr_
 * \param value_
 */
void IO_Read(  uint8_t addr_, uint8_t *value_ );

//---------------------------------------------------------------------------
/*!
 * \brief IO_Clock
 *
 */
void IO_Clock( void );

#endif
