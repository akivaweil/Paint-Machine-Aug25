//* ************************************************************************
//* ************************ MOVE TO POSITION STATE ***********************
//* ************************************************************************
// This state moves the X and Y motors to position 3,3

#include <Arduino.h>
#include "config/Config.h"
#include "config/Pin_Definitions.h"
#include "StateMachine/FUNCTIONS/StepperMotor.h"

// External motor objects (declared in main.cpp)
extern StepperMotor* xMotor;
extern StepperMotor* yMotor;

// State variables
bool moveToPositionStateInitialized = false;

// Target position
#define TARGET_X 5.0
#define TARGET_Y 5.0

// Function to initialize move to position state
void initializeMoveToPositionState() {
    if (!moveToPositionStateInitialized) {
        // Move X to position 3.0
        xMotor->moveToPosition(TARGET_X);
        
        // Move Y to position 3.0
        yMotor->moveToPosition(TARGET_Y);
        
        moveToPositionStateInitialized = true;
    }
}

// Function to run move to position state
int runMoveToPositionState() {
    // Initialize state if needed
    initializeMoveToPositionState();
    
    // Update all motors
    if (xMotor) xMotor->update();
    if (yMotor) yMotor->update();
    
    // Check if all motors have finished moving
    if (!xMotor->isMoving() && !yMotor->isMoving()) {
        return 0; // Transition to IDLE state
    }
    
    // Return current state (2 = MOVE_TO_POSITION)
    return 2;
}

// Function to reset move to position state
void resetMoveToPositionState() {
    moveToPositionStateInitialized = false;
}
