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
  \file  trace_buffer.h

  \brief Implements a circular buffer containing a history of recently
         executed instructions, along with core register context for
         each
*/

#ifndef __TRACE_BUFFER_H__
#define __TRACE_BUFFER_H__

#include <stdint.h>

#include "emu_config.h"
#include "avr_cpu.h"

//---------------------------------------------------------------------------
typedef struct
{
    uint64_t    u64Counter;
    uint64_t    u64CycleCount;
    uint16_t    u16OpCode;
    uint16_t    u16PC;
    uint16_t    u16SP;
    uint8_t     u8SR;

    AVR_CoreRegisters stCoreRegs;

} TraceElement_t;

//---------------------------------------------------------------------------
typedef struct
{
    TraceElement_t  astTraceStep[ CONFIG_TRACEBUFFER_SIZE ];
    uint32_t        u32Index;
} TraceBuffer_t;


//---------------------------------------------------------------------------
typedef enum
{
    TRACE_PRINT_COMPACT     = 1,
    TRACE_PRINT_REGISTERS   = 2,
    TRACE_PRINT_DISASSEMBLY = 4
} TracePrintFormat_t;

//---------------------------------------------------------------------------
/*!
 * \brief TraceBuffer_Init Initialize a tracebuffer prior to use
 *
 * \param pstTraceBuffer_ Pointer to the tracebuffer to initialize
 */
void TraceBuffer_Init( TraceBuffer_t *pstTraceBuffer_ );

//---------------------------------------------------------------------------
/*!
 * \brief TraceBuffer_StoreFromCPU Store a trace element in the tracebuffer at
 *        its current head index.
 *
 * \param pstTraceBuffer_ Pointer to the tracebuffer to store into
 *
 * \param pstCPU_ Pointer to an AVR CPU struct from which to store the trace data
 */
void TraceBuffer_StoreFromCPU( TraceBuffer_t *pstTraceBuffer_, AVR_CPU *pstCPU_ );

//---------------------------------------------------------------------------
/*!
 * \brief TraceBuffer_LoadElement Load an element from the tracebuffer into a
 *        a specified
 *
 * \param pstTraceBuffer_ Pointer to a tracebuffer to load from
 *
 * \param pstElement_ Pointer to a trace element structure to store data into
 *
 * \param u32Element_ Index of the element in the tracebuffer to read
 */
void TraceBuffer_LoadElement( TraceBuffer_t *pstTraceBuffer_, TraceElement_t *pstElement_, uint32_t u32Element_ );

//---------------------------------------------------------------------------
/*!
 * \brief TraceBuffer_PrintElement Print a single element from a tracebuffer
 *        to standard output.  This prints core registers and addresses.
 * \param pstElement_ Pointer to the trace element to print
 * * \param eFormat_ Formatting type for the print
 */
void TraceBuffer_PrintElement( TraceElement_t *pstElement_, TracePrintFormat_t eFormat_, AVR_CPU *pstCPU_ );

//---------------------------------------------------------------------------
/*!
 * \brief TraceBuffer_Print Print the raw contents of a tracebuffer to standard
 *        output.
 *
 * \param pstTraceBuffer_ Pointer to the tracebuffer to print
 *
 * \param eFormat_ Formatting type for the print
 */
void TraceBuffer_Print( TraceBuffer_t *pstTraceBuffer_, TracePrintFormat_t eFormat_, AVR_CPU *pstCPU_ );

#endif
