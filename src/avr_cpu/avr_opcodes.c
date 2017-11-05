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
  \file  avr_opcodes.c

  \brief AVR CPU - Opcode implementation
*/


#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "avr_cpu_print.h"
#include "emu_config.h"
#include "avr_opcodes.h"
#include "interactive.h"
#include "write_callout.h"
#include "interrupt_callout.h"

//---------------------------------------------------------------------------
#define DEBUG_PRINT(...)

//---------------------------------------------------------------------------
static void AVR_Abort(void)
{
    print_core_regs();
    exit(-1);
}

//---------------------------------------------------------------------------
static inline uint32_t Get_ZAddress(void)
{
    return (((uint32_t)stCPU.pstRAM->stRegisters.CORE_REGISTERS.Z) |
           (((uint32_t)stCPU.pstRAM->stRegisters.RAMPZ) << 16));
}

//---------------------------------------------------------------------------
static inline uint32_t Get_ZAddressPostInc(void)
{
    uint32_t u32RC = (((uint32_t)stCPU.pstRAM->stRegisters.CORE_REGISTERS.Z) |
                     (((uint32_t)stCPU.pstRAM->stRegisters.RAMPZ) << 16));

    stCPU.pstRAM->stRegisters.CORE_REGISTERS.Z++;

    return u32RC;
}

//---------------------------------------------------------------------------
static inline uint32_t Get_ZAddressPreDec(void)
{
    stCPU.pstRAM->stRegisters.CORE_REGISTERS.Z--;
    return (((uint32_t)stCPU.pstRAM->stRegisters.CORE_REGISTERS.Z) |
           (((uint32_t)stCPU.pstRAM->stRegisters.RAMPZ) << 16));
}

//---------------------------------------------------------------------------
static inline uint32_t Get_YAddress(void)
{
    return ((uint32_t)stCPU.pstRAM->stRegisters.CORE_REGISTERS.Y);
}

//---------------------------------------------------------------------------
static inline uint32_t Get_XAddress(void)
{
    return ((uint32_t)stCPU.pstRAM->stRegisters.CORE_REGISTERS.X);
}

//---------------------------------------------------------------------------
static void Data_Write( uint32_t u32Addr_, uint8_t u8Val_ )
{
    // Writing to RAM can be a tricky deal, because the address space is shared
    // between RAM, the core registers, and a bunch of peripheral I/O registers.
    DEBUG_PRINT("Write: 0x%08X=%02X\n", u32Addr_, u8Val_ );
    if (!WriteCallout_Run( u32Addr_, u8Val_ ))
    {
        return;
    }

    // Check to see if the write operation falls within the peripheral I/O range
    if (u32Addr_ >= 32 && u32Addr_ <= 255)
    {
        // I/O range - check to see if there's a peripheral installed at this address
        IOWriterList *pstIOWrite = stCPU.apstPeriphWriteTable[ u32Addr_ ];

        // If there is a peripheral or peripherals
        if (pstIOWrite)
        {
            // Iterate through the list of installed peripherals at this address, and
            // call their write handler
            while (pstIOWrite)
            {
                pstIOWrite->pfWriter( pstIOWrite->pvContext, (uint8_t)u32Addr_, u8Val_ );
                pstIOWrite = pstIOWrite->next;
            }
        }
        // Otherwise, there is no peripheral -- just assume we can treat this as normal RAM.
        else
        {
            stCPU.pstRAM->au8RAM[ u32Addr_ ] = u8Val_;
        }
    }
    else if (u32Addr_ >= (stCPU.u32RAMSize + 256))
    {
        fprintf( stderr, "[Write Abort] RAM Address 0x%08X is out of range!\n", u32Addr_ );
        AVR_Abort();
    }
    // RAM address range - direct write-through.
    else
    {
        stCPU.pstRAM->au8RAM[ u32Addr_ ] = u8Val_;
    }

}

//---------------------------------------------------------------------------
static uint8_t Data_Read( uint32_t u32Addr_)
{
    // Writing to RAM can be a tricky deal, because the address space is shared
    // between RAM, the core registers, and a bunch of peripheral I/O registers.

    // Check to see if the write operation falls within the peripheral I/O range
    DEBUG_PRINT( "Data Read: %08X\n", u32Addr_ );
    if (u32Addr_ >= 32 && u32Addr_ <= 255)
    {
        // I/O range - check to see if there's a peripheral installed at this address
        IOReaderList *pstIORead = stCPU.apstPeriphReadTable[ u32Addr_ ];
        DEBUG_PRINT( "Peripheral Read: 0x%08X\n", u32Addr_ );
        // If there is a peripheral or peripherals
        if (pstIORead)
        {
            DEBUG_PRINT(" Found peripheral\n");
            // Iterate through the list of installed peripherals at this address, and
            // call their read handler
            uint8_t u8Val;
            while (pstIORead)
            {
                pstIORead->pfReader( pstIORead->pvContext,  (uint8_t)u32Addr_, &u8Val);
                pstIORead = pstIORead->next;
            }
            return u8Val;
        }
        // Otherwise, there is no peripheral -- just assume we can treat this as normal RAM.
        else
        {
            DEBUG_PRINT(" No peripheral\n");
            return stCPU.pstRAM->au8RAM[ u32Addr_ ];
        }
    }
    else if (u32Addr_ >= (stCPU.u32RAMSize + 256))
    {
        fprintf( stderr, "[Read Abort] RAM Address 0x%04X is out of range!\n", u32Addr_ );
        AVR_Abort();
    }
    // RAM address range - direct read
    else
    {
        return stCPU.pstRAM->au8RAM[ u32Addr_ ];
    }
}

//---------------------------------------------------------------------------
static void AVR_Opcode_NOP( void )
{
    // Nop - do nothing.
}

//---------------------------------------------------------------------------
inline void ADD_Half_Carry( uint8_t Rd_, uint8_t Rr_, uint8_t Result_)
{
    stCPU.pstRAM->stRegisters.SREG.H =
            ( ((Rd_ & Rr_)    & 0x08 )
            | ((Rr_ & (~Result_)) & 0x08 )
            | (((~Result_) & Rd_) & 0x08) ) != false;
}

//---------------------------------------------------------------------------
inline void ADD_Full_Carry( uint8_t Rd_, uint8_t Rr_, uint8_t Result_)
{
    stCPU.pstRAM->stRegisters.SREG.C =
            ( ((Rd_ & Rr_)    & 0x80 )
            | ((Rr_ & (~Result_)) & 0x80 )
            | (((~Result_) & Rd_) & 0x80) ) != false;
}

//---------------------------------------------------------------------------
inline void ADD_Overflow_Flag( uint8_t Rd_, uint8_t Rr_, uint8_t Result_)
{
    stCPU.pstRAM->stRegisters.SREG.V =
             ( ((Rd_ & Rr_ & ~Result_)  & 0x80 )
             | ((~Rd_ & ~Rr_ & Result_) & 0x80 ) ) != 0;
}

//---------------------------------------------------------------------------
inline void Signed_Flag( void )
{
    unsigned int N = stCPU.pstRAM->stRegisters.SREG.N;
    unsigned int V = stCPU.pstRAM->stRegisters.SREG.V;

    stCPU.pstRAM->stRegisters.SREG.S = N ^ V;
}

//---------------------------------------------------------------------------
inline void R8_Zero_Flag( uint8_t R_ )
{
    stCPU.pstRAM->stRegisters.SREG.Z = (R_ == 0);
}

//---------------------------------------------------------------------------
inline void R8_CPC_Zero_Flag( uint8_t R_ )
{
    stCPU.pstRAM->stRegisters.SREG.Z = (stCPU.pstRAM->stRegisters.SREG.Z && (R_ == 0));
}

//---------------------------------------------------------------------------
inline void R8_Negative_Flag( uint8_t R_ )
{
    stCPU.pstRAM->stRegisters.SREG.N = ((R_ & 0x80) == 0x80);
}

//---------------------------------------------------------------------------
static void AVR_Opcode_ADD( void )
{
    uint8_t u8Result;
    uint8_t u8Rd = *(stCPU.Rd);
    uint8_t u8Rr = *(stCPU.Rr);

    u8Result = u8Rd + u8Rr;
    *(stCPU.Rd) = u8Result;

// ---- Update flags ----
    ADD_Half_Carry(  u8Rd, u8Rr, u8Result );
    ADD_Full_Carry(  u8Rd, u8Rr, u8Result );
    ADD_Overflow_Flag(  u8Rd, u8Rr, u8Result );
    R8_Negative_Flag(  u8Result );
    R8_Zero_Flag(  u8Result );
    Signed_Flag();
}

//---------------------------------------------------------------------------
static void AVR_Opcode_ADC( void )
{
    uint8_t u8Result;
    uint8_t u8Rd = *(stCPU.Rd);
    uint8_t u8Rr = *(stCPU.Rr);
    uint8_t u8Carry = (stCPU.pstRAM->stRegisters.SREG.C);

    u8Result = u8Rd + u8Rr + u8Carry;
    *(stCPU.Rd) = u8Result;

// ---- Update flags ----
    ADD_Half_Carry(  u8Rd, u8Rr, u8Result );
    ADD_Full_Carry(  u8Rd, u8Rr, u8Result );
    ADD_Overflow_Flag(  u8Rd, u8Rr, u8Result );
    R8_Negative_Flag(  u8Result );
    R8_Zero_Flag(  u8Result );
    Signed_Flag();
}

//---------------------------------------------------------------------------
inline void R16_Negative_Flag( uint16_t Result_ )
{
    stCPU.pstRAM->stRegisters.SREG.N =
            ((Result_ & 0x8000) != 0);
}

//---------------------------------------------------------------------------
inline void R16_Zero_Flag( uint16_t Result_ )
{
    stCPU.pstRAM->stRegisters.SREG.Z =
            (Result_ == 0);
}

//---------------------------------------------------------------------------
inline void ADIW_Overflow_Flag( uint16_t Rd_, uint16_t Result_ )
{
    stCPU.pstRAM->stRegisters.SREG.V =
            (((Rd_ & 0x8000) == 0) && ((Result_ & 0x8000) == 0x8000));
}

//---------------------------------------------------------------------------
inline void ADIW_Carry_Flag( uint16_t Rd_, uint16_t Result_ )
{
    stCPU.pstRAM->stRegisters.SREG.C =
            (((Rd_ & 0x8000) == 0x8000) && ((Result_ & 0x8000) == 0));
}

//---------------------------------------------------------------------------
static void AVR_Opcode_ADIW( void )
{
    uint16_t u16K = (stCPU.K);
    uint16_t u16Rd = *(stCPU.Rd16);
    uint16_t u16Result;

    u16Result = u16Rd + u16K;
    *(stCPU.Rd16) = u16Result;

// ---- Update Flags ----   
    ADIW_Carry_Flag(  u16Rd, u16Result);
    ADIW_Overflow_Flag(  u16Rd, u16Result );
    R16_Negative_Flag(  u16Result );
    R16_Zero_Flag(  u16Result );
    Signed_Flag();
}

//---------------------------------------------------------------------------
inline void SUB_Overflow_Flag( uint8_t Rd_, uint8_t Rr_, uint8_t Result_)
{
    stCPU.pstRAM->stRegisters.SREG.V =
             ( ((Rd_ & ~Rr_ & ~Result_) & 0x80 )
             | ((~Rd_ & Rr_ & Result_) & 0x80 ) ) != 0;
}
//---------------------------------------------------------------------------
inline void SUB_Half_Carry( uint8_t Rd_, uint8_t Rr_, uint8_t Result_)
{
    stCPU.pstRAM->stRegisters.SREG.H =
             ( ((~Rd_ & Rr_) & 0x08 )
             | ((Rr_ & Result_) & 0x08 )
             | ((Result_ & ~Rd_) & 0x08 ) ) == 0x08;
}
//---------------------------------------------------------------------------
inline void SUB_Full_Carry( uint8_t Rd_, uint8_t Rr_, uint8_t Result_)
{
    stCPU.pstRAM->stRegisters.SREG.C =
             ( ((~Rd_ & Rr_) & 0x80 )
             | ((Rr_ & Result_) & 0x80 )
             | ((Result_ & ~Rd_) & 0x80 ) ) == 0x80;
}

//---------------------------------------------------------------------------
static void AVR_Opcode_SUB( void )
{
    uint8_t u8Rd = *stCPU.Rd;
    uint8_t u8Rr = *stCPU.Rr;
    uint8_t u8Result = u8Rd - u8Rr;

    *stCPU.Rd = u8Result;

    //--Flags
    SUB_Half_Carry( u8Rd, u8Rr, u8Result);
    SUB_Full_Carry( u8Rd, u8Rr, u8Result);
    SUB_Overflow_Flag( u8Rd, u8Rr, u8Result);
    R8_Negative_Flag( u8Result);
    R8_Zero_Flag( u8Result);
    Signed_Flag();
}

//---------------------------------------------------------------------------
static void AVR_Opcode_SUBI( void )
{
    uint8_t u8Rd = *stCPU.Rd;
    uint8_t u8K = (uint8_t)stCPU.K;
    uint8_t u8Result = u8Rd - u8K;

    *stCPU.Rd = u8Result;

    //--Flags
    SUB_Half_Carry( u8Rd, u8K, u8Result);
    SUB_Full_Carry( u8Rd, u8K, u8Result);
    SUB_Overflow_Flag( u8Rd, u8K, u8Result);
    R8_Negative_Flag( u8Result);
    R8_Zero_Flag( u8Result);
    Signed_Flag();
}

//---------------------------------------------------------------------------
static void AVR_Opcode_SBC( void )
{
    uint8_t u8Rd = *stCPU.Rd;
    uint8_t u8Rr = *stCPU.Rr;
    uint8_t u8C = stCPU.pstRAM->stRegisters.SREG.C;
    uint8_t u8Result = u8Rd - u8Rr - u8C;

    *stCPU.Rd = u8Result;

    //--Flags
    SUB_Half_Carry( u8Rd, u8Rr, u8Result);
    SUB_Full_Carry( u8Rd, u8Rr, u8Result);
    SUB_Overflow_Flag( u8Rd, u8Rr, u8Result);
    R8_Negative_Flag( u8Result);
    if (u8Result)
    {
        stCPU.pstRAM->stRegisters.SREG.Z = 0;
    }
    Signed_Flag();
}

//---------------------------------------------------------------------------
static void AVR_Opcode_SBCI( void )
{
    uint8_t u8Rd = *stCPU.Rd;
    uint8_t u8K = (uint8_t)stCPU.K;
    uint8_t u8C = stCPU.pstRAM->stRegisters.SREG.C;
    uint8_t u8Result = u8Rd - u8K - u8C;

    *stCPU.Rd = u8Result;

    //--Flags
    SUB_Half_Carry( u8Rd, u8K, u8Result);
    SUB_Full_Carry( u8Rd, u8K, u8Result);
    SUB_Overflow_Flag( u8Rd, u8K, u8Result);
    R8_Negative_Flag( u8Result);
    if (u8Result)
    {
        stCPU.pstRAM->stRegisters.SREG.Z = 0;
    }
    Signed_Flag();
}


//---------------------------------------------------------------------------
inline void SBIW_Overflow_Flag( uint16_t Rd_, uint16_t Result_)
{
    stCPU.pstRAM->stRegisters.SREG.V =
             ((Rd_ & 0x8000 ) == 0x8000) && ((Result_ & 0x8000) == 0);

}

//---------------------------------------------------------------------------
inline void SBIW_Full_Carry( uint16_t Rd_, uint16_t Result_)
{
    stCPU.pstRAM->stRegisters.SREG.C =
             ((Rd_ & 0x8000 ) == 0) && ((Result_ & 0x8000) == 0x8000);
}

//---------------------------------------------------------------------------
static void AVR_Opcode_SBIW( void )
{
    uint16_t u16Rd = *stCPU.Rd16;
    uint16_t u16Result;

    //fprintf( stderr, "SBIW: RD=[%4X], K=[%2X]\n", u16Rd, stCPU.K );
    u16Result = u16Rd - stCPU.K;

    *stCPU.Rd16 = u16Result;
    //fprintf( stderr, "  Result=[%4X]\n", u16Result );

    SBIW_Full_Carry( u16Rd, u16Result);
    SBIW_Overflow_Flag( u16Rd, u16Result);
    R16_Negative_Flag( u16Result);
    R16_Zero_Flag( u16Result);
    Signed_Flag();

}

//---------------------------------------------------------------------------
static void AVR_Opcode_AND( void )
{
    uint8_t u8Rd = *stCPU.Rd;
    uint8_t u8Rr = *stCPU.Rr;
    uint8_t u8Result = u8Rd & u8Rr;

    *stCPU.Rd = u8Result;

    //--Update Status registers;
    stCPU.pstRAM->stRegisters.SREG.V = 0;
    R8_Negative_Flag(  u8Result );
    R8_Zero_Flag(  u8Result );
    Signed_Flag();
}

//---------------------------------------------------------------------------
static void AVR_Opcode_ANDI( void )
{
    uint8_t u8Rd = *stCPU.Rd;
    uint8_t u8Result = u8Rd & (uint8_t)stCPU.K;

    *stCPU.Rd = u8Result;

    //--Update Status registers;
    stCPU.pstRAM->stRegisters.SREG.V = 0;
    R8_Negative_Flag(  u8Result );
    R8_Zero_Flag(  u8Result );
    Signed_Flag();
}

//---------------------------------------------------------------------------
static void AVR_Opcode_OR( void )
{
    uint8_t u8Rd = *stCPU.Rd;
    uint8_t u8Rr = *stCPU.Rr;
    uint8_t u8Result = u8Rd | u8Rr;

    *stCPU.Rd = u8Result;

    //--Update Status registers;
    stCPU.pstRAM->stRegisters.SREG.V = 0;
    R8_Negative_Flag(  u8Result );
    R8_Zero_Flag(  u8Result );
    Signed_Flag();
}

//---------------------------------------------------------------------------
static void AVR_Opcode_ORI( void )
{
    uint8_t u8Rd = *stCPU.Rd;
    uint8_t u8Result = u8Rd | (uint8_t)stCPU.K;

    *stCPU.Rd = u8Result;

    //--Update Status registers;
    stCPU.pstRAM->stRegisters.SREG.V = 0;
    R8_Negative_Flag(  u8Result );
    R8_Zero_Flag(  u8Result );
    Signed_Flag();
}

//---------------------------------------------------------------------------
static void AVR_Opcode_EOR( void )
{
    uint8_t u8Rd = *stCPU.Rd;
    uint8_t u8Rr = *stCPU.Rr;
    uint8_t u8Result = u8Rd ^ u8Rr;

    *stCPU.Rd = u8Result;

    //--Update Status registers;
    stCPU.pstRAM->stRegisters.SREG.V = 0;
    R8_Negative_Flag(  u8Result );
    R8_Zero_Flag(  u8Result );
    Signed_Flag();
}

//---------------------------------------------------------------------------
static void AVR_Opcode_COM( void )
{
    // 1's complement.
    uint8_t u8Result = *stCPU.Rd;
    u8Result = (0xFF - u8Result);

    *stCPU.Rd = u8Result;

    //--Update Status registers;
    stCPU.pstRAM->stRegisters.SREG.V = 0;
    stCPU.pstRAM->stRegisters.SREG.C = 1;
    R8_Negative_Flag(  u8Result );
    R8_Zero_Flag(  u8Result );
    Signed_Flag();
}

//---------------------------------------------------------------------------
inline void NEG_Overflow_Flag( uint8_t u8Result_ )
{
    stCPU.pstRAM->stRegisters.SREG.V = (u8Result_ == 0x80);
}

//---------------------------------------------------------------------------
inline void NEG_Carry_Flag( uint8_t u8Result_ )
{
    stCPU.pstRAM->stRegisters.SREG.C = (u8Result_ != 0x00);
}

//---------------------------------------------------------------------------
static void AVR_Opcode_NEG( void )
{
    // 2's complement.
    uint8_t u8Result = *stCPU.Rd;
    u8Result = (0 - u8Result);

    *stCPU.Rd = u8Result;

    //--Update Status registers;
    NEG_Overflow_Flag(  u8Result );
    NEG_Carry_Flag(  u8Result );
    R8_Negative_Flag(  u8Result );
    R8_Zero_Flag(  u8Result );
    Signed_Flag();
}

//---------------------------------------------------------------------------
static void AVR_Opcode_SBR( void )
{
    // Set Bits in Register
    uint8_t u8Result = *stCPU.Rd;
    u8Result |= ((uint8_t)stCPU.K);

    *stCPU.Rd = u8Result;

    //--Update Status registers;
    stCPU.pstRAM->stRegisters.SREG.V = 0;
    R8_Negative_Flag(  u8Result );
    R8_Zero_Flag(  u8Result );
    Signed_Flag();
}

//---------------------------------------------------------------------------
static void AVR_Opcode_CBR( void )
{
    // Clear Bits in Register
    uint8_t u8Result = *stCPU.Rd;
    u8Result &= ~((uint8_t)stCPU.K);

    *stCPU.Rd = u8Result;

    //--Update Status registers;
    stCPU.pstRAM->stRegisters.SREG.V = 0;
    R8_Negative_Flag(  u8Result );
    R8_Zero_Flag(  u8Result );
    Signed_Flag();
}

//---------------------------------------------------------------------------
inline void INC_Overflow_Flag( uint8_t u8Result_ )
{
    stCPU.pstRAM->stRegisters.SREG.V = (u8Result_ == 0x80);
}

//---------------------------------------------------------------------------
static void AVR_Opcode_INC( void )
{
    uint8_t u8Result;
    u8Result = *stCPU.Rd + 1;

    *stCPU.Rd = u8Result;

    //--Update Status registers;
    INC_Overflow_Flag(  u8Result );
    R8_Negative_Flag(  u8Result );
    R8_Zero_Flag(  u8Result );
    Signed_Flag();
}
//---------------------------------------------------------------------------
inline void DEC_Overflow_Flag( uint8_t u8Result_ )
{
    stCPU.pstRAM->stRegisters.SREG.V = (u8Result_ == 0x7F);
}
//---------------------------------------------------------------------------
static void AVR_Opcode_DEC( void )
{   
    uint8_t u8Result;   
    u8Result = *stCPU.Rd - 1;

    *stCPU.Rd = u8Result;

    //--Update Status registers;
    DEC_Overflow_Flag(  u8Result );
    R8_Negative_Flag(  u8Result );
    R8_Zero_Flag(  u8Result );
    Signed_Flag();
}

//---------------------------------------------------------------------------
static void AVR_Opcode_SER( void )
{
    *stCPU.Rd = 0xFF;
}

//---------------------------------------------------------------------------
inline void Mul_Carry_Flag( uint16_t R_ )
{
    stCPU.pstRAM->stRegisters.SREG.C = ((R_  & 0x8000) == 0x8000);
}

//---------------------------------------------------------------------------
inline void Mul_Zero_Flag( uint16_t R_ )
{
    stCPU.pstRAM->stRegisters.SREG.Z = (R_ == 0);
}

//---------------------------------------------------------------------------
static void AVR_Opcode_MUL( void )
{
    uint16_t u16Product;
    uint16_t u16R1;
    uint16_t u16R2;

    u16R1 = *stCPU.Rd;
    u16R2 = *stCPU.Rr;

    u16Product = u16R1 * u16R2;

    stCPU.pstRAM->stRegisters.CORE_REGISTERS.r1_0 = u16Product;

    //-- Update Flags --
    Mul_Zero_Flag( u16Product);
    Mul_Carry_Flag( u16Product);
}

//---------------------------------------------------------------------------
static void AVR_Opcode_MULS( void )
{
    int16_t s16Product;
    int16_t s16R1;
    int16_t s16R2;

    s16R1 = (int8_t)*stCPU.Rd;
    s16R2 = (int8_t)*stCPU.Rr;

    s16Product = s16R1 * s16R2;

    stCPU.pstRAM->stRegisters.CORE_REGISTERS.r1_0 = (uint16_t)s16Product;

    //-- Update Flags --
    Mul_Zero_Flag( (uint16_t)s16Product);
    Mul_Carry_Flag( (uint16_t)s16Product);
}

//---------------------------------------------------------------------------
static void AVR_Opcode_MULSU( void )
{
    int16_t s16Product;
    int16_t s16R1;
    uint16_t u16R2;

    s16R1 = (int8_t)*stCPU.Rd;
    u16R2 = *stCPU.Rr;

    s16Product = s16R1 * u16R2;

    stCPU.pstRAM->stRegisters.CORE_REGISTERS.r1_0 = (uint16_t)s16Product;

    //-- Update Flags --
    Mul_Zero_Flag( (uint16_t)s16Product);
    Mul_Carry_Flag( (uint16_t)s16Product);
}

//---------------------------------------------------------------------------
static void AVR_Opcode_FMUL( void )
{
    uint16_t u16Product;
    uint16_t u16R1;
    uint16_t u16R2;

    u16R1 = *stCPU.Rd;
    u16R2 = *stCPU.Rr;

    u16Product = u16R1 * u16R2;

    stCPU.pstRAM->stRegisters.CORE_REGISTERS.r1_0 = u16Product << 1;

    //-- Update Flags --
    Mul_Zero_Flag( u16Product);
    Mul_Carry_Flag( u16Product);
}

//---------------------------------------------------------------------------
static void AVR_Opcode_FMULS( void )
{
    int16_t s16Product;
    int16_t s16R1;
    int16_t s16R2;

    s16R1 = (int8_t)*stCPU.Rd;
    s16R2 = (int8_t)*stCPU.Rr;

    s16Product = s16R1 * s16R2;

    stCPU.pstRAM->stRegisters.CORE_REGISTERS.r1_0 = ((uint16_t)s16Product) << 1;

    //-- Update Flags --
    Mul_Zero_Flag( (uint16_t)s16Product);
    Mul_Carry_Flag( (uint16_t)s16Product);
}

//---------------------------------------------------------------------------
static void AVR_Opcode_FMULSU( void )
{
    int16_t s16Product;
    int16_t s16R1;
    uint16_t u16R2;

    s16R1 = (int8_t)*stCPU.Rd;
    u16R2 = *stCPU.Rr;

    s16Product = s16R1 * u16R2;

    stCPU.pstRAM->stRegisters.CORE_REGISTERS.r1_0 = ((uint16_t)s16Product) << 1;

    //-- Update Flags --
    Mul_Zero_Flag( (uint16_t)s16Product);
    Mul_Carry_Flag( (uint16_t)s16Product);
}

//---------------------------------------------------------------------------
static void AVR_Opcode_DES( void )
{
    //! ToDo - Implement DES
}

//---------------------------------------------------------------------------
static inline Unconditional_Jump( uint16_t u16Addr_ )
{
    stCPU.u32PC = u16Addr_;
    stCPU.u16ExtraPC = 0;

    // Feature -- Terminate emulator if jump-to-zero encountered at runtime.
    if (stCPU.u32PC == 0 && stCPU.bExitOnReset)
    {
        exit(0);
    }
}

//---------------------------------------------------------------------------
static inline Relative_Jump( uint16_t u16Offset_ )
{
    // u16Offset_ Will always be 1 or 2, based on the size of the next opcode
    // in a program

    stCPU.u32PC += u16Offset_;
    stCPU.u16ExtraPC = 0;
    stCPU.u16ExtraCycles += u16Offset_;
}

//---------------------------------------------------------------------------
static void AVR_Opcode_RJMP( void )
{
    int32_t s32NewPC = (int32_t)stCPU.u32PC + (int32_t)stCPU.k_s + 1;

    Unconditional_Jump(  (uint16_t)s32NewPC );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_IJMP( void )
{
    Unconditional_Jump( Get_ZAddress() );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_EIJMP( void )
{
    //! ToDo - implement EIND instructions
}

//---------------------------------------------------------------------------
static void AVR_Opcode_JMP( void )
{
    Unconditional_Jump(  (uint16_t)stCPU.k );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_RCALL( void )
{
    // Push the next instruction address onto the stack
    uint32_t u32PC = (((uint16_t)stCPU.pstRAM->stRegisters.SPH.r) << 8) |
                     (((uint16_t)stCPU.pstRAM->stRegisters.SPL.r));

    uint32_t u32StoredPC = stCPU.u32PC + 1;

    Data_Write(  u32PC, (uint8_t)(u32StoredPC & 0x00FF));
    Data_Write(  u32PC - 1, (uint8_t)(u32StoredPC >> 8));

    // Stack is post-decremented
    u32PC -= 2;

    // Set the new PC (relative call)
    int32_t s32NewPC = (int32_t)stCPU.u32PC + (int32_t)stCPU.k_s + 1;
    uint32_t u32NewPC = (uint32_t)s32NewPC;

    // Store the new SP.
    stCPU.pstRAM->stRegisters.SPH.r = (u32PC >> 8);
    stCPU.pstRAM->stRegisters.SPL.r = (u32PC & 0x00FF);

    // Set the new PC
    Unconditional_Jump(  u32NewPC );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_ICALL( void )
{
    // Push the next instruction address onto the stack
    uint32_t u32SP = (((uint32_t)stCPU.pstRAM->stRegisters.SPH.r) << 8) |
                     (((uint32_t)stCPU.pstRAM->stRegisters.SPL.r));

    uint32_t u32StoredPC = stCPU.u32PC + 1;

    Data_Write(  u32SP, (uint8_t)(u32StoredPC & 0x00FF));
    Data_Write(  u32SP - 1, (uint8_t)(u32StoredPC >> 8));

    // Stack is post-decremented
    u32SP -= 2;

    // Set the new PC
    uint32_t u32NewPC = Get_ZAddress();

    // Store the new SP.
    stCPU.pstRAM->stRegisters.SPH.r = (u32SP >> 8);
    stCPU.pstRAM->stRegisters.SPL.r = (u32SP & 0x00FF);

    // Set the new PC
    Unconditional_Jump(  u32NewPC );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_EICALL( void )
{
    //!! ToDo - Implement EIND calling!
}

//---------------------------------------------------------------------------
static void AVR_Opcode_CALL( void )
{
    // See ICALL for documentation
    uint32_t u32SP = (((uint32_t)stCPU.pstRAM->stRegisters.SPH.r) << 8) |
                     (((uint32_t)stCPU.pstRAM->stRegisters.SPL.r));

    uint32_t u32StoredPC = stCPU.u32PC + 2;

    Data_Write(  u32SP, (uint8_t)(u32StoredPC & 0x00FF));
    Data_Write(  u32SP - 1, (uint8_t)(u32StoredPC >> 8));

    u32SP -= 2;

    uint32_t u32NewPC = stCPU.k;

    stCPU.pstRAM->stRegisters.SPH.r = (u32SP >> 8);
    stCPU.pstRAM->stRegisters.SPL.r = (u32SP & 0x00FF);

    Unconditional_Jump(  u32NewPC );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_RET( void )
{
    // Pop the next instruction off of the stack, pre-incrementing
    uint32_t u32SP = (((uint32_t)stCPU.pstRAM->stRegisters.SPH.r) << 8) |
                     (((uint32_t)stCPU.pstRAM->stRegisters.SPL.r));
    u32SP += 2;

    uint32_t u32High = Data_Read(  u32SP - 1 );
    uint32_t u32Low = Data_Read(  u32SP );
    uint32_t u32NewPC = (u32High << 8) | u32Low;

    stCPU.pstRAM->stRegisters.SPH.r = (u32SP >> 8);
    stCPU.pstRAM->stRegisters.SPL.r = (u32SP & 0x00FF);

    // Set new PC based on address read from stack
    Unconditional_Jump(  u32NewPC );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_RETI( void )
{
    uint32_t u32SP = (((uint32_t)stCPU.pstRAM->stRegisters.SPH.r) << 8) |
                     (((uint32_t)stCPU.pstRAM->stRegisters.SPL.r));
    u32SP += 2;

    uint32_t u32High = Data_Read(  u32SP - 1 );
    uint32_t u32Low = Data_Read(  u32SP );
    uint32_t u32NewPC = (u32High << 8) | u32Low;

    stCPU.pstRAM->stRegisters.SPH.r = (u32SP >> 8);
    stCPU.pstRAM->stRegisters.SPL.r = (u32SP & 0x00FF);

//-- Enable interrupts
    stCPU.pstRAM->stRegisters.SREG.I = 1;
    Unconditional_Jump( u32NewPC );

//-- Run callout functions registered when we return from interrupt.
    InterruptCallout_Run( false, 0 );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_CPSE( void )
{    
    if (*stCPU.Rr == *stCPU.Rd)
    {        
        uint8_t u8NextOpSize = AVR_Opcode_Size( stCPU.pu16ROM[ stCPU.u32PC + 1 ] );
        Relative_Jump(  u8NextOpSize + 1 );
    }
}

//---------------------------------------------------------------------------
inline void CP_Half_Carry( uint8_t Rd_, uint8_t Rr_, uint8_t Result_)
{
    stCPU.pstRAM->stRegisters.SREG.H =
            ( ((~Rd_ & Rr_)    & 0x08 )
            | ((Rr_ & (Result_)) & 0x08 )
            | (((Result_) & ~Rd_) & 0x08) ) != false;
}

//---------------------------------------------------------------------------
inline void CP_Full_Carry( uint8_t Rd_, uint8_t Rr_, uint8_t Result_)
{
    stCPU.pstRAM->stRegisters.SREG.C =
            ( ((~Rd_ & Rr_)    & 0x80 )
            | ((Rr_ & (Result_)) & 0x80 )
            | (((Result_) & ~Rd_) & 0x80) ) != false;
}

//---------------------------------------------------------------------------
inline void CP_Overflow_Flag( uint8_t Rd_, uint8_t Rr_, uint8_t Result_)
{
    stCPU.pstRAM->stRegisters.SREG.V =
             ( ((Rd_ & ~Rr_ & ~Result_)  & 0x80 )
             | ((~Rd_ & Rr_ & Result_) & 0x80 ) ) != 0;
}

//---------------------------------------------------------------------------
static void AVR_Opcode_CP( void )
{
    // Compare
    uint8_t u8Result;
    uint8_t u8Rd = *stCPU.Rd;
    uint8_t u8Rr  = *stCPU.Rr;

    u8Result = u8Rd - u8Rr;

    //---
    CP_Half_Carry(  u8Rd, u8Rr, u8Result );
    CP_Overflow_Flag(  u8Rd, u8Rr, u8Result );
    CP_Full_Carry(  u8Rd, u8Rr, u8Result );

    R8_Zero_Flag(  u8Result );
    R8_Negative_Flag(  u8Result );

    Signed_Flag();
}

//---------------------------------------------------------------------------
static void AVR_Opcode_CPC( void )
{
    // Compare with carry
    uint8_t u8Result;
    uint8_t u8Rd = *stCPU.Rd;
    uint8_t u8Rr  = *stCPU.Rr;
    uint8_t u8C = (stCPU.pstRAM->stRegisters.SREG.C == 1);

    u8Result = u8Rd - u8Rr - u8C;

    //---
    CP_Half_Carry(  u8Rd, u8Rr, u8Result );
    CP_Overflow_Flag(  u8Rd, u8Rr, u8Result );
    CP_Full_Carry(  u8Rd, u8Rr, u8Result );

    R8_CPC_Zero_Flag(  u8Result );
    R8_Negative_Flag(  u8Result );

    Signed_Flag();
}

//---------------------------------------------------------------------------
static void AVR_Opcode_CPI( void )
{
    // Compare with immediate
    uint8_t u8Result;
    uint8_t u8Rd = *stCPU.Rd;
    uint8_t u8K  = stCPU.K;

    u8Result = u8Rd - u8K;

    //---
    CP_Half_Carry(  u8Rd, u8K, u8Result );
    CP_Overflow_Flag(  u8Rd, u8K, u8Result );
    CP_Full_Carry(  u8Rd, u8K, u8Result );

    R8_Zero_Flag(  u8Result );
    R8_Negative_Flag(  u8Result );

    Signed_Flag();
}

//---------------------------------------------------------------------------
static void AVR_Opcode_SBRC( void )
{
    // Skip if Bit in IO register clear    
    if ((*stCPU.Rd & (1 << stCPU.b)) == 0)
    {
        uint8_t u8NextOpSize = AVR_Opcode_Size( stCPU.pu16ROM[ stCPU.u32PC + 1 ] );
        Relative_Jump(  u8NextOpSize + 1 );
    }
}

//---------------------------------------------------------------------------
static void AVR_Opcode_SBRS( void )
{
    // Skip if Bit in IO register set
    if ((*stCPU.Rd & (1 << stCPU.b)) != 0)
    {        
        uint8_t u8NextOpSize = AVR_Opcode_Size( stCPU.pu16ROM[ stCPU.u32PC + 1 ] );
        Relative_Jump(  u8NextOpSize + 1 );
    }
}

//---------------------------------------------------------------------------
static void AVR_Opcode_SBIC( void )
{
    // Skip if Bit in IO register clear
    uint8_t u8IOVal = Data_Read(  32 + stCPU.A );
    if ((u8IOVal & (1 << stCPU.b)) == 0)
    {
        uint8_t u8NextOpSize = AVR_Opcode_Size( stCPU.pu16ROM[ stCPU.u32PC + 1 ] );
        Relative_Jump(  u8NextOpSize + 1 );
    }
}

//---------------------------------------------------------------------------
static void AVR_Opcode_SBIS( void )
{
    // Skip if Bit in IO register set
    uint8_t u8IOVal = Data_Read(  32 + stCPU.A );
    if ((u8IOVal & (1 << stCPU.b)) != 0)
    {
        uint8_t u8NextOpSize = AVR_Opcode_Size( stCPU.pu16ROM[ stCPU.u32PC + 1 ] );
        Relative_Jump(  u8NextOpSize + 1 );
    }
}

//---------------------------------------------------------------------------
static inline Conditional_Branch( void )
{
    stCPU.u32PC = (uint16_t)((int16_t)stCPU.u32PC + stCPU.k_s + 1);
    stCPU.u16ExtraPC = 0;
    stCPU.u16ExtraCycles++;
}

//---------------------------------------------------------------------------
static void AVR_Opcode_BRBS( void )
{
    if (0 != (stCPU.pstRAM->stRegisters.SREG.r & (1 << stCPU.b)))
    {
        Conditional_Branch();
    }
}

//---------------------------------------------------------------------------
static void AVR_Opcode_BRBC( void )
{
    if (0 == (stCPU.pstRAM->stRegisters.SREG.r & (1 << stCPU.b)))
    {
        Conditional_Branch();
    }
}

//---------------------------------------------------------------------------
static void AVR_Opcode_BREQ( void )
{
    if (1 == stCPU.pstRAM->stRegisters.SREG.Z)
    {
        Conditional_Branch();
    }
}

//---------------------------------------------------------------------------
static void AVR_Opcode_BRNE( void )
{
    if (0 == stCPU.pstRAM->stRegisters.SREG.Z)
    {
        Conditional_Branch();
    }
}

//---------------------------------------------------------------------------
static void AVR_Opcode_BRCS( void )
{
    if (1 == stCPU.pstRAM->stRegisters.SREG.C)
    {
        Conditional_Branch();
    }
}

//---------------------------------------------------------------------------
static void AVR_Opcode_BRCC( void )
{
    if (0 == stCPU.pstRAM->stRegisters.SREG.C)
    {
        Conditional_Branch();
    }
}

//---------------------------------------------------------------------------
static void AVR_Opcode_BRSH( void )
{
    if (0 == stCPU.pstRAM->stRegisters.SREG.C)
    {
        Conditional_Branch();
    }
}

//---------------------------------------------------------------------------
static void AVR_Opcode_BRLO( void )
{
    if (1 == stCPU.pstRAM->stRegisters.SREG.C)
    {
        Conditional_Branch();
    }
}

//---------------------------------------------------------------------------
static void AVR_Opcode_BRMI( void )
{
    if (1 == stCPU.pstRAM->stRegisters.SREG.N)
    {
        Conditional_Branch();
    }
}

//---------------------------------------------------------------------------
static void AVR_Opcode_BRPL( void )
{
    if (0 == stCPU.pstRAM->stRegisters.SREG.N)
    {
        Conditional_Branch();
    }
}

//---------------------------------------------------------------------------
static void AVR_Opcode_BRGE( void )
{
    if (0 == stCPU.pstRAM->stRegisters.SREG.S)
    {
        Conditional_Branch();
    }
}

//---------------------------------------------------------------------------
static void AVR_Opcode_BRLT( void )
{
    if (1 == stCPU.pstRAM->stRegisters.SREG.S)
    {
        Conditional_Branch();
    }
}

//---------------------------------------------------------------------------
static void AVR_Opcode_BRHS( void )
{
    if (1 == stCPU.pstRAM->stRegisters.SREG.H)
    {
        Conditional_Branch();
    }
}

//---------------------------------------------------------------------------
static void AVR_Opcode_BRHC( void )
{
    if (0 == stCPU.pstRAM->stRegisters.SREG.H)
    {
        Conditional_Branch();
    }
}

//---------------------------------------------------------------------------
static void AVR_Opcode_BRTS( void )
{
    if (1 == stCPU.pstRAM->stRegisters.SREG.T)
    {
        Conditional_Branch();
    }
}

//---------------------------------------------------------------------------
static void AVR_Opcode_BRTC( void )
{
    if (0 == stCPU.pstRAM->stRegisters.SREG.T)
    {
        Conditional_Branch();
    }
}

//---------------------------------------------------------------------------
static void AVR_Opcode_BRVS( void )
{
    if (1 == stCPU.pstRAM->stRegisters.SREG.V)
    {
        Conditional_Branch();
    }
}

//---------------------------------------------------------------------------
static void AVR_Opcode_BRVC( void )
{
    if (0 == stCPU.pstRAM->stRegisters.SREG.V)
    {
        Conditional_Branch();
    }
}

//---------------------------------------------------------------------------
static void AVR_Opcode_BRIE( void )
{
    if (1 == stCPU.pstRAM->stRegisters.SREG.I)
    {
        Conditional_Branch();
    }
}

//---------------------------------------------------------------------------
static void AVR_Opcode_BRID( void )
{
    if (0 == stCPU.pstRAM->stRegisters.SREG.I)
    {
        Conditional_Branch();
    }
}

//---------------------------------------------------------------------------
static void AVR_Opcode_MOV( void )
{
    *stCPU.Rd = *stCPU.Rr;
}

//---------------------------------------------------------------------------
static void AVR_Opcode_MOVW( void )
{
    *stCPU.Rd16 = *stCPU.Rr16;
}

//---------------------------------------------------------------------------
static void AVR_Opcode_LDI( void )
{
    *stCPU.Rd = stCPU.K;
}

//---------------------------------------------------------------------------
static void AVR_Opcode_LDS( void )
{
    *stCPU.Rd = Data_Read(  stCPU.K );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_LD_X_Indirect( void )
{
    *stCPU.Rd =
            Data_Read(  stCPU.pstRAM->stRegisters.CORE_REGISTERS.X );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_LD_X_Indirect_Postinc( void )
{
    *stCPU.Rd =
        Data_Read(  stCPU.pstRAM->stRegisters.CORE_REGISTERS.X++ );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_LD_X_Indirect_Predec( void )
{
    *stCPU.Rd =
        Data_Read(  --stCPU.pstRAM->stRegisters.CORE_REGISTERS.X );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_LD_Y_Indirect( void )
{
    *stCPU.Rd =
        Data_Read(  stCPU.pstRAM->stRegisters.CORE_REGISTERS.Y );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_LD_Y_Indirect_Postinc( void )
{
    *stCPU.Rd =
        Data_Read(  stCPU.pstRAM->stRegisters.CORE_REGISTERS.Y++ );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_LD_Y_Indirect_Predec( void )
{
    *stCPU.Rd =
        Data_Read(  --stCPU.pstRAM->stRegisters.CORE_REGISTERS.Y );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_LDD_Y( void )
{
    *stCPU.Rd =
        Data_Read(  stCPU.pstRAM->stRegisters.CORE_REGISTERS.Y + stCPU.q );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_LD_Z_Indirect( void )
{
    *stCPU.Rd =
        Data_Read(Get_ZAddress());
}

//---------------------------------------------------------------------------
static void AVR_Opcode_LD_Z_Indirect_Postinc( void )
{
    *stCPU.Rd =
        Data_Read( Get_ZAddressPostInc() );

}

//---------------------------------------------------------------------------
static void AVR_Opcode_LD_Z_Indirect_Predec( void )
{
    *stCPU.Rd =
        Data_Read( Get_ZAddressPreDec() );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_LDD_Z( void )
{
    *stCPU.Rd =
        Data_Read( Get_ZAddress() + stCPU.q );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_STS( void )
{    
    Data_Write(  stCPU.K, *stCPU.Rd );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_ST_X_Indirect( void )
{
    Data_Write(  stCPU.pstRAM->stRegisters.CORE_REGISTERS.X, *stCPU.Rd );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_ST_X_Indirect_Postinc( void )
{
    Data_Write(   stCPU.pstRAM->stRegisters.CORE_REGISTERS.X++, *stCPU.Rd );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_ST_X_Indirect_Predec( void )
{
    Data_Write(  --stCPU.pstRAM->stRegisters.CORE_REGISTERS.X, *stCPU.Rd );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_ST_Y_Indirect( void )
{
    Data_Write(  stCPU.pstRAM->stRegisters.CORE_REGISTERS.Y, *stCPU.Rd );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_ST_Y_Indirect_Postinc( void )
{
    Data_Write(  stCPU.pstRAM->stRegisters.CORE_REGISTERS.Y++, *stCPU.Rd );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_ST_Y_Indirect_Predec( void )
{
    Data_Write(  --stCPU.pstRAM->stRegisters.CORE_REGISTERS.Y, *stCPU.Rd );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_STD_Y( void )
{
    Data_Write(  stCPU.pstRAM->stRegisters.CORE_REGISTERS.Y + stCPU.q, *stCPU.Rd );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_ST_Z_Indirect( void )
{
    Data_Write( Get_ZAddress(), *stCPU.Rd );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_ST_Z_Indirect_Postinc( void )
{
    Data_Write( Get_ZAddressPostInc() , *stCPU.Rd );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_ST_Z_Indirect_Predec( void )
{
    Data_Write( Get_ZAddressPreDec() , *stCPU.Rd );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_STD_Z( void )
{    
    Data_Write( Get_ZAddress() + stCPU.q, *stCPU.Rd );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_LPM( void )
{
    uint8_t u8Temp;
    if (Get_ZAddress() & 0x0001)
    {
        u8Temp = (uint8_t)(stCPU.pu16ROM[ Get_ZAddress() >> 1 ] >> 8);
    }
    else
    {
        u8Temp = (uint8_t)(stCPU.pu16ROM[ Get_ZAddress() >> 1 ] & 0x00FF);
    }

    stCPU.pstRAM->stRegisters.CORE_REGISTERS.r0 = u8Temp;
}

//---------------------------------------------------------------------------
static void AVR_Opcode_LPM_Z( void )
{
    uint8_t u8Temp;
    if (Get_ZAddress() & 0x0001)
    {
        u8Temp = (uint8_t)(stCPU.pu16ROM[ Get_ZAddress() >> 1 ] >> 8);
    }
    else
    {
        u8Temp = (uint8_t)(stCPU.pu16ROM[ Get_ZAddress() >> 1 ] & 0x00FF);
    }

    *stCPU.Rd = u8Temp;
}

//---------------------------------------------------------------------------
static void AVR_Opcode_LPM_Z_Postinc( void )
{
    uint8_t u8Temp;
    if (Get_ZAddress() & 0x0001)
    {
        u8Temp = (uint8_t)(stCPU.pu16ROM[ Get_ZAddressPostInc() >> 1 ] >> 8);
    }
    else
    {
        u8Temp = (uint8_t)(stCPU.pu16ROM[ Get_ZAddressPostInc() >> 1 ] & 0x00FF);
    }

    *stCPU.Rd = u8Temp;
}

//---------------------------------------------------------------------------
static void AVR_Opcode_ELPM( void )
{
    uint8_t u8Temp;
    if (Get_ZAddress() & 0x0001)
    {
        u8Temp = (uint8_t)(stCPU.pu16ROM[ Get_ZAddress() >> 1 ] >> 8);
    }
    else
    {
        u8Temp = (uint8_t)(stCPU.pu16ROM[ Get_ZAddress() >> 1 ] & 0x00FF);
    }

    stCPU.pstRAM->stRegisters.CORE_REGISTERS.r0 = u8Temp;
}

//---------------------------------------------------------------------------
static void AVR_Opcode_ELPM_Z( void )
{
    uint8_t u8Temp;
    if (Get_ZAddress() & 0x0001)
    {
        u8Temp = (uint8_t)(stCPU.pu16ROM[ Get_ZAddress() >> 1 ] >> 8);
    }
    else
    {
        u8Temp = (uint8_t)(stCPU.pu16ROM[ Get_ZAddress() >> 1 ] & 0x00FF);
    }

    *stCPU.Rd = u8Temp;
}

//---------------------------------------------------------------------------
static void AVR_Opcode_ELPM_Z_Postinc( void )
{
    uint8_t u8Temp;
    if (Get_ZAddress() & 0x0001)
    {
        u8Temp = (uint8_t)(stCPU.pu16ROM[ Get_ZAddressPostInc() >> 1 ] >> 8);
    }
    else
    {
        u8Temp = (uint8_t)(stCPU.pu16ROM[ Get_ZAddressPostInc() >> 1 ] & 0x00FF);
    }

    *stCPU.Rd = u8Temp;

}

//---------------------------------------------------------------------------
static void AVR_Opcode_SPM( void )
{
    //!! Implment later...
}

//---------------------------------------------------------------------------
static void AVR_Opcode_SPM_Z_Postinc2( void )
{
    //!! Implement later...
}

//---------------------------------------------------------------------------
static void AVR_Opcode_IN( void )
{    
    *stCPU.Rd = Data_Read(  32 + stCPU.A );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_OUT( void )
{         
     Data_Write(  32 + stCPU.A , *stCPU.Rd );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_PUSH( void )
{    
    uint32_t u32SP = (stCPU.pstRAM->stRegisters.SPL.r) |
                     ((uint32_t)(stCPU.pstRAM->stRegisters.SPH.r) << 8);

    // Store contents from SP to destination register
    Data_Write(  u32SP, *stCPU.Rd );

    // Postdecrement the SP
    u32SP--;

    // Update the SP registers
    stCPU.pstRAM->stRegisters.SPH.r = (uint8_t)(u32SP >> 8);
    stCPU.pstRAM->stRegisters.SPL.r = (uint8_t)(u32SP & 0x00FF);
}

//---------------------------------------------------------------------------
static void AVR_Opcode_POP( void )
{
    // Preincrement the SP
    uint32_t u32SP = (stCPU.pstRAM->stRegisters.SPL.r) |
                     ((uint16_t)(stCPU.pstRAM->stRegisters.SPH.r) << 8);
    u32SP++;

    // Load contents from SP to destination register
    *stCPU.Rd = Data_Read(  u32SP );

    // Update the SP registers
    stCPU.pstRAM->stRegisters.SPH.r = (uint8_t)(u32SP >> 8);
    stCPU.pstRAM->stRegisters.SPL.r = (uint8_t)(u32SP & 0x00FF);
}

//---------------------------------------------------------------------------
static void AVR_Opcode_XCH( void )
{
    uint8_t u8Z;
    uint8_t u8Temp;
    uint32_t u32Addr = Get_ZAddress();

    u8Z = Data_Read(  u32Addr );
    u8Temp = *stCPU.Rd;

    *stCPU.Rd = u8Z;
    Data_Write(  u32Addr, u8Temp );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_LAS( void )
{
    uint8_t u8Z;
    uint8_t u8Temp;

    uint32_t u32Addr = Get_ZAddress();

    u8Z = Data_Read(  u32Addr );
    u8Temp = *stCPU.Rd | u8Z;

    *stCPU.Rd = u8Z;    
    Data_Write(  u32Addr, u8Temp );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_LAC( void )
{
    uint8_t u8Z;
    uint8_t u8Temp;

    uint32_t u32Addr = Get_ZAddress();

    u8Z = Data_Read(  u32Addr );
    u8Temp = *stCPU.Rd & ~(u8Z);
    *stCPU.Rd = u8Z;

    Data_Write(  u32Addr, u8Temp );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_LAT( void )
{
    uint8_t u8Z;
    uint8_t u8Temp;

    uint32_t u32Addr = Get_ZAddress();

    u8Z = Data_Read(  u32Addr );
    u8Temp = *stCPU.Rd ^ u8Z;
    *stCPU.Rd = u8Z;

    Data_Write(  u32Addr, u8Temp );
}

//---------------------------------------------------------------------------
inline void LSL_HalfCarry_Flag( uint8_t R_ )
{
    stCPU.pstRAM->stRegisters.SREG.H = ((R_ & 0x08) == 0x08);
}

//---------------------------------------------------------------------------
inline void Left_Carry_Flag( uint8_t R_  )
{
    stCPU.pstRAM->stRegisters.SREG.C = ((R_ & 0x80) == 0x80);
}

//---------------------------------------------------------------------------
inline void Rotate_Overflow_Flag()
{
    stCPU.pstRAM->stRegisters.SREG.V = ( stCPU.pstRAM->stRegisters.SREG.N ^ stCPU.pstRAM->stRegisters.SREG.C );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_LSL( void )
{
    // Logical shift left
    uint8_t u8Result = 0;
    uint8_t u8Temp = *stCPU.Rd;

    u8Result = (u8Temp << 1);
    *stCPU.Rd = u8Result;

    // ---- Update flags ----
    LSL_HalfCarry_Flag(  u8Result);
    Left_Carry_Flag( u8Temp);

    R8_Negative_Flag(  u8Result );
    R8_Zero_Flag(  u8Result );
    Rotate_Overflow_Flag();
    Signed_Flag();
}

//---------------------------------------------------------------------------
inline void Right_Carry_Flag( uint8_t R_  )
{
    stCPU.pstRAM->stRegisters.SREG.C = ((R_ & 0x01) == 0x01);
}

//---------------------------------------------------------------------------
static void AVR_Opcode_LSR( void )
{
    // Logical shift left
    uint8_t u8Result = 0;
    uint8_t u8Temp = *stCPU.Rd;

    u8Result = (u8Temp >> 1);
    *stCPU.Rd = u8Result;

    // ---- Update flags ----
    Right_Carry_Flag(  u8Temp );
    stCPU.pstRAM->stRegisters.SREG.N = 0;
    R8_Zero_Flag(  u8Result );
    Rotate_Overflow_Flag();
    Signed_Flag();
}

//---------------------------------------------------------------------------
static void AVR_Opcode_ROL( void )
{
    // Rotate left through carry
    uint8_t u8Result = 0;
    uint8_t u8Temp = *stCPU.Rd;

    u8Result = (u8Temp << 1);
    if (stCPU.pstRAM->stRegisters.SREG.C)
    {
        u8Result |= 0x01;
    }
    *stCPU.Rd = u8Result;

    // ---- Update flags ----
    Left_Carry_Flag(  u8Temp );
    R8_Negative_Flag(  u8Result );
    R8_Zero_Flag(  u8Result );
    Rotate_Overflow_Flag();
    Signed_Flag();
}

//---------------------------------------------------------------------------
static void AVR_Opcode_ROR( void )
{
    // Rotate right through carry
    uint8_t u8Result = 0;
    uint8_t u8Temp = *stCPU.Rd;

    u8Result = (u8Temp >> 1);
    if (stCPU.pstRAM->stRegisters.SREG.C)
    {
        u8Result |= 0x80;
    }
    *stCPU.Rd = u8Result;

    // ---- Update flags ----
    Right_Carry_Flag(  u8Temp );
    R8_Negative_Flag(  u8Result );
    R8_Zero_Flag(  u8Result );
    Rotate_Overflow_Flag();
    Signed_Flag();
}

//---------------------------------------------------------------------------
static void AVR_Opcode_ASR( void )
{
    // Shift all bits to the right, keeping sign bit intact
    uint8_t u8Result;
    uint8_t u8Temp = *stCPU.Rd;
    u8Result = (u8Temp & 0x80) | (u8Temp >> 1);
    *stCPU.Rd = u8Result;

    // ---- Update flags ----
    Right_Carry_Flag(  u8Temp );
    R8_Negative_Flag(  u8Result );
    R8_Zero_Flag(  u8Result );
    Rotate_Overflow_Flag();
    Signed_Flag();
}

//---------------------------------------------------------------------------
static void AVR_Opcode_SWAP( void )
{
    uint8_t u8temp;
    u8temp = ((*stCPU.Rd) >> 4) |
             ((*stCPU.Rd) << 4) ;

    *stCPU.Rd = u8temp;
}

//---------------------------------------------------------------------------
static void AVR_Opcode_BSET( void )
{
    stCPU.pstRAM->stRegisters.SREG.r |= (1 << stCPU.b);
}

//---------------------------------------------------------------------------
static void AVR_Opcode_BCLR( void )
{
    stCPU.pstRAM->stRegisters.SREG.r &= ~(1 << stCPU.b);
}

//---------------------------------------------------------------------------
static void AVR_Opcode_SBI( void )
{
    uint8_t u8Temp = Data_Read(  stCPU.A + 32 );
    u8Temp |= (1 << stCPU.b);
    Data_Write(  stCPU.A + 32, u8Temp );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_CBI( void )
{
    uint8_t u8Temp = Data_Read(  stCPU.A + 32 );
    u8Temp &= ~(1 << stCPU.b);
    Data_Write(  stCPU.A + 32, u8Temp );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_BST( void )
{
    if ((*stCPU.Rd) & (1 << stCPU.b))
    {
        stCPU.pstRAM->stRegisters.SREG.T = 1;
    }
    else
    {
        stCPU.pstRAM->stRegisters.SREG.T = 0;
    }
}

//---------------------------------------------------------------------------
static void AVR_Opcode_BLD( void )
{
    if (stCPU.pstRAM->stRegisters.SREG.T)
    {
        *(stCPU.Rd) |= (1 << stCPU.b);
    }
    else
    {
        *(stCPU.Rd) &= ~(1 << stCPU.b);
    }
}

//---------------------------------------------------------------------------
static void AVR_Opcode_BREAK( void )
{
    // Unimplemented - since this requires debugging HW...
}

//---------------------------------------------------------------------------
static void AVR_Opcode_SLEEP( void )
{    
    stCPU.bAsleep = true;
}

//---------------------------------------------------------------------------
static void AVR_Opcode_WDR( void )
{
    stCPU.u32WDTCount = 0;   // Reset watchdog timer counter
}

//---------------------------------------------------------------------------
AVR_Opcode AVR_Opcode_Function( uint16_t OP_ )
{
    switch (OP_)
    {
    case 0x0000: return AVR_Opcode_NOP;

    case 0x9409: return AVR_Opcode_IJMP;
    case 0x9419: return AVR_Opcode_EIJMP;

    case 0x9508: return AVR_Opcode_RET;
    case 0x9509: return AVR_Opcode_ICALL;
    case 0x9518: return AVR_Opcode_RETI;
    case 0x9519: return AVR_Opcode_EICALL;
    case 0x9588: return AVR_Opcode_SLEEP;
    case 0x9598: return AVR_Opcode_BREAK;
    case 0x95A8: return AVR_Opcode_WDR;
    case 0x95C8: return AVR_Opcode_LPM;
    case 0x95D8: return AVR_Opcode_ELPM;
    case 0x95E8: return AVR_Opcode_SPM;
    case 0x95F8: return AVR_Opcode_SPM_Z_Postinc2;
    }

    switch( OP_ & 0xFF8F)
    {
    case 0x9408: return AVR_Opcode_BSET;
    case 0x9488: return AVR_Opcode_BCLR;
    }

    switch (OP_ & 0xFF88)
    {
    case 0x0300: return AVR_Opcode_MULSU;
    case 0x0308: return AVR_Opcode_FMUL;
    case 0x0380: return AVR_Opcode_FMULS;
    case 0x0388: return AVR_Opcode_FMULSU;
    }

    switch (OP_ & 0xFF0F)
    {
    case 0x940B: return AVR_Opcode_DES;
    case 0xEF0F: return AVR_Opcode_SER;
    }

    switch (OP_ & 0xFF00)
    {
    case 0x0100: return AVR_Opcode_MOVW;
    case 0x9600: return AVR_Opcode_ADIW;
    case 0x9700: return AVR_Opcode_SBIW;

    case 0x9800: return AVR_Opcode_CBI;
    case 0x9900: return AVR_Opcode_SBIC;
    case 0x9A00: return AVR_Opcode_SBI;
    case 0x9B00: return AVR_Opcode_SBIS;
    }

    switch (OP_ & 0xFE0F)
    {
    case 0x8008: return AVR_Opcode_LD_Y_Indirect;
    case 0x8000: return AVR_Opcode_LD_Z_Indirect;
    case 0x8200: return AVR_Opcode_ST_Z_Indirect;
    case 0x8208: return AVR_Opcode_ST_Y_Indirect;

    // -- Single 5-bit register...
    case 0x9000: return AVR_Opcode_LDS;
    case 0x9001: return AVR_Opcode_LD_Z_Indirect_Postinc;
    case 0x9002: return AVR_Opcode_LD_Z_Indirect_Predec;
    case 0x9004: return AVR_Opcode_LPM_Z;
    case 0x9005: return AVR_Opcode_LPM_Z_Postinc;
    case 0x9006: return AVR_Opcode_ELPM_Z;
    case 0x9007: return AVR_Opcode_ELPM_Z_Postinc;
    case 0x9009: return AVR_Opcode_LD_Y_Indirect_Postinc;
    case 0x900A: return AVR_Opcode_LD_Y_Indirect_Predec;
    case 0x900C: return AVR_Opcode_LD_X_Indirect;
    case 0x900D: return AVR_Opcode_LD_X_Indirect_Postinc;
    case 0x900E: return AVR_Opcode_LD_X_Indirect_Predec;
    case 0x900F: return AVR_Opcode_POP;

    case 0x9200: return AVR_Opcode_STS;
    case 0x9201: return AVR_Opcode_ST_Z_Indirect_Postinc;
    case 0x9202: return AVR_Opcode_ST_Z_Indirect_Predec;
    case 0x9204: return AVR_Opcode_XCH;
    case 0x9205: return AVR_Opcode_LAS;
    case 0x9206: return AVR_Opcode_LAC;
    case 0x9207: return AVR_Opcode_LAT;
    case 0x9209: return AVR_Opcode_ST_Y_Indirect_Postinc;
    case 0x920A: return AVR_Opcode_ST_Y_Indirect_Predec;
    case 0x920C: return AVR_Opcode_ST_X_Indirect;
    case 0x920D: return AVR_Opcode_ST_X_Indirect_Postinc;
    case 0x920E: return AVR_Opcode_ST_X_Indirect_Predec;
    case 0x920F: return AVR_Opcode_PUSH;

    // -- One-operand instructions
    case 0x9400: return AVR_Opcode_COM;
    case 0x9401: return AVR_Opcode_NEG;
    case 0x9402: return AVR_Opcode_SWAP;
    case 0x9403: return AVR_Opcode_INC;
    case 0x9405: return AVR_Opcode_ASR;
    case 0x9406: return AVR_Opcode_LSR;
    case 0x9407: return AVR_Opcode_ROR;
    case 0x940A: return AVR_Opcode_DEC;

    }
    switch (OP_ & 0xFE0E)
    {
    case 0x940C: return AVR_Opcode_JMP;
    case 0x940E: return AVR_Opcode_CALL;
    }

    switch (OP_ & 0xFE08)
    {

    // -- BLD/BST Encoding
    case 0xF800: return AVR_Opcode_BLD;
    case 0xFA00: return AVR_Opcode_BST;
    // -- SBRC/SBRS Encoding
    case 0xFC00: return AVR_Opcode_SBRC;
    case 0xFE00: return AVR_Opcode_SBRS;
    }

    switch (OP_ & 0xFC07)
    {
    // -- Conditional branches
    case 0xF000: return AVR_Opcode_BRCS;
    // case 0xF000: return AVR_Opcode_BRLO;            // AKA AVR_Opcode_BRCS;
    case 0xF001: return AVR_Opcode_BREQ;
    case 0xF002: return AVR_Opcode_BRMI;
    case 0xF003: return AVR_Opcode_BRVS;
    case 0xF004: return AVR_Opcode_BRLT;
    case 0xF006: return AVR_Opcode_BRTS;
    case 0xF007: return AVR_Opcode_BRIE;
    case 0xF400: return AVR_Opcode_BRCC;
    // case 0xF400: return AVR_Opcode_BRSH;            // AKA AVR_Opcode_BRCC;
    case 0xF401: return AVR_Opcode_BRNE;
    case 0xF402: return AVR_Opcode_BRPL;
    case 0xF403: return AVR_Opcode_BRVC;
    case 0xF404: return AVR_Opcode_BRGE;
    case 0xF405: return AVR_Opcode_BRHC;
    case 0xF406: return AVR_Opcode_BRTC;
    case 0xF407: return AVR_Opcode_BRID;
    }

    switch (OP_ & 0xFC00)
    {
    // -- 4-bit register pair
    case 0x0200: return AVR_Opcode_MULS;

    // -- 5-bit register pairs --
    case 0x0400: return AVR_Opcode_CPC;
    case 0x0800: return AVR_Opcode_SBC;
    case 0x0C00: return AVR_Opcode_ADD;
    // case 0x0C00: return AVR_Opcode_LSL; (!! Implemented with: " add rd, rd"
    case 0x1000: return AVR_Opcode_CPSE;
    case 0x1300: return AVR_Opcode_ROL;
    case 0x1400: return AVR_Opcode_CP;
    case 0x1C00: return AVR_Opcode_ADC;
    case 0x1800: return AVR_Opcode_SUB;
    case 0x2000: return AVR_Opcode_AND;
    // case 0x2000: return AVR_Opcode_TST; (!! Implemented with: " and rd, rd"
    case 0x2400: return AVR_Opcode_EOR;
    case 0x2C00: return AVR_Opcode_MOV;
    case 0x2800: return AVR_Opcode_OR;

    // -- 5-bit register pairs -- Destination = R1:R0
    case 0x9C00: return AVR_Opcode_MUL;
    }

    switch (OP_ & 0xF800)
    {
    case  0xB800: return AVR_Opcode_OUT;
    case  0xB000: return AVR_Opcode_IN;
    }

    switch (OP_ & 0xF000)
    {
    // -- Register immediate --
    case 0x3000: return AVR_Opcode_CPI;
    case 0x4000: return AVR_Opcode_SBCI;
    case 0x5000: return AVR_Opcode_SUBI;
    case 0x6000: return AVR_Opcode_ORI;// return AVR_Opcode_SBR;
    case 0x7000: return AVR_Opcode_ANDI;

    //-- 12-bit immediate
    case 0xC000: return AVR_Opcode_RJMP;
    case 0xD000: return AVR_Opcode_RCALL;

    // -- Register immediate
    case 0xE000: return AVR_Opcode_LDI;
    }

    switch (OP_ & 0xD208)
    {
    // -- 7-bit signed offset
    case 0x8000: return AVR_Opcode_LDD_Z;
    case 0x8008: return AVR_Opcode_LDD_Y;
    case 0x8200: return AVR_Opcode_STD_Z;
    case 0x8208: return AVR_Opcode_STD_Y;
    }

    return AVR_Opcode_NOP;
}

//---------------------------------------------------------------------------
void AVR_RunOpcode(  uint16_t OP_ )
{
    AVR_Opcode myOpcode = AVR_Opcode_Function( OP_);
    myOpcode();
}
