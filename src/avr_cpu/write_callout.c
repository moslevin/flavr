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
    \file   write_callout.h

    \brief  Extended emulator functionality allowing for functions to be triggered
            based on RAM-write operations.
*/

#include "write_callout.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

//---------------------------------------------------------------------------
typedef struct Write_Callout_
{
    struct Write_Callout_ *pstNext;     //!< Pointer to the next callout
    uint16_t          u16Addr;          //!< Address in RAM to monitor
    WriteCalloutFunc  pfCallout;        //!< Function to call on write
} Write_Callout_t;

//---------------------------------------------------------------------------
static Write_Callout_t *pstCallouts = 0;

//---------------------------------------------------------------------------
static bool WriteCallout_IsDuplicate( WriteCalloutFunc pfCallout_, uint16_t u16Addr_ )
{
    Write_Callout_t *pstCallout = pstCallouts;

    while (pstCallout)
    {
        if ( (pstCallout->pfCallout == pfCallout_) &&
             (pstCallout->u16Addr == u16Addr_) )
        {
            return true;
        }

        pstCallout = pstCallout->pstNext;
    }
    return false;
}

//---------------------------------------------------------------------------
void WriteCallout_Add( WriteCalloutFunc pfCallout_, uint16_t u16Addr_ )
{
    if (WriteCallout_IsDuplicate(pfCallout_, u16Addr_))
    {
        return;
    }

    Write_Callout_t *pstNewCallout = (Write_Callout_t*)(malloc(sizeof(*pstNewCallout)));

    pstNewCallout->pstNext = pstCallouts;
    pstNewCallout->u16Addr = u16Addr_;
    pstNewCallout->pfCallout = pfCallout_;

    pstCallouts = pstNewCallout;
}

//---------------------------------------------------------------------------
bool WriteCallout_Run( uint16_t u16Addr_, uint8_t u8Data_ )
{
    Write_Callout_t *pstCallout = pstCallouts;
    bool bRet = true;
    while (pstCallout)
    {
        if ( (pstCallout->u16Addr == u16Addr_) ||
             (pstCallout->u16Addr == 0) )
        {
            if (!pstCallout->pfCallout( u16Addr_, u8Data_ ))
            {
                bRet = false;
            }
        }
        pstCallout = pstCallout->pstNext;
    }
    return bRet;
}
