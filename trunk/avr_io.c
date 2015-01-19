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
  \file  avr_io.c

  \brief Interface to connect I/O register updates to their corresponding
         peripheral plugins.
*/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "emu_config.h"

#include "avr_peripheral.h"
#include "avr_cpu.h"
#include "avr_io.h"

//---------------------------------------------------------------------------
void IO_AddReader(  AVRPeripheral *pstPeriph_, uint8_t addr_)
{
    IOReaderList *node = NULL;

    node = (IOReaderList*)malloc(sizeof(*node));
    if (!node)
    {
        return;
    }

    node->next = stCPU.apstPeriphReadTable[addr_];
    node->pfReader = pstPeriph_->pfRead;
    node->pvContext = pstPeriph_->pvContext;

    stCPU.apstPeriphReadTable[addr_] = node;    
}

//---------------------------------------------------------------------------
void IO_AddWriter(  AVRPeripheral *pstPeriph_, uint8_t addr_)
{
    IOWriterList *node = NULL;

    node = (IOWriterList*)malloc(sizeof(*node));
    if (!node)
    {
        return;
    }

    node->next = stCPU.apstPeriphWriteTable[addr_];
    node->pfWriter = pstPeriph_->pfWrite;
    node->pvContext = pstPeriph_->pvContext;

    stCPU.apstPeriphWriteTable[addr_] = node;
}

//---------------------------------------------------------------------------
void IO_AddClocker(  AVRPeripheral *pstPeriph_ )
{
    IOClockList *node = NULL;

    node = (IOClockList*)malloc(sizeof(*node));
    if (!node)
    {
        return;
    }

    node->next = stCPU.pstClockList;
    node->pfClock = pstPeriph_->pfClock;
    node->pvContext = pstPeriph_->pvContext;

    stCPU.pstClockList = node;
}

//---------------------------------------------------------------------------
void IO_Write(  uint8_t addr_, uint8_t value_ )
{
    IOWriterList *node = stCPU.apstPeriphWriteTable[addr_];
    while (node)
    {
        if (node->pfWriter)
        {
            node->pfWriter( node->pvContext, addr_, value_ );
        }
        node = node->next;
    }
}

//---------------------------------------------------------------------------
void IO_Read(  uint8_t addr_, uint8_t *value_ )
{
    IOReaderList *node = stCPU.apstPeriphReadTable[addr_];
    while (node)
    {
        if (node->pfReader)
        {
            node->pfReader( node->pvContext, addr_, value_ );
        }
        node = node->next;
    }
}

//---------------------------------------------------------------------------
void IO_Clock( void )
{
    IOClockList *node = stCPU.pstClockList;
    while (node)
    {
        if (node->pfClock)
        {
            node->pfClock( node->pvContext );
        }
        node = node->next;
    }
}
