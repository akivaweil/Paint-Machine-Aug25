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
#define X_STEPS_PER_INCH 254      // Steps per inch for X motor

// Speed and acceleration settings (in steps per second)
#define MAX_SPEED 10000              // Default Maximum speed in steps per second
#define MAX_ACCEL 10000             // Default Maximum acceleration in steps per second squared

// X Motor Settings
// NOTE: X, Y, and Fork motor speeds and acceleration are configured on the dashboard
// These values are set to 0 to indicate they should NOT be used from config
// Actual values come from dashboard and are stored in preferences
#define X_MAX_SPEED 0               // Set on dashboard - DO NOT use from config
#define X_MAX_ACCEL 0               // Set on dashboard - DO NOT use from config

// Y Motor Settings
#define Y_MAX_SPEED 0               // Set on dashboard - DO NOT use from config
#define Y_MAX_ACCEL 0               // Set on dashboard - DO NOT use from config

// Fork Motor Settings
#define FORK_MAX_SPEED 0            // Set on dashboard - DO NOT use from config
#define FORK_MAX_ACCEL 0            // Set on dashboard - DO NOT use from config

// Storage motor speed and acceleration settings (in steps per second)
#define STORAGE_MOTOR_SPEED 5000              // Storage motor speed for normal operation
#define STORAGE_MOTOR_ACCEL 5000              // Storage motor acceleration for normal operation
#define STORAGE_MOTOR_CONTINUOUS_SPEED 5000   // Storage motor speed for continuous spinning
#define STORAGE_MOTOR_STEPS_PER_CLICK 120000    // Storage motor steps per click in web dashboard
// Safety settings
#define MAX_TRAVEL_INCHES 14.0         // Maximum travel distance in inches
#define MIN_TRAVEL_INCHES 0.0          // Minimum travel distance in inches

// Timing settings
#define MOTOR_TIMEOUT_MS 30000     // Motor operation timeout in milliseconds


#endif // CONFIG_H 