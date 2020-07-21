#include <msp430.h>
#include <intrinsics.h>
#include <time.h>
#define LEDR BIT0
#define LEDG BIT6
volatile unsigned int i = 0; // i COUNTER j TIMER
volatile unsigned int f = 0; // f = 0 Red/Green LED Alternatively , f = 1 Both LEDs On/Off
volatile unsigned int f2 = 0; // f2 = 0 Green LED/Both Off ,  f2 = 1 Red LED/Both On
volatile unsigned int st = 0; // st = 0 Normal , st = 1 Measurement
time_t begin , end;
int main(void) {
   WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer

   P1DIR |= 0x41;        // Set LED P1.0 P1.6 as output

   //Initial LED
   P1OUT |= LEDG;

   BCSCTL1 &= ~XTS;
   BCSCTL3 |= LFXT1S_2;

   //TIMER0
   TA0CTL = MC_1 | ID_0 | TASSEL_1 | TACLR;
   TA0CCR0 = 16800 - 1;
   TA0CCTL0 |= CCIE;

   //TIMER1
   TA1CCR0 = 1200 - 1;
   TA1CCTL0 |= CCIE;
   TA1CTL = MC_1 | ID_0 | TASSEL_1 | TACLR;

   //ADC
   ADC10CTL0 = SREF_1 + ADC10SHT_2 + ADC10ON + REFON + ADC10IE;
   ADC10CTL1 = INCH_10 + SHS_0 + CONSEQ_0 + ADC10SSEL_0;    // Input from TS

   __bis_SR_register(LPM3_bits + GIE);
   __enable_interrupt();

    return 0;
}
//LED CONTROL EVERY 0.1s DRIVEN BY TIMER1
#pragma vector = TIMER1_A0_VECTOR
__interrupt void TA1_ISR(void){
    if(f == 1){
        if(i < 2){
            i++;
        }else{
            if(f2 == 0){
                //Both LEDs Off -> Both LEDs On
                f2 = 1; i = 0;
                P1OUT = 0x41;
            }else{
                //Both LEDs On -> Both LEDs Off
                f2 = 0; i = 0;
                P1OUT &= ~0x41;
            }
        }
    }else{
        if(i < 5){
            i++;
        }else{
            if(f2 == 0){
                //GREEN LED -> RED LED
                f2 = 1; i = 0;
                P1OUT = LEDR;
            }else{
                //RED LED -> GREEN LED
                f2 = 0; i = 0;
                P1OUT = LEDG;
            }
        }
    }
}
#pragma vector = TIMER0_A0_VECTOR
__interrupt void TIMER0(void){
    ADC10CTL0 |= ENC + ADC10SC;
}
//ADC CONTROL
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void) {
    volatile unsigned int Y = ADC10MEM;
    //JUDGEMENT
    if (Y < 715){
        if(f == 1){
            end = time(NULL);
            volatile double diff_t = difftime(end , begin);
            //CAHNGE TO Red/Green LED Alternatively
           f = 0; i = 0;
           TA0CCR0 = 16800 - 1;
           __bic_SR_register(LPM0_bits);
           __bis_SR_register(LPM3_bits + GIE);
        }
    }else{
        if(f == 0){
            begin = time(NULL);
            //CAHNGE TO Both LEDs On/Off
           f = 1; i = 0;
           TA0CCR0 = 3600 - 1;
           __bic_SR_register(LPM3_bits);
           __bis_SR_register(LPM0_bits + GIE);
        }
    }
//    ADC10CTL0 &= ~ENC;
}

