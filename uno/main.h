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

typedef struct {
    int echoPin;
    int triggerPin;
    unsigned long dist;
    int warn;
    int ID;
  } sensorID;

sensorID one = (sensorID) {2, 3, 0, NO, 10};
sensorID two = (sensorID) {4, 5, 0, NO, 11};
sensorID three = (sensorID) {6, 7, 0, NO, 12};
sensorID four = (sensorID) {8, 9, 0, NO, 13};
