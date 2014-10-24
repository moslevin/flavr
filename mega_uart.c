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
#include "avr_cpu.h"
#include "avr_peripheral.h"
#include "avr_periphregs.h"
#include "avr_interrupt.h"




#if 1
#define DEBUG_PRINT(...)
#else
#define DEBUG_PRINT printf
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
static void Echo_Tx(   )
{
    printf("%c", TSR);
}

//---------------------------------------------------------------------------
static void Echo_Rx(   )
{
    printf("%c", RSR);
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
static bool UART_IsDoubleSpeed(   )
{
    return (stCPU.pstRAM->stRegisters.UCSR0A.U2X0 == 1);
}

//---------------------------------------------------------------------------
static void UART_SetDoubleSpeed(   )
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

    CPU_RegisterInterruptCallback( TXC0_Callback, 0x14); // TX Complete
}

//---------------------------------------------------------------------------
static void UART_Read(void *context_, uint8_t ucAddr_, uint8_t *pucValue_ )
{
    DEBUG_PRINT( "UART Read: 0x%02x\n", ucAddr_);
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
static void UART_WriteBaudReg( )
{
    DEBUG_PRINT( "WriteBaud\n");
    uint16_t u16Baud =  (uint16_t)(stCPU.pstRAM->stRegisters.UBRR0L) |
                        ((uint16_t)(stCPU.pstRAM->stRegisters.UBRR0H) << 8);

    u32BaudTicks = u16Baud;
}

//---------------------------------------------------------------------------
static void UART_WriteDataReg( )
{
    DEBUG_PRINT("UART Write UDR...\n");
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
static void UART_WriteUCSR0B( uint8_t u8Value_)
{

}

//---------------------------------------------------------------------------
static void UART_WriteUCSR0C( uint8_t u8Value_)
{

}

//---------------------------------------------------------------------------
static void UART_Write(void *context_, uint8_t ucAddr_, uint8_t ucValue_ )
{    
    DEBUG_PRINT("UART Write: %2X=%2X\n", ucAddr_, ucValue_ );
    switch (ucAddr_)
    {
    case 0xC0:  //UCSR0A
        UART_WriteUCSR0A( ucValue_ );
        break;
    case 0xC1:  //UCSR0B
        DEBUG_PRINT("Write UCRS0B\n");
        stCPU.pstRAM->au8RAM[ ucAddr_ ] = ucValue_;
        break;
    case 0xC2:  //UCSR0C
        DEBUG_PRINT("Write UCRS0C\n");
        stCPU.pstRAM->au8RAM[ ucAddr_ ] = ucValue_;
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
            }
            // Nothing pending in the TXB?  Flag the TSR as empty, and
            // set the "Transmit complete" flag in the register.
            else
            {
                TXB = 0;
                TSR = 0;
                bTSR_Empty = true;

                UART_TxComplete();
            }
        }
    }
}

//---------------------------------------------------------------------------
static void UART_RxClock(void *context_ )
{
    if (UART_IsRxEnabled() && u32RxTicksRemaining)
    {
        u32RxTicksRemaining--;
        if (!u32RxTicksRemaining)
        {
            // Local echo of the freshly "shifted in" data to the terminal
            Echo_Rx();

            // Move data from receive shift register into the receive buffer
            RXB = RSR;
            RSR = 0;

            // Set the RX Complete flag
            UART_RxComplete();
        }
    }
}
//---------------------------------------------------------------------------
static void UART_Clock(void *context_ )
{    
    // Handle Rx and TX clocks.
    UART_TxClock(context_);
    UART_RxClock(context_);

    // Check interrupts.
    if (stCPU.pstRAM->stRegisters.SREG.I == 1)
    {
        //DEBUG_PRINT("Check UART Interrupts\n");
        if (UART_IsTxIntEnabled(  ) && UART_IsTxComplete(  ))
        {
            DEBUG_PRINT("TXC Interrupt\n");
            AVR_InterruptCandidate( 0x14 );
        }
        if (UART_IsDREIntEnabled(  ) && UART_IsEmpty(  ))
        {
            DEBUG_PRINT("DRE Interrupt\n");
            AVR_InterruptCandidate( 0x13 );
        }
        if (UART_IsRxIntEnabled(  ) && UART_IsRxComplete(  ))
        {
            printf("RXC Interrupt\n");
            AVR_InterruptCandidate( 0x12 );
        }
    }
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
