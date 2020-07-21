#include <msp430.h>
#define LEDR BIT0
#define LEDG BIT6
#define B1 BIT3
#define COUNTER 500000
int main(void) {
   WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer
   P1DIR |= 0x41;        // Set P1.0 P1.6 as output
   P1REN = B1;
   P1OUT = B1;
   P1OUT |= LEDR;
   volatile unsigned int i;  // prevent optimization
         volatile unsigned int f;

   for(;;) {
      P1OUT ^= 0x41;
      i = COUNTER;
      do{
          i--;
          if(f == 0){
               if((P1IN & B1) == 0){
                   f = 1;
                   P1OUT = B1;
               }
           }else{
               if((P1IN & B1) != 0){
                   f = 0;
                   P1OUT = B1;
                   P1OUT |= LEDR;
               }
         }
      }while(i != 0);

   }
   return 0;
}
