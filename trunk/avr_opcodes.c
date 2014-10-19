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
  \file  avr_opcodes.c

  \brief AVR CPU - Opcode implementation
*/


#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "emu_config.h"

#include "avr_opcodes.h"
//---------------------------------------------------------------------------
#define DEBUG_PRINT(...)

//---------------------------------------------------------------------------
static void Data_Write( AVR_CPU *pstCPU_, uint16_t u16Addr_, uint8_t u8Val_ )
{
    // Writing to RAM can be a tricky deal, because the address space is shared
    // between RAM, the core registers, and a bunch of peripheral I/O registers.
    DEBUG_PRINT("Write: 0x%04X=%02X\n", u16Addr_, u8Val_ );
    // Check to see if the write operation falls within the peripheral I/O range
    if (u16Addr_ >= 32 && u16Addr_ <= 255)
    {
        // I/O range - check to see if there's a peripheral installed at this address
        IOWriterList *pstIOWrite = pstCPU_->apstPeriphWriteTable[ u16Addr_ ];

        // If there is a peripheral or peripherals
        if (pstIOWrite)
        {
            // Iterate through the list of installed peripherals at this address, and
            // call their write handler
            while (pstIOWrite)
            {
                pstIOWrite->pfWriter( pstIOWrite->pvContext, pstCPU_, (uint8_t)u16Addr_, u8Val_ );
                pstIOWrite = pstIOWrite->next;
            }
        }
        // Otherwise, there is no peripheral -- just assume we can treat this as normal RAM.
        else
        {
            pstCPU_->pstRAM->au8RAM[ u16Addr_ ] = u8Val_;
        }
    }
    // RAM address range - direct write-through.
    else
    {
        pstCPU_->pstRAM->au8RAM[ u16Addr_ ] = u8Val_;
    }
}

//---------------------------------------------------------------------------
static uint8_t Data_Read( AVR_CPU *pstCPU_, uint16_t u16Addr_)
{
    // Writing to RAM can be a tricky deal, because the address space is shared
    // between RAM, the core registers, and a bunch of peripheral I/O registers.

    // Check to see if the write operation falls within the peripheral I/O range
    DEBUG_PRINT( "Data Read: %04X\n", u16Addr_ );
    if (u16Addr_ >= 32 && u16Addr_ <= 255)
    {
        // I/O range - check to see if there's a peripheral installed at this address
        IOReaderList *pstIORead = pstCPU_->apstPeriphReadTable[ u16Addr_ ];
        DEBUG_PRINT( "Peripheral Read: 0x%04X\n", u16Addr_ );
        // If there is a peripheral or peripherals
        if (pstIORead)
        {
            DEBUG_PRINT(" Found peripheral\n");
            // Iterate through the list of installed peripherals at this address, and
            // call their read handler
            uint8_t u8Val;
            while (pstIORead)
            {
                pstIORead->pfReader( pstIORead->pvContext, pstCPU_, (uint8_t)u16Addr_, &u8Val);
                pstIORead = pstIORead->next;
            }
            return u8Val;
        }
        // Otherwise, there is no peripheral -- just assume we can treat this as normal RAM.
        else
        {
            DEBUG_PRINT(" No peripheral\n");
            return pstCPU_->pstRAM->au8RAM[ u16Addr_ ];
        }
    }
    // RAM address range - direct read
    else
    {
        return pstCPU_->pstRAM->au8RAM[ u16Addr_ ];
    }
}

//---------------------------------------------------------------------------
static void AVR_Opcode_NOP( AVR_CPU *pstCPU_ )
{
    // Nop - do nothing.
}

//---------------------------------------------------------------------------
inline void ADD_Half_Carry( AVR_CPU *pstCPU_, uint8_t Rd_, uint8_t Rr_, uint8_t Result_)
{
    pstCPU_->pstRAM->stRegisters.SREG.H =
            ( ((Rd_ & Rr_)    & 0x08 )
            | ((Rr_ & (~Result_)) & 0x08 )
            | (((~Result_) & Rd_) & 0x08) ) != false;
}

//---------------------------------------------------------------------------
inline void ADD_Full_Carry( AVR_CPU *pstCPU_, uint8_t Rd_, uint8_t Rr_, uint8_t Result_)
{
    pstCPU_->pstRAM->stRegisters.SREG.C =
            ( ((Rd_ & Rr_)    & 0x80 )
            | ((Rr_ & (~Result_)) & 0x80 )
            | (((~Result_) & Rd_) & 0x80) ) != false;
}

//---------------------------------------------------------------------------
inline void ADD_Overflow_Flag( AVR_CPU *pstCPU_, uint8_t Rd_, uint8_t Rr_, uint8_t Result_)
{
    pstCPU_->pstRAM->stRegisters.SREG.V =
             ( ((Rd_ & Rr_ & ~Result_)  & 0x80 )
             | ((~Rd_ & ~Rr_ & Result_) & 0x80 ) ) != 0;
}

//---------------------------------------------------------------------------
inline void Signed_Flag( AVR_CPU *pstCPU_ )
{
    unsigned int N = pstCPU_->pstRAM->stRegisters.SREG.N;
    unsigned int V = pstCPU_->pstRAM->stRegisters.SREG.V;

    pstCPU_->pstRAM->stRegisters.SREG.S = N ^ V;
}

//---------------------------------------------------------------------------
inline void R8_Zero_Flag( AVR_CPU *pstCPU_, uint8_t R_ )
{
    pstCPU_->pstRAM->stRegisters.SREG.Z = (R_ == 0);
}

//---------------------------------------------------------------------------
inline void R8_CPC_Zero_Flag( AVR_CPU *pstCPU_, uint8_t R_ )
{
    pstCPU_->pstRAM->stRegisters.SREG.Z = (pstCPU_->pstRAM->stRegisters.SREG.Z && (R_ == 0));
}

//---------------------------------------------------------------------------
inline void R8_Negative_Flag( AVR_CPU *pstCPU_, uint8_t R_ )
{
    pstCPU_->pstRAM->stRegisters.SREG.N = ((R_ & 0x80) == 0x80);
}

//---------------------------------------------------------------------------
static void AVR_Opcode_ADD( AVR_CPU *pstCPU_ )
{
    uint8_t u8Result;
    uint8_t u8Rd = *(pstCPU_->Rd);
    uint8_t u8Rr = *(pstCPU_->Rr);

    u8Result = u8Rd + u8Rr;
    *(pstCPU_->Rd) = u8Result;

// ---- Update flags ----
    ADD_Half_Carry( pstCPU_, u8Rd, u8Rr, u8Result );
    ADD_Full_Carry( pstCPU_, u8Rd, u8Rr, u8Result );
    ADD_Overflow_Flag( pstCPU_, u8Rd, u8Rr, u8Result );
    R8_Negative_Flag( pstCPU_, u8Result );
    R8_Zero_Flag( pstCPU_, u8Result );
    Signed_Flag( pstCPU_ );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_ADC( AVR_CPU *pstCPU_ )
{
    uint8_t u8Result;
    uint8_t u8Rd = *(pstCPU_->Rd);
    uint8_t u8Rr = *(pstCPU_->Rr);
    uint8_t u8Carry = (pstCPU_->pstRAM->stRegisters.SREG.C);

    u8Result = u8Rd + u8Rr + u8Carry;
    *(pstCPU_->Rd) = u8Result;

// ---- Update flags ----
    ADD_Half_Carry( pstCPU_, u8Rd, u8Rr, u8Result );
    ADD_Full_Carry( pstCPU_, u8Rd, u8Rr, u8Result );
    ADD_Overflow_Flag( pstCPU_, u8Rd, u8Rr, u8Result );
    R8_Negative_Flag( pstCPU_, u8Result );
    R8_Zero_Flag( pstCPU_, u8Result );
    Signed_Flag( pstCPU_ );
}

//---------------------------------------------------------------------------
inline void R16_Negative_Flag( AVR_CPU *pstCPU_, uint16_t Result_ )
{
    pstCPU_->pstRAM->stRegisters.SREG.N =
            ((Result_ & 0x8000) != 0);
}

//---------------------------------------------------------------------------
inline void R16_Zero_Flag( AVR_CPU *pstCPU_, uint16_t Result_ )
{
    pstCPU_->pstRAM->stRegisters.SREG.Z =
            (Result_ == 0);
}

//---------------------------------------------------------------------------
inline void ADIW_Overflow_Flag( AVR_CPU *pstCPU_, uint16_t Rd_, uint16_t Result_ )
{
    pstCPU_->pstRAM->stRegisters.SREG.V =
            (((Rd_ & 0x8000) == 0) && ((Result_ & 0x8000) == 0x8000));
}

//---------------------------------------------------------------------------
inline void ADIW_Carry_Flag( AVR_CPU *pstCPU_, uint16_t Rd_, uint16_t Result_ )
{
    pstCPU_->pstRAM->stRegisters.SREG.C =
            (((Rd_ & 0x8000) == 0x8000) && ((Result_ & 0x8000) == 0));
}

//---------------------------------------------------------------------------
static void AVR_Opcode_ADIW( AVR_CPU *pstCPU_ )
{
    uint16_t u16K = (pstCPU_->K);
    uint16_t u16Rd = *(pstCPU_->Rd16);
    uint16_t u16Result;

    u16Result = u16Rd + u16K;
    *(pstCPU_->Rd16) = u16Result;

// ---- Update Flags ----   
    ADIW_Carry_Flag( pstCPU_, u16Rd, u16Result);
    ADIW_Overflow_Flag( pstCPU_, u16Rd, u16Result );
    R16_Negative_Flag( pstCPU_, u16Result );
    R16_Zero_Flag( pstCPU_, u16Result );
    Signed_Flag( pstCPU_ );
}

//---------------------------------------------------------------------------
inline void SUB_Overflow_Flag( AVR_CPU *pstCPU_, uint8_t Rd_, uint8_t Rr_, uint8_t Result_)
{
    pstCPU_->pstRAM->stRegisters.SREG.V =
             ( ((Rd_ & ~Rr_ & ~Result_) & 0x80 )
             | ((~Rd_ & Rr_ & Result_) & 0x80 ) ) != 0;
}
//---------------------------------------------------------------------------
inline void SUB_Half_Carry( AVR_CPU *pstCPU_, uint8_t Rd_, uint8_t Rr_, uint8_t Result_)
{
    pstCPU_->pstRAM->stRegisters.SREG.H =
             ( ((~Rd_ & Rr_) & 0x08 )
             | ((Rr_ & Result_) & 0x08 )
             | ((Result_ & ~Rd_) & 0x08 ) ) == 0x08;
}
//---------------------------------------------------------------------------
inline void SUB_Full_Carry( AVR_CPU *pstCPU_, uint8_t Rd_, uint8_t Rr_, uint8_t Result_)
{
    pstCPU_->pstRAM->stRegisters.SREG.C =
             ( ((~Rd_ & Rr_) & 0x80 )
             | ((Rr_ & Result_) & 0x80 )
             | ((Result_ & ~Rd_) & 0x80 ) ) == 0x80;
}

//---------------------------------------------------------------------------
static void AVR_Opcode_SUB( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = *pstCPU_->Rd;
    uint8_t u8Rr = *pstCPU_->Rr;
    uint8_t u8Result = u8Rd - u8Rr;

    *pstCPU_->Rd = u8Result;

    //--Flags
    SUB_Half_Carry(pstCPU_, u8Rd, u8Rr, u8Result);
    SUB_Full_Carry(pstCPU_, u8Rd, u8Rr, u8Result);
    SUB_Overflow_Flag(pstCPU_, u8Rd, u8Rr, u8Result);
    R8_Negative_Flag(pstCPU_, u8Result);
    R8_Zero_Flag(pstCPU_, u8Result);
    Signed_Flag(pstCPU_);
}

//---------------------------------------------------------------------------
static void AVR_Opcode_SUBI( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = *pstCPU_->Rd;
    uint8_t u8K = (uint8_t)pstCPU_->K;
    uint8_t u8Result = u8Rd - u8K;

    *pstCPU_->Rd = u8Result;

    //--Flags
    SUB_Half_Carry(pstCPU_, u8Rd, u8K, u8Result);
    SUB_Full_Carry(pstCPU_, u8Rd, u8K, u8Result);
    SUB_Overflow_Flag(pstCPU_, u8Rd, u8K, u8Result);
    R8_Negative_Flag(pstCPU_, u8Result);
    R8_Zero_Flag(pstCPU_, u8Result);
    Signed_Flag(pstCPU_);
}

//---------------------------------------------------------------------------
static void AVR_Opcode_SBC( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = *pstCPU_->Rd;
    uint8_t u8Rr = *pstCPU_->Rr;
    uint8_t u8C = pstCPU_->pstRAM->stRegisters.SREG.C;
    uint8_t u8Result = u8Rd - u8Rr - u8C;

    *pstCPU_->Rd = u8Result;

    //--Flags
    SUB_Half_Carry(pstCPU_, u8Rd, u8Rr, u8Result);
    SUB_Full_Carry(pstCPU_, u8Rd, u8Rr, u8Result);
    SUB_Overflow_Flag(pstCPU_, u8Rd, u8Rr, u8Result);
    R8_Negative_Flag(pstCPU_, u8Result);
    if (u8Result)
    {
        pstCPU_->pstRAM->stRegisters.SREG.Z = 0;
    }
    Signed_Flag(pstCPU_);
}

//---------------------------------------------------------------------------
static void AVR_Opcode_SBCI( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = *pstCPU_->Rd;
    uint8_t u8K = (uint8_t)pstCPU_->K;
    uint8_t u8C = pstCPU_->pstRAM->stRegisters.SREG.C;
    uint8_t u8Result = u8Rd - u8K - u8C;

    *pstCPU_->Rd = u8Result;

    //--Flags
    SUB_Half_Carry(pstCPU_, u8Rd, u8K, u8Result);
    SUB_Full_Carry(pstCPU_, u8Rd, u8K, u8Result);
    SUB_Overflow_Flag(pstCPU_, u8Rd, u8K, u8Result);
    R8_Negative_Flag(pstCPU_, u8Result);
    if (u8Result)
    {
        pstCPU_->pstRAM->stRegisters.SREG.Z = 0;
    }
    Signed_Flag(pstCPU_);
}


//---------------------------------------------------------------------------
inline void SBIW_Overflow_Flag( AVR_CPU *pstCPU_, uint16_t Rd_, uint16_t Result_)
{
    pstCPU_->pstRAM->stRegisters.SREG.V =
             ((Rd_ & 0x8000 ) == 0x8000) && ((Result_ & 0x8000) == 0);

}

//---------------------------------------------------------------------------
inline void SBIW_Full_Carry( AVR_CPU *pstCPU_, uint16_t Rd_, uint16_t Result_)
{
    pstCPU_->pstRAM->stRegisters.SREG.C =
             ((Rd_ & 0x8000 ) == 0) && ((Result_ & 0x8000) == 0x8000);
}

//---------------------------------------------------------------------------
static void AVR_Opcode_SBIW( AVR_CPU *pstCPU_ )
{
    uint16_t u16Rd = *pstCPU_->Rd16;
    uint16_t u16Result;

    //fprintf( stderr, "SBIW: RD=[%4X], K=[%2X]\n", u16Rd, pstCPU_->K );
    u16Result = u16Rd - pstCPU_->K;

    *pstCPU_->Rd16 = u16Result;
    //fprintf( stderr, "  Result=[%4X]\n", u16Result );

    SBIW_Full_Carry(pstCPU_, u16Rd, u16Result);
    SBIW_Overflow_Flag(pstCPU_, u16Rd, u16Result);
    R16_Negative_Flag(pstCPU_, u16Result);
    R16_Zero_Flag(pstCPU_, u16Result);
    Signed_Flag(pstCPU_);

}

//---------------------------------------------------------------------------
static void AVR_Opcode_AND( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = *pstCPU_->Rd;
    uint8_t u8Rr = *pstCPU_->Rr;
    uint8_t u8Result = u8Rd & u8Rr;

    *pstCPU_->Rd = u8Result;

    //--Update Status registers;
    pstCPU_->pstRAM->stRegisters.SREG.V = 0;
    R8_Negative_Flag( pstCPU_, u8Result );
    R8_Zero_Flag( pstCPU_, u8Result );
    Signed_Flag( pstCPU_ );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_ANDI( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = *pstCPU_->Rd;
    uint8_t u8Result = u8Rd & (uint8_t)pstCPU_->K;

    *pstCPU_->Rd = u8Result;

    //--Update Status registers;
    pstCPU_->pstRAM->stRegisters.SREG.V = 0;
    R8_Negative_Flag( pstCPU_, u8Result );
    R8_Zero_Flag( pstCPU_, u8Result );
    Signed_Flag( pstCPU_ );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_OR( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = *pstCPU_->Rd;
    uint8_t u8Rr = *pstCPU_->Rr;
    uint8_t u8Result = u8Rd | u8Rr;

    *pstCPU_->Rd = u8Result;

    //--Update Status registers;
    pstCPU_->pstRAM->stRegisters.SREG.V = 0;
    R8_Negative_Flag( pstCPU_, u8Result );
    R8_Zero_Flag( pstCPU_, u8Result );
    Signed_Flag( pstCPU_ );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_ORI( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = *pstCPU_->Rd;
    uint8_t u8Result = u8Rd | (uint8_t)pstCPU_->K;

    *pstCPU_->Rd = u8Result;

    //--Update Status registers;
    pstCPU_->pstRAM->stRegisters.SREG.V = 0;
    R8_Negative_Flag( pstCPU_, u8Result );
    R8_Zero_Flag( pstCPU_, u8Result );
    Signed_Flag( pstCPU_ );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_EOR( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = *pstCPU_->Rd;
    uint8_t u8Rr = *pstCPU_->Rr;
    uint8_t u8Result = u8Rd ^ u8Rr;

    *pstCPU_->Rd = u8Result;

    //--Update Status registers;
    pstCPU_->pstRAM->stRegisters.SREG.V = 0;
    R8_Negative_Flag( pstCPU_, u8Result );
    R8_Zero_Flag( pstCPU_, u8Result );
    Signed_Flag( pstCPU_ );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_COM( AVR_CPU *pstCPU_ )
{
    // 1's complement.
    uint8_t u8Result = *pstCPU_->Rd;
    u8Result = (0xFF - u8Result);

    *pstCPU_->Rd = u8Result;

    //--Update Status registers;
    pstCPU_->pstRAM->stRegisters.SREG.V = 0;
    pstCPU_->pstRAM->stRegisters.SREG.C = 1;
    R8_Negative_Flag( pstCPU_, u8Result );
    R8_Zero_Flag( pstCPU_, u8Result );
    Signed_Flag( pstCPU_ );
}

//---------------------------------------------------------------------------
inline void NEG_Overflow_Flag( AVR_CPU *pstCPU_, uint8_t u8Result_ )
{
    pstCPU_->pstRAM->stRegisters.SREG.V = (u8Result_ == 0x80);
}

//---------------------------------------------------------------------------
inline void NEG_Carry_Flag( AVR_CPU *pstCPU_ , uint8_t u8Result_ )
{
    pstCPU_->pstRAM->stRegisters.SREG.C = (u8Result_ != 0x00);
}

//---------------------------------------------------------------------------
static void AVR_Opcode_NEG( AVR_CPU *pstCPU_ )
{
    // 2's complement.
    uint8_t u8Result = *pstCPU_->Rd;
    u8Result = (0 - u8Result);

    *pstCPU_->Rd = u8Result;

    //--Update Status registers;
    NEG_Overflow_Flag( pstCPU_, u8Result );
    NEG_Carry_Flag( pstCPU_, u8Result );
    R8_Negative_Flag( pstCPU_, u8Result );
    R8_Zero_Flag( pstCPU_, u8Result );
    Signed_Flag( pstCPU_ );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_SBR( AVR_CPU *pstCPU_ )
{
    // Set Bits in Register
    uint8_t u8Result = *pstCPU_->Rd;
    u8Result |= ((uint8_t)pstCPU_->K);

    *pstCPU_->Rd = u8Result;

    //--Update Status registers;
    pstCPU_->pstRAM->stRegisters.SREG.V = 0;
    R8_Negative_Flag( pstCPU_, u8Result );
    R8_Zero_Flag( pstCPU_, u8Result );
    Signed_Flag( pstCPU_ );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_CBR( AVR_CPU *pstCPU_ )
{
    // Clear Bits in Register
    uint8_t u8Result = *pstCPU_->Rd;
    u8Result &= ~((uint8_t)pstCPU_->K);

    *pstCPU_->Rd = u8Result;

    //--Update Status registers;
    pstCPU_->pstRAM->stRegisters.SREG.V = 0;
    R8_Negative_Flag( pstCPU_, u8Result );
    R8_Zero_Flag( pstCPU_, u8Result );
    Signed_Flag( pstCPU_ );
}

//---------------------------------------------------------------------------
inline void INC_Overflow_Flag( AVR_CPU *pstCPU_, uint8_t u8Result_ )
{
    pstCPU_->pstRAM->stRegisters.SREG.V = (u8Result_ == 0x80);
}

//---------------------------------------------------------------------------
static void AVR_Opcode_INC( AVR_CPU *pstCPU_ )
{
    uint8_t u8Result;
    u8Result = *pstCPU_->Rd + 1;

    *pstCPU_->Rd = u8Result;

    //--Update Status registers;
    INC_Overflow_Flag( pstCPU_, u8Result );
    R8_Negative_Flag( pstCPU_, u8Result );
    R8_Zero_Flag( pstCPU_, u8Result );
    Signed_Flag( pstCPU_ );
}
//---------------------------------------------------------------------------
inline void DEC_Overflow_Flag( AVR_CPU *pstCPU_, uint8_t u8Result_ )
{
    pstCPU_->pstRAM->stRegisters.SREG.V = (u8Result_ == 0x7F);
}
//---------------------------------------------------------------------------
static void AVR_Opcode_DEC( AVR_CPU *pstCPU_ )
{   
    uint8_t u8Result;   
    u8Result = *pstCPU_->Rd - 1;

    *pstCPU_->Rd = u8Result;

    //--Update Status registers;
    DEC_Overflow_Flag( pstCPU_, u8Result );
    R8_Negative_Flag( pstCPU_, u8Result );
    R8_Zero_Flag( pstCPU_, u8Result );
    Signed_Flag( pstCPU_ );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_SER( AVR_CPU *pstCPU_ )
{
    *pstCPU_->Rd = 0xFF;
}

//---------------------------------------------------------------------------
inline void Mul_Carry_Flag( AVR_CPU *pstCPU_, uint16_t R_ )
{
    pstCPU_->pstRAM->stRegisters.SREG.C = ((R_  & 0x8000) == 0x8000);
}

//---------------------------------------------------------------------------
inline void Mul_Zero_Flag( AVR_CPU *pstCPU_, uint16_t R_ )
{
    pstCPU_->pstRAM->stRegisters.SREG.Z = (R_ == 0);
}

//---------------------------------------------------------------------------
static void AVR_Opcode_MUL( AVR_CPU *pstCPU_ )
{
    uint16_t u16Product;
    uint16_t u16R1;
    uint16_t u16R2;

    u16R1 = *pstCPU_->Rd;
    u16R2 = *pstCPU_->Rr;

    u16Product = u16R1 * u16R2;

    pstCPU_->pstRAM->stRegisters.CORE_REGISTERS.r1_0 = u16Product;

    //-- Update Flags --
    Mul_Zero_Flag(pstCPU_, u16Product);
    Mul_Carry_Flag(pstCPU_, u16Product);
}

//---------------------------------------------------------------------------
static void AVR_Opcode_MULS( AVR_CPU *pstCPU_ )
{
    int16_t s16Product;
    int16_t s16R1;
    int16_t s16R2;

    s16R1 = (int8_t)*pstCPU_->Rd;
    s16R2 = (int8_t)*pstCPU_->Rr;

    s16Product = s16R1 * s16R2;

    pstCPU_->pstRAM->stRegisters.CORE_REGISTERS.r1_0 = (uint16_t)s16Product;

    //-- Update Flags --
    Mul_Zero_Flag(pstCPU_, (uint16_t)s16Product);
    Mul_Carry_Flag(pstCPU_, (uint16_t)s16Product);
}

//---------------------------------------------------------------------------
static void AVR_Opcode_MULSU( AVR_CPU *pstCPU_ )
{
    int16_t s16Product;
    int16_t s16R1;
    uint16_t u16R2;

    s16R1 = (int8_t)*pstCPU_->Rd;
    u16R2 = *pstCPU_->Rr;

    s16Product = s16R1 * u16R2;

    pstCPU_->pstRAM->stRegisters.CORE_REGISTERS.r1_0 = (uint16_t)s16Product;

    //-- Update Flags --
    Mul_Zero_Flag(pstCPU_, (uint16_t)s16Product);
    Mul_Carry_Flag(pstCPU_, (uint16_t)s16Product);
}

//---------------------------------------------------------------------------
static void AVR_Opcode_FMUL( AVR_CPU *pstCPU_ )
{
    uint16_t u16Product;
    uint16_t u16R1;
    uint16_t u16R2;

    u16R1 = *pstCPU_->Rd;
    u16R2 = *pstCPU_->Rr;

    u16Product = u16R1 * u16R2;

    pstCPU_->pstRAM->stRegisters.CORE_REGISTERS.r1_0 = u16Product << 1;

    //-- Update Flags --
    Mul_Zero_Flag(pstCPU_, u16Product);
    Mul_Carry_Flag(pstCPU_, u16Product);
}

//---------------------------------------------------------------------------
static void AVR_Opcode_FMULS( AVR_CPU *pstCPU_ )
{
    int16_t s16Product;
    int16_t s16R1;
    int16_t s16R2;

    s16R1 = (int8_t)*pstCPU_->Rd;
    s16R2 = (int8_t)*pstCPU_->Rr;

    s16Product = s16R1 * s16R2;

    pstCPU_->pstRAM->stRegisters.CORE_REGISTERS.r1_0 = ((uint16_t)s16Product) << 1;

    //-- Update Flags --
    Mul_Zero_Flag(pstCPU_, (uint16_t)s16Product);
    Mul_Carry_Flag(pstCPU_, (uint16_t)s16Product);
}

//---------------------------------------------------------------------------
static void AVR_Opcode_FMULSU( AVR_CPU *pstCPU_ )
{
    int16_t s16Product;
    int16_t s16R1;
    uint16_t u16R2;

    s16R1 = (int8_t)*pstCPU_->Rd;
    u16R2 = *pstCPU_->Rr;

    s16Product = s16R1 * u16R2;

    pstCPU_->pstRAM->stRegisters.CORE_REGISTERS.r1_0 = ((uint16_t)s16Product) << 1;

    //-- Update Flags --
    Mul_Zero_Flag(pstCPU_, (uint16_t)s16Product);
    Mul_Carry_Flag(pstCPU_, (uint16_t)s16Product);
}

//---------------------------------------------------------------------------
static void AVR_Opcode_DES( AVR_CPU *pstCPU_ )
{
    //! ToDo - Implement DES
}

//---------------------------------------------------------------------------
static inline Unconditional_Jump( AVR_CPU *pstCPU_, uint16_t u16Addr_ )
{
    pstCPU_->u16PC = u16Addr_;
    pstCPU_->u16ExtraPC = 0;
}

//---------------------------------------------------------------------------
static inline Relative_Jump( AVR_CPU *pstCPU_, uint16_t u16Offset_ )
{
    // u16Offset_ Will always be 1 or 2, based on the size of the next opcode
    // in a program

    pstCPU_->u16PC += u16Offset_;
    pstCPU_->u16ExtraPC = 0;
    pstCPU_->u16ExtraCycles += u16Offset_;
}

//---------------------------------------------------------------------------
static void AVR_Opcode_RJMP( AVR_CPU *pstCPU_ )
{
    int32_t s32NewPC = (int32_t)pstCPU_->u16PC + (int32_t)pstCPU_->k_s + 1;

    Unconditional_Jump( pstCPU_, (uint16_t)s32NewPC );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_IJMP( AVR_CPU *pstCPU_ )
{
    Unconditional_Jump( pstCPU_, pstCPU_->pstRAM->stRegisters.CORE_REGISTERS.Z );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_EIJMP( AVR_CPU *pstCPU_ )
{
    //! ToDo - implement EIND instructions
}

//---------------------------------------------------------------------------
static void AVR_Opcode_JMP( AVR_CPU *pstCPU_ )
{
    Unconditional_Jump( pstCPU_, (uint16_t)pstCPU_->k );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_RCALL( AVR_CPU *pstCPU_ )
{
    // Push the next instruction address onto the stack
    uint16_t u16SP = (((uint16_t)pstCPU_->pstRAM->stRegisters.SPH.r) << 8) |
                     (((uint16_t)pstCPU_->pstRAM->stRegisters.SPL.r));

    uint16_t u16StoredPC = pstCPU_->u16PC + 1;

    Data_Write( pstCPU_, u16SP, (uint8_t)(u16StoredPC & 0x00FF));
    Data_Write( pstCPU_, u16SP - 1, (uint8_t)(u16StoredPC >> 8));

    // Stack is post-decremented
    u16SP -= 2;

    // Set the new PC (relative call)
    int32_t s32NewPC = (int32_t)pstCPU_->u16PC + (int32_t)pstCPU_->k_s + 1;
    uint16_t u16NewPC = (uint16_t)s32NewPC;

    // Store the new SP.
    pstCPU_->pstRAM->stRegisters.SPH.r = (u16SP >> 8);
    pstCPU_->pstRAM->stRegisters.SPL.r = (u16SP & 0x00FF);

    // Set the new PC
    Unconditional_Jump( pstCPU_, u16NewPC );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_ICALL( AVR_CPU *pstCPU_ )
{
    // Push the next instruction address onto the stack
    uint16_t u16SP = (((uint16_t)pstCPU_->pstRAM->stRegisters.SPH.r) << 8) |
                     (((uint16_t)pstCPU_->pstRAM->stRegisters.SPL.r));

    uint16_t u16StoredPC = pstCPU_->u16PC + 1;

    Data_Write( pstCPU_, u16SP, (uint8_t)(u16StoredPC & 0x00FF));
    Data_Write( pstCPU_, u16SP - 1, (uint8_t)(u16StoredPC >> 8));

    // Stack is post-decremented
    u16SP -= 2;

    // Set the new PC
    uint16_t u16NewPC = pstCPU_->pstRAM->stRegisters.CORE_REGISTERS.Z;

    // Store the new SP.
    pstCPU_->pstRAM->stRegisters.SPH.r = (u16SP >> 8);
    pstCPU_->pstRAM->stRegisters.SPL.r = (u16SP & 0x00FF);

    // Set the new PC
    Unconditional_Jump( pstCPU_, u16NewPC );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_EICALL( AVR_CPU *pstCPU_ )
{
    //!! ToDo - Implement EIND calling!
}

//---------------------------------------------------------------------------
static void AVR_Opcode_CALL( AVR_CPU *pstCPU_ )
{
    // See ICALL for documentation
    uint16_t u16SP = (((uint16_t)pstCPU_->pstRAM->stRegisters.SPH.r) << 8) |
                     (((uint16_t)pstCPU_->pstRAM->stRegisters.SPL.r));

    uint16_t u16StoredPC = pstCPU_->u16PC + 2;

    Data_Write( pstCPU_, u16SP, (uint8_t)(u16StoredPC & 0x00FF));
    Data_Write( pstCPU_, u16SP - 1, (uint8_t)(u16StoredPC >> 8));

    u16SP -= 2;

    uint16_t u16NewPC = pstCPU_->k;

    pstCPU_->pstRAM->stRegisters.SPH.r = (u16SP >> 8);
    pstCPU_->pstRAM->stRegisters.SPL.r = (u16SP & 0x00FF);

    Unconditional_Jump( pstCPU_, u16NewPC );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_RET( AVR_CPU *pstCPU_ )
{
    // Pop the next instruction off of the stack, pre-incrementing
    uint16_t u16SP = (((uint16_t)pstCPU_->pstRAM->stRegisters.SPH.r) << 8) |
                     (((uint16_t)pstCPU_->pstRAM->stRegisters.SPL.r));
    u16SP += 2;

    uint16_t u16High = Data_Read( pstCPU_, u16SP - 1 );
    uint16_t u16Low = Data_Read( pstCPU_, u16SP );
    uint16_t u16NewPC = (u16High << 8) | u16Low;

    pstCPU_->pstRAM->stRegisters.SPH.r = (u16SP >> 8);
    pstCPU_->pstRAM->stRegisters.SPL.r = (u16SP & 0x00FF);

    // Set new PC based on address read from stack
    Unconditional_Jump( pstCPU_, u16NewPC );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_RETI( AVR_CPU *pstCPU_ )
{
    uint16_t u16SP = (((uint16_t)pstCPU_->pstRAM->stRegisters.SPH.r) << 8) |
                     (((uint16_t)pstCPU_->pstRAM->stRegisters.SPL.r));
    u16SP += 2;

    uint16_t u16High = Data_Read( pstCPU_, u16SP - 1 );
    uint16_t u16Low = Data_Read( pstCPU_, u16SP );
    uint16_t u16NewPC = (u16High << 8) | u16Low;

    pstCPU_->pstRAM->stRegisters.SPH.r = (u16SP >> 8);
    pstCPU_->pstRAM->stRegisters.SPL.r = (u16SP & 0x00FF);

//-- Enable interrupts
    pstCPU_->pstRAM->stRegisters.SREG.I = 1;
    Unconditional_Jump( pstCPU_, u16NewPC );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_CPSE( AVR_CPU *pstCPU_ )
{    
    if (*pstCPU_->Rr == *pstCPU_->Rd)
    {        
        uint8_t u8NextOpSize = AVR_Opcode_Size( pstCPU_->pu16ROM[ pstCPU_->u16PC + 1 ] );
        Relative_Jump( pstCPU_, u8NextOpSize + 1 );
    }
}

//---------------------------------------------------------------------------
inline void CP_Half_Carry( AVR_CPU *pstCPU_, uint8_t Rd_, uint8_t Rr_, uint8_t Result_)
{
    pstCPU_->pstRAM->stRegisters.SREG.H =
            ( ((~Rd_ & Rr_)    & 0x08 )
            | ((Rr_ & (Result_)) & 0x08 )
            | (((Result_) & ~Rd_) & 0x08) ) != false;
}

//---------------------------------------------------------------------------
inline void CP_Full_Carry( AVR_CPU *pstCPU_, uint8_t Rd_, uint8_t Rr_, uint8_t Result_)
{
    pstCPU_->pstRAM->stRegisters.SREG.C =
            ( ((~Rd_ & Rr_)    & 0x80 )
            | ((Rr_ & (Result_)) & 0x80 )
            | (((Result_) & ~Rd_) & 0x80) ) != false;
}

//---------------------------------------------------------------------------
inline void CP_Overflow_Flag( AVR_CPU *pstCPU_, uint8_t Rd_, uint8_t Rr_, uint8_t Result_)
{
    pstCPU_->pstRAM->stRegisters.SREG.V =
             ( ((Rd_ & ~Rr_ & ~Result_)  & 0x80 )
             | ((~Rd_ & Rr_ & Result_) & 0x80 ) ) != 0;
}

//---------------------------------------------------------------------------
static void AVR_Opcode_CP( AVR_CPU *pstCPU_ )
{
    // Compare
    uint8_t u8Result;
    uint8_t u8Rd = *pstCPU_->Rd;
    uint8_t u8Rr  = *pstCPU_->Rr;

    u8Result = u8Rd - u8Rr;

    //---
    CP_Half_Carry( pstCPU_, u8Rd, u8Rr, u8Result );
    CP_Overflow_Flag( pstCPU_, u8Rd, u8Rr, u8Result );
    CP_Full_Carry( pstCPU_, u8Rd, u8Rr, u8Result );

    R8_Zero_Flag( pstCPU_, u8Result );
    R8_Negative_Flag( pstCPU_, u8Result );

    Signed_Flag( pstCPU_ );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_CPC( AVR_CPU *pstCPU_ )
{
    // Compare with carry
    uint8_t u8Result;
    uint8_t u8Rd = *pstCPU_->Rd;
    uint8_t u8Rr  = *pstCPU_->Rr;
    uint8_t u8C = (pstCPU_->pstRAM->stRegisters.SREG.C == 1);

    u8Result = u8Rd - u8Rr - u8C;

    //---
    CP_Half_Carry( pstCPU_, u8Rd, u8Rr, u8Result );
    CP_Overflow_Flag( pstCPU_, u8Rd, u8Rr, u8Result );
    CP_Full_Carry( pstCPU_, u8Rd, u8Rr, u8Result );

    R8_CPC_Zero_Flag( pstCPU_, u8Result );
    R8_Negative_Flag( pstCPU_, u8Result );

    Signed_Flag( pstCPU_ );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_CPI( AVR_CPU *pstCPU_ )
{
    // Compare with immediate
    uint8_t u8Result;
    uint8_t u8Rd = *pstCPU_->Rd;
    uint8_t u8K  = pstCPU_->K;

    u8Result = u8Rd - u8K;

    //---
    CP_Half_Carry( pstCPU_, u8Rd, u8K, u8Result );
    CP_Overflow_Flag( pstCPU_, u8Rd, u8K, u8Result );
    CP_Full_Carry( pstCPU_, u8Rd, u8K, u8Result );

    R8_Zero_Flag( pstCPU_, u8Result );
    R8_Negative_Flag( pstCPU_, u8Result );

    Signed_Flag( pstCPU_ );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_SBRC( AVR_CPU *pstCPU_ )
{
    // Skip if Bit in IO register clear    
    if ((*pstCPU_->Rd & (1 << pstCPU_->b)) == 0)
    {
        uint8_t u8NextOpSize = AVR_Opcode_Size( pstCPU_->pu16ROM[ pstCPU_->u16PC + 1 ] );
        Relative_Jump( pstCPU_, u8NextOpSize + 1 );
    }
}

//---------------------------------------------------------------------------
static void AVR_Opcode_SBRS( AVR_CPU *pstCPU_ )
{
    // Skip if Bit in IO register set
    if ((*pstCPU_->Rd & (1 << pstCPU_->b)) != 0)
    {        
        uint8_t u8NextOpSize = AVR_Opcode_Size( pstCPU_->pu16ROM[ pstCPU_->u16PC + 1 ] );
        Relative_Jump( pstCPU_, u8NextOpSize + 1 );
    }
}

//---------------------------------------------------------------------------
static void AVR_Opcode_SBIC( AVR_CPU *pstCPU_ )
{
    // Skip if Bit in IO register clear
    uint8_t u8IOVal = Data_Read( pstCPU_, 32 + pstCPU_->A );
    if ((u8IOVal & (1 << pstCPU_->b)) == 0)
    {
        uint8_t u8NextOpSize = AVR_Opcode_Size( pstCPU_->pu16ROM[ pstCPU_->u16PC + 1 ] );
        Relative_Jump( pstCPU_, u8NextOpSize + 1 );
    }
}

//---------------------------------------------------------------------------
static void AVR_Opcode_SBIS( AVR_CPU *pstCPU_ )
{
    // Skip if Bit in IO register set
    uint8_t u8IOVal = Data_Read( pstCPU_, 32 + pstCPU_->A );
    if ((u8IOVal & (1 << pstCPU_->b)) != 0)
    {
        uint8_t u8NextOpSize = AVR_Opcode_Size( pstCPU_->pu16ROM[ pstCPU_->u16PC + 1 ] );
        Relative_Jump( pstCPU_, u8NextOpSize + 1 );
    }
}

//---------------------------------------------------------------------------
static inline Conditional_Branch( AVR_CPU *pstCPU_ )
{
    pstCPU_->u16PC = (uint16_t)((int16_t)pstCPU_->u16PC + pstCPU_->k_s + 1);
    pstCPU_->u16ExtraPC = 0;
    pstCPU_->u16ExtraCycles++;
}

//---------------------------------------------------------------------------
static void AVR_Opcode_BRBS( AVR_CPU *pstCPU_ )
{
    if (0 != (pstCPU_->pstRAM->stRegisters.SREG.r & (1 << pstCPU_->b)))
    {
        Conditional_Branch( pstCPU_ );
    }
}

//---------------------------------------------------------------------------
static void AVR_Opcode_BRBC( AVR_CPU *pstCPU_ )
{
    if (0 == (pstCPU_->pstRAM->stRegisters.SREG.r & (1 << pstCPU_->b)))
    {
        Conditional_Branch( pstCPU_ );
    }
}

//---------------------------------------------------------------------------
static void AVR_Opcode_BREQ( AVR_CPU *pstCPU_ )
{
    if (1 == pstCPU_->pstRAM->stRegisters.SREG.Z)
    {
        Conditional_Branch( pstCPU_ );
    }
}

//---------------------------------------------------------------------------
static void AVR_Opcode_BRNE( AVR_CPU *pstCPU_ )
{
    if (0 == pstCPU_->pstRAM->stRegisters.SREG.Z)
    {
        Conditional_Branch( pstCPU_ );
    }
}

//---------------------------------------------------------------------------
static void AVR_Opcode_BRCS( AVR_CPU *pstCPU_ )
{
    if (1 == pstCPU_->pstRAM->stRegisters.SREG.C)
    {
        Conditional_Branch( pstCPU_ );
    }
}

//---------------------------------------------------------------------------
static void AVR_Opcode_BRCC( AVR_CPU *pstCPU_ )
{
    if (0 == pstCPU_->pstRAM->stRegisters.SREG.C)
    {
        Conditional_Branch( pstCPU_ );
    }
}

//---------------------------------------------------------------------------
static void AVR_Opcode_BRSH( AVR_CPU *pstCPU_ )
{
    if (0 == pstCPU_->pstRAM->stRegisters.SREG.C)
    {
        Conditional_Branch( pstCPU_ );
    }
}

//---------------------------------------------------------------------------
static void AVR_Opcode_BRLO( AVR_CPU *pstCPU_ )
{
    if (1 == pstCPU_->pstRAM->stRegisters.SREG.C)
    {
        Conditional_Branch( pstCPU_ );
    }
}

//---------------------------------------------------------------------------
static void AVR_Opcode_BRMI( AVR_CPU *pstCPU_ )
{
    if (1 == pstCPU_->pstRAM->stRegisters.SREG.N)
    {
        Conditional_Branch( pstCPU_ );
    }
}

//---------------------------------------------------------------------------
static void AVR_Opcode_BRPL( AVR_CPU *pstCPU_ )
{
    if (0 == pstCPU_->pstRAM->stRegisters.SREG.N)
    {
        Conditional_Branch( pstCPU_ );
    }
}

//---------------------------------------------------------------------------
static void AVR_Opcode_BRGE( AVR_CPU *pstCPU_ )
{
    if (0 == pstCPU_->pstRAM->stRegisters.SREG.S)
    {
        Conditional_Branch( pstCPU_ );
    }
}

//---------------------------------------------------------------------------
static void AVR_Opcode_BRLT( AVR_CPU *pstCPU_ )
{
    if (1 == pstCPU_->pstRAM->stRegisters.SREG.S)
    {
        Conditional_Branch( pstCPU_ );
    }
}

//---------------------------------------------------------------------------
static void AVR_Opcode_BRHS( AVR_CPU *pstCPU_ )
{
    if (1 == pstCPU_->pstRAM->stRegisters.SREG.H)
    {
        Conditional_Branch( pstCPU_ );
    }
}

//---------------------------------------------------------------------------
static void AVR_Opcode_BRHC( AVR_CPU *pstCPU_ )
{
    if (0 == pstCPU_->pstRAM->stRegisters.SREG.H)
    {
        Conditional_Branch( pstCPU_ );
    }
}

//---------------------------------------------------------------------------
static void AVR_Opcode_BRTS( AVR_CPU *pstCPU_ )
{
    if (1 == pstCPU_->pstRAM->stRegisters.SREG.T)
    {
        Conditional_Branch( pstCPU_ );
    }
}

//---------------------------------------------------------------------------
static void AVR_Opcode_BRTC( AVR_CPU *pstCPU_ )
{
    if (0 == pstCPU_->pstRAM->stRegisters.SREG.T)
    {
        Conditional_Branch( pstCPU_ );
    }
}

//---------------------------------------------------------------------------
static void AVR_Opcode_BRVS( AVR_CPU *pstCPU_ )
{
    if (1 == pstCPU_->pstRAM->stRegisters.SREG.V)
    {
        Conditional_Branch( pstCPU_ );
    }
}

//---------------------------------------------------------------------------
static void AVR_Opcode_BRVC( AVR_CPU *pstCPU_ )
{
    if (0 == pstCPU_->pstRAM->stRegisters.SREG.V)
    {
        Conditional_Branch( pstCPU_ );
    }
}

//---------------------------------------------------------------------------
static void AVR_Opcode_BRIE( AVR_CPU *pstCPU_ )
{
    if (1 == pstCPU_->pstRAM->stRegisters.SREG.I)
    {
        Conditional_Branch( pstCPU_ );
    }
}

//---------------------------------------------------------------------------
static void AVR_Opcode_BRID( AVR_CPU *pstCPU_ )
{
    if (0 == pstCPU_->pstRAM->stRegisters.SREG.I)
    {
        Conditional_Branch( pstCPU_ );
    }
}

//---------------------------------------------------------------------------
static void AVR_Opcode_MOV( AVR_CPU *pstCPU_ )
{
    *pstCPU_->Rd = *pstCPU_->Rr;
}

//---------------------------------------------------------------------------
static void AVR_Opcode_MOVW( AVR_CPU *pstCPU_ )
{
    *pstCPU_->Rd16 = *pstCPU_->Rr16;
}

//---------------------------------------------------------------------------
static void AVR_Opcode_LDI( AVR_CPU *pstCPU_ )
{
    *pstCPU_->Rd = pstCPU_->K;
}

//---------------------------------------------------------------------------
static void AVR_Opcode_LDS( AVR_CPU *pstCPU_ )
{
    *pstCPU_->Rd = Data_Read( pstCPU_, pstCPU_->K );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_LD_X_Indirect( AVR_CPU *pstCPU_ )
{
    *pstCPU_->Rd =
            Data_Read( pstCPU_, pstCPU_->pstRAM->stRegisters.CORE_REGISTERS.X );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_LD_X_Indirect_Postinc( AVR_CPU *pstCPU_ )
{
    *pstCPU_->Rd =
        Data_Read( pstCPU_, pstCPU_->pstRAM->stRegisters.CORE_REGISTERS.X++ );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_LD_X_Indirect_Predec( AVR_CPU *pstCPU_ )
{
    *pstCPU_->Rd =
        Data_Read( pstCPU_, --pstCPU_->pstRAM->stRegisters.CORE_REGISTERS.X );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_LD_Y_Indirect( AVR_CPU *pstCPU_ )
{
    *pstCPU_->Rd =
        Data_Read( pstCPU_, pstCPU_->pstRAM->stRegisters.CORE_REGISTERS.Y );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_LD_Y_Indirect_Postinc( AVR_CPU *pstCPU_ )
{
    *pstCPU_->Rd =
        Data_Read( pstCPU_, pstCPU_->pstRAM->stRegisters.CORE_REGISTERS.Y++ );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_LD_Y_Indirect_Predec( AVR_CPU *pstCPU_ )
{
    *pstCPU_->Rd =
        Data_Read( pstCPU_, --pstCPU_->pstRAM->stRegisters.CORE_REGISTERS.Y );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_LDD_Y( AVR_CPU *pstCPU_ )
{
    *pstCPU_->Rd =
        Data_Read( pstCPU_, pstCPU_->pstRAM->stRegisters.CORE_REGISTERS.Y + pstCPU_->q );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_LD_Z_Indirect( AVR_CPU *pstCPU_ )
{
    *pstCPU_->Rd =
        Data_Read( pstCPU_, pstCPU_->pstRAM->stRegisters.CORE_REGISTERS.Z );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_LD_Z_Indirect_Postinc( AVR_CPU *pstCPU_ )
{
    *pstCPU_->Rd =
        Data_Read( pstCPU_, pstCPU_->pstRAM->stRegisters.CORE_REGISTERS.Z++ );

}

//---------------------------------------------------------------------------
static void AVR_Opcode_LD_Z_Indirect_Predec( AVR_CPU *pstCPU_ )
{
    *pstCPU_->Rd =
        Data_Read( pstCPU_, --pstCPU_->pstRAM->stRegisters.CORE_REGISTERS.Z );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_LDD_Z( AVR_CPU *pstCPU_ )
{
    *pstCPU_->Rd =
        Data_Read( pstCPU_, pstCPU_->pstRAM->stRegisters.CORE_REGISTERS.Z + pstCPU_->q );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_STS( AVR_CPU *pstCPU_ )
{    
    Data_Write( pstCPU_, pstCPU_->K, *pstCPU_->Rd );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_ST_X_Indirect( AVR_CPU *pstCPU_ )
{
    Data_Write( pstCPU_, pstCPU_->pstRAM->stRegisters.CORE_REGISTERS.X, *pstCPU_->Rd );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_ST_X_Indirect_Postinc( AVR_CPU *pstCPU_ )
{
    Data_Write( pstCPU_,  pstCPU_->pstRAM->stRegisters.CORE_REGISTERS.X++, *pstCPU_->Rd );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_ST_X_Indirect_Predec( AVR_CPU *pstCPU_ )
{
    Data_Write( pstCPU_, --pstCPU_->pstRAM->stRegisters.CORE_REGISTERS.X, *pstCPU_->Rd );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_ST_Y_Indirect( AVR_CPU *pstCPU_ )
{
    Data_Write( pstCPU_, pstCPU_->pstRAM->stRegisters.CORE_REGISTERS.Y, *pstCPU_->Rd );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_ST_Y_Indirect_Postinc( AVR_CPU *pstCPU_ )
{
    Data_Write( pstCPU_, pstCPU_->pstRAM->stRegisters.CORE_REGISTERS.Y++, *pstCPU_->Rd );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_ST_Y_Indirect_Predec( AVR_CPU *pstCPU_ )
{
    Data_Write( pstCPU_, --pstCPU_->pstRAM->stRegisters.CORE_REGISTERS.Y, *pstCPU_->Rd );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_STD_Y( AVR_CPU *pstCPU_ )
{
    Data_Write( pstCPU_, pstCPU_->pstRAM->stRegisters.CORE_REGISTERS.Y + pstCPU_->q, *pstCPU_->Rd );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_ST_Z_Indirect( AVR_CPU *pstCPU_ )
{
    Data_Write( pstCPU_, pstCPU_->pstRAM->stRegisters.CORE_REGISTERS.Z, *pstCPU_->Rd );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_ST_Z_Indirect_Postinc( AVR_CPU *pstCPU_ )
{
    Data_Write( pstCPU_, pstCPU_->pstRAM->stRegisters.CORE_REGISTERS.Z++ , *pstCPU_->Rd );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_ST_Z_Indirect_Predec( AVR_CPU *pstCPU_ )
{
    Data_Write( pstCPU_, --pstCPU_->pstRAM->stRegisters.CORE_REGISTERS.Z , *pstCPU_->Rd );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_STD_Z( AVR_CPU *pstCPU_ )
{    
    Data_Write( pstCPU_, pstCPU_->pstRAM->stRegisters.CORE_REGISTERS.Z + pstCPU_->q, *pstCPU_->Rd );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_LPM( AVR_CPU *pstCPU_ )
{
    uint8_t u8Temp;
    if (pstCPU_->pstRAM->stRegisters.CORE_REGISTERS.Z & 0x0001)
    {
        u8Temp = (uint8_t)(pstCPU_->pu16ROM[ pstCPU_->pstRAM->stRegisters.CORE_REGISTERS.Z >> 1 ] >> 8);
    }
    else
    {
        u8Temp = (uint8_t)(pstCPU_->pu16ROM[ pstCPU_->pstRAM->stRegisters.CORE_REGISTERS.Z >> 1 ] & 0x00FF);
    }

    pstCPU_->pstRAM->stRegisters.CORE_REGISTERS.r0 = u8Temp;
}

//---------------------------------------------------------------------------
static void AVR_Opcode_LPM_Z( AVR_CPU *pstCPU_ )
{
    uint8_t u8Temp;
    if (pstCPU_->pstRAM->stRegisters.CORE_REGISTERS.Z & 0x0001)
    {
        u8Temp = (uint8_t)(pstCPU_->pu16ROM[ pstCPU_->pstRAM->stRegisters.CORE_REGISTERS.Z >> 1 ] >> 8);
    }
    else
    {
        u8Temp = (uint8_t)(pstCPU_->pu16ROM[ pstCPU_->pstRAM->stRegisters.CORE_REGISTERS.Z >> 1 ] & 0x00FF);
    }

    *pstCPU_->Rd = u8Temp;
}

//---------------------------------------------------------------------------
static void AVR_Opcode_LPM_Z_Postinc( AVR_CPU *pstCPU_ )
{
    uint8_t u8Temp;
    if (pstCPU_->pstRAM->stRegisters.CORE_REGISTERS.Z & 0x0001)
    {
        u8Temp = (uint8_t)(pstCPU_->pu16ROM[ pstCPU_->pstRAM->stRegisters.CORE_REGISTERS.Z >> 1 ] >> 8);
    }
    else
    {
        u8Temp = (uint8_t)(pstCPU_->pu16ROM[ pstCPU_->pstRAM->stRegisters.CORE_REGISTERS.Z >> 1 ] & 0x00FF);
    }

    *pstCPU_->Rd = u8Temp;
    pstCPU_->pstRAM->stRegisters.CORE_REGISTERS.Z++;
}

//---------------------------------------------------------------------------
static void AVR_Opcode_ELPM( AVR_CPU *pstCPU_ )
{
    //!! ToDo - Add in RAMPZ register.
    uint8_t u8Temp;
    if (pstCPU_->pstRAM->stRegisters.CORE_REGISTERS.Z & 0x0001)
    {
        u8Temp = (uint8_t)(pstCPU_->pu16ROM[ pstCPU_->pstRAM->stRegisters.CORE_REGISTERS.Z >> 1 ] >> 8);
    }
    else
    {
        u8Temp = (uint8_t)(pstCPU_->pu16ROM[ pstCPU_->pstRAM->stRegisters.CORE_REGISTERS.Z >> 1 ] & 0x00FF);
    }

    pstCPU_->pstRAM->stRegisters.CORE_REGISTERS.r0 = u8Temp;
}

//---------------------------------------------------------------------------
static void AVR_Opcode_ELPM_Z( AVR_CPU *pstCPU_ )
{
    uint8_t u8Temp;
    if (pstCPU_->pstRAM->stRegisters.CORE_REGISTERS.Z & 0x0001)
    {
        u8Temp = (uint8_t)(pstCPU_->pu16ROM[ pstCPU_->pstRAM->stRegisters.CORE_REGISTERS.Z >> 1 ] >> 8);
    }
    else
    {
        u8Temp = (uint8_t)(pstCPU_->pu16ROM[ pstCPU_->pstRAM->stRegisters.CORE_REGISTERS.Z >> 1 ] & 0x00FF);
    }

    *pstCPU_->Rd = u8Temp;
}

//---------------------------------------------------------------------------
static void AVR_Opcode_ELPM_Z_Postinc( AVR_CPU *pstCPU_ )
{
    uint8_t u8Temp;
    if (pstCPU_->pstRAM->stRegisters.CORE_REGISTERS.Z & 0x0001)
    {
        u8Temp = (uint8_t)(pstCPU_->pu16ROM[ pstCPU_->pstRAM->stRegisters.CORE_REGISTERS.Z >> 1 ] >> 8);
    }
    else
    {
        u8Temp = (uint8_t)(pstCPU_->pu16ROM[ pstCPU_->pstRAM->stRegisters.CORE_REGISTERS.Z >> 1 ] & 0x00FF);
    }

    *pstCPU_->Rd = u8Temp;

    pstCPU_->pstRAM->stRegisters.CORE_REGISTERS.Z++;
}

//---------------------------------------------------------------------------
static void AVR_Opcode_SPM( AVR_CPU *pstCPU_ )
{
    //!! Implment later...
}

//---------------------------------------------------------------------------
static void AVR_Opcode_SPM_Z_Postinc2( AVR_CPU *pstCPU_ )
{
    //!! Implement later...
}

//---------------------------------------------------------------------------
static void AVR_Opcode_IN( AVR_CPU *pstCPU_ )
{    
    *pstCPU_->Rd = Data_Read( pstCPU_, 32 + pstCPU_->A );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_OUT( AVR_CPU *pstCPU_ )
{         
     Data_Write( pstCPU_, 32 + pstCPU_->A , *pstCPU_->Rd );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_PUSH( AVR_CPU *pstCPU_ )
{    
    uint16_t u16SP = (pstCPU_->pstRAM->stRegisters.SPL.r) |
                     ((uint16_t)(pstCPU_->pstRAM->stRegisters.SPH.r) << 8);

    // Store contents from SP to destination register
    Data_Write( pstCPU_, u16SP, *pstCPU_->Rd );

    // Postdecrement the SP
    u16SP--;

    // Update the SP registers
    pstCPU_->pstRAM->stRegisters.SPH.r = (uint8_t)(u16SP >> 8);
    pstCPU_->pstRAM->stRegisters.SPL.r = (uint8_t)(u16SP & 0x00FF);
}

//---------------------------------------------------------------------------
static void AVR_Opcode_POP( AVR_CPU *pstCPU_ )
{
    // Preincrement the SP
    uint16_t u16SP = (pstCPU_->pstRAM->stRegisters.SPL.r) |
                     ((uint16_t)(pstCPU_->pstRAM->stRegisters.SPH.r) << 8);
    u16SP++;

    // Load contents from SP to destination register
    *pstCPU_->Rd = Data_Read( pstCPU_, u16SP );

    // Update the SP registers
    pstCPU_->pstRAM->stRegisters.SPH.r = (uint8_t)(u16SP >> 8);
    pstCPU_->pstRAM->stRegisters.SPL.r = (uint8_t)(u16SP & 0x00FF);
}

//---------------------------------------------------------------------------
static void AVR_Opcode_XCH( AVR_CPU *pstCPU_ )
{
    uint8_t u8Z;
    uint8_t u8Temp;
    uint16_t u16Addr = pstCPU_->pstRAM->stRegisters.CORE_REGISTERS.Z;

    u8Z = Data_Read( pstCPU_, u16Addr );
    u8Temp = *pstCPU_->Rd;

    *pstCPU_->Rd = u8Z;
    Data_Write( pstCPU_, u16Addr, u8Temp );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_LAS( AVR_CPU *pstCPU_ )
{
    uint8_t u8Z;
    uint8_t u8Temp;

    uint16_t u16Addr = pstCPU_->pstRAM->stRegisters.CORE_REGISTERS.Z;

    u8Z = Data_Read( pstCPU_, u16Addr );
    u8Temp = *pstCPU_->Rd | u8Z;

    *pstCPU_->Rd = u8Z;    
    Data_Write( pstCPU_, u16Addr, u8Temp );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_LAC( AVR_CPU *pstCPU_ )
{
    uint8_t u8Z;
    uint8_t u8Temp;

    uint16_t u16Addr = pstCPU_->pstRAM->stRegisters.CORE_REGISTERS.Z;

    u8Z = Data_Read( pstCPU_, u16Addr );    
    u8Temp = *pstCPU_->Rd & ~(u8Z);
    *pstCPU_->Rd = u8Z;

    Data_Write( pstCPU_, u16Addr, u8Temp );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_LAT( AVR_CPU *pstCPU_ )
{
    uint8_t u8Z;
    uint8_t u8Temp;

    uint16_t u16Addr = pstCPU_->pstRAM->stRegisters.CORE_REGISTERS.Z;

    u8Z = Data_Read( pstCPU_, u16Addr );
    u8Temp = *pstCPU_->Rd ^ u8Z;
    *pstCPU_->Rd = u8Z;

    Data_Write( pstCPU_, u16Addr, u8Temp );
}

//---------------------------------------------------------------------------
inline void LSL_HalfCarry_Flag( AVR_CPU *pstCPU_, uint8_t R_ )
{
    pstCPU_->pstRAM->stRegisters.SREG.H = ((R_ & 0x08) == 0x08);
}

//---------------------------------------------------------------------------
inline void Left_Carry_Flag( AVR_CPU *pstCPU_, uint8_t R_  )
{
    pstCPU_->pstRAM->stRegisters.SREG.C = ((R_ & 0x80) == 0x80);
}

//---------------------------------------------------------------------------
inline void Rotate_Overflow_Flag( AVR_CPU *pstCPU_  )
{
    pstCPU_->pstRAM->stRegisters.SREG.V = ( pstCPU_->pstRAM->stRegisters.SREG.N ^ pstCPU_->pstRAM->stRegisters.SREG.C );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_LSL( AVR_CPU *pstCPU_ )
{
    // Logical shift left
    uint8_t u8Result = 0;
    uint8_t u8Temp = *pstCPU_->Rd;

    u8Result = (u8Temp << 1);
    *pstCPU_->Rd = u8Result;

    // ---- Update flags ----
    LSL_HalfCarry_Flag( pstCPU_, u8Result);
    Left_Carry_Flag(pstCPU_, u8Temp);

    R8_Negative_Flag( pstCPU_, u8Result );
    R8_Zero_Flag( pstCPU_, u8Result );
    Rotate_Overflow_Flag( pstCPU_ );
    Signed_Flag( pstCPU_ );
}

//---------------------------------------------------------------------------
inline void Right_Carry_Flag( AVR_CPU *pstCPU_, uint8_t R_  )
{
    pstCPU_->pstRAM->stRegisters.SREG.C = ((R_ & 0x01) == 0x01);
}

//---------------------------------------------------------------------------
static void AVR_Opcode_LSR( AVR_CPU *pstCPU_ )
{
    // Logical shift left
    uint8_t u8Result = 0;
    uint8_t u8Temp = *pstCPU_->Rd;

    u8Result = (u8Temp >> 1);
    *pstCPU_->Rd = u8Result;

    // ---- Update flags ----
    Right_Carry_Flag( pstCPU_, u8Temp );
    pstCPU_->pstRAM->stRegisters.SREG.N = 0;
    R8_Zero_Flag( pstCPU_, u8Result );
    Rotate_Overflow_Flag( pstCPU_ );
    Signed_Flag( pstCPU_ );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_ROL( AVR_CPU *pstCPU_ )
{
    // Rotate left through carry
    uint8_t u8Result = 0;
    uint8_t u8Temp = *pstCPU_->Rd;

    u8Result = (u8Temp << 1);
    if (pstCPU_->pstRAM->stRegisters.SREG.C)
    {
        u8Result |= 0x01;
    }
    *pstCPU_->Rd = u8Result;

    // ---- Update flags ----
    Left_Carry_Flag( pstCPU_, u8Temp );
    R8_Negative_Flag( pstCPU_, u8Result );
    R8_Zero_Flag( pstCPU_, u8Result );
    Rotate_Overflow_Flag( pstCPU_ );
    Signed_Flag( pstCPU_ );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_ROR( AVR_CPU *pstCPU_ )
{
    // Rotate right through carry
    uint8_t u8Result = 0;
    uint8_t u8Temp = *pstCPU_->Rd;

    u8Result = (u8Temp >> 1);
    if (pstCPU_->pstRAM->stRegisters.SREG.C)
    {
        u8Result |= 0x80;
    }
    *pstCPU_->Rd = u8Result;

    // ---- Update flags ----
    Right_Carry_Flag( pstCPU_, u8Temp );
    R8_Negative_Flag( pstCPU_, u8Result );
    R8_Zero_Flag( pstCPU_, u8Result );
    Rotate_Overflow_Flag( pstCPU_ );
    Signed_Flag( pstCPU_ );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_ASR( AVR_CPU *pstCPU_ )
{
    // Shift all bits to the right, keeping sign bit intact
    uint8_t u8Result;
    uint8_t u8Temp = *pstCPU_->Rd;
    u8Result = (u8Temp & 0x80) | (u8Temp >> 1);
    *pstCPU_->Rd = u8Result;

    // ---- Update flags ----
    Right_Carry_Flag( pstCPU_, u8Temp );
    R8_Negative_Flag( pstCPU_, u8Result );
    R8_Zero_Flag( pstCPU_, u8Result );
    Rotate_Overflow_Flag( pstCPU_ );
    Signed_Flag( pstCPU_ );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_SWAP( AVR_CPU *pstCPU_ )
{
    uint8_t u8temp;
    u8temp = ((*pstCPU_->Rd) >> 4) |
             ((*pstCPU_->Rd) << 4) ;

    *pstCPU_->Rd = u8temp;
}

//---------------------------------------------------------------------------
static void AVR_Opcode_BSET( AVR_CPU *pstCPU_ )
{
    pstCPU_->pstRAM->stRegisters.SREG.r |= (1 << pstCPU_->b);
}

//---------------------------------------------------------------------------
static void AVR_Opcode_BCLR( AVR_CPU *pstCPU_ )
{
    pstCPU_->pstRAM->stRegisters.SREG.r &= ~(1 << pstCPU_->b);
}

//---------------------------------------------------------------------------
static void AVR_Opcode_SBI( AVR_CPU *pstCPU_ )
{
    uint8_t u8Temp = Data_Read( pstCPU_, pstCPU_->A + 32 );
    u8Temp |= (1 << pstCPU_->b);
    Data_Write( pstCPU_, pstCPU_->A + 32, u8Temp );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_CBI( AVR_CPU *pstCPU_ )
{
    uint8_t u8Temp = Data_Read( pstCPU_, pstCPU_->A + 32 );
    u8Temp &= ~(1 << pstCPU_->b);
    Data_Write( pstCPU_, pstCPU_->A + 32, u8Temp );
}

//---------------------------------------------------------------------------
static void AVR_Opcode_BST( AVR_CPU *pstCPU_ )
{
    if ((*pstCPU_->Rd) & (1 << pstCPU_->b))
    {
        pstCPU_->pstRAM->stRegisters.SREG.T = 1;
    }
    else
    {
        pstCPU_->pstRAM->stRegisters.SREG.T = 0;
    }
}

//---------------------------------------------------------------------------
static void AVR_Opcode_BLD( AVR_CPU *pstCPU_ )
{
    if (pstCPU_->pstRAM->stRegisters.SREG.T)
    {
        *(pstCPU_->Rd) |= (1 << pstCPU_->b);
    }
    else
    {
        *(pstCPU_->Rd) &= ~(1 << pstCPU_->b);
    }
}

//---------------------------------------------------------------------------
static void AVR_Opcode_BREAK( AVR_CPU *pstCPU_ )
{
    // Unimplemented - since this requires debugging HW...
}

//---------------------------------------------------------------------------
static void AVR_Opcode_SLEEP( AVR_CPU *pstCPU_ )
{    
    pstCPU_->bAsleep = true;
}

//---------------------------------------------------------------------------
static void AVR_Opcode_WDR( AVR_CPU *pstCPU_ )
{
    pstCPU_->u32WDTCount = 0;   // Reset watchdog timer counter
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
void AVR_RunOpcode( AVR_CPU *pstCPU_,  uint16_t OP_ )
{
    AVR_Opcode myOpcode = AVR_Opcode_Function( OP_);
    myOpcode( pstCPU_ );
}
