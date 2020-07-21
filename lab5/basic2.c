#include <msp430.h>
#include <intrinsics.h>
#include <time.h>
#define LEDR BIT0
#define LEDG BIT6
volatile unsigned int i = 0 , j; // i COUNTER j TIMER
volatile unsigned int f = 0; // f = 0 Red/Green LED Alternatively , f = 1 Both LEDs On/Off
volatile unsigned int f2 = 0; // f2 = 0 Green LED/Both Off ,  f2 = 1 Red LED/Both On
volatile unsigned int st = 0; // st = 0 Normal , st = 1 Measurement
volatile unsigned int adc[5];
volatile unsigned int Y;
volatile double diff_t = 0;
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
   TA1CTL = MC_1 | ID_0 | TASSEL_1 | TACLR;
   TA1CCTL0 |= CCIE;

   //ADC
   ADC10CTL0 = SREF_1 + ADC10SHT_2 + ADC10ON + REFON + ADC10IE + ENC + ADC10SC;
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
//ADC CONTROL
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void) {
    //JUDGEMENT
    if (f == 1){
        Y = 0;
        for(j = 0;j < 5;j++){
            Y += adc[j];
        }
        Y = Y / 5;
        ADC10SA = (int)adc;
        if(Y < 800){
            end = time(NULL);
            diff_t += difftime(end , begin);
            //CAHNGE TO Red/Green LED Alternatively
           f = 0; i = 0;
           TA0CCR0 = 16800 - 1;

           //ADC
           ADC10CTL0 = SREF_1 + ADC10SHT_2 + ADC10ON + REFON + ADC10IE + ENC + ADC10SC;
           ADC10CTL1 = INCH_10 + SHS_0 + CONSEQ_0 + ADC10SSEL_0;    // Input from TS

           __bic_SR_register(LPM0_bits);
           __bis_SR_register(LPM3_bits + GIE);
        }
    }else{
        Y = ADC10MEM;
        if(Y > 800){
            begin = time(NULL);
            //CAHNGE TO Both LEDs On/Off
           f = 1; i = 0;
           TA0CCR0 = 3600 - 1;

           //ADC
           ADC10CTL0 = SREF_1 + ADC10SHT_2 + ADC10ON + REFON + ADC10IE + ENC;
           ADC10CTL1 = INCH_10 + SHS_1 + CONSEQ_2;    // Input from TS

           ADC10DTC1 = 5;
           ADC10SA = (int)adc;

           __bic_SR_register(LPM3_bits);
           __bis_SR_register(LPM0_bits + GIE);
        }
    }
}

