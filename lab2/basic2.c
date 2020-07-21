#include <msp430.h>
#define LEDR BIT0
#define LEDG BIT6
#define B1 BIT3
int main(void) {
   WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer

   P1DIR |= 0x41;        // Set P1.0 P1.6 as output
   P1REN = B1;
   P1OUT = B1;
   P1OUT |= LEDG;

   BCSCTL3 |= LFXT1S_2;
   TA0CCR0 = 2399;
   TA0CTL = MC_1 | TASSEL_1 | TACLR;


   volatile unsigned int i = 0;
   volatile unsigned int f = 0; // f = 0 release f = 1 pressed
   volatile unsigned int f2 = 0; // f2 = 0 green , off f2 = 1 red , on

   for(;;) {
      while(!(TA0CTL & TAIFG)){}
      TA0CTL &= ~TAIFG;
        if(f == 1){
            if((P1IN & B1) != 0){
                f = 0; f2 = 0; i = 0;
                P1OUT = B1; P1OUT |= LEDG;
            }else if(f2 == 0){
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
            if((P1IN & B1) == 0){
                f = 1; f2 = 0; i = 0;
                P1OUT = B1;
            }else if(f2 == 0){
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
   return 0;
}

