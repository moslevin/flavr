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
void IO_AddReader(struct _AVR_CPU *pstCPU_, AVRPeripheral *pstPeriph_, uint8_t addr_)
{
    IOReaderList *node = NULL;

    node = (IOReaderList*)malloc(sizeof(*node));
    if (!node)
    {
        return;
    }

    node->next = pstCPU_->apstPeriphReadTable[addr_];
    node->pfReader = pstPeriph_->pfRead;
    node->pvContext = pstPeriph_->pvContext;

    pstCPU_->apstPeriphReadTable[addr_] = node;

    printf( "Peripheral reader added @ addr 0x%04X\n", addr_ );
}

//---------------------------------------------------------------------------
void IO_AddWriter(struct _AVR_CPU *pstCPU_, AVRPeripheral *pstPeriph_, uint8_t addr_)
{
    IOWriterList *node = NULL;

    node = (IOWriterList*)malloc(sizeof(*node));
    if (!node)
    {
        return;
    }

    node->next = pstCPU_->apstPeriphWriteTable[addr_];
    node->pfWriter = pstPeriph_->pfWrite;
    node->pvContext = pstPeriph_->pvContext;

    pstCPU_->apstPeriphWriteTable[addr_] = node;

    printf( "Peripheral writer added @ addr 0x%04X\n", addr_ );
}

//---------------------------------------------------------------------------
void IO_AddClocker(struct _AVR_CPU *pstCPU_, AVRPeripheral *pstPeriph_ )
{
    IOClockList *node = NULL;

    node = (IOClockList*)malloc(sizeof(*node));
    if (!node)
    {
        return;
    }

    node->next = pstCPU_->pstClockList;
    node->pfClock = pstPeriph_->pfClock;
    node->pvContext = pstPeriph_->pvContext;

    pstCPU_->pstClockList = node;
}

//---------------------------------------------------------------------------
void IO_Write(struct _AVR_CPU *pstCPU_, uint8_t addr_, uint8_t value_ )
{
    IOWriterList *node = pstCPU_->apstPeriphWriteTable[addr_];
    while (node)
    {
        node->pfWriter( node->pvContext, pstCPU_, addr_, value_ );
        node = node->next;
    }
}

//---------------------------------------------------------------------------
void IO_Read(struct _AVR_CPU *pstCPU_, uint8_t addr_, uint8_t *value_ )
{
    IOReaderList *node = pstCPU_->apstPeriphReadTable[addr_];
    while (node)
    {
        node->pfReader( node->pvContext, pstCPU_, addr_, value_ );
        node = node->next;
    }
}

//---------------------------------------------------------------------------
void IO_Clock( struct _AVR_CPU *pstCPU_ )
{
    IOClockList *node = pstCPU_->pstClockList;
    while (node)
    {
        node->pfClock( node->pvContext, pstCPU_ );
        node = node->next;
    }
}
