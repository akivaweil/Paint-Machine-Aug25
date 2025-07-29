//* ************************************************************************
//* ************************ TEST SEQUENCE STATE ***************************
//* ************************************************************************
// This state handles the test button sequence:
// 1. Move to 5,5
// 2. Extend fork 3.8 inches
// 3. Move Y up 0.5 inches
// 4. Retract fork to 0
// 5. Move to 10,5
// 6. Extend fork 3.8 inches
// 7. Move Y down 0.5 inches
// 8. Retract fork to 0
// 9. Return to 1,1

#include <Arduino.h>
#include "config/Config.h"
#include "config/Pin_Definitions.h"
#include "StateMachine/FUNCTIONS/StepperMotor.h"

// External motor objects (declared in main.cpp)
extern StepperMotor* x1Motor;
extern StepperMotor* x2Motor;
extern StepperMotor* yMotor;
extern StepperMotor* forkMotor;

// State variables
bool testSequenceInitialized = false;
bool testSequenceActive = false;
int testSequenceStep = 0;
float yStartPosition = 0.0; // Store Y position before moving up/down

// Function to initialize test sequence state
void initializeTestSequenceState() {
    if (!testSequenceInitialized) {
        // Reset sequence variables
        testSequenceActive = false;
        testSequenceStep = 0;
        yStartPosition = 0.0;
        
        Serial.println("=== TEST SEQUENCE STATE INITIALIZED ===");
        Serial.println("Starting test sequence automatically");
        Serial.println("Sequence: 5,5 -> extend fork -> Y+0.5 -> retract fork -> 10,5 -> extend fork -> Y-0.5 -> retract fork -> 1,1");
        Serial.println("=========================================");
        
        // Automatically start the sequence when entering this state
        startTestSequence();
        
        testSequenceInitialized = true;
    }
}

// Function to start the test sequence
void startTestSequence() {
    if (!testSequenceActive) {
        testSequenceActive = true;
        testSequenceStep = 0;
        yStartPosition = yMotor->getCurrentPosition();
        
        Serial.println("=== TEST SEQUENCE STARTED ===");
        Serial.println("Step 1: Moving to position 5,5");
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
        switch (testSequenceStep) {
            case 0:
                //! ************************************************************************
                //! STEP 5: MOVE TO POSITION 5,5
                //! ************************************************************************
                Serial.println("=== STEP 1: Moving to position 5,5 ===");
                x1Motor->moveToPosition(5.0);
                x2Motor->moveToPosition(5.0);
                yMotor->moveToPosition(5.0);
                testSequenceStep++;
                break;
                
            case 1:
                //! ************************************************************************
                //! STEP 6: EXTEND FORK 3.8 INCHES
                //! ************************************************************************
                Serial.println("=== STEP 2: Extending fork 3.8 inches ===");
                forkMotor->moveToPosition(3.8);
                testSequenceStep++;
                break;
                
            case 2:
                //! ************************************************************************
                //! STEP 7: MOVE Y UP 0.5 INCHES
                //! ************************************************************************
                Serial.println("=== STEP 3: Moving Y up 0.5 inches ===");
                float currentY = yMotor->getCurrentPosition();
                yMotor->moveToPosition(currentY + 0.5);
                testSequenceStep++;
                break;
                
            case 3:
                //! ************************************************************************
                //! STEP 8: RETRACT FORK TO 0
                //! ************************************************************************
                Serial.println("=== STEP 4: Retracting fork to 0 ===");
                forkMotor->moveToPosition(0.0);
                testSequenceStep++;
                break;
                
            case 4:
                //! ************************************************************************
                //! STEP 9: MOVE TO POSITION 10,5
                //! ************************************************************************
                Serial.println("=== STEP 5: Moving to position 10,5 ===");
                x1Motor->moveToPosition(10.0);
                x2Motor->moveToPosition(10.0);
                yMotor->moveToPosition(5.0);
                testSequenceStep++;
                break;
                
            case 5:
                //! ************************************************************************
                //! STEP 10: EXTEND FORK 3.8 INCHES
                //! ************************************************************************
                Serial.println("=== STEP 6: Extending fork 3.8 inches ===");
                forkMotor->moveToPosition(3.8);
                testSequenceStep++;
                break;
                
            case 6:
                //! ************************************************************************
                //! STEP 11: MOVE Y DOWN 0.5 INCHES
                //! ************************************************************************
                Serial.println("=== STEP 7: Moving Y down 0.5 inches ===");
                float currentY2 = yMotor->getCurrentPosition();
                yMotor->moveToPosition(currentY2 - 0.5);
                testSequenceStep++;
                break;
                
            case 7:
                //! ************************************************************************
                //! STEP 12: RETRACT FORK TO 0
                //! ************************************************************************
                Serial.println("=== STEP 8: Retracting fork to 0 ===");
                forkMotor->moveToPosition(0.0);
                testSequenceStep++;
                break;
                
            case 8:
                //! ************************************************************************
                //! STEP 13: RETURN TO POSITION 1,1
                //! ************************************************************************
                Serial.println("=== STEP 9: Returning to position 1,1 ===");
                x1Motor->moveToPosition(1.0);
                x2Motor->moveToPosition(1.0);
                yMotor->moveToPosition(1.0);
                testSequenceStep++;
                break;
                
            case 9:
                //! ************************************************************************
                //! STEP 14: SEQUENCE COMPLETE
                //! ************************************************************************
                Serial.println("=== TEST SEQUENCE COMPLETE ===");
                Serial.println("All movements finished successfully");
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
    testSequenceInitialized = false;
    testSequenceActive = false;
    testSequenceStep = 0;
    yStartPosition = 0.0;
} 