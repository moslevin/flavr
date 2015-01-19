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
  \file  avr_periphregs.h

  \brief Module defining bitfield/register definitions for memory-mapped
         peripherals located within IO memory space.
*/

#ifndef __AVR_PERIPHREGS_H__
#define __AVR_PERIPHREGS_H__

#include <stdint.h>

//---------------------------------------------------------------------------
// UART/USART register struct definitions.
//---------------------------------------------------------------------------
typedef struct  __attribute__ ((__packed__))
{
    union  __attribute__ ((__packed__))
    {
        uint8_t r;
        struct  __attribute__ ((__packed__))
        {
            unsigned int MPCM0: 1;
            unsigned int U2X0 : 1;
            unsigned int UPE0 : 1;
            unsigned int DOR0 : 1;
            unsigned int FE0  : 1;
            unsigned int UDRE0 : 1;
            unsigned int TXC0 : 1;
            unsigned int RXC0 : 1;
        };
    };
} AVR_UCSR0A;

//---------------------------------------------------------------------------
typedef struct  __attribute__ ((__packed__))
{
    union  __attribute__ ((__packed__))
    {
        uint8_t r;
        struct  __attribute__ ((__packed__))
        {
            unsigned int TXB80  : 1;
            unsigned int RXB80  : 1;
            unsigned int UCSZ02 : 1;
            unsigned int TXEN0  : 1;
            unsigned int RXEN0  : 1;
            unsigned int UDRIE0 : 1;
            unsigned int TXCIE0 : 1;
            unsigned int RXCIE0 : 1;
        };
    };
} AVR_UCSR0B;

//---------------------------------------------------------------------------
typedef struct  __attribute__ ((__packed__))
{
    union  __attribute__ ((__packed__))
    {
        uint8_t r;
        struct  __attribute__ ((__packed__))
        {
            unsigned int UCPOL0  : 1;
            unsigned int UCPHA0  : 1;
            unsigned int UDORD0  : 1;
            unsigned int USBS0   : 1;
            unsigned int UPM00   : 1;
            unsigned int UPM01   : 1;
            unsigned int UMSEL00 : 1;
            unsigned int UMSEL01 : 1;
        };
    };
} AVR_UCSR0C;

//---------------------------------------------------------------------------
// TWI interface register struct definitions
//---------------------------------------------------------------------------
typedef struct  __attribute__ ((__packed__))
{
    union  __attribute__ ((__packed__))
    {
        uint8_t r;
        struct  __attribute__ ((__packed__))
        {
            unsigned int reserved : 1;
            unsigned int TWAM0 : 1;
            unsigned int TWAM1 : 1;
            unsigned int TWAM2 : 1;
            unsigned int TWAM3 : 1;
            unsigned int TWAM4 : 1;
            unsigned int TWAM5 : 1;
            unsigned int TWAM6 : 1;
        };
    };
} AVR_TWAMR;

//---------------------------------------------------------------------------
typedef struct  __attribute__ ((__packed__))
{
    union  __attribute__ ((__packed__))
    {
        uint8_t r;
        struct  __attribute__ ((__packed__))
        {
            unsigned int TWIE  : 1;
            unsigned int reserved : 1;
            unsigned int TWEN  : 1;
            unsigned int TWWC  : 1;
            unsigned int TWSTO : 1;
            unsigned int TWSTA : 1;
            unsigned int TWEA  : 1;
            unsigned int TWINT : 1;
        };
    };
} AVR_TWCR;

//---------------------------------------------------------------------------
typedef struct  __attribute__ ((__packed__))
{
    union  __attribute__ ((__packed__))
    {
        uint8_t r;
        struct  __attribute__ ((__packed__))
        {
            unsigned int TWGCE: 1;
            unsigned int TWA0 : 1;
            unsigned int TWA1 : 1;
            unsigned int TWA2 : 1;
            unsigned int TWA3 : 1;
            unsigned int TWA4 : 1;
            unsigned int TWA5 : 1;
            unsigned int TWA6 : 1;
        };
    };
} AVR_TWAR;

//---------------------------------------------------------------------------
typedef struct  __attribute__ ((__packed__))
{
    union  __attribute__ ((__packed__))
    {
        uint8_t r;
        struct  __attribute__ ((__packed__))
        {
            unsigned int TWPS0 : 1;
            unsigned int TWPS1 : 1;
            unsigned int reserved : 1;
            unsigned int TWPS3 : 1;
            unsigned int TWPS4 : 1;
            unsigned int TWPS5 : 1;
            unsigned int TWPS6 : 1;
            unsigned int TWPS7 : 1;
        };
    };
} AVR_TWSR;

//---------------------------------------------------------------------------
// Timer 2 register struct  __attribute__ ((__packed__)) definitins.
//---------------------------------------------------------------------------
typedef struct  __attribute__ ((__packed__))
{
    union  __attribute__ ((__packed__))
    {
        uint8_t r;
        struct  __attribute__ ((__packed__))
        {
            unsigned int TCR2BUB : 1;
            unsigned int TCR2AUB : 1;
            unsigned int OCR2BUB : 1;
            unsigned int OCR2AUB : 1;
            unsigned int TCN2UB  : 1;
            unsigned int AS2     : 1;
            unsigned int EXCLK   : 1;
            unsigned int reserved : 1;
        };
    };
} AVR_ASSR;

//---------------------------------------------------------------------------
typedef struct  __attribute__ ((__packed__))
{
    union  __attribute__ ((__packed__))
    {
        uint8_t r;
        struct  __attribute__ ((__packed__))
        {
            unsigned int CS20  : 1;
            unsigned int CS21  : 1;
            unsigned int CS22  : 1;
            unsigned int WGM22 : 1;
            unsigned int reserved : 2;
            unsigned int FOC2B : 1;
            unsigned int FOC2A : 1;
        };
    };
} AVR_TCCR2B;

//---------------------------------------------------------------------------
typedef struct  __attribute__ ((__packed__))
{
    union  __attribute__ ((__packed__))
    {
        uint8_t r;
        struct  __attribute__ ((__packed__))
        {
            unsigned int WGM20 : 1;
            unsigned int WGM21 : 1;
            unsigned int reserved : 2;
            unsigned int COM2B0 : 1;
            unsigned int COM2B1 : 1;
            unsigned int COM2A0 : 1;
            unsigned int COM2A1 : 1;
        };
    };
} AVR_TCCR2A;

//---------------------------------------------------------------------------
// Timer 1 Register struct  __attribute__ ((__packed__)) definitions
//---------------------------------------------------------------------------

typedef struct  __attribute__ ((__packed__))
{
    union  __attribute__ ((__packed__))
    {
        uint8_t r;
        struct  __attribute__ ((__packed__))
        {
            unsigned int WGM10 : 1;
            unsigned int WGM11 : 1;
            unsigned int reserved : 2;
            unsigned int COM1B0 : 1;
            unsigned int COM1B1 : 1;
            unsigned int COM1A0 : 1;
            unsigned int COM1A1 : 1;
        };
    };
} AVR_TCCR1A;

//---------------------------------------------------------------------------
typedef struct  __attribute__ ((__packed__))
{
    union  __attribute__ ((__packed__))
    {
        uint8_t r;
        struct  __attribute__ ((__packed__))
        {
            unsigned int CS10  : 1;
            unsigned int CS11  : 1;
            unsigned int CS12  : 1;
            unsigned int WGM12 : 1;
            unsigned int WGM13 : 1;
            unsigned int reserved : 1;
            unsigned int ICES1 : 1;
            unsigned int ICNC1 : 1;
        };
    };
} AVR_TCCR1B;

//---------------------------------------------------------------------------
typedef struct  __attribute__ ((__packed__))
{
    union  __attribute__ ((__packed__))
    {
        uint8_t r;
        struct  __attribute__ ((__packed__))
        {
            unsigned int reserved : 6;
            unsigned int FOC1B : 1;
            unsigned int FOC1A : 1;
        };
    };
} AVR_TCCR1C;

//---------------------------------------------------------------------------
// A2D converter register definitions
//---------------------------------------------------------------------------
typedef struct  __attribute__ ((__packed__))
{
    union  __attribute__ ((__packed__))
    {
        uint8_t r;
        struct  __attribute__ ((__packed__))
        {
            unsigned int AIN0D : 1;
            unsigned int AIN1D : 1;
            unsigned int reserved : 6;
        };
    };
} AVR_DIDR1;

//---------------------------------------------------------------------------
typedef struct  __attribute__ ((__packed__))
{
    union  __attribute__ ((__packed__))
    {
        uint8_t r;
        struct  __attribute__ ((__packed__))
        {
            unsigned int ADC0D : 1;
            unsigned int ADC1D : 1;
            unsigned int ADC2D : 1;
            unsigned int ADC3D : 1;
            unsigned int ADC4D : 1;
            unsigned int ADC5D : 1;
            unsigned int reserved : 2;
        };
    };
} AVR_DIDR0;

//---------------------------------------------------------------------------
typedef struct  __attribute__ ((__packed__))
{
    union  __attribute__ ((__packed__))
    {
        uint8_t r;
        struct  __attribute__ ((__packed__))
        {
            unsigned int MUX0     : 1;
            unsigned int MUX1     : 1;
            unsigned int MUX2     : 1;
            unsigned int MUX3     : 1;
            unsigned int reserved : 1;
            unsigned int ADLAR    : 1;
            unsigned int REFS0    : 1;
            unsigned int REFS1    : 1;
        };
    };
} AVR_ADMUX;

//---------------------------------------------------------------------------
typedef struct  __attribute__ ((__packed__))
{
    union  __attribute__ ((__packed__))
    {
        uint8_t r;
        struct  __attribute__ ((__packed__))
        {
            unsigned int ADPS0 : 1;
            unsigned int ADPS1 : 1;
            unsigned int ADPS2 : 1;
            unsigned int ADIE  : 1;
            unsigned int ADIF  : 1;
            unsigned int ADATE : 1;
            unsigned int ADSC  : 1;
            unsigned int ADEN  : 1;
        };
    };
} AVR_ADCSRA;

//---------------------------------------------------------------------------
typedef struct  __attribute__ ((__packed__))
{
    union  __attribute__ ((__packed__))
    {
        uint8_t r;
        struct  __attribute__ ((__packed__))
        {
            unsigned int ADTS0     : 1;
            unsigned int ADTS1     : 1;
            unsigned int ADTS2     : 1;
            unsigned int reserved  : 3;
            unsigned int ACMD      : 1;
            unsigned int reserved_ : 1;
        };
    };
} AVR_ADCSRB;

//---------------------------------------------------------------------------
// Timer interrupt mask registers.
//---------------------------------------------------------------------------
typedef struct  __attribute__ ((__packed__))
{
    union  __attribute__ ((__packed__))
    {
        uint8_t r;
        struct  __attribute__ ((__packed__))
        {
            unsigned int TOIE2    : 1;
            unsigned int OCIE2A   : 1;
            unsigned int OCIE2B   : 1;
            unsigned int reserved : 5;
        };
    };
} AVR_TIMSK2;

//---------------------------------------------------------------------------
typedef struct  __attribute__ ((__packed__))
{
    union  __attribute__ ((__packed__))
    {
        uint8_t r;
        struct  __attribute__ ((__packed__))
        {
            unsigned int TOIE1    : 1;
            unsigned int OCIE1A   : 1;
            unsigned int OCIE1B   : 1;
            unsigned int reserved : 2;
            unsigned int ICIE1    : 1;
            unsigned int reserved_ : 2;
        };
    };
} AVR_TIMSK1;

//---------------------------------------------------------------------------
typedef struct  __attribute__ ((__packed__))
{
    union  __attribute__ ((__packed__))
    {
        uint8_t r;
        struct  __attribute__ ((__packed__))
        {
            unsigned int TOIE0    : 1;
            unsigned int OCIE0A   : 1;
            unsigned int OCIE0B   : 1;
            unsigned int reserved : 5;
        };
    };
} AVR_TIMSK0;

//---------------------------------------------------------------------------
// Pin change interrupt mask bit definitions
//---------------------------------------------------------------------------
typedef struct  __attribute__ ((__packed__))
{
    union  __attribute__ ((__packed__))
    {
        uint8_t r;
        struct  __attribute__ ((__packed__))
        {
            unsigned int PCINT16 : 1;
            unsigned int PCINT17 : 1;
            unsigned int PCINT18 : 1;
            unsigned int PCINT19 : 1;
            unsigned int PCINT20 : 1;
            unsigned int PCINT21 : 1;
            unsigned int PCINT22 : 1;
            unsigned int PCINT23 : 1;
        };
    };
} AVR_PCMSK2;

//---------------------------------------------------------------------------
typedef struct  __attribute__ ((__packed__))
{
    union  __attribute__ ((__packed__))
    {
        uint8_t r;
        struct  __attribute__ ((__packed__))
        {
            unsigned int PCINT8 : 1;
            unsigned int PCINT9 : 1;
            unsigned int PCINT10 : 1;
            unsigned int PCINT11 : 1;
            unsigned int PCINT12 : 1;
            unsigned int PCINT13 : 1;
            unsigned int PCINT14 : 1;
            unsigned int PCINT15 : 1;
        };
    };
} AVR_PCMSK1;

//---------------------------------------------------------------------------
typedef struct  __attribute__ ((__packed__))
{
    union  __attribute__ ((__packed__))
    {
        uint8_t r;
        struct  __attribute__ ((__packed__))
        {
            unsigned int PCINT0 : 1;
            unsigned int PCINT1 : 1;
            unsigned int PCINT2 : 1;
            unsigned int PCINT3 : 1;
            unsigned int PCINT4 : 1;
            unsigned int PCINT5 : 1;
            unsigned int PCINT6 : 1;
            unsigned int PCINT7 : 1;
        };
    };
} AVR_PCMSK0;

//---------------------------------------------------------------------------
typedef struct  __attribute__ ((__packed__))
{
    union  __attribute__ ((__packed__))
    {
        uint8_t r;
        struct  __attribute__ ((__packed__))
        {
            unsigned int PCIE0 : 1;
            unsigned int PCIE1 : 1;
            unsigned int PCIE2 : 1;
            unsigned int reserved : 5;
        };
    };
} AVR_PCICR;

//---------------------------------------------------------------------------
typedef struct  __attribute__ ((__packed__))
{
    union  __attribute__ ((__packed__))
    {
        uint8_t r;
        struct  __attribute__ ((__packed__))
        {
            unsigned int ISC00    : 1;
            unsigned int ISC01    : 1;
            unsigned int ISC10    : 1;
            unsigned int ISC11    : 1;
            unsigned int reserved : 4;
        };
    };
} AVR_EICRA;

//---------------------------------------------------------------------------
typedef struct  __attribute__ ((__packed__))
{
    union  __attribute__ ((__packed__))
    {
        uint8_t r;
        struct  __attribute__ ((__packed__))
        {
            unsigned int PRADC : 1;
            unsigned int PRUSART0 : 1;
            unsigned int PRSPI: 1;
            unsigned int PRTIM1: 1;
            unsigned int reserved : 1;
            unsigned int PRTIM0 : 1;
            unsigned int PRTIM2 : 1;
            unsigned int PRTWI : 1;
        };
    };
} AVR_PRR;

//---------------------------------------------------------------------------
typedef struct  __attribute__ ((__packed__))
{
    union  __attribute__ ((__packed__))
    {
        uint8_t r;
        struct  __attribute__ ((__packed__))
        {
            unsigned int CLKPS0 : 1;
            unsigned int CLKPS1 : 1;
            unsigned int CLKPS2 : 1;
            unsigned int CLKPS3 : 1;
            unsigned int reserved : 3;
            unsigned int CLKPCE : 1;
        };
    };
} AVR_CLKPR;

//---------------------------------------------------------------------------
typedef struct  __attribute__ ((__packed__))
{
    union  __attribute__ ((__packed__))
    {
        uint8_t r;
        struct  __attribute__ ((__packed__))
        {
            unsigned int WDP0 : 1;
            unsigned int WDP1 : 1;
            unsigned int WDP2 : 1;
            unsigned int WDE  : 1;
            unsigned int WDCE : 1;
            unsigned int WDP3 : 1;
            unsigned int WDIE : 1;
            unsigned int WDIF : 1;
        };
    };
} AVR_WDTCSR;

//---------------------------------------------------------------------------
typedef struct  __attribute__ ((__packed__))
{
    union  __attribute__ ((__packed__))
    {
        uint8_t r;
        struct  __attribute__ ((__packed__))
        {
            unsigned int C : 1;
            unsigned int Z : 1;
            unsigned int N : 1;
            unsigned int V : 1;
            unsigned int S : 1;
            unsigned int H : 1;
            unsigned int T : 1;
            unsigned int I : 1;
        };
    };
} AVR_SREG;

//---------------------------------------------------------------------------
typedef struct  __attribute__ ((__packed__))
{
    union  __attribute__ ((__packed__))
    {
        uint8_t r;
        struct  __attribute__ ((__packed__))
        {
            unsigned int SP0 : 1;
            unsigned int SP1 : 1;
            unsigned int SP2 : 1;
            unsigned int SP3 : 1;
            unsigned int SP4 : 1;
            unsigned int SP5 : 1;
            unsigned int SP6 : 1;
            unsigned int SP7 : 1;
        };
    };
} AVR_SPL;

//---------------------------------------------------------------------------
typedef struct  __attribute__ ((__packed__))
{
    union  __attribute__ ((__packed__))
    {
        uint8_t r;
        struct  __attribute__ ((__packed__))
        {
            unsigned int SP8      : 1;
            unsigned int SP9      : 1;
            unsigned int SP10     : 1;
            unsigned int reserved : 5;
        };
    };
} AVR_SPH;

//---------------------------------------------------------------------------
typedef struct  __attribute__ ((__packed__))
{
    union  __attribute__ ((__packed__))
    {
        uint8_t r;
        struct  __attribute__ ((__packed__))
        {
            unsigned int SELFPRGEN : 1;
            unsigned int PGERS     : 1;
            unsigned int PGWRT     : 1;
            unsigned int BLBSET    : 1;
            unsigned int RWWSRE    : 1;
            unsigned int RWWSB     : 1;
            unsigned int SPMIE     : 1;
        };
    };
} AVR_SPMCSR;

//---------------------------------------------------------------------------
typedef struct  __attribute__ ((__packed__))
{
    union  __attribute__ ((__packed__))
    {
        uint8_t r;
        struct  __attribute__ ((__packed__))
        {
            unsigned int IVCE      : 1;
            unsigned int IVSEL     : 1;
            unsigned int reserved  : 2;
            unsigned int PUD       : 1;
            unsigned int BODSE     : 1;
            unsigned int BODS      : 1;
            unsigned int reserved_ : 1;
        };
    };
} AVR_MCUCR;

//---------------------------------------------------------------------------
typedef struct  __attribute__ ((__packed__))
{
    union  __attribute__ ((__packed__))
    {
        uint8_t r;
        struct  __attribute__ ((__packed__))
        {
            unsigned int PORF     : 1;
            unsigned int EXTRF    : 1;
            unsigned int BORF     : 1;
            unsigned int WDRF     : 1;
            unsigned int reserved : 4;
        };
    };
} AVR_MCUSR;

//---------------------------------------------------------------------------
typedef struct  __attribute__ ((__packed__))
{
    union  __attribute__ ((__packed__))
    {
        uint8_t r;
        struct  __attribute__ ((__packed__))
        {
            unsigned int SE       : 1;
            unsigned int SM0      : 1;
            unsigned int SM1      : 1;
            unsigned int SM2      : 1;
            unsigned int reserved : 4;
        };
    };
} AVR_SMCR;

//---------------------------------------------------------------------------
typedef struct  __attribute__ ((__packed__))
{
    union  __attribute__ ((__packed__))
    {
        uint8_t r;
        struct  __attribute__ ((__packed__))
        {
            unsigned int ACIS0 : 1;
            unsigned int ACIS1 : 1;
            unsigned int ACIC  : 1;
            unsigned int ACIE  : 1;
            unsigned int ACI   : 1;
            unsigned int AC0   : 1;
            unsigned int ACBG  : 1;
            unsigned int ACD   : 1;
        };
    };
} AVR_ACSR;

//---------------------------------------------------------------------------
typedef struct  __attribute__ ((__packed__))
{
    union  __attribute__ ((__packed__))
    {
        uint8_t r;
        struct  __attribute__ ((__packed__))
        {
            unsigned int SPR0 : 1;
            unsigned int SPR1 : 1;
            unsigned int CPHA : 1;
            unsigned int CPOL : 1;
            unsigned int MSTR : 1;
            unsigned int DORD : 1;
            unsigned int SPE  : 1;
            unsigned int SPIE : 1;
        };
    };
} AVR_SPCR;

//---------------------------------------------------------------------------
typedef struct  __attribute__ ((__packed__))
{
    union  __attribute__ ((__packed__))
    {
        uint8_t r;
        struct  __attribute__ ((__packed__))
        {
            unsigned int SPI2X    : 1;
            unsigned int reserved : 5;
            unsigned int WCOL     : 1;
            unsigned int SPIF     : 1;
        };
    };
} AVR_SPSR;

//---------------------------------------------------------------------------
typedef struct  __attribute__ ((__packed__))
{
    union  __attribute__ ((__packed__))
    {
        uint8_t r;
        struct  __attribute__ ((__packed__))
        {
            unsigned int PSRSYNC  : 1;
            unsigned int PSRASY   : 1;
            unsigned int reserved : 5;
            unsigned int TSM      : 1;
        };
    };
} AVR_GTCCR;

//---------------------------------------------------------------------------
typedef struct  __attribute__ ((__packed__))
{
    union  __attribute__ ((__packed__))
    {
        uint8_t r;
        struct  __attribute__ ((__packed__))
        {
            unsigned int WGM00    : 1;
            unsigned int WGM01    : 1;
            unsigned int reserved : 2;
            unsigned int COM0B0   : 1;
            unsigned int COM0B1   : 1;
            unsigned int COM0A0   : 1;
            unsigned int COM0A1   : 1;
        };
    };
} AVR_TCCR0A;

//---------------------------------------------------------------------------
typedef struct  __attribute__ ((__packed__))
{
    union  __attribute__ ((__packed__))
    {
        uint8_t r;
        struct  __attribute__ ((__packed__))
        {
            unsigned int CS00     : 1;
            unsigned int CS01     : 1;
            unsigned int CS02     : 1;
            unsigned int WGM02    : 1;
            unsigned int reserved : 2;
            unsigned int FOC0B    : 1;
            unsigned int FOC0A    : 1;
        };
    };
} AVR_TCCR0B;

//---------------------------------------------------------------------------
typedef struct  __attribute__ ((__packed__))
{
    union  __attribute__ ((__packed__))
    {
        uint8_t r;
        struct  __attribute__ ((__packed__))
        {
            unsigned int EERE     : 1;
            unsigned int EEPE     : 1;
            unsigned int EEMPE    : 1;
            unsigned int EERIE    : 1;
            unsigned int EEPM0    : 1;
            unsigned int EEPM1    : 1;
            unsigned int reserved : 2;
        };
    };
} AVR_EECR;

//---------------------------------------------------------------------------
// External interrupt flag register definitions
//---------------------------------------------------------------------------
typedef struct  __attribute__ ((__packed__))
{
    union  __attribute__ ((__packed__))
    {
        uint8_t r;
        struct  __attribute__ ((__packed__))
        {
            unsigned int INTF0    : 1;
            unsigned int INTF1    : 1;
            unsigned int reserved : 6;
        };
    };
} AVR_EIFR;

//---------------------------------------------------------------------------
// External interrupt mask register definitions
//---------------------------------------------------------------------------
typedef struct  __attribute__ ((__packed__))
{
    union  __attribute__ ((__packed__))
    {
        uint8_t r;
        struct  __attribute__ ((__packed__))
        {
            unsigned int INT0    : 1;
            unsigned int INT1    : 1;
            unsigned int reserved : 6;
        };
    };
} AVR_EIMSK;

//---------------------------------------------------------------------------
// Pin (GPIO) register definitions
//---------------------------------------------------------------------------
typedef struct  __attribute__ ((__packed__))
{
    union  __attribute__ ((__packed__))
    {
        uint8_t r;
        struct  __attribute__ ((__packed__))
        {
            unsigned int PIN0 : 1;
            unsigned int PIN1 : 1;
            unsigned int PIN2 : 1;
            unsigned int PIN3 : 1;
            unsigned int PIN4 : 1;
            unsigned int PIN5 : 1;
            unsigned int PIN6 : 1;
            unsigned int PIN7 : 1;
        };
    };
} AVR_PIN;

//---------------------------------------------------------------------------
// Data-direction register (GPIO) definitions
//---------------------------------------------------------------------------
typedef struct  __attribute__ ((__packed__))
{
    union  __attribute__ ((__packed__))
    {
        uint8_t r;
        struct  __attribute__ ((__packed__))
        {
            unsigned int DDR0 : 1;
            unsigned int DDR1 : 1;
            unsigned int DDR2 : 1;
            unsigned int DDR3 : 1;
            unsigned int DDR4 : 1;
            unsigned int DDR5 : 1;
            unsigned int DDR6 : 1;
            unsigned int DDR7 : 1;
        };
    };
} AVR_DDR;

//---------------------------------------------------------------------------
// Port (GPIO) register definitions
//---------------------------------------------------------------------------
typedef struct  __attribute__ ((__packed__))
{
    union  __attribute__ ((__packed__))
    {
        uint8_t r;
        struct  __attribute__ ((__packed__))
        {
            unsigned int PORT0 : 1;
            unsigned int PORT1 : 1;
            unsigned int PORT2 : 1;
            unsigned int PORT3 : 1;
            unsigned int PORT4 : 1;
            unsigned int PORT5 : 1;
            unsigned int PORT6 : 1;
            unsigned int PORT7 : 1;
        };
    };
} AVR_PORT;


//---------------------------------------------------------------------------
// Timer interrupt flag register struct  __attribute__ ((__packed__)) definitions
//---------------------------------------------------------------------------
typedef struct  __attribute__ ((__packed__))
{
    union  __attribute__ ((__packed__))
    {
        uint8_t r;
        struct  __attribute__ ((__packed__))
        {
            unsigned int TOV0     : 1;
            unsigned int OCF0A    : 1;
            unsigned int OCF0B    : 1;
            unsigned int reserved : 5;
        };
    };
} AVR_TIFR0;

//---------------------------------------------------------------------------
typedef struct  __attribute__ ((__packed__))
{
    union  __attribute__ ((__packed__))
    {
        uint8_t r;
        struct  __attribute__ ((__packed__))
        {
            unsigned int TOV1      : 1;
            unsigned int OCF1A     : 1;
            unsigned int OCF1B     : 1;
            unsigned int reserved  : 2;
            unsigned int ICF1      : 1;
            unsigned int reserved_ : 2;
        };
    };
} AVR_TIFR1;

//---------------------------------------------------------------------------
typedef struct  __attribute__ ((__packed__))
{
    union  __attribute__ ((__packed__))
    {
        uint8_t r;
        struct  __attribute__ ((__packed__))
        {
            unsigned int TOV2     : 1;
            unsigned int OCF2A    : 1;
            unsigned int OCF2B    : 1;
            unsigned int reserved : 5;
        };
    };
} AVR_TIFR2;

//---------------------------------------------------------------------------
// Pin-change interrupt flag bits
//---------------------------------------------------------------------------
typedef struct  __attribute__ ((__packed__))
{
    union  __attribute__ ((__packed__))
    {
        uint8_t r;
        struct  __attribute__ ((__packed__))
        {
            unsigned int PCIF0    : 1;
            unsigned int PCIF1    : 1;
            unsigned int PCIF2    : 1;
            unsigned int reserved : 5;
        };
    };
} AVR_PCIFR;

#endif // __AVR_PERIPHREGS_H__
