int DEBUG = 1;  //Set to 1 to enable serial monitor debugging info

/////////Switch Variables///////////
int state_s1 = 0;
int state_prev_s1 = 0;
int pin_s1 = 10;
int val_s1 = 0;
unsigned long t_s1 = 0;
unsigned long t_0_s1 = 0;
unsigned long bounce_delay_s1 = 40;
int count_s1 = 0;
////////////////////////////////////


////////LED Variables///////////////
int state_led1 = 0;
int state_prev_led1 = 0;
int pin_led1 = 12;
int val_led1 = 0;
unsigned long t_led1 = 0;
unsigned long t_0_led1 = 0;
unsigned long on_delay_led1 = 1000;
unsigned long off_delay_led1 = 1000;
int beep_count_led1 = 0;
int beep_number_led1 = 10;
//////////////////////////////////////


void setup() {
  // put your setup code here, to run once:

  pinMode(pin_s1, INPUT_PULLUP);
  pinMode(pin_led1, OUTPUT);

  Serial.begin(115200);

    //if DEBUG is turned on, intiialize serial connection
  if(DEBUG) {Serial.begin(115200);Serial.println("Debugging is ON");}
}

void loop() {
  // put your main code here, to run repeatedly:

  SM_s1();
  SM_led1();

  if (state_s1 == 4 && state_led1 == 1) {
    Serial.println("TRIGGERED!!!");
    state_led1 = 2;
  }else if (state_s1 == 4 && state_led1 != 1) {
    state_led1 = 6;
  }

  if(DEBUG) {
    //Make a note whenever a state machine changes state
    //("Is the current state different from the previous? Yes? OK well let's tell the world the new state")
    if((state_prev_s1 != state_s1) | (state_prev_led1 != state_led1)) {
      Serial.print("Switch State: "); Serial.print(state_s1);
      Serial.print(" | LED State: "); Serial.println(state_led1);
    }
  }
}

void SM_bus1(){
  
}

void SM_led1() {
  //Almost every state needs these lines so I'll put it outside the State Machine
  state_prev_led1 = state_led1;
  //State Machine Section
  switch(state_led1) {
    case 0: //RESET
    state_led1 = 1;
    break;
    case 1: //WAIT
    //do nothing only the top level loop can force us out of this loop
    break;
    case 2: //BLINK START
    t_0_led1 = millis();
    digitalWrite(pin_led1,HIGH);
    state_led1 = 3;
    break;
    case 3: //BLINK HIGH
    //Wait for time to elapse, then proceed to TURN OFF
    t_led1 = millis();
    if (t_led1 - t_0_led1 > on_delay_led1) {state_led1 = 4;}
    break;
    case 4: //BLINK END
    //Increment the beep counter, turn off buzzer, proceed to OFF
    beep_count_led1++;
    t_0_led1 = millis();
    digitalWrite(pin_led1,LOW);
    state_led1 = 5;
    break;
    case 5: //BLINK LOW
    t_led1 = millis();
    if(t_led1  - t_0_led1 > off_delay_led1){state_led1 = 2;}
    break;
    case 6: //OFF
    digitalWrite(pin_led1,LOW);
    state_led1 = 0;
    break;
  }
  
}


void SM_s1() {
  //Almost every state needs these lines so I'll put it outside the State Machine
  state_prev_s1 = state_s1;
  //State Machine Section
  switch(state_s1) {
    case 0: //RESET!
    state_s1 = 1;
    break;
    
    case 1: //START
    val_s1 = digitalRead(pin_s1);

    if (val_s1 == LOW) {state_s1 = 2;}
    break;
    
    case 2: //GO!
    t_0_s1 = millis();
    state_s1 = 3;
    break;

    case 3: //WAIT
    val_s1 = digitalRead(pin_s1);
    t_s1 = millis();

    if(val_s1 == HIGH) {state_s1 = 0;}
    if(t_s1 - t_0_s1 > bounce_delay_s1) {
      state_s1 = 5;
      }
    break;

    case 4: //TRIGGERED
    state_s1 = 0;
    break;

    case 5: //ARMED
    val_s1 = digitalRead(pin_s1);
    if(val_s1 == HIGH) {state_s1 = 4;}
 }
}