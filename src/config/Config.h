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

#endif // CONFIG_H 