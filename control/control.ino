#include "main.h"

String inputString = "";         // a String to hold incoming data
bool stringComplete = false;  // whether the string is complete

void setup() {
  Serial.begin(9600);

  sensor_setup(one);
  sensor_setup(two);
  sensor_setup(three); 
  sensor_setup(four);
  btSetup();
  
  REG[0x8] = 2; // MODE_OFF
  REG[0x9] = 2; // MODE_OFF

  Wire.begin(0x62);             // join i2c bus with address #62
  Wire.onReceive(receiveEvent); // register event / Slave Reader
  Wire.onRequest(requestEvent); // register event / Slave Writer
}

void loop() {
  
  noInterrupts(); // disable interrupts while measuring
  setDistance(&one, measure(one));
  setDistance(&two, measure(two));
  setDistance(&three, measure(three));
  setDistance(&four, measure(four));
  interrupts(); // re-enable interrupts
  
 /*
  Serial.print("One Measured distance: ");
  Serial.println((unsigned long)one.dist);
  Serial.print("Two Measured distance: ");
  Serial.println((unsigned long)two.dist);
  Serial.print("Three Measured distance: ");
  Serial.println((unsigned long)three.dist);
  Serial.print("Four Measured distance: ");
  Serial.println((unsigned long)four.dist);
  Serial.println();*/
  
  
  updateMove();
  // setting mode

  REG[MODE] = REG[SET_MODE];
  if (REG[MODE] == AUTO) {
    // auto mode
    setMotors(MAX_SPEED * ((ns & 0x02) >> 1), MAX_SPEED * (!(ns & 0x02)), MAX_SPEED * (ns & 0x01), MAX_SPEED * (!(ns & 0x01)));
  } else if (REG[MODE] == MAN) {
    // manual mode
    joystickToMotor();
  } else {
    // off
    setMotors(0, 0, 0, 0);
  }

  ps = ns; 
  
  if (stringComplete) {
    //Serial.println(inputString);
    // clear the string:
    inputString = "";
    stringComplete = false;
  }
  
}


void sensor_setup(sensorID sense) {
  pinMode(sense.echoPin, INPUT);
  pinMode(sense.triggerPin, OUTPUT);
}

void motorSetup() {
  pinMode(wheels.leftFPin, OUTPUT);
  pinMode(wheels.leftBPin, OUTPUT);
  pinMode(wheels.rightFPin, OUTPUT);
  pinMode(wheels.rightBPin, OUTPUT);
}

void btSetup() {
  // reserve 20 bytes for inputString
  inputString.reserve(200);
  Serial3.begin(115200);

}

void setDistance(sensorID* sense, unsigned long distance) { 
  sense->dist = distance; 
  sense-> warn = (distance < 10)? YES : NO;
  // write in register
  REG[0x6 + (sense->ID) * 4] = (distance >> 0x24) & 0xFF; 
  REG[0x7 + (sense->ID) * 4] = (distance >> 0x16) & 0xFF; 
  REG[0x8 + (sense->ID) * 4] = (distance >> 0x8) & 0xFF; 
  REG[0x9 + (sense->ID) * 4] = (distance) & 0xFF;
  String data = "SENSOR" + String(sense->ID) + " " + String(distance) + "\0";
  char cString[20]; 
  for (int i = 0; i < data.length(); i++) {cString[i] = data[i];}
  // Serial3.write(cString); 
  // Serial.println(cString);

}

unsigned long measure(sensorID sense) {
  // send a trigger pulse of 1ms
  digitalWrite(sense.triggerPin, LOW);
  delayMicroseconds(5);
  digitalWrite(sense.triggerPin, HIGH);
  delayMicroseconds(50); 
  digitalWrite(sense.triggerPin, LOW);

  unsigned long duration = pulseIn(sense.echoPin, HIGH);
  return (duration/2) * 0.0343; 
}

void setMotors(int speedLF, int speedLB, int speedRF, int speedRB) {
  // ramping
  
  int diffLF = speedLF - (wheels.leftF);
  (&wheels)->leftF += REPSONSE_FACTOR * diffLF;
  int diffLB = speedLB - (wheels.leftB);
  (&wheels)->leftB += REPSONSE_FACTOR * diffLB;

  int diffRF = speedRF - (wheels.rightF);
  (&wheels)->rightF += REPSONSE_FACTOR * diffRF;
  int diffRB = speedRB - (wheels.rightB);
  (&wheels)->rightB += REPSONSE_FACTOR * diffRB;
 
  // write in register
  
  // left 
  int leftData = max(wheels.leftF, wheels.leftB);
  REG[0x1A] = (leftData >> 0x24) & 0xFF; 
  REG[0x1B] = (leftData >> 0x16) & 0xFF; 
  REG[0x1C] = (leftData >> 0x8) & 0xFF; 
  REG[0x1D] = (leftData) & 0xFF; 

  // right
  int rightData = max(wheels.rightF, wheels.rightB);
  REG[0x1E] = (rightData >> 0x24) & 0xFF; 
  REG[0x1F] = (rightData >> 0x16) & 0xFF; 
  REG[0x20] = (rightData >> 0x8) & 0xFF; 
  REG[0x21] = (rightData) & 0xFF; 
  
  analogWrite(wheels.leftFPin, wheels.leftF);
  analogWrite(wheels.leftBPin, wheels.leftB);

  analogWrite(wheels.rightFPin, wheels.rightF);
  analogWrite(wheels.rightBPin, wheels.rightB);
  
}

void updateMove() {
    int nsLeft = !two.warn | four.warn;
    int nsRight = (!one.warn  & (!four.warn | two.warn)) | (two.warn & (!four.warn | three.warn));
    ns = nsRight | (nsLeft << 1);         
}

void joystickToMotor() {
  int x = (int) REG[0x0] << 24 | REG[0x1] << 16 | REG[0x2] << 8 | REG[0x3];
  int y = (int) REG[0x4] << 24 | REG[0x5] << 16 | REG[0x6] << 8 | REG[0x7];

  Serial.print("X");
  Serial.println(x);
  Serial.print("Y");
  Serial.println(y);
  
  int lf, rf, lb, rb;
  
  int deltaX =  (x / 100.0 * MAX_SPEED);
  int deltaY = (y / 100.0 * MAX_SPEED);

  if (deltaX > 0 && deltaY >= 0) {
    // first quadrant
    lf = abs(deltaY + deltaX) / 2;
    rf = abs(deltaY - deltaX) / 2;
    lb = 0;
    rb = 0;
  } else if (deltaX <= 0 && deltaY > 0) { 
    // second quadrant
    lf = abs(deltaY - deltaX) / 2;
    rf = abs(deltaY + deltaX) / 2;
    lb = 0;
    rb = 0; 
  } else if (deltaX < 0 && deltaY <= 0) {
    // third quadrant
    lb = abs(deltaY - deltaX) / 2;
    rb = abs(deltaY + deltaX) / 2;
    lf = 0;
    rf = 0; 
  } else { 
    // fourth qudrant
    lb = abs(deltaY + deltaX) / 2;
    rb = abs(deltaY - deltaX) / 2;
    lf = 0;
    rf = 0;  
  }

  (&wheels)->leftF = lf;
  (&wheels)->leftB = lb;
  (&wheels)->rightF = rf;
  (&wheels)->rightB = rb;

  analogWrite(wheels.leftFPin, wheels.leftF);
  analogWrite(wheels.leftBPin, wheels.leftB);
  analogWrite(wheels.rightFPin, wheels.rightF);
  analogWrite(wheels.rightBPin, wheels.rightB);

}
 
// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void receiveEvent(int howMany) {
  if (Wire.available() >= 1) { // loop through all but the last
    int index = Wire.read();
    ACTIVE_INDEX = index;
    while (Wire.available() > 0) {
      char data = Wire.read(); // receive byte as a character
        REG[ACTIVE_INDEX] = data; 
        /*
        Serial.print("Set: ");
        Serial.print(ACTIVE_INDEX, HEX);
        Serial.print(" to ");
        Serial.println(REG[ACTIVE_INDEX], HEX);
        */
        ACTIVE_INDEX ++;
    }
    ACTIVE_INDEX = index;
  }
}

// function that executes whenever data is requested by master
// this function is registered as an event, see setup()
void requestEvent() {
  
  // write if index >= 0x9
  if (ACTIVE_INDEX > 0x9) {
    unsigned char arr [4] = {REG[ACTIVE_INDEX], REG[ACTIVE_INDEX+1], REG[ACTIVE_INDEX+2], REG[ACTIVE_INDEX+3]};
    Wire.write(arr, 4); 
  } else if (ACTIVE_INDEX == 0x9) {
    Wire.write(REG[MODE]);
  }

  // as expected by master
}


void serialEvent3() {
  while (Serial3.available()) {
    //Serial.println(Serial3.available());
   
    // get the new byte:
    char inChar = (char)Serial3.read();
    // add it to the inputString:
    Serial.println(inChar, HEX);
    inputString += inChar;
    stringComplete = true;
    // if the incoming character is a null terminator, set a flag so the main loop can
    // do something about it:
    
    if (inChar == ':') {
      stringComplete = true;
    } 
    // if the incoming character is a null terminator, set a flag so the main loop can
    // do something about it:
  }
  Serial.println();
}







  
