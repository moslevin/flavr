#include "kernel_aware.h"
#include "debug_sym.h"

#include "ka_file.h"

#include <fcntl.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

//---------------------------------------------------------------------------
typedef struct {
    uint16_t u16PathAddress;
    bool bRead;
    bool bWrite;
    bool bCreate;
    bool bAppend;
    bool bTruncate;
} OpenRequest_t;

typedef struct {
    int32_t iHostFd;
} OpenReturn_t;

void KA_Command_Open(void)
{
    Debug_Symbol_t *pstSymbol = Symbol_Find_Obj_By_Name( "g_stKAData" );
    if (!pstSymbol)
    {
        return;
    }

    OpenRequest_t *pstOpen = (OpenRequest_t*)&stCPU.pstRAM->au8RAM[ pstSymbol->u32StartAddr ];
    const char* path = (const char*)&stCPU.pstRAM->au8RAM[ pstOpen->u16PathAddress ];

    int flags = 0;
    if (pstOpen->bRead && !pstOpen->bWrite) {
        flags = O_RDONLY;
    }
    else if (!pstOpen->bRead && pstOpen->bWrite)
    {
        flags = O_WRONLY;
    }
    else if (pstOpen->bRead && pstOpen->bWrite)
    {
        flags = O_RDWR;
    }
    else
    {
        OpenReturn_t *pstReturn = (OpenReturn_t*)&stCPU.pstRAM->au8RAM[ pstSymbol->u32StartAddr ];
        pstReturn->iHostFd = -1;
        return;
    }

    if (pstOpen->bAppend)
    {
        flags |= O_APPEND;
    }
    if (pstOpen->bCreate)
    {
        flags |= O_CREAT;
    }
    if (pstOpen->bTruncate)
    {
        flags |= O_TRUNC;
    }

    int fd = open(path, flags, 0660);

    OpenReturn_t *pstReturn = (OpenReturn_t*)&stCPU.pstRAM->au8RAM[ pstSymbol->u32StartAddr ];
    pstReturn->iHostFd = fd;
}

//---------------------------------------------------------------------------
typedef struct {
    int32_t iHostFd;
} CloseRequest_t;

void KA_Command_Close(void)
{
    Debug_Symbol_t *pstSymbol = Symbol_Find_Obj_By_Name( "g_stKAData" );
    if (!pstSymbol)
    {
        return;
    }
    CloseRequest_t *pstClose = (CloseRequest_t*)&stCPU.pstRAM->au8RAM[ pstSymbol->u32StartAddr ];
    close(pstClose->iHostFd);
}

//---------------------------------------------------------------------------
typedef struct {
    int32_t iHostFd;
    uint16_t u16ReadBufAddress;
    uint16_t u16BytesToRead;
} KernelAwareRead_t;

typedef struct {
    int32_t iBytesRead;
} KernelAwareReadReturn_t;

void KA_Command_Read(void)
{
    Debug_Symbol_t *pstSymbol = Symbol_Find_Obj_By_Name( "g_stKAData" );
    if (!pstSymbol)
    {
        return;
    }

    KernelAwareRead_t *pstRead = (KernelAwareRead_t*)&stCPU.pstRAM->au8RAM[ pstSymbol->u32StartAddr ];
    void* pvReadPtr = (void*)&stCPU.pstRAM->au8RAM[ pstRead->u16ReadBufAddress ];
    size_t iBytesRead = read(pstRead->iHostFd, pvReadPtr, pstRead->u16BytesToRead);

    KernelAwareReadReturn_t *pstReadResult = (KernelAwareReadReturn_t*)&stCPU.pstRAM->au8RAM[ pstSymbol->u32StartAddr ];
    pstReadResult->iBytesRead = iBytesRead;
}

//---------------------------------------------------------------------------
typedef struct {
    int32_t iHostFd;
    uint16_t u16WriteBufAddress;
    uint16_t u16BytesToWrite;
} KernelAwareWrite_t;

typedef struct {
    volatile int32_t iBytesWritten;
} KernelAwareWriteReturn_t;

void KA_Command_Write(void)
{
    Debug_Symbol_t *pstSymbol = Symbol_Find_Obj_By_Name( "g_stKAData" );
    if (!pstSymbol)
    {
        return;
    }

    KernelAwareWrite_t *pstWrite = (KernelAwareWrite_t*)&stCPU.pstRAM->au8RAM[ pstSymbol->u32StartAddr ];
    void* pvWritePtr = (void*)&stCPU.pstRAM->au8RAM[ pstWrite->u16WriteBufAddress ];
    size_t iBytesWritten = write(pstWrite->iHostFd, pvWritePtr, pstWrite->u16BytesToWrite );

    KernelAwareWriteReturn_t *pstWriteReturn = (KernelAwareWriteReturn_t*)&stCPU.pstRAM->au8RAM[ pstSymbol->u32StartAddr ];
    pstWriteReturn->iBytesWritten = iBytesWritten;
}

//---------------------------------------------------------------------------
typedef struct {
    int32_t iHostFd;
    bool bBlocking;
} KernelAwareBlocking_t;

void KA_Command_Blocking(void)
{
    Debug_Symbol_t *pstSymbol = Symbol_Find_Obj_By_Name( "g_stKAData" );
    if (!pstSymbol)
    {
        return;
    }

    KernelAwareBlocking_t *pstBlocking = (KernelAwareBlocking_t*)&stCPU.pstRAM->au8RAM[ pstSymbol->u32StartAddr ];
    int flags = fcntl(pstBlocking->iHostFd, F_GETFL);
    if (!pstBlocking->bBlocking)
    {
        flags |= O_NONBLOCK;
    }
    else
    {
        flags &= ~O_NONBLOCK;
    }
    fcntl(pstBlocking->iHostFd, F_SETFL, flags);
}
