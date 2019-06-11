#include "main.h"

void sensorSetup(sensorID sense) {
  pinMode(sense.echoPin, INPUT);
  pinMode(sense.triggerPin, OUTPUT);
}

void setDistance(sensorID* sense, unsigned long distance) {
  sense->dist = distance;
  sense-> warn = (distance < 30) ? YES : NO;

  digitalWrite(sense->ID, sense->warn);
}

unsigned long measure(sensorID sense) {
  // send a trigger pulse of 1ms
  digitalWrite(sense.triggerPin, LOW);
  delayMicroseconds(5);
  digitalWrite(sense.triggerPin, HIGH);
  delayMicroseconds(50);
  digitalWrite(sense.triggerPin, LOW);

  unsigned long duration = pulseIn(sense.echoPin, HIGH);
  return (duration / 2) * 0.0343;
}

void setup() {
  Serial.begin(9600);
  
  // sensor setup
  sensorSetup(one);
  sensorSetup(two);
  sensorSetup(three);
  sensorSetup(four);
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);

}

void loop() {
  setDistance(&one, measure(one));
  
  setDistance(&two, measure(two));

  setDistance(&three, measure(three));

  setDistance(&four, measure(four));

}
