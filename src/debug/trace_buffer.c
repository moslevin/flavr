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
 * (c) Copyright 2014-15, Funkenstein Software Consulting, All rights reserved
 *     See license.txt for details
 ****************************************************************************/
/*!
  \file  trace_buffer.c

  \brief Implements a circular buffer containing a history of recently
         executed instructions, along with core register context for
         each
*/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "trace_buffer.h"
#include "emu_config.h"

#include "avr_disasm.h"
#include "avr_op_decode.h"

//---------------------------------------------------------------------------
void TraceBuffer_Init( TraceBuffer_t *pstTraceBuffer_ )
{
    memset( pstTraceBuffer_, 0, sizeof(*pstTraceBuffer_) );
}

//---------------------------------------------------------------------------
void TraceBuffer_StoreFromCPU( TraceBuffer_t *pstTraceBuffer_  )
{
    TraceElement_t *pstTraceElement = &pstTraceBuffer_->astTraceStep[ pstTraceBuffer_->u32Index ];

    // Manually copy over whatever elements we need to
    pstTraceElement->u64Counter    = stCPU.u64InstructionCount;
    pstTraceElement->u64CycleCount = stCPU.u64CycleCount;
    pstTraceElement->u16PC         = stCPU.u16PC;
    pstTraceElement->u16SP         = ((uint16_t)(stCPU.pstRAM->stRegisters.SPH.r) << 8) |
                                      (uint16_t)(stCPU.pstRAM->stRegisters.SPL.r);

    pstTraceElement->u16OpCode     = stCPU.pu16ROM[ stCPU.u16PC ];
    pstTraceElement->u8SR          = stCPU.pstRAM->stRegisters.SREG.r;

    // Memcpy the core registers in one chunk
    memcpy(&(pstTraceElement->stCoreRegs), &(stCPU.pstRAM->stRegisters.CORE_REGISTERS), sizeof(pstTraceElement->stCoreRegs));

    // Update the index of the write buffer
    pstTraceBuffer_->u32Index++;
    if (pstTraceBuffer_->u32Index >= CONFIG_TRACEBUFFER_SIZE)
    {
        pstTraceBuffer_->u32Index = 0;
    }
}

//---------------------------------------------------------------------------
void TraceBuffer_LoadElement( TraceBuffer_t *pstTraceBuffer_, TraceElement_t *pstElement_, uint32_t u32Element_ )
{
    TraceElement_t *pstSourceElement = &pstTraceBuffer_->astTraceStep[ pstTraceBuffer_->u32Index ];

    memcpy(pstElement_, pstSourceElement, sizeof(*pstElement_));
}

//---------------------------------------------------------------------------
void TraceBuffer_PrintElement( TraceElement_t *pstElement_, TracePrintFormat_t eFormat_  )
{
    printf( "[%08d] 0x%04X:0x%04X: ",
            pstElement_->u64Counter, pstElement_->u16PC, pstElement_->u16OpCode );
    if (eFormat_ & TRACE_PRINT_DISASSEMBLY)
    {
        uint16_t u16TempPC = stCPU.u16PC;
        stCPU.u16PC = pstElement_->u16PC;

        AVR_Disasm pfOp = AVR_Disasm_Function( pstElement_->u16OpCode );

        char szBuf[256];
        AVR_Decode( pstElement_->u16OpCode );        
        pfOp( szBuf );
        printf( "%s", szBuf );

        stCPU.u16PC = u16TempPC;
    }

    if (eFormat_ & TRACE_PRINT_COMPACT)
    {
        printf( "%04X ", pstElement_->u16SP );

        int i;
        for (i = 0; i < 32; i++)
        {
            printf( "%02X ", pstElement_->stCoreRegs.r[i] );
        }
        printf( "\n" );
    }
    if (eFormat_ & TRACE_PRINT_REGISTERS)
    {
        uint8_t i;
        for (i = 0; i < 32; i++)
        {
            printf( "[R%02d] = 0x%02X\n", i, pstElement_->stCoreRegs.r[i] );
        }
        printf("[SP]  = 0x%04X\n", pstElement_->u16SP );
        printf("[PC]  = 0x%04X\n", (uint16_t)pstElement_->u16PC );
        printf("[SREG]= 0x%02X", pstElement_->u8SR );
        printf( "\n" );
    }
}

//---------------------------------------------------------------------------
void TraceBuffer_Print( TraceBuffer_t *pstTraceBuffer_, TracePrintFormat_t eFormat_ )
{
    int i;
    for (i = pstTraceBuffer_->u32Index; i < CONFIG_TRACEBUFFER_SIZE; i++)
    {
        TraceBuffer_PrintElement(&pstTraceBuffer_->astTraceStep[i], eFormat_ );
    }
    for (i = 0; i < pstTraceBuffer_->u32Index; i++)
    {
        TraceBuffer_PrintElement(&pstTraceBuffer_->astTraceStep[i], eFormat_ );
    }
}
