#include <msp430.h>
#include <stdio.h>
#define LEDR BIT0
#define LEDG BIT6
#define UART_TXD 0x02
#define UART_RXD 0x04
#define UART_TBIT_DIV_2     (1000000 / (9600 * 2))
#define UART_TBIT           (1000000 / 9600)
//TXD variable
volatile unsigned int txData;
volatile unsigned int tx_flag = 0;
volatile unsigned int tx_count = 0;
//RXD variable
volatile unsigned char rxBuffer;
volatile unsigned int rx_flag = 0;
volatile char rx_str[4] = {0};
volatile unsigned int rx_p = 0;
//LED variable
volatile unsigned int led_state = 0; //0 : off , 1 : on
volatile unsigned int led_on = 0 , led_off = 0;
//ADC variable
volatile unsigned int adc_times = 0;
volatile unsigned int Temp[64] = {0} , adc_p = 0;
//General variable
volatile unsigned int state = 0; //0 : Normal , 1 : Emergency
volatile unsigned int i = 0;

void TimerA_UART_init(void);
void TimerA_UART_tx(unsigned char byte);
void TimerA_UART_print(char *string);
void flash(char id , int on , int off);
void temp(int interval , int times);

void main(void) {
  // Stop watchdog timer
  WDTCTL = WDTPW + WDTHOLD;

  // Set LED P1.0 P1.6 as output
  P1DIR |= 0x41;

  // Set DCOCLK to 1MHz
  DCOCTL = 0x00;
  BCSCTL1 = CALBC1_1MHZ;
  DCOCTL = CALDCO_1MHZ;

  // TXD/RXD pins
  P1OUT = 0x00;
  P1SEL = UART_TXD + UART_RXD;
  P1DIR = 0xFF & ~UART_RXD;

  //ADC
  ADC10CTL0 = SREF_1 + ADC10SHT_2 + ADC10ON + REFON + ADC10IE;
  ADC10CTL1 = INCH_10 + SHS_0 + CONSEQ_0 + ADC10SSEL_0;

  //Initialize to Normal State
  state = 0;
  flash('0' , 500 , 1500);
  temp(1000 , 1500);

  __enable_interrupt();

    for (;;) {
        //Emergency State UART
        if(state == 1){
            //Transfer "Hot!"
            if(tx_flag == 1){
                TimerA_UART_print("Hot!\r\n");
                tx_flag = 0;
            }
            //Recieve "Ack!"
            if(rx_flag == 1){
                for(i = 0;i < 3;i++){ rx_str[i] = rx_str[i + 1];}
                rx_str[3] = rxBuffer;
                if(rx_str[0] == 'A' && rx_str[1] == 'c' && rx_str[2] == 'k' && rx_str[3] == '!'){
                    //Change to Normal State
                    state = 0;
                    flash('0' , 500 , 1500);
                    temp(1000 , 1500);
                }
                rx_flag = 0;
            }
        }
    }
}

//LED settings
void flash(char id , int on , int off){
    if(id == '0'){
        //TURN ON GREEN LED
        P1OUT |= LEDG;
        P1OUT &= ~LEDR;
    }else{
        //TURN ON RED LED
        P1OUT &= ~LEDG;
        P1OUT |= LEDR;
    }
    //LED variable settings
    led_on = on;
    led_off = off;
    led_state = 1;
    //TIMER1 settings -> Led Flashing
    TA1CCR0 = led_on * 12 - 1;
    TA1CTL = MC_1 | ID_0 | TASSEL_1 | TACLR;
    TA1CCTL0 |= CCIE;
    __bis_SR_register(LPM3_bits+GIE);
}

//ADC settings
void temp(int interval , int times){
    adc_times = times;
    //TimerA0_CCR0
    TA0CTL = MC_1 | ID_0 | TASSEL_1 | TACLR;
    TA0CCR0 = interval * 12 - 1;
    TA0CCTL0 |= CCIE;
}

//Initialize TimerA UART
void TimerA_UART_init(void) {
    //TXD
    //TA0CCTL0 = OUT;   // Set TXD idle as '1'
    TA0CTL = TASSEL_2 + MC_2 + TACLR; // SMCLK, continuous mode

    //RXD
    TA0CCTL1 = SCS + CM1 + CAP + CCIE; // CCIS1 = 0 : CCI1A
    //Set RXD: sync, neg edge, capture, interrupt

    //TXD RXD Variable
    rx_p = 0;
    tx_count = 0;
}

//Transfer String
void TimerA_UART_print(char *string) {
  while (*string) TimerA_UART_tx(*string++);
}

//Transfer One byte
void TimerA_UART_tx(unsigned char byte) {
  while (TA0CCTL0 & CCIE); // Ensure last char TX'd
    if(byte == 0x0d){
        txData = byte & 0x7F;       // Load char to be TXD
    }else{
        txData = byte & 0x7F;       // Load char to be TXD
        volatile unsigned int var_s = txData , var_c = 0;
        while(var_s != 0){
            var_c ^= var_s & 1;
            var_s >>= 1;
        }
        txData = (var_c == 1)? txData | 0x80 : txData;
        var_c = 0;
    }
    txData |= 0x100;    // Add stop bit to TXData
    txData <<= 1;       // Add start bit to TXData
    TA0CCR0 = TA0R;      // Current count of TA counter
    TA0CCR0 += UART_TBIT; // One bit time till 1st bit
    TA0CCTL0 = OUT + CCIE; // Set TXD on EQU0, Int
}

// TXD interrupt
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A0_ISR(void) {
    if(state == 0){
        //Enable ADC
        ADC10CTL0 |= ENC + ADC10SC;
    }else{
        static unsigned char txBitCnt = 10; //start bit + one byte + stop bit
        TA0CCR0 += UART_TBIT; // Set TA0CCR0 for next intrp
        if (txBitCnt == 0) {  // All bits TXed?
        TA0CCTL0 &= ~CCIE;  // Yes, disable intrpt
        txBitCnt = 10;      // Re-load bit counter
        } else {
            if (txData & 0x01) { // Check next bit to TX
                TA0CCTL0 |= OUT;  // Transfer 1
            } else {
                TA0CCTL0 &= ~OUT; // Transfer 0
            }
            txData >>= 1;        txBitCnt--;
        }
    }
}
// RXD interrupt
#pragma vector = TIMER0_A1_VECTOR
__interrupt void Timer_A1_ISR(void) {
  static unsigned char rxBitCnt = 8;
  static unsigned char rxData = 0;
  switch (__even_in_range(TA0IV, TA0IV_TAIFG)) {
    case TA0IV_TACCR1:     // TACCR1 - UART RXD
      TA0CCR1 += UART_TBIT; // Set TACCR1 for next int
      if (TA0CCTL1 & CAP) { // On start bit edge
        TA0CCTL1 &= ~CAP;   // Switch to compare mode
        TA0CCR1 += UART_TBIT_DIV_2; // To middle of D0
      } else {  // Get next data bit
            rxData >>= 1;
            if (TA0CCTL1 & SCCI) { // Get bit from latch
            rxData |= 0x80;
            }
            rxBitCnt--;
            if (rxBitCnt == 0) {  // All bits RXed?
                rxData &= 0x7F;
                rxBuffer = rxData;  // Store in global
                rxBitCnt = 8;       // Re-load bit counter
                TA0CCTL1 |= CAP;     // Switch to capture
                rx_flag = 1;
            }
        }
        break;
    }
}

//LED Flashing , Calculate transfer time
#pragma vector = TIMER1_A0_VECTOR
__interrupt void TA1_ISR(void){
    _BIC_SR(LPM3_EXIT);
    //LED Flashing
    if(led_state == 0){
        led_state = 1;
        P1OUT = (state == 0)? P1OUT | LEDG : P1OUT | LEDR;
        TA1CCR0 = led_on * 12 - 1;
    }else{
        led_state = 0;
        P1OUT = (state == 0)? P1OUT & ~LEDG : P1OUT & ~LEDR;
        TA1CCR0 = led_off * 12 - 1;
    }
    //Calculate transfer time
    if(state == 1){
        tx_count++;
        if(tx_count == 4){
            tx_flag = 1;
            tx_count = 0;
        }
    }
}

//ADC CONTROL
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void) {
    volatile unsigned int Y = ADC10MEM;
    Temp[adc_p] = Y;
    volatile unsigned int avg_Y = (adc_p != 0)? (Temp[adc_p] + Temp[adc_p - 1]) / 2 : (Temp[0] + Temp[63]) / 2;
    adc_p = (adc_p + 1) % 64;
    if(state == 0){
        if(avg_Y > 735){
            //Emergency state
            state = 1;
            flash('1' , 200 , 300);
            TimerA_UART_init();
        }else{
            adc_times--;
            if(adc_times == 0){
                TA0CCTL0 &= ~CCIE;
            }
        }
    }
    ADC10CTL0 &= ~ENC;
}

