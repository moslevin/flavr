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
    \file elf_types.h

    \brief Defines and types used by ELF loader and supporting functionality.
*/

#ifndef __ELF_TYPES_H__
#define __ELF_TYPES_H__

#include <stdint.h>

//---------------------------------------------------------------------------
#define ELF_MAGIC_NUMBER            ((uint32_t)0x464C457F) // "~ELF"

#define ELF_CLASS_32BIT             ((uint8_t)1)
#define ELF_CLASS_64BIT             ((uint8_t)2)

#define ELF_ENDIAN_LITTLE           ((uint8_t)1)
#define ELF_ENDIAN_BIG              ((uint8_t)2)

#define ELF_IDENT_VERSION_ORIGINAL  ((uint8_t)1)

#define ELF_OSABI_SYSV              ((uint8_t)0x00)
#define ELF_OSABI_HPUX              ((uint8_t)0x01)
#define ELF_OSABI_NETBSD            ((uint8_t)0x02)
#define ELF_OSABI_LINUX             ((uint8_t)0x03)
#define ELF_OSABI_SOLARIS           ((uint8_t)0x06)
#define ELF_OSABI_AIX               ((uint8_t)0x07)
#define ELF_OSABI_IRIX              ((uint8_t)0x08)
#define ELF_OSABI_FREEBSD           ((uint8_t)0x09)
#define ELF_OSABI_OPENBSD           ((uint8_t)0x0C)

#define ELF_TYPE_RELOCATABLE        ((uint8_t)0x01)
#define ELF_TYPE_EXECUTABLE         ((uint8_t)0x02)
#define ELF_TYPE_SHARED             ((uint8_t)0x03)
#define ELF_TYPE_CORE               ((uint8_t)0x04)

#define ELF_MACHINE_SPARC           ((uint16_t)0x02)
#define ELF_MACHINE_X86             ((uint16_t)0x03)
#define ELF_MACHINE_MIPS            ((uint16_t)0x08)
#define ELF_MACHINE_POWERPC         ((uint16_t)0x14)
#define ELF_MACHINE_ARM             ((uint16_t)0x28)
#define ELF_MACHINE_SUPERH          ((uint16_t)0x2A)
#define ELF_MACHINE_IA64            ((uint16_t)0x32)
#define ELF_MACHINE_X86_64          ((uint16_t)0x3E)
#define ELF_MACHINE_AVR             ((uint16_t)0x53)
#define ELF_MACHINE_AARCH64         ((uint16_t)0xB7)

#define ELF_VERSION_ORIGINAL        ((uint32_t)1)

#define ELF_SECTION_TYPE_NULL       ((uint32_t)0)
#define ELF_SECTION_TYPE_PROGBITS   ((uint32_t)1)
#define ELF_SECTION_TYPE_SYMTAB     ((uint32_t)2)
#define ELF_SECTION_TYPE_STRTAB     ((uint32_t)3)
#define ELF_SECTION_TYPE_NOBITS     ((uint32_t)8)

//---------------------------------------------------------------------------
typedef struct
{
    // (Explicit line breaks to show 32-bit alignment)
    //---- 0x00
    uint32_t    u32IdentMagicNumber;

    //---- 0x04
    uint8_t     u8IdentFormat;
    uint8_t     u8IdentEndianness;
    uint8_t     u8IdentVersion;
    uint8_t     u8IdentABI;

    //---- 0x08
    uint8_t     u8IdentABIVersion;
    uint8_t     u8Pad1[7];

    //---- 0x10
    uint16_t    u16Type;
    uint16_t    u16Machine;

    //---- 0x14
    uint32_t    u32Version;

    //---- 0x18
    uint32_t    u32EntryPoint;

    //---- 0x1C
    uint32_t    u32PHOffset;

    //---- 0x20
    uint32_t    u32SHOffset;

    //---- 0x24
    uint32_t    u32Flags;

    //---- 0x28
    uint16_t    u16EHSize;
    uint16_t    u16PHSize;

    //---- 0x2C
    uint16_t    u16PHNum;
    uint16_t    u16SHSize;

    //---- 0x30
    uint16_t    u16SHNum;
    uint16_t    u16SHIndex;

} ElfHeader_t;

//---------------------------------------------------------------------------
typedef struct
{
    uint32_t    u32Type;
    uint32_t    u32Offset;
    uint32_t    u32VirtualAddress;
    uint32_t    u32PhysicalAddress;
    uint32_t    u32FileSize;
    uint32_t    u32MemSize;
    uint32_t    u32Flags;
    uint32_t    u32Alignment;
} ElfProgramHeader_t;

//---------------------------------------------------------------------------
typedef struct
{
    uint32_t    u32Name;
    uint32_t    u32Type;
    uint32_t    u32Flags;
    uint32_t    u32Address;
    uint32_t    u32Offset;
    uint32_t    u32Size;
    uint32_t    u32Link;
    uint32_t    u32Info;
    uint32_t    u32Alignment;
    uint32_t    u32EntrySize;
} ElfSectionHeader_t;

//---------------------------------------------------------------------------
typedef struct
{
    uint32_t    u32Name;
    uint32_t    u32Value;
    uint32_t    u32Size;
    uint8_t     u8Info;
    uint8_t     u8Other;
    uint16_t    u16SHIndex;
} ElfSymbol_t;


#endif //__ELF_TYPES_H__
