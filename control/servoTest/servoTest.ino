
int pinF = 2;
int pinB = 3;


void setup() {
  pinMode(pinF, OUTPUT);
  pinMode(pinB, OUTPUT);
  
  // put your setup code here, to run once:
}

void loop() {
  // put your main code here, to run repeatedly:
  // clockwise increase
  // counterclockwise decrease
  analogWrite(pinF,100);
  analogWrite(pinB, 0);
}
