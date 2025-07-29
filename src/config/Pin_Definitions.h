#ifndef PIN_DEFINITIONS_H
#define PIN_DEFINITIONS_H

//* ************************************************************************
//* ************************ PIN DEFINITIONS *******************************
//* ************************************************************************

// Stepper Motor 1 (Top X-axis)
#define X1_STEP_PIN 48
#define X1_DIR_PIN 45

// Stepper Motor 2 (Bottom X-axis)
#define X2_STEP_PIN 19
#define X2_DIR_PIN 20

// Stepper Motor 3 (Y-axis)
#define Y_STEP_PIN 35
#define Y_DIR_PIN 36

// Stepper Motor 4 (Fork - Z direction)
#define FORK_STEP_PIN 12
#define FORK_DIR_PIN 11

// Home switches (Active HIGH with pulldown)
#define X1_HOME_PIN 6
#define X2_HOME_PIN 7
#define Y_HOME_PIN 4
#define FORK_HOME_PIN 5

// Limit switches (Active LOW with pullup)
#define X1_LIMIT_PIN 14
#define X2_LIMIT_PIN 15
#define Y_LIMIT_PIN 16
#define FORK_LIMIT_PIN 17

// Storage Stepper Motor
#define STORAGE_STEP_PIN 21
#define STORAGE_DIR_PIN 47

// Status LEDs
#define STATUS_LED_PIN 2  // Built-in LED on most ESP32 boards
#define ERROR_LED_PIN 2   // Using same pin for now

#endif // PIN_DEFINITIONS_H 