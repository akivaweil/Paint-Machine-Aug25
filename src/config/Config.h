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

// Speed and acceleration settings
#define MAX_SPEED_INCHES_PER_SEC 2.0    // Maximum speed in inches per second
#define ACCELERATION_INCHES_PER_SEC2 20.0 // Acceleration in inches per second squared

// Home switch configuration
#define HOME_SPEED_INCHES_PER_SEC 0.4   // Speed for homing operation
#define HOME_ACCELERATION_INCHES_PER_SEC2 8.0 // Acceleration for homing

// Safety settings
#define MAX_TRAVEL_INCHES 40.0         // Maximum travel distance in inches
#define MIN_TRAVEL_INCHES 0.0          // Minimum travel distance in inches

// Timing settings
#define MOTOR_TIMEOUT_MS 30000     // Motor operation timeout in milliseconds

#endif // CONFIG_H 