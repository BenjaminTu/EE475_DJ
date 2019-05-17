#include "main.h"

void setup() {
  sensor_setup(one);
  sensor_setup(two);
  sensor_setup(three); 
  sensor_setup(four);
  Serial.begin(9600);
}

void loop() {
  setDistance(&one, measure(one));
  setDistance(&two, measure(two));
  setDistance(&three, measure(three));
  setDistance(&four, measure(four));

  Serial.print("One Measured distance: ");
  Serial.println((unsigned long)one.dist);
  Serial.print("Two Measured distance: ");
  Serial.println((unsigned long)two.dist);
  Serial.print("Three Measured distance: ");
  Serial.println((unsigned long)three.dist);
  Serial.print("Four Measured distance: ");
  Serial.println((unsigned long)four.dist);
  Serial.println();
  
  updateMove();
  Serial.print("Present State: ");
  Serial.println(ns);
  Serial.print("Next State: ");
  Serial.println(ns);
  Serial.println();

  setMotors(MAX_SPEED * (ns & 0x02)>>1, MAX_SPEED * (ns & 0x01));
    
  Serial.print("Left: ");
  Serial.println(wheels.left);
  Serial.print("Right: ");
  Serial.println(wheels.right);
  Serial.println();
  
  ps = ns;
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





  
