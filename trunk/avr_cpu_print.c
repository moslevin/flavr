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
  \file  avr_cpu_print.c

  \brief Helper module used to print the contents of a virtual AVR's internal
         registers and memory.
*/

#include "avr_cpu.h"

#include "emu_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

//---------------------------------------------------------------------------
#define PRINT_FUNC      printf

#define RAM_DISPLAY_SPAN        (16)
#define ROM_DISPLAY_SPAN        (8)

//---------------------------------------------------------------------------
void print_core_regs( AVR_CPU *pstCPU_ )
{
    uint8_t i;
    for (i = 0; i < 32; i++)
    {
        PRINT_FUNC( "[R%02d] = 0x%02X\n", i, pstCPU_->pstRAM->stRegisters.CORE_REGISTERS.r[i] );
    }
    PRINT_FUNC("[SP]  = 0x%02X%02X\n", (uint8_t)pstCPU_->pstRAM->stRegisters.SPH.r, (uint8_t)pstCPU_->pstRAM->stRegisters.SPL.r );
    PRINT_FUNC("[PC]  = 0x%04X\n", (uint16_t)pstCPU_->u16PC );
    PRINT_FUNC("[SREG]= 0x%02X   [", pstCPU_->pstRAM->stRegisters.SREG.r );

    if (1 == pstCPU_->pstRAM->stRegisters.SREG.I)
    {
        PRINT_FUNC("I");
    }
    else
    {
        PRINT_FUNC("-");
    }
    if (1 == pstCPU_->pstRAM->stRegisters.SREG.T)
    {
        PRINT_FUNC("T");
    }
    else
    {
        PRINT_FUNC("-");
    }
    if (1 == pstCPU_->pstRAM->stRegisters.SREG.H)
    {
        PRINT_FUNC("H");
    }
    else
    {
        PRINT_FUNC("-");
    }
    if (1 == pstCPU_->pstRAM->stRegisters.SREG.S)
    {
        PRINT_FUNC("S");
    }
    else
    {
        PRINT_FUNC("-");
    }
    if (1 == pstCPU_->pstRAM->stRegisters.SREG.V)
    {
        PRINT_FUNC("V");
    }
    else
    {
        PRINT_FUNC("-");
    }
    if (1 == pstCPU_->pstRAM->stRegisters.SREG.N)
    {
        PRINT_FUNC("N");
    }
    else
    {
        PRINT_FUNC("-");
    }
    if (1 == pstCPU_->pstRAM->stRegisters.SREG.Z)
    {
        PRINT_FUNC("Z");
    }
    else
    {
        PRINT_FUNC("-");
    }
    if (1 == pstCPU_->pstRAM->stRegisters.SREG.C)
    {
        PRINT_FUNC("C");
    }
    else
    {
        PRINT_FUNC("-");
    }
    PRINT_FUNC("]\n");
}

//---------------------------------------------------------------------------
void print_io_reg( AVR_CPU *pstCPU_, uint8_t u8Addr_ )
{
    PRINT_FUNC( "[IO%02X]= 0x%02X\n", u8Addr_, pstCPU_->pstRAM->au8RAM[u8Addr_] );
}

//---------------------------------------------------------------------------
void print_io_reg_with_name( AVR_CPU *pstCPU_, uint8_t u8Addr_, const char *szName_ )
{
    PRINT_FUNC( "[%s]= 0x%02X\n", szName_, pstCPU_->pstRAM->au8RAM[u8Addr_] );
}

//---------------------------------------------------------------------------
void print_ram( AVR_CPU *pstCPU_, uint16_t u16Start_, uint16_t u16Span_ )
{
    uint16_t i, j;

    while (u16Span_)
    {
        // Print the current memory address
        PRINT_FUNC( "[0x%04X]", u16Start_ );
        if (u16Span_ < RAM_DISPLAY_SPAN)
        {
            j = u16Span_;
        }
        else
        {
            j = RAM_DISPLAY_SPAN;
        }

        // Print a divider, followed by the ASCII codes for each char
        PRINT_FUNC( "|" );
        for (i = 0; i < j; i++)
        {
            uint8_t u8Char = pstCPU_->pstRAM->au8RAM[u16Start_ + i];
            if (u8Char < 32)
            {
                u8Char = '.';
            }

            PRINT_FUNC( " %c", u8Char );
        }
        i = j;
        while (i < RAM_DISPLAY_SPAN)
        {
            PRINT_FUNC("  ");
            i++;
        }

        // Print a divider, followed by the HEX code for each char
        PRINT_FUNC( "|" );
        for (i = 0; i < j; i++)
        {
            PRINT_FUNC( " %02X", pstCPU_->pstRAM->au8RAM[u16Start_ + i]);
        }

        if (u16Span_ < RAM_DISPLAY_SPAN)
        {
            u16Span_ = 0;
        }
        else
        {
            u16Span_ -= RAM_DISPLAY_SPAN;
        }
        u16Start_ += RAM_DISPLAY_SPAN;
        PRINT_FUNC( "\n" );
    }
}

//---------------------------------------------------------------------------
void print_rom( AVR_CPU *pstCPU_, uint16_t u16Start_, uint16_t u16Span_ )
{
    uint16_t i, j;

    while (u16Span_)
    {
        // Print the current memory address
        PRINT_FUNC( "[0x%04X]", u16Start_ );
        if (u16Span_ < ROM_DISPLAY_SPAN)
        {
            j = u16Span_;
        }
        else
        {
            j = ROM_DISPLAY_SPAN;
        }

        // Print a divider, followed by the ASCII codes for each char
        PRINT_FUNC( "|" );
        for (i = 0; i < j; i++)
        {
            uint16_t u16Val = pstCPU_->pu16ROM[u16Start_ + i];
            uint8_t u8High = u16Val >> 8;
            uint8_t u8Low = u16Val & 0x00FF;

            if (u8High < 32)
            {
                u8High = '.';
            }
            if (u8Low < 32)
            {
                u8Low = '.';
            }

            PRINT_FUNC( " %c%c", u8High, u8Low );
        }
        i = j;
        while (i < ROM_DISPLAY_SPAN)
        {
            PRINT_FUNC("  ");
            i++;
        }

        // Print a divider, followed by the HEX code for each char
        PRINT_FUNC( "|" );
        for (i = 0; i < j; i++)
        {
            PRINT_FUNC( " %04X", pstCPU_->pu16ROM[u16Start_ + i]);
        }

        if (u16Span_ < ROM_DISPLAY_SPAN)
        {
            u16Span_ = 0;
        }
        else
        {
            u16Span_ -= ROM_DISPLAY_SPAN;
        }
        u16Start_ += ROM_DISPLAY_SPAN;
        PRINT_FUNC( "\n" );
    }
}
