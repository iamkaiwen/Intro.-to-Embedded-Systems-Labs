#include <Arduino_FreeRTOS.h>
int pr_pin = A0;
int red_pin = 9 , blue_pin = 11 , green_pin = 10;
int x_pin = A2 , y_pin = A1 , btn_pin = 7;

int pr_val;
int x_val , y_val;

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
  while(!Serial) {} // 等 serial 設定好
  // 宣告 task
  xTaskCreate(TaskLED, "LED", 128, NULL, 1, NULL);
  xTaskCreate(TaskPR, "PR", 128, NULL, 1, NULL);
  xTaskCreate(TaskJoystick, "Joystick", 128, NULL, 1, NULL);
}

void loop() {
  // empty
}

void TaskLED(void* parameters) {
  (void) parameters;


  for (;;) {
    int red_val = (int)((pr_val / 512.0)) * (x_val / 4.0);
    int blue_val = (int)((pr_val / 512.0)) * (y_val / 4.0);
    Serial.print("RED_VAL : ");
    Serial.print(red_val);
    Serial.print(" BLUE_VAL : ");
    Serial.print(blue_val);
    Serial.print(" RR_VAL : ");
    Serial.println(pr_val);
    analogWrite(red_pin , red_val);
    analogWrite(blue_pin , blue_val);
    analogWrite(green_pin , 125);
    vTaskDelay(20);
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
  }

  vTaskDelete(NULL);
}
