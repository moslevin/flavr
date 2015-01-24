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

#include "kernel_aware.h"
#include "debug_sym.h"
#include "write_callout.h"
#include "interrupt_callout.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/*!
  Things we would want from a kernel-aware plugin..

  1) Trace execution priority
    -- On modification of g_pstCurrent (And on interrupt/reti)
    -- Log cycle count and thread's priority.

  2) Trace execution thread
    -- On modification of g_pstCurrent

  3) Interrupt latency
    -- After first interrupt enable, track time from int to reti.

  4) CPU Loading
    -- Track total CPU cycles, vs cycles spent in Idle thread

  5) Interrupt loading
    -- Track CPU cycles, vs cycles spent in interrupts

*/


//---------------------------------------------------------------------------
typedef struct
{
    //! Link list pointers
    uint16_t u16NextPtr;
    uint16_t u16PrevPtr;

    //! Pointer to the top of the thread's stack
    uint16_t u16StackTopPtr;

    //! Pointer to the thread's stack
    uint16_t u16StackPtr;

    //! Size of the stack (in bytes)
    uint16_t u16StackSize;

    //! Thread quantum (in milliseconds)
    uint16_t u16Quantum;

    //! Thread ID
    uint8_t u8ThreadID;

    //! Default priority of the thread
    uint8_t u8Priority;

    //! Current priority of the thread (priority inheritence)
    uint8_t u8CurPriority;

    //! The entry-point function called when the thread starts
    uint16_t u16EntryPoint;

    //! Pointer to the argument passed into the thread's entrypoint
    void *m_pvArg;

} Mark3_Thread_t;


//---------------------------------------------------------------------------
static Mark3_Thread_t *Mark3KA_GetCurrentThread(void)
{
    Debug_Symbol_t *pstSymbol = 0;
    pstSymbol = Symbol_Find_Obj_By_Name( "g_pstCurrent" );

    // Use pstSymbol's address to get a pointer to the current thread.
    if (!pstSymbol)
    {
        return 0;
    }

    uint16_t u16CurrPtr = (uint16_t)(pstSymbol->u32StartAddr & 0x0000FFFF);
    if (!u16CurrPtr)
    {
        return 0;
    }

    // Now that we have the address of g_pstCurrent, dereference the pointer
    // to get the address of the current thread.
#if 0
    printf( "g_pstCurrent = @ 0x%04X\n", u16CurrPtr );
    printf( "  %02X %02X\n", stCPU.pstRAM->au8RAM[ u16CurrPtr ], stCPU.pstRAM->au8RAM[ u16CurrPtr + 1 ] );
#endif

    uint16_t u16CurrAddr = ((uint16_t)(stCPU.pstRAM->au8RAM[ u16CurrPtr + 1 ]) << 8) +
                            stCPU.pstRAM->au8RAM[ u16CurrPtr ];

#if 0
    printf( "Current Thread @ 0x%04X\n", u16CurrAddr );
#endif

    // Return a pointer to the thread as it is in memory.
    return (Mark3_Thread_t*)(&stCPU.pstRAM->au8RAM[ u16CurrAddr ]);
}

//---------------------------------------------------------------------------
static uint8_t Mark3KA_GetCurrentPriority(void)
{
    Mark3_Thread_t *pstThread = Mark3KA_GetCurrentThread();
    if (!pstThread)
    {
        return 0;
    }
    uint8_t *pucData = (uint8_t*)pstThread;

    // If the curpriority member is set, it means we're in the middle of
    // priority inheritence.  If it's zero, return the normal priority
    if (0 == pstThread->u8CurPriority)
    {
        return pstThread->u8Priority;
    }
    return pstThread->u8CurPriority;
}

//---------------------------------------------------------------------------
static void KA_ThreadChange( uint16_t u16Addr_, uint8_t u8Data_ )
{
    uint8_t u8Pri = Mark3KA_GetCurrentPriority();
    uint8_t u8Thread = Mark3KA_GetCurrentThread()->u8ThreadID;

    printf( "%llu, %d, %d\n", stCPU.u64CycleCount, u8Thread, u8Pri );
}

//---------------------------------------------------------------------------
static void KA_Interrupt( bool bEntry_ )
{
    if (bEntry_)
    {
        printf( "%llu, %d, %d\n", stCPU.u64CycleCount, -1, 8 );
    }
    else
    {
        uint8_t u8Pri = Mark3KA_GetCurrentPriority();
        uint8_t u8Thread = Mark3KA_GetCurrentThread()->u8ThreadID;

        printf( "%llu, %d, %d\n", stCPU.u64CycleCount, u8Thread, u8Pri );
    }
}

//---------------------------------------------------------------------------
void KernelAware_Init( void )
{
    Debug_Symbol_t *pstSymbol = 0;
    pstSymbol = Symbol_Find_Obj_By_Name( "g_pstCurrent" );

    // Use pstSymbol's address to get a pointer to the current thread.
    if (!pstSymbol)
    {
        return;
    }

    uint16_t u16CurrPtr = (uint16_t)(pstSymbol->u32StartAddr & 0x0000FFFF);
    if (!u16CurrPtr)
    {
        return;
    }

    // Add a callback so that when g_pstCurrent changes, we can update our
    // locally-tracked statistics.
    WriteCallout_Add( KA_ThreadChange , u16CurrPtr + 1 );

    InterruptCallout_Add( KA_Interrupt );
}
