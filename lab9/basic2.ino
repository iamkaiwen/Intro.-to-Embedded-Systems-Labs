#include <Arduino_FreeRTOS.h>
int pr_pin = A0;
int red_pin = 9 , blue_pin = 11 , green_pin = 10;
int x_pin = A2 , y_pin = A1;
int button_int = 0 , mode = 0 , count = 0;

int pr_val;
int x_val , y_val;
int select , ok;

// 宣告 task handle
TaskHandle_t handleLED;
TaskHandle_t handlePR;
TaskHandle_t handleJoystick;

// 宣告 function
void TaskLED(void *parameters);
void TaskPR(void *parameters);
void TaskJoystick(void *parameters);

void setup() {
  Serial.begin(9600);
  pinMode(red_pin, OUTPUT);
  pinMode(blue_pin, OUTPUT);
  pinMode(green_pin, OUTPUT);
  pinMode(pr_pin, INPUT);
  pinMode(x_pin, INPUT);
  pinMode(y_pin, INPUT);

  //Led Setting
  analogWrite(red_pin , 100);
  analogWrite(blue_pin , 100);
  analogWrite(green_pin , 100);
  
  while(!Serial) {} // 等 serial 設定好
  
  // 宣告 task
  xTaskCreate(TaskLED, "LED", 128, NULL, 1, NULL);
  xTaskCreate(TaskPR, "PR", 128, NULL, 1, NULL);
  xTaskCreate(TaskJoystick, "Joystick", 128, NULL, 1, NULL);

  //BUTTON CLICK
  attachInterrupt(button_int, handle_click, RISING);
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
    if(mode == 1){
      Serial.println("Game Start!");
    }else{
      analogWrite(red_pin , 100);
      analogWrite(blue_pin , 100);
      analogWrite(green_pin , 100);
    }
  }

  last_int_time = int_time;
}

void TaskLED(void* parameters) {
  (void) parameters;


  for (;;) {
    if(mode == 1){
      select = random(1 , 4);
      int led_num = (select == 1)? red_pin :
                    (select == 2)? blue_pin : green_pin;
      int led_val = (int)((pr_val / 512.0)) * random(128 , 256);
      if(select != 1){
        analogWrite(red_pin , 0);
      }
      if(select != 2){
        analogWrite(blue_pin , 0);
      }
      if(select != 3){
        analogWrite(green_pin , 0);
      }
      analogWrite(led_num , led_val);
      ok = 0;
      vTaskDelay(50);
      if(ok == 1){
        count += 1;
      }else{
        count -= 1;
      }
      Serial.print("Score : ");
      Serial.println(count);
      int secs = random(1 , 4);
      vTaskDelay(secs * 1000 / portTICK_PERIOD_MS - 50);
    }else{
      Serial.println("Standby!");
    }
  }

  vTaskDelete(NULL);
}

void TaskPR(void* parameters) {
  (void) parameters;
  
  for (;;) {
    pr_val = analogRead(pr_pin);
    vTaskDelay(5);
  }

  vTaskDelete(NULL);
}

void TaskJoystick(void* parameters) {
  (void) parameters;


  for (;;) {
    x_val = analogRead(x_pin);
    y_val = analogRead(y_pin);
    if(x_val < 400 && select == 1){
      ok = 1;
    }else if(x_val > 700 && select == 2){
      ok = 1;
    }else if(y_val > 700 && select == 3){
      ok = 1;
    }else{
      ok = 0;
    }
  }

  vTaskDelete(NULL);
}
