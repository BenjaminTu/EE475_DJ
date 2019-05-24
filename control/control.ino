#include "main.h"

void setup() {
  sensor_setup(one);
  sensor_setup(two);
  sensor_setup(three); 
  sensor_setup(four);
  Serial.begin(9600);


  Wire.begin(0x62);                // join i2c bus with address #62
  Wire.onReceive(receiveEvent); // register event / Slave Reader
  Wire.onRequest(requestEvent); // register event / Slave Writer

}

void loop() {

  noInterrupts(); // disable interrupts while measuring
  /*
  setDistance(&one, measure(one));
  setDistance(&two, measure(two));
  setDistance(&three, measure(three));
  setDistance(&four, measure(four));*/
  interrupts(); // re-enable interrupts

  Serial.print("One Measured distance: ");
  Serial.println((unsigned long)one.dist);
  Serial.print("Two Measured distance: ");
  Serial.println((unsigned long)two.dist);
  Serial.print("Three Measured distance: ");
  Serial.println((unsigned long)three.dist);
  Serial.print("Four Measured distance: ");
  Serial.println((unsigned long)four.dist);
  Serial.println();
  
  noInterrupts(); // disable interrupts while updating direction
  updateMove();
  interrupts(); // re-enable interrupts

  Serial.print("Present State: ");
  Serial.println(ns);
  Serial.print("Next State: ");
  Serial.println(ns);
  Serial.println();

  noInterrupts(); // disable interrupts while setting motors
  setMotors(MAX_SPEED * (ns & 0x02)>>1, MAX_SPEED * (ns & 0x01));
  interrupts(); // re-enable interrupts

  Serial.print("Left: ");
  Serial.println(wheels.left);
  Serial.print("Right: ");
  Serial.println(wheels.right);
  Serial.println();
  
  noInterrupts(); // disable interrupts while updating state
  ps = ns; 
  interrupts(); // re-enable interrupts
  delay(10);
}


void sensor_setup(sensorID sense) {
  pinMode(sense.echoPin, INPUT);
  pinMode(sense.triggerPin, OUTPUT);
}

void motorSetup() {
  pinMode(wheels.leftPin, OUTPUT);
  pinMode(wheels.rightPin, OUTPUT);
}

void setDistance(sensorID* sense, unsigned long distance) { 
  sense->dist = distance; 
  sense-> warn = (distance < 10)? YES : NO;  
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

void setMotors(int speedL,int speedR) {
  // ramping 
  int diffL = speedL - (wheels.left);
  (&wheels)->left += REPSONSE_FACTOR * diffL;
  int diffR = speedR - (wheels.right);
  (&wheels)->right += REPSONSE_FACTOR * diffR;
  
  analogWrite(wheels.leftPin, wheels.left);
  analogWrite(wheels.rightPin, wheels.right);
}

void updateMove() {
    int nsLeft = !two.warn | four.warn;
    int nsRight = (!one.warn  & (!four.warn | two.warn)) | (two.warn & (!four.warn | three.warn));
    ns = nsRight | (nsLeft << 1);         
}

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void receiveEvent(int howMany) {
  if (Wire.available() > 1) { // loop through all but the last
    int index = Wire.read();
    ACTIVE_INDEX = index;
    while (Wire.available() > 1) {
      char data = Wire.read(); // receive byte as a character
      // receive if index < 0x9
      if (ACTIVE_INDEX < 0x9) {
        REG[ACTIVE_INDEX] = data; 
        ACTIVE_INDEX ++;
      }
    }
    ACTIVE_INDEX = index;
  }
}

// function that executes whenever data is requested by master
// this function is registered as an event, see setup()
void requestEvent() {
  // write if index >= 0x9
  if (ACTIVE_INDEX >= 0x9) {
    Wire.write(REG[ACTIVE_INDEX]); //
  }
  ACTIVE_INDEX ++;

  // as expected by master
}






  
