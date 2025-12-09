#ifndef CONFIG_H
#define CONFIG_H

//* ************************************************************************
//* ************************ PAINT MACHINE CONFIG ***************************
//* ************************************************************************

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 🏷️ MACHINE IDENTIFICATION                                            ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝
#define MACHINE_NAME "Paint Machine"
#define MACHINE_VERSION "1.0.0"

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 📡 NETWORK CONFIGURATION                                             ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝
#define WIFI_SSID "Everwood"
#define WIFI_PASSWORD "Everwood-Staff"

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ ⚙️ BASE MOTOR CONFIGURATION                                          ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝
#define STEPPER_STEPS_PER_REV 400      // 400 steps per revolution
#define MICROSTEPPING 1                // No microstepping (full steps)
#define STEPS_PER_INCH 254             // Steps per inch (400 steps / 1.575 inches per rev = 254 steps/inch) - Default for Y, Fork, Storage
#define X_STEPS_PER_INCH 254           // Steps per inch for X motor
#define MAX_SPEED 10000                // Default Maximum speed in steps per second
#define MAX_ACCEL 10000                // Default Maximum acceleration in steps per second squared

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 🔄 X, Y, FORK MOTOR SETTINGS                                         ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝
// NOTE: X, Y, and Fork motor speeds and acceleration are configured on the dashboard
// These values are set to 0 to indicate they should NOT be used from config
// Actual values come from dashboard and are stored in preferences
#define X_MAX_SPEED 0                  // Set on dashboard - DO NOT use from config
#define X_MAX_ACCEL 0                  // Set on dashboard - DO NOT use from config
#define Y_MAX_SPEED 0                  // Set on dashboard - DO NOT use from config
#define Y_MAX_ACCEL 0                  // Set on dashboard - DO NOT use from config
#define Y_SPEED_UP_MULTIPLIER 0.7      // Y-axis speed multiplier when moving up (0.7 = 70% of normal speed to prevent stalling against gravity)
#define FORK_MAX_SPEED 0               // Set on dashboard - DO NOT use from config
#define FORK_MAX_ACCEL 0               // Set on dashboard - DO NOT use from config

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 📦 STORAGE MOTOR SETTINGS                                            ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝
// Storage motor: 1600 steps/rev with 8:24 gear ratio (3:1 reduction)
// 1/6 rev output per click = 1/2 rev motor = 800 steps
#define STORAGE_MOTOR_SPEED 2000              // Storage motor speed for normal operation
#define STORAGE_MOTOR_ACCEL 2000              // Storage motor acceleration for normal operation
#define STORAGE_MOTOR_HOMING_SPEED 500        // Storage motor speed for homing operation
#define STORAGE_MOTOR_HOMING_TRIM 38           // Storage motor homing trim amount in steps (extra distance after reaching home switch)
#define STORAGE_MOTOR_STEPS_PER_CLICK 800      // Storage motor steps per click (1/6 rev output = 800 steps)
#define STORAGE_COLUMN_SPACING_STEPS 762       // Distance between storage columns in steps
#define STORAGE_MOTOR_SENSOR_IGNORE_STEPS 63   // Steps to ignore sensor at start to clear current column

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 🎨 PAINTING CONFIGURATION                                            ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝
#include "Painting_Config.h"

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 🛡️ SAFETY SETTINGS                                                   ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝
#define MAX_TRAVEL_INCHES 14.0         // Maximum travel distance in inches
#define MIN_TRAVEL_INCHES 0.0          // Minimum travel distance in inches

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ ⏱️ TIMING SETTINGS                                                   ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝
#define MOTOR_TIMEOUT_MS 30000         // Motor operation timeout in milliseconds

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 🧪 TEST SEQUENCE SETTINGS                                            ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝
#define POSITION_HEIGHT_SPACING_MM 49.0
#define POSITION_HEIGHT_SPACING_INCHES (POSITION_HEIGHT_SPACING_MM / 25.4)
#define TEST_Y_SPEED_FORK_EXTENDED 7000       // Y-axis speed when fork is extended (steps per second)
#define TEST_Y_ACCEL_FORK_EXTENDED 7000       // Y-axis acceleration when fork is extended (steps per second squared)

#endif // CONFIG_H 