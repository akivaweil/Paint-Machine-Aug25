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
#define STEPS_PER_INCH 254         // Steps per inch (400 steps / 1.575 inches per rev = 254 steps/inch) - Default for Y, Fork, Storage

// X-axis motor specific steps per inch (if different from default)
#define X1_STEPS_PER_INCH 150      // Steps per inch for X1 motor
#define X2_STEPS_PER_INCH 254      // Steps per inch for X2 motor

// Speed and acceleration settings (in steps per second)
#define MAX_SPEED 1000              // Maximum speed in steps per second
#define MAX_ACCEL 3000             // Maximum acceleration in steps per second squared

// Storage motor speed and acceleration settings (in steps per second)
#define STORAGE_MOTOR_SPEED 50              // Storage motor speed for normal operation
#define STORAGE_MOTOR_ACCEL 200              // Storage motor acceleration for normal operation
#define STORAGE_MOTOR_CONTINUOUS_SPEED 50   // Storage motor speed for continuous spinning
// Safety settings
#define MAX_TRAVEL_INCHES 40.0         // Maximum travel distance in inches
#define MIN_TRAVEL_INCHES 0.0          // Minimum travel distance in inches

// Timing settings
#define MOTOR_TIMEOUT_MS 30000     // Motor operation timeout in milliseconds

//* ************************************************************************
//* ************************ TEST SEQUENCE CONFIG ***************************
//* ************************************************************************

// Test sequence positions (in inches) - Adjusted for negative homing direction
#define TEST_POSITION_1_X 1.0      // First position X coordinate (moved away from home)
#define TEST_POSITION_1_Y 10      // First position Y coordinate (moved away from home)
#define TEST_POSITION_2_X 10.0      // Second position X coordinate (reduced from 13.0)
#define TEST_POSITION_2_Y 8      // Second position Y coordinate (moved away from home)
#define TEST_POSITION_FINAL_X 1.0  // Final position X coordinate (moved away from home)
#define TEST_POSITION_FINAL_Y 2.0  // Final position Y coordinate (moved away from home)

// Test sequence fork movements (in inches)
#define TEST_FORK_EXTEND_DISTANCE 3.8  // Distance to extend fork
#define TEST_FORK_RETRACT_POSITION 0.0 // Position to retract fork to

// Test sequence Y movements (in inches)
#define TEST_Y_MOVE_UP_DISTANCE 0.5    // Distance to move Y up
#define TEST_Y_MOVE_DOWN_DISTANCE 0.5  // Distance to move Y down

#endif // CONFIG_H 