#include "StateMachine/WEB_CONTROL/Web_Manual_Control.h"
#include "StateMachine/FUNCTIONS/StepperMotor.h"
#include "Web_Manager.h"
#include <Arduino.h>

// External motor references
extern StepperMotor* xMotor;
extern StepperMotor* yMotor;

// State variables
static int manualStep = 0;
static bool manualMovementStarted = false;
static float manualTargetX = 0.0;
static float manualTargetY = 0.0;

//* ************************************************************************
//* ************************ MANUAL CONTROL *******************************
//* ************************************************************************

void initializeManualControl() {
    manualStep = 0;
    manualMovementStarted = false;
    
    // Get targets from Web Manager
    manualTargetX = getWebTargetX();
    manualTargetY = getWebTargetY();
    
    // Clear the request flag now that we've accepted it
    clearWebMoveRequest();
    
    Serial.println("--- MANUAL CONTROL STARTED ---");
    Serial.print("Target: X=");
    Serial.print(manualTargetX);
    Serial.print(", Y=");
    Serial.println(manualTargetY);
}

int executeManualControl() {
    
    //! ************************************************************************
    //! STEP 1: VALIDATE COORDINATES (HANDLED BY STEPPER MOTOR CLASS)
    //! ************************************************************************
    if (manualStep == 0) {
        // Bounds checking is handled by StepperMotor class
        manualStep++;
    }
    
    //! ************************************************************************
    //! STEP 2: START MOVEMENT (ABSOLUTE)
    //! ************************************************************************
    if (manualStep == 1) {
        if (!manualMovementStarted) {
            Serial.println("Starting movement (Absolute)...");
            
            Serial.print("Moving X to: ");
            Serial.println(manualTargetX);
            Serial.print("Moving Y to: ");
            Serial.println(manualTargetY);
            
            xMotor->moveToPosition(manualTargetX);
            yMotor->moveToPosition(manualTargetY);
            
            manualMovementStarted = true;
        }
        
        // Check if all motors have reached their target
        if (!xMotor->isMoving() && !yMotor->isMoving()) {
            Serial.println("Manual Movement complete");
            manualStep++;
        }
    }
    
    //! ************************************************************************
    //! STEP 3: COMPLETE
    //! ************************************************************************
    if (manualStep == 2) {
        return 0; // Return to IDLE (0) or Complete
    }
    
    return 4; // Stay in WEB_CONTROL state
}

bool isManualMovementComplete() {
    return (manualStep >= 2);
}

