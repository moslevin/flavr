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
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "tlv_file.h"

//---------------------------------------------------------------------------
static FILE *fMyFile = NULL;

//---------------------------------------------------------------------------
void TLV_WriteInit( const char *szPath_ )
{
    if (!fMyFile)
    {
        fMyFile = fopen( szPath_, "wb" );
    }
}

//---------------------------------------------------------------------------
void TLV_WriteFinish( void )
{
    if (fMyFile)
    {
        fclose(fMyFile);
    }
    fMyFile = NULL;
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
//---------------------------------------------------------------------------
int TLV_ReadInit( const char *szPath_, uint8_t **pu8Buffer_ )
{
    FILE *fReadFile = fopen( szPath_, "rb" );
    struct stat stStat;

    if (!fReadFile)
    {
        fprintf(stderr, "Unable to open tlv for input!\n" );
        return 0;
    }

    stat( szPath_, &stStat );
    *pu8Buffer_ = (uint8_t*)malloc( stStat.st_size );
    if (!pu8Buffer_)
    {
        fclose(fReadFile);
        fprintf(stderr, "Unable to allocate local tlv read buffer!\n" );
        return 0;
    }
    fread(*pu8Buffer_, 1, stStat.st_size, fReadFile );

    fclose(fReadFile);
    return stStat.st_size;
}

//---------------------------------------------------------------------------
int TLV_Read( TLV_t *pstTLV_, uint8_t *pu8Buffer_, int iIndex_)
{
    //!! ToDo -- add checks around buffer usage
    TLV_t *pstStreamTLV = (TLV_t*)&(pu8Buffer_[iIndex_]);
    pstTLV_->eTag = pstStreamTLV->eTag;
    pstTLV_->u16Len = pstStreamTLV->u16Len;
    memcpy( pstTLV_->au8Data, pstStreamTLV->au8Data, pstTLV_->u16Len );
    return (sizeof(TLV_t) + pstTLV_->u16Len - 1);
}

//---------------------------------------------------------------------------
void TLV_ReadFinish ( uint8_t *pu8Buffer_ )
{
    if (pu8Buffer_)
    {
        free( pu8Buffer_ );
    }
}
