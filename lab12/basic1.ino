#include <Arduino_FreeRTOS.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <IRremote.h>
#include <queue.h>
#include <semphr.h>

TaskHandle_t handleTaskIR;
TaskHandle_t handleTaskLED;
TaskHandle_t handleTaskLCD;

void TaskIR(void *parameters);
void TaskJoystick(void *parameters);
void TaskLCD(void *parameters);

int RECV_PIN = 2;
char ir_ch = 0;
int x_pin = A1 , y_pin = A2;
IRrecv irrecv(RECV_PIN);
decode_results results;

char arr[4];
int p;

SemaphoreHandle_t  gatekeeper = 0 , control = 0; /* global handler */

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

void setup() {
  p = 0;
  pinMode(x_pin, INPUT);
  pinMode(y_pin, INPUT);
  lcd.begin(16, 2);
  Serial.begin(9600);
  lcd.clear();
  lcd.backlight();    // open LCD backlight
  irrecv.enableIRIn(); // Start the receiver
  control = xSemaphoreCreateMutex();
  if(xSemaphoreTake(control, 5)){
  }
  xTaskCreate(TaskIR, "IR", 96, NULL, 1, &handleTaskIR);
  xTaskCreate(TaskJoystick, "LED", 96, NULL, 1, &handleTaskLED);
  xTaskCreate(TaskLCD, "LCD", 96, NULL, 1, &handleTaskLCD);
  attachInterrupt(0, ir_click, RISING);
  gatekeeper = xSemaphoreCreateMutex();

  vTaskStartScheduler();
}

// IR ISR
void ir_click() {
    if (irrecv.decode(&results)) {   
        ir_ch = translateIR(); // Function to translate code
        irrecv.resume();    // Receive the next code
    }
}

void TaskIR(void *parameters){
  (void) parameters;
  for(;;){
      while(ir_ch == 0){
        vTaskDelay(5);
      }
      if(p < 4){
          if(xSemaphoreTake(gatekeeper, 100)){
            arr[p++] = ir_ch;
            ir_ch = 0;
            xSemaphoreGive(control);
            xSemaphoreGive(gatekeeper);
          }
      }
      vTaskDelay(1);
  }
}

void TaskJoystick(void *parameters){
  (void) parameters;
  int prev_x_val , x_val , prev_y_val , y_val;
  for(;;){
    x_val = analogRead(x_pin);
    y_val = analogRead(y_pin);
    
    if(p < 4){
      if(xSemaphoreTake(gatekeeper, 100)){
            if(x_val < 400 && prev_x_val > 400){
              arr[p++] = '0';
              xSemaphoreGive(control);
            }else if(x_val > 700 && prev_x_val < 700){
              arr[p++] = '2';
              xSemaphoreGive(control);
            }else if(y_val > 700 && prev_y_val < 700){
              arr[p++] = '1';
              xSemaphoreGive(control);
            }else if(y_val < 400 && prev_y_val > 400){
              arr[p++] = '3';
              xSemaphoreGive(control);
            }
            
            xSemaphoreGive(gatekeeper);
      }
    }
    
    prev_x_val = x_val;
    prev_y_val = y_val;
    vTaskDelay(1);
  }
}

void TaskLCD(void *parameters){
  (void) parameters;
  for(;;){
    if(xSemaphoreTake(control, 100)){
        lcd.clear();
        lcd.backlight();    // open LCD backlight
        lcd.setCursor(0, 0);  // setting cursor
        if(p < 4){
          for(int i = 0;i < p;i++){
            lcd.print(arr[i]);
            lcd.setCursor(i + 1, 0);  // setting cursor
          }
          Serial.println(F("print"));
        }else{
          lcd.clear();
          lcd.backlight();    // open LCD backlight
          lcd.setCursor(0, 0);  // setting cursor
          if(arr[0] == '0' && arr[1] == '1' && arr[2] == '2' && arr[3] == '3'){
            lcd.print("access granted");
          }else{
            lcd.print("access denied");
          }
          vTaskDelay(3000 / portTICK_PERIOD_MS);
          lcd.clear();
          lcd.backlight();    // open LCD backlight
          p = 0;
        }
      }
      vTaskDelay(20);
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

void loop() {

}
