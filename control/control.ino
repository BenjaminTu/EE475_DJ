enum state {OFF, AUTO, MAN};
enum dir {FORWARD, REVERSE};

typedef struct{
    int leftPin;
    int rightPin;
    dir leftDir;
    dir rightDir;
    int left;
    int right;
    state stat;
  } control;

control wheels = (control) {2, 3, FORWARD, FORWARD, 0, 0, OFF};

typedef struct {
    int echoPin;
    int triggerPin;
    unsigned long dist;
  } sensorID;

sensorID one = (sensorID) {13, 12, 0};
sensorID two = (sensorID) {12, 13, 0};
sensorID three = (sensorID) {12, 13, 0};
sensorID four = (sensorID) {12, 13, 0};



void setup() {
  sensor_setup(one);
  setMotors(128, 64);
  Serial.begin(9600);
}

void loop() {
  setDistance(&one, measure(one));
  Serial.print("Measured distance: ");
  Serial.println((unsigned long)one.dist);
  Serial.print("Left: ");
  Serial.println(wheels.left);
  Serial.print("Right: ");
  Serial.println(wheels.right);

  delay(100);
}

void sensor_setup(sensorID sense) {
  pinMode(sense.echoPin, INPUT);
  pinMode(sense.triggerPin, OUTPUT);
}

void setDistance(sensorID* sense, unsigned long distance) { sense->dist = distance; }

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
  (&wheels)->left = speedL; 
  (&wheels)->right = speedR;
  analogWrite(wheels.leftPin, wheels.left);
  analogWrite(wheels.rightPin, wheels.right);
}

void motorSetup() {
  pinMode(wheels.leftPin, OUTPUT);
  pinMode(wheels.rightPin, OUTPUT);
}





  
