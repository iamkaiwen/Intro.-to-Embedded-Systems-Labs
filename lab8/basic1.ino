int toggle = 0;
int intensity , counter = 0;
int redPin = 3 , bluePin = 9;
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
  OCR1A = 7812 * 2;  // target for counting
  TIMSK1 |= (1<<OCIE1A); // enable timer compare int.
  // initialize timer0
  TCCR0A = 0;    TCCR0B = 0;    TCNT0 = 0;
  TCCR0B |= (1 << WGM12); // turn on CTC
  TCCR0B |= (1<<CS12) | (1<<CS10); // 1024 prescaler
  OCR0A = 156;  // target for counting
  TIMSK0 |= (1<<OCIE0A); // enable timer compare int.
  sei(); // enable all interrupts
}

ISR(TIMER0_COMPA_vect) { // Timer1 ISR
  if (counter < intensity) {
    digitalWrite(redPin , LOW);
  } else {
    digitalWrite(redPin , HIGH);
  }
  if(counter == 4){
    counter = 0;
  }else{
    counter++;
  }  
}

ISR(TIMER1_COMPA_vect) { // Timer1 ISR
  intensity = random(0 , 6);
  toggle = 0 , counter = 0;
}

void loop() {
  Serial.println(intensity);
}
