#define tx 3
#define rx 2

enum busStatus_t {idle, attempt, master, collision, slave };
busStatus_t bus;

bool trigger, LEDgo;
unsigned char LEDstate;
unsigned long LEDstateTime;

void pullDown(unsigned char pin) {pinMode(pin, OUTPUT);digitalWrite (pin, LOW);}
void leaveHigh(unsigned char pin) {pinMode(pin, INPUT_PULLUP);}

void setup() {
  Serial.begin(9600);
  Serial.println(" ");

  trigger = LEDgo = false;
  leaveHigh(tx);
  digitalWrite (tx, LOW);
  leaveHigh(rx);
  digitalWrite (rx, LOW);
  Serial.begin(9600);
  randomSeed(analogRead(0));
}
void loop ()
{
  //bus arbitrator
  {
    static char state, rnd;
    static unsigned long t;
//    {
//      static char old;
//      if (old!=state) {
//        Serial.print("arbitrator state = "); Serial.println(state, DEC);
//       old=state;
//     }
//    }
//
//    Serial.print("arbitrator state = "); Serial.println(state, DEC);
    
    switch(state) {
      Serial.println("Idle");
      case 0:{ // idle
        // tx==true, rx==true
        bus = idle;
        leaveHigh(tx);
        leaveHigh(rx);
        if(!(digitalRead(tx) && digitalRead(rx))) state = 4; // we are slave
        else if(trigger) {
          state = 1; //attempt
          t=millis();
          rnd=random(50,100);
        }
        else state = 0; // rx is high and no trigger, i.e. we are idle
        break;
      }
      case 1:{ // attempt
      Serial.println("Attempt");
        bus = attempt;
        pullDown(tx);
        if (!trigger) state = 0; //back to idle
        else if (!digitalRead(rx)) {
          state = 3; //collision
          t=millis();
          rnd=random(10);
        }
        else if (((long)(millis()-t))>=rnd) state = 2; // we are master
        else state = 1; // still attempt
        break;
      }
      case 2:{ // we are master
        Serial.println("We are Primary"); 
        bus = master;
        if (!trigger) {
          state = 0; //go back to idle
          leaveHigh(tx);
          leaveHigh(rx);
        }
        else if (!LEDgo) { //still trigget but no LEDgo
          pullDown(tx);
          state = 2;
        }
        else { // trigger and LEDgo
          if((LEDstate-1)/2) leaveHigh(tx);
          else pullDown(tx);
          if((LEDstate-1)%2) leaveHigh(rx);
          else pullDown(rx);
        }        
        break;
      }
      case 3:{ // collision
      Serial.println("Collision");
        bus = collision;
        leaveHigh(tx);
        leaveHigh(rx);
        if (((long)(millis()-t))<rnd) state = 3; // still collision
        else if (digitalRead(tx) && digitalRead(rx)) state = 0; // back to idle
        else state = 4; //we are slave
        break;
      }
      case 4:{ // we are slave
      Serial.println("We are Secondary");
        bus = slave;
        leaveHigh(tx);
        leaveHigh(rx);
        if (digitalRead(tx) && digitalRead(rx)) {
          // de-bounce idle state condition
          state = 5;
          t=millis();
        }
        else if (!digitalRead(rx) && !digitalRead(tx)) {
          // de-bounce LEDstate = 1
          state = 6;
          t=millis();
        }
        else if (!digitalRead(rx) && digitalRead(tx)) {
          // de-bounce LEDstate = 2
          state = 7;
          t=millis();
        }
        else if (digitalRead(rx) && !digitalRead(tx)) {
          // de-bounce LEDstate = 3
          state = 8;
          t=millis();
        }
        break;
      }
      case 5:{ // de-bounce idle state condition
        unsigned long m;
        bus = slave;
        leaveHigh(tx);
        leaveHigh(rx);
        m=millis();
        if (((long)(m-t))>150) state = 0; // go to idle
        else if (!(digitalRead(tx) && digitalRead(rx))) state = 4; // go back to slave
        else state = 5; // wait for the de-bounce period
        break;
      }
      case 6:{ // de-bounce LEDstate = 1
        unsigned long m;
        bus = slave;
        leaveHigh(tx);
        leaveHigh(rx);
        m=millis();
        if (((long)(m-t))<150 && (!digitalRead(rx) && !digitalRead(tx))) state = 6; // stay here
        else if (!(!digitalRead(rx) && !digitalRead(tx))) state = 4; // go back to slave
        else {
          LEDstate = 1; // accept LEDstate = 1
          LEDstateTime = m-150;
          state = -6;
        }
        break;
      }
      case -6:{ // LEDstate = 1 with LEDstateTime set
        bus = slave;
        leaveHigh(tx);
        leaveHigh(rx);
        if (!(!digitalRead(rx) && !digitalRead(tx))) state = 4; // go back to slave
        else LEDstate = 1; // accept LEDstate = 1
        break;
      }
      case 7:{ // de-bounce LEDstate = 2
        unsigned long m;
        bus = slave;
        leaveHigh(tx);
        leaveHigh(rx);
        m=millis();
        //Serial.print("\t");Serial.println((long)(m-t), DEC);
        if (((long)(m-t))<150 && (!digitalRead(rx) && digitalRead(tx))) state = 7; // stay here
        else if (!(!digitalRead(rx) && digitalRead(tx))) state = 4; // go back to slave
        else {
          LEDstate = 2; // accept LEDstate = 2
          LEDstateTime = m-150;
          state = -7;
        }
        break;
      }
      case -7:{ // LEDstate = 2 with LEDstateTime set
        bus = slave;
        leaveHigh(tx);
        leaveHigh(rx);
        if (!(!digitalRead(rx) && digitalRead(tx))) state = 4; // go back to slave
        else LEDstate = 2; // accept LEDstate = 2
        break;
      }
      case 8:{ // de-bounce LEDstate = 3
        unsigned long m;
        bus = slave;
        leaveHigh(tx);
        leaveHigh(rx);
        m=millis();
        if (((long)(m-t))<150 && (digitalRead(rx) && !digitalRead(tx))) state = 8; // stay here
        else if (!(digitalRead(rx) && !digitalRead(tx))) state = 4; // go back to slave
        else {
          LEDstate = 3; // accept LEDstate = 1
          LEDstateTime = m-150;
          state = -8;
        }
        break;
      }
      case -8:{ // LEDstate = 3 with LEDstateTime set
        bus = slave;
        leaveHigh(tx);
        leaveHigh(rx);
        if (!(digitalRead(rx) && !digitalRead(tx))) state = 4; // go back to slave
        else LEDstate = 3; // accept LEDstate = 3
        break;
      }
      default: state=0;
    } // end switch
  } // end bus arbitrator
}
