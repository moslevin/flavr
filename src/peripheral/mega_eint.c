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
  \file  mega_eint.c

  \brief ATMega External Interrupt Implementation
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
typedef enum
{
    INT_SENSE_LOW = 0,  //!< Logic low triggers interrupt
    INT_SENSE_CHANGE,   //!< Change in state triggers interrupt
    INT_SENSE_FALL,     //!< Falling edge triggers interrupt
    INT_SENSE_RISE      //!< Rising edge triggers interrupt
} InterruptSense_t;


//---------------------------------------------------------------------------
static InterruptSense_t eINT0Sense;
static InterruptSense_t eINT1Sense;
static uint8_t ucLastINT0;
static uint8_t ucLastINT1;

//---------------------------------------------------------------------------
static void EINT_AckInt(  uint8_t ucVector_);

//---------------------------------------------------------------------------
static void EINT_Init(void *context_ )
{
    eINT0Sense = INT_SENSE_LOW;
    eINT1Sense = INT_SENSE_LOW;
    ucLastINT0 = 0;
    ucLastINT1 = 0;

    // Register interrupt callback functions
    CPU_RegisterInterruptCallback(EINT_AckInt, 0x01);
    CPU_RegisterInterruptCallback(EINT_AckInt, 0x02);
}

//---------------------------------------------------------------------------
static void EINT_Read(void *context_, uint8_t ucAddr_, uint8_t *pucValue_ )
{
    *pucValue_ = stCPU.pstRAM->au8RAM[ucAddr_];
}

//---------------------------------------------------------------------------
static void EICRA_Write( uint8_t ucValue_ )
{
    DEBUG_PRINT("EICRA Clock\n");
    ucValue_ &= 0x0F;    // Only the bottom 2 bits are valid.
    stCPU.pstRAM->stRegisters.EICRA.r = ucValue_;

    // Change local interrupt sense value.
    if ((stCPU.pstRAM->stRegisters.EICRA.ISC00 == 0) &&
        (stCPU.pstRAM->stRegisters.EICRA.ISC01 == 0))
    {
        DEBUG_PRINT("I0-low\n");
        eINT0Sense = INT_SENSE_LOW;
    }
    else if ((stCPU.pstRAM->stRegisters.EICRA.ISC00 == 1) &&
             (stCPU.pstRAM->stRegisters.EICRA.ISC01 == 0))
    {
        DEBUG_PRINT("I0-change\n");
        eINT0Sense = INT_SENSE_CHANGE;
    }
    else if ((stCPU.pstRAM->stRegisters.EICRA.ISC00 == 0) &&
             (stCPU.pstRAM->stRegisters.EICRA.ISC01 == 1))
    {
        DEBUG_PRINT("I0-fall\n");
        eINT0Sense = INT_SENSE_FALL;
    }
    else if ((stCPU.pstRAM->stRegisters.EICRA.ISC00 == 1) &&
             (stCPU.pstRAM->stRegisters.EICRA.ISC01 == 1))
    {
        DEBUG_PRINT("I0-risel\n");
        eINT0Sense = INT_SENSE_RISE;
    }

    if ((stCPU.pstRAM->stRegisters.EICRA.ISC10 == 0) &&
        (stCPU.pstRAM->stRegisters.EICRA.ISC11 == 0))
    {
        eINT1Sense = INT_SENSE_LOW;
    }
    else if ((stCPU.pstRAM->stRegisters.EICRA.ISC10 == 1) &&
             (stCPU.pstRAM->stRegisters.EICRA.ISC11 == 0))
    {
        eINT1Sense = INT_SENSE_CHANGE;
    }
    else if ((stCPU.pstRAM->stRegisters.EICRA.ISC10 == 0) &&
             (stCPU.pstRAM->stRegisters.EICRA.ISC11 == 1))
    {
        eINT1Sense = INT_SENSE_RISE;
    }
    else if ((stCPU.pstRAM->stRegisters.EICRA.ISC10 == 1) &&
             (stCPU.pstRAM->stRegisters.EICRA.ISC11 == 1))
    {
        eINT1Sense = INT_SENSE_FALL;
    }
    DEBUG_PRINT ("IntSense0,1: %d, %d\n", eINT0Sense, eINT1Sense);
    DEBUG_PRINT ("EICRA: %d, ISC00 : %d, ISC01 : %d, ISC10: %d, ISC11: %d\n",
                stCPU.pstRAM->stRegisters.EICRA.r,
                stCPU.pstRAM->stRegisters.EICRA.ISC00,
                stCPU.pstRAM->stRegisters.EICRA.ISC01,
                stCPU.pstRAM->stRegisters.EICRA.ISC10,
                stCPU.pstRAM->stRegisters.EICRA.ISC11
            );
}

//---------------------------------------------------------------------------
static void EIFR_Write( uint8_t ucValue_ )
{
    DEBUG_PRINT("EIFR Clock\n");
    ucValue_ &= 0x03;   // Only the bottom-2 bits are set
    stCPU.pstRAM->stRegisters.EIFR.r = ucValue_;
}

//---------------------------------------------------------------------------
static void EIMSK_Write( uint8_t ucValue_ )
{
    DEBUG_PRINT("EIMSK Write\n");
    ucValue_ &= 0x03;   // Only the bottom-2 bits are set
    stCPU.pstRAM->stRegisters.EIMSK.r = ucValue_;    
}

//---------------------------------------------------------------------------
static void EINT_Write(void *context_, uint8_t ucAddr_, uint8_t ucValue_ )
{
    DEBUG_PRINT("EINT Write\n");
    switch (ucAddr_)
    {
    case 0x69:  // EICRA        
        EICRA_Write(ucValue_);
        break;
    case 0x3C:  // EIFR        
        EIFR_Write(ucValue_);
        break;
    case 0x3D:  // EIMSK        
        EIMSK_Write(ucValue_);
        break;
    default:
        break;
    }
}

//---------------------------------------------------------------------------
static void EINT_Clock(void *context_ )
{
    // Check to see if interrupts are enabled.  If so, check to see if the
    // interrupt mask is set, and then finally - whether or not an interupt
    // condition has occurred based on the interrupt sense mode.
    bool bSetINT0 = false;
    bool bSetINT1 = false;

    //!! ToDo - Consider adding support for external stimulus (which would
    //!! Invoke inputs on PIND as opposed to PORTD)...  This will only work
    //!! as software interrupts in its current state    

    if (stCPU.pstRAM->stRegisters.EIMSK.INT0 == 1)
    {
        switch (eINT0Sense)
        {
        case INT_SENSE_LOW:
            if (stCPU.pstRAM->stRegisters.PORTD.PORT2 == 0)
            {
                DEBUG_PRINT(" SET INT0\n");
                bSetINT0 = true;
            }
            break;
        case INT_SENSE_CHANGE:
            if (stCPU.pstRAM->stRegisters.PORTD.PORT2 != ucLastINT0)
            {
                DEBUG_PRINT(" SET INT0\n");
                bSetINT0 = true;
            }
            break;
        case INT_SENSE_FALL:
            if ((stCPU.pstRAM->stRegisters.PORTD.PORT2 == 0) && (ucLastINT0 == 1))
            {
                DEBUG_PRINT(" SET INT0\n");
                bSetINT0 = true;
            }
            break;
        case INT_SENSE_RISE:
            if ((stCPU.pstRAM->stRegisters.PORTD.PORT2 == 1) && (ucLastINT0 == 0))
            {
                DEBUG_PRINT(" SET INT0\n");
                bSetINT0 = true;
            }
            break;
        }
    }
    if (stCPU.pstRAM->stRegisters.EIMSK.INT1 == 1)
    {
        switch (eINT0Sense)
        {
        case INT_SENSE_LOW:
            if (stCPU.pstRAM->stRegisters.PORTD.PORT3 == 0)
            {
                bSetINT1 = true;
            }
            break;
        case INT_SENSE_CHANGE:
            if (stCPU.pstRAM->stRegisters.PORTD.PORT3 != ucLastINT1)
            {
                bSetINT1 = true;
            }
            break;
        case INT_SENSE_FALL:
            if ((stCPU.pstRAM->stRegisters.PORTD.PORT3 == 0) && (ucLastINT1 == 1))
            {
                bSetINT1 = true;
            }
            break;
        case INT_SENSE_RISE:
            if ((stCPU.pstRAM->stRegisters.PORTD.PORT3 == 1) && (ucLastINT1 == 0))
            {
                bSetINT1 = true;
            }
            break;
        }
    }

    // Trigger interrupts where necessary
    if (bSetINT0)
    {
        stCPU.pstRAM->stRegisters.EIFR.INTF0 = 1;
        AVR_InterruptCandidate(0x01);
    }
    if (bSetINT1)
    {
        stCPU.pstRAM->stRegisters.EIFR.INTF1 = 1;
        AVR_InterruptCandidate(0x02);

    }
    // Update locally-cached copy of previous INT0/INT1 pin status.
    ucLastINT0 = stCPU.pstRAM->stRegisters.PORTD.PORT2;
    ucLastINT1 = stCPU.pstRAM->stRegisters.PORTD.PORT3;
}

//---------------------------------------------------------------------------
static void EINT_AckInt(  uint8_t ucVector_)
{
    // We automatically clear the INTx flag as soon as the interrupt
    // is acknowledged.
    switch (ucVector_)
    {
    case 0x01:
        DEBUG_PRINT("INT0!\n");
        stCPU.pstRAM->stRegisters.EIFR.INTF0 = 0;
        break;
    case 0x02:
        DEBUG_PRINT("INT1!\n");
        stCPU.pstRAM->stRegisters.EIFR.INTF1 = 0;
        break;
    }
}

//---------------------------------------------------------------------------
AVRPeripheral stEINT_a =
{
    EINT_Init,
    EINT_Read,
    EINT_Write,
    EINT_Clock,
    NULL,
    0x69,
    0x69
};

//---------------------------------------------------------------------------
AVRPeripheral stEINT_b =
{
    NULL,
    EINT_Read,
    EINT_Write,
    NULL,
    NULL,
    0x3C,
    0x3D
};
