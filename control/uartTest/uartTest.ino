#include <SPI.h>

String inputString = "";         // a String to hold incoming data
bool stringComplete = false;  // whether the string is complete

void setup() {
  btSetup();
  spiSetup();
}

void loop() {
  // print the string when a newline arrives
  if (stringComplete) {
    // clear the string:
    spiWrite();
    inputString = "";
    stringComplete = false;
  }
  /*
  String data = "SENSOR" + String(3) + " " + String(20);
  char nulL = ':';
  char cString[data.length()+1];
  for (int i = 0; i < data.length(); i++) {cString[i] = data[i];}
  cString[data.length()] = nulL;
  
  Serial.write(cString); 
  */
  delay(10);
}

void btSetup() {
  // reserve 200 bytes for inputString
  inputString.reserve(200);
  Serial.begin(115200);
}

void spiSetup() {
   digitalWrite(SS, HIGH); // disable Slave Select
   SPI.begin ();
   SPI.setClockDivider(SPI_CLOCK_DIV8);//divide the clock by 8
}

void spiWrite() {
   char c;
   digitalWrite(SS, LOW); // enable Slave Select
   // send test string
   char arr[inputString.length()];
   inputString.toCharArray(arr, inputString.length());
   for (int i = 0; i < inputString.length(); i++) {
      SPI.transfer (arr[i]);
   }
   digitalWrite(SS, HIGH); // disable Slave Select
  }

/*
  SerialEvent occurs whenever a new data comes in the hardware serial RX. This
  routine is run between each time loop() runs, so using delay inside loop can
  delay response. Multiple bytes of data may be available.
*/
void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == 0x0) {
      stringComplete = true;
    }
  }
}