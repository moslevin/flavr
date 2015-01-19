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
/*!
    Struct defining the CPU's running state at each tracebuffer sample point.
*/
typedef struct
{
    uint64_t    u64Counter;         //!< Instruction counter
    uint64_t    u64CycleCount;      //!< CPU Cycle counter
    uint16_t    u16OpCode;          //!< opcode @ trace sample
    uint16_t    u16PC;              //!< program counter @ trace sample
    uint16_t    u16SP;              //!< stack pointer @ trace sample
    uint8_t     u8SR;               //!< status register @ trace sample

    AVR_CoreRegisters stCoreRegs;   //!< core CPU registers @ trace sample

} TraceElement_t;

//---------------------------------------------------------------------------
/*!
    Implements a circular buffer of trace elements, sized according to the 
    compile-time configuration.
*/
typedef struct
{
    TraceElement_t  astTraceStep[ CONFIG_TRACEBUFFER_SIZE ];    //!< Array of trace samples
    uint32_t        u32Index;                                   //!< Current sample index
} TraceBuffer_t;

//---------------------------------------------------------------------------
/*!
    Enumerated values defining the various formats for printing/displaying 
    tracebuffer information.
*/
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
 */
void TraceBuffer_StoreFromCPU( TraceBuffer_t *pstTraceBuffer_ );

//---------------------------------------------------------------------------
/*!
 * \brief TraceBuffer_LoadElement Load an element from the tracebuffer into a
 *        a specified output element.
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
void TraceBuffer_PrintElement( TraceElement_t *pstElement_, TracePrintFormat_t eFormat_ );

//---------------------------------------------------------------------------
/*!
 * \brief TraceBuffer_Print Print the raw contents of a tracebuffer to standard
 *        output.
 *
 * \param pstTraceBuffer_ Pointer to the tracebuffer to print
 *
 * \param eFormat_ Formatting type for the print
 */
void TraceBuffer_Print( TraceBuffer_t *pstTraceBuffer_, TracePrintFormat_t eFormat_ );

#endif
