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
  \file  breakpoint.c

  \brief Implements instruction breakpoints for debugging based on code path
*/

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "breakpoint.h"

//---------------------------------------------------------------------------
void BreakPoint_Insert( uint32_t u32Addr_ )
{
    // Don't add multiple breakpoints at the same address
    if (BreakPoint_EnabledAtAddress( u32Addr_ ))
    {
        return;
    }

    BreakPoint_t *pstNewBreak = NULL;

    pstNewBreak = (BreakPoint_t*)malloc( sizeof(BreakPoint_t) );

    pstNewBreak->next = stCPU.pstBreakPoints;
    pstNewBreak->prev = NULL;

    pstNewBreak->u32Addr = u32Addr_;

    if (stCPU.pstBreakPoints)
    {
        BreakPoint_t *pstTemp = stCPU.pstBreakPoints;
        pstTemp->prev = pstNewBreak;
    }
    stCPU.pstBreakPoints = pstNewBreak;
}

//---------------------------------------------------------------------------
void BreakPoint_Delete( uint32_t u32Addr_ )
{
    BreakPoint_t *pstTemp = stCPU.pstBreakPoints;

    while (pstTemp)
    {
        if (pstTemp->u32Addr == u32Addr_)
        {
            // Remove node -- reconnect surrounding elements
            BreakPoint_t *pstNext = pstTemp->next;
            if (pstNext)
            {
                pstNext->prev = pstTemp->prev;
            }

            BreakPoint_t *pstPrev = pstTemp->prev;
            if (pstPrev)
            {
                pstPrev->next = pstTemp->next;
            }

            // Adjust list-head if necessary
            if (pstTemp == stCPU.pstBreakPoints)
            {
                stCPU.pstBreakPoints = pstNext;
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
bool BreakPoint_EnabledAtAddress( uint32_t u32Addr_ )
{
    BreakPoint_t *pstTemp = stCPU.pstBreakPoints;

    while (pstTemp)
    {
        if (pstTemp->u32Addr == u32Addr_)
        {
            return true;
        }
        pstTemp = pstTemp->next;
    }
    return false;
}
