/*
 * main.c
 *
 *  Created on: 8 Feb 2017
 *      Author: kimvw
 *
 *  Modified Feb 2020 , David Cussans. Change to P2.0 , P2.1 to suit custom driver PCB
 */
#include "msp430g2553.h"

// set volatile to indicate counter might be changed
// by interrupt routine.
volatile unsigned short counterVal = 0;

const unsigned short cycleLength = 10922 ;  // number of ticks in one cycle
const unsigned char cycleShim0 = 4; // Add an additional 1 tick delay to cycle every 4 cycles
const unsigned char cycleShim1 = 14; // add an additional  1 tick delay every 14

int main(void) {

    WDTCTL = WDTPW | WDTHOLD;       // Stop watchdog timer

    // set up clock for pendulum
    BCSCTL1 |= DIVA_0;  // DIVA_0 = ACLK/1 , DIVA_1 = ACLK/2
    BCSCTL2 |= XCAP_3; // 12.5pF load cap setting

    P1SEL = 0xFF; // disable I/O
    P2SEL = 0xFF - (BIT0 + BIT1); // set bits 0,1 to be I/O
    P3SEL = 0xFF;
    P2DIR = (BIT0 + BIT1); // XTal uses P2.6 , P2.7


    // set up I/O to drive pendulum.
    P2OUT = 0x00; // set all P2 ports to zero



    // Timer-A0 control
    // TACTL = TASSEL_1 + MC_1 + ID_3;           // ACLK ( TASSEL_1 ) /8 ( ID_3 ), up mode ( MC_1 )
    TACTL = TASSEL_1 + MC_1 + ID_0;           // ACLK ( TASSEL_1 ) /0 ( ID_0 ), up mode ( MC_1 )
    // 32k clock is 32768 Hz
    // So ( I think ) frequency is (32k / 8) = ACLK , /8 = counter , /500 for CCR0=500 , /2 = rise/fall = 32k/(8*8*2*500) = 0.512 Hz, 1/f= 1.953 s
    // which is what we see.
    // with DIVA_1 , ID_0
    // frequency = 32k ( ACLK ) / 10922 CCR0 , /2 = rise/fall =  1.5001 Hz

    // set CCR0 to get correct period
    CCR0 =  cycleLength;

    // Set CCR1 to get a big enough phase shift between two outputs
    //CCR1 = 9000;
    // CCR1 = 9284; // 0.85 of period
    CCR1 = cycleLength * 0.9 ; // 0.9 of period.
    //CCR1 = 10376; // 0.95 of period
    //CCR1 = 10103; // 0.925 of period
    // CCR1 = 9966; // 0.9125 of period

    CCTL0 = CCIE;                   // CCR0 interrupt enabled
    CCTL1 = CCIE;                  //  CCR1 interrupt enabled

    _BIS_SR(LPM3_bits + GIE);          // Enter LPM0 w/ interrupt

    // should never get here.
    P1DIR = BIT0;
    P1OUT = BIT0; // turn on LED if we fall off end of loop
}


// #pragma CODE_SECTION(cycleLengthFn,".run_from_ram")
// Pad out cycle length by a fractional amount ( i.e.
inline unsigned short cycleLengthFn ( void ) {

    unsigned short cycleLengthInt ;

    counterVal = ( counterVal + 1 ) & 0xFFF ; // cycle through 0 - 0xFFF

    cycleLengthInt = cycleLength;
  // -1/4 ,-1/16 , -1/64 ,-1/128 ,-1/1024    
    if (( counterVal & 0x0003 ) == 0 ) { cycleLengthInt += 1; } // add one if multiple of 4
    if (( counterVal & 0x000F ) == 1 ) { cycleLengthInt += 1; } // add one if multiple of 16
    if (( counterVal & 0x003F ) == 2 ) { cycleLengthInt += 1; } // add one if multiple of 64
    if (( counterVal & 0x007F ) == 3 ) { cycleLengthInt += 1; } // add one if multiple of 128
    if (( counterVal & 0x03FF ) == 4 ) { cycleLengthInt += 1; } // add one if multiple of 1024

    return cycleLengthInt;
}

// Timer A0 interrupt service routine
// #pragma vector=TIMERA0_VECTOR
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{

    P2OUT ^= BIT0;                         // Toggle P2.0
    P2DIR = 0; // set all bits of P2 as inputs

    // CCR0 = cycleLengthFn();
    CCR0 =  cycleLengthFn();
}

// Timer A1 ISR
#pragma vector=TIMER0_A1_VECTOR
__interrupt void Timer_A1 (void)
{

   switch( TAIV )
    {
    case 2:
        P2DIR = BIT0 | BIT1; // set bits 0,1 as outputs
        P2OUT ^= (BIT1);                          // Toggle P1.6

        break;
    }


}
