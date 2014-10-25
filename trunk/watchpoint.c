
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
 * (c) Copyright 2014, Funkenstein Software Consulting, All rights reserved
 *     See license.txt for details
 ****************************************************************************/
/*!
  \file  watchpoint.c

  \brief Implements data watchpoints for debugging based on memory accesses.
*/

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "watchpoint.h"

//---------------------------------------------------------------------------
void WatchPoint_Insert( uint16_t u16Addr_ )
{
    // Don't add multiple watchpoints at the same address
    if (WatchPoint_EnabledAtAddress( u16Addr_ ))
    {
        return;
    }

    WatchPoint_t *pstNewWatch = NULL;

    pstNewWatch = (WatchPoint_t*)malloc( sizeof(WatchPoint_t) );

    pstNewWatch->next = stCPU.pstWatchPoints;
    pstNewWatch->prev = NULL;

    pstNewWatch->u16Addr = u16Addr_;

    if (stCPU.pstWatchPoints)
    {
        WatchPoint_t *pstTemp = stCPU.pstWatchPoints;
        pstTemp->prev = pstNewWatch;
    }
    stCPU.pstWatchPoints = pstNewWatch;
}

//---------------------------------------------------------------------------
void WatchPoint_Delete( uint16_t u16Addr_ )
{
    WatchPoint_t *pstTemp = stCPU.pstWatchPoints;

    while (pstTemp)
    {
        if (pstTemp->u16Addr == u16Addr_)
        {
            // Remove node -- reconnect surrounding elements
            WatchPoint_t *pstNext = pstTemp->next;
            if (pstNext)
            {
                pstNext->prev = pstTemp->prev;
            }

            WatchPoint_t *pstPrev = pstTemp->prev;
            if (pstPrev)
            {
                pstPrev->next = pstTemp->next;
            }

            // Adjust list-head if necessary
            if (pstTemp == stCPU.pstWatchPoints)
            {
                stCPU.pstWatchPoints = pstNext;
            }

            // Free the node/iterate to next node.
            pstPrev = pstTemp;
            pstTemp = pstTemp->next;
            free(pstPrev);
        }
        else
        {
            pstTemp = pstTemp->next;
        }
    }
}

//---------------------------------------------------------------------------
bool WatchPoint_EnabledAtAddress( uint16_t u16Addr_ )
{
    WatchPoint_t *pstTemp = stCPU.pstWatchPoints;

    while (pstTemp)
    {
        if (pstTemp->u16Addr == u16Addr_)
        {
            return true;
        }
        pstTemp = pstTemp->next;
    }
    return false;
}
