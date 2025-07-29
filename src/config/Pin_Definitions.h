#ifndef PIN_DEFINITIONS_H
#define PIN_DEFINITIONS_H

//* ************************************************************************
//* ************************ PIN DEFINITIONS *******************************
//* ************************************************************************

// Stepper Motor 1 (X1-axis - Left)
#define X1_STEP_PIN 2
#define X1_DIR_PIN 3
#define X1_ENABLE_PIN 4

// Stepper Motor 2 (X2-axis - Right)
#define X2_STEP_PIN 5
#define X2_DIR_PIN 6
#define X2_ENABLE_PIN 7

// Stepper Motor 3 (Y-axis)
#define Y_STEP_PIN 8
#define Y_DIR_PIN 9
#define Y_ENABLE_PIN 10

// Stepper Motor 4 (Fork - Z direction)
#define FORK_STEP_PIN 11
#define FORK_DIR_PIN 12
#define FORK_ENABLE_PIN 13

// Home switches (Active LOW with pullup)
#define X1_HOME_PIN 14
#define X2_HOME_PIN 15
#define Y_HOME_PIN 16
#define FORK_HOME_PIN 17

// Limit switches (Active LOW with pullup)
#define X1_LIMIT_PIN 18
#define X2_LIMIT_PIN 19
#define Y_LIMIT_PIN 20
#define FORK_LIMIT_PIN 21

// Pneumatic solenoid pins (5V relay control)
#define SOLENOID_1_PIN 17
#define SOLENOID_2_PIN 18
#define SOLENOID_3_PIN 19
#define SOLENOID_4_PIN 20

// Status LED pins
#define STATUS_LED_PIN 22
#define ERROR_LED_PIN 23

// Emergency stop (Active HIGH with pulldown)
#define E_STOP_PIN 24

// Start/Stop buttons (Active HIGH with pulldown)
#define START_BUTTON_PIN 25
#define STOP_BUTTON_PIN 26

// Additional control pins
#define RESET_PIN 27
#define PAUSE_PIN 28

#endif // PIN_DEFINITIONS_H 