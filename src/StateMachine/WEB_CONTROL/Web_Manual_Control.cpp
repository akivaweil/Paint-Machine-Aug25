#include "StateMachine/WEB_CONTROL/Web_Manual_Control.h"
#include "StateMachine/FUNCTIONS/StepperMotor.h"
#include "Web_Manager.h"
#include <Arduino.h>

// External motor references
extern StepperMotor* x1Motor;
extern StepperMotor* x2Motor;
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
    //! STEP 1: VALIDATE COORDINATES (SKIPPED FOR RELATIVE MOVEMENT)
    //! ************************************************************************
    if (manualStep == 0) {
        // Bounds checking is handled by StepperMotor class
        manualStep++;
    }
    
    //! ************************************************************************
    //! STEP 2: START MOVEMENT (RELATIVE)
    //! ************************************************************************
    if (manualStep == 1) {
        if (!manualMovementStarted) {
            Serial.println("Starting movement (Relative)...");
            
            Serial.print("Moving X relative by: ");
            Serial.println(manualTargetX);
            Serial.print("Moving Y relative by: ");
            Serial.println(manualTargetY);
            
            x1Motor->moveRelative(manualTargetX);
            x2Motor->moveRelative(manualTargetX);
            yMotor->moveRelative(manualTargetY);
            
            manualMovementStarted = true;
        }
        
        // Check if all motors have reached their target
        if (!x1Motor->isMoving() && !x2Motor->isMoving() && !yMotor->isMoving()) {
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

