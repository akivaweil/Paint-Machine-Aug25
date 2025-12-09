#ifndef PIN_DEFINITIONS_H
#define PIN_DEFINITIONS_H

//* ************************************************************************
//* ************************ PIN DEFINITIONS *******************************
//* ************************************************************************

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 🔄 STEPPER MOTORS - X, Y, FORK                                      ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝
// X-axis Motor
#define X_STEP_PIN 19
#define X_DIR_PIN 20

// Y-axis Motor
#define Y_STEP_PIN 35
#define Y_DIR_PIN 36

// Fork Motor (Z direction)
#define FORK_STEP_PIN 12
#define FORK_DIR_PIN 11

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 🔄 STEPPER MOTORS - STORAGE & PAINT ROTATION                        ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝
// Storage Motor
#define STORAGE_STEP_PIN 21
#define STORAGE_DIR_PIN 47

// Paint Rotation Motor
#define PAINT_ROTATION_STEP_PIN 14
#define PAINT_ROTATION_DIR_PIN 13
#define PAINT_ROTATION_ENABLE_PIN 10

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 🏠 HOME SWITCHES (Active HIGH with pulldown)                        ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝
#define X_HOME_PIN 7
#define X_HOME_PIN2 6              // Second X home switch
#define Y_HOME_PIN 4
#define FORK_HOME_PIN 18

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 🔍 SENSORS                                                           ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝
#define STORAGE_POSITION_SENSOR_PIN 2      // Active HIGH with pulldown
#define SQUARE_PRESENT_SENSOR_PIN 16       // Active LOW with pullup

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 🎨 PAINTING COMPONENTS                                               ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝
#define SUCTION_PIN 41
#define PAINT_GUN_PIN 42
#define PRESSURE_POT_PIN 39
#define SERVO_PIN 40                 // 24V 160kg 180 degree servo

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 💡 STATUS & CONTROL                                                  ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝
#define STATUS_LED_PIN 2            // Built-in LED on most ESP32 boards
#define ERROR_LED_PIN 2             // Using same pin for now
#define TEST_BUTTON_PIN 17

#endif // PIN_DEFINITIONS_H 