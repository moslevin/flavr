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
  \file  mega_timer16.c

  \brief ATMega 16-bit timer implementation.
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
    WGM_PWM_PC_8BIT,
    WGM_PWM_PC_9BIT,
    WGM_PWM_PC_10BIT,
    WGM_CTC_OCR,
    WGM_PWM_8BIT,
    WGM_PWM_9BIT,
    WGM_PWM_10BIT,
    WGM_PWM_PC_FC_ICR,
    WGM_PWM_PC_FC_OCR,
    WGM_PWM_PC_ICR,
    WGM_PWM_PC_OCR,
    WGM_CTC_ICR,
    WGM_RESERVED,
    WGM_FAST_PWM_ICR,
    WGM_FAST_PWM_OCR
} WaveformGeneratorMode_t;

//---------------------------------------------------------------------------
typedef enum
{
    COM_NORMAL,         // OCA1/B disconnected
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
static void TCNT1_Increment()
{
    uint16_t u16NewVal = 0;

    u16NewVal = (stCPU.pstRAM->stRegisters.TCNT1H << 8 ) |
                 stCPU.pstRAM->stRegisters.TCNT1L;

    u16NewVal++;
    stCPU.pstRAM->stRegisters.TCNT1L = (u16NewVal & 0x00FF);
    stCPU.pstRAM->stRegisters.TCNT1H = (u16NewVal >> 8);
}

//---------------------------------------------------------------------------
static uint16_t TCNT1_Read()
{
    uint16_t u16Ret = 0;

    u16Ret = (stCPU.pstRAM->stRegisters.TCNT1H << 8 ) |
              stCPU.pstRAM->stRegisters.TCNT1L;
    return u16Ret;
}

//---------------------------------------------------------------------------
static void TCNT1_Clear()
{
    stCPU.pstRAM->stRegisters.TCNT1H = 0;
    stCPU.pstRAM->stRegisters.TCNT1L = 0;
}

//---------------------------------------------------------------------------
static uint16_t OCR1A_Read()
{
    uint16_t u16Ret = 0;

    u16Ret = (stCPU.pstRAM->stRegisters.OCR1AH << 8 ) |
              stCPU.pstRAM->stRegisters.OCR1AL;
    return u16Ret;
}

//---------------------------------------------------------------------------
static uint16_t OCR1B_Read()
{
    uint16_t u16Ret = 0;

    u16Ret = (stCPU.pstRAM->stRegisters.OCR1BH << 8 ) |
              stCPU.pstRAM->stRegisters.OCR1BL;
    return u16Ret;
}

//---------------------------------------------------------------------------
static uint16_t ICR1_Read()
{
    uint16_t u16Ret = 0;

    u16Ret = (stCPU.pstRAM->stRegisters.ICR1H << 8 ) |
              stCPU.pstRAM->stRegisters.ICR1L;
    return u16Ret;
}

//---------------------------------------------------------------------------
static bool Timer16_Is_TOIE1_Enabled()
{
    return (stCPU.pstRAM->stRegisters.TIMSK1.TOIE1 == 1);
}

//---------------------------------------------------------------------------
static bool Timer16_Is_OCIE1A_Enabled()
{
    return (stCPU.pstRAM->stRegisters.TIMSK1.OCIE1A == 1);
}

//---------------------------------------------------------------------------
static bool Timer16_Is_OCIE1B_Enabled()
{
    return (stCPU.pstRAM->stRegisters.TIMSK1.OCIE1B == 1);
}

//---------------------------------------------------------------------------
static bool Timer16_Is_ICIE1_Enabled()
{
    return (stCPU.pstRAM->stRegisters.TIMSK1.ICIE1 == 1);
}

//---------------------------------------------------------------------------
static void OV1_Ack(  uint8_t ucVector_)
{
    stCPU.pstRAM->stRegisters.TIFR1.TOV1 = 0;
}

//---------------------------------------------------------------------------
static void IC1_Ack(  uint8_t ucVector_)
{
    stCPU.pstRAM->stRegisters.TIFR1.ICF1 = 0;
}

//---------------------------------------------------------------------------
static void COMP1A_Ack(  uint8_t ucVector_)
{
    static uint64_t lastcycles = 0;
   // printf("COMP1A - Ack'd: %d delta\n", stCPU.u64CycleCount - lastcycles);
    lastcycles = stCPU.u64CycleCount;

    stCPU.pstRAM->stRegisters.TIFR1.OCF1A = 0;
}

//---------------------------------------------------------------------------
static void COMP1B_Ack(  uint8_t ucVector_)
{
    stCPU.pstRAM->stRegisters.TIFR1.OCF1B = 0;
}

//---------------------------------------------------------------------------
static void Timer16_Init(void *context_ )
{
    DEBUG_PRINT(stderr, "Timer16 Init\n");    

    CPU_RegisterInterruptCallback( OV1_Ack, 0x0D);
    CPU_RegisterInterruptCallback( IC1_Ack, 0x0A);
    CPU_RegisterInterruptCallback( COMP1A_Ack, 0x0B);
    CPU_RegisterInterruptCallback( COMP1B_Ack, 0x0C);
}

//---------------------------------------------------------------------------
static void Timer16_Read(void *context_, uint8_t ucAddr_, uint8_t *pucValue_ )
{
    DEBUG_PRINT(stderr, "Timer16 Read: 0x%02x\n", ucAddr_);
    *pucValue_ = stCPU.pstRAM->au8RAM[ ucAddr_ ];
}

//---------------------------------------------------------------------------
static void TCCR1A_Write( uint8_t ucAddr_, uint8_t ucValue_)
{
    // Update the waveform generator mode (WGM11:10) bits.
    uint8_t u8WGMBits = ucValue_ & 0x03; // WGM11 and 10 are in bits 0,1
    uint8_t u8WGMTemp = (uint8_t)eWGM;
    u8WGMTemp &= ~(0x03);
    u8WGMTemp |= u8WGMBits;
    eWGM = (WaveformGeneratorMode_t)u8WGMTemp;

    // Update the memory-mapped register.
    stCPU.pstRAM->stRegisters.TCCR1A.r = ucValue_ & 0xF3;
}

//---------------------------------------------------------------------------
static void TCCR1B_Write( uint8_t ucAddr_, uint8_t ucValue_)
{
    // Update the waveform generator mode (WGM13:12) bits.
    uint8_t u8WGMBits = (ucValue_ >> 1) & 0x0C; // WGM13 and 12 are in register bits 3,4
    uint8_t u8WGMTemp = (uint8_t)eWGM;
    u8WGMTemp &= ~(0x0C);
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

    // Update the memory-mapped register.
    stCPU.pstRAM->stRegisters.TCCR1B.r = ucValue_ & 0xDF; // Bit 5 is read-only
}

//---------------------------------------------------------------------------
static void TCCR1C_Write( uint8_t ucAddr_, uint8_t ucValue_)
{
    stCPU.pstRAM->stRegisters.TCCR1C.r = ucValue_;
}

//---------------------------------------------------------------------------
static void TCNT1L_Write( uint8_t ucAddr_, uint8_t ucValue_)
{
    // Writing the low-word forces the high-word to be stored from the internal
    // temp register... which is why the high byte must be written first.
    stCPU.pstRAM->stRegisters.TCNT1L = ucValue_;
    stCPU.pstRAM->stRegisters.TCNT1H = u8Temp;
}
//---------------------------------------------------------------------------
static void TCNT1H_Write( uint8_t ucAddr_, uint8_t ucValue_)
{
    u8Temp = ucValue_;
}
//---------------------------------------------------------------------------
static void ICR1L_Write( uint8_t ucAddr_, uint8_t ucValue_)
{
    // Writing the low-word forces the high-word to be stored from the internal
    // temp register... which is why the high byte must be written first.
    stCPU.pstRAM->stRegisters.ICR1L = ucValue_;
    stCPU.pstRAM->stRegisters.ICR1H = u8Temp;
}
//---------------------------------------------------------------------------
static void ICR1H_Write( uint8_t ucAddr_, uint8_t ucValue_)
{
    u8Temp = ucValue_;
}

//---------------------------------------------------------------------------
static void OCR1AL_Write( uint8_t ucAddr_, uint8_t ucValue_)
{
    // Writing the low-word forces the high-word to be stored from the internal
    // temp register... which is why the high byte must be written first.
    stCPU.pstRAM->stRegisters.OCR1AL = ucValue_;
    stCPU.pstRAM->stRegisters.OCR1AH = u8Temp;
}

//---------------------------------------------------------------------------
static void OCR1AH_Write( uint8_t ucAddr_, uint8_t ucValue_)
{
    u8Temp = ucValue_;
}

//---------------------------------------------------------------------------
static void OCR1BL_Write( uint8_t ucAddr_, uint8_t ucValue_)
{
    // Writing the low-word forces the high-word to be stored from the internal
    // temp register... which is why the high byte must be written first.
    stCPU.pstRAM->stRegisters.OCR1BL = ucValue_;
    stCPU.pstRAM->stRegisters.OCR1BH = u8Temp;
}

//---------------------------------------------------------------------------
static void OCR1BH_Write( uint8_t ucAddr_, uint8_t ucValue_)
{
    u8Temp = ucValue_;
}

//---------------------------------------------------------------------------
static void Timer16_IntFlagUpdate(void)
{
    if (stCPU.pstRAM->stRegisters.TIMSK1.TOIE1 == 1)
    {
        if (stCPU.pstRAM->stRegisters.TIFR1.TOV1 == 1)
        {
            DEBUG_PRINT(" TOV1 Interrupt Candidate\n" );
            AVR_InterruptCandidate(0x0D);
        }
        else
        {
            AVR_ClearCandidate(0x0D);
        }
    }

    if (stCPU.pstRAM->stRegisters.TIMSK1.OCIE1A == 1)
    {
        if (stCPU.pstRAM->stRegisters.TIFR1.OCF1A == 1)
        {
            DEBUG_PRINT(" OCF1A Interrupt Candidate\n" );
            AVR_InterruptCandidate(0x0B);
        }
        else
        {
            AVR_ClearCandidate(0x0B);
        }
    }

    if (stCPU.pstRAM->stRegisters.TIMSK1.OCIE1B == 1)
    {
        if (stCPU.pstRAM->stRegisters.TIFR1.OCF1B == 1)
        {
            DEBUG_PRINT(" OCF1B Interrupt Candidate\n" );
            AVR_InterruptCandidate(0x0C);
        }
        else
        {
            AVR_ClearCandidate(0x0C);
        }
    }

    if (stCPU.pstRAM->stRegisters.TIMSK1.ICIE1 == 1)
    {
        if (stCPU.pstRAM->stRegisters.TIFR1.ICF1 == 1)
        {
            DEBUG_PRINT(" ICF1 Interrupt Candidate\n" );
            AVR_InterruptCandidate(0x0A);
        }
        else
        {
            AVR_ClearCandidate(0x0A);
        }
    }
}

//---------------------------------------------------------------------------
// TIFR & TMSK
static void Timer16b_Write(void *context_, uint8_t ucAddr_, uint8_t ucValue_ )
{
    stCPU.pstRAM->au8RAM[ucAddr_] = ucValue_;
    Timer16_IntFlagUpdate();
}

//---------------------------------------------------------------------------
static void Timer16_Write(void *context_, uint8_t ucAddr_, uint8_t ucValue_ )
{
    switch (ucAddr_)
    {
    case 0x80:  //TCCR1A
        TCCR1A_Write(ucAddr_, ucValue_);
        break;
    case 0x81:  //TCCR1B
        TCCR1B_Write(ucAddr_, ucValue_);
        break;
    case 0x82:  //TCCR1C
        TCCR1C_Write(ucAddr_, ucValue_);
        break;
    case 0x83:  // Reserved
        break;
    case 0x84:  // TCNT1L
        TCNT1L_Write(ucAddr_, ucValue_);
        break;
    case 0x85:  // TCNT1H
        TCNT1H_Write(ucAddr_, ucValue_);
        break;
    case 0x86:  // ICR1L
        ICR1L_Write(ucAddr_, ucValue_);
        break;
    case 0x87:  // ICR1H
        ICR1H_Write(ucAddr_, ucValue_);
        break;
    case 0x88:  // OCR1AL
        OCR1AL_Write(ucAddr_, ucValue_);
        break;
    case 0x89:  // OCR1AH
        OCR1AH_Write(ucAddr_, ucValue_);
        break;
    case 0x8A:  // OCR1BL
        OCR1BL_Write(ucAddr_, ucValue_);
        break;
    case 0x8B:  // OCR1BH
        OCR1BH_Write(ucAddr_, ucValue_);
        break;
    default:
        break;
    }
}

//---------------------------------------------------------------------------
static void Timer16_Clock(void *context_ )
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
            //DEBUG_PRINT(" expire and reset\n");
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
        bool bICR   = false;
        bool bIntr  = false;

        //DEBUG_PRINT( " WGM Mode %d\n", eWGM );
        switch (eWGM)
        {
        case WGM_NORMAL:
        {
            DEBUG_PRINT(" Update Normal\n");
            TCNT1_Increment();
            if (TCNT1_Read() == 0)
            {
                bOVF = true;
            }
        }
            break;
        case WGM_CTC_OCR:
        {
            DEBUG_PRINT(" Update CTC\n");
            TCNT1_Increment();
            if (TCNT1_Read() == 0)
            {
                bOVF = true;
            }
            else
            {
                bool bClearTCNT1 = false;
                if (TCNT1_Read() == OCR1A_Read())
                {
                    DEBUG_PRINT(" CTC1A Match\n" );
                    bCTCA = true;
                    bClearTCNT1 = true;
                }
                if (TCNT1_Read() == ICR1_Read())
                {
                    DEBUG_PRINT(" ICR1 Match\n" );
                    bICR = true;
                    bClearTCNT1 = true;
                }
                if (bClearTCNT1)
                {
                    TCNT1_Clear();
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
            DEBUG_PRINT(" TOV1 Set\n" );
            stCPU.pstRAM->stRegisters.TIFR1.TOV1 = 1;
            bIntr = true;            
        }
        if (bCTCA)
        {
            DEBUG_PRINT(" OCF1A Set\n" );
            stCPU.pstRAM->stRegisters.TIFR1.OCF1A = 1;
            bIntr = true;
        }
        if (bCTCB)
        {
            DEBUG_PRINT(" OCF1B Set\n" );
            stCPU.pstRAM->stRegisters.TIFR1.OCF1B = 1;
            bIntr = true;
        }
        if (bICR)
        {
            DEBUG_PRINT(" ICF1 Set\n" );
            stCPU.pstRAM->stRegisters.TIFR1.ICF1 = 1;
            bIntr = true;
        }

        if (bIntr)
        {
            Timer16_IntFlagUpdate();
        }
    }
}

//---------------------------------------------------------------------------
AVRPeripheral stTimer16 =
{
    Timer16_Init,
    Timer16_Read,
    Timer16_Write,
    Timer16_Clock,
    0,
    0x80,
    0x8B
};

//---------------------------------------------------------------------------
AVRPeripheral stTimer16a =
{
    0,
    Timer16_Read,
    Timer16b_Write,
    0,
    0,
    0x36,
    0x36
};

//---------------------------------------------------------------------------
AVRPeripheral stTimer16b =
{
    0,
    Timer16_Read,
    Timer16b_Write,
    0,
    0,
    0x6F,
    0x6F
};
