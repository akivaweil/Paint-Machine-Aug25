//* ************************************************************************
//* ************************ TEST POSITION STATE ***************************
//* ************************************************************************
// This state moves the machine to a predefined test position
// Used for testing and calibration
// All test position logic is centralized in this file

#include <Arduino.h>
#include "config/Config.h"
#include "config/Pin_Definitions.h"
#include "StateMachine/FUNCTIONS/StepperMotor.h"
#include "Web_Manager.h"

// External motor objects (declared in main.cpp)
extern StepperMotor* x1Motor;
extern StepperMotor* x2Motor;
extern StepperMotor* yMotor;

// State variables
static bool movementStarted = false;
static int currentStep = 0;

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ ⚔️ TEST POSITION STATE                                               ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

// Check if test position should be triggered (from web request)
bool shouldTriggerTestPosition(int currentState) {
    // Don't interrupt homing state
    if (currentState == 1) {
        return false;
    }
    
    // Check for web test position request
    if (isWebTestPositionRequested()) {
        clearWebTestPositionRequest(); // Clear the flag
        return true;
    }
    
    return false;
}

void resetTestPositionState() {
    movementStarted = false;
    currentStep = 0;
}

void initializeTestPositionState() {
    // Initialize if needed
    if (!movementStarted) {
        resetTestPositionState();
    }
}

int runTestPositionState() {
    
    //! ************************************************************************
    //! STEP 1: UPDATE ALL MOTORS FOR MOVEMENT
    //! ************************************************************************
    x1Motor->update();
    x2Motor->update();
    yMotor->update();
    
    //! ************************************************************************
    //! STEP 2: START MOVEMENT TO TEST POSITION
    //! ************************************************************************
    if (currentStep == 0) {
        if (!movementStarted) {
            // Move to test position (absolute positioning)
            x1Motor->moveToPosition(TEST_POSITION_1_X);
            x2Motor->moveToPosition(TEST_POSITION_1_X);
            yMotor->moveToPosition(TEST_POSITION_1_Y);
            
            movementStarted = true;
        }
        
        // Check if all motors have reached their target
        if (!x1Motor->isMoving() && !x2Motor->isMoving() && !yMotor->isMoving()) {
            currentStep++;
        }
    }
    
    //! ************************************************************************
    //! STEP 3: RETURN TO IDLE
    //! ************************************************************************
    if (currentStep == 1) {
        return 0; // Return to IDLE
    }
    
    return 2; // Stay in TEST_POSITION state
}

int processCommand(String command) {
    // Command processing if needed in future
    return 2;
}

