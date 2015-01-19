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
  \file  intel_hex.c

  \brief Module for decoding Intel hex formatted programming files
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/fcntl.h>

#include "emu_config.h"

#include "intel_hex.h"

//---------------------------------------------------------------------------
void HEX_Print_Record( HEX_Record_t *stRecord_ )
{
    printf(  "Line: %d\n"
             "ByteCount: %d\n"
             "RecordType: %d\n"
             "Address: %X\n"
             "Data:",
             stRecord_->u32Line,
             stRecord_->u8ByteCount,
             stRecord_->u8RecordType,
             stRecord_->u16Address );
    int i;
    for (i = 0; i < stRecord_->u8ByteCount; i++)
    {
        printf( " %02X", stRecord_->u8Data[i]);
    }
    printf( "\n" );
}

//---------------------------------------------------------------------------
static bool HEX_Read_Header( int fd_ )
{
    ssize_t bytes_read;
    char acBuf[2] = {0};

    bytes_read = read(fd_, acBuf, 1);
    if (1 != bytes_read)
    {
        return false;
    }
    if (':' == acBuf[0])
    {
        return true;
    }
    return false;
}

//---------------------------------------------------------------------------
static bool HEX_Next_Line( int fd_, HEX_Record_t *stRecord_ )
{
    ssize_t bytes_read;
    char acBuf[2] = {0};

    stRecord_->u32Line++;
    do
    {
        bytes_read = read(fd_, acBuf, 1);
        if (1 != bytes_read)
        {
            return false;
        }
    } while(acBuf[0] != '\n');

    return true;
}

//---------------------------------------------------------------------------
static bool HEX_Read_Record_Type( int fd_, HEX_Record_t *stRecord_ )
{
    ssize_t bytes_read;
    uint32_t u32Hex;
    char acBuf[3] = {0};

    bytes_read = read(fd_, acBuf, 2);
    if (2 != bytes_read)
    {
        return false;
    }
    sscanf(acBuf, "%02X", &u32Hex);
    stRecord_->u8RecordType = (uint8_t)u32Hex;

    if (stRecord_->u8RecordType >= RECORD_TYPE_MAX)
    {
        return false;
    }

    return true;
}

//---------------------------------------------------------------------------
static bool HEX_Read_Byte_Count( int fd_, HEX_Record_t *stRecord_ )
{
    ssize_t bytes_read;
    uint32_t u32Hex;
    char acBuf[3] = {0};

    bytes_read = read(fd_, acBuf, 2);
    if (2 != bytes_read)
    {
        return false;
    }
    sscanf(acBuf, "%02X", &u32Hex);
    stRecord_->u8ByteCount = (uint8_t)u32Hex;

    return true;
}

//---------------------------------------------------------------------------
static bool HEX_Read_Address( int fd_, HEX_Record_t *stRecord_ )
{
    ssize_t bytes_read;
    uint32_t u32Hex;
    char acBuf[5] = {0};

    bytes_read = read(fd_, acBuf, 4);
    if (4 != bytes_read)
    {
        return false;
    }
    sscanf(acBuf, "%04X", &u32Hex);
    stRecord_->u16Address = (uint16_t)u32Hex;

    return true;
}

//---------------------------------------------------------------------------
static bool HEX_Read_Data( int fd_, HEX_Record_t *stRecord_ )
{
    ssize_t bytes_read;
    uint32_t u32Hex;
    char acBuf[MAX_HEX_DATA_BYTES * 2] = {0};

    int i;
    for (i = 0; i < stRecord_->u8ByteCount; i++)
    {
        // printf("i:%d\n", i);
        bytes_read = read(fd_, acBuf, 2);
        if (2 != bytes_read)
        {
            return false;
        }
        sscanf(acBuf, "%02X", &u32Hex);
        stRecord_->u8Data[i] = (uint8_t)u32Hex;
    }

    return true;
}

//---------------------------------------------------------------------------
static bool HEX_Read_Checksum( int fd_, HEX_Record_t *stRecord_ )
{
    ssize_t bytes_read;
    uint32_t u32Hex;
    char acBuf[3] = {0,0,0};

    bytes_read = read(fd_, acBuf, 2);
    if (2 != bytes_read)
    {
        return false;
    }
    sscanf(acBuf, "%02X", &u32Hex);
    stRecord_->u8Checksum = (uint8_t)u32Hex;

    return true;
}

//---------------------------------------------------------------------------
static bool HEX_Line_Validate( HEX_Record_t *stRecord_ )
{
    // Calculate the CRC for the fields in the struct and compare
    // against the value read from file...
    uint8_t u8CRC = 0;
    u8CRC += (uint8_t)(stRecord_->u16Address >> 8);
    u8CRC += (uint8_t)(stRecord_->u16Address & 0x00FF);
    u8CRC += stRecord_->u8ByteCount;
    u8CRC += stRecord_->u8RecordType;

    uint8_t i;
    for (i = 0; i < stRecord_->u8ByteCount; i++)
    {
        u8CRC += stRecord_->u8Data[i];
    }

    u8CRC = (~u8CRC) + 1;   // Spec says to take the 2's complement
    if (u8CRC != stRecord_->u8Checksum)
    {
        return false;
    }

    return true;
}

//---------------------------------------------------------------------------
bool HEX_Read_Record( int fd_, HEX_Record_t *stRecord_ )
{
    bool rc = true;
    if (rc)
    {
        rc = HEX_Read_Header(fd_);
    }
    if (rc)
    {
        rc = HEX_Read_Byte_Count(fd_, stRecord_);
    }
    if (rc)
    {
        rc = HEX_Read_Address(fd_, stRecord_);
    }
    if (rc)
    {
        rc = HEX_Read_Record_Type(fd_, stRecord_);
    }
    if (rc)
    {
        rc = HEX_Read_Data(fd_, stRecord_);
    }
    if (rc)
    {
        rc = HEX_Read_Checksum(fd_, stRecord_);
    }
    if (rc)
    {
        rc = HEX_Line_Validate(stRecord_);
    }

    HEX_Next_Line(fd_, stRecord_);
    return rc;
}
