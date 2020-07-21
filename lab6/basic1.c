#include <msp430.h>
#include <stdio.h>
#define UART_TXD 0x02 
#define UART_RXD 0x04 
#define UART_TBIT_DIV_2     (1000000 / (9600 * 2))
#define UART_TBIT           (1000000 / 9600) 
unsigned int txData;  
unsigned int count = 0;  
unsigned int tx_flag = 0;  
unsigned int rx_flag = 0; 
unsigned char rxBuffer; 
char count_str[80];

void TimerA_UART_init(void);
void TimerA_UART_tx(unsigned char byte);
void TimerA_UART_print(char *string);

void main(void) {
  //Stop Watch Dog Timer
  WDTCTL = WDTPW + WDTHOLD;

  DCOCTL = 0x00;
  BCSCTL1 = CALBC1_1MHZ;
  DCOCTL = CALDCO_1MHZ;

  //UART settings
  P1OUT = 0x00;       
  P1SEL = UART_TXD + UART_RXD; 
  P1DIR = 0xFF & ~UART_RXD;

  //TIMER1
  TA1CCR0 = 3000 - 1;
  TA1CTL = MC_1 | ID_3 | TASSEL_1 | TACLR;
  TA1CCTL0 |= CCIE;

  __enable_interrupt();

  TimerA_UART_init();

  for (;;) {
    //Transfer
    if(tx_flag == 1){
        TimerA_UART_print("HELLO!\r\n");
        tx_flag = 0;
    }
    //Recieve
    if(rx_flag == 1){
        if(rxBuffer == '\x0d'){
            sprintf(count_str , "%d\r\n" , count);
            TimerA_UART_print(count_str);
            count = 0;
        }else{
            count++;
        }
        rx_flag = 0;
    }
  }
}

//UART settings
void TimerA_UART_init(void) {
  TA0CCTL0 = OUT;   //Set TXD
  TA0CCTL1 = SCS + CM1 + CAP + CCIE; 
  //Set RXD: sync, neg edge, capture, interrupt
  TA0CTL = TASSEL_2 + MC_2; // Timer0 SMCLK, continuous mode
}

//Transfer String
void TimerA_UART_print(char *string) {
  while (*string) TimerA_UART_tx(*string++);
}

//Transfer One Byte
void TimerA_UART_tx(unsigned char byte) {
  while (TA0CCTL0 & CCIE); 
  TA0CCR0 = TA0R;      
  TA0CCR0 += UART_TBIT; 
  TA0CCTL0 = OUTMOD0 + CCIE; 
  txData = byte;       
  txData |= 0x100;    
  txData <<= 1;       
}
//Transfer One Bit
#pragma vector = TIMER0_A0_VECTOR  
__interrupt void Timer_A0_ISR(void) {
  static unsigned char txBitCnt = 10; 
  TA0CCR0 += UART_TBIT; 
  if (txBitCnt == 0) {  
    TA0CCTL0 &= ~CCIE;  
    txBitCnt = 10;      
  } else {
    if (txData & 0x01) {
      TA0CCTL0 &= ~OUTMOD2; 
    } else {
      TA0CCTL0 |= OUTMOD2;} 
    txData >>= 1;        txBitCnt--;
  }
}
//Recieve One Bit
#pragma vector = TIMER0_A1_VECTOR 
__interrupt void Timer_A1_ISR(void) {
  static unsigned char rxBitCnt = 8;
  static unsigned char rxData = 0;
  switch (__even_in_range(TA0IV, TA0IV_TAIFG)) { 
    case TA0IV_TACCR1:     
      TA0CCR1 += UART_TBIT; 
      if (TA0CCTL1 & CAP) { 
        TA0CCTL1 &= ~CAP;   
        TA0CCR1 += UART_TBIT_DIV_2;
      } else {             
        rxData >>= 1;
        if (TA0CCTL1 & SCCI) { 
          rxData |= 0x80; }
        rxBitCnt--;
        if (rxBitCnt == 0) {  
          rxBuffer = rxData;  
          rxBitCnt = 8;       
          TA0CCTL1 |= CAP;     
          rx_flag = 1;
        }
      }
      break;
    }
}
//Transfer
#pragma vector = TIMER1_A0_VECTOR
__interrupt void TA1_ISR(void){
    tx_flag = 1;
}

