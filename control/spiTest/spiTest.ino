#include <SPI.h>
char buff [50];
volatile byte indx;
volatile boolean process;

int MODE = 0;
int X = 0;
int Y = 0;

void setup (void) {
   Serial.begin(9600);
   pinMode(MISO, OUTPUT); // have to send on master in so it set as output
   SPCR |= _BV(SPE); // turn on SPI in slave mode
   indx = 0; // buffer empty
   process = false;
   SPI.attachInterrupt(); // turn on interrupt
}


ISR (SPI_STC_vect) {// SPI interrupt routine 
   byte c = SPDR; // read byte from SPI Data Register
   if (indx < sizeof buff) {
      buff [indx++] = c; // save data in the next index in the array buff
      if (c == '\0') { //check for the end of the word
        process = true;
      }
   }
}

void processSPI()
{
  if (process) {
      process = false; //reset the process
      Serial.println(buff); //print the array on serial monitor
      char sw = buff[0];
      if (sw == "S") 
      {
        
      }
      else if (sw == "X") 
      {
        
      }
      else if (sw == "Y")
      {
        
      }
      
      indx = 0; //reset button to zero
   }
}

void loop (void) {
   processSPI();
}
