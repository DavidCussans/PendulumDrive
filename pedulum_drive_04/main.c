/*
 * main.c
 *
 *  Created on: 8 Feb 2017
 *      Author: kimvw
 */
#include "msp430g2553.h"

// se volatile to indicate counter might be changed
// by interrupt routine.
volatile int counter = 0;

int main(void) {

    WDTCTL = WDTPW | WDTHOLD;       // Stop watchdog timer

    // set up clock for pendulum
    BCSCTL1 |= DIVA_0;  // DIVA_0 = ACLK/1 , DIVA_1 = ACLK/2
    BCSCTL2 |= XCAP_3; // 12.5pF load cap setting

    P1SEL = 0; // set to be I/O
    //P2SEL = 0;
    P3SEL = 0;
    P1DIR = 0xF; // set to be outputs
    //P2DIR = 0xF;
    P3DIR = 0xF;

    // set up I/O to drive pendulum.
    //P1DIR = BIT3 | BIT5;                  // Set P1.3 , P1.5 to output direction
    P1OUT = 0;
    CCTL0 = CCIE;                             // CCR0 interrupt enabled
    CCTL1 = CCIE;

    // Timer-A control
    // TACTL = TASSEL_1 + MC_1 + ID_3;           // ACLK ( TASSEL_1 ) /8 ( ID_3 ), up mode ( MC_1 )
    TACTL = TASSEL_1 + MC_1 + ID_0;           // ACLK ( TASSEL_1 ) /0 ( ID_0 ), up mode ( MC_1 )
    // 32k clock is 32768 Hz
    // So ( I think ) frequency is (32k / 8) = ACLK , /8 = counter , /500 for CCR0=500 , /2 = rise/fall = 32k/(8*8*2*500) = 0.512 Hz, 1/f= 1.953 s
    // which is what we see.
    // with DIVA_1 , ID_0
    // frequency = 32k ( ACLK ) / 10922 CCR0 , /2 = rise/fall =  1.5001 Hz
    CCR0 =  10922;
    //CCR1 = 9000;
    //CCR1 = 9830; // 0.9 of period.
    //CCR1 = 10376; // 0.95 of period
    //CCR1 = 10103; // 0.925 of period
    CCR1 = 9966; // 0.9125 of period

    _BIS_SR(CPUOFF + GIE);          // Enter LPM0 w/ interrupt

//    while(1)                          //Loop forever, we work with interrupts!
//      {}
// should never get here.
    P1DIR = BIT0;
    P1OUT = BIT0; // turn on LED if we fall off end of loop
}


// Timer A0 interrupt service routine
// #pragma vector=TIMERA0_VECTOR
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{

    // for a simple test have a counter loop 0..5
    // counter = (counter + 1) % 6 ;

    P1OUT ^= BIT3;                          // Toggle P1.
    P1DIR = BIT0 | BIT1 | BIT2 | BIT4 | BIT6 | BIT7; // BIT3 , BIT5 as inputs ( high Z ), reset as outputs
    //P1DIR = 0;
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
        P1DIR = BIT3 | BIT5;
        P1OUT ^= BIT5;                          // Toggle P1.6
        break;
    }


}
