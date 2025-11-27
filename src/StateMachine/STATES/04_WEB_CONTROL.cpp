#include "StateMachine/STATES/04_WEB_CONTROL.h"
#include "StateMachine/FUNCTIONS/StepperMotor.h"
#include "config/Config.h"
#include "Web_Manager.h"

// External motor references
extern StepperMotor* x1Motor;
extern StepperMotor* x2Motor;
extern StepperMotor* yMotor;

// State variables
static int currentStep = 0;
static bool movementStarted = false;
static float targetX = 0.0;
static float targetY = 0.0;

//* ************************************************************************
//* ************************ WEB CONTROL STATE *****************************
//* ************************************************************************

void resetWebControlState() {
    currentStep = 0;
    movementStarted = false;
    
    // Get targets from Web Manager
    targetX = getWebTargetX();
    targetY = getWebTargetY();
    
    // Clear the request flag now that we've accepted it
    clearWebMoveRequest();
    
    Serial.println("--- ENTERING WEB CONTROL STATE ---");
    Serial.print("Target: X=");
    Serial.print(targetX);
    Serial.print(", Y=");
    Serial.println(targetY);
}

int runWebControlState() {
    
    //! ************************************************************************
    //! STEP 1: VALIDATE COORDINATES
    //! ************************************************************************
    if (currentStep == 0) {
        // Safety checks
        if (targetX < MIN_TRAVEL_INCHES || targetX > MAX_TRAVEL_INCHES) {
            Serial.println("Error: X coordinate out of bounds");
            return 0; // Return to IDLE
        }
        
        if (targetY < MIN_TRAVEL_INCHES || targetY > MAX_TRAVEL_INCHES) {
            Serial.println("Error: Y coordinate out of bounds");
            return 0; // Return to IDLE
        }
        
        currentStep++;
    }
    
    //! ************************************************************************
    //! STEP 2: START MOVEMENT
    //! ************************************************************************
    if (currentStep == 1) {
        if (!movementStarted) {
            Serial.println("Starting movement...");
            
            // Move motors (absolute positioning in inches)
            // Note: StepperMotor class handles inch-to-step conversion
            
            x1Motor->moveToPosition(targetX);
            x2Motor->moveToPosition(targetX);
            yMotor->moveToPosition(targetY);
            
            movementStarted = true;
        }
        
        // Check if all motors have reached their target
        if (!x1Motor->isMoving() && !x2Motor->isMoving() && !yMotor->isMoving()) {
            Serial.println("Movement complete");
            currentStep++;
        }
    }
    
    //! ************************************************************************
    //! STEP 3: RETURN TO IDLE
    //! ************************************************************************
    if (currentStep == 2) {
        Serial.println("--- WEB CONTROL COMPLETE ---");
        return 0; // Return to IDLE
    }
    
    return 4; // Stay in WEB_CONTROL state
}

