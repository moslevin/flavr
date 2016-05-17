#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <fcntl.h>
#include "avr_cpu.h"
#include "options.h"
#include "kernel_aware.h"
#include "ka_thread.h"
#include "debug_sym.h"

//---------------------------------------------------------------------------
//#define DEBUG
#if !defined( DEBUG )
# define DEBUG_PRINT(...)
#else
# define DEBUG_PRINT            fprintf
#endif

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
    GDB_COMMAND_T,  // Thread still alive
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
extern Mark3_Context_t *KA_Get_Thread_Context(uint8_t id_);
int KA_Get_Thread_ID(void);

//---------------------------------------------------------------------------
static void GDB_SendAck( void );
static char *GDB_Packetize( const char *ppcResponse_ );
static uint8_t GDB_BuildChecksum( const char *ppcResponse_ );
static void GDB_SendStatus( char *ppcResponse_, uint8_t signo_ );

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
static bool GDB_Handler_SetBreakPoint( const char *pcCmd_, char *ppcResponse_ );
static bool GDB_Handler_ClearBreakPoint( const char *pcCmd_, char *ppcResponse_ );
static bool GDB_Handler_Kill( const char *pcCmd_, char *ppcResponse_ );
static bool GDB_Handler_PokeThread( const char *pcCmd_, char *ppcResponse_ );
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
    { GDB_COMMAND_k,        "k",    GDB_Handler_Kill }, // kill target.
    { GDB_COMMAND_R,        "R",    GDB_Handler_Unsupported },
    { GDB_COMMAND_t,        "t",    GDB_Handler_Unsupported },
    // ACK required (OK or E<02X>)
    { GDB_COMMAND_BANG,     "!",    GDB_Handler_Unsupported },
    { GDB_COMMAND_A,        "A",    GDB_Handler_Unsupported },
    { GDB_COMMAND_D,        "D",    GDB_Handler_Kill }, // Treat disconnect and kill the same...
    { GDB_COMMAND_G,        "G",    GDB_Handler_WriteRegs }, // Write General Registers
    { GDB_COMMAND_H,        "H",    GDB_Handler_SetThread },
    { GDB_COMMAND_M,        "M",    GDB_Handler_WriteMem }, // Write memory
    { GDB_COMMAND_P,        "P",    GDB_Handler_WriteReg },
    { GDB_COMMAND_Qxxxx,    "Q",    GDB_Handler_Unsupported },
    { GDB_COMMAND_T,        "T",    GDB_Handler_PokeThread },
    { GDB_COMMAND_X,        "X",    GDB_Handler_Unsupported },
    { GDB_COMMAND_x,        "x",    GDB_Handler_Unsupported },
    { GDB_COMMAND_Z,        "Z",    GDB_Handler_SetBreakPoint },
    // Return data or error code
    { GDB_COMMAND_QMARK,    "?",    GDB_Handler_QuestionMark },
    { GDB_COMMAND_c,        "c",    GDB_Handler_Continue },  // Continue execution
    { GDB_COMMAND_C,        "C",    GDB_Handler_Unsupported }, // Continue, with signal
    { GDB_COMMAND_g,        "g",    GDB_Handler_ReadRegs }, // Read General Registers
    { GDB_COMMAND_m,        "m",    GDB_Handler_ReadMem }, // Read memory
    { GDB_COMMAND_p,        "p",    GDB_Handler_ReadReg}, // Read value of register
    { GDB_COMMAND_qxxxx,    "q",    GDB_Handler_Query },
    { GDB_COMMAND_s,        "s",    GDB_Handler_Step }, // Step
    { GDB_COMMAND_S,        "S",    GDB_Handler_Unsupported },
    { GDB_COMMAND_v,        "v",    GDB_Handler_V },
    { GDB_COMMAND_z,        "z",    GDB_Handler_ClearBreakPoint },
};

//---------------------------------------------------------------------------
static volatile bool bRetrigger = false;
static volatile bool bIsInteractive = false;
static volatile bool bStepping = false;
static volatile int break_count = 0;
static int mark3_thread = -1;

#if _WIN32
#include <io.h>
#include <WinSock2.h>
#include <WS2tcpip.h>

static SOCKET  my_socket    = INVALID_SOCKET;
static SOCKET  gdb_socket   = INVALID_SOCKET;

#pragma comment(lib, "Ws2_32.lib")
static WSADATA ws;

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
            DEBUG_PRINT(stderr, "Error initializing winsock - bailing\n");
            break;
        }

        // Figure out what address to use for our server, specifying we want TCP/IP
        hints.ai_family = AF_INET;
        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;

        const char *portnum = Options_GetByName("--gdb");
        if (!portnum)
        {
            portnum = "3333";
        }

        err = getaddrinfo(NULL, portnum, &hints, &localaddr);
        if (0 != err)
        {
            DEBUG_PRINT(stderr, "Error getting address info - bailing\n");
            break;
        }

        // Create a socket to listen for gdb incoming connections
        my_socket = socket(localaddr->ai_family, localaddr->ai_socktype, localaddr->ai_protocol);
        if (INVALID_SOCKET == my_socket)
        {
            DEBUG_PRINT(stderr, "Error creating socket - bailing\n" );
            err = -1;
            break;
        }

        // Setup the TCP listening socket
        if (SOCKET_ERROR == bind(my_socket, localaddr->ai_addr, (int)localaddr->ai_addrlen))
        {
            DEBUG_PRINT(stderr, "Error on socket bind - bailing\n");
            err = -1;
            break;
        }

        if (SOCKET_ERROR == listen(my_socket, SOMAXCONN))
        {
            DEBUG_PRINT(stderr, "Error on socket listen - bailing\n");
            err = -1;
            break;
        }

        gdb_socket = accept(my_socket, NULL, NULL);
        if (INVALID_SOCKET == gdb_socket)
        {
            DEBUG_PRINT(stderr, "Error on socket accept - bailing\n");
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

    DEBUG_PRINT(stderr, "[GDB Connected!]\n");
}
#else // POSIX Implementation
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

static int  my_socket    = 0;
static int  gdb_socket   = 0;

//---------------------------------------------------------------------------
static void GDB_ServerCreate(void)
{
    fprintf(stderr, "[Initializing GDB socket]");

    const char *portnum_s = Options_GetByName("--gdb");
    if (!portnum_s)
    {
        portnum_s = "3333";
    }
    int portnum = atoi(portnum_s);

    my_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (my_socket <= 0)
    {
        fprintf( stderr, "Error creating socket on port %s, bailing\n", portnum_s );
        exit(-1);
    }

    struct sockaddr_in serv_addr = { 0 };
    struct sockaddr_in cli_addr = { 0 };

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portnum);

    if (bind(my_socket, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
    {
        fprintf(stderr, "Error binding socket -- bailing\n");
        exit(-1);
    }

    listen(my_socket,1);

    int clilen = sizeof(cli_addr);
    gdb_socket = accept(my_socket, (struct sockaddr *)&cli_addr, &clilen);
    if (gdb_socket < 0)
    {
         fprintf(stderr, "Error on accept -- bailing\n");
         exit(-1);
    }
    fprintf( stderr, "GDB Socket: %d", gdb_socket );
    fprintf( stderr, "[GDB Connected]" );
}

#endif

//---------------------------------------------------------------------------
static uint8_t wait_for_data( void )
{
    char ch;
    int count;

    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(gdb_socket, &read_fds);
    select(gdb_socket+1, &read_fds, NULL, NULL, NULL);
    FD_CLR(gdb_socket, &read_fds);

    count = recv(gdb_socket, &ch, 1, 0 );
    if ( 0 == count )
    {
        DEBUG_PRINT(stderr, "Socket disconnected - bailing\n");
        bIsInteractive = true;
        usleep(500000);
        exit(-1);
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

    // Wait until there's data on the socket to read
    DEBUG_PRINT(stderr, "[Begin Packet]\n");

    ch = wait_for_data();
    // Search for the telltale "$" at the beginning of a packet
    while (ch != '$')
    {
        // 3 indicates CTRL^C -- break;
        if (ch == 3)
        {
            fprintf(stderr, "[GDB - Received Break]\n");
            bIsInteractive = true;
            break_count++;

            return false;
        }
        ch = wait_for_data();
    }

    // Found the header ,Read the remainder of the packet
    ch = wait_for_data();
    while (ch != '#')
    {
        szCmdBuf[idx++] = ch;
        ch = wait_for_data();
    }

    // End of packet found, read the checksum.  ToDo -- validate it
    ch = wait_for_data();
    ch = wait_for_data();

    // Null-terminate the packet
    szCmdBuf[idx] = 0;
    GDB_SendAck();

    //!! Todo - defensive programming.
    DEBUG_PRINT(stderr, "[RX]%s\n", szCmdBuf);
    // Go through our list of commands, and dispatch a handler based on command string
    int i;
    for (i = 0; i < sizeof(astCommands)/sizeof(GDBCommandMap_t); i++)
    {
        if (astCommands[i].szToken[0] == szCmdBuf[0])
        {            
            szRespBuf[0] = 0;
            bool ret = astCommands[i].pfHandler( szCmdBuf, szRespBuf );

            // if ret == false, it means that we're going to let the simulator run
            // until there's data from GDB (typically on a break or EOF).
            if (!ret)
            {
                // Otherwise, we have data to send back to GDB immediately.
                char *resp = GDB_Packetize( szRespBuf );
                send(gdb_socket, resp, strlen(resp), 0);
                DEBUG_PRINT(stderr, "%s", resp);
                free(resp);
            }

            return ret;
        }
    }

    return false;
}

//---------------------------------------------------------------------------
void GDB_Set( void )
{
    DEBUG_PRINT( stderr, "Listen for GDB\n");
    bIsInteractive = true;
}

//---------------------------------------------------------------------------
static void *GDB_CatchIO(void *unused_)
{
    while(1)
    {
        if (false == bIsInteractive)
        {
            fd_set read_fds;
            FD_ZERO(&read_fds);
            FD_SET(gdb_socket, &read_fds);

            int err = select( gdb_socket+1, &read_fds, NULL, NULL, NULL );
            if (err > 0)
            {
                char ch;
                if (1 == recv(gdb_socket, &ch, 1, 0))
                {
                    if (ch == 3) // Ctrl^C
                    {
                        fprintf(stderr, "[GDB - Signal Break]\n");
                        bIsInteractive = true;
                    }
                    else if (ch == 0)
                    {
                        fprintf(stderr, "[GDB - Signal EOF]\n");
                        exit(0);
                    }
                }
            } else {
                return NULL;
            }
        }        
    }
}

//---------------------------------------------------------------------------
static void GDB_InstallBreakHandler( void )
{
    pthread_attr_t attr;
    pthread_attr_init(&attr);

    struct sched_param param;
    param.sched_priority = sched_get_priority_max(SCHED_RR) - 1;
    pthread_attr_setschedparam(&attr, &param );

    pthread_t thread_id;
    pthread_create(&thread_id, &attr, GDB_CatchIO, NULL);
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
    DEBUG_PRINT(stderr, "[GDB] Debugging @ Address [0x%X]\n", stCPU.u16PC << 1 );

    DEBUG_PRINT(stderr, "BreakCount: %d, Stepping: %d\n", break_count, bStepping);
    if (break_count || bStepping)
    {
        char szRespBuf[1024];
        GDB_SendStatus(szRespBuf, 0);
        char *resp = GDB_Packetize( szRespBuf );
        send(gdb_socket, resp, strlen(resp), 0);
        DEBUG_PRINT(stderr, "%s", resp);
        free(resp);
    }

    bStepping = false;
    break_count++;

    // Keep attempting to parse commands until a valid one was encountered
    while (!GDB_Execute_i()) { /* Do Nothing */ }

    DEBUG_PRINT(stderr, "[Exit Break]\n");
    DEBUG_PRINT(stderr, " IsInteractive: %d, Retrigger %d\n", bIsInteractive, bRetrigger);
    mark3_thread = -1;
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
        if (u32Addr >= stCPU.u32ROMSize) {
            sprintf(ppcResponse_, "E01");
            return false;
        }
        // Eclipse seems to hate disassembling if we bail entirely...
        if ((u32Addr + u32Count) > (stCPU.u32ROMSize + 2)) {
            u32Count = (u32Addr + u32Count) - (stCPU.u32ROMSize + 2);
        }

        uint16_t r16;
        u32Addr >>= 1;
        while (u32Count > 1)
        {
            r16 = stCPU.pu16ROM[ u32Addr ];
            r16 = ((r16 & 0xFF00) >> 8) | ((r16 & 0x00FF) << 8);
            sprintf( dst, "%04x", &r16 );
            dst += 4;

            u32Count -= 2;
            u32Addr ++;
        }

        return false;
    }
    else if ((u32Addr >= 0x800000) && (u32Addr < 0x810000))
    {
        if ((u32Addr - 0x800000) > stCPU.u32RAMSize) {
            sprintf(ppcResponse_, "E01");
            return false;
        }

        if (((u32Addr - 0x800000) + u32Count) > stCPU.u32RAMSize) {
            u32Count = ((u32Addr - 0x800000) + u32Count) - stCPU.u32RAMSize;
        }

        r = (char*)&stCPU.pstRAM->au8RAM[u32Addr & 0xFFFFF];
    }
    else if (u32Addr >= 0x810000 && u32Addr <= 0x820000)
    {
        if ((u32Addr - 0x810000) > stCPU.u32EEPROMSize) {
            sprintf(ppcResponse_, "E01");
            return false;
        }

        if (((u32Addr - 0x810000) + u32Count) > stCPU.u32EEPROMSize) {
            u32Count = ((u32Addr - 0x810000) + u32Count) - stCPU.u32EEPROMSize;
        }
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
static bool GDB_Handler_ReadReg_i( const char *pcCmd_, char *ppcResponse_, uint8_t* r,
                                   uint8_t SREG, uint8_t SPH, uint8_t SPL, uint16_t u16PC_)
{
    char *src = (char*)&pcCmd_[1];
    char *dst = ppcResponse_;

    uint8_t u8Reg;

    sscanf(src, "%2X", &u8Reg );

    if (u8Reg < 32)
    {
        WRITE_HEX_BYTE(dst,r[u8Reg]);
    }
    else if (u8Reg == 32)
    {
        WRITE_HEX_BYTE(dst, SREG);
    }
    else if (u8Reg == 33)
    {
        WRITE_HEX_BYTE(dst, SPL);
        WRITE_HEX_BYTE(dst, SPH);
    }
    else if (u8Reg == 34)
    {
        uint16_t PC = u16PC_ << 1;
        WRITE_HEX_BYTE(dst, PC & 0x00FF );
        WRITE_HEX_BYTE(dst, PC >> 8 );
        WRITE_HEX_BYTE(dst, 0);
        WRITE_HEX_BYTE(dst, 0);
    }
    *dst = 0;
    return false;
}

//---------------------------------------------------------------------------
static bool GDB_Handler_ReadReg( const char *pcCmd_, char *ppcResponse_ )
{
    return GDB_Handler_ReadReg_i(pcCmd_, ppcResponse_, &(stCPU.pstRAM->stRegisters.CORE_REGISTERS.r[0]),
                                 stCPU.pstRAM->stRegisters.SREG.r,
                                 stCPU.pstRAM->stRegisters.SPH.r,
                                 stCPU.pstRAM->stRegisters.SPL.r,
                                 stCPU.u16PC );
}


//---------------------------------------------------------------------------
static bool GDB_Handler_ReadRegs_i( const char *pcCmd_, char *ppcResponse_, uint8_t* r,
                                    uint8_t SREG, uint8_t SPH, uint8_t SPL, uint16_t u16PC_)
{
    char *dst = ppcResponse_;
    int i;
    for (i = 0; i < 32; i++)
    {
        WRITE_HEX_BYTE(dst, r[i]);
    }

    WRITE_HEX_BYTE(dst, SREG);

    WRITE_HEX_BYTE(dst, SPL);
    WRITE_HEX_BYTE(dst, SPH);

    uint16_t PC = u16PC_ << 1;
    WRITE_HEX_BYTE(dst, PC & 0x00FF );
    WRITE_HEX_BYTE(dst, PC >> 8 );
    WRITE_HEX_BYTE(dst, 0);
    WRITE_HEX_BYTE(dst, 0);

    *dst = 0;
    return false;

}
//---------------------------------------------------------------------------
static bool GDB_Handler_ReadRegs( const char *pcCmd_, char *ppcResponse_ )
{
    if (mark3_thread == -1)
    {
        return GDB_Handler_ReadRegs_i(pcCmd_, ppcResponse_, &(stCPU.pstRAM->stRegisters.CORE_REGISTERS.r[0]),
                                 stCPU.pstRAM->stRegisters.SREG.r,
                                 stCPU.pstRAM->stRegisters.SPH.r,
                                 stCPU.pstRAM->stRegisters.SPL.r,
                                 stCPU.u16PC );
    }
    else
    {
        Mark3_Context_t *context = KA_Get_Thread_Context(mark3_thread);

        GDB_Handler_ReadRegs_i(pcCmd_, ppcResponse_, context->r,
                context->SREG,
                context->SPH,
                context->SPL,
                context->PC );
        free(context);
        return false;

    }
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

    data = strchr(pcCmd_, ':') + 1;

    if (u32Addr < 0x800000)
    {
        if (u32Addr >= stCPU.u32ROMSize) {
            sprintf(ppcResponse_, "E01");
            return false;
        }
        // Eclipse seems to hate disassembling if we bail entirely...
        if ((u32Addr + u32Count) > (stCPU.u32ROMSize + 2)) {
            u32Count = (u32Addr + u32Count) - (stCPU.u32ROMSize + 2);
        }

        // 16-bit words, address as such
        u32Addr >>= 1;

        uint16_t r16;
        while (u32Count)
        {
            sscanf( data, "%04x", &r16 );
            r16 = ((r16 & 0xFF00) >> 8) | ((r16 & 0x00FF) << 8);
            data += 4;

            stCPU.pu16ROM[ u32Addr ] = r16;
            u32Count -= 2;
            u32Addr ++;
        }

        return false;
    }
    else if ((u32Addr >= 0x800000) && (u32Addr < 0x810000))
    {
        if ((u32Addr - 0x800000) > stCPU.u32RAMSize) {
            sprintf(ppcResponse_, "E01");
            return false;
        }

        if (((u32Addr - 0x800000) + u32Count) > stCPU.u32RAMSize) {
            u32Count = ((u32Addr - 0x800000) + u32Count) - stCPU.u32RAMSize;
        }

        r = (char*)&stCPU.pstRAM->au8RAM[u32Addr & 0xFFFF];

        while (u32Count--)
        {
            READ_HEX_BYTE(data, r);
            r++;
        }

    }
    else if (u32Addr >= 0x810000 && u32Addr <= 0x820000)
    {
        if ((u32Addr - 0x810000) > stCPU.u32EEPROMSize) {
            sprintf(ppcResponse_, "E01");
            return false;
        }

        if (((u32Addr - 0x810000) + u32Count) > stCPU.u32EEPROMSize) {
            u32Count = ((u32Addr - 0x810000) + u32Count) - stCPU.u32EEPROMSize;
        }

        r = (char*)&stCPU.pu8EEPROM[ u32Addr & 0xFFFFF ];

        while (u32Count--)
        {
            READ_HEX_BYTE(data, r);
            r++;
        }
    }
    else
    {
        sprintf(ppcResponse_, "E01");
        GDB_Packetize(ppcResponse_);
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

extern char *KA_Get_Thread_Info_XML(uint8_t **thread_ids, uint16_t *thread_count);

//---------------------------------------------------------------------------
static bool GDB_Handler_Query( const char *pcCmd_, char *ppcResponse_ )
{
    if (0 != strstr(pcCmd_, "Supported"))
    {
#if 0
        if (Options_GetByName("--mark3"))
        {
            sprintf(ppcResponse_, "qXfer:threads:read+");
        }
        else
        {
            sprintf(ppcResponse_, "qXfer:memory-map:read+");
        }
#endif
    }
    else if (0 != strstr(pcCmd_, "Attached"))
    {
        sprintf(ppcResponse_, "1");
    }
#if 1
    else if (0 != strstr(pcCmd_, "fThreadInfo"))
    {
        //!!
        // Get a list of all active threads.
        uint8_t *ids = NULL;
        uint16_t count = 0;
        char *resp;
        resp = KA_Get_Thread_Info_XML(&ids, &count);
        free(resp);

        int i;
        char *out = ppcResponse_;

        if (!count)
        {
            sprintf(ppcResponse_, "m0");
            return false;
        }

        out += sprintf(out, "m%x", ids[0] + 1);
        for (i = 1; i < count; i++) {
            out += sprintf(out, ",%x", ids[i] + 1);
        }
        free(ids);
    }
    else if (0 != strstr(pcCmd_, "sThreadInfo"))
    {
        // Assume we won't have a long enough threadlist to saturate
        // multiple packets.
        sprintf(ppcResponse_,"l");
    }
    else if (0 != strstr(pcCmd_, "ThreadExtraInfo"))
    {
        int id;
        char *searchstr = strstr(pcCmd_, "," );
        searchstr++;
        sscanf(searchstr, "%x", &id);

        char unencoded[1024] = {0};
        if (id == 0)
        {
            sprintf(unencoded, "Kernel not started [Running]");
        }
        else if (id == 256)
        {
            sprintf(unencoded, "Idle Thread [Ready]");
        }
        else
        {
            int priority = KA_Get_Thread_Priority(id - 1);
            const char *state = KA_Get_Thread_State(id - 1);

            sprintf(unencoded, "Mk3: Pri=%d [%s]", priority, state);

        }
        int k;

        char *dst = ppcResponse_;
        for (k = 0; k < strlen(unencoded); k++)
        {
            sprintf( dst, "%02x", unencoded[k] );
            dst+=2;
        }
    }
    else if (0 == strcmp(pcCmd_, "qC"))
    {
        // Get the current running thread ID.
        int id = KA_Get_Thread_ID();
        if (id == 0)
        {
            sprintf(ppcResponse_, "QC0");
        }
        else
        {
            sprintf(ppcResponse_,"QC%x",id-1);
        }
    }
#endif
#if 0
    else if (0 != strstr(pcCmd_, "Xfer:memory-map:read"))
    {
        sprintf(ppcResponse_,
                    "l<memory-map>\n"
                    " <memory type='ram' start='0x800000' length='%#x'/>\n"
                    " <memory type='flash' start='0' length='%#x'>\n"
                    "  <property name='blocksize'>0x80</property>\n"
                    " </memory>\n"
                    "</memory-map>",
                    stCPU.u32RAMSize,
                    stCPU.u32ROMSize );
    }
#endif
    else if (0 != strstr(pcCmd_, "Xfer:threads:read"))
    {
        char *resp = KA_Get_Thread_Info_XML(NULL,NULL);
        sprintf(ppcResponse_, "%s", resp);
        free(resp);
    }
    return false;
}

//---------------------------------------------------------------------------
static bool GDB_Handler_QuestionMark( const char *pcCmd_, char *ppcResponse_ )
{
    GDB_SendStatus(ppcResponse_, 5);
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
    DEBUG_PRINT( stderr, "Continuing\n" );
    return true;
}

//---------------------------------------------------------------------------
static bool GDB_Handler_Step( const char *pcCmd_, char *ppcResponse_ )
{
    bRetrigger = true;
    bIsInteractive = true;
    bStepping = true;
    DEBUG_PRINT( stderr, "Stepping\n" );
    return true;
}

//---------------------------------------------------------------------------
static bool GDB_Handler_SetThread( const char *pcCmd_, char *ppcResponse_ )
{
    //!! ToDo -- tie this in with Kernel-Aware support (modify GetStatus +
    //!! ReadReg functions)
    int id;
    sscanf(&pcCmd_[2], "%x", &id );

    if (pcCmd_[1] == 'c') {
        //  Continue (only supported on current thread)
        //return GDB_Handler_Continue( pcCmd_, ppcResponse_ );
        sprintf(ppcResponse_, "OK");
    }
    else if (pcCmd_[1] == 'g') {
        // Get register info for the threads.
        mark3_thread = id;

        if (id == 0)
        {
            mark3_thread = -1; // current thread.
        }
        else if (id == 256) // idle context... not a real thread
        {
            mark3_thread = -1;
        }

        Mark3_Context_t *context = KA_Get_Thread_Context(id - 1);
        if (!context)
        {
            mark3_thread = -1; // current thread.
        }
        mark3_thread = id - 1;
        sprintf( ppcResponse_, "OK" );
        free(context);
    }

    return false;
}

//---------------------------------------------------------------------------
static bool GDB_Handler_SetBreakPoint( const char *pcCmd_, char *ppcResponse_ )
{
    uint16_t addr;
    char *addr_str;
    char *addr_end;

    addr_str = strstr(pcCmd_,",");
    addr_str++;

    addr_end = strstr(addr_str,",");
    *addr_end = 0;
    sscanf(addr_str, "%4x", &addr);
    addr >>= 1;
    switch (pcCmd_[1] == '0')
    {
    case 0: // Hard + soft breakpoints
    case 1:
        if (!BreakPoint_EnabledAtAddress( addr ))
        {
            DEBUG_PRINT(stderr, "Inserting breakpoint @ %04X", addr);
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
    addr >>= 1;

    //!! ToDo - access watchpoints.
    switch (pcCmd_[1] == '0')
    {    
    case 0: // no difference between hardware + software breakpoints in the emulator.
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
static bool GDB_Handler_Kill( const char *pcCmd_, char *ppcResponse_ )
{
    // Kill the target (exit flavr)
    exit(0);
}

//---------------------------------------------------------------------------
static bool GDB_Handler_PokeThread( const char *pcCmd_, char *ppcResponse_ )
{
    int id;
    sscanf( &pcCmd_[1], "%x", &id );

    // If we get a context back from the KA module, the thread's live.
    // Otherwise, the thread's dead.
    Mark3_Context_t *context = KA_Get_Thread_Context( id - 1);
    if (context)
    {
        sprintf( ppcResponse_, "OK" );
        free(context);
    }
    else
    {
        sprintf( ppcResponse_, "E01" );
    }

    return false;
}

//---------------------------------------------------------------------------
static void GDB_SendStatus( char *ppcResponse_, uint8_t signo_ )
{
    uint16_t PC = stCPU.u16PC << 1;
    sprintf(ppcResponse_, "T%02x20:%02x;21:%02x%02x;22:%02x%02x0000;",
        signo_, stCPU.pstRAM->stRegisters.SREG.r,
        stCPU.pstRAM->stRegisters.SPL.r,
        stCPU.pstRAM->stRegisters.SPH.r,
        PC & 0xff, (PC >> 8) & 0xff);
}

//---------------------------------------------------------------------------
static bool GDB_Handler_Unsupported( const char *pcCmd_, char *ppcResponse_ )
{
    DEBUG_PRINT( stderr, "[UNSUPPORTED COMMAND: %s]\n", pcCmd_ );

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
    // packet format: [$][data][#][2-byte checksum]
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
void GDB_SendAck( void )
{
    send(gdb_socket, "+", 1, 0);
}

//---------------------------------------------------------------------------
void GDB_Init( void )
{   
    bool watch_zero = true;
    if (Options_GetByName("--mark3"))
    {
        Debug_Symbol_t *main_sym = Symbol_Find_Func_By_Name("main");
        if (main_sym)
        {
            BreakPoint_Insert(main_sym->u32StartAddr);
            watch_zero = false;
        }
    }

    if (watch_zero)
    {
        BreakPoint_Insert(0);
    }

    WriteCallout_Add( GDB_WatchpointCallback, 0 );
    GDB_ServerCreate();
    GDB_InstallBreakHandler();
}
