#include <Arduino_FreeRTOS.h>
#include <Wire.h>  
#include <LiquidCrystal_I2C.h>
int pr_pin = A0;
int red_pin = 9 , blue_pin = 11 , green_pin = 10;
int x_pin = A1 , y_pin = A2;
int button_int = 0 , mode = 0 , count = 0;

int pr_val;
int x_val , y_val;
int select , ok , btn ,win , loss;
int color;

// 宣告 task handle
TaskHandle_t handleStandBy;
TaskHandle_t handleTask1;
TaskHandle_t handleTask2;
TaskHandle_t handleTask3;

// 宣告 function
void TaskStandBy(void *parameters);
void Task1(void *parameters);
void Task2(void *parameters);
void Task3(void *parameters);

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

void setup() {
  Serial.begin(9600);
  pinMode(red_pin, OUTPUT);
  pinMode(blue_pin, OUTPUT);
  pinMode(green_pin, OUTPUT);
  pinMode(pr_pin, INPUT);
  pinMode(x_pin, INPUT);
  pinMode(y_pin, INPUT);
  lcd.begin(16, 2);
  
  //Led Setting
  analogWrite(red_pin , 100);
  analogWrite(blue_pin , 100);
  analogWrite(green_pin , 100);
  
  while(!Serial) {} // 等 serial 設定好

  btn = 0; win = 0; loss = 0;
  
  // 宣告 task
  xTaskCreate(TaskStandBy, "StandBy", 128, NULL, 1, &handleStandBy);
  xTaskCreate(Task1, "Task1", 128, NULL, 4, &handleTask1);
  vTaskSuspend(handleTask1);
  xTaskCreate(Task2, "Task2", 128, NULL, 3, &handleTask2);
  vTaskSuspend(handleTask2);
  xTaskCreate(Task3, "Task3", 128, NULL, 2, &handleTask3);
  vTaskSuspend(handleTask3);
  
  //BUTTON CLICK
  attachInterrupt(button_int, handle_click, RISING);

  vTaskStartScheduler();
}

void loop() {
  // empty
}

// button ISR
void handle_click() { // button debouncing, toggle LED
  static unsigned long last_int_time = 0;
  unsigned long int_time = millis(); // Read the clock

  if (int_time - last_int_time > 200 ) {  
    // Ignore when < 200 msec
    mode = 1 - mode;
    count = 0; ok =0;
    btn = 1;
  }
  last_int_time = int_time;
}

void TaskStandBy(void *parameters){
   (void) parameters;
   for(;;){
    if(btn == 1){
      btn = 0;
      if(mode == 1){
         win = 0; loss = 0;
        analogWrite(red_pin , 0);
        analogWrite(blue_pin , 0);
        analogWrite(green_pin , 0);
        vTaskResume(handleTask1);
      }else{
        vTaskPrioritySet(handleStandBy, 10);
        vTaskSuspend(handleTask1);
        analogWrite(red_pin , 100);
        analogWrite(blue_pin , 100);
        analogWrite(green_pin , 100);
        vTaskPrioritySet(handleStandBy, 1);
        vTaskResume(handleStandBy);
      }
    }
    vTaskDelay(10);
   }
}

void Task1(void* parameters) {
  (void) parameters;
  TickType_t xLastWakeTime;
  TickType_t xFrequency = 10;
//  xLastWakeTime = xTaskGetTickCount();
  for(;;){
    xLastWakeTime = xTaskGetTickCount();
    select = random(1 , 4);
    int led_num = (select == 1)? red_pin :
                    (select == 2)? blue_pin : green_pin;
    pr_val = analogRead(pr_pin);
    color = (int)((pr_val / 512)) * random(128 , 256);
//    Serial.println(pr_val);
    if(select != 1){
      analogWrite(red_pin , 0);
    }
    if(select != 2){
      analogWrite(blue_pin , 0);
    }
    if(select != 3){
      analogWrite(green_pin , 0);
    }
    analogWrite(led_num , color);
    Serial.println(F("Task 1 call resume Task 2!"));
    vTaskResume(handleTask2);
    Serial.println(F("Task 1 to Task 2!"));
    xFrequency = random(700 , 1500) / portTICK_PERIOD_MS;
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
}

void Task2(void* parameters) {
  (void) parameters;
  for(;;){
    Serial.println(F("Task 2 from Task 1!"));
    vTaskDelay(700 / portTICK_PERIOD_MS); // Block for 700 ms
    x_val = analogRead(x_pin);
    y_val = analogRead(y_pin);
    
    if(x_val < 400 && select == 1){
      win += 1;
    }else if(x_val > 700 && select == 3){
      win += 1;
    }else if(y_val > 700 && select == 2){
      win += 1;
    }else{
      loss += 1;
    }
    Serial.println(F("Task 2 call resume Task 3!"));
    vTaskResume(handleTask3);
    Serial.println(F("Task 2 to Task 3!"));
    vTaskSuspend(handleTask2);
  }
}
void Task3(void* parameters) {
  (void) parameters;
  for(;;){
    Serial.println(F("Task 3 from Task 2!"));
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
    vTaskSuspend(handleTask3);
  }
}
