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

#include "elf_process.h"
#include "elf_types.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define DEBUG_PRINT(...)

//---------------------------------------------------------------------------
uint32_t ELF_GetHeaderStringTableOffset( const uint8_t *pau8Buffer_ )
{
    ElfHeader_t *pstHeader = (ElfHeader_t*)pau8Buffer_;

    ElfSectionHeader_t *pstStringTable =
            (ElfSectionHeader_t*)(&pau8Buffer_[pstHeader->u32SHOffset + (pstHeader->u16SHSize * pstHeader->u16SHIndex)]);

    return pstStringTable->u32Offset;
}

//---------------------------------------------------------------------------
uint32_t ELF_GetSymbolStringTableOffset( const uint8_t *pau8Buffer_ )
{
    uint32_t u32Offset;
    uint16_t u16SHCount;

    ElfHeader_t *pstHeader = (ElfHeader_t*)pau8Buffer_;
    uint32_t u32StringOffset = ELF_GetHeaderStringTableOffset( pau8Buffer_ );

    u32Offset = pstHeader->u32SHOffset;
    u16SHCount = pstHeader->u16SHNum;

    while (u16SHCount)
    {
        ElfSectionHeader_t *pstSHeader = (ElfSectionHeader_t*)(&pau8Buffer_[u32Offset]);
        if (
            (ELF_SECTION_TYPE_STRTAB == pstSHeader->u32Type) &&
            (0 == strcmp( ".strtab", &pau8Buffer_[u32StringOffset + pstSHeader->u32Name]))
           )

        {
            return u32Offset;
        }

        //--
        u16SHCount--;
        u32Offset += pstHeader->u16SHSize;
    }

    return 0;
}

//---------------------------------------------------------------------------
uint32_t ELF_GetSymbolTableOffset( const uint8_t *pau8Buffer_ )
{
    uint32_t u32Offset;
    uint16_t u16SHCount;

    ElfHeader_t *pstHeader = (ElfHeader_t*)pau8Buffer_;

    u32Offset = pstHeader->u32SHOffset;
    u16SHCount = pstHeader->u16SHNum;

    while (u16SHCount)
    {
        ElfSectionHeader_t *pstSHeader = (ElfSectionHeader_t*)(&pau8Buffer_[u32Offset]);
        if (ELF_SECTION_TYPE_SYMTAB == pstSHeader->u32Type)
        {
            return u32Offset;
        }

        //--
        u16SHCount--;
        u32Offset += pstHeader->u16SHSize;
    }

    return 0;
}

//---------------------------------------------------------------------------
int ELF_LoadFromFile( uint8_t **ppau8Buffer_, const char *szPath_ )
{
    size_t      file_size;
    FILE        *my_file;

    my_file = fopen( szPath_, "rb" );
    if (NULL == my_file)
    {
        DEBUG_PRINT( "Unable to read file @ %s\n", szPath_ );
        return -1;
    }
    fseek(my_file, 0, SEEK_END);
    file_size = ftell(my_file);
    fseek(my_file, 0, SEEK_SET);

    uint8_t *bufptr = (uint8_t*)malloc(file_size);
    *ppau8Buffer_ = bufptr;

    if (!bufptr)
    {
        DEBUG_PRINT( "Unable to malloc elf file buffer\n" );
        fclose( my_file );
        return -1;
    }

    size_t bytes_read = 0;
    while (bytes_read < file_size)
    {
        size_t iter_read = fread( bufptr, 1, 4096, my_file );
        if( iter_read == 0 )
        {
            DEBUG_PRINT( "%d read total\n", bytes_read );
            break;
        }
        bytes_read += iter_read;
        bufptr += iter_read;
    }

    DEBUG_PRINT( "Success reading %d bytes\n", file_size );
    fclose( my_file );
    return 0;
}
