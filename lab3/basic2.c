#include <msp430.h>
#include <intrinsics.h>
#define LEDR BIT0
#define LEDG BIT6
#define B1 BIT3
volatile unsigned int i = 0;
volatile unsigned int f = 0; // f = 0 release f = 1 pressed
volatile unsigned int f2 = 0; // f2 = 0 green , off f2 = 1 red , on
int main(void) {
   WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer

   P1DIR |= 0x41;        // Set P1.0 P1.6 as output
   P1REN = B1;
   P1OUT = B1;
   P1IE |= B1;
   P1IES |= B1;
   P1IFG &= ~B1;

   P1OUT |= LEDG;

   BCSCTL1 |= CALBC1_12MHZ;
   BCSCTL3 |= LFXT1S_2;
   TA0CCR0 = 2399;
   TA0CCTL0 = CCIE;
   TA0CTL = MC_1 | TASSEL_1 | TACLR;

   DCOCTL = CALDCO_1MHZ;
   BCSCTL1 = CALBC1_1MHZ;
   BCSCTL2 &= ~SELS;
   BCSCTL2 |= DIVS_3;
   TA1CCR0 = 31249;
   TA1CTL = MC_1 | ID_3 | TASSEL_2 | TACLR;

   __enable_interrupt();

   for(;;) {}
   return 0;
}
#pragma vector = TIMER0_A0_VECTOR
__interrupt void TA0_ISR(void){
    if(f == 1){
        if(f2 == 0){
            f2 = 1; i = 0;
            P1OUT = B1; P1OUT |= 0x41;
        }else{
            if(i >= 4){
                f2 = 0; i = 0;
                P1OUT = B1;
            }else{
                i++;
            }
        }
    }else{
        if(f2 == 0){
              f2 = 1; i = 0;
              P1OUT = B1; P1OUT |= LEDR;
        }else{
             if(i >= 4){
                 f2 = 0; i = 0;
                 P1OUT = B1; P1OUT |= LEDG;
             }else{
                  i++;
             }
        }
    }
}
#pragma vector = TIMER1_A0_VECTOR
__interrupt void TA1_ISR(void){
    if(f == 1){
        f = 0; f2 = 0; i = 0;
        P1OUT = B1; P1OUT |= LEDG;
    }else{
        f = 1; f2 = 0; i = 0;
        P1OUT = B1;
    }
    TA1CCTL0 &= ~CCIE;
}
#pragma vector = PORT1_VECTOR
__interrupt void Port_1(void){
    TA1CCTL0 = CCIE;
    TA1CTL |= TACLR;
    P1IFG &= ~B1;
}

