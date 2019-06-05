
int pin = 9;
static int degree = 0;
void setup() {
  pinMode(pin, OUTPUT);
  // put your setup code here, to run once:
}

void loop() {
  // put your main code here, to run repeatedly:
  // clockwise increase
  // counterclockwise decrease
  analogWrite(pin, 128);
}
