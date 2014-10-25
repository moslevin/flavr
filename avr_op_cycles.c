/****************************************************************************
 *     (     (                      (     |
 *    )\ )  )\ )    (              )\ )   |
 *   (()/( (()/(    )\     (   (  (()/(   | -- [ Funkenstein ] -------------
 *    /(_)) /(_))((((_)()\  )\  /(_))  | -- [ Litle ] -------------------
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
  \file  avr_op_cycles.c

  \brief Opcode cycle counting functions.
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

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_ADD()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_ADC()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_ADIW()
{
    return 2;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_SUB()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_SUBI()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_SBC()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_SBCI()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_SBIW()
{
    return 2;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_AND()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_ANDI()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_OR()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_ORI()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_EOR()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_COM()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_NEG()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_SBR()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_CBR()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_INC()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_DEC()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_TST()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_CLR()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_SER()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_MUL()
{
    return 2;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_MULS()
{
    return 2;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_MULSU()
{
    return 2;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_FMUL()
{
    return 2;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_FMULS()
{
    return 2;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_FMULSU()
{
    return 2;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_DES()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_RJMP()
{
    return 2;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_IJMP()
{
    return 2;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_EIJMP()
{
    return 2;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_JMP()
{
    return 2;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_RCALL()
{
    return 3; //!! ToDo -- n cycles on devices w/22-bit PC
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_ICALL()
{
    return 3; //!! ToDo -- n cycles on devices w/22-bit PC
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_EICALL()
{
    return 4;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_CALL()
{
    return 4; //!! ToDo -- 5 cycles on devices w/22-bit PC
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_RET()
{
    return 4; //!! ToDo -- 5 cycles on devices w/22-bit PC
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_RETI()
{
    return 4; //!! ToDo -- 5 cycles on devices w/22-bit PC
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_CPSE()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_CP()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_CPC()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_CPI()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_SBRC()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_SBRS()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_SBIC()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_SBIS()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_BRBS()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_BRBC()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_BREQ()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_BRNE()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_BRCS()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_BRCC()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_BRSH()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_BRLO()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_BRMI()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_BRPL()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_BRGE()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_BRLT()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_BRHS()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_BRHC()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_BRTS()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_BRTC()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_BRVS()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_BRVC()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_BRIE()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_BRID()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_MOV()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_MOVW()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_LDI()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_LDS()
{
    return 2;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_LD_X_Indirect()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_LD_X_Indirect_Postinc()
{
    return 2;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_LD_X_Indirect_Predec()
{
    return 3;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_LD_Y_Indirect()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_LD_Y_Indirect_Postinc()
{
    return 2;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_LD_Y_Indirect_Predec()
{
    return 3;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_LDD_Y()
{
    return 2;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_LD_Z_Indirect()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_LD_Z_Indirect_Postinc()
{
    return 2; //!! ToDo - Cycles on XMEGA/tinyAVR
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_LD_Z_Indirect_Predec()
{
    return 3; //!! ToDo - Cycles on XMEGA/tinyAVR
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_LDD_Z()
{
    return 2;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_STS()
{
    return 2;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_ST_X_Indirect()
{
    return 2; //!! ToDo - Cycles on XMEGA/tinyAVR
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_ST_X_Indirect_Postinc()
{
    return 2; //!! ToDo - Cycles on XMEGA/tinyAVR
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_ST_X_Indirect_Predec()
{
    return 2; //!! ToDo - Cycles on XMEGA/tinyAVR
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_ST_Y_Indirect()
{
    return 2; //!! ToDo - Cycles on XMEGA/tinyAVR
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_ST_Y_Indirect_Postinc()
{
    return 2; //!! ToDo - Cycles on XMEGA/tinyAVR
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_ST_Y_Indirect_Predec()
{
    return 2; //!! ToDo - Cycles on XMEGA/tinyAVR
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_STD_Y()
{
    return 2; //!! ToDo - Cycles on XMEGA/tinyAVR
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_ST_Z_Indirect()
{
    return 2; //!! ToDo - Cycles on XMEGA/tinyAVR
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_ST_Z_Indirect_Postinc()
{
    return 2; //!! ToDo - Cycles on XMEGA/tinyAVR
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_ST_Z_Indirect_Predec()
{
    return 2; //!! ToDo - Cycles on XMEGA/tinyAVR
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_STD_Z()
{
    return 2; //!! ToDo - Cycles on XMEGA/tinyAVR
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_LPM()
{
    return 3;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_LPM_Z()
{
    return 3;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_LPM_Z_Postinc()
{
    return 3;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_ELPM()
{
    return 3;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_ELPM_Z()
{
    return 3;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_ELPM_Z_Postinc()
{
    return 3;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_SPM()
{
    return 2; //!!ToDo - Datasheet says "Depends on the operation"...
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_SPM_Z_Postinc2()
{
    return 2; //!!ToDo - Datasheet says "Depends on the operation"...
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_IN()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_OUT()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_LAC()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_LAS()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_LAT()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_LSL()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_LSR()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_POP()
{
    return 2;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_PUSH()
{
    return 2;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_ROL()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_ROR()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_ASR()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_SWAP()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_BSET()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_BCLR()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_SBI()
{
    return 2;   //!! ToDo - take into account XMEGA/tinyAVR timing
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_CBI()
{
    return 2;   //!! ToDo - take into account XMEGA/tinyAVR timing
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_BST()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_BLD()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_SEC()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_CLC()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_SEN()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_CLN()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_SEZ()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_CLZ()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_SEI()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_CLI()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_SES()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_CLS()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_SEV()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_CLV()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_SET()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_CLT()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_SEH()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_CLH()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_BREAK()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_NOP()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_SLEEP()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_WDR()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_XCH()
{
    return 1;
}

//---------------------------------------------------------------------------
static uint8_t AVR_Opcode_Cycles_Unimplemented()
{
    return 1;
}

//---------------------------------------------------------------------------
uint8_t AVR_Opcode_Cycles( uint16_t OP_ )
{
    // Special instructions - "static" encoding
    switch (OP_)
    {
    case 0x0000: return AVR_Opcode_Cycles_NOP();

    case 0x9408: return AVR_Opcode_Cycles_SEC();
    case 0x9409: return AVR_Opcode_Cycles_IJMP();
    case 0x9418: return AVR_Opcode_Cycles_SEZ();
    case 0x9419: return AVR_Opcode_Cycles_EIJMP();
    case 0x9428: return AVR_Opcode_Cycles_SEN();
    case 0x9438: return AVR_Opcode_Cycles_SEV();
    case 0x9448: return AVR_Opcode_Cycles_SES();
    case 0x9458: return AVR_Opcode_Cycles_SEH();
    case 0x9468: return AVR_Opcode_Cycles_SET();
    case 0x9478: return AVR_Opcode_Cycles_SEI();

    case 0x9488: return AVR_Opcode_Cycles_CLC();
    case 0x9498: return AVR_Opcode_Cycles_CLZ();
    case 0x94A8: return AVR_Opcode_Cycles_CLN();
    case 0x94B8: return AVR_Opcode_Cycles_CLV();
    case 0x94C8: return AVR_Opcode_Cycles_CLS();
    case 0x94D8: return AVR_Opcode_Cycles_CLH();
    case 0x94E8: return AVR_Opcode_Cycles_CLT();
    case 0x94F8: return AVR_Opcode_Cycles_CLI();

    case 0x9508: return AVR_Opcode_Cycles_RET();
    case 0x9509: return AVR_Opcode_Cycles_ICALL();
    case 0x9518: return AVR_Opcode_Cycles_RETI();
    case 0x9519: return AVR_Opcode_Cycles_EICALL();
    case 0x9588: return AVR_Opcode_Cycles_SLEEP();
    case 0x9598: return AVR_Opcode_Cycles_BREAK();
    case 0x95A8: return AVR_Opcode_Cycles_WDR();
    case 0x95C8: return AVR_Opcode_Cycles_LPM();
    case 0x95D8: return AVR_Opcode_Cycles_ELPM();
    case 0x95E8: return AVR_Opcode_Cycles_SPM();
    case 0x95F8: return AVR_Opcode_Cycles_SPM_Z_Postinc2();
    }

#if 0
    // Note: These disasm handlers are generalized versions of specific mnemonics in the above list.
    // For disassembly, it's probably easier to read the output from the more "spcific" mnemonics, so
    // those are used.  For emulation, using the generalized functions may be more desirable.
    switch( OP_ & 0xFF8F)
    {
    case 0x9408: return AVR_Opcode_Cycles_BSET();
    case 0x9488: return AVR_Opcode_Cycles_BCLR();
    }
#endif

    switch (OP_ & 0xFF88)
    {
    case 0x0300: return AVR_Opcode_Cycles_MULSU();
    case 0x0308: return AVR_Opcode_Cycles_FMUL();
    case 0x0380: return AVR_Opcode_Cycles_FMULS();
    case 0x0388: return AVR_Opcode_Cycles_FMULSU();
    }

    switch (OP_ & 0xFF0F)
    {
    case 0x940B: return AVR_Opcode_Cycles_DES();
    case 0xEF0F: return AVR_Opcode_Cycles_SER();
    }

    switch (OP_ & 0xFF00)
    {
    case 0x0100: return AVR_Opcode_Cycles_MOVW();
    case 0x9600: return AVR_Opcode_Cycles_ADIW();
    case 0x9700: return AVR_Opcode_Cycles_SBIW();

    case 0x9800: return AVR_Opcode_Cycles_CBI();
    case 0x9900: return AVR_Opcode_Cycles_SBIC();
    case 0x9A00: return AVR_Opcode_Cycles_SBI();
    case 0x9B00: return AVR_Opcode_Cycles_SBIS();
    }

    switch (OP_ & 0xFE0F)
    {
    case 0x8008: return AVR_Opcode_Cycles_LD_Y_Indirect();
    case 0x8000: return AVR_Opcode_Cycles_LD_Z_Indirect();
    case 0x8200: return AVR_Opcode_Cycles_ST_Z_Indirect();
    case 0x8208: return AVR_Opcode_Cycles_ST_Y_Indirect();

    // -- Single 5-bit register...
    case 0x9000: return AVR_Opcode_Cycles_LDS();
    case 0x9001: return AVR_Opcode_Cycles_LD_Z_Indirect_Postinc();
    case 0x9002: return AVR_Opcode_Cycles_LD_Z_Indirect_Predec();
    case 0x9004: return AVR_Opcode_Cycles_LPM_Z();
    case 0x9005: return AVR_Opcode_Cycles_LPM_Z_Postinc();
    case 0x9006: return AVR_Opcode_Cycles_ELPM_Z();
    case 0x9007: return AVR_Opcode_Cycles_ELPM_Z_Postinc();
    case 0x9009: return AVR_Opcode_Cycles_LD_Y_Indirect_Postinc();
    case 0x900A: return AVR_Opcode_Cycles_LD_Y_Indirect_Predec();
    case 0x900C: return AVR_Opcode_Cycles_LD_X_Indirect();
    case 0x900D: return AVR_Opcode_Cycles_LD_X_Indirect_Postinc();
    case 0x900E: return AVR_Opcode_Cycles_LD_X_Indirect_Predec();
    case 0x900F: return AVR_Opcode_Cycles_POP();

    case 0x9200: return AVR_Opcode_Cycles_STS();
    case 0x9201: return AVR_Opcode_Cycles_ST_Z_Indirect_Postinc();
    case 0x9202: return AVR_Opcode_Cycles_ST_Z_Indirect_Predec();
    case 0x9204: return AVR_Opcode_Cycles_XCH();
    case 0x9205: return AVR_Opcode_Cycles_LAS();
    case 0x9206: return AVR_Opcode_Cycles_LAC();
    case 0x9207: return AVR_Opcode_Cycles_LAT();
    case 0x9209: return AVR_Opcode_Cycles_ST_Y_Indirect_Postinc();
    case 0x920A: return AVR_Opcode_Cycles_ST_Y_Indirect_Predec();
    case 0x920C: return AVR_Opcode_Cycles_ST_X_Indirect();
    case 0x920D: return AVR_Opcode_Cycles_ST_X_Indirect_Postinc();
    case 0x920E: return AVR_Opcode_Cycles_ST_X_Indirect_Predec();
    case 0x920F: return AVR_Opcode_Cycles_PUSH();

    // -- One-operand instructions
    case 0x9400: return AVR_Opcode_Cycles_COM();
    case 0x9401: return AVR_Opcode_Cycles_NEG();
    case 0x9402: return AVR_Opcode_Cycles_SWAP();
    case 0x9403: return AVR_Opcode_Cycles_INC();
    case 0x9405: return AVR_Opcode_Cycles_ASR();
    case 0x9406: return AVR_Opcode_Cycles_LSR();
    case 0x9407: return AVR_Opcode_Cycles_ROR();
    case 0x940A: return AVR_Opcode_Cycles_DEC();

    }
    switch (OP_ & 0xFE0E)
    {
    case 0x940C: return AVR_Opcode_Cycles_JMP();
    case 0x940E: return AVR_Opcode_Cycles_CALL();
    }

    switch (OP_ & 0xFE08)
    {

    // -- BLD/BST Encoding
    case 0xF800: return AVR_Opcode_Cycles_BLD();
    case 0xFA00: return AVR_Opcode_Cycles_BST();
    // -- SBRC/SBRS Encoding
    case 0xFC00: return AVR_Opcode_Cycles_SBRC();
    case 0xFE00: return AVR_Opcode_Cycles_SBRS();
    }

    switch (OP_ & 0xFC07)
    {
    // -- Conditional branches
    case 0xF000: return AVR_Opcode_Cycles_BRCS();
    // case 0xF000: return AVR_Opcode_Cycles_BRLO();            // AKA AVR_Opcode_Cycles_BRCS();
    case 0xF001: return AVR_Opcode_Cycles_BREQ();
    case 0xF002: return AVR_Opcode_Cycles_BRMI();
    case 0xF003: return AVR_Opcode_Cycles_BRVS();
    case 0xF004: return AVR_Opcode_Cycles_BRLT();
    case 0xF006: return AVR_Opcode_Cycles_BRTS();
    case 0xF007: return AVR_Opcode_Cycles_BRIE();
    case 0xF400: return AVR_Opcode_Cycles_BRCC();
    // case 0xF400: return AVR_Opcode_Cycles_BRSH();            // AKA AVR_Opcode_Cycles_BRCC();
    case 0xF401: return AVR_Opcode_Cycles_BRNE();
    case 0xF402: return AVR_Opcode_Cycles_BRPL();
    case 0xF403: return AVR_Opcode_Cycles_BRVC();
    case 0xF404: return AVR_Opcode_Cycles_BRGE();
    case 0xF405: return AVR_Opcode_Cycles_BRHC();
    case 0xF406: return AVR_Opcode_Cycles_BRTC();
    case 0xF407: return AVR_Opcode_Cycles_BRID();
    }

    switch (OP_ & 0xFC00)
    {
    // -- 4-bit register pair
    case 0x0200: return AVR_Opcode_Cycles_MULS();

    // -- 5-bit register pairs --
    case 0x0400: return AVR_Opcode_Cycles_CPC();
    case 0x0800: return AVR_Opcode_Cycles_SBC();
    case 0x0C00: return AVR_Opcode_Cycles_ADD();
    // case 0x0C00: return AVR_Opcode_Cycles_LSL(); (!! Implemented with: " add rd, rd"
    case 0x1000: return AVR_Opcode_Cycles_CPSE();
    case 0x1300: return AVR_Opcode_Cycles_ROL();
    case 0x1400: return AVR_Opcode_Cycles_CP();
    case 0x1C00: return AVR_Opcode_Cycles_ADC();
    case 0x1800: return AVR_Opcode_Cycles_SUB();
    case 0x2000: return AVR_Opcode_Cycles_AND();
    // case 0x2000: return AVR_Opcode_Cycles_TST(); (!! Implemented with: " and rd, rd"
    case 0x2400: return AVR_Opcode_Cycles_EOR();
    case 0x2C00: return AVR_Opcode_Cycles_MOV();
    case 0x2800: return AVR_Opcode_Cycles_OR();

    // -- 5-bit register pairs -- Destination = R1:R0
    case 0x9C00: return AVR_Opcode_Cycles_MUL();
    }

    switch (OP_ & 0xF800)
    {
    case  0xB800: return AVR_Opcode_Cycles_OUT();
    case  0xB000: return AVR_Opcode_Cycles_IN();
    }

    switch (OP_ & 0xF000)
    {
    // -- Register immediate --
    case 0x3000: return AVR_Opcode_Cycles_CPI();
    case 0x4000: return AVR_Opcode_Cycles_SBCI();
    case 0x5000: return AVR_Opcode_Cycles_SUBI();
    case 0x6000: return AVR_Opcode_Cycles_ORI();
    case 0x7000: return AVR_Opcode_Cycles_ANDI();

    //-- 12-bit immediate
    case 0xC000: return AVR_Opcode_Cycles_RJMP();
    case 0xD000: return AVR_Opcode_Cycles_RCALL();

    // -- Register immediate
    case 0xE000: return AVR_Opcode_Cycles_LDI();
    }

    switch (OP_ & 0xD208)
    {
    // -- 7-bit signed offset
    case 0x8000: return AVR_Opcode_Cycles_LDD_Z();
    case 0x8008: return AVR_Opcode_Cycles_LDD_Y();
    case 0x8200: return AVR_Opcode_Cycles_STD_Z();
    case 0x8208: return AVR_Opcode_Cycles_STD_Y();
    }

    return AVR_Opcode_Cycles_Unimplemented();
}

