#define red_pin 9
#define blue_pin 6
#define green_pin 10
#include <Arduino_FreeRTOS.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <IRremote.h>
#include <queue.h>

int RECV_PIN = 2;      // Receive IR code at pin 2
char ir_ch = 0;
IRrecv irrecv(RECV_PIN);
decode_results results;

TaskHandle_t handleTaskIR;
TaskHandle_t handleTaskLED;
TaskHandle_t handleTaskLCD;

void TaskIR(void *parameters);
void TaskLED(void *parameters);
void TaskLCD(void *parameters);

QueueHandle_t Q1 = 0 , Q2 = 0;

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

void setup() {
    Q1 = xQueueCreate(1 , sizeof(char));
    Q2 = xQueueCreate(2 , sizeof(char));
    
    Serial.begin(9600);

    pinMode(red_pin, OUTPUT);
    pinMode(blue_pin, OUTPUT);
    pinMode(green_pin, OUTPUT);
    lcd.begin(16, 2);
    
    //Led Setting
    analogWrite(red_pin , 100);
    analogWrite(blue_pin , 100);
    analogWrite(green_pin , 100);
    irrecv.enableIRIn(); // Start the receiver
    
    xTaskCreate(TaskIR, "IR", 96, NULL, 1, &handleTaskIR);
    xTaskCreate(TaskLED, "LED", 96, NULL, 1, &handleTaskLED);
    xTaskCreate(TaskLCD, "LCD", 96, NULL, 2, &handleTaskLCD);
    vTaskSuspend(handleTaskLCD);
    attachInterrupt(0, ir_click, RISING);
    vTaskStartScheduler();
}

void loop() {
  // put your main code here, to run repeatedly:

}

// IR ISR
void ir_click() {
    if (irrecv.decode(&results)) {   
        ir_ch = translateIR(); // Function to translate code
        irrecv.resume();    // Receive the next code
        Serial.println(portTICK_PERIOD_MS);
        xTaskResumeFromISR(handleTaskIR);
    }
}

void TaskIR(void *parameters){
    (void) parameters;
    char state = 'S';
    char peek_state = '0';
    xQueueSend(Q1, &state, 5);
    lcd.clear();
    lcd.backlight();    // open LCD backlight
    lcd.setCursor(0, 0);  // setting cursor    
    lcd.print("StandBy mode!");
    for(;;){
        vTaskSuspend(handleTaskIR);
        if(ir_ch == '0'){
            if(xQueueReceive(Q1 , &peek_state , 5)){
                if(peek_state == 'S'){
                    state = 'G';
                    detachInterrupt(0);
                }else{
                    state = 'S';
                    attachInterrupt(0, ir_click, RISING);
                }
                if(xQueueSend(Q1, &state , 5)){
        //            Serial.println("Failed to send to Q1");
                    lcd.clear();
                    lcd.backlight();    // open LCD backlight
                    lcd.setCursor(0, 0);  // setting cursor
                    if(state == 'S'){    
                      lcd.print("StandBy mode!");
                    }else{
                      lcd.print("Game mode!");
                    }
                }
            }
        }else if(ir_ch == '2' || ir_ch == '4' || ir_ch == '6'){
            xQueuePeek(Q1 , &peek_state , 5);
            if(peek_state == 'G'){
                if(!xQueueSend(Q2, &ir_ch , 5)){
    //              Serial.println("IR Failed to send to Q2");
                }
            }
        }
    }
}
void TaskLED(void *parameters){
    char peek_state = '0';
    for(;;){
        xQueuePeek(Q1 , &peek_state , 5);
        if(peek_state == 'S'){
            analogWrite(red_pin , 100);
            analogWrite(blue_pin , 100);
            analogWrite(green_pin , 100);
            vTaskResume(handleTaskLCD);
            attachInterrupt(0, ir_click, RISING);
        }else if(peek_state == 'G'){
            int select = random(1 , 4);
            char led_ch = (select == 1)? 'R':
                        (select == 2)? 'B' : 'G';
            if(select != 1){
                analogWrite(red_pin , 0);
            }else{
                analogWrite(red_pin , 100);
            }
            if(select != 2){
                analogWrite(blue_pin , 0);
            }else{
                analogWrite(blue_pin , 100);
            }
            if(select != 3){
                analogWrite(green_pin , 0);
            }else{
                analogWrite(green_pin , 100);
            }
            xQueueReset(Q2);
            if(!xQueueSend(Q2, &led_ch , 5)){
        //        Serial.println("LED Failed to send to Q2");
            }
            TickType_t xFrequency = random(10 , 15) * 100 / portTICK_PERIOD_MS;
            attachInterrupt(0, ir_click, RISING);
            vTaskDelay(xFrequency);
            detachInterrupt(0);
            vTaskResume(handleTaskLCD);
        }
        vTaskDelay(20);
    }
}
void TaskLCD(void *parameters){
    int win = 0;
    int loss = 0;
    char ir_recv , led_recv , peek_state;
    for(;;){
        xQueuePeek(Q1 , &peek_state , 5);
        if(peek_state == 'G'){
            led_recv = '-'; ir_recv = '-';
            if(xQueueReceive(Q2 , &led_recv , 5)){
                if(xQueueReceive(Q2 , &ir_recv , 5)){
                    if(led_recv == 'R' && ir_recv == '2'){
                        win += 1;
                    }else if(led_recv == 'G' && ir_recv == '4'){
                        win += 1;
                    }else if(led_recv == 'B' && ir_recv == '6'){
                        win += 1;
                    }else{
                        loss += 1;
                    }
                }else{
                    loss += 1;
                }
                
                lcd.clear();
                lcd.backlight();    // open LCD backlight
                lcd.setCursor(0, 0);  // setting cursor    
                lcd.print("Win: ");
                lcd.setCursor(6, 0);   
                lcd.print(win);  
                lcd.setCursor(0, 1);
                lcd.print("Loss: ");
                lcd.setCursor(6, 1);   
                lcd.print(loss);
                lcd.setCursor(10, 1);   
                lcd.print(led_recv);
                lcd.setCursor(12, 1);   
                lcd.print(ir_recv);
            }
        }else{
            win = 0; loss = 0;
        }
        vTaskSuspend(handleTaskLCD);
    }
}

char translateIR() // takes action based on IR code received describing Car MP3 IR codes
{
    char ch = 0;
    switch (results.value) {
        case 0xFFA25D:
        //Serial.println(" CH-            ");
        break;
        case 0xE318261B:
        //Serial.println(" CH-            ");
        break;
        case 0xFF629D:
        //Serial.println(" CH             ");
        break;
        case 0x511DBB:
        //Serial.println(" CH             ");
        break;
        case 0xFFE21D:
        //Serial.println(" CH+            ");
        break;
        case 0xEE886D7F:
        //Serial.println(" CH+            ");
        break;
        case 0xFF22DD:
        //Serial.println("|<<          ");
        break;
        case 0x52A3D41F:
        //Serial.println("|<<          ");
        break;
        case 0xFF02FD:
        //Serial.println(">>|       ");
        break;
        case 0xD7E84B1B:
        //Serial.println(">>|        ");
        break;
        case 0xFFC23D:
        //Serial.println(" PLAY/PAUSE     ");
        break;
        case 0xBE90A873:
        //Serial.println(" PLAY/PAUSE     ");
        break;
        case 0xFFE01F:
        //Serial.println(" VOL-           ");
        ch = '-';
        break;
        case 0xF076C13B:
        //Serial.println(" VOL-           ");
        ch = '-';
        break;
        case 0xFFA857:
        //Serial.println(" VOL+           ");
        ch = '+';
        break;
        case 0xA3C8EDDB:
        //Serial.println(" VOL+           ");
        ch = '+';
        break;
        case 0xFF906F:
        //Serial.println(" EQ             ");
        ch = '=';
        break;
        case 0xE5CFBD7F:
        //Serial.println(" EQ             ");
        ch = '=';
        break;
        case 0xFF6897:
        //Serial.println(" 0              ");
        ch = '0';
        break;
        case 0xC101E57B:
        //Serial.println(" 0              ");
        ch = '0';
        break;
        case 0xFF9867:
        //Serial.println(" 100+           ");
        ch = '*';
        break;
        case 0x97483BFB:
        //Serial.println(" 100+           ");
        ch = '*';
        break;
        case 0xFFB04F:
        //Serial.println(" 200+           ");
        ch = '#';
        break;
        case 0xF0C41643:
        //Serial.println(" 200+           ");
        ch = '#';
        break;
        case 0xFF30CF:
        //Serial.println(" 1              ");
        ch = '1';
        break;
        case 0x9716BE3F:
        //Serial.println(" 1              ");
        ch = '1';
        break;
        case 0xFF18E7:
        //Serial.println(" 2              ");
        ch = '2';
        break;
        case 0x3D9AE3F7:
        //Serial.println(" 2              ");
        ch = '2';
        break;
        case 0xFF7A85:
        //Serial.println(" 3              ");
        ch = '3';
        break;
        case 0x6182021B:
        //Serial.println(" 3              ");
        ch = '3';
        break;
        case 0xFF10EF:
        //Serial.println(" 4              ");
        ch = '4';
        break;
        case 0x8C22657B:
        //Serial.println(" 4              ");
        ch = '4';
        break;
        case 0xFF38C7:
        //Serial.println(" 5              ");
        ch = '5';
        break;
        case 0x488F3CBB:
        //Serial.println(" 5              ");
        ch = '5';
        break;
        case 0xFF5AA5:
        //Serial.println(" 6              ");
        ch = '6';
        break;
        case 0x449E79F:
        //Serial.println(" 6              ");
        ch = '6';
        break;
        case 0xFF42BD:
        //Serial.println(" 7              ");
        ch = '7';
        break;
        case 0x32C6FDF7:
        //Serial.println(" 7              ");
        ch = '7';
        break;
        case 0xFF4AB5:
        //Serial.println(" 8              ");
        ch = '8';
        break;
        case 0x1BC0157B:
        //Serial.println(" 8              ");
        ch = '8';
        break;
        case 0xFF52AD:
        //Serial.println(" 9              ");
        ch = '9';
        break;
        case 0x3EC3FC1B:
        //Serial.println(" 9              ");
        ch = '9';
        break;
        case 0xFFFFFFFF:
        //Serial.println("               ");  //ignore the buffer error
        break;
        case 0xFF:
        //Serial.println("               "); // ignore buffer error
        break;
        //default:
        //Serial.print(" unknown button   ");
        //Serial.println(results.value, HEX);
    }
    if (ch == 0)
        return 0;
    else
        return ch;
    delay(500);
}
