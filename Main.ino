int DEBUG = 1;  //Set to 1 to enable serial monitor debugging info

/////////Bus Variables///////////
int state_bus1 = 0;
int state_prev_bus1 = 0;
int tx = 3;
int rx = 2;
int val_tx = 0;
int val_rx = 0;
unsigned long t_bus1 = 0;
unsigned long t_0_bus1 = 0;
unsigned long randNumber = 0;
////////////////////////////////////

/////////Secondary Variables///////////
int state_sec1 = 0;
int state_prev_sec1 = 0;
////////////////////////////////////

/////////Switch Variables///////////
int state_s1 = 0;
int state_prev_s1 = 0;
int pin_s1 = 10;
int val_s1 = 0;
unsigned long t_s1 = 0;
unsigned long t_0_s1 = 0;
unsigned long bounce_delay_s1 = 40;
// int count_s1 = 0;
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
// int beep_count_led1 = 0;
// int beep_number_led1 = 10;
//////////////////////////////////////


void setup() {
  // put your setup code here, to run once:

  /////PIN State//////////
  pinMode(pin_s1, INPUT_PULLUP);
  pinMode(pin_led1, OUTPUT);
  pinMode(rx, INPUT);
  pinMode(tx, OUTPUT);
  ///////////////////////
  Serial.begin(115200);

    //if DEBUG is turned on, intiialize serial connection
  randomSeed(analogRead(0));
  if(DEBUG) {Serial.begin(115200);Serial.println("Debugging is ON");}
}

void loop() {
  // put your main code here, to run repeatedly:
  SM_bus1();
  SM_s1();
  SM_led1();

  //State Activator Protocol

  //BUTTON ACCESS
  if(state_bus1 == 3 && state_s1 == 0){
    // Serial.println("I TOUCH THE BUTTON MMMM");
    state_s1 = 1;
  }
  //LED Control
  if (state_s1 == 5 && state_led1 == 1) {
    Serial.println("TRIGGERED!!!");
    state_led1 = 2;
  }else if (state_s1 == 5 && state_led1 != 1) {
    state_led1 = 6;
  }

  if(DEBUG) {
    //Make a note whenever a state machine changes state
    //("Is the current state different from the previous? Yes? OK well let's tell the world the new state")
    if((state_prev_s1 != state_s1) | (state_prev_led1 != state_led1) | (state_prev_bus1 != state_bus1)) {
      Serial.print("Bus State: "); Serial.print(state_bus1);
      Serial.print(" | Switch State: "); Serial.print(state_s1);
      Serial.print(" | LED State: "); Serial.println(state_led1);
    }
  }
}

void SM_bus1(){
  //Almost every state needs these lines so I'll put it outside the State Machine
  state_prev_bus1 = state_bus1;
  //State Machine Section

  switch(state_bus1) {
    case 0: //START
    state_bus1 = 5;
    break;

    case 1: //ATTEMPT
    val_rx = digitalRead(rx);
    // This will decide if it's going to be a collision or become Primary/Secondary
    if(val_tx == 1 && val_rx == HIGH || val_tx == 0 && val_rx == LOW){
      state_bus1 = 2;// goto Collision
    } else if(val_tx == 0 && val_rx == HIGH){
      state_bus1 = 4;// goto Secondary
    } else if (val_tx == 1 && val_rx == LOW){
      state_bus1 = 3;// goto Primary
    }
    break;

    case 2: //COLLISION
    randNumber = random(0, 50);
    if(randNumber > 30){
      digitalWrite(tx,HIGH);
      val_tx = 1;
    } else{
      digitalWrite(tx,LOW);
      val_tx = 0;
    }
    break;

    case 3: //PRIMARY
    Serial.println('I AM NUMBER ONE WAAAAAAAAAAAAAAA');
    digitalWrite(tx,HIGH);
    val_tx = 1;
    break;

    case 4: //SECONDARY
    Serial.println("WALUIGI GET'S NO RESPECT");
    digitalWrite(tx,LOW);
    val_tx = 0;
    break;

    case 5: //TX PowerState
    digitalWrite(tx,HIGH);
    val_tx = 1;
    state_bus1 = 1;
    break;
  }
}

void SM_sec1(){
  
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
    // beep_count_led1++;
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
    case 0: //I'm not ready yet "waaaaaaaaaa"
    //do nothing only the Bus loop can force us out of this loop
    break;  

    case 1: //RESET!
    state_s1 = 2;
    break;
    
    case 2: //START
    val_s1 = digitalRead(pin_s1);

    if (val_s1 == LOW) {state_s1 = 3;}
    break;
    
    case 3: //GO!
    t_0_s1 = millis();
    state_s1 = 4;
    break;

    case 4: //WAIT
    val_s1 = digitalRead(pin_s1);
    t_s1 = millis();

    if(val_s1 == HIGH) {state_s1 = 1;}
    if(t_s1 - t_0_s1 > bounce_delay_s1) {
      state_s1 = 6;
      }
    break;

    case 5: //TRIGGERED
    state_s1 = 1;
    break;

    case 6: //ARMED
    val_s1 = digitalRead(pin_s1);
    if(val_s1 == HIGH) {state_s1 = 5;}
    break;

 }
}