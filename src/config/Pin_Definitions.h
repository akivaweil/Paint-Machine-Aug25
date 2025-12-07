#ifndef PIN_DEFINITIONS_H
#define PIN_DEFINITIONS_H

//* ************************************************************************
//* ************************ PIN DEFINITIONS *******************************
//* ************************************************************************

// Stepper Motor 1 (X-axis)
#define X_STEP_PIN 19
#define X_DIR_PIN 20

// Stepper Motor 3 (Y-axis)
#define Y_STEP_PIN 35
#define Y_DIR_PIN 36

// Stepper Motor 4 (Fork - Z direction)
#define FORK_STEP_PIN 12
#define FORK_DIR_PIN 11

// Home switches (Active HIGH with pulldown)
#define X_HOME_PIN 7
#define X_HOME_PIN2 6    // Second X home switch
#define Y_HOME_PIN 4
#define FORK_HOME_PIN 18

// Storage Stepper Motor
#define STORAGE_STEP_PIN 21
#define STORAGE_DIR_PIN 47

// Paint Rotation Motor
#define PAINT_ROTATION_STEP_PIN 3
#define PAINT_ROTATION_DIR_PIN 8
#define PAINT_ROTATION_ENABLE_PIN 9

// Suction and Paint Gun
#define SUCTION_PIN 41
#define PAINT_GUN_PIN 42

// Pressure Pot
#define PRESSURE_POT_PIN 39

// Status LEDs
#define STATUS_LED_PIN 2  // Built-in LED on most ESP32 boards
#define ERROR_LED_PIN 2   // Using same pin for now

// Test button
#define TEST_BUTTON_PIN 17

// Storage Position Sensor (Active HIGH with pulldown)
#define STORAGE_POSITION_SENSOR_PIN 2

// Square Present Sensor (Active LOW with pullup)
#define SQUARE_PRESENT_SENSOR_PIN 16

// Servo Motor (24V 160kg 180 degree)
#define SERVO_PIN 40

#endif // PIN_DEFINITIONS_H 