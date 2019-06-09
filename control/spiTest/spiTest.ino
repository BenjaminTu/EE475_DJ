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

int processValue(char *buff, int startIndex)
{
  int index = startIndex;
  char thisChar = buff[startIndex];
  int value = 0;
  int sign = 1; // 1 = positive, -1 = negative
  int numbers = 0;
  while(thisChar != '\0' && (index-startIndex) <= 5)
  {
    if (thisChar != ' ')
    {
      if (thisChar == '-')
      {
         sign = -1;
      }
      else if (thisChar <= 57 && thisChar >= 48) 
      {
          int charVal = thisChar - 48;
          value *= 10;
          value += charVal;
      }
    }
    thisChar = buff[++index];
  }
  if (index-startIndex > 5) { return 0; }
  return value * sign;
}

int limitVal(int val) 
{
  return val % 100;
}

void processSPI()
{
  if (process) {
      process = false; //reset the process
      char sw = buff[0];
      int val = processValue(buff, 1);

      if (sw == 'S') 
      {
        MODE = val;
      }
      else if (sw == 'X') 
      {
        X = limitVal(val);
      }
      else if (sw == 'Y')
      {
        Y = limitVal(val);
      }            
      indx = 0; //reset button to zero
   }
}

void loop (void) {
   processSPI();
}
