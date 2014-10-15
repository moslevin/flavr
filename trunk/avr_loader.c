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
  \file  avr_loader.c

  \brief Functions to load intel-formatted programming files into a virtual AVR.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/fcntl.h>

#include "emu_config.h"

#include "avr_cpu.h"
#include "intel_hex.h"

//---------------------------------------------------------------------------
static void AVR_Copy_Record( AVR_CPU *pstCPU_, HEX_Record_t *pstHex_)
{
    uint16_t u16Data;
    uint16_t i;
    for (i = 0; i < pstHex_->u8ByteCount; i += 2)
    {
        u16Data = pstHex_->u8Data[i+1];
        u16Data <<= 8;
        u16Data |= pstHex_->u8Data[i];

        pstCPU_->pusROM[(pstHex_->u16Address + i) >> 1] = u16Data;
    }
}

//---------------------------------------------------------------------------
bool AVR_Load_HEX( AVR_CPU *pstCPU_, const char *szFilePath_)
{
    HEX_Record_t stRecord;
    uint32_t u32Addr = 0;
    int fd = -1;

    if (!pstCPU_)
    {
        fprintf(stderr, "CPU Not Specified\n");
        return false;
    }

    if (!szFilePath_)
    {
        fprintf(stderr, "No programming file specified\n");
        return false;
    }

    fd = open(szFilePath_, O_RDONLY);

    if (-1 == fd)
    {
        fprintf(stderr, "Unable to open file\n");
        return false;
    }

    bool rc = true;

    while (rc)
    {
        rc = HEX_Read_Record(fd, &stRecord);        
        if (RECORD_EOF == stRecord.u8RecordType)
        {
            break;
        }
        if (RECORD_DATA == stRecord.u8RecordType)
        {
            AVR_Copy_Record(pstCPU_, &stRecord);
        }
    }

cleanup:
    close(fd);
    return rc;
}
