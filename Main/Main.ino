int DEBUG = 1;  //Set to 1 to enable serial monitor debugging info

/////////Bus Variables///////////
int tx = 3;
int rx = 2;
int LEDstate = 1;
int state_bus1 = 0;
int state_prev_bus1 = 0;
static char rnd;
static unsigned long t;
enum busStatus_t { idle,
                   attempt,
                   master,
                   collision,
                   slave };
busStatus_t bus;
bool trigger, LEDgo;
unsigned long LEDstateTime;

void pullDown(unsigned char pin) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
}

void leaveHigh(unsigned char pin) {
    pinMode(pin, INPUT_PULLUP);
}
////////////////////////////////////

/////////Secondary Variables///////////
int state_sec1 = 0;
int state_prev_sec1 = 0;
////////////////////////////////////

/////////Switch Variables///////////
int state_s1 = 1;
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

    ////Bus Arbitrator Shit//
    trigger = LEDgo = false;
    leaveHigh(tx);
    digitalWrite(tx, LOW);
    leaveHigh(rx);
    digitalWrite(rx, LOW);
    ///////////////////////

    ////PIN State//////////
    pinMode(pin_s1, INPUT_PULLUP);
    pinMode(pin_led1, OUTPUT);
    ///////////////////////

    Serial.begin(115200);

    //if DEBUG is turned on, intiialize serial connection
    randomSeed(analogRead(0));  //Keep Random Values Random hahahahahahahahahaha

    if (DEBUG) {
        Serial.begin(115200);
        Serial.println("Debugging is ON");
    }
}

void loop() {
    // put your main code here, to run repeatedly:
    SM_bus1();
    SM_s1();
    SM_led1();

    //State Activator Protocol

    //BUTTON ACCESS
    if (state_bus1 == 2 && state_s1 == 0) {
        state_s1 = 1;
    }

    if (state_bus1 == 4) {
        state_s1 = 0;
    }

    //LED Control
    if (state_s1 == 5 && state_led1 == 1 && LEDgo == true) {
        Serial.println("TRIGGERED!!!");
        state_led1 = 2;
    } else if (state_s1 == 5 && state_led1 != 1 && LEDgo == true) {
        state_led1 = 14;
    }
    //Bus Arbitrator Controller
    if (state_bus1 == 0) {
        trigger = true;  //Triggers Bus Arbitrator and doesn't retrigger it if Arduino State is slave or has already settled on Master
    }

    if (state_bus1 == 2) {
        LEDgo = true;  //Allow LED Access
    }

    //DEBUG Output
    if (DEBUG) {
        //Make a note whenever a state machine changes state
        //("Is the current state different from the previous? Yes? OK well let's tell the world the new state")
        if ((state_prev_s1 != state_s1) | (state_prev_led1 != state_led1) | (state_prev_bus1 != state_bus1)) {
            Serial.print("Bus State: ");
            Serial.print(state_bus1);
            Serial.print(" | Switch State: ");
            Serial.print(state_s1);
            Serial.print(" | LED State: ");
            Serial.println(state_led1);
        }
    }
}

void SM_bus1() {
    //Almost every state needs these lines so I'll put it outside the State Machine
    state_prev_bus1 = state_bus1;
    //State Machine Section

    switch (state_bus1) {
        case 0: {  // idle
            // tx==true, rx==true
            bus = idle;
            leaveHigh(tx);
            leaveHigh(rx);
            if (!(digitalRead(tx) && digitalRead(rx)))
                state_bus1 = 4;  // we are slave
            else if (trigger) {
                state_bus1 = 1;  //attempt
                t = millis();
                rnd = random(50, 100);
            } else
                state_bus1 = 0;  // rx is high and no trigger, i.e. we are idle
            break;
        }
        case 1: {  // attempt
            bus = attempt;
            pullDown(tx);
            if (!trigger)
                state_bus1 = 0;  //back to idle
            else if (!digitalRead(rx)) {
                state_bus1 = 3;  //collision
                t = millis();
                rnd = random(10);
            } else if (((long)(millis() - t)) >= rnd)
                state_bus1 = 2;  // we are master
            else
                state_bus1 = 1;  // still attempt
            break;
        }
        case 2: {  // we are master
            bus = master;
            if (!trigger) {
                state_bus1 = 0;  //go back to idle
                leaveHigh(tx);
                leaveHigh(rx);
            } else if (!LEDgo) {  //still trigger but no LEDgo
                pullDown(tx);
                state_bus1 = 2;
            } else {  // trigger and LEDgo
                if ((LEDstate - 1) / 2)
                    leaveHigh(tx);
                else
                    pullDown(tx);
                if ((LEDstate - 1) % 2)
                    leaveHigh(rx);
                else
                    pullDown(rx);
            }
            break;
        }
        case 3: {  // collision
            bus = collision;
            leaveHigh(tx);
            leaveHigh(rx);
            if (((long)(millis() - t)) < rnd)
                state_bus1 = 3;  // still collision
            else if (digitalRead(tx) && digitalRead(rx))
                state_bus1 = 0;  // back to idle
            else
                state_bus1 = 4;  //we are slave
            break;
        }
        case 4: {  // we are slave
            bus = slave;
            leaveHigh(tx);
            leaveHigh(rx);
            if (digitalRead(tx) && digitalRead(rx)) {
                // de-bounce idle state condition
                state_bus1 = 5;
                t = millis();
            } else if (!digitalRead(rx) && !digitalRead(tx)) {
                // de-bounce LEDstate = 1
                state_bus1 = 6;
                t = millis();
            } else if (!digitalRead(rx) && digitalRead(tx)) {
                // de-bounce LEDstate = 2
                state_bus1 = 7;
                t = millis();
            } else if (digitalRead(rx) && !digitalRead(tx)) {
                // de-bounce LEDstate = 3
                state_bus1 = 8;
                t = millis();
            }
            break;
        }
        case 5: {  // de-bounce idle state condition
            unsigned long m;
            bus = slave;
            leaveHigh(tx);
            leaveHigh(rx);
            m = millis();
            if (((long)(m - t)) > 150)
                state_bus1 = 0;  // go to idle
            else if (!(digitalRead(tx) && digitalRead(rx)))
                state_bus1 = 4;  // go back to slave
            else
                state_bus1 = 5;  // wait for the de-bounce period
            break;
        }
        case 6: {  // de-bounce LEDstate = 1
            unsigned long m;
            bus = slave;
            leaveHigh(tx);
            leaveHigh(rx);
            m = millis();
            if (((long)(m - t)) < 150 && (!digitalRead(rx) && !digitalRead(tx)))
                state_bus1 = 6;  // stay here
            else if (!(!digitalRead(rx) && !digitalRead(tx)))
                state_bus1 = 4;  // go back to slave
            else {
                LEDstate = 1;  // accept LEDstate = 1
                LEDstateTime = m - 150;
                state_bus1 = -6;
            }
            break;
        }
        case -6: {  // LEDstate = 1 with LEDstateTime set
            bus = slave;
            leaveHigh(tx);
            leaveHigh(rx);
            if (!(!digitalRead(rx) && !digitalRead(tx)))
                state_bus1 = 4;  // go back to slave
            else
                LEDstate = 1;  // accept LEDstate = 1
            break;
        }
        case 7: {  // de-bounce LEDstate = 2
            unsigned long m;
            bus = slave;
            leaveHigh(tx);
            leaveHigh(rx);
            m = millis();
            //Serial.print("\t");Serial.println((long)(m-t), DEC);
            if (((long)(m - t)) < 150 && (!digitalRead(rx) && digitalRead(tx)))
                state_bus1 = 7;  // stay here
            else if (!(!digitalRead(rx) && digitalRead(tx)))
                state_bus1 = 4;  // go back to slave
            else {
                LEDstate = 2;  // accept LEDstate = 2
                LEDstateTime = m - 150;
                state_bus1 = -7;
            }
            break;
        }
        case -7: {  // LEDstate = 2 with LEDstateTime set
            bus = slave;
            leaveHigh(tx);
            leaveHigh(rx);
            if (!(!digitalRead(rx) && digitalRead(tx)))
                state_bus1 = 4;  // go back to slave
            else
                LEDstate = 2;  // accept LEDstate = 2
            break;
        }
        case 8: {  // de-bounce LEDstate = 3
            unsigned long m;
            bus = slave;
            leaveHigh(tx);
            leaveHigh(rx);
            m = millis();
            if (((long)(m - t)) < 150 && (digitalRead(rx) && !digitalRead(tx)))
                state_bus1 = 8;  // stay here
            else if (!(digitalRead(rx) && !digitalRead(tx)))
                state_bus1 = 4;  // go back to slave
            else {
                LEDstate = 3;  // accept LEDstate = 1
                LEDstateTime = m - 150;
                state_bus1 = -8;
            }
            break;
        }
        case -8: {  // LEDstate = 3 with LEDstateTime set
            bus = slave;
            leaveHigh(tx);
            leaveHigh(rx);
            if (!(digitalRead(rx) && !digitalRead(tx)))
                state_bus1 = 4;  // go back to slave
            else
                LEDstate = 3;  // accept LEDstate = 3
            break;
        }
        default:
            state_bus1 = 0;
    }  // end switch
}  // end bus arbitrator

void SM_led1() {
    //Almost every state needs these lines so I'll put it outside the State Machine
    state_prev_led1 = state_led1;
    //State Machine Section
    switch (state_led1) {
        case 0:  //RESET
            state_led1 = 1;
            break;

        case 1:  //WAIT
            //do nothing only the top level loop can force us out of this loop
            break;

        case 2:  //BLINK START
            t_0_led1 = millis();
            digitalWrite(pin_led1, HIGH);
            state_led1 = -2;
            LEDstate = 1;
            break;

        case -2:  //BLINK HIGH
            //Wait for time to elapse, then proceed to TURN OFF
            t_led1 = millis();
            if (t_led1 - t_0_led1 > 1000) {
                state_led1 = 3;
            }
            break;

        case 3:  //BLINK END
            //Increment the beep counter, turn off buzzer, proceed to OFF
            // beep_count_led1++;
            t_0_led1 = millis();
            digitalWrite(pin_led1, LOW);
            state_led1 = -3;
            break;

        case -3:  //BLINK LOW
            t_led1 = millis();
            if (t_led1 - t_0_led1 > 1000) {
                state_led1 = 4;
            }
            break;

        case 4:  //BLINK START
            t_0_led1 = millis();
            digitalWrite(pin_led1, HIGH);
            state_led1 = -4;
            LEDstate = 2;
            break;

        case -4:  //BLINK HIGH
            //Wait for time to elapse, then proceed to TURN OFF
            t_led1 = millis();
            if (t_led1 - t_0_led1 > 500) {
                state_led1 = 5;
            }
            break;

        case 5:  //BLINK END
            //Increment the beep counter, turn off buzzer, proceed to OFF
            // beep_count_led1++;
            t_0_led1 = millis();
            digitalWrite(pin_led1, LOW);
            state_led1 = -5;
            break;

        case -5:  //BLINK LOW
            t_led1 = millis();
            if (t_led1 - t_0_led1 > 500) {
                state_led1 = 6;
            }
            break;

        case 6:  //BLINK START
            t_0_led1 = millis();
            digitalWrite(pin_led1, HIGH);
            state_led1 = -6;
            break;

        case -6:  //BLINK HIGH
            //Wait for time to elapse, then proceed to TURN OFF
            t_led1 = millis();
            if (t_led1 - t_0_led1 > 500) {
                state_led1 = 7;
            }
            break;

        case 7:  //BLINK END
            //Increment the beep counter, turn off buzzer, proceed to OFF
            // beep_count_led1++;
            t_0_led1 = millis();
            digitalWrite(pin_led1, LOW);
            state_led1 = -7;
            break;

        case -7:  //BLINK LOW
            t_led1 = millis();
            if (t_led1 - t_0_led1 > 500) {
                state_led1 = 8;
            }
            break;

        case 8:  //BLINK START
            t_0_led1 = millis();
            digitalWrite(pin_led1, HIGH);
            state_led1 = -8;
            LEDstate = 3;
            break;

        case -8:  //BLINK HIGH
            //Wait for time to elapse, then proceed to TURN OFF
            t_led1 = millis();
            if (t_led1 - t_0_led1 > 333) {
                state_led1 = 9;
            }
            break;

        case 9:  //BLINK END
            //Increment the beep counter, turn off buzzer, proceed to OFF
            // beep_count_led1++;
            t_0_led1 = millis();
            digitalWrite(pin_led1, LOW);
            state_led1 = -9;
            break;

        case -9:  //BLINK LOW
            t_led1 = millis();
            if (t_led1 - t_0_led1 > 333) {
                state_led1 = 10;
            }
            break;

        case 10:  //BLINK START
            t_0_led1 = millis();
            digitalWrite(pin_led1, HIGH);
            state_led1 = -10;
            break;

        case -10:  //BLINK HIGH
            //Wait for time to elapse, then proceed to TURN OFF
            t_led1 = millis();
            if (t_led1 - t_0_led1 > 333) {
                state_led1 = 11;
            }
            break;

        case 11:  //BLINK END
            //Increment the beep counter, turn off buzzer, proceed to OFF
            // beep_count_led1++;
            t_0_led1 = millis();
            digitalWrite(pin_led1, LOW);
            state_led1 = -11;
            break;

        case -11:  //BLINK LOW
            t_led1 = millis();
            if (t_led1 - t_0_led1 > 333) {
                state_led1 = 12;
            }
            break;

        case 12:  //BLINK START
            //Increment the beep counter, turn off buzzer, proceed to OFF
            // beep_count_led1++;
            t_0_led1 = millis();
            digitalWrite(pin_led1, HIGH);
            state_led1 = -12;
            break;

        case -12:  //BLINK HIGH
            t_led1 = millis();
            if (t_led1 - t_0_led1 > 334) {
                state_led1 = 13;
            }
            break;
        case 13:  //BLINK END
            //Increment the beep counter, turn off buzzer, proceed to OFF
            // beep_count_led1++;
            t_0_led1 = millis();
            digitalWrite(pin_led1, LOW);
            state_led1 = -13;
            break;

        case -13:  //BLINK LOW
            t_led1 = millis();
            if (t_led1 - t_0_led1 > 334) {
                state_led1 = 2;
            }
            break;

        case 14:  //OFF
            digitalWrite(pin_led1, LOW);
            state_led1 = 0;
            break;
    }
}

void SM_s1() {
    //Almost every state needs these lines so I'll put it outside the State Machine
    state_prev_s1 = state_s1;
    //State Machine Section
    switch (state_s1) {
        case 0:  //I'm not ready yet "waaaaaaaaaa"
            //do nothing only the Bus Arbitrator can force us out of this loop
            break;

        case 1:  //RESET!
            state_s1 = 2;
            break;

        case 2:  //START
            val_s1 = digitalRead(pin_s1);

            if (val_s1 == LOW) {
                state_s1 = 3;
            }
            break;

        case 3:  //GO!
            t_0_s1 = millis();
            state_s1 = 4;
            break;

        case 4:  //WAIT
            val_s1 = digitalRead(pin_s1);
            t_s1 = millis();

            if (val_s1 == HIGH) {
                state_s1 = 1;
            }
            if (t_s1 - t_0_s1 > bounce_delay_s1) {
                state_s1 = 6;
            }
            break;

        case 5:  //TRIGGERED
            state_s1 = 1;
            break;

        case 6:  //ARMED
            val_s1 = digitalRead(pin_s1);
            if (val_s1 == HIGH) {
                state_s1 = 5;
            }
            break;
    }
}
