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
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*!
  Things we would want from a kernel-aware plugin..

  1) Trace execution priority (check!)
    -- On modification of g_pstCurrent (And on interrupt/reti)
    -- Log cycle count and thread's priority.

  2) Trace execution thread (check!)
    -- On modification of g_pstCurrent

  3) Interrupt latency (check!)
    -- After first interrupt enable, track time from int to reti.
    -- !!ToDo -- check to make sure nesting and multiple pending interrupts work properly.

  4) CPU Loading (check!)
    -- Track total CPU cycles, vs cycles spent in Idle thread

  5) Interrupt loading (check!)
    -- Track CPU cycles, vs cycles spent in interrupts

  6) Stack usage (check!)
    -- Track the # of bytes free stack (stack margin) remaining in the current thread

  7) Monitor for stack overflow (check!)
    -- Print warnings whenever a thread's stack reaches critical utilization
*/


//!! Todo - Logic to ensure we don't mess things up do to "kernel start" conditions.

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
static uint64_t u64IntCycles = 0;
static uint64_t u64IntLatencyMax = 0;
static uint64_t u64IdleTime = 0;
static FILE *fOutput = NULL;

//---------------------------------------------------------------------------
static void Mark3KA_InitOutputFile(void)
{
    char acFileName[256];
    int iWritten;

    time_t myTime;
    struct tm *myLocalTime;

    time(&myTime);
    myLocalTime = localtime(&myTime);

    iWritten = strftime(acFileName, 256, "%Y%m%d_%H%M%S", myLocalTime);
    sprintf( &acFileName[iWritten], "-Mark3KA.csv" );
    printf( "%s\n", acFileName );

    fOutput = fopen(acFileName, "w");

    if (fOutput)
    {
        fprintf( fOutput, "Cycle Count, Thread ID, Priority, Interrupt Time (%%), Interrupt Latency (cycles), Idle Time (%%), Stack Margin (Bytes)\n" );
    }
}

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

    uint16_t u16CurrAddr = ((uint16_t)(stCPU.pstRAM->au8RAM[ u16CurrPtr + 1 ]) << 8) +
                            stCPU.pstRAM->au8RAM[ u16CurrPtr ];

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
static uint16_t Mark3KA_GetCurrentStackMargin(void)
{
    uint16_t u16StackBase = Mark3KA_GetCurrentThread()->u16StackPtr;
    uint16_t u16StackSize = Mark3KA_GetCurrentThread()->u16StackSize;

    int i;

    for (i = 0; i < u16StackSize; i++)
    {
        if (255 != stCPU.pstRAM->au8RAM[ u16StackBase + i ])
        {
            return (uint16_t)i;
        }
    }

    return u16StackSize;
}

//---------------------------------------------------------------------------
static void KA_StackWarning( uint16_t u16Addr_, uint8_t u8Data_ )
{
    if (u8Data_ != 0xFF && stCPU.pstRAM->au8RAM[ u16Addr_ ] == 0xFF )
    {
        fprintf( stderr, "[WARNING] Near stack-overflow detected - Thread %d, Stack Margin %d\n",
                Mark3KA_GetCurrentThread()->u8ThreadID,
                Mark3KA_GetCurrentStackMargin() );
    }
}

//---------------------------------------------------------------------------
static void KA_ThreadChange( uint16_t u16Addr_, uint8_t u8Data_ )
{
    static uint8_t u8LastPri = 255;
    static uint64_t u64LastTime = 0;

    uint8_t u8Pri = Mark3KA_GetCurrentPriority();
    uint8_t u8Thread = Mark3KA_GetCurrentThread()->u8ThreadID;

    uint16_t u16Margin = Mark3KA_GetCurrentStackMargin();

    if (fOutput)
    {
        fprintf( fOutput, "%llu, %d, %d, %0.3f, %llu, %0.3f, %d\n",
                stCPU.u64CycleCount,
                u8Thread,
                u8Pri,
                (double)(u64IntCycles) /(double)stCPU.u64CycleCount,
                u64IntLatencyMax,
                (double)(u64IdleTime) / (double)stCPU.u64CycleCount,
                u16Margin
                );
    }
    if (u8LastPri == 0)
    {
        u64IdleTime += (stCPU.u64CycleCount - u64LastTime);
    }

    u64LastTime = stCPU.u64CycleCount;
    u8LastPri = u8Pri;

    // Add watchpoints on active thread stack at 32-bytes from the end
    // of the stack.  That way, we can immediately detect stack smashing threats
    // without having to hunt.

    uint16_t u16StackWarning = Mark3KA_GetCurrentThread()->u16StackPtr + 32;
    WriteCallout_Add( KA_StackWarning, u16StackWarning );
}

//---------------------------------------------------------------------------
static void KA_Interrupt( bool bEntry_ )
{
    static uint64_t u64TempIntCycles = 0;
    static bool bReset = true;
    if (bEntry_)
    {
        if (true == bReset)
        {
            u64TempIntCycles = stCPU.u64CycleCount;
            bReset = false;
        }
    }
    else
    {
        // Only reset latency score if there are no more pending interrupts...
        if (0 == stCPU.u32IntFlags)
        {
            if (stCPU.u64CycleCount > 25000) // hack to bypass startup code
            {
                uint8_t u8Pri = Mark3KA_GetCurrentPriority();
                uint8_t u8Thread = Mark3KA_GetCurrentThread()->u8ThreadID;

                u64IntCycles += stCPU.u64CycleCount - u64TempIntCycles;
                if ( ( stCPU.u64CycleCount - u64TempIntCycles ) > u64IntLatencyMax)
                {
                    u64IntLatencyMax = stCPU.u64CycleCount - u64TempIntCycles;
                };
            }
            bReset = true;
        }
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

    Mark3KA_InitOutputFile();
}
