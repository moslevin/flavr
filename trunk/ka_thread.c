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
#include "ka_thread.h"

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

    //! Thread ID
    uint8_t u8ThreadID;

    //! Default priority of the thread
    uint8_t u8Priority;

    //! Current priority of the thread (priority inheritence)
    uint8_t u8CurPriority;

    //! Size of the stack (in bytes)
    uint16_t u16StackSize;

    //! Threadlists
    uint16_t u16CurrentThreadList;
    uint16_t u16OwnerThreadList;

    //! The entry-point function called when the thread starts
    uint16_t u16EntryPoint;

    //! Pointer to the argument passed into the thread's entrypoint
    void *m_pvArg;

    //! Thread quantum (in milliseconds)
    uint16_t u16Quantum;

} Mark3_Thread_t;

//---------------------------------------------------------------------------
typedef struct
{
    Mark3_Thread_t *pstThread;
    uint8_t        u8ThreadID;
    uint64_t       u64TotalCycles;
    uint64_t       u64EpockCycles;
    bool           bActive;
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
            // If there are other threads that exist at this address,
            if (pstThread == pstThread_)
            {
                // If the stored thread's ID is different than the ID being presented here,
                // then it's a dynamic thread involved.  Create a new threadinfo object to track it.
                if (pstThreadInfo[i].u8ThreadID != pstThread_->u8ThreadID)
                {
                    pstThreadInfo[i].bActive = false;
                }
                // Thread IDs are the same, thread has already been tracked, don't do anything.
                else
                {
                    bExists = true;
                }
            }
        }
    }

    // If not already known, add the thread to the list of known threads.
    if (!bExists)
    {
        u16NumThreads++;
        pstThreadInfo = (Mark3_Thread_Info_t*)realloc(pstThreadInfo, sizeof(Mark3_Thread_Info_t) * u16NumThreads);

        pstThreadInfo[u16NumThreads - 1].pstThread = pstThread_;
        pstThreadInfo[u16NumThreads - 1].u64EpockCycles = 0;
        pstThreadInfo[u16NumThreads - 1].u64TotalCycles = 0;
        pstThreadInfo[u16NumThreads - 1].u8ThreadID = pstThread_->u8ThreadID;
        pstThreadInfo[u16NumThreads - 1].bActive = true;
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
static bool KA_StackWarning( uint16_t u16Addr_, uint8_t u8Data_ )
{
    if (u8Data_ != 0xFF && stCPU.pstRAM->au8RAM[ u16Addr_ ] == 0xFF )
    {
        fprintf( stderr, "[WARNING] Near stack-overflow detected - Thread %d, Stack Margin %d\n",
                Mark3KA_GetCurrentThread()->u8ThreadID,
                Mark3KA_GetCurrentStackMargin() );
    }
    return true;
}

//---------------------------------------------------------------------------
static bool KA_ThreadChange( uint16_t u16Addr_, uint8_t u8Data_ )
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

    return true;
}

//---------------------------------------------------------------------------
void KA_PrintThreadInfo(void)
{
    int i;
    uint64_t u64TrackedThreadTime = 0;

    uint16_t u16LastThread = (uint16_t)((void*)Mark3KA_GetCurrentThread() - (void*)&stCPU.pstRAM->au8RAM[0]);

    KA_ThreadChange( u16LastThread, 0 );

    for ( i = 0; i < u16NumThreads; i++ )
    {
        u64TrackedThreadTime += pstThreadInfo[i].u64TotalCycles;
    }

    printf( "ThreadID, ThreadAddr, TotalCycles, PercentCPU, IsActive, Prio, StackMargin\n");
    for ( i = 0; i < u16NumThreads; i++ )
    {
        printf( "%d, %04X, %llu, %0.3f, %d, %d, %d\n",
                    pstThreadInfo[i].u8ThreadID,
                    (uint16_t)((void*)(pstThreadInfo[i].pstThread) - (void*)(&stCPU.pstRAM->au8RAM[0])),
                    pstThreadInfo[i].u64TotalCycles,
                    (double)pstThreadInfo[i].u64TotalCycles / u64TrackedThreadTime * 100.0f,
                    pstThreadInfo[i].bActive,
                    (pstThreadInfo[i].bActive ? pstThreadInfo[i].pstThread->u8Priority : 0),
                    (pstThreadInfo[i].bActive ? Mark3KA_GetStackMargin(pstThreadInfo[i].pstThread) : 0)
                ) ;
    }
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

    atexit( KA_PrintThreadInfo );
}

//---------------------------------------------------------------------------
char *KA_Get_Thread_Info_XML(uint8_t **thread_ids, uint16_t *thread_count)
{
    char *ret = (char*)malloc(4096);
    char *writer = ret;
    uint8_t *new_ids;
    fprintf(stderr, "1");
    if (u16NumThreads && thread_ids)
    {
        new_ids = (uint8_t*)malloc(u16NumThreads);
        *thread_ids = new_ids;
    }

    fprintf(stderr, "2");
    writer += sprintf( writer,
            "<threads>" );

    if (!u16NumThreads) {
        writer += sprintf( writer,
        "  <thread id=\"0\" core=\"0\">"
        "  System Thread - Priority N/A [Running] "
        "  </thread>");
    }

    int i;
    int count = 0;
    for (i = 0; i < u16NumThreads; i++)
    {
        if (pstThreadInfo[i].bActive)
        {
            if (pstThreadInfo[i].u8ThreadID == 255)
            {
                fprintf(stderr, "a");
                writer += sprintf(writer,
                "  <thread id=\"255\" core=\"0\">"
                "  Mark3 Thread - Priority 0 [IDLE]");
            }
            else if (pstThreadInfo[i].u8ThreadID == Mark3KA_GetCurrentThread()->u8ThreadID)
            {
                fprintf(stderr, "b");
                writer += sprintf(writer,
                "  <thread id=\"%d\" core=\"0\">"
                "  Mark3 Thread - Priority %d [Running] " ,
                pstThreadInfo[i].u8ThreadID,
                pstThreadInfo[i].pstThread->u8CurPriority );
            }
            else
            {
                fprintf(stderr, "c");
                writer += sprintf(writer,
                "  <thread id=\"%d\" core=\"0\">"
                "  Mark3 Thread - Priority %d" ,
                pstThreadInfo[i].u8ThreadID,
                pstThreadInfo[i].pstThread->u8CurPriority );
            }
            if (thread_ids)
            {
                new_ids[count++] = pstThreadInfo[i].u8ThreadID;
            }
        }
        writer += sprintf( writer, "  </thread>");
    }

    sprintf( writer, "</threads>" );
    if (thread_count)
    {
        *thread_count = count;
    }
    fprintf(stderr, "%s", ret);
    return ret;
}

//---------------------------------------------------------------------------
Mark3_Context_t *KA_Get_Thread_Context(uint8_t id_)
{
    int i;
    for (i = 0; i < u16NumThreads; i++)
    {
        if (pstThreadInfo[i].bActive)
        {
            if (pstThreadInfo[i].u8ThreadID == id_)
            {
                Mark3_Context_t *new_ctx = (Mark3_Context_t*)malloc(sizeof(Mark3_Context_t));
                uint16_t context_addr = pstThreadInfo[i].pstThread->u16StackTopPtr;

                new_ctx->SPH = stCPU.pstRAM->au8RAM[context_addr - 1];
                new_ctx->SPL = stCPU.pstRAM->au8RAM[context_addr];

                int j = 0;
                for (i = 31; i >= 0; i--)
                {
                    new_ctx->r[i] = stCPU.pstRAM->au8RAM[context_addr + 1 + j];
                    j++;
                }
                new_ctx->SREG = stCPU.pstRAM->au8RAM[context_addr + 33];
                uint16_t PC = *(uint16_t*)(&stCPU.pstRAM->au8RAM[context_addr + 34]);
                PC = ((PC & 0xFF00)>>8) | ((PC & 0x00FF) << 8);
                new_ctx->PC = PC;

                return new_ctx;
            }
        }
    }
    return NULL;
}
