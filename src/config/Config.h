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
#define MAX_SPEED_STEPS_PER_SEC 508    // Maximum speed in steps per second (2.0 in/s * 254 steps/in)
#define ACCELERATION_STEPS_PER_SEC2 5080 // Acceleration in steps per second squared (20.0 in/s² * 254 steps/in)

// Home switch configuration (in steps per second)
#define HOME_SPEED_STEPS_PER_SEC 102   // Speed for homing operation (0.4 in/s * 254 steps/in)
#define HOME_ACCELERATION_STEPS_PER_SEC2 2032 // Acceleration for homing (8.0 in/s² * 254 steps/in)

// Safety settings
#define MAX_TRAVEL_INCHES 40.0         // Maximum travel distance in inches
#define MIN_TRAVEL_INCHES 0.0          // Minimum travel distance in inches

// Timing settings
#define MOTOR_TIMEOUT_MS 30000     // Motor operation timeout in milliseconds

#endif // CONFIG_H 