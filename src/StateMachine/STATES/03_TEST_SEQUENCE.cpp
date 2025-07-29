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

// Forward declaration
void startTestSequence();

// Function to initialize test sequence state
void initializeTestSequenceState() {
    if (!testSequenceInitialized) {
        // Reset sequence variables
        testSequenceActive = false;
        testSequenceStep = 0;
        yStartPosition = 0.0;
        
        Serial.println("=== TEST SEQUENCE STATE INITIALIZED ===");
        Serial.println("Starting test sequence automatically");
        Serial.print("Sequence: ");
        Serial.print(TEST_POSITION_1_X);
        Serial.print(",");
        Serial.print(TEST_POSITION_1_Y);
        Serial.print(" -> extend fork ");
        Serial.print(TEST_FORK_EXTEND_DISTANCE);
        Serial.print(" -> Y+");
        Serial.print(TEST_Y_MOVE_UP_DISTANCE);
        Serial.print(" -> retract fork -> ");
        Serial.print(TEST_POSITION_2_X);
        Serial.print(",");
        Serial.print(TEST_POSITION_2_Y);
        Serial.print(" -> extend fork ");
        Serial.print(TEST_FORK_EXTEND_DISTANCE);
        Serial.print(" -> Y-");
        Serial.print(TEST_Y_MOVE_DOWN_DISTANCE);
        Serial.print(" -> retract fork -> ");
        Serial.print(TEST_POSITION_FINAL_X);
        Serial.print(",");
        Serial.println(TEST_POSITION_FINAL_Y);
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
        // Variables for Y position calculations
        float currentY, currentY2;
        
        switch (testSequenceStep) {
            case 0:
                //! ************************************************************************
                //! STEP 5: MOVE TO POSITION 1 (FROM CONFIG)
                //! ************************************************************************
                Serial.print("=== STEP 1: Moving to position ");
                Serial.print(TEST_POSITION_1_X);
                Serial.print(",");
                Serial.print(TEST_POSITION_1_Y);
                Serial.println(" ===");
                Serial.print("Current positions - X1: ");
                Serial.print(x1Motor->getCurrentPosition());
                Serial.print(", X2: ");
                Serial.print(x2Motor->getCurrentPosition());
                Serial.print(", Y: ");
                Serial.println(yMotor->getCurrentPosition());
                x1Motor->moveToPosition(TEST_POSITION_1_X);
                x2Motor->moveToPosition(TEST_POSITION_1_X);
                yMotor->moveToPosition(TEST_POSITION_1_Y);
                testSequenceStep++;
                break;
                
            case 1:
                //! ************************************************************************
                //! STEP 6: EXTEND FORK (FROM CONFIG)
                //! ************************************************************************
                Serial.print("=== STEP 2: Extending fork ");
                Serial.print(TEST_FORK_EXTEND_DISTANCE);
                Serial.println(" inches ===");
                forkMotor->moveToPosition(TEST_FORK_EXTEND_DISTANCE);
                testSequenceStep++;
                break;
                
            case 2:
                //! ************************************************************************
                //! STEP 7: MOVE Y UP (FROM CONFIG)
                //! ************************************************************************
                Serial.print("=== STEP 3: Moving Y up ");
                Serial.print(TEST_Y_MOVE_UP_DISTANCE);
                Serial.println(" inches ===");
                currentY = yMotor->getCurrentPosition();
                yMotor->moveToPosition(currentY + TEST_Y_MOVE_UP_DISTANCE);
                testSequenceStep++;
                break;
                
            case 3:
                //! ************************************************************************
                //! STEP 8: RETRACT FORK (FROM CONFIG)
                //! ************************************************************************
                Serial.print("=== STEP 4: Retracting fork to ");
                Serial.print(TEST_FORK_RETRACT_POSITION);
                Serial.println(" ===");
                forkMotor->moveToPosition(TEST_FORK_RETRACT_POSITION);
                testSequenceStep++;
                break;
                
            case 4:
                //! ************************************************************************
                //! STEP 9: MOVE TO POSITION 2 (FROM CONFIG)
                //! ************************************************************************
                Serial.print("=== STEP 5: Moving to position ");
                Serial.print(TEST_POSITION_2_X);
                Serial.print(",");
                Serial.print(TEST_POSITION_2_Y);
                Serial.println(" ===");
                Serial.print("Current positions - X1: ");
                Serial.print(x1Motor->getCurrentPosition());
                Serial.print(", X2: ");
                Serial.print(x2Motor->getCurrentPosition());
                Serial.print(", Y: ");
                Serial.println(yMotor->getCurrentPosition());
                x1Motor->moveToPosition(TEST_POSITION_2_X);
                x2Motor->moveToPosition(TEST_POSITION_2_X);
                yMotor->moveToPosition(TEST_POSITION_2_Y);
                testSequenceStep++;
                break;
                
            case 5:
                //! ************************************************************************
                //! STEP 10: EXTEND FORK (FROM CONFIG)
                //! ************************************************************************
                Serial.print("=== STEP 6: Extending fork ");
                Serial.print(TEST_FORK_EXTEND_DISTANCE);
                Serial.println(" inches ===");
                forkMotor->moveToPosition(TEST_FORK_EXTEND_DISTANCE);
                testSequenceStep++;
                break;
                
            case 6:
                //! ************************************************************************
                //! STEP 11: MOVE Y DOWN (FROM CONFIG)
                //! ************************************************************************
                Serial.print("=== STEP 7: Moving Y down ");
                Serial.print(TEST_Y_MOVE_DOWN_DISTANCE);
                Serial.println(" inches ===");
                currentY2 = yMotor->getCurrentPosition();
                yMotor->moveToPosition(currentY2 - TEST_Y_MOVE_DOWN_DISTANCE);
                testSequenceStep++;
                break;
                
            case 7:
                //! ************************************************************************
                //! STEP 12: RETRACT FORK (FROM CONFIG)
                //! ************************************************************************
                Serial.print("=== STEP 8: Retracting fork to ");
                Serial.print(TEST_FORK_RETRACT_POSITION);
                Serial.println(" ===");
                forkMotor->moveToPosition(TEST_FORK_RETRACT_POSITION);
                testSequenceStep++;
                break;
                
            case 8:
                //! ************************************************************************
                //! STEP 13: RETURN TO FINAL POSITION (FROM CONFIG)
                //! ************************************************************************
                Serial.print("=== STEP 9: Returning to position ");
                Serial.print(TEST_POSITION_FINAL_X);
                Serial.print(",");
                Serial.print(TEST_POSITION_FINAL_Y);
                Serial.println(" ===");
                Serial.print("Current positions - X1: ");
                Serial.print(x1Motor->getCurrentPosition());
                Serial.print(", X2: ");
                Serial.print(x2Motor->getCurrentPosition());
                Serial.print(", Y: ");
                Serial.println(yMotor->getCurrentPosition());
                x1Motor->moveToPosition(TEST_POSITION_FINAL_X);
                x2Motor->moveToPosition(TEST_POSITION_FINAL_X);
                yMotor->moveToPosition(TEST_POSITION_FINAL_Y);
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