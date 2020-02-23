/*
 * main.c
 *
 *  Created on: 8 Feb 2017
 *      Author: kimvw
 */
#include "msp430g2553.h"

// se volatile to indicate counter might be changed
// by interrupt routine.
volatile unsigned char outval = 0;

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
    //P1DIR = BIT3 | BIT5;                  // Set P1.3 , P1.5 to output direction
    P2OUT = 0x00;



    // Timer-A0 control
    // TACTL = TASSEL_1 + MC_1 + ID_3;           // ACLK ( TASSEL_1 ) /8 ( ID_3 ), up mode ( MC_1 )
    TACTL = TASSEL_1 + MC_1 + ID_0;           // ACLK ( TASSEL_1 ) /0 ( ID_0 ), up mode ( MC_1 )
    // 32k clock is 32768 Hz
    // So ( I think ) frequency is (32k / 8) = ACLK , /8 = counter , /500 for CCR0=500 , /2 = rise/fall = 32k/(8*8*2*500) = 0.512 Hz, 1/f= 1.953 s
    // which is what we see.
    // with DIVA_1 , ID_0
    // frequency = 32k ( ACLK ) / 10922 CCR0 , /2 = rise/fall =  1.5001 Hz

    // set CCR0 to get correct period
    CCR0 =  10922;

    // Set CCR1 to get a big enough phase shift between
    //CCR1 = 9000;
    // CCR1 = 9284; // 0.85 of period
    CCR1 = 9830; // 0.9 of period.
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


// Timer A0 interrupt service routine
// #pragma vector=TIMERA0_VECTOR
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{

    P2OUT ^= BIT0;                         // Toggle P2.0
    outval = P2OUT;
    //P2DIR =  BIT0 | BIT1 ; // BIT0 , BIT1 as inputs ( high Z ), reset as outputs

}

// Timer A1 ISR
#pragma vector=TIMER0_A1_VECTOR
__interrupt void Timer_A1 (void)
{

    // for a simple test have a counter loop 0..5
    // counter = (counter + 1) % 6 ;
    switch( TAIV )
    {
    case 2:
        //P1DIR = 0xF ; // set all as outputs. Doesn't work for some reason... but does when set BIT3 | BIT5;
        //P2DIR = BIT0 | BIT1;
        P2OUT ^= (BIT1);                          // Toggle P1.6

        break;
    }


}
