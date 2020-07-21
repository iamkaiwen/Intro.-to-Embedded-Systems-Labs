#include <msp430.h>
volatile unsigned int x , y;
int main(void) {
   WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer
   P1DIR |= 0x41;        // Set P1.0 P1.6 as output
   P1OUT = 0x01;
   P1IN = 0x00;

   for(;;) {
      volatile unsigned int i;  // prevent optimization
      P1OUT ^= 0x01;     // Toggle P1.0 using XOR
      P1OUT ^= 0x40; // Toggle P1.6 using XOR
      x = P1IN;
      y = P1OUT;
      i = 100000;         // SW Delay
      do i--;
      while(i != 0);
      P1OUT ^= 0x01;     // Toggle P1.0 using XOR
      P1OUT ^= 0x40; // Toggle P1.6 using XOR
      x = P1IN;
      y = P1OUT;
      i = 100000;         // SW Delay
      do i--;
      while(i != 0);
   }
   return 0;
}
