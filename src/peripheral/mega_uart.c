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
 * (c) Copyright 2014-17, Funkenstein Software Consulting, All rights reserved
 *     See license.txt for details
 ****************************************************************************/
/*!
  \file  mega_uart.c

  \brief Implements an atmega UART plugin.
*/

/*!
  Plugin must interface with the following registers:

  UDRn
  UCSRnA
  UCSRnB
  UCSRnC
  UBBRnL
  UBBRnH

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "avr_cpu.h"
#include "avr_peripheral.h"
#include "avr_periphregs.h"
#include "avr_interrupt.h"
#include "options.h"

#if 1
#define DEBUG_PRINT(...)
#else
#define DEBUG_PRINT printf
#endif

//---------------------------------------------------------------------------
static bool    use_uart_socket = false;

#if _WIN32
#include <io.h>
#include <WinSock2.h>
#include <WS2tcpip.h>

static SOCKET  listener_socket = INVALID_SOCKET;
static SOCKET  uart_socket     = INVALID_SOCKET;

#pragma comment(lib, "Ws2_32.lib")
static WSADATA ws;

//---------------------------------------------------------------------------
static void UART_BeginServer(void)
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

        const char *portnum = Options_GetByName("--uart");
        if (!portnum)
        {
            portnum = "4444";
        }

        err = getaddrinfo(NULL, portnum, &hints, &localaddr);
        if (0 != err)
        {
            DEBUG_PRINT(stderr, "Error getting address info - bailing\n");
            break;
        }

        // Create a socket to listen for UART connections
        listener_socket = socket(localaddr->ai_family, localaddr->ai_socktype, localaddr->ai_protocol);
        if (INVALID_SOCKET == listener_socket)
        {
            DEBUG_PRINT(stderr, "Error creating socket - bailing\n" );
            err = -1;
            break;
        }

        // Setup the TCP listening socket
        if (SOCKET_ERROR == bind(listener_socket, localaddr->ai_addr, (int)localaddr->ai_addrlen))
        {
            DEBUG_PRINT(stderr, "Error on socket bind - bailing\n");
            err = -1;
            break;
        }

        if (SOCKET_ERROR == listen(listener_socket, SOMAXCONN))
        {
            DEBUG_PRINT(stderr, "Error on socket listen - bailing\n");
            err = -1;
            break;
        }

        printf("[Waiting for incoming conneciton on port %s]\n", portnum);
        uart_socket = accept(listener_socket, NULL, NULL);
        if (INVALID_SOCKET == uart_socket)
        {
            DEBUG_PRINT(stderr, "Error on socket accept - bailing\n");
            err = -1;
            break;
        }

        unsigned long mode = 1;
        int rc = ioctlsocket(uart_socket, FIONBIO, &mode);
        if (NO_ERROR != rc) {
            DEBUG_PRINT(stderr, "Error setting non-blocking\n");
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
        if (INVALID_SOCKET != listener_socket)
        {
            closesocket(listener_socket);
        }
        if (INVALID_SOCKET != uart_socket)
        {
            closesocket(uart_socket);
        }
        WSACleanup();
        exit(-1);
    }

    printf("[UART Connected!]\n");
}
#else
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

static int  listener_socket = 0;
static int  uart_socket     = 0;

//---------------------------------------------------------------------------
static void UART_BeginServer(void)
{
    fprintf(stderr, "[Initializing UART socket]");

    const char *port_string = Options_GetByName("--uart");
    if (!port_string)
    {
        port_string = "4444";
    }
    int portnum = atoi(port_string);

    listener_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (listener_socket <= 0)
    {
        fprintf( stderr, "Error creating socket on port %s, bailing\n", port_string );
        exit(-1);
    }

    struct sockaddr_in serv_addr = { 0 };
    struct sockaddr_in cli_addr = { 0 };

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portnum);

    if (bind(listener_socket, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
    {
        fprintf(stderr, "Error binding socket -- bailing\n");
        exit(-1);
    }
    int enable = 1;
    setsockopt(listener_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));

    listen(listener_socket,1);

    int clilen = sizeof(cli_addr);
    uart_socket = accept(listener_socket, (struct sockaddr *)&cli_addr, &clilen);
    printf("[Waiting for incoming conneciton on port %d]\n", portnum);
    if (uart_socket < 0)
    {
         fprintf(stderr, "Error on accept -- bailing\n");
         exit(-1);
    }
    sleep(1);

    int flags;
    flags = fcntl(uart_socket, F_GETFL, 0);
    fcntl(uart_socket, F_SETFL, flags | O_NONBLOCK);

    printf( "[UART Connected!]" );
}

#endif
//---------------------------------------------------------------------------
static bool bUDR_Empty = true;
static bool bTSR_Empty = true;

static uint8_t RXB = 0; // receive buffer
static uint8_t TXB = 0; // transmit buffer
static uint8_t TSR = 0; // transmit shift register.
static uint8_t RSR = 0; // receive shift register.

static uint32_t u32BaudTicks = 0;
static uint32_t u32TxTicksRemaining = 0;
static uint32_t u32RxTicksRemaining = 0;

//---------------------------------------------------------------------------
static void Echo_Tx()
{
    if (use_uart_socket) {
        if (send(uart_socket, &TSR, 1, 0) <= 0) {
            exit(-1);
        }
    } else {
        printf("%c", TSR);
    }
}

//---------------------------------------------------------------------------
static void Echo_Rx()
{
    if (use_uart_socket) {
        if (send(uart_socket, &RSR, 1, 0) <= 0) {
            exit(-1);
        }
    } else {
        printf("%c", RSR);
    }
}

//---------------------------------------------------------------------------
static bool UART_IsRxEnabled( void )
{
    //DEBUG_PRINT( "RxEnabled\n");
    return (stCPU.pstRAM->stRegisters.UCSR0B.RXEN0 == 1);
}

//---------------------------------------------------------------------------
static bool UART_IsTxEnabled( void )
{
    //DEBUG_PRINT( "TxEnabled\n");
    return (stCPU.pstRAM->stRegisters.UCSR0B.TXEN0 == 1);
}

//---------------------------------------------------------------------------
static bool UART_IsTxIntEnabled( void )
{
    return (stCPU.pstRAM->stRegisters.UCSR0B.TXCIE0 == 1);
}

//---------------------------------------------------------------------------
static bool UART_IsDREIntEnabled( void )
{
    return (stCPU.pstRAM->stRegisters.UCSR0B.UDRIE0 == 1);
}

//---------------------------------------------------------------------------
static bool UART_IsRxIntEnabled( void )
{
    return (stCPU.pstRAM->stRegisters.UCSR0B.RXCIE0 == 1);
}

//---------------------------------------------------------------------------
static bool UART_IsDoubleSpeed()
{
    return (stCPU.pstRAM->stRegisters.UCSR0A.U2X0 == 1);
}

//---------------------------------------------------------------------------
static void UART_SetDoubleSpeed()
{
    stCPU.pstRAM->stRegisters.UCSR0A.U2X0 = 1;
}

//---------------------------------------------------------------------------
static void UART_SetEmpty( void )
{
    stCPU.pstRAM->stRegisters.UCSR0A.UDRE0 = 1;
}

//---------------------------------------------------------------------------
static void UART_ClearEmpty( void )
{
    stCPU.pstRAM->stRegisters.UCSR0A.UDRE0 = 0;
}

//---------------------------------------------------------------------------
static bool UART_IsEmpty( void )
{
    return (stCPU.pstRAM->stRegisters.UCSR0A.UDRE0 == 1);
}

//---------------------------------------------------------------------------
static bool UART_IsTxComplete( void )
{
    return (stCPU.pstRAM->stRegisters.UCSR0A.TXC0 == 1);
}

//---------------------------------------------------------------------------
static void UART_TxComplete( void )
{
    stCPU.pstRAM->stRegisters.UCSR0A.TXC0 = 1;    
}

//---------------------------------------------------------------------------
static bool UART_IsRxComplete( void )
{
    return (stCPU.pstRAM->stRegisters.UCSR0A.RXC0 == 1);
}

//---------------------------------------------------------------------------
static void UART_RxComplete( void )
{
    stCPU.pstRAM->stRegisters.UCSR0A.RXC0 = 1;
}

//---------------------------------------------------------------------------
static void TXC0_Callback(  uint8_t ucVector_ )
{
    // On TX Complete interrupt, automatically clear the TXC0 flag.
    stCPU.pstRAM->stRegisters.UCSR0A.TXC0 = 0;
}

//---------------------------------------------------------------------------
static void UART_Init(void *context_ )
{
    DEBUG_PRINT("UART Init\n");
    stCPU.pstRAM->stRegisters.UCSR0A.UDRE0 = 1;

    CPU_RegisterInterruptCallback(TXC0_Callback, stCPU.pstVectorMap->USART0_TX); // TX Complete

    if (Options_GetByName("--uart")) {
        use_uart_socket = true;
        UART_BeginServer();
    }
}

//---------------------------------------------------------------------------
static void UART_Read(void *context_, uint8_t ucAddr_, uint8_t *pucValue_ )
{
    DEBUG_PRINT( "UART Read: 0x%02x == 0x%02X\n", ucAddr_, stCPU.pstRAM->au8RAM[ ucAddr_ ]);
    DEBUG_PRINT("ADDR=%08X\n", stCPU.u32PC);
    *pucValue_ = stCPU.pstRAM->au8RAM[ ucAddr_ ];
    switch (ucAddr_)
    {
        case 0xC6: // UDR0
            stCPU.pstRAM->stRegisters.UCSR0A.RXC0 = 0;
            break;
        default:
            break;
    }
}

//---------------------------------------------------------------------------
static void UART_WriteBaudReg()
{
    DEBUG_PRINT( "WriteBaud\n");
    uint16_t u16Baud =  (uint16_t)(stCPU.pstRAM->stRegisters.UBRR0L) |
                        ((uint16_t)(stCPU.pstRAM->stRegisters.UBRR0H) << 8);

    u32BaudTicks = u16Baud;
}

//---------------------------------------------------------------------------
static void UART_WriteDataReg()
{
    DEBUG_PRINT("UART Write UDR...\n");
    DEBUG_PRINT("ADDR=%08X\n", stCPU.u32PC);
    if (UART_IsTxEnabled())
    {
        DEBUG_PRINT("Enabled...\n");
        // Only set the baud timer if the UART is idle
        if (!u32TxTicksRemaining)
        {
            u32TxTicksRemaining = u32BaudTicks;
            if (UART_IsDoubleSpeed())
            {
                u32TxTicksRemaining >>= 1;
            }
        }

        // If the shift register is empty, load it immediately
        if (bTSR_Empty)
        {            
            TSR = stCPU.pstRAM->stRegisters.UDR0;
            TXB = 0;
            bTSR_Empty = false;
            bUDR_Empty = true;
            UART_SetEmpty();

            if (UART_IsDREIntEnabled())
            {
                DEBUG_PRINT("DRE Interrupt\n");
                AVR_InterruptCandidate( stCPU.pstVectorMap->USART0_UDRE );
            }
        }        
        else
        {
            TXB = stCPU.pstRAM->stRegisters.UDR0;
            bTSR_Empty = false;
            bUDR_Empty = false;
            UART_ClearEmpty();
        }
    }
    else
    {
        DEBUG_PRINT("Disabled...\n");
    }
}

//---------------------------------------------------------------------------
static void UART_WriteUCSR0A( uint8_t u8Value_)
{
    DEBUG_PRINT("UART Write UCSR0A...\n");
    uint8_t u8Reg = stCPU.pstRAM->stRegisters.UCSR0A.r;
    if (u8Value_ & 0x40) // TXC was set explicitly -- clear it in the SR.
    {
        u8Reg &= ~0x40;
    }
    u8Reg &= ~(0xBC);

    stCPU.pstRAM->stRegisters.UCSR0A.r |= u8Reg;
}

//---------------------------------------------------------------------------
static void UART_UpdateInterruptFlags(void)
{
    //DEBUG_PRINT("Check UART Interrupts\n");
    if (UART_IsTxIntEnabled())
    {
        if (UART_IsTxComplete())
        {
            DEBUG_PRINT("Enable TXC Interrupt\n");
            AVR_InterruptCandidate( stCPU.pstVectorMap->USART0_TX );
        }
        else
        {
            DEBUG_PRINT("Clear TXC Interrupt\n");
            AVR_ClearCandidate( stCPU.pstVectorMap->USART0_TX );
        }
    }
    if (UART_IsDREIntEnabled())
    {
        if( UART_IsEmpty())
        {
            DEBUG_PRINT("Enable DRE Interrupt\n");
            AVR_InterruptCandidate( stCPU.pstVectorMap->USART0_UDRE );
        }
        else
        {
            DEBUG_PRINT("Clear DRE Interrupt\n");
            AVR_ClearCandidate( stCPU.pstVectorMap->USART0_UDRE );
        }
    }
    if (UART_IsRxIntEnabled())
    {
        if (UART_IsRxComplete())
        {
            DEBUG_PRINT("Enable RXC Interrupt\n");
            AVR_InterruptCandidate( stCPU.pstVectorMap->USART0_RX );
        }
        else
        {
            DEBUG_PRINT("Clear RXC Interrupt\n");
            AVR_ClearCandidate( stCPU.pstVectorMap->USART0_RX );
        }
    }
}

//---------------------------------------------------------------------------
static void UART_WriteUCSR0B( uint8_t u8Value_)
{
    DEBUG_PRINT("Write UCSR0B = %02x\n", u8Value_);
    stCPU.pstRAM->stRegisters.UCSR0B.r = u8Value_;
    UART_UpdateInterruptFlags();
}

//---------------------------------------------------------------------------
static void UART_WriteUCSR0C( uint8_t u8Value_)
{
    DEBUG_PRINT("Write UCRS0C\n");
    stCPU.pstRAM->stRegisters.UCSR0C.r == u8Value_;
}

//---------------------------------------------------------------------------
static void UART_Write(void *context_, uint8_t ucAddr_, uint8_t ucValue_ )
{    
    DEBUG_PRINT("UART Write: %2X=%2X\n", ucAddr_, ucValue_ );
    DEBUG_PRINT("ADDR=%08X\n", stCPU.u32PC);
    switch (ucAddr_)
    {
    case 0xC0:  //UCSR0A
        UART_WriteUCSR0A( ucValue_ );
        break;
    case 0xC1:  //UCSR0B
        UART_WriteUCSR0B( ucValue_ );
        break;
    case 0xC2:  //UCSR0C
        UART_WriteUCSR0C( ucValue_ );
        break;
    case 0xC3:  // NA.
        break;
    case 0xC4:  //UBRR0L
    case 0xC5:  //UBRR0H
        DEBUG_PRINT("Write UBRR0x\n");
        stCPU.pstRAM->au8RAM[ ucAddr_ ] = ucValue_;
        UART_WriteBaudReg();
        break;
    case 0xC6:  //UDR0
        DEBUG_PRINT("Write UDR0\n");
        stCPU.pstRAM->au8RAM[ ucAddr_ ] = ucValue_;
        UART_WriteDataReg();
        break;
    default:
        break;
    }
}

//---------------------------------------------------------------------------
static void UART_TxClock(void *context_ )
{
    //DEBUG_PRINT("TX clock...\n");
    if (UART_IsTxEnabled() && u32TxTicksRemaining)
    {
        DEBUG_PRINT("Countdown %d ticks remain\n", u32TxTicksRemaining);
        u32TxTicksRemaining--;
        if (!u32TxTicksRemaining)
        {
            // Local echo of the freshly "shifted out" data to the terminal
            Echo_Tx();

            // If there's something queued in the TXB, reload the TSR
            // register, flag the UDR as empty, and TSR as full.
            if (!bUDR_Empty)
            {
                TSR = TXB;
                TXB = 0;
                bUDR_Empty = true;
                bTSR_Empty = false;

                UART_SetEmpty();

                if (UART_IsDREIntEnabled())
                {
                    DEBUG_PRINT("DRE Interrupt\n");
                    AVR_InterruptCandidate( stCPU.pstVectorMap->USART0_UDRE );
                }
            }
            // Nothing pending in the TXB?  Flag the TSR as empty, and
            // set the "Transmit complete" flag in the register.
            else
            {
                TXB = 0;
                TSR = 0;
                bTSR_Empty = true;

                UART_TxComplete();
                if (UART_IsTxIntEnabled())
                {
                    DEBUG_PRINT("TXC Interrupt\n");
                    AVR_InterruptCandidate( stCPU.pstVectorMap->USART0_TX );
                }
            }
        }
    }    
}

//---------------------------------------------------------------------------
static void UART_RxClock(void *context_ )
{
    if (UART_IsRxEnabled())
    {
        if (u32RxTicksRemaining) {
            u32RxTicksRemaining--;
            if (!u32RxTicksRemaining)
            {
                // Move data from receive shift register into the receive buffer
                RXB = RSR;
                RSR = 0;

                stCPU.pstRAM->stRegisters.UDR0 = RXB;

                // Set the RX Complete flag
                UART_RxComplete();
                if (UART_IsRxIntEnabled())
                {
                    DEBUG_PRINT("RXC Interrupt\n");
                    AVR_InterruptCandidate( stCPU.pstVectorMap->USART0_RX );
                }
            }
        } else {
            if (use_uart_socket) {
                static int interval = 0;
                interval++;
                if (interval == 200) { // poll for input every X cycles
                    interval = 0;
                    uint8_t rx_byte;
                    int bytes_read = recv(uart_socket, &rx_byte, 1, 0);
                    if (bytes_read == 1) {
                        RSR = rx_byte;
                        u32RxTicksRemaining = u32BaudTicks;
                    }
                }
            }
        }
    }
}
//---------------------------------------------------------------------------
static void UART_Clock(void *context_ )
{    
    // Handle Rx and TX clocks.
    UART_TxClock(context_);
    UART_RxClock(context_);
}

//---------------------------------------------------------------------------
AVRPeripheral stUART =
{
    UART_Init,
    UART_Read,
    UART_Write,
    UART_Clock,
    0,
    0xC0,
    0xC6
};
