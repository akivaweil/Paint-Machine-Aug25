//* ************************************************************************
//* ************************ TEST SEQUENCE STATE ***************************
//* ************************************************************************
// This state handles the test button sequence:
// 1. Move to Pick X, Pick Y (from Web Config)
// 2. Extend fork Pick Fork Distance (from Web Config)
// 3. Move Y up (Hardcoded lift distance for now)
// 4. Retract fork
// 5. Move to Place X, Place Y (from Web Config)
// 6. Extend fork Place Fork Distance (from Web Config)
// 7. Move Y down
// 8. Retract fork
// 9. Return to IDLE

#include <Arduino.h>
#include "config/Config.h"
#include "config/Pin_Definitions.h"
#include "StateMachine/FUNCTIONS/StepperMotor.h"
#include "Web_Manager.h" // Include Web Manager to get config

// External motor objects (declared in main.cpp)
extern StepperMotor* x1Motor;
extern StepperMotor* x2Motor;
extern StepperMotor* yMotor;
extern StepperMotor* forkMotor;

// State variables
bool testSequenceInitialized = false;
bool testSequenceActive = false;
int testSequenceStep = 0;
float yPositionBeforeUp = 0.0;    // Store Y position before moving up
float yPositionBeforeDown = 0.0;  // Store Y position before moving down

// Configuration variables (loaded from Web Manager on start)
float cfgPickX = 0.0;
float cfgPickY = 0.0;
float cfgPlaceX = 0.0;
float cfgPlaceY = 0.0;
float cfgPickForkDist = 0.0;
float cfgPlaceForkDist = 0.0;

// Forward declaration
void startTestSequence();

// Function to initialize test sequence state
void initializeTestSequenceState() {
    if (!testSequenceInitialized) {
        // Only reset if not already active
        if (!testSequenceActive) {
            // Reset sequence variables
            testSequenceActive = false;
            testSequenceStep = 0;
            yPositionBeforeUp = 0.0;
            yPositionBeforeDown = 0.0;
            
            // Load configuration from Web Manager
            cfgPickX = getWebPickX();
            cfgPickY = getWebPickY();
            cfgPlaceX = getWebPlaceX();
            cfgPlaceY = getWebPlaceY();
            cfgPickForkDist = getWebPickForkDistance();
            cfgPlaceForkDist = getWebPlaceForkDistance();
            
            // Fallback to defaults if 0 (optional, but safer to just use what's there or print warning)
            if (cfgPickX == 0 && cfgPickY == 0 && cfgPlaceX == 0 && cfgPlaceY == 0) {
                Serial.println("WARNING: No web configuration loaded. Using 0,0.");
            }
            
            Serial.println("=== TEST SEQUENCE STATE INITIALIZED ===");
            Serial.println("Test sequence ready - press test button to start");
            Serial.println("Sequence Configuration:");
            Serial.print("Pick: "); Serial.print(cfgPickX); Serial.print(", "); Serial.println(cfgPickY);
            Serial.print("Pick Fork Dist: "); Serial.println(cfgPickForkDist);
            Serial.print("Place: "); Serial.print(cfgPlaceX); Serial.print(", "); Serial.println(cfgPlaceY);
            Serial.print("Place Fork Dist: "); Serial.println(cfgPlaceForkDist);
            Serial.println("=========================================");
        }
        
        testSequenceInitialized = true;
    }
}

// Function to start the test sequence
void startTestSequence() {
    if (!testSequenceActive) {
        // Refresh config just in case
        cfgPickX = getWebPickX();
        cfgPickY = getWebPickY();
        cfgPlaceX = getWebPlaceX();
        cfgPlaceY = getWebPlaceY();
        cfgPickForkDist = getWebPickForkDistance();
        cfgPlaceForkDist = getWebPlaceForkDistance();
        
        testSequenceActive = true;
        testSequenceStep = 0;
        
        Serial.println("=== TEST SEQUENCE STARTED ===");
        Serial.println("Step 1: Moving to Pick position");
    }
}

// Function to run test sequence state
int runTestSequenceState() {
    // Initialize state if needed
    initializeTestSequenceState();
    
    //! ************************************************************************
    //! STEP 1: UPDATE ALL MOTORS FOR MOVEMENT
    //! ************************************************************************
    x1Motor->update();
    x2Motor->update();
    yMotor->update();
    forkMotor->update();
    
    //! ************************************************************************
    //! STEP 2: CHECK IF TEST SEQUENCE IS ACTIVE
    //! ************************************************************************
    if (!testSequenceActive) {
        return 3; // Stay in TEST_SEQUENCE state
    }
    
    //! ************************************************************************
    //! STEP 3: CHECK IF MOTORS ARE STILL MOVING
    //! ************************************************************************
    bool motorsMoving = x1Motor->isMoving() || x2Motor->isMoving() || yMotor->isMoving() || forkMotor->isMoving();
    
    //! ************************************************************************
    //! STEP 4: EXECUTE NEXT MOVEMENT ONLY IF MOTORS HAVE STOPPED
    //! ************************************************************************
    if (!motorsMoving) {
        // Variables for Y position calculations
        float newYPosition, newYPositionDown;
        
        switch (testSequenceStep) {
            case 0:
                //! ************************************************************************
                //! STEP 5: MOVE TO PICK POSITION
                //! ************************************************************************
                Serial.print("=== STEP 1: Moving to Pick ");
                Serial.print(cfgPickX);
                Serial.print(",");
                Serial.print(cfgPickY);
                Serial.println(" ===");
                
                x1Motor->moveToPosition(cfgPickX);
                x2Motor->moveToPosition(cfgPickX);
                yMotor->moveToPosition(cfgPickY);
                testSequenceStep++;
                break;
                
            case 1:
                //! ************************************************************************
                //! STEP 6: EXTEND FORK (PICK)
                //! ************************************************************************
                Serial.print("=== STEP 2: Extending fork (Pick) ");
                Serial.print(cfgPickForkDist);
                Serial.println(" inches ===");
                
                forkMotor->moveToPosition(cfgPickForkDist);
                testSequenceStep++;
                break;
                
            case 2:
                //! ************************************************************************
                //! STEP 7: MOVE Y UP (LIFT)
                //! ************************************************************************
                Serial.print("=== STEP 3: Moving Y up (Lift) ");
                Serial.print(TEST_Y_MOVE_UP_DISTANCE);
                Serial.println(" inches ===");
                
                yPositionBeforeUp = yMotor->getCurrentPosition();
                newYPosition = yPositionBeforeUp + TEST_Y_MOVE_UP_DISTANCE;
                yMotor->moveToPosition(newYPosition);
                testSequenceStep++;
                break;
                
            case 3:
                //! ************************************************************************
                //! STEP 8: RETRACT FORK
                //! ************************************************************************
                Serial.print("=== STEP 4: Retracting fork to 0 ===");
                
                forkMotor->moveToPosition(0); // Retract to 0 (Home)
                testSequenceStep++;
                break;
                
            case 4:
                //! ************************************************************************
                //! STEP 9: MOVE TO PLACE POSITION
                //! ************************************************************************
                Serial.print("=== STEP 5: Moving to Place ");
                Serial.print(cfgPlaceX);
                Serial.print(",");
                Serial.print(cfgPlaceY);
                Serial.println(" ===");
                
                x1Motor->moveToPosition(cfgPlaceX);
                x2Motor->moveToPosition(cfgPlaceX);
                yMotor->moveToPosition(cfgPlaceY);
                testSequenceStep++;
                break;
                
            case 5:
                //! ************************************************************************
                //! STEP 10: EXTEND FORK (PLACE)
                //! ************************************************************************
                Serial.print("=== STEP 6: Extending fork (Place) ");
                Serial.print(cfgPlaceForkDist);
                Serial.println(" inches ===");
                
                forkMotor->moveToPosition(cfgPlaceForkDist);
                testSequenceStep++;
                break;
                
            case 6:
                //! ************************************************************************
                //! STEP 11: MOVE Y DOWN (DROP)
                //! ************************************************************************
                Serial.print("=== STEP 7: Moving Y down (Drop) ");
                Serial.print(TEST_Y_MOVE_DOWN_DISTANCE);
                Serial.println(" inches ===");
                
                yPositionBeforeDown = yMotor->getCurrentPosition();
                newYPositionDown = yPositionBeforeDown - TEST_Y_MOVE_DOWN_DISTANCE;
                yMotor->moveToPosition(newYPositionDown);
                testSequenceStep++;
                break;
                
            case 7:
                //! ************************************************************************
                //! STEP 12: RETRACT FORK
                //! ************************************************************************
                Serial.print("=== STEP 8: Retracting fork to 0 ===");
                
                forkMotor->moveToPosition(0); // Retract to 0
                testSequenceStep++;
                break;
                
            case 8:
                //! ************************************************************************
                //! STEP 13: SEQUENCE COMPLETE
                //! ************************************************************************
                Serial.println("=== TEST SEQUENCE COMPLETE ===");
                Serial.println("Returning to IDLE state");
                
                testSequenceActive = false;
                testSequenceStep = 0;
                return 0; // Return to IDLE state
                break;
        }
    }
    
    // Return current state (3 = TEST_SEQUENCE)
    return 3;
}

// Function to reset test sequence state
void resetTestSequenceState() {
    // Only reset if sequence is not currently active
    if (!testSequenceActive) {
        testSequenceInitialized = false;
        testSequenceActive = false;
        testSequenceStep = 0;
        yPositionBeforeUp = 0.0;
        yPositionBeforeDown = 0.0;
    }
}
