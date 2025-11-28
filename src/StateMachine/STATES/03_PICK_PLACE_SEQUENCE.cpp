//* ************************************************************************
//* ************************ PICK AND PLACE SEQUENCE STATE ******************
//* ************************************************************************
// This state executes the pick and place sequence using web dashboard settings:
// 1. Move to Pick Location (from web config)
// 2. Extend Fork (pick fork distance from web config)
// 3. Move Y Up (Pick)
// 4. Retract Fork
// 5. Move to Place Location (from web config)
// 6. Extend Fork (place fork distance from web config)
// 7. Move Y Down (Place)
// 8. Retract Fork
// 9. Return to Home Offset Position
// 10. Sequence Complete - Return to IDLE

#include <Arduino.h>
#include "config/Config.h"
#include "config/Pin_Definitions.h"
#include "config/Homing_Config.h"
#include "StateMachine/FUNCTIONS/StepperMotor.h"
#include "Web_Manager.h"

// External motor objects (declared in main.cpp)
extern StepperMotor* xMotor;
extern StepperMotor* yMotor;
extern StepperMotor* forkMotor;

// State variables
bool pickPlaceStateInitialized = false;
int pickPlaceStep = 0;
unsigned long stepStartTime = 0;
const int STEP_DELAY_MS = 500; // Delay between steps

// Pick/Place movement distances (small mechanical movements, not positions)
const float PICK_Y_MOVE_UP_DISTANCE = 0.5;    // Distance to move Y up for pick
const float PLACE_Y_MOVE_DOWN_DISTANCE = 0.5;  // Distance to move Y down for place
const float FORK_RETRACT_POSITION = 0.0;       // Position to retract fork to

// Function to initialize pick place state
void initializePickPlaceState() {
    if (!pickPlaceStateInitialized) {
        pickPlaceStep = 1;
        stepStartTime = 0;
        pickPlaceStateInitialized = true;
        Serial.println("=== STARTING PICK AND PLACE SEQUENCE ===");
    }
}

// Function to run pick place state
int runPickPlaceState() {
    // Initialize state if needed
    initializePickPlaceState();
    
    // Update motors
    if (xMotor) xMotor->update();
    if (yMotor) yMotor->update();
    if (forkMotor) forkMotor->update();

    // Check if motors are moving - if so, wait
    if ((xMotor && xMotor->isMoving()) || 
        (yMotor && yMotor->isMoving()) || 
        (forkMotor && forkMotor->isMoving())) {
        return 3; // Stay in this state
    }

    // Delay handling between steps
    if (stepStartTime == 0) {
        stepStartTime = millis();
    }
    
    if (millis() - stepStartTime < STEP_DELAY_MS) {
        return 3; // Wait for delay
    }

    // Sequence logic
    switch (pickPlaceStep) {
        case 1: // Move to Pick Location
            //! ************************************************************************
            //! STEP 1: MOVE TO PICK LOCATION
            //! ************************************************************************
            Serial.print("Step 1: Moving to Pick Location (");
            Serial.print(getWebPickX());
            Serial.print(", ");
            Serial.print(getWebPickY());
            Serial.println(")");
            
            xMotor->moveToPosition(getWebPickX());
            yMotor->moveToPosition(getWebPickY());
            
            pickPlaceStep++;
            stepStartTime = 0; // Reset delay for next step
            break;

        case 2: // Extend Fork
            //! ************************************************************************
            //! STEP 2: EXTEND FORK
            //! ************************************************************************
            Serial.print("Step 2: Extending Fork to ");
            Serial.println(getWebPickForkDistance());
            
            forkMotor->moveToPosition(getWebPickForkDistance());
            
            pickPlaceStep++;
            stepStartTime = 0;
            break;

        case 3: // Move Y Up (Pick Action)
            //! ************************************************************************
            //! STEP 3: MOVE Y UP (PICK)
            //! ************************************************************************
            Serial.print("Step 3: Moving Y Up by ");
            Serial.println(PICK_Y_MOVE_UP_DISTANCE);
            
            yMotor->moveRelative(PICK_Y_MOVE_UP_DISTANCE);
            
            pickPlaceStep++;
            stepStartTime = 0;
            break;

        case 4: // Retract Fork
            //! ************************************************************************
            //! STEP 4: RETRACT FORK
            //! ************************************************************************
            Serial.print("Step 4: Retracting Fork to ");
            Serial.println(FORK_RETRACT_POSITION);
            
            forkMotor->moveToPosition(FORK_RETRACT_POSITION);
            
            pickPlaceStep++;
            stepStartTime = 0;
            break;

        case 5: // Move to Place Location
            //! ************************************************************************
            //! STEP 5: MOVE TO PLACE LOCATION
            //! ************************************************************************
            Serial.print("Step 5: Moving to Place Location (");
            Serial.print(getWebPlaceX());
            Serial.print(", ");
            Serial.print(getWebPlaceY());
            Serial.println(")");
            
            xMotor->moveToPosition(getWebPlaceX());
            yMotor->moveToPosition(getWebPlaceY());
            
            pickPlaceStep++;
            stepStartTime = 0;
            break;

        case 6: // Extend Fork
            //! ************************************************************************
            //! STEP 6: EXTEND FORK
            //! ************************************************************************
            Serial.print("Step 6: Extending Fork to ");
            Serial.println(getWebPlaceForkDistance());
            
            forkMotor->moveToPosition(getWebPlaceForkDistance());
            
            pickPlaceStep++;
            stepStartTime = 0;
            break;

        case 7: // Move Y Down (Place Action)
            //! ************************************************************************
            //! STEP 7: MOVE Y DOWN (PLACE)
            //! ************************************************************************
            Serial.print("Step 7: Moving Y Down by ");
            Serial.println(PLACE_Y_MOVE_DOWN_DISTANCE);
            
            yMotor->moveRelative(-PLACE_Y_MOVE_DOWN_DISTANCE); // Negative for down
            
            pickPlaceStep++;
            stepStartTime = 0;
            break;

        case 8: // Retract Fork
            //! ************************************************************************
            //! STEP 8: RETRACT FORK
            //! ************************************************************************
            Serial.print("Step 8: Retracting Fork to ");
            Serial.println(FORK_RETRACT_POSITION);
            
            forkMotor->moveToPosition(FORK_RETRACT_POSITION);
            
            pickPlaceStep++;
            stepStartTime = 0;
            break;

        case 9: // Return to Home Offset Position
            //! ************************************************************************
            //! STEP 9: RETURN TO HOME OFFSET POSITION
            //! ************************************************************************
            Serial.print("Step 9: Returning to Home Offset Position (");
            Serial.print(MOVE_AWAY_FROM_HOME_DISTANCE);
            Serial.print(", ");
            Serial.print(MOVE_AWAY_FROM_HOME_DISTANCE);
            Serial.print(", ");
            Serial.print(MOVE_AWAY_FROM_HOME_DISTANCE);
            Serial.println(")");
            
            xMotor->moveToPosition(MOVE_AWAY_FROM_HOME_DISTANCE);
            yMotor->moveToPosition(MOVE_AWAY_FROM_HOME_DISTANCE);
            forkMotor->moveToPosition(MOVE_AWAY_FROM_HOME_DISTANCE);
            
            pickPlaceStep++;
            stepStartTime = 0;
            break;

        case 10: // Sequence Complete
            //! ************************************************************************
            //! SEQUENCE COMPLETE
            //! ************************************************************************
            Serial.println("=== PICK AND PLACE SEQUENCE COMPLETE ===");
            return 0; // Return to IDLE state

        default:
            return 0; // Should not happen, but return to IDLE
    }

    return 3; // Stay in this state
}

// Function to reset pick place state
void resetPickPlaceState() {
    pickPlaceStateInitialized = false;
    pickPlaceStep = 0;
    stepStartTime = 0;
}

