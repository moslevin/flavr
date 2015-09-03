#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include "avr_cpu.h"

//---------------------------------------------------------------------------
typedef enum {
    // No ACK required
    GDB_COMMAND_f,
    GDB_COMMAND_i,  // Step target
    GDB_COMMAND_I,
    GDB_COMMAND_k,  // kill target.
    GDB_COMMAND_R,
    GDB_COMMAND_t,
    GDB_COMMAND_vFlashDone,
    // ACK required (OK or E<02X>)
    GDB_COMMAND_BANG,
    GDB_COMMAND_A,
    GDB_COMMAND_D,
    GDB_COMMAND_G,  // Write General Registers
    GDB_COMMAND_H,  // Set Thread ID for subsequent operations
    GDB_COMMAND_M,  // Write memory
    GDB_COMMAND_P,  // Write Register
    GDB_COMMAND_Qxxxx,
    GDB_COMMAND_T,
    GDB_COMMAND_X,
    GDB_COMMAND_x,
    GDB_COMMAND_Z,
    // Return data or error code
    GDB_COMMAND_QMARK,
    GDB_COMMAND_c,  // Continue execution
    GDB_COMMAND_C,  // Continue, with signal
    GDB_COMMAND_g,  // Read General Registers
    GDB_COMMAND_m,  // Read memory
    GDB_COMMAND_p,  // Read value of register
    GDB_COMMAND_qxxxx,
    GDB_COMMAND_s,
    GDB_COMMAND_S,
    GDB_COMMAND_v,
    GDB_COMMAND_z,  // Clear break/watchpoint
} GDBCommandType_t;

//---------------------------------------------------------------------------
typedef bool (*GDBCommandHandler_t)( const char *pcCmd_, char *ppcResponse_ );

//---------------------------------------------------------------------------
typedef struct
{
    GDBCommandType_t        eCmd;
    const char              *szToken;
    GDBCommandHandler_t     pfHandler;
} GDBCommandMap_t;

//---------------------------------------------------------------------------
static char *GDB_Packetize( const char *ppcResponse_ );
static uint8_t GDB_BuildChecksum( const char *ppcResponse_ );
static void GDB_SendStatus( const char *ppcResponse_, uint8_t signo_ );

//---------------------------------------------------------------------------
static bool GDB_Handler_ReadMem( const char *pcCmd_, char *ppcResponse_ );
static bool GDB_Handler_ReadReg( const char *pcCmd_, char *ppcResponse_ );
static bool GDB_Handler_ReadRegs( const char *pcCmd_, char *ppcResponse_ );
static bool GDB_Handler_WriteMem( const char *pcCmd_, char *ppcResponse_ );
static bool GDB_Handler_WriteReg( const char *pcCmd_, char *ppcResponse_ );
static bool GDB_Handler_WriteRegs( const char *pcCmd_, char *ppcResponse_ );
static bool GDB_Handler_Step( const char *pcCmd_, char *ppcResponse_ );
static bool GDB_Handler_Continue( const char *pcCmd_, char *ppcResponse_ );
static bool GDB_Handler_Query( const char *pcCmd_, char *ppcResponse_ );
static bool GDB_Handler_SetThread( const char *pcCmd_, char *ppcResponse_ );
static bool GDB_Handler_QuestionMark( const char *pcCmd_, char *ppcResponse_ );
static bool GDB_Handler_V( const char *pcCmd_, char *ppcResponse_ );
static bool GDB_Handler_Continue( const char *pcCmd_, char *ppcResponse_ );
static bool GDB_Handler_BreakPoint( const char *pcCmd_, char *ppcResponse_ );
static bool GDB_Handler_ClearBreakPoint( const char *pcCmd_, char *ppcResponse_ );
//---------------------------------------------------------------------------

static bool GDB_Handler_Unsupported( const char *pcCmd_, char *ppcResponse_ );

//---------------------------------------------------------------------------
#define WRITE_HEX_BYTE(x,y) { sprintf((char*)(x), "%02X", (uint8_t)(y)); (x) += 2; }
#define READ_HEX_BYTE(x,y)  { sscanf((char*)(x), "%02X", (uint8_t*)(y)); (x) += 2; }

//---------------------------------------------------------------------------
static const GDBCommandMap_t astCommands[] =
{
    { GDB_COMMAND_f,        "f",    GDB_Handler_Unsupported },
    { GDB_COMMAND_i,        "i",    GDB_Handler_Unsupported }, // Step target
    { GDB_COMMAND_I,        "I",    GDB_Handler_Unsupported },
    { GDB_COMMAND_k,        "k",    GDB_Handler_Unsupported }, // kill target.
    { GDB_COMMAND_R,        "R",    GDB_Handler_Unsupported },
    { GDB_COMMAND_t,        "t",    GDB_Handler_Unsupported },
    // ACK required (OK or E<02X>)
    { GDB_COMMAND_BANG,     "!",    GDB_Handler_Unsupported },
    { GDB_COMMAND_A,        "A",    GDB_Handler_Unsupported },
    { GDB_COMMAND_D,        "D",    GDB_Handler_Unsupported },
    { GDB_COMMAND_G,        "G",    GDB_Handler_WriteRegs }, // Write General Registers
    { GDB_COMMAND_H,        "H",    GDB_Handler_SetThread },
    { GDB_COMMAND_M,        "M",    GDB_Handler_WriteMem }, // Write memory
    { GDB_COMMAND_P,        "P",    GDB_Handler_WriteReg },
    { GDB_COMMAND_Qxxxx,    "Q",    GDB_Handler_Unsupported },
    { GDB_COMMAND_T,        "T",    GDB_Handler_Unsupported },
    { GDB_COMMAND_X,        "X",    GDB_Handler_Unsupported },
    { GDB_COMMAND_x,        "x",    GDB_Handler_Unsupported },
    { GDB_COMMAND_Z,        "Z",    GDB_Handler_BreakPoint },
    // Return data or error code
    { GDB_COMMAND_QMARK,    "?",    GDB_Handler_QuestionMark },
    { GDB_COMMAND_c,        "c",    GDB_Handler_Continue },  // Continue execution
    { GDB_COMMAND_C,        "C",    GDB_Handler_Unsupported }, // Continue, with signal
    { GDB_COMMAND_g,        "g",    GDB_Handler_ReadRegs }, // Read General Registers
    { GDB_COMMAND_m,        "m",    GDB_Handler_ReadMem }, // Read memory
    { GDB_COMMAND_p,        "p",    GDB_Handler_ReadReg}, // Read value of register
    { GDB_COMMAND_qxxxx,    "q",    GDB_Handler_Query },
    { GDB_COMMAND_s,        "s",    GDB_Handler_Step },
    { GDB_COMMAND_S,        "S",    GDB_Handler_Unsupported },
    { GDB_COMMAND_v,        "v",    GDB_Handler_V },
    { GDB_COMMAND_z,        "z",    GDB_Handler_ClearBreakPoint },

};


//---------------------------------------------------------------------------
static volatile bool bRetrigger = false;
static volatile bool bIsInteractive = false;
static int break_count = 0;

#include <io.h>
#include <fcntl.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <stdlib.h>

#pragma comment(lib, "Ws2_32.lib")
static WSADATA ws;
static SOCKET  my_socket    = INVALID_SOCKET;
static SOCKET  gdb_socket   = INVALID_SOCKET;
//---------------------------------------------------------------------------
static void GDB_ServerCreate(void)
{
    int err;

    struct addrinfo *localaddr = 0;
    struct addrinfo hints = { 0 };

    do
    {
        // Initialize winsock prior to use.
        err = WSAStartup(MAKEWORD(2,2), &ws);
        if (0 != err)
        {
            fprintf(stderr, "Error initializing winsock - bailing\n");
            break;
        }

        // Figure out what address to use for our server, specifying we want TCP/IP
        hints.ai_family = AF_INET;
        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;

        err = getaddrinfo(NULL, "1337", &hints, &localaddr);
        if (0 != err)
        {
            fprintf(stderr, "Error getting address info - bailing\n");
            break;
        }

        // Create a socket to listen for gdb incoming connections
        my_socket = socket(localaddr->ai_family, localaddr->ai_socktype, localaddr->ai_protocol);
        if (INVALID_SOCKET == my_socket)
        {
            fprintf(stderr, "Error creating socket - bailing\n" );
            err = -1;
            break;
        }

        // Setup the TCP listening socket
        if (SOCKET_ERROR == bind(my_socket, localaddr->ai_addr, (int)localaddr->ai_addrlen))
        {
            fprintf(stderr, "Error on socket bind - bailing\n");
            err = -1;
            break;
        }

        if (SOCKET_ERROR == listen(my_socket, SOMAXCONN))
        {
            fprintf(stderr, "Error on socket listen - bailing\n");
            err = -1;
            break;
        }

        gdb_socket = accept(my_socket, NULL, NULL);
        if (INVALID_SOCKET == gdb_socket)
        {
            fprintf(stderr, "Error on socket accept - bailing\n");
            err = -1;
            break;
        }
    } while(0);


    if (localaddr)
    {
        freeaddrinfo(localaddr);
    }

    if (0 != err)
    {
        if (INVALID_SOCKET != my_socket)
        {
            closesocket(my_socket);
        }
        if (INVALID_SOCKET != gdb_socket)
        {
            closesocket(gdb_socket);
        }
        WSACleanup();
        exit(-1);
    }

    fprintf(stderr, "[GDB Connected!]\n");
}

//---------------------------------------------------------------------------
static uint8_t wait_for_data( void )
{
    static bool bInit = false;

    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(gdb_socket, &read_fds);
    select(1, &read_fds, NULL, NULL, NULL);
    FD_CLR(gdb_socket, &read_fds);

    char ch;
    if ( 0 == recv(gdb_socket, &ch, 1, NULL )) {
        fprintf(stderr, "Socket disconnected - bailing\n");
        exit(0);
    }
    return ch;
}

//---------------------------------------------------------------------------
static bool GDB_Execute_i( void )
{
    char szCmdBuf[1024];
    char szRespBuf[1024];

    int idx = 0;
    char ch;

    fprintf( stderr, "Begin\n" );
    fflush( stderr );

    ch = wait_for_data(); //(stdin);
    while (ch != '$')
    {
        if (ch == 0)
        {
            fprintf(stderr, "[EOF]\n");
            exit(0);
        }
        if (ch == 3)
        {
            fprintf(stderr, "[BREAK]\n");
            return false;
        }
        ch = wait_for_data();
    }
    ch = wait_for_data();
    while (ch != '#')
    {
        szCmdBuf[idx++] = ch;
        ch = wait_for_data();
    }
    ch = wait_for_data();
    ch = wait_for_data();

    szCmdBuf[idx] = 0;

    fprintf(stderr, "[Sending ACK]\n");
    send(gdb_socket, "+", 1, 0);

    int i;
    for (i = 0; i < sizeof(astCommands)/sizeof(GDBCommandMap_t); i++)
    {
        if (astCommands[i].szToken[0] == szCmdBuf[0])
        {
            fprintf(stderr, "compare command %s with %s\n", astCommands[i].szToken, szCmdBuf );
            szRespBuf[0] = 0;
            bool ret = astCommands[i].pfHandler( szCmdBuf, szRespBuf );

            if (!ret) {
                char *resp = GDB_Packetize( szRespBuf );
                send(gdb_socket, resp, strlen(resp), 0);
                fprintf(stderr, "%s", resp);
                free(resp);
            }

            return ret;
        }
    }
    fprintf(stderr, "unsupported/unimplemented command %s\n", szCmdBuf );
    return false;
}

//---------------------------------------------------------------------------
void GDB_Set( void )
{
    bIsInteractive = true;
}

//---------------------------------------------------------------------------
#ifdef _WIN32
// Hack to get ctrl-c redirection working on Windows
#include <conio.h>
#include <windows.h>

static void *GDB_CatchIO(void *unused_)
{
    while(1)
    {
        if (false == bIsInteractive)
        {
            if (_kbhit())
            {
                if (3 == _getch())
                {
                    GDB_Set();
                }
            }
        }
        Sleep(100);
    }
}
#else
// Catch ctrl-c from GDB

#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

static void *GDB_CatchIO(void *unused_)
{
    while(1)
    {
        if (false == bIsInteractive)
        {
            fd_set read_fds;
            FD_ZERO(&read_fds);
            FD_SET(0, &read_fds);

            int err = select( 1, &read_fds, NULL, NULL, NULL );
            if (err > 0 && (false == bIsInteractive))
            {
                char ch;
                if (1 == read(0, &ch, 1))
                {
                    if (ch == 3) // Ctrl^C
                    {
                        GDB_Set();
                    }
                }
            }
        }
        usleep(100000);
    }
}

#endif
//---------------------------------------------------------------------------
static void GDB_InstallBreakHandler( void )
{
    pthread_attr_t attr;
    pthread_attr_init(&attr);

    pthread_t thread_id;
   // pthread_create(&thread_id, &attr, GDB_CatchIO, NULL);
}

//---------------------------------------------------------------------------
bool GDB_CheckAndExecute( void )
{
    // If we're in non-interactive mode (i.e. native execution), then return
    // out instantly.    
    if (false == bIsInteractive)
    {
        if (false == bRetrigger)
        {
            return;
        }
        bIsInteractive = true;
        bRetrigger = false;
    }
    fprintf(stderr, "[GDB] Debugging @ Address [0x%X]\n", stCPU.u16PC );

    if (break_count)
    {
        char szRespBuf[1024];
        GDB_SendStatus(szRespBuf, 5);
        char *resp = GDB_Packetize( szRespBuf );
        send(gdb_socket, resp, strlen(resp), 0);
        fprintf(stderr, "%s", resp);
        free(resp);
    }

    break_count++;

    // Keep attempting to parse commands until a valid one was encountered
    while (!GDB_Execute_i()) { /* Do Nothing */ }

    // !! Install monitor
}

//---------------------------------------------------------------------------
static bool GDB_Handler_ReadMem( const char *pcCmd_, char *ppcResponse_ )
{
    uint32_t u32Addr;
    uint32_t u32Count;
    char *src = (char*)&pcCmd_[1];
    char *dst = ppcResponse_;
    char *r;
    sscanf(src, "%X,%X", &u32Addr, &u32Count);

    if (u32Addr < 0x800000)
    {
        r = (char*)&stCPU.pu16ROM[u32Addr];
    }
    else if ((u32Addr >= 0x800000) && (u32Addr < 0x810000))
    {
        r = (char*)&stCPU.pstRAM->au8RAM[u32Addr & 0xFFFFF];
    }
    else if (u32Addr >= 0x810000 && u32Addr <= 0x820000)
    {
        r = (char*)&stCPU.pu8EEPROM[ u32Addr & 0xFFFFF ];
    }
    else
    {
        sprintf(ppcResponse_, "E01");
    }

    while (u32Count--)
    {
        WRITE_HEX_BYTE(dst, *r);
        r++;
    }
    return false;
}

//---------------------------------------------------------------------------
static bool GDB_Handler_ReadReg( const char *pcCmd_, char *ppcResponse_ )
{
    char *src = (char*)&pcCmd_[1];
    char *dst = ppcResponse_;

    uint8_t u8Reg;

    sscanf(src, "%2X", &u8Reg );

    if (u8Reg < 32)
    {
        WRITE_HEX_BYTE(dst, stCPU.pstRAM->stRegisters.CORE_REGISTERS.r[u8Reg]);
    }
    else if (u8Reg == 32)
    {
        WRITE_HEX_BYTE(dst, stCPU.pstRAM->stRegisters.SREG.r);
    }
    else if (u8Reg == 33)
    {
        WRITE_HEX_BYTE(dst, stCPU.pstRAM->stRegisters.SPL.r);
        WRITE_HEX_BYTE(dst, stCPU.pstRAM->stRegisters.SPH.r);
    }
    else if (u8Reg == 34)
    {
        WRITE_HEX_BYTE(dst, stCPU.u16PC & 0x00FF );
        WRITE_HEX_BYTE(dst, stCPU.u16PC >> 8 );
        WRITE_HEX_BYTE(dst, 0);
        WRITE_HEX_BYTE(dst, 0);
    }
    *dst = 0;
    return false;
}

//---------------------------------------------------------------------------
static bool GDB_Handler_ReadRegs( const char *pcCmd_, char *ppcResponse_ )
{
    int i;
    char *dst = ppcResponse_;

    for (i = 0; i < 32; i++)
    {
        WRITE_HEX_BYTE(dst, stCPU.pstRAM->stRegisters.CORE_REGISTERS.r[i]);
    }

    WRITE_HEX_BYTE(dst, stCPU.pstRAM->stRegisters.SREG.r);

    WRITE_HEX_BYTE(dst, stCPU.pstRAM->stRegisters.SPL.r);
    WRITE_HEX_BYTE(dst, stCPU.pstRAM->stRegisters.SPH.r);

    WRITE_HEX_BYTE(dst, stCPU.u16PC & 0x00FF );
    WRITE_HEX_BYTE(dst, stCPU.u16PC >> 8 );
    WRITE_HEX_BYTE(dst, 0);
    WRITE_HEX_BYTE(dst, 0);

    *dst = 0;
    return false;
}

//---------------------------------------------------------------------------
static bool GDB_Handler_WriteMem( const char *pcCmd_, char *ppcResponse_ )
{
    uint32_t u32Addr;
    uint32_t u32Count;
    char *src = (char*)&pcCmd_[1];
    char *data;
    char *r;
    sscanf(src, "%X,%X", &u32Addr, &u32Count);

    if (u32Addr < 0x800000)
    {
        r = (char*)&stCPU.pu16ROM[u32Addr];
    }
    else if ((u32Addr >= 0x800000) && (u32Addr < 0x810000))
    {
        r = (char*)&stCPU.pstRAM->au8RAM[u32Addr & 0xFFFF];
    }
    else if (u32Addr >= 0x810000 && u32Addr <= 0x820000)
    {
        r = (char*)&stCPU.pu8EEPROM[ u32Addr & 0xFFFF];
    }
    else
    {
        sprintf(ppcResponse_, "E01");
        GDB_Packetize(ppcResponse_);
    }

    data = strchr(pcCmd_, ':');

    while (u32Count--)
    {
        READ_HEX_BYTE(data, r);
        r++;
    }
    return false;
}
//---------------------------------------------------------------------------
static bool GDB_Handler_WriteReg( const char *pcCmd_, char *ppcResponse_ )
{
    char *src = (char*)&pcCmd_[1];
    char *data;
    uint8_t u8Reg;

    data = strchr(src, '=');
    *data = 0;
    data++;

    sscanf(src, "%2X", &u8Reg );

    if (u8Reg < 32)
    {
        READ_HEX_BYTE(data, &(stCPU.pstRAM->stRegisters.CORE_REGISTERS.r[u8Reg]));
    }
    else if (u8Reg == 32)
    {
        READ_HEX_BYTE(data, &(stCPU.pstRAM->stRegisters.SREG.r));
    }
    else if (u8Reg == 33)
    {
        READ_HEX_BYTE(data, &(stCPU.pstRAM->stRegisters.SPL.r));
        READ_HEX_BYTE(data, &(stCPU.pstRAM->stRegisters.SPH.r));
    }
    else if (u8Reg == 34)
    {
        uint8_t u8l, u8h;
        READ_HEX_BYTE(data, &u8l );
        READ_HEX_BYTE(data, &u8h );
        stCPU.u16PC = ( (((uint16_t)u8h) << 8) | (uint16_t)u8l );
    }

    sprintf(ppcResponse_, "OK");
    return false;
}

//---------------------------------------------------------------------------
static bool GDB_Handler_WriteRegs( const char *pcCmd_, char *ppcResponse_ )
{
    int i;
    char *src= (char*)&pcCmd_[1];

    for (i = 0; i < 32; i++)
    {
        READ_HEX_BYTE(src, &(stCPU.pstRAM->stRegisters.CORE_REGISTERS.r[i]));
    }

    READ_HEX_BYTE(src, &(stCPU.pstRAM->stRegisters.SREG.r));

    READ_HEX_BYTE(src, &(stCPU.pstRAM->stRegisters.SPL.r));
    READ_HEX_BYTE(src, &(stCPU.pstRAM->stRegisters.SPH.r));

    uint8_t u8l, u8h;

    READ_HEX_BYTE(src, &u8l );
    READ_HEX_BYTE(src, &u8h );
    stCPU.u16PC = ( (((uint16_t)u8h) << 8) | (uint16_t)u8l );

    sprintf(ppcResponse_, "OK");
    return false;
}


//---------------------------------------------------------------------------
static bool GDB_Handler_Query( const char *pcCmd_, char *ppcResponse_ )
{
    if (0 != strstr(pcCmd_, "Supported"))
    {
        sprintf(ppcResponse_, ""); //qXfer:memory-map:read+");
    }
    else if (0 != strstr(pcCmd_, "Attached"))
    {
        sprintf(ppcResponse_, "1");
    }
    else
    {
        sprintf(ppcResponse_, "");
    }
    return false;
}

//---------------------------------------------------------------------------
static bool GDB_Handler_QuestionMark( const char *pcCmd_, char *ppcResponse_ )
{
    sprintf(ppcResponse_, "T%02x20:%02x;21:%02x%02x;22:%02x%02x0000;",
        5, stCPU.pstRAM->stRegisters.SREG.r,
        stCPU.pstRAM->stRegisters.SPL.r,
        stCPU.pstRAM->stRegisters.SPH.r,
        stCPU.u16PC & 0xff, (stCPU.u16PC >> 8) & 0xff);

    return false;
}

//---------------------------------------------------------------------------
static bool GDB_Handler_V( const char *pcCmd_, char *ppcResponse_ )
{
    return false;
}

//---------------------------------------------------------------------------
static bool GDB_Handler_Continue( const char *pcCmd_, char *ppcResponse_ )
{
    bRetrigger = false;
    bIsInteractive = false;
    return true;
}

//---------------------------------------------------------------------------
static bool GDB_Handler_Step( const char *pcCmd_, char *ppcResponse_ )
{
    bRetrigger = true;
    bIsInteractive = false;
    return true;
}

//---------------------------------------------------------------------------
static bool GDB_Handler_SetThread( const char *pcCmd_, char *ppcResponse_ )
{
    sprintf(ppcResponse_, "OK");
    return false;
}

//---------------------------------------------------------------------------
static bool GDB_Handler_BreakPoint( const char *pcCmd_, char *ppcResponse_ )
{
    uint16_t addr;
    char *addr_str;
    char *addr_end;

    addr_str = strstr(pcCmd_,",");
    addr_str++;

    addr_end = strstr(addr_str,",");
    *addr_end = 0;
    sscanf(addr_str, "%4x", &addr);

    switch (pcCmd_[1] == '0')
    {
    case 0:
    case 1:
        if (!BreakPoint_EnabledAtAddress( addr ))
        {
            BreakPoint_Insert(addr);
            sprintf(ppcResponse_, "OK");
            return false;
        }      
        break;
    default:
        break;
    }
    sprintf(ppcResponse_, "E01");
    return false;
}

//---------------------------------------------------------------------------
static bool GDB_Handler_ClearBreakPoint( const char *pcCmd_, char *ppcResponse_ )
{
    uint16_t addr;
    char *addr_str;
    char *addr_end;

    addr_str = strstr(pcCmd_,",");
    addr_str++;

    addr_end = strstr(addr_str,",");
    *addr_end = 0;
    sscanf(addr_str, "%4x", &addr);

    switch (pcCmd_[1] == '0')
    {
    case 0:
    case 1:
        if (BreakPoint_EnabledAtAddress( addr ))
        {
            BreakPoint_Delete(addr);
            sprintf(ppcResponse_, "OK");
            return false;
        }
        break;
    default:
        break;
    }
    sprintf(ppcResponse_, "E01");
    return false;
}

//---------------------------------------------------------------------------
static void GDB_SendStatus( const char *ppcResponse_, uint8_t signo_ )
{
    sprintf(ppcResponse_, "T%02x20:%02x;21:%02x%02x;22:%02x%02x0000;",
    signo_, stCPU.pstRAM->stRegisters.SREG.r,
    stCPU.pstRAM->stRegisters.SPL.r,
    stCPU.pstRAM->stRegisters.SPH.r,
    stCPU.u16PC & 0xff, (stCPU.u16PC >> 8) & 0xff);
}

//---------------------------------------------------------------------------
static bool GDB_Handler_Unsupported( const char *pcCmd_, char *ppcResponse_ )
{
    fprintf( stderr, "[UNSUPPORTED COMMAND: %s]\n", pcCmd_ );

    return false;
}

//---------------------------------------------------------------------------
static uint8_t GDB_BuildChecksum( const char *ppcResponse_ )
{
    uint8_t u8Chk = 0;
    while (*ppcResponse_)
    {
        u8Chk += *ppcResponse_++;
    }
    return u8Chk;
}

//---------------------------------------------------------------------------
static char *GDB_Packetize( const char *ppcResponse_ )
{
    char *dst = (char*)(malloc(1024));
    uint8_t u8Chk = GDB_BuildChecksum( ppcResponse_ );
    sprintf( dst, "$%s#%02x", ppcResponse_, u8Chk );
    return dst;
}

//---------------------------------------------------------------------------
static bool GDB_WatchpointCallback( uint16_t u16Addr_, uint8_t u8Val_ )
{
    if (WatchPoint_EnabledAtAddress(u16Addr_))
    {        
        Interactive_Set();
    }
    return true;
}

//---------------------------------------------------------------------------
void GDB_Init( void )
{   
    WriteCallout_Add( GDB_WatchpointCallback, 0 );
    GDB_InstallBreakHandler();
    GDB_ServerCreate();
    send(gdb_socket, "+", 1, 0);
}


