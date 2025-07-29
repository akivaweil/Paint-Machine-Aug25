#ifndef HOMING_CONFIG_H
#define HOMING_CONFIG_H

//* ************************************************************************
//* ************************ HOMING CONFIGURATION ***************************
//* ************************************************************************

// X1 Motor Homing Settings
#define X1_HOME_SPEED_INCHES_PER_SEC 2.0      // Speed for X1 homing operation
#define X1_HOME_ACCELERATION_INCHES_PER_SEC2 12.0 // Acceleration for X1 homing

// X2 Motor Homing Settings
#define X2_HOME_SPEED_INCHES_PER_SEC 2.0      // Speed for X2 homing operation
#define X2_HOME_ACCELERATION_INCHES_PER_SEC2 12.0 // Acceleration for X2 homing

// Y Motor Homing Settings
#define Y_HOME_SPEED_INCHES_PER_SEC 2.0       // Speed for Y homing operation
#define Y_HOME_ACCELERATION_INCHES_PER_SEC2 16.0 // Acceleration for Y homing

// Fork Motor Homing Settings
#define FORK_HOME_SPEED_INCHES_PER_SEC 2.0    // Speed for Fork homing operation
#define FORK_HOME_ACCELERATION_INCHES_PER_SEC2 8.0 // Acceleration for Fork homing

// Homing timeout settings
#define HOMING_TIMEOUT_MS 60000            // 60 seconds timeout for entire homing sequence
#define PHASE_TIMEOUT_MS 30000             // 30 seconds timeout per phase

// Homing direction settings (true = positive direction, false = negative direction)
#define X1_HOME_DIRECTION_POSITIVE true    // X1 homes in positive direction
#define X2_HOME_DIRECTION_POSITIVE true    // X2 homes in positive direction
#define Y_HOME_DIRECTION_POSITIVE false    // Y homes in negative direction
#define FORK_HOME_DIRECTION_POSITIVE false  // Fork homes in negative direction

// Homing distance settings (how far to move during homing)
#define X1_HOME_DISTANCE_STEPS 10000       // Steps to move during X1 homing
#define X2_HOME_DISTANCE_STEPS 10000       // Steps to move during X2 homing
#define Y_HOME_DISTANCE_STEPS 8000         // Steps to move during Y homing
#define FORK_HOME_DISTANCE_STEPS 6000      // Steps to move during Fork homing

#endif // HOMING_CONFIG_H 