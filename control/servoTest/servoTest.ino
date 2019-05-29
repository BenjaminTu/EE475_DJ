#include <Servo.h>

Servo servo;
static int degree = 0;
void setup() {
  // put your setup code here, to run once:
  servo.attach(9);
}

void loop() {
  // put your main code here, to run repeatedly:
  // clockwise increase
  // counterclockwise decrease
  /*for (; degree < 360; degree++) {
    servo.write(degree);  
      delay(1000);
  }*/
}
