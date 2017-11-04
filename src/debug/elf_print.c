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
    \file

    \brief
*/

#include "elf_print.h"
#include "elf_types.h"
#include "elf_process.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

//---------------------------------------------------------------------------
void ELF_PrintHeader( const uint8_t *pau8Buffer_ )
{
    ElfHeader_t *pstHeader = (ElfHeader_t*)pau8Buffer_;

    if (!pstHeader)
    {
        printf("NULL Header object\n");
        return;
    }

    printf( "--[Magic Number:  ");
    if (pstHeader->u32IdentMagicNumber == ELF_MAGIC_NUMBER)
    {
        printf( "Valid]\n");
    }
    else
    {
        printf( "Invalid (%08X)]\n", pstHeader->u32IdentMagicNumber);
        return;
    }

    printf( "--[Format:  ");
    switch (pstHeader->u8IdentFormat)
    {
    case ELF_CLASS_32BIT:   printf( "32Bit]\n" ); break;
    case ELF_CLASS_64BIT:   printf( "64Bit]\n" ); break;
    default:
        printf( "Unknown (0x%02X)]\n", pstHeader->u8IdentFormat );
        break;
    }

    printf( "--[Endianness:  ");
    switch (pstHeader->u8IdentEndianness)
    {
    case ELF_ENDIAN_BIG:    printf( "Big]\n" ); break;
    case ELF_ENDIAN_LITTLE: printf( "Little]\n" ); break;
    default:
        printf( "Unknown (0x%02X)]\n", pstHeader->u8IdentEndianness );
        break;
    }

    printf( "--[Version:  ");
    if (pstHeader->u8IdentVersion == ELF_IDENT_VERSION_ORIGINAL)
    {
        printf( "Original ELF]\n");
    }
    else
    {
        printf( "Unknown (0x%02X)]\n", pstHeader->u8IdentVersion );
    }

    printf( "--[ABI Format: ");
    switch (pstHeader->u8IdentABI)
    {
    case ELF_OSABI_SYSV:        printf( "System V]\n" ); break;
    case ELF_OSABI_HPUX:        printf( "HP-UX]\n" ); break;
    case ELF_OSABI_NETBSD:      printf( "NetBSD]\n" ); break;
    case ELF_OSABI_LINUX:       printf( "Linux]\n" ); break;
    case ELF_OSABI_SOLARIS:     printf( "Solarix]\n" ); break;
    case ELF_OSABI_AIX:         printf( "AIX]\n" ); break;
    case ELF_OSABI_IRIX:        printf( "IRIX]\n" ); break;
    case ELF_OSABI_FREEBSD:     printf( "FreeBSD]\n" ); break;
    case ELF_OSABI_OPENBSD:     printf( "OpenBSD]\n" ); break;
    default:
        printf( "unknown (0x%02X)]\n", pstHeader->u8IdentABI );
        break;
    }

    printf( "--[ABI Version: 0x%02X]\n", pstHeader->u8IdentABIVersion );

    printf( "--[Binary Type: ");
    switch (pstHeader->u16Type)
    {
    case ELF_TYPE_RELOCATABLE:  printf( "Relocatable]\n"); break;
    case ELF_TYPE_EXECUTABLE:   printf( "Executable]\n"); break;
    case ELF_TYPE_SHARED:       printf( "Shared]\n"); break;
    case ELF_TYPE_CORE:         printf( "Core]\n"); break;
    default:
        printf( "unknown (0x%04X)]\n", pstHeader->u16Type );
        break;
    }

    printf( "--[Machine Type: ");
    switch (pstHeader->u16Machine)
    {
    case ELF_MACHINE_SPARC:     printf( "SPARC]\n" ); break;
    case ELF_MACHINE_X86:       printf( "x86]\n" ); break;
    case ELF_MACHINE_MIPS:      printf( "MIPS]\n" ); break;
    case ELF_MACHINE_POWERPC:   printf( "PowerPC]\n" ); break;
    case ELF_MACHINE_ARM:       printf( "ARM]\n" ); break;
    case ELF_MACHINE_SUPERH:    printf( "SuperH]\n" ); break;
    case ELF_MACHINE_IA64:      printf( "IA64]\n" ); break;
    case ELF_MACHINE_X86_64:    printf( "x86-64]\n" ); break;
    case ELF_MACHINE_AARCH64:   printf( "AArch64]\n" ); break;
    case ELF_MACHINE_AVR:       printf( "Atmel AVR]\n" ); break;
    default:
        printf( "unknown (0x%04X)]\n", pstHeader->u16Machine );
        break;
    }

    printf( "--[Version: 0x%08X]\n",               pstHeader->u32Version );
    printf( "--[Entry Point: 0x%08X]\n",           pstHeader->u32EntryPoint );
    printf( "--[Program Header Offset: 0x%08X]\n", pstHeader->u32PHOffset );
    printf( "--[Section Header Offset: 0x%08X]\n", pstHeader->u32SHOffset );
    printf( "--[Flags: 0x%08X]\n",                 pstHeader->u32Flags );
    printf( "--[Elf Header Size: %d]\n",           pstHeader->u16EHSize );
    printf( "--[Program Header Size: %d]\n",       pstHeader->u16PHSize );
    printf( "--[Program Header Count: %d]\n",      pstHeader->u16PHNum );
    printf( "--[Section Header Size: %d]\n",       pstHeader->u16SHSize );
    printf( "--[Section Header Count: %d]\n",      pstHeader->u16SHNum );
    printf( "--[Sextion Header Index: %d]\n",      pstHeader->u16SHIndex );
}

//---------------------------------------------------------------------------
void ELF_PrintSections( const uint8_t *pau8Buffer_ )
{
    ElfHeader_t *pstHeader = (ElfHeader_t*)pau8Buffer_;
    uint32_t u32StringOffset = ELF_GetHeaderStringTableOffset( pau8Buffer_ );

    uint32_t u32Offset;
    uint16_t u16SHCount;

    u32Offset = pstHeader->u32SHOffset;
    u16SHCount = pstHeader->u16SHNum;

    while (u16SHCount)
    {
        ElfSectionHeader_t *pstSHeader = (ElfSectionHeader_t*)(&pau8Buffer_[u32Offset]);
        printf( "\n--[Section header @ 0x%08X]\n", u32Offset );
        printf( "--[Name       %s]\n", &pau8Buffer_[u32StringOffset + pstSHeader->u32Name] );

        printf( "--[Type       " );
        switch (pstSHeader->u32Type)
        {
        case ELF_SECTION_TYPE_NULL:     printf( "NULL]\n" ); break;
        case ELF_SECTION_TYPE_NOBITS:   printf( "NOBITS]\n" ); break;
        case ELF_SECTION_TYPE_PROGBITS: printf( "PROGBITS]\n" ); break;
        case ELF_SECTION_TYPE_STRTAB:   printf( "STRTAB]\n" ); break;
        case ELF_SECTION_TYPE_SYMTAB:   printf( "SYMTAB]\n" ); break;
        default:
            printf( "(unknown) 0x%08X]\n", pstSHeader->u32Type );
            break;
        }

        printf( "--[Flags      @ 0x%08X]\n", pstSHeader->u32Flags );
        printf( "--[Address    @ 0x%08X]\n", pstSHeader->u32Address );
        printf( "--[Offset     @ 0x%08X]\n", pstSHeader->u32Offset );
        printf( "--[Size       @ 0x%08X]\n", pstSHeader->u32Size );
        printf( "--[Link       @ 0x%08X]\n", pstSHeader->u32Link );
        printf( "--[Info       @ 0x%08X]\n", pstSHeader->u32Info );
        printf( "--[Alignment  @ 0x%08X]\n", pstSHeader->u32Alignment );
        printf( "--[Entry Size @ 0x%08X]\n", pstSHeader->u32EntrySize );
        //--
        u16SHCount--;
        u32Offset += (pstHeader->u16SHSize);
    }
}

//---------------------------------------------------------------------------
void ELF_PrintSymbols( const uint8_t *pau8Buffer_ )
{
    // Get a pointer to the section header for the symbol table
    uint32_t u32Offset = ELF_GetSymbolTableOffset( pau8Buffer_ );
    ElfSectionHeader_t *pstSymHeader = (ElfSectionHeader_t*)(&pau8Buffer_[u32Offset]);

    // Get a pointer to the section header for the symbol table's strings
    u32Offset = ELF_GetSymbolStringTableOffset( pau8Buffer_ );
    ElfSectionHeader_t *pstStrHeader = (ElfSectionHeader_t*)(&pau8Buffer_[u32Offset]);

    // Iterate through the symbol table section, printing out the details of each.
    uint32_t u32SymOffset = pstSymHeader->u32Offset;
    ElfSymbol_t *pstSymbol = (ElfSymbol_t*)(&pau8Buffer_[u32SymOffset]);

    printf( "VALUE SIZE TYPE SCOPE ID NAME\n");
    while (u32SymOffset < (pstSymHeader->u32Offset + pstSymHeader->u32Size))
    {
        printf( "%08X, ", pstSymbol->u32Value );
        printf( "%5d, ", pstSymbol->u32Size );
        uint8_t u8Type = pstSymbol->u8Info & 0x0F;
        switch (u8Type)
        {
            case 0:     printf( "NOTYPE, " ); break;
            case 1:     printf( "OBJECT, " ); break;
            case 2:     printf( "FUNC,   " ); break;
            case 3:     printf( "SECTION," ); break;
            case 4:     printf( "FILE,   " ); break;
            default:    printf( "Unknown (%02X), ", u8Type); break;
        }
        u8Type = (pstSymbol->u8Info >> 4) & 0x0F;
        switch (u8Type)
        {
            case 0:     printf( "LOCAL, " ); break;
            case 1:     printf( "GLOBAL " ); break;
            case 2:     printf( "WEAK,  " ); break;
            default:    printf( "Unknown (%02X), ", u8Type); break;
        }

        if (65521 == pstSymbol->u16SHIndex) // 65521 == special value "ABS"
        {
            printf("  ABS, ");
        }
        else
        {
            printf( "%5d, ", pstSymbol->u16SHIndex );
        }
        printf( "%s\n", &pau8Buffer_[pstSymbol->u32Name + pstStrHeader->u32Offset] );

        u32SymOffset += pstSymHeader->u32EntrySize;
        pstSymbol = (ElfSymbol_t*)(&pau8Buffer_[u32SymOffset]);
    }
}

//---------------------------------------------------------------------------
void ELF_PrintProgramHeaders( const uint8_t *pau8Buffer_ )
{
    ElfHeader_t *pstHeader = (ElfHeader_t*)(pau8Buffer_);
    uint32_t u32Offset = pstHeader->u32PHOffset;
    uint32_t u16Count = pstHeader->u16PHNum;

    while (u16Count)
    {
        ElfProgramHeader_t *pstProgHeader = (ElfProgramHeader_t*)(&pau8Buffer_[u32Offset]);
        printf( "Program Header:\n" );
        printf( "--[Type:      %08X]\n",  pstProgHeader->u32Type );
        printf( "--[Offset:    %08X]\n",  pstProgHeader->u32Offset );
        printf( "--[VAddr:     %08X]\n",  pstProgHeader->u32VirtualAddress );
        printf( "--[PAddr:     %08X]\n",  pstProgHeader->u32PhysicalAddress );
        printf( "--[FileSize:  %08X]\n",  pstProgHeader->u32FileSize );
        printf( "--[MemSize:   %08X]\n",  pstProgHeader->u32MemSize );
        printf( "--[Flags:     %08X]\n",  pstProgHeader->u32Flags );
        printf( "--[Alignment: %08X]\n",  pstProgHeader->u32Alignment );

        //---
        u32Offset += pstHeader->u16PHSize;
        u16Count--;
    }
}
