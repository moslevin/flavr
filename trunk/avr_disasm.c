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
  \file  avr_disasm.c

  \brief AVR Disassembler Implementation
*/

#include <stdint.h>
#include <stdio.h>

#include "emu_config.h"

#include "avr_op_decode.h"
#include "avr_opcodes.h"
#include "avr_op_size.h"
#include "avr_cpu.h"
#include "avr_cpu_print.h"
#include "avr_loader.h"

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
inline uint8_t Register_From_Rd( AVR_CPU *pstCPU_ )
{
    return pstCPU_->Rd - &(pstCPU_->pstRAM->stRegisters.CORE_REGISTERS.r0);
}
 //---------------------------------------------------------------------------
inline uint8_t Register_From_Rr( AVR_CPU *pstCPU_ )
{
    return pstCPU_->Rr - &(pstCPU_->pstRAM->stRegisters.CORE_REGISTERS.r0);
}

//---------------------------------------------------------------------------
inline uint8_t Register_From_Rd16( AVR_CPU *pstCPU_ )
{
    return (uint8_t*)(pstCPU_->Rd16) - &(pstCPU_->pstRAM->stRegisters.CORE_REGISTERS.r0);
}

//---------------------------------------------------------------------------
inline uint8_t Register_From_Rr16( AVR_CPU *pstCPU_ )
{
    return (uint8_t*)(pstCPU_->Rr16) - &(pstCPU_->pstRAM->stRegisters.CORE_REGISTERS.r0);
}

//---------------------------------------------------------------------------
static void AVR_Disasm_ADD( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );
    uint8_t u8Rr = Register_From_Rr( pstCPU_ );

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "add r%d, r%d              \t ; Add: r%d = r%d + r%d\n",
                 u8Rd, u8Rr,
                 u8Rd, u8Rd, u8Rr );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_ADC( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );
    uint8_t u8Rr = Register_From_Rr( pstCPU_ );

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "adc r%d, r%d              \t ; Add with carry: r%d = r%d + r%d + C\n",
                 u8Rd, u8Rr,
                 u8Rd, u8Rd, u8Rr );

}

//---------------------------------------------------------------------------
static void AVR_Disasm_ADIW( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd16( pstCPU_ );
    uint8_t u8K = pstCPU_->K;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "adiw r%d:%d, %d           \t ; Add immediate to word: r%d:%d = r%d:%d + %d \n",
                u8Rd + 1, u8Rd, u8K,
                u8Rd + 1, u8Rd, u8Rd + 1, u8Rd, u8K
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_SUB( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );
    uint8_t u8Rr = Register_From_Rr( pstCPU_ );

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "sub r%d, r%d              \t ; Subtract: r%d = r%d - r%d \n",
                u8Rd, u8Rr,
                u8Rd, u8Rd, u8Rr
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_SUBI( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );
    uint8_t u8K = pstCPU_->K;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "subi r%d, %d              \t ; Subtract immediate: r%d = r%d - %d \n",
                u8Rd, u8K,
                u8Rd, u8Rd, u8K
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_SBC( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );
    uint8_t u8Rr = Register_From_Rr( pstCPU_ );

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "sbc r%d, r%d              \t ; Subtract with carry: r%d = r%d - r%d - C \n",
                u8Rd, u8Rr,
                u8Rd, u8Rd, u8Rr
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_SBCI( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );
    uint8_t u8K = pstCPU_->K;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "sbci r%d, %d              \t ; Subtract immediate with carry: r%d = r%d - %d - C\n",
                u8Rd, u8K,
                u8Rd, u8Rd, u8K
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_SBIW( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd16( pstCPU_ );
    uint8_t u8K = pstCPU_->K;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "sbiw r%d:%d, %d           \t ; Subtract immediate from word: r%d:%d = r%d:%d + %d \n",
                u8Rd + 1, u8Rd, u8K,
                u8Rd + 1, u8Rd, u8Rd + 1, u8Rd, u8K
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_AND( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );
    uint8_t u8Rr = Register_From_Rr( pstCPU_ );

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "and r%d, r%d              \t ; Logical AND: r%d = r%d & r%d \n",
                u8Rd, u8Rr,
                u8Rd, u8Rd, u8Rr
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_ANDI( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );
    uint8_t u8K = pstCPU_->K;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "andi r%d, %d              \t ; Logical AND with Immediate: r%d = r%d & %d\n",
                u8Rd, u8K,
                u8Rd, u8Rd, u8K
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_OR( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );
    uint8_t u8Rr = Register_From_Rr( pstCPU_ );

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "or r%d, r%d               \t ; Logical OR: r%d = r%d | r%d \n",
                u8Rd, u8Rr,
                u8Rd, u8Rd, u8Rr
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_ORI( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );
    uint8_t u8K = pstCPU_->K;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "ori r%d, %d               \t ; Logical OR with Immediate: r%d = r%d | %d\n",
                u8Rd, u8K,
                u8Rd, u8Rd, u8K
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_EOR( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );
    uint8_t u8Rr = Register_From_Rr( pstCPU_ );

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "eor r%d, r%d              \t ; Exclusive OR: r%d = r%d ^ r%d \n",
                u8Rd, u8Rr,
                u8Rd, u8Rd, u8Rr
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_COM( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "com r%d                   \t ; One's complement (bitwise inverse): r%d = 0xFF - r%d\n",
                u8Rd,
                u8Rd, u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_NEG( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "neg r%d                   \t ; Two's complement (sign swap): r%d = 0x00 - r%d\n",
                u8Rd,
                u8Rd, u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_SBR( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );
    uint8_t u8K = pstCPU_->K;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "sbr r%d, %d               \t ; Set Bits in Register: r%d = r%d | %d\n",
                u8Rd, u8K,
                u8Rd, u8Rd, u8K
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_CBR( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );
    uint8_t u8K = pstCPU_->K;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "cbr r%d, %d               \t ; Clear Bits in Register: r%d = r%d & (0xFF - %d)\n",
                u8Rd, u8K,
                u8Rd, u8Rd, u8K
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_INC( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "inc r%d                   \t ; Increment Register: r%d = r%d + 1\n",
                u8Rd,
                u8Rd, u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_DEC( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "dec r%d                   \t ; Decrement Register: r%d = r%d - 1\n",
                u8Rd,
                u8Rd, u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_TST( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "tst r%d                   \t ; Test Register for Zero or Negative\n",
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_CLR( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "clr r%d                   \t ; Clear Register\n",
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_SER( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "ser r%d                   \t ; Set All Bits in Register\n",
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_MUL( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );
    uint8_t u8Rr = Register_From_Rr( pstCPU_ );

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "mul r%d, r%d              \t ; Unsigned Multiply: r1:0 = r%d * r%d\n",
                u8Rd, u8Rr,
                u8Rd, u8Rr );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_MULS( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );
    uint8_t u8Rr = Register_From_Rr( pstCPU_ );

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "muls r%d, r%d             \t ; Signed Multiply: r1:0 = r%d * r%d\n",
                u8Rd, u8Rr,
                u8Rd, u8Rr );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_MULSU( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );
    uint8_t u8Rr = Register_From_Rr( pstCPU_ );

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "mulsu r%d, r%d            \t ; Signed * Unsigned Multiply: r1:0 = r%d * r%d\n",
                u8Rd, u8Rr,
                u8Rd, u8Rr );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_FMUL( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );
    uint8_t u8Rr = Register_From_Rr( pstCPU_ );

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "fmul r%d, r%d             \t ; Fractional Multiply: r1:0 = r%d * r%d\n",
                u8Rd, u8Rr,
                u8Rd, u8Rr );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_FMULS( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );
    uint8_t u8Rr = Register_From_Rr( pstCPU_ );

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "fmuls r%d, r%d            \t ; Signed Fractional Multiply: r1:0 = r%d * r%d\n",
                u8Rd, u8Rr,
                u8Rd, u8Rr );

}

//---------------------------------------------------------------------------
static void AVR_Disasm_FMULSU( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );
    uint8_t u8Rr = Register_From_Rr( pstCPU_ );

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "fmulsu r%d, r%d           \t ; Signed * Unsigned Fractional Multiply: r1:0 = r%d * r%d\n",
                u8Rd, u8Rr,
                u8Rd, u8Rr );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_DES( AVR_CPU *pstCPU_ )
{
    uint8_t u8K = pstCPU_->K;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "des %d                    \t ; DES Encrypt/Decrypt\n",
            u8K );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_RJMP( AVR_CPU *pstCPU_ )
{
    int16_t i16k = pstCPU_->k_s;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "rjmp %d                   \t ; Relative Jump: PC = PC + %d + 1\n",
                i16k, i16k );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_IJMP( AVR_CPU *pstCPU_ )
{
    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "ijmp                      \t ; Indirect Jump: PC = Z\n");
}

//---------------------------------------------------------------------------
static void AVR_Disasm_EIJMP( AVR_CPU *pstCPU_ )
{
    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "eijmp                     \t ; Extended Indirect Jump: PC(15:0) = Z(15:0), PC(21:16) = EIND\n" );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_JMP( AVR_CPU *pstCPU_ )
{
    uint32_t u32k = pstCPU_->k;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "jmp 0x%X                     \t ; Jump to 0x%X \n",
                u32k, u32k );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_RCALL( AVR_CPU *pstCPU_ )
{
    int16_t i16k = pstCPU_->k_s;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "rcall %d                  \t ; Relative call to Subroutine: PC = PC +%d + 1\n",
                i16k, i16k
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_ICALL( AVR_CPU *pstCPU_ )
{
    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "icall                     \t ; Indirect Jump: PC = Z\n");
}

//---------------------------------------------------------------------------
static void AVR_Disasm_EICALL( AVR_CPU *pstCPU_ )
{
    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "eicall                    \t ; Extended Indirect Jump: PC(15:0) = Z(15:0), PC(21:16) = EIND\n" );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_CALL( AVR_CPU *pstCPU_ )
{
    uint32_t u32k = pstCPU_->k;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "call 0x%X                 \t ; Long Call to Subroutine: PC = 0x%X \n",
                u32k, u32k
                );
}
//---------------------------------------------------------------------------
static void AVR_Disasm_RET( AVR_CPU *pstCPU_ )
{
    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "ret                       \t ; Return from subroutine\n" );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_RETI( AVR_CPU *pstCPU_ )
{
    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "reti                      \t ; Return from interrupt\n" );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_CPSE( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );
    uint8_t u8Rr = Register_From_Rr( pstCPU_ );

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "cpse r%d, r%d             \t ; Compare, Skip Next If r%d = r%d\n",
                u8Rd, u8Rr,
                u8Rd, u8Rr
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_CP( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );
    uint8_t u8Rr = Register_From_Rr( pstCPU_ );

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "cp r%d, r%d               \t ; Compare: r%d == r%d\n",
                u8Rd, u8Rr,
                u8Rd, u8Rr
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_CPC( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );
    uint8_t u8Rr = Register_From_Rr( pstCPU_ );

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "cpc r%d, r%d              \t ; Compare with carry: r%d == r%d + C\n",
                u8Rd, u8Rr,
                u8Rd, u8Rr
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_CPI( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );
    uint8_t u8K = pstCPU_->K;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "cpi r%d, %d               \t ; Compare with Immediate: r%d == %d\n",
                u8Rd, u8K,
                u8Rd, u8K
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_SBRC( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );
    uint8_t u8b = pstCPU_->b;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "sbrc r%d, %d              \t ; Skip if Bit (%d) in Register (r%d) Cleared \n",
                u8Rd, u8b,
                u8Rd, u8b
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_SBRS( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );
    uint8_t u8b = pstCPU_->b;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "sbrs r%d, %d              \t ; Skip if Bit (%d) in Register (r%d) Set \n",
                u8Rd, u8b,
                u8Rd, u8b
                );

}

//---------------------------------------------------------------------------
static void AVR_Disasm_SBIC( AVR_CPU *pstCPU_ )
{
    uint8_t u8A = pstCPU_->A;
    uint8_t u8b = pstCPU_->b;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "sbic %d, %d               \t ; Skip if Bit (%d) in IO Register (r%d) Cleared \n",
                u8A, u8b,
                u8A, u8b
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_SBIS( AVR_CPU *pstCPU_ )
{
    uint8_t u8A = pstCPU_->A;
    uint8_t u8b = pstCPU_->b;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "sbis %d, %d               \t ; Skip if Bit (%d) in IO Register (r%d) Set \n",
                u8A, u8b,
                u8A, u8b
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_BRBS( AVR_CPU *pstCPU_ )
{
    uint8_t u8s = pstCPU_->s;
    int8_t  s8k = pstCPU_->k_s;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "brbs %d, %d               \t ; Branch if Bit (%d) in SR set: PC = PC + %d + 1\n",
                u8s, s8k,
                u8s, s8k
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_BRBC( AVR_CPU *pstCPU_ )
{
    uint8_t u8s = pstCPU_->s;
    int8_t  s8k = pstCPU_->k_s;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "brbc %d, %d               \t ; Branch if Bit (%d) in SR clear: PC = PC + %d + 1\n",
                u8s, s8k,
                u8s, s8k
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_BREQ( AVR_CPU *pstCPU_ )
{    
    int8_t  s8k = pstCPU_->k_s;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "breq %d                   \t ; Branch if zero flag set: PC = PC + %d + 1\n",
                s8k,
                s8k
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_BRNE( AVR_CPU *pstCPU_ )
{
    int8_t  s8k = pstCPU_->k_s;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "brne %d                   \t ; Branch if zero flag clear: PC = PC + %d + 1\n",
                s8k,
                s8k
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_BRCS( AVR_CPU *pstCPU_ )
{
    int8_t  s8k = pstCPU_->k_s;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "brcs %d                   \t ; Branch if carry flag set: PC = PC + %d + 1\n",
                s8k,
                s8k
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_BRCC( AVR_CPU *pstCPU_ )
{
    int8_t  s8k = pstCPU_->k_s;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "brcc %d                   \t ; Branch if carry flag clear: PC = PC + %d + 1\n",
                s8k,
                s8k
                );

}

//---------------------------------------------------------------------------
static void AVR_Disasm_BRSH( AVR_CPU *pstCPU_ )
{
    int8_t  s8k = pstCPU_->k_s;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "brsh %d                   \t ; Branch if same or higher: PC = PC + %d + 1\n",
                s8k,
                s8k
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_BRLO( AVR_CPU *pstCPU_ )
{
    int8_t  s8k = pstCPU_->k_s;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "brlo %d                   \t ; Branch if lower: PC = PC + %d + 1\n",
                s8k,
                s8k
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_BRMI( AVR_CPU *pstCPU_ )
{
    int8_t  s8k = pstCPU_->k_s;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "brmi %d                   \t ; Branch if minus: PC = PC + %d + 1\n",
                s8k,
                s8k
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_BRPL( AVR_CPU *pstCPU_ )
{
    int8_t  s8k = pstCPU_->k_s;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "brpl %d                   \t ; Branch if plus: PC = PC + %d + 1\n",
                s8k,
                s8k
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_BRGE( AVR_CPU *pstCPU_ )
{
    int8_t  s8k = pstCPU_->k_s;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "brge %d                   \t ; Branch if greater-or-equal (signed): PC = PC + %d + 1\n",
                s8k,
                s8k
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_BRLT( AVR_CPU *pstCPU_ )
{
    int8_t  s8k = pstCPU_->k_s;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "brlt %d                   \t ; Branch if less-than (signed): PC = PC + %d + 1\n",
                s8k,
                s8k
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_BRHS( AVR_CPU *pstCPU_ )
{
    int8_t  s8k = pstCPU_->k_s;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "brlt %d                   \t ; Branch if half-carry set: PC = PC + %d + 1\n",
                s8k,
                s8k
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_BRHC( AVR_CPU *pstCPU_ )
{
    int8_t  s8k = pstCPU_->k_s;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "brhc %d                   \t ; Branch if half-carry clear: PC = PC + %d + 1\n",
                s8k,
                s8k
                );

}

//---------------------------------------------------------------------------
static void AVR_Disasm_BRTS( AVR_CPU *pstCPU_ )
{
    int8_t  s8k = pstCPU_->k_s;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "brts %d                   \t ; Branch if T-flag set: PC = PC + %d + 1\n",
                s8k,
                s8k
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_BRTC( AVR_CPU *pstCPU_ )
{
    int8_t  s8k = pstCPU_->k_s;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "brtc %d                   \t ; Branch if T-flag clear: PC = PC + %d + 1\n",
                s8k,
                s8k
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_BRVS( AVR_CPU *pstCPU_ )
{
    int8_t  s8k = pstCPU_->k_s;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "brvs %d                   \t ; Branch if Overflow set: PC = PC + %d + 1\n",
                s8k,
                s8k
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_BRVC( AVR_CPU *pstCPU_ )
{
    int8_t  s8k = pstCPU_->k_s;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "brvc %d                   \t ; Branch if Overflow clear: PC = PC + %d + 1\n",
                s8k,
                s8k
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_BRIE( AVR_CPU *pstCPU_ )
{
    int8_t  s8k = pstCPU_->k_s;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "brie %d                   \t ; Branch if Interrupt Enabled: PC = PC + %d + 1\n",
                s8k,
                s8k
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_BRID( AVR_CPU *pstCPU_ )
{
    int8_t  s8k = pstCPU_->k_s;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "brid %d                   \t ; Branch if Interrupt Disabled: PC = PC + %d + 1\n",
                s8k,
                s8k
                );

}

//---------------------------------------------------------------------------
static void AVR_Disasm_MOV( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );
    uint8_t u8Rr = Register_From_Rr( pstCPU_ );

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "mov r%d, r%d              \t ; Copy Register: r%d = r%d\n",
                u8Rd, u8Rr,
                u8Rd, u8Rr
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_MOVW( AVR_CPU *pstCPU_ )
{
    uint16_t u16Rd = Register_From_Rd16( pstCPU_ );
    uint16_t u16Rr = Register_From_Rr16( pstCPU_ );

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "movw r%d:r%d, r%d:r%d     \t ; Copy Register (Word): r%d:r%d = r%d:r%d\n",
                u16Rd+1, u16Rd, u16Rr+1, u16Rr,
                u16Rd+1, u16Rd, u16Rr+1, u16Rr
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_LDI( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );
    uint8_t u8K = pstCPU_->K;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "ldi r%d, %d               \t ; Load Immediate: r%d = %d\n",
                u8Rd, u8K,
                u8Rd, u8K
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_LDS( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );
    uint16_t u16k = pstCPU_->k;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "lds r%d, %d               \t ; Load Direct from Data Space: r%d = (%d)\n",
                u8Rd, u16k,
                u8Rd, u16k
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_LD_X_Indirect( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "ld r%d, X                 \t ; Load Indirect from Data Space\n",
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_LD_X_Indirect_Postinc( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "ld r%d, X+                \t ; Load Indirect from Data Space w/Postincrement\n",
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_LD_X_Indirect_Predec( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "ld r%d, -X                \t ; Load Indirect from Data Space w/Predecrement\n",
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_LD_Y_Indirect( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "ld r%d, Y                 \t ; Load Indirect from Data Space\n",
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_LD_Y_Indirect_Postinc( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "ld r%d, Y+                \t ; Load Indirect from Data Space w/Postincrement\n",
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_LD_Y_Indirect_Predec( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "ld r%d, -Y                \t ; Load Indirect from Data Space w/Predecrement\n",
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_LDD_Y( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );
    uint8_t u8q = pstCPU_->q;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "ldd r%d, Y+%d             \t ; Load Indirect from Data Space (with Displacement)\n",
                u8Rd, u8q
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_LD_Z_Indirect( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "ld r%d, Z                 \t ; Load Indirect from Data Space\n",
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_LD_Z_Indirect_Postinc( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "ld r%d, Z+                \t ; Load Indirect from Data Space w/Postincrement\n",
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_LD_Z_Indirect_Predec( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "ld r%d, -Z                \t ; Load Indirect from Data Space w/Predecrement\n",
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_LDD_Z( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );
    uint8_t u8q = pstCPU_->q;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "ldd r%d, Z+%d             \t ; Load Indirect from Data Space (with Displacement)\n",
                u8Rd, u8q
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_STS( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );
    uint16_t u16k = pstCPU_->k;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "sts %d, r%d               \t ; Store Direct to Data Space: (%d) = r%d\n",
                u16k, u8Rd,
                u16k, u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_ST_X_Indirect( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "st X, r%d                 \t ; Store Indirect\n",
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_ST_X_Indirect_Postinc( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "st X+, r%d                \t ; Store Indirect w/Postincrement \n",
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_ST_X_Indirect_Predec( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "st -X, r%d                \t ; Store Indirect w/Predecrement\n",
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_ST_Y_Indirect( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "st Y, r%d                 \t ; Store Indirect\n",
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_ST_Y_Indirect_Postinc( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "st Y+, r%d                \t ; Store Indirect w/Postincrement \n",
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_ST_Y_Indirect_Predec( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "st -Y, r%d                \t ; Store Indirect w/Predecrement\n",
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_STD_Y( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );
    uint8_t u8q = pstCPU_->q;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "std Y+%d, r%d             \t ; Store Indirect from Data Space (with Displacement)\n",
                u8q, u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_ST_Z_Indirect( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "st Z, r%d                 \t ; Store Indirect\n",
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_ST_Z_Indirect_Postinc( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "st Z+, r%d                \t ; Store Indirect w/Postincrement \n",
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_ST_Z_Indirect_Predec( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "st -Z, r%d                \t ; Store Indirect w/Predecrement\n",
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_STD_Z( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );
    uint8_t u8q = pstCPU_->q;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "std Z+%d, r%d             \t ; Store Indirect from Data Space (with Displacement)\n",
                u8q, u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_LPM( AVR_CPU *pstCPU_ )
{
    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "lpm                       \t ; Load Program Memory: r0 = (Z)\n" );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_LPM_Z( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "lpm r%d, Z                \t ; Load Program Memory: r%d = (Z)\n",
                u8Rd,
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_LPM_Z_Postinc( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "lpm r%d, Z+               \t ; Load Program Memory with Postincrement: r%d = (Z), Z = Z + 1\n",
                u8Rd,
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_ELPM( AVR_CPU *pstCPU_ )
{
    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "elpm                      \t ; (Extended) Load Program Memory: r0 = (Z)\n" );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_ELPM_Z( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "elpm r%d, Z               \t ; (Extended) Load Program Memory: r%d = (Z)\n",
                u8Rd,
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_ELPM_Z_Postinc( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "elpm r%d, Z+              \t ; (Extended) Load Program Memory w/Postincrement: r%d = (Z), Z = Z + 1\n",
                u8Rd,
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_SPM( AVR_CPU *pstCPU_ )
{
    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "spm                       \t ; Store Program Memory\n" );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_SPM_Z_Postinc2( AVR_CPU *pstCPU_ )
{
    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "spm Z+                    \t ; Store Program Memory Z = Z + 2 \n" );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_IN( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );
    uint8_t u8A = pstCPU_->A;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "in r%d, %d                \t ; Load an I/O location to register\n",
                u8Rd,
                u8A
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_OUT( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );
    uint8_t u8A = pstCPU_->A;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "out %d, r%d               \t ; Load an I/O location to register\n",
                u8A,
                u8Rd
                );

}

//---------------------------------------------------------------------------
static void AVR_Disasm_LAC( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "lac Z, r%d                   \t ; Load And Clear\n",
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_LAS( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "las Z, r%d                   \t ; Load And Set\n",
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_LAT( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "lat Z, r%d                   \t ; Load And Toggle\n",
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_LSL( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "lsl r%d                   \t ; Logical shift left r%d by 1 bit\n",
                u8Rd,
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_LSR( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "lsr r%d                   \t ; Logical shift right r%d by 1 bit\n",
                u8Rd,
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_POP( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "pop r%d                   \t ; Pop byte from stack into r%d\n",
                u8Rd,
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_PUSH( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "push r%d                  \t ; Push register r%d to stack\n",
                u8Rd,
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_ROL( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "rol r%d                   \t ; Rotate Left through Carry\n",
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_ROR( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "ror r%d                   \t ; Rotate Right through Carry\n",
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_ASR( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "asr r%d                   \t ; Arithmatic Shift Right\n",
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_SWAP( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "swap r%d                  \t ; Swap high/low Nibbles in Register\n",
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_BSET( AVR_CPU *pstCPU_ )
{
    uint8_t u8s = pstCPU_->s;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "bset %d                   \t ; Set bit %d in status register\n",
                u8s,
                u8s
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_BCLR( AVR_CPU *pstCPU_ )
{
    uint8_t u8s = pstCPU_->s;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "bclr %d                   \t ; Clear bit %d in status register\n",
                u8s,
                u8s
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_SBI( AVR_CPU *pstCPU_ )
{
    uint8_t u8b = pstCPU_->b;
    uint8_t u8A = pstCPU_->A;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "sbi %d, %d                \t ; Set bit in I/O register\n",
                u8A,
                u8b
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_CBI( AVR_CPU *pstCPU_ )
{
    uint8_t u8s = pstCPU_->b;
    uint8_t u8A = pstCPU_->A;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "cbi %d, %d                \t ; Clear bit in I/O register\n",
                u8A,
                u8s
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_BST( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );
    uint8_t u8b = pstCPU_->b;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "bst r%d, %d               \t ; Store Bit %d of r%d in the T register\n",
                u8Rd, u8b,
                u8b, u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_BLD( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );
    uint8_t u8b = pstCPU_->b;

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "bld r%d, %d               \t ; Load the T register into Bit %d of r%d\n",
                u8Rd, u8b,
                u8b, u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_SEC( AVR_CPU *pstCPU_ )
{
    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "sec                       \t ; Set the carry flag in the SR\n" );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_CLC( AVR_CPU *pstCPU_ )
{
    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "clc                       \t ; Clear the carry flag in the SR\n" );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_SEN( AVR_CPU *pstCPU_ )
{
    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "sen                       \t ; Set the negative flag in the SR\n" );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_CLN( AVR_CPU *pstCPU_ )
{
    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "cln                       \t ; Clear the negative flag in the SR\n" );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_SEZ( AVR_CPU *pstCPU_ )
{
    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "sez                       \t ; Set the zero flag in the SR\n" );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_CLZ( AVR_CPU *pstCPU_ )
{
    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "clz                       \t ; Clear the zero flag in the SR\n" );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_SEI( AVR_CPU *pstCPU_ )
{
    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "sei                       \t ; Enable MCU interrupts\n" );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_CLI( AVR_CPU *pstCPU_ )
{
    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "cli                       \t ; Disable MCU interrupts\n" );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_SES( AVR_CPU *pstCPU_ )
{
    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "ses                       \t ; Set the sign flag in the SR\n" );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_CLS( AVR_CPU *pstCPU_ )
{
    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "cls                       \t ; Clear the sign flag in the SR\n" );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_SEV( AVR_CPU *pstCPU_ )
{
    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "sev                       \t ; Set the overflow flag in the SR\n" );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_CLV( AVR_CPU *pstCPU_ )
{
    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "clv                       \t ; Clear the overflow flag in the SR\n" );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_SET( AVR_CPU *pstCPU_ )
{
    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "set                       \t ; Set the T-flag in the SR\n" );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_CLT( AVR_CPU *pstCPU_ )
{
    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "clt                       \t ; Clear the T-flag in the SR\n" );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_SEH( AVR_CPU *pstCPU_ )
{
    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "seh                       \t ; Set half-carry flag in SR\n" );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_CLH( AVR_CPU *pstCPU_ )
{
    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "clh                       \t ; Clear half-carry flag in SR\n" );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_BREAK( AVR_CPU *pstCPU_ )
{
    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "break                     \t ; Halt for debugger\n" );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_NOP( AVR_CPU *pstCPU_ )
{
    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "nop                       \t ; Do nothing\n" );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_SLEEP( AVR_CPU *pstCPU_ )
{
    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "sleep                     \t ; Put MCU into sleep mode\n" );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_WDR( AVR_CPU *pstCPU_ )
{
    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "wdr                       \t ; Reset Watchdog Timer\n" );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_XCH( AVR_CPU *pstCPU_ )
{
    uint8_t u8Rd = Register_From_Rd( pstCPU_ );

    //ruler: 0----5----10---15---20---25---30---35---40" );
    printf( "xch Z, r%d                \t ; Exchange registers w/memory\n",
                u8Rd
                );
}

//---------------------------------------------------------------------------
static void AVR_Disasm_Unimplemented(  AVR_CPU *pstCPU_ )
{
    printf( ".db 0x%04X ; Data (not an opcode)\n", pstCPU_->pu16ROM[ pstCPU_->u16PC ] );
}

//---------------------------------------------------------------------------
AVR_Opcode AVR_Disasm_Function( uint16_t OP_ )
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

