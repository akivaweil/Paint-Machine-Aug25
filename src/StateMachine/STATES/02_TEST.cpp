#include <Arduino.h>
#include "StateMachine/STATES/02_TEST.h"
#include "../../config/Config.h"
#include "StateMachine/FUNCTIONS/StepperMotor.h"
#include "Web_Manager.h"

// External motor instances (defined in Web_Manager.cpp)
extern StepperMotor* motorX;
extern StepperMotor* motorY;
extern StepperMotor* motorFork;

// OTA Manager function
extern void updateOTA();

// State machine function
extern void setMachineState(int state);
#define STATE_IDLE 1

// Test position values (set from web interface)
extern float testPos1X;
extern float testPos1Y;
extern float testPos1Fork;
extern float testPos2X;
extern float testPos2Y;
extern float testPos2Fork;

//* ************************************************************************
//* ************************ TEST STATE ***********************************
//* ************************************************************************

void testState() {
    static int step = 0;
    static bool testStarted = false;
    
    // Initialize on first entry
    if (!testStarted) {
        step = 0;
        testStarted = true;
        // Apply motor settings from dashboard before starting test
        applyMotorSettings();
    }
    
    //! ************************************************************************
    //! STEP 1: MOVE TO POSITION 1
    //! ************************************************************************
    if (step == 0) {
        // Get current positions
        float currentX = motorX->stepsToInches(motorX->getCurrentPosition());
        float currentY = motorY->stepsToInches(motorY->getCurrentPosition());
        
        // Calculate target absolute positions (negate because positive direction moves toward home switches)
        float targetX = -testPos1X;
        float targetY = -testPos1Y;
        
        // Calculate relative movement needed to reach absolute position
        float moveX = targetX - currentX;
        float moveY = targetY - currentY;
        
        // Move X and Y simultaneously to absolute position 1
        motorX->moveInches(moveX);
        motorY->moveInches(moveY);
        
        // Wait for both motors to finish
        while (motorX->isMotorRunning() || motorY->isMotorRunning()) {
            updateOTA();
            delay(1);
        }
        step = 1;
    }
    
    //! ************************************************************************
    //! STEP 2: EXTEND FORK AT POSITION 1
    //! ************************************************************************
    else if (step == 1) {
        // Get current fork position
        float currentFork = motorFork->stepsToInches(motorFork->getCurrentPosition());
        
        // Calculate target absolute position (negate because positive direction moves toward home switches)
        float targetFork = -testPos1Fork;
        
        // Calculate relative movement needed to reach absolute position
        float moveFork = targetFork - currentFork;
        
        motorFork->moveInches(moveFork);
        
        // Wait for fork to finish extending
        while (motorFork->isMotorRunning()) {
            updateOTA();
            delay(1);
        }
        step = 2;
    }
    
    //! ************************************************************************
    //! STEP 3: RAISE Y BY 0.5 INCHES AT POSITION 1
    //! ************************************************************************
    else if (step == 2) {
        motorY->moveInches(-0.5);
        
        // Wait for X to finish raising
        while (motorY->isMotorRunning()) {
            updateOTA();
            delay(1);
        }
        step = 3;
    }
    
    //! ************************************************************************
    //! STEP 4: RETRACT FORK AT POSITION 1
    //! ************************************************************************
    else if (step == 3) {
        motorFork->moveInches(testPos1Fork);
        
        // Wait for fork to finish retracting
        while (motorFork->isMotorRunning()) {
            updateOTA();
            delay(1);
        }
        step = 4;
    }
    
    //! ************************************************************************
    //! STEP 5: MOVE TO POSITION 2
    //! ************************************************************************
    else if (step == 4) {
        // Get current positions
        float currentX = motorX->stepsToInches(motorX->getCurrentPosition());
        float currentY = motorY->stepsToInches(motorY->getCurrentPosition());
        
        // Calculate target absolute positions (negate because positive direction moves toward home switches)
        float targetX = -testPos2X;
        float targetY = -testPos2Y;
        
        // Calculate relative movement needed to reach absolute position
        float moveX = targetX - currentX;
        float moveY = targetY - currentY;
        
        // Move X and Y simultaneously to absolute position 2
        motorX->moveInches(moveX);
        motorY->moveInches(moveY);
        
        // Wait for both motors to finish
        while (motorX->isMotorRunning() || motorY->isMotorRunning()) {
            updateOTA();
            delay(1);
        }
        step = 5;
    }
    
    //! ************************************************************************
    //! STEP 6: EXTEND FORK AT POSITION 2
    //! ************************************************************************
    else if (step == 5) {
        // Get current fork position
        float currentFork = motorFork->stepsToInches(motorFork->getCurrentPosition());
        
        // Calculate target absolute position (negate because positive direction moves toward home switches)
        float targetFork = -testPos2Fork;
        
        // Calculate relative movement needed to reach absolute position
        float moveFork = targetFork - currentFork;
        
        motorFork->moveInches(moveFork);
        
        // Wait for fork to finish extending
        while (motorFork->isMotorRunning()) {
            updateOTA();
            delay(1);
        }
        step = 6;
    }
    
    //! ************************************************************************
    //! STEP 7: LOWER Y BY 0.5 INCHES AT POSITION 2
    //! ************************************************************************
    else if (step == 6) {
        motorY->moveInches(0.5);
        
        // Wait for X to finish lowering
        while (motorY->isMotorRunning()) {
            updateOTA();
            delay(1);
        }
        step = 7;
    }
    
    //! ************************************************************************
    //! STEP 8: RETRACT FORK AT POSITION 2
    //! ************************************************************************
    else if (step == 7) {
        motorFork->moveInches(testPos2Fork);
        
        // Wait for fork to finish retracting
        while (motorFork->isMotorRunning()) {
            updateOTA();
            delay(1);
        }
        step = 8;
    }
    
    //! ************************************************************************
    //! STEP 9: RETURN TO HOME POSITION
    //! ************************************************************************
    else if (step == 8) {
        // Get current positions
        float currentX = motorX->stepsToInches(motorX->getCurrentPosition());
        float currentY = motorY->stepsToInches(motorY->getCurrentPosition());
        float currentFork = motorFork->stepsToInches(motorFork->getCurrentPosition());
        
        // Move all axes back to home (0, 0, 0)
        motorX->moveInches(-currentX);
        motorY->moveInches(-currentY);
        motorFork->moveInches(-currentFork);
        
        // Wait for all motors to finish
        while (motorX->isMotorRunning() || motorY->isMotorRunning() || motorFork->isMotorRunning()) {
            updateOTA();
            delay(1);
        }
        
        // Test complete - return to idle
        testStarted = false;
        setMachineState(STATE_IDLE);
    }
}

