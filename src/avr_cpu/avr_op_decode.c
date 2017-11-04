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
  \file  avr_op_decode.c

  \brief Module providing logic to decode AVR CPU Opcodes.

  Implemented based on descriptions provided in Atmel document doc0856

*/

#include <stdint.h>

#include "emu_config.h"

#include "avr_op_decode.h"

//---------------------------------------------------------------------------
static void AVR_Decoder_NOP( uint16_t OP_)
{
    // Nothing to do here...
}
//---------------------------------------------------------------------------
static void AVR_Decoder_Register_Pair_4bit( uint16_t OP_)
{
    uint8_t Rr = (OP_ & 0x000F);
    uint8_t Rd = ((OP_ & 0x00F0) >> 4);

    stCPU.Rr16 = &(stCPU.pstRAM->stRegisters.CORE_REGISTERS.r_word[Rr]);
    stCPU.Rd16 = &(stCPU.pstRAM->stRegisters.CORE_REGISTERS.r_word[Rd]);
}
//---------------------------------------------------------------------------
static void AVR_Decoder_Register_Pair_3bit( uint16_t OP_)
{
    uint8_t Rr = (OP_ & 0x0007) + 16;
    uint8_t Rd = ((OP_ & 0x0070) >> 4) + 16;

    stCPU.Rr = &(stCPU.pstRAM->stRegisters.CORE_REGISTERS.r[Rr]);
    stCPU.Rd = &(stCPU.pstRAM->stRegisters.CORE_REGISTERS.r[Rd]);
}
//---------------------------------------------------------------------------
static void AVR_Decoder_Register_Pair_5bit( uint16_t OP_)
{
    uint8_t Rr = (OP_ & 0x000F) | ((OP_ & 0x0200) >> 5);
    uint8_t Rd = (OP_ & 0x01F0) >> 4;

    stCPU.Rr = &(stCPU.pstRAM->stRegisters.CORE_REGISTERS.r[Rr]);
    stCPU.Rd = &(stCPU.pstRAM->stRegisters.CORE_REGISTERS.r[Rd]);
}
//---------------------------------------------------------------------------
static void AVR_Decoder_Register_Immediate( uint16_t OP_)
{
    uint8_t K = (OP_ & 0x000F) | ((OP_ & 0x0F00) >> 4);
    uint8_t Rd = ((OP_ & 0x00F0) >> 4) + 16;

    stCPU.K = K;
    stCPU.Rd = &(stCPU.pstRAM->stRegisters.CORE_REGISTERS.r[Rd]);
}
//---------------------------------------------------------------------------
static void AVR_Decoder_LDST_YZ_k( uint16_t OP_)
{
    uint8_t q = (OP_ & 0x0007) |            // Awkward encoding... see manual for details.
                ((OP_ & 0x0C00) >> (7)) |
                ((OP_ & 0x2000) >> (8));

    uint8_t Rd = (OP_ & 0x01F0) >> 4;

    stCPU.q = q;
    stCPU.Rd = &(stCPU.pstRAM->stRegisters.CORE_REGISTERS.r[Rd]);
}
//---------------------------------------------------------------------------
static void AVR_Decoder_LDST( uint16_t OP_)
{
    uint8_t Rd = (OP_ & 0x01F0) >> 4;

    stCPU.Rd = &(stCPU.pstRAM->stRegisters.CORE_REGISTERS.r[Rd]);        
}
//---------------------------------------------------------------------------
static void AVR_Decoder_LDS_STS( uint16_t OP_)
{
    uint8_t Rd = (OP_ & 0x01F0) >> 4;

    stCPU.Rd = &(stCPU.pstRAM->stRegisters.CORE_REGISTERS.r[Rd]);
    stCPU.K = stCPU.pu16ROM[ stCPU.u32PC + 1 ];
}
//---------------------------------------------------------------------------
static void AVR_Decoder_Register_Single( uint16_t OP_)
{
    uint8_t Rd = (OP_ & 0x01F0) >> 4;

    stCPU.Rd = &(stCPU.pstRAM->stRegisters.CORE_REGISTERS.r[Rd]);
}
//---------------------------------------------------------------------------
static void AVR_Decoder_Register_SC( uint16_t OP_)
{
    uint8_t b = (OP_ & 0x0070) >> 4;

    stCPU.b = b;
}
//---------------------------------------------------------------------------
static void AVR_Decoder_Misc( uint16_t OP_)
{
    // Nothing to do here.
}
//---------------------------------------------------------------------------
static void AVR_Decoder_Indirect_Jump( uint16_t OP_)
{
    // Nothing to do here.
}
//---------------------------------------------------------------------------
static void AVR_Decoder_DEC_Rd( uint16_t OP_)
{
    uint8_t Rd = (OP_ & 0x01F0) >> 4;

    stCPU.Rd = &(stCPU.pstRAM->stRegisters.CORE_REGISTERS.r[Rd]);
}
//---------------------------------------------------------------------------
static void AVR_Decoder_DES_round_4( uint16_t OP_)
{
    uint8_t K = (OP_ & 0x00F0) >> 4;
    stCPU.K = K;
}
//---------------------------------------------------------------------------
static void AVR_Decoder_JMP_CALL_22( uint16_t OP_)
{
    uint16_t op = stCPU.pu16ROM[ stCPU.u32PC + 1 ];
    uint32_t k = op;
    k |= (((OP_ & 0x0001) | (OP_ & 0x01F0) >> 3) << 16);

    stCPU.k = k;

    // These are 2-cycle instructions.  Clock the CPU here, since we're fetching
    // the second word of data for this opcode here.
    IO_Clock();
}
//---------------------------------------------------------------------------
static void AVR_Decoder_ADIW_SBIW_6( uint16_t OP_)
{
    uint8_t K = (OP_ & 0x000F) | ((OP_ & 0x00C0) >> 2);
    uint8_t Rd16 = (((OP_ & 0x0030) >> 4) * 2) + 24;

    stCPU.K = K;
    stCPU.Rd16 = &(stCPU.pstRAM->stRegisters.CORE_REGISTERS.r_word[Rd16 >> 1]);
}
//---------------------------------------------------------------------------
static void AVR_Decoder_IO_Bit( uint16_t OP_)
{
    uint8_t b = (OP_ & 0x0007);
    uint8_t A = (OP_ & 0x00F8) >> 3;

    stCPU.b = b;
    stCPU.A = A;
}
//---------------------------------------------------------------------------
static void AVR_Decoder_MUL( uint16_t OP_)
{
    uint8_t Rr = (OP_ & 0x000F) | ((OP_ & 0x0200) >> 5);
    uint8_t Rd = (OP_ & 0x01F0) >> 4;

    stCPU.Rr = &(stCPU.pstRAM->stRegisters.CORE_REGISTERS.r[Rr]);
    stCPU.Rd = &(stCPU.pstRAM->stRegisters.CORE_REGISTERS.r[Rd]);
}
//---------------------------------------------------------------------------
static void AVR_Decoder_IO_In_Out( uint16_t OP_)
{
    uint8_t A = (OP_ & 0x000F) | ((OP_ & 0x0600) >> 5);
    uint8_t Rd = (OP_ & 0x01F0) >> 4;

    stCPU.A = A;
    stCPU.Rd = &(stCPU.pstRAM->stRegisters.CORE_REGISTERS.r[Rd]);
}
//---------------------------------------------------------------------------
static void AVR_Decoder_Relative_Jump( uint16_t OP_)
{
    // NB: -2K <= k <= 2K
    uint16_t k = (OP_ & 0x0FFF);

    // Check for sign bit in 12-bit value...
    if (k & 0x0800)
    {
        stCPU.k_s = (int32_t)((~k & 0x07FF) + 1) * -1;
    }
    else
    {
        stCPU.k_s = (int32_t)k;
    }
}
//---------------------------------------------------------------------------
static void AVR_Decoder_LDI( uint16_t OP_)
{
    uint8_t K = (OP_ & 0x000F) | ((OP_ & 0x0F00) >> 4);
    uint8_t Rd = ((OP_ & 0x00F0) >> 4) + 16;

    stCPU.K = K;
    stCPU.Rd = &(stCPU.pstRAM->stRegisters.CORE_REGISTERS.r[Rd]);
}
//---------------------------------------------------------------------------
static void AVR_Decoder_Conditional_Branch( uint16_t OP_)
{
    // NB: -64 <= k <= 63
    uint8_t b = (OP_ & 0x0007);
    uint8_t k = ((OP_ & 0x03F8) >> 3);

    stCPU.b = b;

    // Check for sign bit in 7-bit value...
    if (k & 0x40)
    {
        // Convert to signed 32-bit integer... probably a cleaner way
        // of doing this, but I'm tired.
        stCPU.k_s = (int32_t)((~k & 0x3F) + 1) * -1;
    }
    else
    {
        stCPU.k_s = (int32_t)k;
    }
}
//---------------------------------------------------------------------------
static void AVR_Decoder_BLD_BST( uint16_t OP_)
{
    uint8_t b = (OP_ & 0x0007);
    uint8_t Rd = ((OP_ & 0x01F0) >> 4);

    stCPU.b = b;
    stCPU.Rd = &(stCPU.pstRAM->stRegisters.CORE_REGISTERS.r[Rd]);
}

//---------------------------------------------------------------------------
static void AVR_Decoder_SBRC_SBRS( uint16_t OP_)
{
    uint8_t b = (OP_ & 0x0007);
    uint8_t Rd = ((OP_ & 0x01F0) >> 4);

    stCPU.b = b;
    stCPU.Rd = &(stCPU.pstRAM->stRegisters.CORE_REGISTERS.r[Rd]);
}

//---------------------------------------------------------------------------
AVR_Decoder AVR_Decoder_Function( uint16_t OP_ )
{
    if (( OP_ & 0xFF0F) == 0x9408 )
    {
        //!! MOS Verified
        // SEx/CLx status register clear/set bit.
        return AVR_Decoder_Register_SC;
    }
    else if (( OP_ & 0xFF0F) == 0x9508 )
    {
        //!! MOS Verified
        // Miscellaneous instruction
        return AVR_Decoder_Misc;
    }
    else if (( OP_ & 0xFF0F) == 0x940B )
    {
        //!! MOS Verified
        // Des round k
        return AVR_Decoder_DES_round_4;
    }
    else if ( (( OP_ & 0xFF00 ) == 0x0100 ) ||
              (( OP_ & 0xFF00 ) == 0x0200 ) )
    {
        //!! MOS Verified
        // Register pair 4bit (MOVW, MULS)
        return  AVR_Decoder_Register_Pair_4bit;
    }
    else if (( OP_ & 0xFF00 ) == 0x0300 )
    {
        //!! MOS Verified
        // 3-bit register pair (R16->R23) - (FMUL, FMULS, FMULSU, MULSU)
        return AVR_Decoder_Register_Pair_3bit;
    }
    else if (( OP_ & 0xFF00 ) <= 0x2F00 )
    {
        // Register pair 5bit
        return AVR_Decoder_Register_Pair_5bit;
    }
    else if (( OP_ & 0xFF00) <= 0x7F00 )
    {
        //!! MOS Verified
        // Register immediate
        return AVR_Decoder_Register_Immediate;
    }
    else if (( OP_ & 0xFEEF) == 0x9409 )
    {
        //!! MOS Verified
        // Indirect Jump/call
        return AVR_Decoder_Indirect_Jump;
    }
    else if (( OP_ & 0xFE08) == 0x9400 )
    {
        //!! MOS Verfied
        // 1-operand instructions.
        return AVR_Decoder_Register_Single;
    }
    else if (( OP_ & 0xFE0F) == 0x940A )
    {
        //!! MOS Verified
        // Dec Rd
        return AVR_Decoder_DEC_Rd;
    }
    else if (( OP_ & 0xFE0C) == 0x940C )
    {
        //!! MOS Verified
        // Jmp/call abs22
        return AVR_Decoder_JMP_CALL_22;
    }
    else if (( OP_ & 0xFE00) == 0x9600 )
    {
        //!! MOS Verified
        // ADIW/SBIW Rp
        return AVR_Decoder_ADIW_SBIW_6;
    }
    else if (( OP_ & 0xFC0F) == 0x9000 )
    {
        //!! MOS Verified
        // LDS/STS
        return AVR_Decoder_LDS_STS;
    }
    else if (( OP_ & 0xFC00) == 0x9000 )
    {
        //!! MOS Verified
        // LD/ST other
        return AVR_Decoder_LDST;
    }
    else if (( OP_ & 0xFC00) == 0x9800 )
    {
        //!! MOS Verified
        // IO Space bit operations
        return AVR_Decoder_IO_Bit;
    }
    else if (( OP_ & 0xFC00) == 0x9C00 )
    {
        //!! MOS Verified
        // MUL unsigned R1:R0 = Rr x Rd
        return AVR_Decoder_MUL;
    }
    else if (( OP_ & 0xFC00) == 0xF800 )
    {
        //!! MOS Verified
        // BLD/BST register bit to STATUS.T
        return AVR_Decoder_BLD_BST;
    }
    else if (( OP_ & 0xFC00) == 0xFC00 )
    {
        //!! MOS Verified
        // SBRC/SBRS
        return AVR_Decoder_SBRC_SBRS;
    }
    else if (( OP_ & 0xF800) == 0xF000 )
    {
        //!! MOS Verified
        // Conditional branch
        return AVR_Decoder_Conditional_Branch;
    }
    else if (( OP_ & 0xF000) == 0xE000 )
    {
        //!! MOS Verified
        // LDI Rh, K
        return AVR_Decoder_LDI;
    }
    else if (( OP_ & 0xF000) == 0xB000 )
    {
        //!! MOS Verified
        // IO space IN/OUT operations
        return AVR_Decoder_IO_In_Out;
    }
    else if (( OP_ & 0xE000) == 0xC000 )
    {
        //!! MOS Verified
        // RElative Jump/Call
        return AVR_Decoder_Relative_Jump;
    }
    else if (( OP_ & 0xD000) == 0x8000 )
    {
        //!! MOS Verified
        // LDD/STD to Z+kY+k
        return AVR_Decoder_LDST_YZ_k;
    }
    else if ( OP_ == 0 )
    {
        //!!MOS Verified
        return AVR_Decoder_NOP;
    }
    return AVR_Decoder_NOP;
}

//---------------------------------------------------------------------------
void AVR_Decode( uint16_t OP_ )
{
    AVR_Decoder myDecoder;
    myDecoder = AVR_Decoder_Function(OP_);
    myDecoder( OP_);
}
