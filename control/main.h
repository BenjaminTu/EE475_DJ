#include <Wire.h>

// direction
#define OFF     0
#define LEFT    1
#define RIGHT   2
#define FRONT   3

// wheel
#define FORWARD 1
#define REVERSE -1

// mode
#define MODE_OFF  2
#define AUTO      1
#define MAN       0 

// warning
#define YES     1
#define NO      0

// REG INDICES

// JOYSTICK X
#define X_HH          0x0
#define X_HL          0x1
#define X_LH          0x2
#define X_LL          0x3

// JOYSTICK Y
#define Y_HH          0x4
#define Y_HL          0x5
#define Y_LH          0x6
#define Y_LL          0x7

// MODES
#define SET_MODE      0x8
#define MODE          0x9

//SENSOR 1
#define SENSOR1_HH    0xA
#define SENSOR1_HL    0xB
#define SENSOR1_LH    0xC
#define SENSOR1_LL    0xD

// SENSOR 2
#define SENSOR2_HH    0xE
#define SENSOR2_HL    0xF
#define SENSOR2_LH    0x10
#define SENSOR2_LL    0x11

// SENSOR 3
#define SENSOR3_HH    0x12
#define SENSOR3_HL    0x13
#define SENSOR3_LH    0x14
#define SENSOR3_LL    0x15

// SENSOR 4
#define SENSOR4_HH    0x16
#define SENSOR4_HL    0x17
#define SENSOR4_LH    0x18
#define SENSOR4_LL    0x19

// WHEEL 1
#define WHEEL1_HH     0x1A
#define WHEEL1_HL     0x1B
#define WHEEL1_LH     0x1C
#define WHEEL1_LL     0x1D

// WHEEL 2
#define WHEEL2_HH     0x1E
#define WHEEL2_HL     0x1F
#define WHEEL2_LH     0x20
#define WHEEL2_LL     0x21

#define REPSONSE_FACTOR 0.5
#define MAX_SPEED 255

int ps = OFF, ns = OFF;
unsigned char REG[34] =  {0}; 
int ACTIVE_INDEX;

typedef struct{
    int leftFPin;
    int leftBPin;
    int rightFPin;
    int rightBPin;
    int leftDir;
    int rightDir;
    int leftF;
    int leftB;
    int rightF;
    int rightB;
    int stat;
  } control;

control wheels = (control) {2, 3, 4, 5, OFF, OFF, 0, 0, 0, 0, OFF};

typedef struct {
    int echoPin;
    int triggerPin;
    unsigned long dist;
    int warn;
    int ID;
  } sensorID;

sensorID one = (sensorID) {13, 12, 0, NO, 1};
sensorID two = (sensorID) {11, 10, 0, NO, 2};
sensorID three = (sensorID) {9, 8, 0, NO, 3};
sensorID four = (sensorID) {7, 6, 0, NO, 4};
