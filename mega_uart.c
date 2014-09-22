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

#define DEBUG_PRINT(...)


#if 0

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
static void Echo_Tx( AVR_CPU *pstCPU_ )
{
    printf("%c", TSR);
}

//---------------------------------------------------------------------------
static void Echo_Rx( AVR_CPU *pstCPU_ )
{
    printf("%c", RSR);
}

//---------------------------------------------------------------------------
static bool UART_IsRxEnabled( struct _AVR_CPU *pstCPU_)
{
    // DEBUG_PRINT( "RxEnabled\n");
    return (pstCPU_->pstRAM->stRegisters.UCSR0B.RXEN0 == 1);
}

//---------------------------------------------------------------------------
static bool UART_IsTxEnabled( struct _AVR_CPU *pstCPU_)
{
    // DEBUG_PRINT( "TxEnabled\n");
    return (pstCPU_->pstRAM->stRegisters.UCSR0B.TXEN0 == 1);
}

//---------------------------------------------------------------------------
static bool UART_IsTxIntEnabled( struct _AVR_CPU *pstCPU_)
{
    return (pstCPU_->pstRAM->stRegisters.UCSR0B.TXCIE0 == 1);
}

//---------------------------------------------------------------------------
static bool UART_IsDREIntEnabled( struct _AVR_CPU *pstCPU_)
{
    return (pstCPU_->pstRAM->stRegisters.UCSR0B.UDRIE0 == 1);
}

//---------------------------------------------------------------------------
static bool UART_IsRxIntEnabled( struct _AVR_CPU *pstCPU_)
{
    return (pstCPU_->pstRAM->stRegisters.UCSR0B.RXCIE0 == 1);
}

//---------------------------------------------------------------------------
static bool UART_IsDoubleSpeed( struct _AVR_CPU *pstCPU_ )
{
    return (pstCPU_->pstRAM->stRegisters.UCSR0A.U2X0 == 1);
}

//---------------------------------------------------------------------------
static void UART_SetDoubleSpeed( struct _AVR_CPU *pstCPU_ )
{
    pstCPU_->pstRAM->stRegisters.UCSR0A.U2X0 = 1;
}

//---------------------------------------------------------------------------
static void UART_SetEmpty( struct _AVR_CPU *pstCPU_)
{
    pstCPU_->pstRAM->stRegisters.UCSR0A.UDRE0 = 1;
}

//---------------------------------------------------------------------------
static void UART_ClearEmpty( struct _AVR_CPU *pstCPU_)
{
    pstCPU_->pstRAM->stRegisters.UCSR0A.UDRE0 = 0;
}

//---------------------------------------------------------------------------
static bool UART_IsEmpty( struct _AVR_CPU *pstCPU_)
{
    return (pstCPU_->pstRAM->stRegisters.UCSR0A.UDRE0 == 1);
}

//---------------------------------------------------------------------------
static bool UART_IsTxComplete( struct _AVR_CPU *pstCPU_)
{
    return (pstCPU_->pstRAM->stRegisters.UCSR0A.TXC0 == 1);
}

//---------------------------------------------------------------------------
static void UART_TxComplete( struct _AVR_CPU *pstCPU_)
{
    pstCPU_->pstRAM->stRegisters.UCSR0A.TXC0 = 1;    
}

//---------------------------------------------------------------------------
static bool UART_IsRxComplete( struct _AVR_CPU *pstCPU_)
{
    return (pstCPU_->pstRAM->stRegisters.UCSR0A.RXC0 == 1);
}

//---------------------------------------------------------------------------
static void UART_RxComplete( struct _AVR_CPU *pstCPU_)
{
    pstCPU_->pstRAM->stRegisters.UCSR0A.RXC0 = 1;
}

//---------------------------------------------------------------------------
static void UART_Init(void *context_, struct _AVR_CPU *pstCPU_)
{
    DEBUG_PRINT(stderr, "UART Init\n");
    pstCPU_->pstRAM->stRegisters.UCSR0A.UDRE0 = 1;
}

//---------------------------------------------------------------------------
static void UART_Read(void *context_, struct _AVR_CPU *pstCPU_, uint8_t ucAddr_, uint8_t *pucValue_ )
{
    DEBUG_PRINT(stderr, "UART Read: 0x%02x\n", ucAddr_);
    *pucValue_ = pstCPU_->pstRAM->aucRAM[ ucAddr_ ];
}

//---------------------------------------------------------------------------
static void UART_WriteBaudReg(struct _AVR_CPU *pstCPU_)
{
    DEBUG_PRINT( "WriteBaud\n");
    uint16_t u16Baud =  (uint16_t)(pstCPU_->pstRAM->stRegisters.UBRR0L) |
                        ((uint16_t)(pstCPU_->pstRAM->stRegisters.UBRR0H) << 8);

    u32BaudTicks = u16Baud;
}

//---------------------------------------------------------------------------
static void UART_WriteDataReg(struct _AVR_CPU *pstCPU_)
{
    if (UART_IsTxEnabled(pstCPU_))
    {
        // Only set the baud timer if the UART is idle
        if (!u32TxTicksRemaining)
        {
            u32TxTicksRemaining = u32BaudTicks;
            if (UART_IsDoubleSpeed(pstCPU_))
            {
                u32TxTicksRemaining >>= 1;
            }
        }

        // If the shift register is empty, load it immediately
        if (bTSR_Empty)
        {
            TSR = pstCPU_->pstRAM->stRegisters.UDR0;
            TXB = 0;
            bTSR_Empty = false;
            bUDR_Empty = true;
            UART_SetEmpty(pstCPU_);
        }
        // Otherwise, just load the TXB register, and wait for the current
        // shift operation to end.
        else
        {
            TXB = pstCPU_->pstRAM->stRegisters.UDR0;
            bTSR_Empty = false;
            bUDR_Empty = false;
        }
    }        
}

//---------------------------------------------------------------------------
static void UART_WriteUCSR0A(struct _AVR_CPU *pstCPU_, uint8_t u8Value_)
{
    uint8_t u8Reg = pstCPU_->pstRAM->stRegisters.UCSR0A.r;
    if (u8Value_ & 0x40) // TXC was set explicitly -- clear it in the SR.
    {
        u8Reg &= ~0x40;
    }
    u8Reg &= ~(0xBC);

    pstCPU_->pstRAM->stRegisters.UCSR0A.r |= u8Reg;

}

//---------------------------------------------------------------------------
static void UART_WriteUCSR0B(struct _AVR_CPU *pstCPU_, uint8_t u8Value_)
{

}

//---------------------------------------------------------------------------
static void UART_WriteUCSR0C(struct _AVR_CPU *pstCPU_, uint8_t u8Value_)
{

}

//---------------------------------------------------------------------------
static void UART_Write(void *context_, struct _AVR_CPU *pstCPU_, uint8_t ucAddr_, uint8_t ucValue_ )
{    
    switch (ucAddr_)
    {
    case 0xC0:  //UCSR0A
        UART_WriteUCSR0A( pstCPU_, ucValue_ );
        break;
    case 0xC1:  //UCSR0B
        pstCPU_->pstRAM->aucRAM[ ucAddr_ ] = ucValue_;
        break;
    case 0xC2:  //UCSR0C
        pstCPU_->pstRAM->aucRAM[ ucAddr_ ] = ucValue_;
        break;
    case 0xC3:  // NA.
        break;
    case 0xC4:  //UBRR0L
    case 0xC5:  //UBRR0H
        pstCPU_->pstRAM->aucRAM[ ucAddr_ ] = ucValue_;
        UART_WriteBaudReg(pstCPU_);
        break;
    case 0xC6:  //UDR0
        pstCPU_->pstRAM->aucRAM[ ucAddr_ ] = ucValue_;
        UART_WriteDataReg(pstCPU_);
        break;
    default:
        break;
    }
}

//---------------------------------------------------------------------------
static void UART_TxClock(void *context_, struct _AVR_CPU *pstCPU_)
{
    if (UART_IsTxEnabled(pstCPU_) && u32TxTicksRemaining)
    {
        u32TxTicksRemaining--;
        if (!u32TxTicksRemaining)
        {
            // Local echo of the freshly "shifted out" data to the terminal
            Echo_Tx(pstCPU_);

            // If there's something queued in the TXB, reload the TSR
            // register, flag the UDR as empty, and TSR as full.
            if (!bUDR_Empty)
            {
                TSR = TXB;
                TXB = 0;
                bUDR_Empty = true;
                bTSR_Empty = false;

                UART_SetEmpty(pstCPU_);
            }
            // Nothing pending in the UDR?  Flag the TSR as empty, and
            // set the "Transmit complete" flag in the register.
            else
            {
                TXB = 0;
                TSR = 0;
                bTSR_Empty = true;

                UART_TxComplete(pstCPU_);
            }
        }
    }
}

//---------------------------------------------------------------------------
static void UART_RxClock(void *context_, struct _AVR_CPU *pstCPU_)
{
    if (UART_IsRxEnabled(pstCPU_) && u32RxTicksRemaining)
    {
        u32RxTicksRemaining--;
        if (!u32RxTicksRemaining)
        {
            // Local echo of the freshly "shifted in" data to the terminal
            Echo_Rx(pstCPU_);

            // Move data from receive shift register into the receive buffer
            RXB = RSR;
            RSR = 0;

            // Set the RX Complete flag
            UART_RxComplete(pstCPU_);
        }
    }
}
//---------------------------------------------------------------------------
static void UART_Clock(void *context_, struct _AVR_CPU *pstCPU_)
{    
    // Handle Rx and TX clocks.
    UART_TxClock(context_, pstCPU_);
    UART_RxClock(context_, pstCPU_);

    // Check interrupts.
    if (pstCPU_->pstRAM->stRegisters.SREG.I == 1)
    {
        if (UART_IsTxIntEnabled( pstCPU_ ) && UART_IsTxComplete( pstCPU_ ))
        {
            AVR_InterruptCandidate( pstCPU_, 0x14 );
        }
        if (UART_IsDREIntEnabled( pstCPU_ ) && UART_IsEmpty( pstCPU_ ))
        {
            AVR_InterruptCandidate( pstCPU_, 0x13 );
        }
        if (UART_IsRxIntEnabled( pstCPU_ ) && UART_IsRxComplete( pstCPU_ ))
        {
            AVR_InterruptCandidate( pstCPU_, 0x12 );
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
