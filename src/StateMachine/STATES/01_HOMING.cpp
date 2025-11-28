//* ************************************************************************
//* ************************ HOMING STATE **********************************
//* ************************************************************************
// This state handles simultaneous homing of all motors
// Each motor moves away immediately after homing (independently)

#include <Arduino.h>
#include "config/Config.h"
#include "config/Pin_Definitions.h"
#include "config/Homing_Config.h" // Ensure this is included!
#include "StateMachine/FUNCTIONS/StepperMotor.h"

// External motor objects (declared in main.cpp)
extern StepperMotor* xMotor;
extern StepperMotor* yMotor;
extern StepperMotor* forkMotor;

// Forward declaration for OTA manager
void updateOTA();

// State variables
bool homingStateInitialized = false;
int homingPhase = 0; // 0 = homing, 1 = moving away from home

// Individual motor tracking
bool xHomed = false;
bool yHomed = false;
bool forkHomed = false;
bool xMovedAway = false;
bool yMovedAway = false;
bool forkMovedAway = false;

// Function to initialize homing state
void initializeHomingState() {
    if (!homingStateInitialized) {
        // Reset tracking variables for all motors
        xHomed = false;
        yHomed = false;
        forkHomed = false;
        xMovedAway = false;
        yMovedAway = false;
        forkMovedAway = false;
        
        // Start all motors homing simultaneously
        xMotor->home();
        yMotor->home();
        forkMotor->home();
        
        homingPhase = 0;
        homingStateInitialized = true;
    }
}

// Function to run homing state
int runHomingState() {
    // Ensure OTA updates are handled even during homing
    updateOTA();

    // Initialize state if needed
    initializeHomingState();
    
    //! ************************************************************************
    //! STEP 1: CHECK FOR SERIAL COMMANDS (ALLOW SKIP TO TEST)
    //! ************************************************************************
    if (Serial.available()) {
        String command = Serial.readString();
        command.trim();
        command.toLowerCase();
        
        if (command == "m") {
            return 2; // Transition to TEST_POSITION state
        }
    }
    
    //! ************************************************************************
    //! STEP 2: FREQUENT SWITCH UPDATES FOR MAXIMUM RESPONSIVENESS
    //! ************************************************************************
    // Update all motor states during homing (includes switch debouncing)
    xMotor->updateHoming();
    yMotor->updateHoming();
    forkMotor->updateHoming();
    
    //! ************************************************************************
    //! STEP 3: ADDITIONAL SWITCH UPDATES FOR EXTRA RESPONSIVENESS
    //! ************************************************************************
    // Force additional switch updates for maximum responsiveness
    xMotor->updateSwitches();
    yMotor->updateSwitches();
    forkMotor->updateSwitches();
    
    // Handle independent motor homing and moving away
    switch (homingPhase) {
        case 0: // All motors homing simultaneously
            //! ************************************************************************
            //! STEP 4: IMMEDIATE HOME SWITCH CHECKING FOR FASTER RESPONSE
            //! ************************************************************************
            // Check each motor individually for homing completion with immediate response
            // Use direct home switch check and force stop immediately when triggered
            if (!xHomed && xMotor->isHomeSwitchTriggered()) {
                xHomed = true;
                xMotor->forceStop(); // Immediate stop when switch is triggered
                xMotor->setCurrentPositionAsZero();
                // Move X motor away immediately after homing
                xMotor->moveAwayFromHome();
                xMovedAway = true;
            }
            
            if (!yHomed && yMotor->isHomeSwitchTriggered()) {
                yHomed = true;
                yMotor->forceStop(); // Immediate stop when switch is triggered
                yMotor->setCurrentPositionAsZero();
                // Move Y motor away immediately after homing
                yMotor->moveAwayFromHome();
                yMovedAway = true;
            }
            
            if (!forkHomed && forkMotor->isHomeSwitchTriggered()) {
                forkHomed = true;
                forkMotor->forceStop(); // Immediate stop when switch is triggered
                forkMotor->setCurrentPositionAsZero();
                // Fork motor stays at home position - no move away
                forkMovedAway = true; // Mark as moved away even though it didn't move
            }
            
            // Check if all motors have finished homing and moving away
            if (xHomed && yHomed && forkHomed) {
                // Check if all motors have finished moving away (fork stays at home)
                if (!xMotor->isMoving() && !yMotor->isMoving()) {
                    
                    //! ************************************************************************
                    //! STEP 6: RESET COORDINATES TO ZERO
                    //! ************************************************************************
                    // Reset all motor positions to 0.0 after moving away.
                    // This establishes the new 0,0,0 origin.
                    Serial.println("Homing Phase 1 Complete - Resetting all coordinates to zero");
                    xMotor->setCurrentPositionAsZero();
                    yMotor->setCurrentPositionAsZero();
                    forkMotor->setCurrentPositionAsZero();
                    
                    return 0; // Transition to IDLE state
                }
            }
            break;
    }
    
    // Return current state (1 = HOMING)
    return 1;
}

// Function to reset homing state
void resetHomingState() {
    homingStateInitialized = false;
    homingPhase = 0;
    
    // Reset tracking variables
    xHomed = false;
    yHomed = false;
    forkHomed = false;
    xMovedAway = false;
    yMovedAway = false;
    forkMovedAway = false;
}
