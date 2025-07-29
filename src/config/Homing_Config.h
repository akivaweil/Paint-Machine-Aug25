#ifndef HOMING_CONFIG_H
#define HOMING_CONFIG_H

//* ************************************************************************
//* ************************ HOMING CONFIGURATION ***************************
//* ************************************************************************

// X1 Motor Homing Settings (in steps per second)
#define X1_HOME_SPEED_STEPS_PER_SEC 508      // Speed for X1 homing operation (2.0 in/s * 254 steps/in)
#define X1_HOME_ACCELERATION_STEPS_PER_SEC2 3048 // Acceleration for X1 homing (12.0 in/s² * 254 steps/in)

// X2 Motor Homing Settings (in steps per second)
#define X2_HOME_SPEED_STEPS_PER_SEC 508      // Speed for X2 homing operation (2.0 in/s * 254 steps/in)
#define X2_HOME_ACCELERATION_STEPS_PER_SEC2 3048 // Acceleration for X2 homing (12.0 in/s² * 254 steps/in)

// Y Motor Homing Settings (in steps per second)
#define Y_HOME_SPEED_STEPS_PER_SEC 508       // Speed for Y homing operation (2.0 in/s * 254 steps/in)
#define Y_HOME_ACCELERATION_STEPS_PER_SEC2 4064 // Acceleration for Y homing (16.0 in/s² * 254 steps/in)

// Fork Motor Homing Settings (in steps per second)
#define FORK_HOME_SPEED_STEPS_PER_SEC 508    // Speed for Fork homing operation (2.0 in/s * 254 steps/in)
#define FORK_HOME_ACCELERATION_STEPS_PER_SEC2 2032 // Acceleration for Fork homing (8.0 in/s² * 254 steps/in)

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