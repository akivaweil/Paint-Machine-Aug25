#ifndef HOMING_CONFIG_H
#define HOMING_CONFIG_H

//* ************************************************************************
//* ************************ HOMING CONFIGURATION ***************************
//* ************************************************************************

// X1 Motor Homing Settings (in steps per second)
#define X1_HOME_SPEED 800          // Speed for X1 homing operation (reduced for better switch response)
#define X1_HOME_ACCEL 2500         // Acceleration for X1 homing (reduced for smoother approach)

// X2 Motor Homing Settings (in steps per second)
#define X2_HOME_SPEED 800          // Speed for X2 homing operation (reduced for better switch response)
#define X2_HOME_ACCEL 2500         // Acceleration for X2 homing (reduced for smoother approach)

// Y Motor Homing Settings (in steps per second)
#define Y_HOME_SPEED 800           // Speed for Y homing operation (reduced for better switch response)
#define Y_HOME_ACCEL 3000          // Acceleration for Y homing (reduced for smoother approach)

// Fork Motor Homing Settings (in steps per second)
#define FORK_HOME_SPEED 800        // Speed for Fork homing operation (reduced for better switch response)
#define FORK_HOME_ACCEL 1500       // Acceleration for Fork homing (reduced for smoother approach)

// Storage Motor Settings (in steps per second) - No homing switches
#define STORAGE_HOME_SPEED 400     // Speed for Storage motor operation
#define STORAGE_HOME_ACCEL 2000    // Acceleration for Storage motor operation

// Homing timeout settings
#define HOMING_TIMEOUT_MS 60000            // 60 seconds timeout for entire homing sequence
#define PHASE_TIMEOUT_MS 30000             // 30 seconds timeout per phase

// Home switch debounce settings (in milliseconds)
#define HOME_SWITCH_DEBOUNCE_MS 1         // Debounce time for home switches to prevent false triggers (reduced for faster response)

// Homing direction settings (true = positive direction, false = negative direction)
#define X1_HOME_DIRECTION_POSITIVE false   // X1 homes in negative direction
#define X2_HOME_DIRECTION_POSITIVE false   // X2 homes in negative direction
#define Y_HOME_DIRECTION_POSITIVE false    // Y homes in negative direction
#define FORK_HOME_DIRECTION_POSITIVE false  // Fork homes in negative direction
#define STORAGE_HOME_DIRECTION_POSITIVE true   // Storage motor direction (not used for homing)

// Homing distance settings (how far to move during homing)
#define X1_HOME_DISTANCE_STEPS 10000       // Steps to move during X1 homing
#define X2_HOME_DISTANCE_STEPS 10000       // Steps to move during X2 homing
#define Y_HOME_DISTANCE_STEPS 8000         // Steps to move during Y homing
#define FORK_HOME_DISTANCE_STEPS 6000      // Steps to move during Fork homing
#define STORAGE_HOME_DISTANCE_STEPS 10000  // Steps to move for Storage motor (not used for homing)

// Move away from home switch distance (in inches)
// This is the base distance to move away
#define MOVE_AWAY_FROM_HOME_DISTANCE 2.0   // Distance to move away from home switch after homing

// X-Gantry Offsets (in inches)
// Adjust these values to compensate for physical misalignment of home switches
#define X1_HOME_OFFSET 0.0   // Additional offset for X1 (Reset to 0 to prevent binding)
#define X2_HOME_OFFSET 0.0   // Additional offset for X2

// Move away direction settings (true = positive direction, false = negative direction)
// These should be set based on the physical layout of your machine
#define X1_MOVE_AWAY_DIRECTION_POSITIVE true    // X1 moves positive to get away from home switch
#define X2_MOVE_AWAY_DIRECTION_POSITIVE true    // X2 moves positive to get away from home switch
#define Y_MOVE_AWAY_DIRECTION_POSITIVE true     // Y moves positive to get away from home switch
#define FORK_MOVE_AWAY_DIRECTION_POSITIVE true  // Fork moves positive to get away from home switch
#define STORAGE_MOVE_AWAY_DIRECTION_POSITIVE true  // Storage motor direction (not used for homing)

#endif // HOMING_CONFIG_H
