#include <msp430.h>
#define LEDR BIT0
#define LEDG BIT6
int main(void) {
   WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer
   P1DIR |= 0x41;        // Set P1.0 P1.6 as output
   P1OUT = LEDR;
   BCSCTL3 |= LFXT1S_2;
   TA0CCR0 = 2399;
   TA0CTL = MC_1 | TASSEL_1 | TACLR;


   volatile unsigned int i = 0;
   volatile unsigned int f = 1;

   for(;;) {
      while(!(TA0CTL & TAIFG)){}
      TA0CTL &= ~TAIFG;
      if(f == 0){
              f = 1;
              i = 0;
              P1OUT = LEDR;

      }else{
             if(i >= 4){
                 f = 0;
                 P1OUT = LEDG;
             }else{
                  i++;
             }
      }
   }
   return 0;
}

