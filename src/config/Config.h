#ifndef CONFIG_H
#define CONFIG_H

//* ************************************************************************
//* ************************ PAINT MACHINE CONFIG ***************************
//* ************************************************************************

// Machine identification
#define MACHINE_NAME "Paint Machine"
#define MACHINE_VERSION "1.0.0"

// Network configuration
#define WIFI_SSID "Everwood"
#define WIFI_PASSWORD "Everwood-Staff"

// Motor configuration
#define STEPPER_STEPS_PER_REV 400  // 400 steps per revolution
#define MICROSTEPPING 1            // No microstepping (full steps)
#define STEPS_PER_INCH 254         // Steps per inch (400 steps / 1.575 inches per rev = 254 steps/inch)

// Speed and acceleration settings (in steps per second)
#define MAX_SPEED 30000              // Maximum speed in steps per second
#define MAX_ACCEL 10000             // Maximum acceleration in steps per second squared

// Home switch configuration (in steps per second)
#define HOME_SPEED 500             // Speed for homing operation
#define HOME_ACCEL 2000            // Acceleration for homing

// Safety settings
#define MAX_TRAVEL_INCHES 40.0         // Maximum travel distance in inches
#define MIN_TRAVEL_INCHES 0.0          // Minimum travel distance in inches

// Timing settings
#define MOTOR_TIMEOUT_MS 30000     // Motor operation timeout in milliseconds

//* ************************************************************************
//* ************************ TEST SEQUENCE CONFIG ***************************
//* ************************************************************************

// Test sequence positions (in inches)
#define TEST_POSITION_1_X 2.0      // First position X coordinate
#define TEST_POSITION_1_Y 5.0      // First position Y coordinate
#define TEST_POSITION_2_X 13.0     // Second position X coordinate
#define TEST_POSITION_2_Y 5.0      // Second position Y coordinate
#define TEST_POSITION_FINAL_X 1.0  // Final position X coordinate
#define TEST_POSITION_FINAL_Y 1.0  // Final position Y coordinate

// Test sequence fork movements (in inches)
#define TEST_FORK_EXTEND_DISTANCE 3.8  // Distance to extend fork
#define TEST_FORK_RETRACT_POSITION 0.0 // Position to retract fork to

// Test sequence Y movements (in inches)
#define TEST_Y_MOVE_UP_DISTANCE 0.5    // Distance to move Y up
#define TEST_Y_MOVE_DOWN_DISTANCE 0.5  // Distance to move Y down

#endif // CONFIG_H 