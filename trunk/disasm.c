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
  \file  disasm.c

  \brief Main AVR disassembler entrypoint
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

//---------------------------------------------------------------------------
#include "avr_coreregs.h"
#include "avr_periphregs.h"
#include "avr_op_cycles.h"
#include "avr_op_decode.h"
#include "avr_op_size.h"
#include "avr_cpu_print.h"
#include "avr_cpu.h"
#include "avr_disasm.h"

//---------------------------------------------------------------------------
int main(void)
{
    AVR_CPU stCPU;
    int i;
    CPU_Init(&stCPU);

    AVR_Load_HEX( &stCPU, "Markade2.hex");

    stCPU.u16PC = 0;
    i = 0;
    while (stCPU.u16PC < 16384)
    {
        uint16_t OP = stCPU.pusROM[stCPU.u16PC];
        printf("0x%04X: [0x%04X] ", stCPU.u16PC, OP);
        AVR_Decode(&stCPU, OP);
        AVR_Disasm_Function(OP)(&stCPU);
        stCPU.u16PC += AVR_Opcode_Size(OP);
    }

    return 0;
    print_core_regs(&stCPU);
    print_io_reg_with_name( &stCPU, 0x24, "DDRB" );
    print_io_reg( &stCPU, 0x24 );
    print_ram( &stCPU, 256, 255 );
    print_rom( &stCPU, 512, 512 );
    return 0;

}
