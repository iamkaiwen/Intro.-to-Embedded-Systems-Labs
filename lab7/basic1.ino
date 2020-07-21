#define BLUELED 13
#define REDLED 12
#define GREENLED 11
#define BTNR 3
#define BTNG 2
int state; //state = 0 standby 1 not standby
int led_state; //1 red 2 green 3 both
int led_turn; //0 going to turn on 1 going to turn off 2 waiting 3 reset
int tfr = 0 , tfg = 0;
int score = 0;
int delayTime = 0 ,  delay_counter = 0;
void General_setup(){
  state = 0;
  led_state = 0;
  led_turn = 0;
  tfr = 0 , tfg = 0;
  score = 0;
  OCR1A = 7812 * 2;
}
void print_score(int x){
   Serial.print("You have got ");
   Serial.print(score);
   Serial.println(" points.");
}
void setup() {
  // put your setup code here, to run once:
  //General
  General_setup();
  // Serial communication at 9600 bits per second:
  Serial.begin(9600);
  //LED
  pinMode(BLUELED,OUTPUT);
  pinMode(REDLED,OUTPUT);
  pinMode(GREENLED,OUTPUT);
  //Button
  pinMode(BTNR,INPUT);
  pinMode(BTNG,INPUT);
  // Timer
  noInterrupts();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1<<CS12) | (1<<CS10);
  OCR1A = 7812 * 2;
  interrupts();
  Serial.println("The game start!");
}

void loop() {
  // put your main code here, to run repeatedly:
  tfr = 0 , tfg = 0;
  for(int i = 0;i < 5;i++){
    if(tfr == 0 && digitalRead(BTNR) == 1){
      tfr = 1;
    }
    if(tfg == 0 && digitalRead(BTNG) == 1){
      tfg = 1;
    }
    delay(100);
  }
  if(score <= -5){
    General_setup();
  }else if(state == 0 && tfr == 1 && tfg == 1 || state == 1 && led_turn == 3){
    state = 1;    led_turn = 0;
    delayTime = random(1 , 4) , delay_counter = 0;
    led_state = random(1 , 4);
    OCR1A = 15625;
    digitalWrite(BLUELED, LOW);
  }else if(state == 1 && led_turn == 2){
    if(led_state == 1){
      if(tfr == 1 && tfg == 0){
        led_turn = 3; score++;
        print_score(score);
      }else if(tfr + tfg >= 1){
        led_turn = 3; score--;
        print_score(score);
      }
    }else if(led_state == 2){
      if(tfr == 0 && tfg == 1){
        led_turn = 3; score++;
        print_score(score);
      }else if(tfr + tfg >= 1){
        led_turn = 3; score--;
        print_score(score);
      }
   }else{
      if(tfr == 1 && tfg == 1){
          led_turn = 3; score++;
          print_score(score);
      }else if(tfr + tfg >= 1){
          led_turn = 3; score--;
          print_score(score);
      }
   }
  }

  
  if (TIFR1 & (1 << OCF1A)){
      if(state == 0){
          digitalWrite(BLUELED, digitalRead(BLUELED) ^ 1);
      }else if(led_turn == 0){
          if(delay_counter < delayTime){
            delay_counter++;
          }else{
            if(led_state == 1){
              digitalWrite(REDLED, HIGH);
            }else if(led_state == 2){
              digitalWrite(GREENLED, HIGH);
            }else{
              digitalWrite(REDLED, HIGH);
              digitalWrite(GREENLED, HIGH);
            }
            OCR1A = 7812;
            led_turn = 1;
            delay_counter = 0;
          }
      }else if(led_turn == 1){
        digitalWrite(REDLED, LOW);
        digitalWrite(GREENLED, LOW);
        led_turn = 2;
      }
      TIFR1 = (1 << OCF1A);
    }
}
