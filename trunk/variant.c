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
  \file  variant.c

  \brief Module containing a table of device variants supported by flavr.
*/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "variant.h"

//---------------------------------------------------------------------------
#define KB  * (1024)

//---------------------------------------------------------------------------
static AVR_Variant_t astVariants[] =
{
    { "atmega328p",  2 KB,   32 KB, 1 KB,    NULL },
    { "atmega328",   2 KB,   32 KB, 1 KB,    NULL },
    { "atmega168pa", 1 KB,   16 KB, 0.5 KB,  NULL },
    { "atmega168",   1 KB,   16 KB, 0.5 KB,  NULL },
    { "atmega88pa",  1 KB,   8 KB,  0.5 KB,  NULL },
    { "atmega88",    1 KB,   8 KB,  0.5 KB,  NULL },
    { "atmega44pa",  0.5 KB, 4 KB,  0.25 KB, NULL },
    { "atmega44",    0.5 KB, 4 KB,  0.25 KB, NULL },
    { 0 }
};

//---------------------------------------------------------------------------
const AVR_Variant_t *Variant_GetByName( const char *szName_ )
{
    AVR_Variant_t *pstVariant = astVariants;
    while (pstVariant->szName)
    {
        if (0 == strcmp(pstVariant->szName, szName_ ) )
        {
            return pstVariant;
        }
        pstVariant++;
    }
    return NULL;
}
