#include <msp430.h>
#include <intrinsics.h>
#define LEDR BIT0
#define LEDG BIT6
#define B1 BIT3
volatile unsigned int i = 0 , j;
volatile unsigned int f = 0; // f = 0 release f = 1 pressed
volatile unsigned int f2 = 0; // f2 = 0 green , off f2 = 1 red , on
volatile unsigned int st = 0; // state = 0 normal state = 1 measure
volatile unsigned int adc[5];
volatile unsigned int Y;
int main(void) {
   WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer

   P1DIR |= 0x41;        // Set P1.0 P1.6 as output

   P1REN = B1;
   P1OUT = B1;
   P1IE |= B1;
   P1IES |= B1;
   P1IFG &= ~B1;
   P1OUT |= LEDG;

   DCOCTL = CALDCO_1MHZ;
   BCSCTL1 = CALBC1_1MHZ;
   BCSCTL2 &= ~SELS;
   BCSCTL2 |= DIVS_3;
   TA0CCR0 = 3125 - 1;
   TA0CTL = MC_1 | ID_3 | TASSEL_2 | TACLR;

   BCSCTL3 |= LFXT1S_2;
   TA1CCR0 = 2399;
   TA1CCTL0 = CCIE;
//   TA1CCTL0 &= ~CCIFG;
   TA1CTL = MC_1 | TASSEL_1 | TACLR;

   ADC10CTL0 = SREF_1 + ADC10SHT_2 + ADC10ON + REFON + ADC10IE;
   ADC10CTL1 = INCH_10 + SHS_1 + CONSEQ_2;    // Input from TS

   ADC10DTC1 = 5;
   ADC10SA = (int)adc;

   __enable_interrupt();

   for(;;) {}
   return 0;
}
#pragma vector = TIMER1_A0_VECTOR
__interrupt void TA1_ISR(void){
    if(f == 1){
        if(f2 == 0){
            f2 = 1; i = 0;
             P1OUT = B1 | 0x41;
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
              P1OUT = B1 | LEDR;
        }else{
             if(i >= 4){
                 f2 = 0; i = 0;
                 P1OUT = B1 | LEDG;
             }else{
                  i++;
             }
        }
    }
}


#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void) {
    Y = 0;
    for(j = 0;j < 5;j++){
        Y += adc[j];
    }
    Y = Y / 5;
    volatile unsigned int Voltage = Y * 1.5 /1023;
    volatile unsigned int Celsius = (Voltage - 0.986)/0.00355;
    if (Y < 748){
        f = 0; f2 = 0; i = 0;
        P1OUT = B1 | LEDG;
    }else{
        f = 1; f2 = 0; i = 0;
        P1OUT = B1;
     }

    TA1CTL |= TACLR;

    ADC10SA = (int)adc;
}

#pragma vector = PORT1_VECTOR
__interrupt void Port_1(void){
    if(st == 0){
        st = 1;
        ADC10CTL0 |= ENC;
    }else{
        f = 0; f2 = 0; i = 0;
        P1OUT = B1 | LEDG;
        st = 0;
        ADC10CTL0 &= ~ENC;
    }
    P1IFG &= ~B1;
}

