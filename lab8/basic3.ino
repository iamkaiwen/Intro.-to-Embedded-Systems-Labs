#include <Stepper.h>
#define STEPS 2048
Stepper stepper(STEPS, 8 , 10 , 9 , 11);
int mode = 0 , counter = 0;
int redPin = 3 , bluePin = 5;
int xAxis = A0 , yAxis = A1;
int button_int = 0;
int xVal , yVal, prexVal , preyVal;
void setup() {
  pinMode(redPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  
  // Serial communication at 9600 bits per second:
  Serial.begin(9600);
  
  cli(); // stop interrupts, atomic access to reg.
  // initialize timer1 
  TCCR1A = 0;    TCCR1B = 0;    TCNT1 = 0;
  TCCR1B |= (1 << WGM12); // turn on CTC
  TCCR1B |= (1<<CS12) | (1<<CS10); // 1024 prescaler
  OCR1A = 7812;  // target for counting
  TIMSK1 |= (1<<OCIE1A); // enable timer compare int.
  sei(); // enable all interrupts

  //BUTTON CLICK
  attachInterrupt(button_int, handle_click, RISING);

  stepper.setSpeed(5);
}

ISR(TIMER1_COMPA_vect) { // Timer1 ISR
  if(mode == 1){
    if(counter == 1){
      analogWrite(redPin , xVal / 4);
      analogWrite(bluePin , yVal / 4);
      counter = 0;
    }else{
      analogWrite(redPin , 0);
      analogWrite(bluePin , 0);
      counter = 1;
    }
  }
}

// button ISR
void handle_click() { // button debouncing, toggle LED
  static unsigned long last_int_time = 0;
  unsigned long int_time = millis(); // Read the clock

  if (int_time - last_int_time > 200 ) {  
    // Ignore when < 200 msec
    mode = 1 - mode;
    counter = 0;
  }

  last_int_time = int_time;
}

void loop() {
  xVal = analogRead(xAxis);
  yVal = analogRead(yAxis);
  if(xVal == 1023 && prexVal < 1020){
    stepper.step(256);
    Serial.println("yes");
    digitalWrite(8 , LOW); digitalWrite(9 , LOW); digitalWrite(10 , LOW); digitalWrite(11 , LOW);
  }else if(yVal == 1023 && preyVal < 1020){
    stepper.step(-256);
    digitalWrite(8 , LOW); digitalWrite(9 , LOW); digitalWrite(10 , LOW); digitalWrite(11 , LOW);
  }
  Serial.print(xVal);
  Serial.print(" ");
  Serial.println(prexVal);
  prexVal = xVal;
  preyVal = yVal;
  if(mode == 0){
    analogWrite(redPin , double(xVal) / 4);
    analogWrite(bluePin , double(yVal) / 4);
  }
}
