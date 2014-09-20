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
  \file  avr_op_size.c

  \brief Module providing opcode sizes
*/

#include <stdint.h>

#include "emu_config.h"

#include "avr_op_size.h"

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Size_NOP( uint16_t OP_);
static uint8_t AVR_Opcode_Size_Register_Pair_4bit( uint16_t OP_);
static uint8_t AVR_Opcode_Size_Register_Pair_3bit( uint16_t OP_);
static uint8_t AVR_Opcode_Size_Register_Pair_5bit( uint16_t OP_);
static uint8_t AVR_Opcode_Size_Register_Immediate( uint16_t OP_);
static uint8_t AVR_Opcode_Size_LDST_YZ_k( uint16_t OP_);
static uint8_t AVR_Opcode_Size_LDST( uint16_t OP_);
static uint8_t AVR_Opcode_Size_Register_Single( uint16_t OP_);
static uint8_t AVR_Opcode_Size_Register_SC( uint16_t OP_);
static uint8_t AVR_Opcode_Size_Misc( uint16_t OP_);
static uint8_t AVR_Opcode_Size_Indirect_Jump( uint16_t OP_);
static uint8_t AVR_Opcode_Size_DEC_Rd( uint16_t OP_);
static uint8_t AVR_Opcode_Size_DES_round_4( uint16_t OP_);
static uint8_t AVR_Opcode_Size_JMP_CALL_22( uint16_t OP_);
static uint8_t AVR_Opcode_Size_ADIW_SBIW_6( uint16_t OP_);
static uint8_t AVR_Opcode_Size_IO_Bit( uint16_t OP_);
static uint8_t AVR_Opcode_Size_MUL( uint16_t OP_);
static uint8_t AVR_Opcode_Size_IO_In_Out( uint16_t OP_);
static uint8_t AVR_Opcode_Size_Relative_Jump( uint16_t OP_);
static uint8_t AVR_Opcode_Size_LDI( uint16_t OP_);
static uint8_t AVR_Opcode_Size_Conditional_Branch( uint16_t OP_);
static uint8_t AVR_Opcode_Size_BLD_BST( uint16_t OP_);
static uint8_t AVR_Opcode_Size_SBRC_SBRS( uint16_t OP_);

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Size_NOP( uint16_t OP_)
{
    return 1;
}
//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Size_Register_Pair_4bit( uint16_t OP_)
{
    return 1;
}
//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Size_Register_Pair_3bit( uint16_t OP_)
{
    return 1;
}
//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Size_Register_Pair_5bit( uint16_t OP_)
{
    return 1;
}
//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Size_Register_Immediate( uint16_t OP_)
{
    return 1;
}
//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Size_LDST_YZ_k( uint16_t OP_)
{
    return 1;
}
//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Size_LDST( uint16_t OP_)
{
    return 1;
}
//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Size_LDS_STS( uint16_t OP_)
{
    return 2;
}
//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Size_Register_Single( uint16_t OP_)
{
    return 1;
}
//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Size_Register_SC( uint16_t OP_)
{
    return 1;
}
//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Size_Misc( uint16_t OP_)
{
    return 1;
}
//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Size_Indirect_Jump( uint16_t OP_)
{
    return 1;
}
//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Size_DEC_Rd( uint16_t OP_)
{
    return 1;
}
//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Size_DES_round_4( uint16_t OP_)
{
    return 1;
}
//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Size_JMP_CALL_22( uint16_t OP_)
{
    return 2;
}
//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Size_ADIW_SBIW_6( uint16_t OP_)
{
    return 1;
}
//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Size_IO_Bit( uint16_t OP_)
{
    return 1;
}
//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Size_MUL( uint16_t OP_)
{
    return 1;
}
//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Size_IO_In_Out( uint16_t OP_)
{
    return 1;
}
//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Size_Relative_Jump( uint16_t OP_)
{
    return 1;
}
//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Size_LDI( uint16_t OP_)
{
    return 1;
}
//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Size_Conditional_Branch( uint16_t OP_)
{
    return 1;
}
//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Size_BLD_BST( uint16_t OP_)
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Size_SBRC_SBRS( uint16_t OP_)
{
    return 1;
}

//---------------------------------------------------------------------------
uint8_t AVR_Opcode_Size( uint16_t OP_ )
{
    if (( OP_ & 0xFF0F) == 0x9408 )
    {
        // SEx/CLx status register clear/set bit.
        return AVR_Opcode_Size_Register_SC( OP_ );
    }
    else if (( OP_ & 0xFF0F) == 0x9508 )
    {
        // Miscellaneous instruction
        return AVR_Opcode_Size_Misc( OP_ );
    }
    else if (( OP_ & 0xFF0F) == 0x940B )
    {
        // Des round k
        return AVR_Opcode_Size_DES_round_4( OP_ );
    }
    else if ( (( OP_ & 0xFF00 ) == 0x0100 ) ||
              (( OP_ & 0xFF00 ) == 0x0200 ) )
    {
        // Register pair 4bit (MOVW, MULS)
         return AVR_Opcode_Size_Register_Pair_4bit( OP_ );
    }
    else if (( OP_ & 0xFF00 ) == 0x0300 )
    {
        // 3-bit register pair (R16->R23) - (FMUL, FMULS, FMULSU, MULSU)
        return AVR_Opcode_Size_Register_Pair_3bit( OP_ );
    }
    else if (( OP_ & 0xFF00 ) <= 0x4F00 )
    {
        // Register pair 5bit
        return AVR_Opcode_Size_Register_Pair_5bit( OP_ );
    }
    else if (( OP_ & 0xFF00) <= 0x7F00 )
    {
        // Register immediate
        return AVR_Opcode_Size_Register_Immediate( OP_ );
    }
    else if (( OP_ & 0xFEEF) == 0x9409 )
    {
        // Indirect Jump/call
        return AVR_Opcode_Size_Indirect_Jump( OP_ );
    }
    else if (( OP_ & 0xFE08) == 0x9400 )
    {
        // 1-operand instructions.
        return AVR_Opcode_Size_Register_Single( OP_ );
    }
    else if (( OP_ & 0xFE0F) == 0x940A )
    {
        // Dec Rd
        return AVR_Opcode_Size_DEC_Rd( OP_ );
    }
    else if (( OP_ & 0xFE0C) == 0x940C )
    {
        // Jmp/call abs22
        return AVR_Opcode_Size_JMP_CALL_22( OP_ );
    }
    else if (( OP_ & 0xFE00) == 0x9600 )
    {
        // ADIW/SBIW Rp
        return AVR_Opcode_Size_ADIW_SBIW_6( OP_ );
    }
    else if (( OP_ & 0xFC0F) == 0x9000 )
    {
        // LDS/STS
        return AVR_Opcode_Size_LDS_STS( OP_ );
    }
    else if (( OP_ & 0xFC00) == 0x9000 )
    {
        // LD/ST other
        return AVR_Opcode_Size_LDST( OP_ );
    }
    else if (( OP_ & 0xFC00) == 0x9800 )
    {
        // IO Space bit operations
        return AVR_Opcode_Size_IO_Bit( OP_ );
    }
    else if (( OP_ & 0xFC00) == 0x9C00 )
    {
        // MUL unsigned R1:R0 = Rr x Rd
        return AVR_Opcode_Size_MUL( OP_ );
    }
    else if (( OP_ & 0xFC00) == 0xF800 )
    {
        // BLD/BST register bit to STATUS.T
        return AVR_Opcode_Size_BLD_BST( OP_ );
    }
    else if (( OP_ & 0xFC00) == 0xFC00 )
    {
        // SBRC/SBRS
        return AVR_Opcode_Size_SBRC_SBRS( OP_ );
    }
    else if (( OP_ & 0xF800) == 0xF000 )
    {
        // Conditional branch
        return AVR_Opcode_Size_Conditional_Branch( OP_ );
    }
    else if (( OP_ & 0xF000) == 0xE000 )
    {
        // LDI Rh, K
        return AVR_Opcode_Size_LDI( OP_ );
    }
    else if (( OP_ & 0xF000) == 0xB000 )
    {
        // IO space IN/OUT operations
        return AVR_Opcode_Size_IO_In_Out( OP_ );
    }
    else if (( OP_ & 0xE000) == 0xC000 )
    {
        // RElative Jump/Call
        return AVR_Opcode_Size_Relative_Jump( OP_ );
    }
    else if (( OP_ & 0xD000) == 0x8000 )
    {
        // LDD/STD to Z+kY+k
        return AVR_Opcode_Size_LDST_YZ_k( OP_ );
    }
    else if ( OP_ == 0 )
    {
        return AVR_Opcode_Size_NOP( OP_ );
    }
    return AVR_Opcode_Size_NOP( OP_ );
}
