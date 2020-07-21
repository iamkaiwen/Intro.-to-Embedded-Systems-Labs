#include <msp430.h>
#include <intrinsics.h>
#define LEDR BIT0
#define LEDG BIT6
volatile unsigned int i = 0;
volatile unsigned int f = 0; // f = 0 release f = 1 pressed
volatile unsigned int f2 = 0; // f2 = 0 green , off f2 = 1 red , on
int main(void) {
   WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer

   P1DIR |= 0x41;        // Set P1.0 P1.6 as output

   P1OUT |= LEDG;

   DCOCTL = CALDCO_1MHZ;
   BCSCTL1 = CALBC1_1MHZ;
   BCSCTL2 &= ~SELS;
   BCSCTL2 |= DIVS_3;
   TA0CCR0 = 15625 - 1;
   TA0CCTL0 = CCIE;
   TA0CTL = MC_1 | ID_3 | TASSEL_2 | TACLR;

   BCSCTL3 |= LFXT1S_2;
   TA1CCR0 = 2399;
   TA1CCTL0 = CCIE;
   TA1CCTL0 &= ~CCIFG;
   TA1CTL = MC_1 | TASSEL_1 | TACLR;

   ADC10CTL0 = SREF_1 + ADC10SHT_3 + ADC10ON+ + REFON;
   ADC10CTL1 = INCH_10 + SHS_0 + ADC10DIV_0 + ADC10SSEL_2;    // Input from TS

   __enable_interrupt();

   for(;;) {}
   return 0;
}
#pragma vector = TIMER1_A0_VECTOR
__interrupt void TA1_ISR(void){
    if(f == 1){
        if(f2 == 0){
            f2 = 1; i = 0;
             P1OUT = 0x41;
        }else{
            if(i >= 4){
                f2 = 0; i = 0;
                P1OUT = 0;
            }else{
                i++;
            }
        }
    }else{
        if(f2 == 0){
              f2 = 1; i = 0;
              P1OUT = LEDR;
        }else{
             if(i >= 4){
                 f2 = 0; i = 0;
                 P1OUT = LEDG;
             }else{
                  i++;
             }
        }
    }
    TA1CCTL0 &= ~CCIFG;
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void TA0_ISR(void){
    ADC10CTL0 |= ENC + ADC10SC + ADC10IE; // Start sampling
}

#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void) {
    volatile unsigned int X = ADC10MEM;
  if (X < 732){
      f = 0; f2 = 0; i = 0;
      P1OUT = LEDG;
  }else{
      f = 1; f2 = 0; i = 0;
      P1OUT = 0;
  }
//  TA1CCTL0 &= ~CCIFG;
  TA1CTL |= TACLR;
}


