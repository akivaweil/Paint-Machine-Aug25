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
#define STEPS_PER_MM 10           // Steps per mm (400 steps / 40mm per rev = 10 steps/mm)

// Speed and acceleration settings
#define MAX_SPEED_MM_PER_SEC 50    // Maximum speed in mm per second
#define ACCELERATION_MM_PER_SEC2 500 // Acceleration in mm per second squared

// Home switch configuration
#define HOME_SPEED_MM_PER_SEC 10   // Speed for homing operation
#define HOME_ACCELERATION_MM_PER_SEC2 200 // Acceleration for homing

// Safety settings
#define MAX_TRAVEL_MM 1000         // Maximum travel distance in mm
#define MIN_TRAVEL_MM 0            // Minimum travel distance in mm

// Timing settings
#define MOTOR_TIMEOUT_MS 30000     // Motor operation timeout in milliseconds

#endif // CONFIG_H 