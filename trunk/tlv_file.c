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
  \file  tlv_file.c

  \brief Tag-length-value file format used for encoding simulator run-time
         data (kernel-aware plugin data, code profiling statistics, etc.).
*/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tlv_file.h"

//---------------------------------------------------------------------------
static FILE *fMyFile = NULL;

//---------------------------------------------------------------------------
void TLV_Init( const char *szPath_ )
{
    if (!fMyFile)
    {
        fMyFile = fopen( szPath_, "wb" );
    }
}

//---------------------------------------------------------------------------
TLV_t *TLV_Alloc( uint16_t u16Len_ )
{
    return (TLV_t*)(malloc(sizeof(TLV_t) + u16Len_ - 1));
}

//---------------------------------------------------------------------------
void TLV_Free( TLV_t *pstTLV_ )
{
    free( pstTLV_ );
}

//---------------------------------------------------------------------------
int TLV_Write( TLV_t *pstData_ )
{
    if (fMyFile)
    {
        return fwrite( (void*)pstData_, sizeof(uint8_t), sizeof(TLV_t) + pstData_->u16Len - 1, fMyFile );
    }
    return -1;
}
