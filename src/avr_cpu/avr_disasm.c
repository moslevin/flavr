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
  \file  avr_disasm.c

  \brief AVR Disassembler Implementation
*/

#include <stdint.h>
#include <stdio.h>

#include "emu_config.h"

#include "avr_disasm.h"
#include "avr_op_decode.h"
#include "avr_opcodes.h"
#include "avr_op_size.h"
#include "avr_cpu.h"
#include "avr_cpu_print.h"
#include "avr_loader.h"

//---------------------------------------------------------------------------
inline int8_t Signed_From_Unsigned_6( uint8_t u8Signed_ )
{
    int8_t i8Ret = 0;
    if( u8Signed_ & 0x20 )
    {
        //Sign extend...
        i8Ret = (int8_t)(u8Signed_ | 0xC0);
    }
    else
    {
        i8Ret = (int8_t)u8Signed_;
    }
    return i8Ret;
}

//---------------------------------------------------------------------------
inline uint8_t Register_From_Rd( void )
{
    return stCPU.Rd - &(stCPU.pstRAM->stRegisters.CORE_REGISTERS.r0);
}
 //---------------------------------------------------------------------------
inline uint8_t Register_From_Rr( void )
{
    return stCPU.Rr - &(stCPU.pstRAM->stRegisters.CORE_REGISTERS.r0);
}

//---------------------------------------------------------------------------
inline uint8_t Register_From_Rd16( void )
{
    return (uint8_t*)(stCPU.Rd16) - &(stCPU.pstRAM->stRegisters.CORE_REGISTERS.r0);
}

//---------------------------------------------------------------------------
inline uint8_t Register_From_Rr16( void )
{
    return (uint8_t*)(stCPU.Rr16) - &(stCPU.pstRAM->stRegisters.CORE_REGISTERS.r0);
}

//---------------------------------------------------------------------------
static void AVR_Disasm_ADD( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();
    uint8_t u8Rr = Register_From_Rr();

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "add r%d, r%d              \t ; Add: r%d = r%d + r%d\n",
                 u8Rd, u8Rr,
                 u8Rd, u8Rd, u8Rr );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_ADC( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();
    uint8_t u8Rr = Register_From_Rr();

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "adc r%d, r%d              \t ; Add with carry: r%d = r%d + r%d + C\n",
                 u8Rd, u8Rr,
                 u8Rd, u8Rd, u8Rr );

}

//---------------------------------------------------------------------------
static void AVR_Disasm_ADIW( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd16();
    uint8_t u8K = stCPU.K;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "adiw r%d:%d, %d           \t ; Add immediate to word: r%d:%d = r%d:%d + %d \n",
                u8Rd + 1, u8Rd, u8K,
                u8Rd + 1, u8Rd, u8Rd + 1, u8Rd, u8K
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_SUB( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();
    uint8_t u8Rr = Register_From_Rr();

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "sub r%d, r%d              \t ; Subtract: r%d = r%d - r%d \n",
                u8Rd, u8Rr,
                u8Rd, u8Rd, u8Rr
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_SUBI( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();
    uint8_t u8K = stCPU.K;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "subi r%d, %d              \t ; Subtract immediate: r%d = r%d - %d \n",
                u8Rd, u8K,
                u8Rd, u8Rd, u8K
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_SBC( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();
    uint8_t u8Rr = Register_From_Rr();

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "sbc r%d, r%d              \t ; Subtract with carry: r%d = r%d - r%d - C \n",
                u8Rd, u8Rr,
                u8Rd, u8Rd, u8Rr
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_SBCI( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();
    uint8_t u8K = stCPU.K;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "sbci r%d, %d              \t ; Subtract immediate with carry: r%d = r%d - %d - C\n",
                u8Rd, u8K,
                u8Rd, u8Rd, u8K
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_SBIW( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd16();
    uint8_t u8K = stCPU.K;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "sbiw r%d:%d, %d           \t ; Subtract immediate from word: r%d:%d = r%d:%d + %d \n",
                u8Rd + 1, u8Rd, u8K,
                u8Rd + 1, u8Rd, u8Rd + 1, u8Rd, u8K
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_AND( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();
    uint8_t u8Rr = Register_From_Rr();

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "and r%d, r%d              \t ; Logical AND: r%d = r%d & r%d \n",
                u8Rd, u8Rr,
                u8Rd, u8Rd, u8Rr
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_ANDI( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();
    uint8_t u8K = stCPU.K;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "andi r%d, %d              \t ; Logical AND with Immediate: r%d = r%d & %d\n",
                u8Rd, u8K,
                u8Rd, u8Rd, u8K
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_OR( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();
    uint8_t u8Rr = Register_From_Rr();

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "or r%d, r%d               \t ; Logical OR: r%d = r%d | r%d \n",
                u8Rd, u8Rr,
                u8Rd, u8Rd, u8Rr
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_ORI( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();
    uint8_t u8K = stCPU.K;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "ori r%d, %d               \t ; Logical OR with Immediate: r%d = r%d | %d\n",
                u8Rd, u8K,
                u8Rd, u8Rd, u8K
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_EOR( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();
    uint8_t u8Rr = Register_From_Rr();

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "eor r%d, r%d              \t ; Exclusive OR: r%d = r%d ^ r%d \n",
                u8Rd, u8Rr,
                u8Rd, u8Rd, u8Rr
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_COM( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "com r%d                   \t ; One's complement (bitwise inverse): r%d = 0xFF - r%d\n",
                u8Rd,
                u8Rd, u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_NEG( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "neg r%d                   \t ; Two's complement (sign swap): r%d = 0x00 - r%d\n",
                u8Rd,
                u8Rd, u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_SBR( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();
    uint8_t u8K = stCPU.K;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "sbr r%d, %d               \t ; Set Bits in Register: r%d = r%d | %d\n",
                u8Rd, u8K,
                u8Rd, u8Rd, u8K
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_CBR( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();
    uint8_t u8K = stCPU.K;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "cbr r%d, %d               \t ; Clear Bits in Register: r%d = r%d & (0xFF - %d)\n",
                u8Rd, u8K,
                u8Rd, u8Rd, u8K
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_INC( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "inc r%d                   \t ; Increment Register: r%d = r%d + 1\n",
                u8Rd,
                u8Rd, u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_DEC( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "dec r%d                   \t ; Decrement Register: r%d = r%d - 1\n",
                u8Rd,
                u8Rd, u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_TST( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "tst r%d                   \t ; Test Register for Zero or Negative\n",
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_CLR( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "clr r%d                   \t ; Clear Register\n",
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_SER( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "ser r%d                   \t ; Set All Bits in Register\n",
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_MUL( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();
    uint8_t u8Rr = Register_From_Rr();

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "mul r%d, r%d              \t ; Unsigned Multiply: r1:0 = r%d * r%d\n",
                u8Rd, u8Rr,
                u8Rd, u8Rr );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_MULS( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();
    uint8_t u8Rr = Register_From_Rr();

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "muls r%d, r%d             \t ; Signed Multiply: r1:0 = r%d * r%d\n",
                u8Rd, u8Rr,
                u8Rd, u8Rr );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_MULSU( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();
    uint8_t u8Rr = Register_From_Rr();

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "mulsu r%d, r%d            \t ; Signed * Unsigned Multiply: r1:0 = r%d * r%d\n",
                u8Rd, u8Rr,
                u8Rd, u8Rr );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_FMUL( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();
    uint8_t u8Rr = Register_From_Rr();

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "fmul r%d, r%d             \t ; Fractional Multiply: r1:0 = r%d * r%d\n",
                u8Rd, u8Rr,
                u8Rd, u8Rr );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_FMULS( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();
    uint8_t u8Rr = Register_From_Rr();

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "fmuls r%d, r%d            \t ; Signed Fractional Multiply: r1:0 = r%d * r%d\n",
                u8Rd, u8Rr,
                u8Rd, u8Rr );

}

//---------------------------------------------------------------------------
static void AVR_Disasm_FMULSU( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();
    uint8_t u8Rr = Register_From_Rr();

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "fmulsu r%d, r%d           \t ; Signed * Unsigned Fractional Multiply: r1:0 = r%d * r%d\n",
                u8Rd, u8Rr,
                u8Rd, u8Rr );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_DES( char *szOutput_ )
{
    uint8_t u8K = stCPU.K;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "des %d                    \t ; DES Encrypt/Decrypt\n",
            u8K );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_RJMP( char *szOutput_ )
{
    int16_t i16k = stCPU.k_s;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "rjmp %d                   \t ; Relative Jump: PC = PC + %d + 1\n",
                i16k, i16k );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_IJMP( char *szOutput_ )
{
    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "ijmp                      \t ; Indirect Jump: PC = Z\n");
}

//---------------------------------------------------------------------------
static void AVR_Disasm_EIJMP( char *szOutput_ )
{
    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "eijmp                     \t ; Extended Indirect Jump: PC(15:0) = Z(15:0), PC(21:16) = EIND\n" );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_JMP( char *szOutput_ )
{
    uint32_t u32k = stCPU.k;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "jmp 0x%X                     \t ; Jump to 0x%X \n",
                u32k, u32k );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_RCALL( char *szOutput_ )
{
    int16_t i16k = stCPU.k_s;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "rcall %d                  \t ; Relative call to Subroutine: PC = PC +%d + 1\n",
                i16k, i16k
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_ICALL( char *szOutput_ )
{
    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "icall                     \t ; Indirect Jump: PC = Z\n");
}

//---------------------------------------------------------------------------
static void AVR_Disasm_EICALL( char *szOutput_ )
{
    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "eicall                    \t ; Extended Indirect Jump: PC(15:0) = Z(15:0), PC(21:16) = EIND\n" );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_CALL( char *szOutput_ )
{
    uint32_t u32k = stCPU.k;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "call 0x%X                 \t ; Long Call to Subroutine: PC = 0x%X \n",
                u32k, u32k
                );
}
//---------------------------------------------------------------------------
static void AVR_Disasm_RET( char *szOutput_ )
{
    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "ret                       \t ; Return from subroutine\n" );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_RETI( char *szOutput_ )
{
    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "reti                      \t ; Return from interrupt\n" );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_CPSE( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();
    uint8_t u8Rr = Register_From_Rr();

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "cpse r%d, r%d             \t ; Compare, Skip Next If r%d = r%d\n",
                u8Rd, u8Rr,
                u8Rd, u8Rr
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_CP( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();
    uint8_t u8Rr = Register_From_Rr();

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "cp r%d, r%d               \t ; Compare: r%d == r%d\n",
                u8Rd, u8Rr,
                u8Rd, u8Rr
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_CPC( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();
    uint8_t u8Rr = Register_From_Rr();

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "cpc r%d, r%d              \t ; Compare with carry: r%d == r%d + C\n",
                u8Rd, u8Rr,
                u8Rd, u8Rr
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_CPI( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();
    uint8_t u8K = stCPU.K;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "cpi r%d, %d               \t ; Compare with Immediate: r%d == %d\n",
                u8Rd, u8K,
                u8Rd, u8K
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_SBRC( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();
    uint8_t u8b = stCPU.b;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "sbrc r%d, %d              \t ; Skip if Bit (%d) in Register (r%d) Cleared \n",
                u8Rd, u8b,
                u8Rd, u8b
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_SBRS( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();
    uint8_t u8b = stCPU.b;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "sbrs r%d, %d              \t ; Skip if Bit (%d) in Register (r%d) Set \n",
                u8Rd, u8b,
                u8Rd, u8b
                );

}

//---------------------------------------------------------------------------
static void AVR_Disasm_SBIC( char *szOutput_ )
{
    uint8_t u8A = stCPU.A;
    uint8_t u8b = stCPU.b;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "sbic %d, %d               \t ; Skip if Bit (%d) in IO Register (r%d) Cleared \n",
                u8A, u8b,
                u8A, u8b
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_SBIS( char *szOutput_ )
{
    uint8_t u8A = stCPU.A;
    uint8_t u8b = stCPU.b;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "sbis %d, %d               \t ; Skip if Bit (%d) in IO Register (r%d) Set \n",
                u8A, u8b,
                u8A, u8b
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_BRBS( char *szOutput_ )
{
    uint8_t u8s = stCPU.s;
    int8_t  s8k = stCPU.k_s;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "brbs %d, %d               \t ; Branch if Bit (%d) in SR set: PC = PC + %d + 1\n",
                u8s, s8k,
                u8s, s8k
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_BRBC( char *szOutput_ )
{
    uint8_t u8s = stCPU.s;
    int8_t  s8k = stCPU.k_s;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "brbc %d, %d               \t ; Branch if Bit (%d) in SR clear: PC = PC + %d + 1\n",
                u8s, s8k,
                u8s, s8k
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_BREQ( char *szOutput_ )
{    
    int8_t  s8k = stCPU.k_s;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "breq %d                   \t ; Branch if zero flag set: PC = PC + %d + 1\n",
                s8k,
                s8k
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_BRNE( char *szOutput_ )
{
    int8_t  s8k = stCPU.k_s;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "brne %d                   \t ; Branch if zero flag clear: PC = PC + %d + 1\n",
                s8k,
                s8k
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_BRCS( char *szOutput_ )
{
    int8_t  s8k = stCPU.k_s;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "brcs %d                   \t ; Branch if carry flag set: PC = PC + %d + 1\n",
                s8k,
                s8k
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_BRCC( char *szOutput_ )
{
    int8_t  s8k = stCPU.k_s;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "brcc %d                   \t ; Branch if carry flag clear: PC = PC + %d + 1\n",
                s8k,
                s8k
                );

}

//---------------------------------------------------------------------------
static void AVR_Disasm_BRSH( char *szOutput_ )
{
    int8_t  s8k = stCPU.k_s;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "brsh %d                   \t ; Branch if same or higher: PC = PC + %d + 1\n",
                s8k,
                s8k
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_BRLO( char *szOutput_ )
{
    int8_t  s8k = stCPU.k_s;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "brlo %d                   \t ; Branch if lower: PC = PC + %d + 1\n",
                s8k,
                s8k
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_BRMI( char *szOutput_ )
{
    int8_t  s8k = stCPU.k_s;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "brmi %d                   \t ; Branch if minus: PC = PC + %d + 1\n",
                s8k,
                s8k
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_BRPL( char *szOutput_ )
{
    int8_t  s8k = stCPU.k_s;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "brpl %d                   \t ; Branch if plus: PC = PC + %d + 1\n",
                s8k,
                s8k
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_BRGE( char *szOutput_ )
{
    int8_t  s8k = stCPU.k_s;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "brge %d                   \t ; Branch if greater-or-equal (signed): PC = PC + %d + 1\n",
                s8k,
                s8k
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_BRLT( char *szOutput_ )
{
    int8_t  s8k = stCPU.k_s;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "brlt %d                   \t ; Branch if less-than (signed): PC = PC + %d + 1\n",
                s8k,
                s8k
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_BRHS( char *szOutput_ )
{
    int8_t  s8k = stCPU.k_s;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "brlt %d                   \t ; Branch if half-carry set: PC = PC + %d + 1\n",
                s8k,
                s8k
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_BRHC( char *szOutput_ )
{
    int8_t  s8k = stCPU.k_s;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "brhc %d                   \t ; Branch if half-carry clear: PC = PC + %d + 1\n",
                s8k,
                s8k
                );

}

//---------------------------------------------------------------------------
static void AVR_Disasm_BRTS( char *szOutput_ )
{
    int8_t  s8k = stCPU.k_s;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "brts %d                   \t ; Branch if T-flag set: PC = PC + %d + 1\n",
                s8k,
                s8k
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_BRTC( char *szOutput_ )
{
    int8_t  s8k = stCPU.k_s;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "brtc %d                   \t ; Branch if T-flag clear: PC = PC + %d + 1\n",
                s8k,
                s8k
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_BRVS( char *szOutput_ )
{
    int8_t  s8k = stCPU.k_s;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "brvs %d                   \t ; Branch if Overflow set: PC = PC + %d + 1\n",
                s8k,
                s8k
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_BRVC( char *szOutput_ )
{
    int8_t  s8k = stCPU.k_s;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "brvc %d                   \t ; Branch if Overflow clear: PC = PC + %d + 1\n",
                s8k,
                s8k
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_BRIE( char *szOutput_ )
{
    int8_t  s8k = stCPU.k_s;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "brie %d                   \t ; Branch if Interrupt Enabled: PC = PC + %d + 1\n",
                s8k,
                s8k
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_BRID( char *szOutput_ )
{
    int8_t  s8k = stCPU.k_s;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "brid %d                   \t ; Branch if Interrupt Disabled: PC = PC + %d + 1\n",
                s8k,
                s8k
                );

}

//---------------------------------------------------------------------------
static void AVR_Disasm_MOV( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();
    uint8_t u8Rr = Register_From_Rr();

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "mov r%d, r%d              \t ; Copy Register: r%d = r%d\n",
                u8Rd, u8Rr,
                u8Rd, u8Rr
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_MOVW( char *szOutput_ )
{
    uint16_t u16Rd = Register_From_Rd16();
    uint16_t u16Rr = Register_From_Rr16();

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "movw r%d:r%d, r%d:r%d     \t ; Copy Register (Word): r%d:r%d = r%d:r%d\n",
                u16Rd+1, u16Rd, u16Rr+1, u16Rr,
                u16Rd+1, u16Rd, u16Rr+1, u16Rr
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_LDI( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();
    uint8_t u8K = stCPU.K;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "ldi r%d, %d               \t ; Load Immediate: r%d = %d\n",
                u8Rd, u8K,
                u8Rd, u8K
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_LDS( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();
    uint16_t u16k = stCPU.k;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "lds r%d, %d               \t ; Load Direct from Data Space: r%d = (%d)\n",
                u8Rd, u16k,
                u8Rd, u16k
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_LD_X_Indirect( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "ld r%d, X                 \t ; Load Indirect from Data Space\n",
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_LD_X_Indirect_Postinc( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "ld r%d, X+                \t ; Load Indirect from Data Space w/Postincrement\n",
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_LD_X_Indirect_Predec( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "ld r%d, -X                \t ; Load Indirect from Data Space w/Predecrement\n",
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_LD_Y_Indirect( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "ld r%d, Y                 \t ; Load Indirect from Data Space\n",
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_LD_Y_Indirect_Postinc( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "ld r%d, Y+                \t ; Load Indirect from Data Space w/Postincrement\n",
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_LD_Y_Indirect_Predec( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "ld r%d, -Y                \t ; Load Indirect from Data Space w/Predecrement\n",
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_LDD_Y( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();
    uint8_t u8q = stCPU.q;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "ldd r%d, Y+%d             \t ; Load Indirect from Data Space (with Displacement)\n",
                u8Rd, u8q
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_LD_Z_Indirect( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "ld r%d, Z                 \t ; Load Indirect from Data Space\n",
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_LD_Z_Indirect_Postinc( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "ld r%d, Z+                \t ; Load Indirect from Data Space w/Postincrement\n",
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_LD_Z_Indirect_Predec( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "ld r%d, -Z                \t ; Load Indirect from Data Space w/Predecrement\n",
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_LDD_Z( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();
    uint8_t u8q = stCPU.q;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "ldd r%d, Z+%d             \t ; Load Indirect from Data Space (with Displacement)\n",
                u8Rd, u8q
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_STS( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();
    uint16_t u16k = stCPU.k;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "sts %d, r%d               \t ; Store Direct to Data Space: (%d) = r%d\n",
                u16k, u8Rd,
                u16k, u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_ST_X_Indirect( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "st X, r%d                 \t ; Store Indirect\n",
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_ST_X_Indirect_Postinc( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "st X+, r%d                \t ; Store Indirect w/Postincrement \n",
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_ST_X_Indirect_Predec( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "st -X, r%d                \t ; Store Indirect w/Predecrement\n",
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_ST_Y_Indirect( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "st Y, r%d                 \t ; Store Indirect\n",
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_ST_Y_Indirect_Postinc( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "st Y+, r%d                \t ; Store Indirect w/Postincrement \n",
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_ST_Y_Indirect_Predec( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "st -Y, r%d                \t ; Store Indirect w/Predecrement\n",
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_STD_Y( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();
    uint8_t u8q = stCPU.q;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "std Y+%d, r%d             \t ; Store Indirect from Data Space (with Displacement)\n",
                u8q, u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_ST_Z_Indirect( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "st Z, r%d                 \t ; Store Indirect\n",
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_ST_Z_Indirect_Postinc( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "st Z+, r%d                \t ; Store Indirect w/Postincrement \n",
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_ST_Z_Indirect_Predec( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "st -Z, r%d                \t ; Store Indirect w/Predecrement\n",
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_STD_Z( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();
    uint8_t u8q = stCPU.q;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "std Z+%d, r%d             \t ; Store Indirect from Data Space (with Displacement)\n",
                u8q, u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_LPM( char *szOutput_ )
{
    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "lpm                       \t ; Load Program Memory: r0 = (Z)\n" );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_LPM_Z( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "lpm r%d, Z                \t ; Load Program Memory: r%d = (Z)\n",
                u8Rd,
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_LPM_Z_Postinc( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "lpm r%d, Z+               \t ; Load Program Memory with Postincrement: r%d = (Z), Z = Z + 1\n",
                u8Rd,
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_ELPM( char *szOutput_ )
{
    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "elpm                      \t ; (Extended) Load Program Memory: r0 = (Z)\n" );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_ELPM_Z( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "elpm r%d, Z               \t ; (Extended) Load Program Memory: r%d = (Z)\n",
                u8Rd,
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_ELPM_Z_Postinc( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "elpm r%d, Z+              \t ; (Extended) Load Program Memory w/Postincrement: r%d = (Z), Z = Z + 1\n",
                u8Rd,
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_SPM( char *szOutput_ )
{
    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "spm                       \t ; Store Program Memory\n" );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_SPM_Z_Postinc2( char *szOutput_ )
{
    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "spm Z+                    \t ; Store Program Memory Z = Z + 2 \n" );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_IN( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();
    uint8_t u8A = stCPU.A;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "in r%d, %d                \t ; Load an I/O location to register\n",
                u8Rd,
                u8A
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_OUT( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();
    uint8_t u8A = stCPU.A;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "out %d, r%d               \t ; Load an I/O location to register\n",
                u8A,
                u8Rd
                );

}

//---------------------------------------------------------------------------
static void AVR_Disasm_LAC( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "lac Z, r%d                   \t ; Load And Clear\n",
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_LAS( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "las Z, r%d                   \t ; Load And Set\n",
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_LAT( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "lat Z, r%d                   \t ; Load And Toggle\n",
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_LSL( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "lsl r%d                   \t ; Logical shift left r%d by 1 bit\n",
                u8Rd,
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_LSR( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "lsr r%d                   \t ; Logical shift right r%d by 1 bit\n",
                u8Rd,
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_POP( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "pop r%d                   \t ; Pop byte from stack into r%d\n",
                u8Rd,
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_PUSH( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "push r%d                  \t ; Push register r%d to stack\n",
                u8Rd,
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_ROL( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "rol r%d                   \t ; Rotate Left through Carry\n",
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_ROR( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "ror r%d                   \t ; Rotate Right through Carry\n",
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_ASR( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "asr r%d                   \t ; Arithmatic Shift Right\n",
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_SWAP( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "swap r%d                  \t ; Swap high/low Nibbles in Register\n",
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_BSET( char *szOutput_ )
{
    uint8_t u8s = stCPU.s;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "bset %d                   \t ; Set bit %d in status register\n",
                u8s,
                u8s
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_BCLR( char *szOutput_ )
{
    uint8_t u8s = stCPU.s;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "bclr %d                   \t ; Clear bit %d in status register\n",
                u8s,
                u8s
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_SBI( char *szOutput_ )
{
    uint8_t u8b = stCPU.b;
    uint8_t u8A = stCPU.A;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "sbi %d, %d                \t ; Set bit in I/O register\n",
                u8A,
                u8b
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_CBI( char *szOutput_ )
{
    uint8_t u8s = stCPU.b;
    uint8_t u8A = stCPU.A;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "cbi %d, %d                \t ; Clear bit in I/O register\n",
                u8A,
                u8s
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_BST( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();
    uint8_t u8b = stCPU.b;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "bst r%d, %d               \t ; Store Bit %d of r%d in the T register\n",
                u8Rd, u8b,
                u8b, u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_BLD( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();
    uint8_t u8b = stCPU.b;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "bld r%d, %d               \t ; Load the T register into Bit %d of r%d\n",
                u8Rd, u8b,
                u8b, u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_SEC( char *szOutput_ )
{
    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "sec                       \t ; Set the carry flag in the SR\n" );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_CLC( char *szOutput_ )
{
    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "clc                       \t ; Clear the carry flag in the SR\n" );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_SEN( char *szOutput_ )
{
    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "sen                       \t ; Set the negative flag in the SR\n" );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_CLN( char *szOutput_ )
{
    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "cln                       \t ; Clear the negative flag in the SR\n" );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_SEZ( char *szOutput_ )
{
    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "sez                       \t ; Set the zero flag in the SR\n" );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_CLZ( char *szOutput_ )
{
    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "clz                       \t ; Clear the zero flag in the SR\n" );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_SEI( char *szOutput_ )
{
    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "sei                       \t ; Enable MCU interrupts\n" );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_CLI( char *szOutput_ )
{
    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "cli                       \t ; Disable MCU interrupts\n" );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_SES( char *szOutput_ )
{
    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "ses                       \t ; Set the sign flag in the SR\n" );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_CLS( char *szOutput_ )
{
    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "cls                       \t ; Clear the sign flag in the SR\n" );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_SEV( char *szOutput_ )
{
    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "sev                       \t ; Set the overflow flag in the SR\n" );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_CLV( char *szOutput_ )
{
    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "clv                       \t ; Clear the overflow flag in the SR\n" );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_SET( char *szOutput_ )
{
    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "set                       \t ; Set the T-flag in the SR\n" );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_CLT( char *szOutput_ )
{
    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "clt                       \t ; Clear the T-flag in the SR\n" );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_SEH( char *szOutput_ )
{
    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "seh                       \t ; Set half-carry flag in SR\n" );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_CLH( char *szOutput_ )
{
    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "clh                       \t ; Clear half-carry flag in SR\n" );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_BREAK( char *szOutput_ )
{
    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "break                     \t ; Halt for debugger\n" );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_NOP( char *szOutput_ )
{
    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "nop                       \t ; Do nothing\n" );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_SLEEP( char *szOutput_ )
{
    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "sleep                     \t ; Put MCU into sleep mode\n" );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_WDR( char *szOutput_ )
{
    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "wdr                       \t ; Reset Watchdog Timer\n" );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_XCH( char *szOutput_ )
{
    uint8_t u8Rd = Register_From_Rd();

    //ruler: 0----5----10---15---20---25---30---35---40" );
    sprintf( szOutput_, "xch Z, r%d                \t ; Exchange registers w/memory\n",
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_Unimplemented( char *szOutput_ )
{
    sprintf( szOutput_, ".db 0x%04X ; Data (not an opcode)\n", stCPU.pu16ROM[ stCPU.u32PC ] );
}

//---------------------------------------------------------------------------
AVR_Disasm AVR_Disasm_Function( uint16_t OP_ )
{
    // Special instructions - "static" encoding
    switch (OP_)
    {
    case 0x0000: return AVR_Disasm_NOP;

    case 0x9408: return AVR_Disasm_SEC;
    case 0x9409: return AVR_Disasm_IJMP;
    case 0x9418: return AVR_Disasm_SEZ;
    case 0x9419: return AVR_Disasm_EIJMP;
    case 0x9428: return AVR_Disasm_SEN;
    case 0x9438: return AVR_Disasm_SEV;
    case 0x9448: return AVR_Disasm_SES;
    case 0x9458: return AVR_Disasm_SEH;
    case 0x9468: return AVR_Disasm_SET;
    case 0x9478: return AVR_Disasm_SEI;

    case 0x9488: return AVR_Disasm_CLC;
    case 0x9498: return AVR_Disasm_CLZ;
    case 0x94A8: return AVR_Disasm_CLN;
    case 0x94B8: return AVR_Disasm_CLV;
    case 0x94C8: return AVR_Disasm_CLS;
    case 0x94D8: return AVR_Disasm_CLH;
    case 0x94E8: return AVR_Disasm_CLT;
    case 0x94F8: return AVR_Disasm_CLI;

    case 0x9508: return AVR_Disasm_RET;
    case 0x9509: return AVR_Disasm_ICALL;
    case 0x9518: return AVR_Disasm_RETI;
    case 0x9519: return AVR_Disasm_EICALL;
    case 0x9588: return AVR_Disasm_SLEEP;
    case 0x9598: return AVR_Disasm_BREAK;
    case 0x95A8: return AVR_Disasm_WDR;
    case 0x95C8: return AVR_Disasm_LPM;
    case 0x95D8: return AVR_Disasm_ELPM;
    case 0x95E8: return AVR_Disasm_SPM;
    case 0x95F8: return AVR_Disasm_SPM_Z_Postinc2;
    }

#if 0
    // Note: These disasm handlers are generalized versions of specific mnemonics in the above list.
    // For disassembly, it's probably easier to read the output from the more "spcific" mnemonics, so
    // those are used.  For emulation, using the generalized functions may be more desirable.
    switch( OP_ & 0xFF8F)
    {
    case 0x9408: return AVR_Disasm_BSET;
    case 0x9488: return AVR_Disasm_BCLR;
    }
#endif

    switch (OP_ & 0xFF88)
    {
    case 0x0300: return AVR_Disasm_MULSU;
    case 0x0308: return AVR_Disasm_FMUL;
    case 0x0380: return AVR_Disasm_FMULS;
    case 0x0388: return AVR_Disasm_FMULSU;
    }

    switch (OP_ & 0xFF0F)
    {
    case 0x940B: return AVR_Disasm_DES;
    case 0xEF0F: return AVR_Disasm_SER;
    }

    switch (OP_ & 0xFF00)
    {
    case 0x0100: return AVR_Disasm_MOVW;
    case 0x9600: return AVR_Disasm_ADIW;
    case 0x9700: return AVR_Disasm_SBIW;

    case 0x9800: return AVR_Disasm_CBI;
    case 0x9900: return AVR_Disasm_SBIC;
    case 0x9A00: return AVR_Disasm_SBI;
    case 0x9B00: return AVR_Disasm_SBIS;
    }

    switch (OP_ & 0xFE0F)
    {
    case 0x8008: return AVR_Disasm_LD_Y_Indirect;
    case 0x8000: return AVR_Disasm_LD_Z_Indirect;
    case 0x8200: return AVR_Disasm_ST_Z_Indirect;
    case 0x8208: return AVR_Disasm_ST_Y_Indirect;

    // -- Single 5-bit register...
    case 0x9000: return AVR_Disasm_LDS;        
    case 0x9001: return AVR_Disasm_LD_Z_Indirect_Postinc;
    case 0x9002: return AVR_Disasm_LD_Z_Indirect_Predec;
    case 0x9004: return AVR_Disasm_LPM_Z;
    case 0x9005: return AVR_Disasm_LPM_Z_Postinc;
    case 0x9006: return AVR_Disasm_ELPM_Z;
    case 0x9007: return AVR_Disasm_ELPM_Z_Postinc;
    case 0x9009: return AVR_Disasm_LD_Y_Indirect_Postinc;
    case 0x900A: return AVR_Disasm_LD_Y_Indirect_Predec;
    case 0x900C: return AVR_Disasm_LD_X_Indirect;
    case 0x900D: return AVR_Disasm_LD_X_Indirect_Postinc;
    case 0x900E: return AVR_Disasm_LD_X_Indirect_Predec;
    case 0x900F: return AVR_Disasm_POP;

    case 0x9200: return AVR_Disasm_STS;
    case 0x9201: return AVR_Disasm_ST_Z_Indirect_Postinc;
    case 0x9202: return AVR_Disasm_ST_Z_Indirect_Predec;
    case 0x9204: return AVR_Disasm_XCH;
    case 0x9205: return AVR_Disasm_LAS;
    case 0x9206: return AVR_Disasm_LAC;
    case 0x9207: return AVR_Disasm_LAT;
    case 0x9209: return AVR_Disasm_ST_Y_Indirect_Postinc;
    case 0x920A: return AVR_Disasm_ST_Y_Indirect_Predec;
    case 0x920C: return AVR_Disasm_ST_X_Indirect;
    case 0x920D: return AVR_Disasm_ST_X_Indirect_Postinc;
    case 0x920E: return AVR_Disasm_ST_X_Indirect_Predec;
    case 0x920F: return AVR_Disasm_PUSH;

    // -- One-operand instructions
    case 0x9400: return AVR_Disasm_COM;
    case 0x9401: return AVR_Disasm_NEG;
    case 0x9402: return AVR_Disasm_SWAP;
    case 0x9403: return AVR_Disasm_INC;
    case 0x9405: return AVR_Disasm_ASR;
    case 0x9406: return AVR_Disasm_LSR;
    case 0x9407: return AVR_Disasm_ROR;
    case 0x940A: return AVR_Disasm_DEC;

    }
    switch (OP_ & 0xFE0E)
    {
    case 0x940C: return AVR_Disasm_JMP;
    case 0x940E: return AVR_Disasm_CALL;
    }

    switch (OP_ & 0xFE08)
    {

    // -- BLD/BST Encoding
    case 0xF800: return AVR_Disasm_BLD;
    case 0xFA00: return AVR_Disasm_BST;
    // -- SBRC/SBRS Encoding
    case 0xFC00: return AVR_Disasm_SBRC;
    case 0xFE00: return AVR_Disasm_SBRS;
    }

    switch (OP_ & 0xFC07)
    {
    // -- Conditional branches
    case 0xF000: return AVR_Disasm_BRCS;
    // case 0xF000: return AVR_Disasm_BRLO;            // AKA AVR_Disasm_BRCS;
    case 0xF001: return AVR_Disasm_BREQ;
    case 0xF002: return AVR_Disasm_BRMI;
    case 0xF003: return AVR_Disasm_BRVS;
    case 0xF004: return AVR_Disasm_BRLT;
    case 0xF006: return AVR_Disasm_BRTS;
    case 0xF007: return AVR_Disasm_BRIE;
    case 0xF400: return AVR_Disasm_BRCC;
    // case 0xF400: return AVR_Disasm_BRSH;            // AKA AVR_Disasm_BRCC;
    case 0xF401: return AVR_Disasm_BRNE;
    case 0xF402: return AVR_Disasm_BRPL;
    case 0xF403: return AVR_Disasm_BRVC;
    case 0xF404: return AVR_Disasm_BRGE;
    case 0xF405: return AVR_Disasm_BRHC;
    case 0xF406: return AVR_Disasm_BRTC;
    case 0xF407: return AVR_Disasm_BRID;
    }

    switch (OP_ & 0xFC00)
    {
    // -- 4-bit register pair
    case 0x0200: return AVR_Disasm_MULS;

    // -- 5-bit register pairs --
    case 0x0400: return AVR_Disasm_CPC;
    case 0x0800: return AVR_Disasm_SBC;
    case 0x0C00: return AVR_Disasm_ADD;
    // case 0x0C00: return AVR_Disasm_LSL; (!! Implemented with: " add rd, rd"
    case 0x1000: return AVR_Disasm_CPSE;
    case 0x1300: return AVR_Disasm_ROL;
    case 0x1400: return AVR_Disasm_CP;
    case 0x1C00: return AVR_Disasm_ADC;
    case 0x1800: return AVR_Disasm_SUB;
    case 0x2000: return AVR_Disasm_AND;
    // case 0x2000: return AVR_Disasm_TST; (!! Implemented with: " and rd, rd"
    case 0x2400: return AVR_Disasm_EOR;
    case 0x2C00: return AVR_Disasm_MOV;
    case 0x2800: return AVR_Disasm_OR;

    // -- 5-bit register pairs -- Destination = R1:R0
    case 0x9C00: return AVR_Disasm_MUL;
    }

    switch (OP_ & 0xF800)
    {
    case  0xB800: return AVR_Disasm_OUT;
    case  0xB000: return AVR_Disasm_IN;
    }

    switch (OP_ & 0xF000)
    {
    // -- Register immediate --
    case 0x3000: return AVR_Disasm_CPI;
    case 0x4000: return AVR_Disasm_SBCI;
    case 0x5000: return AVR_Disasm_SUBI;
    case 0x6000: return AVR_Disasm_ORI;// return AVR_Disasm_SBR;
    case 0x7000: return AVR_Disasm_ANDI;

    //-- 12-bit immediate
    case 0xC000: return AVR_Disasm_RJMP;
    case 0xD000: return AVR_Disasm_RCALL;

    // -- Register immediate
    case 0xE000: return AVR_Disasm_LDI;
    }

    switch (OP_ & 0xD208)
    {
    // -- 7-bit signed offset
    case 0x8000: return AVR_Disasm_LDD_Z;
    case 0x8008: return AVR_Disasm_LDD_Y;
    case 0x8200: return AVR_Disasm_STD_Z;
    case 0x8208: return AVR_Disasm_STD_Y;
    }

    return AVR_Disasm_Unimplemented;
}

