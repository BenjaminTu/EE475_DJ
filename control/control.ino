#include "main.h"
#include <Wire.h>
#include <SPI.h>
#include <math.h>

// Set to 0 to turn off prints
#define DEBUG 1

/***
 *  SCOPE OF CODE
 *  This code is used to interface the DJ Roomba to the:
 *    - Raspberry Pi (for optional speech recognition mode control) via I2C (slave)
 *    - Bluetooth Module (for communication to the android app) via UART / Serial3
 *    - Arduino Uno (for sensor arbitration) via SPI (slave)
 *    - Motors (for motor control)
 */

#define DIR_FWD 3
#define DIR_LEFT 2
#define DIR_RIGHT 1

int NS = 0;
int PS = 0;

void setup()
{
  Serial.begin(9600);
  setupUART();
  setupI2C();
  setupSPI();
  setupPWM();
  REG[SET_MODE] = 0x02;
  REG[MODE] = 0x02;
}
void setupUART()
{
  Serial3.begin(115200);
  CURRENT_UART_STRING.reserve(200);
  CURRENT_UART_STRING = "";
  UART_PROCESS = false;
}

void setupPWM()
{
  pinMode(LEFT_MOTOR_B, OUTPUT);
  pinMode(LEFT_MOTOR_F, OUTPUT);
  pinMode(RIGHT_MOTOR_B, OUTPUT);
  pinMode(RIGHT_MOTOR_F, OUTPUT);

  pinMode(PIN_WARNING_3, INPUT);
  pinMode(PIN_WARNING_2, INPUT);
  pinMode(PIN_WARNING_1, INPUT);
}

void setupI2C()
{
  Wire.begin(I2C_SLAVE_ADDR);
  Wire.onReceive(I2C_RECEIVE); // register event / Slave Reader
  Wire.onRequest(I2C_REQUEST); // register event / Slave Writer
}

void setupSPI()
{
  pinMode(MISO, OUTPUT); // have to send on master in so it set as output
  SPCR |= _BV(SPE); // turn on SPI in slave mode
  SPI_BUFFER_INDEX = 0; // buffer empty
  SPI_PROCESS = false;
  SPI.attachInterrupt(); // turn on interrupt
}

void loop()
{
  if (REG[MODE] == 0x02) { setMotors(0.0, 0.0); }
  else if (REG[MODE] == 0x01) // AUTO
  {
    // TODO: Deal with this later
    updateMove();
    autoMode();
  }
  else if (REG[MODE] == 0x00) // MANUAL
  {
    joystickToMotors();
  }
  
  REG[MODE] = REG[SET_MODE];
  
  sendSerialTelemetry();
  checkWarning();
}

// Sets joystick value X
void setJX(int newJX)
{
  newJX %= 100;
  REG[JX_MSB] = (newJX & 0xFF00) >> 8;
  REG[JX_LSB] = (newJX & 0x00FF);
}

// Sets joystick value Y
void setJY(int newJY)
{
  newJY %= 100;
  REG[JY_MSB] = (newJY & 0xFF00) >> 8;
  REG[JY_LSB] = (newJY & 0x00FF);
}

void setMode(int newMode)
{
  REG[SET_MODE] = newMode;

}


// Processes a value from a given buffer
int processValueFromBuffer(char *buff, int startIndex)
{
  int index = startIndex;
  char thisChar = buff[startIndex];
  int value = 0;
  int sign = 1; // 1 = positive, -1 = negative
  int numbers = 0;
  while(thisChar != '\0' && (index-startIndex) <= 6)
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
    //buff[index] = 0xFF;
    thisChar = buff[++index];
  }
  if ((index-startIndex) > 5) { return 0; }
  return value * sign;
}

void processSPI()
{
  if (SPI_PROCESS) {
    if (SPI_BUFFER_INDEX > 3) // Check to make sure it read enugh data. Otherwise reset.
    {
      char SPI_HEADER[3];
      for (int i = 0; i < 3; i++) 
      { 
        SPI_HEADER[i] = SPI_BUFFER[i]; 
        //SPI_BUFFER[i] = 0xFF;
      }
      //Serial.println(SPI_BUFFER);
      int val = processValueFromBuffer(SPI_BUFFER, 3);
      if (SPI_HEADER[2] == '1') 
      {
        //Serial.print("S1 gets: ");
        //Serial.println(val);
        SENSOR1_MOST_RECENT_READING = val;
        // Set sensor registers
        REG[SENSOR1_MSB] = (val & 0xFF0000) >> 16;
        REG[SENSOR1_MMB] = (val & 0x00FF00) >>  8;
        REG[SENSOR1_LSB] = (val & 0x0000FF) >>  0;
      }
      else if (SPI_HEADER[2] == '2') 
      {
        //Serial.print("S2 gets: ");
        //Serial.println(val);
        SENSOR2_MOST_RECENT_READING = val;
        // Set sensor registers
        REG[SENSOR2_MSB] = (val & 0xFF0000) >> 16;
        REG[SENSOR2_MMB] = (val & 0x00FF00) >>  8;
        REG[SENSOR2_LSB] = (val & 0x0000FF) >>  0;
      }
      else if (SPI_HEADER[2] == '3')
      { 
        SENSOR3_MOST_RECENT_READING = val;
        // Set sensor registers
        REG[SENSOR3_MSB] = (val & 0xFF0000) >> 16;
        REG[SENSOR3_MMB] = (val & 0x00FF00) >>  8;
        REG[SENSOR3_LSB] = (val & 0x0000FF) >>  0;
      }  
      else if (SPI_HEADER[2] == '4')
      {
        //Serial.print("S4 gets: ");
        //Serial.println(val);
        SENSOR4_MOST_RECENT_READING = val;
        // Set sensor registers
        REG[SENSOR4_MSB] = (val & 0xFF0000) >> 16;
        REG[SENSOR4_MMB] = (val & 0x00FF00) >>  8;
        REG[SENSOR4_LSB] = (val & 0x0000FF) >>  0;
      }
    }          
    
    SPI_PROCESS = false;  // reset the process
    SPI_BUFFER_INDEX = 0; // reset index to zero
  } 
}

void checkWarning()
{
  boolean warn3 = digitalRead(PIN_WARNING_3);
  boolean warn2 = digitalRead(PIN_WARNING_2);
  boolean warn1 = digitalRead(PIN_WARNING_1);
  if (warn3 != WARN_3)
  {
    WARN_3_CS = true;
  }
  if (warn2 != WARN_2)
  {
    WARN_2_CS = true;
  }
  if (warn1 != WARN_1)
  {
    WARN_1_CS = true; 
  }
  WARN_1 = warn1;
  WARN_2 = warn2;
  WARN_3 = warn3;
}

void processSerialString()
{
  if (UART_PROCESS)
  {
    char sw = CURRENT_UART_STRING[0];
    // Will this work? IDK
    char charBuf[200];
    CURRENT_UART_STRING.toCharArray(charBuf, 200);
    int value = processValueFromBuffer(charBuf, 1);
    if (sw == 'S')
    {
      setMode(value);
    }
    else if (sw == 'X')
    {
      setJX(value);
    }
    else if (sw == 'Y')
    {
      setJY(value);
    }
    CURRENT_UART_STRING = "";
  }
}

void sendSerialTelemetry()
{
  // Send sensor information
  if (WARN_1_CS) 
  {
    Serial3.print("SE1 ");
    Serial3.print(WARN_1);
    Serial3.print('\0');
    
    //Serial.print("SE1 ");
    //Serial.print(WARN_1);
    //Serial.println('\0');
    WARN_1_CS = false;
  }

  if (WARN_2_CS) 
  {
    Serial3.print("SE2 ");
    Serial3.print(WARN_2);
    Serial3.print('\0');

    //Serial.print("SE2 ");
    //Serial.print(WARN_2);
    //Serial.println('\0');
    WARN_2_CS = false;
  }

  if (WARN_3_CS) 
  {
    Serial3.print("SE4 ");
    Serial3.print(WARN_3);
    Serial3.print('\0');

    
    //Serial.print("SE4 ");
    //Serial.print(WARN_3);
    //Serial.println('\0');
    WARN_3_CS = false;
  }

  if (WARN_0_CS) 
  {
    Serial3.print("XXX ");
    Serial3.print(WARN_0);
    Serial3.print('\0');

    
    //Serial.print("XXX ");
    //Serial.print(WARN_0);
    //Serial.println('\0');
    WARN_0_CS = false;
  }
  
  // Send motor information
  Serial3.print("W1 ");
  Serial3.print(MOTOR_L_SPEED);
  Serial3.print('\0');
  Serial3.print("W2 ");
  Serial3.print(MOTOR_R_SPEED);
  Serial3.print('\0');

  delay(10);
}

// Interrupt on Serial3
void serialEvent3()
{
  while (Serial3.available()) {
    char inChar = (char)Serial3.read();
    // add it to the inputString:
    CURRENT_UART_STRING += inChar;
    UART_PROCESS = false;
    
    if (inChar == '\0') {
      UART_PROCESS = true;
      processSerialString();
    }
  }
}


void updateMove() 
{
    int nsRight = !WARN_2 | WARN_3;
    int nsLeft = (!WARN_1 & (!WARN_3 | WARN_2)) | (WARN_2 & !WARN_3);
    NS = nsRight | (nsLeft << 1);
    PS = NS;         
}

void autoMode()
{
  int lb = 200 * ((NS & 0x03) == 0x02);
  int lf = 200 * (NS & 0x01);
  int rb = 200 * ((NS & 0x03) == 0x01);
  int rf = 200 * ((NS & 0x02) >> 1);

  Serial.print("LB: ");
  Serial.println(lb);
  Serial.print("LF: ");
  Serial.println(lf);
  Serial.print("RB: ");
  Serial.println(rb);
  Serial.print("RF: ");
  Serial.println(rf);
  Serial.println();
  
  analogWrite(LEFT_MOTOR_B, 0);
  analogWrite(LEFT_MOTOR_F, lf);
  analogWrite(RIGHT_MOTOR_B, 0);
  analogWrite(RIGHT_MOTOR_F, rf);
  
}

void joystickToMotors()
{
  int x = (int) (REG[JX_MSB] << 8) | REG[JX_LSB];
  int y = (int) (REG[JY_MSB] << 8) | REG[JY_LSB];  
  int lf, rf, lb, rb;
   
  int deltaX =  (x / 100.0 * MAX_SPEED);
  int deltaY = (y / 100.0 * MAX_SPEED);

  /*
  if (deltaY >= 0) {
    lf = deltaY + (deltaX+128);
    rf = deltaY + (deltaX+128);
    lb = 0;
    rb = 0;
  } else {
    lb = -deltaY + (deltaX+128);
    rb = -deltaY + (deltaX-128);
    lf = 0;
    rf = 0;
  }*/
  
  if (abs(x) < 5 && abs(y) < 5){
    // origin
    lf = 0;
    rf = 0;
    lb = 0;
    rb = 0;
  } else if (y == 0 && x > 0) {
    // full right
    // maybe very hard to get here, observing
    int fullSpeed = deltaX;
    lf = fullSpeed;
    rf = 0;
    lb = 0;
    rb = fullSpeed;
  } else if (y == 0 && x < 0) {
    // full left
    // maybe also very hard to get there
    int fullSpeed = -1 * deltaX;
    lf = 0;
    rf = fullSpeed;
    lb = fullSpeed;
    rb = 0;
  } else if (y > 0 && x == 0) {
    // full front
    int fullSpeed = deltaY;
    lf = fullSpeed;
    rf = fullSpeed;
    lb = 0;
    rb = 0;
  } else if (y < 0 && x == 0) {
    // full backwards
    int fullSpeed = -1 * deltaY;
    lf = 0;
    rf = 0;
    lb = fullSpeed;
    rb = fullSpeed;
  } else if (x > 0 && y > 0) {
    // first quardent
    int rightAngle = atan2((double)x, (double)y) * (180 / 3.14);
    int fullSpeed = sqrt(pow(deltaX, 2) + pow(deltaY, 2));
    lf = fullSpeed;
    rf = (1 - (rightAngle / 90.0)) * fullSpeed;
    lb = 0;
    rb = 0;
  } else if (x > 0 && y < 0) {
    // fourth quardent
    int leftAngle = atan2((double)x, (double)(-1 * y)) * (180 / 3.14);
    int fullSpeed = sqrt(pow(deltaX, 2) + pow(deltaY, 2));
    lf = 0;
    rf = 0;
    lb = fullSpeed;
    rb = (1 - (leftAngle/90.0)) * fullSpeed;
  } else if (x < 0 && y < 0) {
    // third Quardent
    int fullSpeed = sqrt(pow(deltaX, 2) + pow(deltaY, 2));
    int rightAngle = atan2((double)(-1 * x), (double)(-1 * y)) * (180 / 3.14);
    lf = 0;
    rf = 0;
    lb = (1 - (rightAngle/90.0)) * fullSpeed;
    rb = fullSpeed;
  } else if (x < 0 && y > 0) {
    // second quardent
    int fullSpeed = sqrt(pow(deltaX, 2) + pow(deltaY, 2));
    int leftAngle = atan2((double)(-1 * x), (double)y) * (180 / 3.14);
    lf = (1 - (leftAngle/90.0)) * fullSpeed;
    rf = fullSpeed;
    lb = 0;
    rb = 0;
  }
  
  
  analogWrite(LEFT_MOTOR_B, lb);
  analogWrite(LEFT_MOTOR_F, lf);
  analogWrite(RIGHT_MOTOR_B, rb);
  analogWrite(RIGHT_MOTOR_F, rf);
}

void setMotors(int left, int right)
{
  // Process input
  if (left > 512) { left = 512; }
  if (left < 0) {left = 0; }
  if (right > 512) { right = 512; }
  if (right < 0) { right = 0; }
  
  int sl = left;
  int sr = right;
  
  if (left < 256)
  {
    analogWrite(LEFT_MOTOR_B, sl);
    analogWrite(LEFT_MOTOR_F, 0);
  }
  else
  {
    analogWrite(LEFT_MOTOR_F, sl - 256);
    analogWrite(LEFT_MOTOR_B, 0);
  }

  if (right < 256)
  {
    analogWrite(RIGHT_MOTOR_B, sr);
    analogWrite(RIGHT_MOTOR_F, 0);
  }
  else
  {
    analogWrite(RIGHT_MOTOR_F, sr - 256);
    analogWrite(RIGHT_MOTOR_B, 0);
  }
  
  REG[MOTOR_L_SPEED] = sl;
  REG[MOTOR_R_SPEED] = sr;
}

// SPI INTERRUPT
ISR (SPI_STC_vect) // SPI interrupt routine 
{
   byte c = SPDR; // read byte from SPI Data Register
   if (SPI_BUFFER_INDEX < sizeof SPI_BUFFER) {
      SPI_BUFFER[SPI_BUFFER_INDEX++] = c; // save data in the next index in the array buff
      if (c == '\0') { //check for the end of the word
        SPI_PROCESS = true;
      }
   }
}

void I2C_RECEIVE(int QTY)
{
  if (Wire.available() >= 1) // loop through all but the last
  { 
    int index = Wire.read();
    I2C_ACTIVE_INDEX = index;
    while (Wire.available() > 0) 
    {
      char data = Wire.read(); // receive byte as a character
      REG[I2C_ACTIVE_INDEX] = data; 
      I2C_ACTIVE_INDEX ++;
    }
    I2C_ACTIVE_INDEX = index;
  }
}

void I2C_REQUEST()
{
  // write if index >= 0x9
  if (I2C_ACTIVE_INDEX != MODE) {
    unsigned char arr[4] = {REG[I2C_ACTIVE_INDEX], REG[I2C_ACTIVE_INDEX+1], REG[I2C_ACTIVE_INDEX+2], REG[I2C_ACTIVE_INDEX+3]};
    Wire.write(arr, 4); 
  } 
  else if (I2C_ACTIVE_INDEX == MODE) 
  {
    Wire.write(REG[MODE]);
  }
}

// Print line
void pl(char *p)
{
  if (DEBUG)
  {
    Serial.println(p);
  }
}

// Print without line
void p(char *p)
{
  if (DEBUG)
  {
    Serial.print(p);
  }
}
