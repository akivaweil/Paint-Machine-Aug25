#ifndef PIN_DEFINITIONS_H
#define PIN_DEFINITIONS_H

//* ************************************************************************
//* ************************ PIN DEFINITIONS *******************************
//* ************************************************************************

// Stepper Motor 1 (X-axis)
#define X_STEP_PIN 2
#define X_DIR_PIN 3
#define X_ENABLE_PIN 4

// Stepper Motor 2 (Y-axis)
#define Y_STEP_PIN 5
#define Y_DIR_PIN 6
#define Y_ENABLE_PIN 7

// Stepper Motor 3 (Z-axis)
#define Z_STEP_PIN 8
#define Z_DIR_PIN 9
#define Z_ENABLE_PIN 10

// Home switches (Active LOW with pullup)
#define X_HOME_PIN 11
#define Y_HOME_PIN 12
#define Z_HOME_PIN 13

// Limit switches (Active LOW with pullup)
#define X_LIMIT_PIN 14
#define Y_LIMIT_PIN 15
#define Z_LIMIT_PIN 16

// Pneumatic solenoid pins (5V relay control)
#define SOLENOID_1_PIN 17
#define SOLENOID_2_PIN 18
#define SOLENOID_3_PIN 19
#define SOLENOID_4_PIN 20

// Status LED pins
#define STATUS_LED_PIN 21
#define ERROR_LED_PIN 22

// Emergency stop (Active HIGH with pulldown)
#define E_STOP_PIN 23

// Start/Stop buttons (Active HIGH with pulldown)
#define START_BUTTON_PIN 24
#define STOP_BUTTON_PIN 25

// Additional control pins
#define RESET_PIN 26
#define PAUSE_PIN 27

#endif // PIN_DEFINITIONS_H 