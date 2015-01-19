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

#include "elf_types.h"
#include "elf_process.h"
#include "elf_print.h"

//---------------------------------------------------------------------------
static void AVR_Copy_Record( HEX_Record_t *pstHex_)
{
    uint16_t u16Data;
    uint16_t i;
    for (i = 0; i < pstHex_->u8ByteCount; i += 2)
    {
        u16Data = pstHex_->u8Data[i+1];
        u16Data <<= 8;
        u16Data |= pstHex_->u8Data[i];

        stCPU.pu16ROM[(pstHex_->u16Address + i) >> 1] = u16Data;
    }
}

//---------------------------------------------------------------------------
bool AVR_Load_HEX( const char *szFilePath_)
{
    HEX_Record_t stRecord;
    uint32_t u32Addr = 0;
    int fd = -1;

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
            AVR_Copy_Record(&stRecord);
        }
    }

cleanup:
    close(fd);
    return rc;
}

//---------------------------------------------------------------------------
bool AVR_Load_ELF( const char *szFilePath_)
{
    uint8_t *pu8Buffer;

    // Load the ELF Binary from into a newly-created local buffer
    if (0 != ELF_LoadFromFile(&pu8Buffer, szFilePath_))
    {
        return false;
    }

    // Loaded ELF successfully, load program sections into AVR memory.
    ElfHeader_t *pstHeader = (ElfHeader_t*)(pu8Buffer);
    uint32_t     u32Offset = pstHeader->u32PHOffset;
    uint32_t     u32MaxOffset = pstHeader->u32PHOffset
                                + (pstHeader->u16PHNum * pstHeader->u16PHSize);

    ELF_PrintProgramHeaders(pu8Buffer);

    // Iterate through every program header section in the elf-file
    while (u32Offset < u32MaxOffset)
    {
        ElfProgramHeader_t *pstPHeader = (ElfProgramHeader_t*)(&pu8Buffer[u32Offset]);
        printf("Header @ Offset %08X\n", u32Offset);

        printf("  Section File Size: %d\n", pstPHeader->u32FileSize );
        printf("  Section Mem Size: %d\n", pstPHeader->u32MemSize );
        printf("  Section VAddr: %08X\n", pstPHeader->u32VirtualAddress );

        // RAM encoded in ELF file using addresses >= 0x00800000
        if (pstPHeader->u32VirtualAddress >= 0x00800000)
        {
            printf("  Destination is RAM... @ %04X\n", pstPHeader->u32VirtualAddress & 0x0000FFFF );
            // Clear range in segment
            memset( &(stCPU.pstRAM->au8RAM[pstPHeader->u32VirtualAddress & 0x0000FFFF]),
                    0,
                    pstPHeader->u32MemSize /2 );
            // Copy program segment from ELF into CPU RAM
            memcpy( &(stCPU.pstRAM->au8RAM[pstPHeader->u32VirtualAddress & 0x0000FFFF]),
                    &pu8Buffer[pstPHeader->u32Offset],
                    pstPHeader->u32FileSize /2 );
        }
        else
        {
            printf("  Destination is FLASH... @ %04X\n", pstPHeader->u32VirtualAddress );
            // Clear range in segment
            memset( &(stCPU.pu16ROM[pstPHeader->u32VirtualAddress >> 1]),
                    0,
                    pstPHeader->u32MemSize /2 );

            // Copy program segment from ELF into CPU Flash
            memcpy( &(stCPU.pu16ROM[pstPHeader->u32VirtualAddress >> 1]),
                    &pu8Buffer[pstPHeader->u32Offset],
                    pstPHeader->u32FileSize /2 );
        }

        // Next Section...
        u32Offset += pstHeader->u16PHSize;
    }
    free( pu8Buffer );
    printf("done...\n");
    return true;
}
