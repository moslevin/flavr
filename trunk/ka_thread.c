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
    \file   ka_thread.c

    \brief  Mark3 RTOS Kernel-Aware Thread Profiling
*/

#include "kernel_aware.h"
#include "debug_sym.h"
#include "write_callout.h"
#include "interrupt_callout.h"
#include "tlv_file.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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
typedef struct
{
    Mark3_Thread_t *pstThread;
    uint8_t        u8ThreadID;
    uint64_t       u64TotalCycles;
    uint64_t       u64EpockCycles;
} Mark3_Thread_Info_t;

//---------------------------------------------------------------------------
typedef struct
{
    uint64_t        u64Timestamp;
    uint16_t        u16StackMargin;
    uint8_t         u8ThreadID;
    uint8_t         u8ThreadPri;
} Mark3ContextSwitch_TLV_t;

//---------------------------------------------------------------------------
static uint64_t u64IdleTime = 0;
static FILE *fKernelState = NULL;
static FILE *fInterrupts = NULL;
static Mark3_Thread_Info_t *pstThreadInfo = NULL;
static uint16_t u16NumThreads = 0;

static Mark3_Thread_t *pstLastThread = NULL;
static uint64_t u64LastTime = 0;
static uint8_t u8LastPri = 255;
//---------------------------------------------------------------------------
static TLV_t *pstTLV = NULL;

//---------------------------------------------------------------------------
static void Mark3KA_AddKnownThread( Mark3_Thread_t *pstThread_ )
{
    // Bail if the thread pointer is NULL
    if (!pstThread_ || ((uint32_t)pstThread_ == (uint32_t)stCPU.pstRAM->au8RAM))
    {
        return;
    }

    // Check to see if a thread has already been tagged at this address
    bool bExists = false;
    if (pstThreadInfo)
    {
        int i;
        for (i = 0; i < u16NumThreads; i++)
        {
            Mark3_Thread_t *pstThread = pstThreadInfo[i].pstThread;
            if ( (pstThread == pstThread_) &&
                 (pstThreadInfo[i].u8ThreadID == pstThread_->u8ThreadID) )
            {
                bExists = true;
                // If this thread is a dynamic thread being "recycled", reset the CPU usage stats associated with it.
                if (pstThread->u8ThreadID != pstThreadInfo[i].u8ThreadID)
                {
                    pstThreadInfo[i].u64EpockCycles = 0;
                    pstThreadInfo[i].u64TotalCycles = 0;
                    pstThreadInfo[i].u8ThreadID = pstThread->u8ThreadID;
                }
                break;
            }
        }
    }

    // If not, add it to the list of known threads.
    if (!bExists)
    {
        u16NumThreads++;
        pstThreadInfo = (Mark3_Thread_Info_t*)realloc(pstThreadInfo, sizeof(Mark3_Thread_Info_t) * u16NumThreads);

        pstThreadInfo[u16NumThreads - 1].pstThread = pstThread_;
        pstThreadInfo[u16NumThreads - 1].u64EpockCycles = 0;
        pstThreadInfo[u16NumThreads - 1].u64TotalCycles = 0;
        pstThreadInfo[u16NumThreads - 1].u8ThreadID = pstThread_->u8ThreadID;
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
static uint16_t Mark3KA_GetStackMargin( Mark3_Thread_t *pstThread_ )
{
    uint16_t u16StackBase = pstThread_->u16StackPtr;
    uint16_t u16StackSize = pstThread_->u16StackSize;

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
static uint16_t Mark3KA_GetCurrentStackMargin(void)
{
   return Mark3KA_GetStackMargin( Mark3KA_GetCurrentThread() );
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
    uint8_t  u8Pri = Mark3KA_GetCurrentPriority();
    uint8_t  u8Thread = Mark3KA_GetCurrentThread()->u8ThreadID;
    uint16_t u16Margin = Mark3KA_GetCurrentStackMargin();

    // -- Add context switch instrumentation to TLV
    Mark3ContextSwitch_TLV_t stData;

    stData.u8ThreadID = u8Thread;
    stData.u8ThreadPri = u8Pri;
    stData.u16StackMargin = u16Margin;
    stData.u64Timestamp = stCPU.u64CycleCount;

    memcpy( &(pstTLV->au8Data[0]), &stData, sizeof(stData) );
    TLV_Write( pstTLV );

    if (u8LastPri == 0)
    {
        u64IdleTime += (stCPU.u64CycleCount - u64LastTime);
    }


    // Track this as a known-thread internally for future reporting.
    Mark3KA_AddKnownThread( Mark3KA_GetCurrentThread() );

    if (pstLastThread && u64LastTime)
    {
        Mark3_Thread_t *pstThread;
        int i;
        for ( i = 0; i < u16NumThreads; i++ )
        {
            if  ( (pstLastThread == pstThreadInfo[i].pstThread) &&
                  (pstLastThread->u8ThreadID == pstThreadInfo[i].u8ThreadID) )
            {
                pstThreadInfo[i].u64TotalCycles += stCPU.u64CycleCount - u64LastTime;
            }
        }
    }

    u64LastTime = stCPU.u64CycleCount;
    u8LastPri = u8Pri;

    // Add watchpoints on active thread stack at 32-bytes from the end
    // of the stack.  That way, we can immediately detect stack smashing threats
    // without having to hunt.

    uint16_t u16StackWarning = Mark3KA_GetCurrentThread()->u16StackPtr + 32;
    WriteCallout_Add( KA_StackWarning, u16StackWarning );

    // Cache the current thread for use as the "last run" thread in
    // subsequent iterations
    pstLastThread = Mark3KA_GetCurrentThread();
}

//---------------------------------------------------------------------------
void KA_Thread_Init( void )
{
    Debug_Symbol_t *pstSymbol = 0;
    pstSymbol = Symbol_Find_Obj_By_Name( "g_pstCurrent" );

    // Use pstSymbol's address to get a pointer to the current thread.
    if (!pstSymbol)
    {
        return;
    }

    // Ensure that we actually have the information we need at a valid address
    uint16_t u16CurrPtr = (uint16_t)(pstSymbol->u32StartAddr & 0x0000FFFF);
    if (!u16CurrPtr)
    {
        return;
    }

    // Add a callback so that when g_pstCurrent changes, we can update our
    // locally-tracked statistics.
    WriteCallout_Add( KA_ThreadChange , u16CurrPtr + 1 );

    pstTLV = TLV_Alloc( sizeof(Mark3ContextSwitch_TLV_t) );
    pstTLV->eTag = TAG_KERNEL_AWARE_CONTEXT_SWITCH;
    pstTLV->u16Len = sizeof(Mark3ContextSwitch_TLV_t);

}
