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
  \file  mega_timer8.c

  \brief ATMega 8-bit timer implementation.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "avr_cpu.h"
#include "avr_peripheral.h"
#include "avr_periphregs.h"
#include "avr_interrupt.h"

#define DEBUG_PRINT(...)

//---------------------------------------------------------------------------
//!! This implementation only tracks the basic timer/capture/compare
//!  functionality of the peripheral, to match what's used in Mark3.
//!  Future considerations, TBD.
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
typedef enum
{
    CLK_SRC_OFF,
    CLK_SRC_DIV_1,
    CLK_SRC_DIV_8,
    CLK_SRC_DIV_64,
    CLK_SRC_DIV_256,
    CLK_SRC_DIV_1024,
    CLK_SRC_T1_FALL,
    CLK_SRC_T1_RISE
} ClockSource_t;

//---------------------------------------------------------------------------
typedef enum
{
    WGM_NORMAL,
    WGM_PWM_PC_FF,
    WGM_CTC_OCR,
    WGM_FAST_PWM_FF,
    WGM_RESERVED_1, // Not a valid mode
    WGM_PWM_PC_OCR,
    WGM_RESERVED_2, // Not a valid mode
    WGM_FAST_PWM_OCR
} WaveformGeneratorMode_t;

//---------------------------------------------------------------------------
typedef enum
{
    COM_NORMAL,         // OCA
    COM_TOGGLE_MATCH,   // Toggle on match
    COM_CLEAR_MATCH,
    COM_SET_MATCH
} CompareOutputMode_t;

//---------------------------------------------------------------------------
static uint16_t u16DivCycles = 0;
static uint16_t u16DivRemain = 0;
static ClockSource_t eClockSource   = CLK_SRC_OFF;
static WaveformGeneratorMode_t eWGM = WGM_NORMAL;
static CompareOutputMode_t eCOM1A = COM_NORMAL;
static CompareOutputMode_t eCOM1B = COM_NORMAL;

//---------------------------------------------------------------------------
static uint8_t  u8Temp;  // The 8-bit temporary register used in 16-bit register accesses
static uint16_t u8Count; // Internal 16-bit count register

//---------------------------------------------------------------------------
static void TCNT0_Increment()
{
    stCPU.pstRAM->stRegisters.TCNT0++;    
}

//---------------------------------------------------------------------------
static uint8_t TCNT0_Read()
{    
    return stCPU.pstRAM->stRegisters.TCNT0;
}

//---------------------------------------------------------------------------
static void TCNT0_Clear()
{
    stCPU.pstRAM->stRegisters.TCNT0 = 0;
}

//---------------------------------------------------------------------------
static uint8_t OCR0A_Read()
{    
    return stCPU.pstRAM->stRegisters.OCR0A;
}

//---------------------------------------------------------------------------
static uint8_t OCR0B_Read()
{
    return stCPU.pstRAM->stRegisters.OCR0B;
}

//---------------------------------------------------------------------------
static bool Timer8_Is_TOIE0_Enabled()
{
    return (stCPU.pstRAM->stRegisters.TIMSK0.TOIE0 == 1);
}

//---------------------------------------------------------------------------
static bool Timer8_Is_OCIE0A_Enabled()
{
    return (stCPU.pstRAM->stRegisters.TIMSK0.OCIE0A == 1);
}

//---------------------------------------------------------------------------
static bool Timer8_Is_OCIE1B_Enabled()
{
    return (stCPU.pstRAM->stRegisters.TIMSK0.OCIE0B == 1);
}

//---------------------------------------------------------------------------
static void OV0_Ack(  uint8_t ucVector_)
{
    static uint64_t lastcycles = 0;
    stCPU.pstRAM->stRegisters.TIFR0.TOV0 = 0;
   // printf("OV0 - Ack'd: %d delta\n", stCPU.u64CycleCount - lastcycles);
    lastcycles = stCPU.u64CycleCount;
}

//---------------------------------------------------------------------------
static void COMP0A_Ack(  uint8_t ucVector_)
{
    stCPU.pstRAM->stRegisters.TIFR0.OCF0A = 0;
}

//---------------------------------------------------------------------------
static void COMP0B_Ack(  uint8_t ucVector_)
{
    stCPU.pstRAM->stRegisters.TIFR0.OCF0B = 0;
}

//---------------------------------------------------------------------------
static void Timer8_Init(void *context_ )
{
    DEBUG_PRINT( "Timer8 Init\n");
    CPU_RegisterInterruptCallback( OV0_Ack, 0x10);
    CPU_RegisterInterruptCallback( COMP0A_Ack, 0x0E);
    CPU_RegisterInterruptCallback( COMP0B_Ack, 0x0F);
}

//---------------------------------------------------------------------------
static void Timer8_Read(void *context_, uint8_t ucAddr_, uint8_t *pucValue_ )
{
    DEBUG_PRINT( "Timer8 Read: 0x%02x\n", ucAddr_);
    *pucValue_ = stCPU.pstRAM->au8RAM[ ucAddr_ ];
}

//---------------------------------------------------------------------------
static void TCCR0A_Write( uint8_t ucAddr_, uint8_t ucValue_)
{
    // Update the waveform generator mode (WGM1:0) bits.
    uint8_t u8WGMBits = ucValue_ & 0x03; // WGM1 and 0 are in bits 0,1
    uint8_t u8WGMTemp = (uint8_t)eWGM;
    u8WGMTemp &= ~(0x03);
    u8WGMTemp |= u8WGMBits;
    eWGM = (WaveformGeneratorMode_t)u8WGMTemp;

    // Update the memory-mapped register.
    stCPU.pstRAM->stRegisters.TCCR0A.r = ucValue_ & 0xF3;
}

//---------------------------------------------------------------------------
static void TCCR0B_Write( uint8_t ucAddr_, uint8_t ucValue_)
{
    // Update the waveform generator mode (WGM2) bit
    uint8_t u8WGMBits = (ucValue_ >> 1) & 0x04; // WGM2 is in bit 3 of the register
    uint8_t u8WGMTemp = (uint8_t)eWGM;
    u8WGMTemp &= ~(0x04);
    u8WGMTemp |= u8WGMBits;
    eWGM = (WaveformGeneratorMode_t)u8WGMTemp;

    // Update the clock-select bits
    uint8_t u8ClockSource = ucValue_ & 0x07; // clock select is last 3 bits in reg
    eClockSource = (ClockSource_t)u8ClockSource;
    switch (eClockSource)
    {
    case CLK_SRC_DIV_1:
        u16DivCycles = 1;
        break;
    case CLK_SRC_DIV_8:
        u16DivCycles = 8;
        break;
    case CLK_SRC_DIV_64:
        u16DivCycles = 64;
        break;
    case CLK_SRC_DIV_256:
        u16DivCycles = 256;
        break;
    case CLK_SRC_DIV_1024:
        u16DivCycles = 1024;
        break;
    default:
        u16DivCycles = 0;
        break;
    }
    DEBUG_PRINT(" ClockSource = %d, %d cycles\n", eClockSource, u16DivCycles);
    // Update the memory-mapped register.
    stCPU.pstRAM->stRegisters.TCCR0B.r = ucValue_ & 0xCF; // Bit 5&6 are read-only
}

//---------------------------------------------------------------------------
static void TCNT0_Write( uint8_t ucAddr_, uint8_t ucValue_)
{    
    stCPU.pstRAM->stRegisters.TCNT0 = ucValue_;
}

//---------------------------------------------------------------------------
static void OCR0A_Write( uint8_t ucAddr_, uint8_t ucValue_)
{    
    stCPU.pstRAM->stRegisters.OCR0A = ucValue_;
}

//---------------------------------------------------------------------------
static void OCR0B_Write( uint8_t ucAddr_, uint8_t ucValue_)
{
    stCPU.pstRAM->stRegisters.OCR0B = ucValue_;
}

//---------------------------------------------------------------------------
static void Timer8_IntFlagUpdate(void)
{
    if (stCPU.pstRAM->stRegisters.TIMSK0.TOIE0 == 1)
    {
        if (stCPU.pstRAM->stRegisters.TIFR0.TOV0 == 1)
        {
            DEBUG_PRINT(" TOV0 Interrupt Candidate\n" );
            AVR_InterruptCandidate(0x10);
        }
        else
        {
            AVR_ClearCandidate(0x10);
        }
    }
    if (stCPU.pstRAM->stRegisters.TIMSK0.OCIE0A == 1)
    {
        if (stCPU.pstRAM->stRegisters.TIFR0.OCF0A == 1)
        {
            DEBUG_PRINT(" OCF0A Interrupt Candidate\n" );
            AVR_InterruptCandidate(0x0E);
        }
        else
        {
            AVR_ClearCandidate(0x0E);
        }
    }
    if (stCPU.pstRAM->stRegisters.TIMSK0.OCIE0B == 1)
    {
        if (stCPU.pstRAM->stRegisters.TIFR0.OCF0B == 1)
        {
            DEBUG_PRINT(" OCF0B Interrupt Candidate\n" );
            AVR_InterruptCandidate(0x0F);
        }
        else
        {
            AVR_ClearCandidate(0x0F);
        }
    }
}

//--------------------------------------------------------------------------
static void Timer8b_Write(void *context_, uint8_t ucAddr_, uint8_t ucValue_ )
{
    stCPU.pstRAM->au8RAM[ucAddr_] = ucValue_;
    Timer8_IntFlagUpdate();
}

//--------------------------------------------------------------------------
static void Timer8_Write(void *context_, uint8_t ucAddr_, uint8_t ucValue_ )
{
    DEBUG_PRINT("Timer8_Write: %d=%d\n", ucAddr_, ucValue_);
    switch (ucAddr_)
    {
    case 0x44:  //TCCR1A
        TCCR0A_Write(ucAddr_, ucValue_);
        break;
    case 0x45:  //TCCR1B
        TCCR0B_Write(ucAddr_, ucValue_);
        break;
    case 0x46:  // TCNT0
        TCNT0_Write(ucAddr_, ucValue_);
        break;
    case 0x47:  // OCR0A
        OCR0A_Write(ucAddr_, ucValue_);
        break;
    case 0x48:  // OCR0B
        OCR0B_Write(ucAddr_, ucValue_);
        break;
    default:
        break;
    }
}

//---------------------------------------------------------------------------
static void Timer8_Clock(void *context_ )
{
    if (eClockSource == CLK_SRC_OFF)
    {        
        return;
    }

    // Handle clock division logic
    bool bUpdateTimer = false;
    switch (eClockSource)
    {
    case CLK_SRC_DIV_1:
    case CLK_SRC_DIV_8:
    case CLK_SRC_DIV_64:
    case CLK_SRC_DIV_256:
    case CLK_SRC_DIV_1024:
    {
        // Decrement the clock-divide value
        if (u16DivRemain)
        {
            //DEBUG_PRINT(" %d ticks remain\n", u16DivRemain);
            u16DivRemain--;
        }

        if (!u16DivRemain)
        {
            // clock-divider count hits zero, reset and trigger an update.
            DEBUG_PRINT(" expire and reset\n");
            if (u16DivCycles)
            {
                u16DivRemain = u16DivCycles;
                bUpdateTimer = true;
            }
        }
    }
        break;
    default:
        //!! ToDo - Handle external timer generated events.
        break;
    }


    if (bUpdateTimer)
    {
        // Handle event flags on timer updates
        bool bOVF   = false;
        bool bCTCA  = false;
        bool bCTCB  = false;
        bool bIntr  = false;

        switch (eWGM)
        {
        case WGM_NORMAL:
        {
            DEBUG_PRINT(" Update Normal, TCNT = %d\n", TCNT0_Read());
            TCNT0_Increment();
            if (TCNT0_Read() == 0)
            {
                bOVF = true;
            }
        }
            break;
        case WGM_CTC_OCR:
        {
            DEBUG_PRINT(" Update CTC\n");
            TCNT0_Increment();
            if (TCNT0_Read() == 0)
            {
                bOVF = true;
            }
            else
            {                
                if (TCNT0_Read() == OCR0A_Read())
                {
                    DEBUG_PRINT(" CTC0A Match\n" );
                    bCTCA = true;
                    TCNT0_Clear();
                }
            }
        }
            break;
        default:
            break;
        }

        // Set interrupt flags if an appropriate transition has taken place
        if (bOVF)
        {
            DEBUG_PRINT(" TOV0 Set\n" );
            stCPU.pstRAM->stRegisters.TIFR0.TOV0 = 1;
            bIntr = true;            
        }
        if (bCTCA)
        {
            DEBUG_PRINT(" OCF0A Set\n" );
            stCPU.pstRAM->stRegisters.TIFR0.OCF0A = 1;
            bIntr = true;
        }
        if (bCTCB)
        {
            DEBUG_PRINT(" OCF0B Set\n" );
            stCPU.pstRAM->stRegisters.TIFR0.OCF0B = 1;
            bIntr = true;
        }

        if (bIntr)
        {
            Timer8_IntFlagUpdate();
        }
    }
}

//---------------------------------------------------------------------------
AVRPeripheral stTimer8 =
{
    Timer8_Init,
    Timer8_Read,
    Timer8_Write,
    Timer8_Clock,
    0,
    0x44,
    0x48
};


//---------------------------------------------------------------------------
AVRPeripheral stTimer8a =
{
    0,
    Timer8_Read,
    Timer8b_Write,
    0,
    0,
    0x35,
    0x35
};

//---------------------------------------------------------------------------
AVRPeripheral stTimer8b =
{
    0,
    Timer8_Read,
    Timer8b_Write,
    0,
    0,
    0x6E,
    0x6E
};
