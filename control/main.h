#ifndef __MAIN_H__
#define __MAIN_H__

// DEFINE REGISTERS
#define JX_MSB      0x00
#define JX_LSB      0x01

#define JY_MSB      0x02
#define JY_LSB      0x03

#define SET_MODE    0x04
#define MODE        0x09 // CHANGED THIS TO NOT CONFLICT WITH DANIEL'S RPi code (we

#define SENSOR1_MSB 0x0A
#define SENSOR1_MMB 0x0B
#define SENSOR1_LSB 0x0C

#define SENSOR2_MSB 0x0D
#define SENSOR2_MMB 0x0E
#define SENSOR2_LSB 0x0F

#define SENSOR3_MSB 0x10
#define SENSOR3_MMB 0x11
#define SENSOR3_LSB 0x12

#define SENSOR4_MSB 0x13
#define SENSOR4_MMB 0x14
#define SENSOR4_LSB 0x15

#define MOTOR_L_SPEED 0x16
#define MOTOR_R_SPEED 0x17

#define REGISTERS   0x14

unsigned char REG[REGISTERS] = {0};
// END REGISTER DEFINITIONS

// PIN DEFINITIONS

#define LEFT_MOTOR_B  3
#define LEFT_MOTOR_F  2

#define RIGHT_MOTOR_B 5
#define RIGHT_MOTOR_F 6

#define PIN_WARNING_3 50 // SENSOR 3
#define PIN_WARNING_2 51 // SENSOR 2
#define PIN_WARNING_1 53 // SENSOR 1

// END PIN DEFS

#define MAX_SPEED 255

#define I2C_SLAVE_ADDR 0x62

#define SPI_BUFFER_WIDTH 50

uint32_t SENSOR1_MOST_RECENT_READING;
uint32_t SENSOR2_MOST_RECENT_READING;
uint32_t SENSOR3_MOST_RECENT_READING;
uint32_t SENSOR4_MOST_RECENT_READING;

char SPI_BUFFER[SPI_BUFFER_WIDTH] = {0};
volatile boolean SPI_PROCESS;
volatile byte SPI_BUFFER_INDEX;

boolean WARN_3, WARN_2, WARN_1, WARN_0;
boolean WARN_3_CS, WARN_2_CS, WARN_1_CS, WARN_0_CS;

char I2C_ACTIVE_INDEX = 0x00;
void I2C_RECEIVE(int QTY);
void I2C_REQUEST();

#define USART3_BAUDRATE 115200
String CURRENT_UART_STRING;
volatile boolean UART_PROCESS;

void setupUART();
void setupI2C();
void setupSPI();
void setupPWM();

int processValueFromBuffer(char *buff, int startIndex);
void processSerialString();
void sendSerialTelemetry();
void processSPI();

void setJX(int newJX);
void setJY(int newJY);

void pl(char* string);
void p(char* string);

void setMotors(int left, int right);

#endif
