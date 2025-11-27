//* ************************************************************************
//* ************************ HOMING STATE **********************************
//* ************************************************************************
// This state handles simultaneous homing of all motors
// Phase 0: All motors home simultaneously
// Phase 1: All motors move away from home simultaneously

#include <Arduino.h>
#include "config/Config.h"
#include "config/Pin_Definitions.h"
#include "config/Homing_Config.h" // Ensure this is included!
#include "StateMachine/FUNCTIONS/StepperMotor.h"

// External motor objects (declared in main.cpp)
extern StepperMotor* x1Motor;
extern StepperMotor* x2Motor;
extern StepperMotor* yMotor;
extern StepperMotor* forkMotor;

// Forward declaration for OTA manager
void updateOTA();

// State variables
bool homingStateInitialized = false;
int homingPhase = 0; // 0 = homing, 1 = moving away from home

// Individual motor tracking
bool x1Homed = false;
bool x2Homed = false;
bool yHomed = false;
bool forkHomed = false;
bool x1MovedAway = false;
bool x2MovedAway = false;
bool yMovedAway = false;
bool forkMovedAway = false;

// Function to initialize homing state
void initializeHomingState() {
    if (!homingStateInitialized) {
        // Reset tracking variables for all motors
        x1Homed = false;
        x2Homed = false;
        yHomed = false;
        forkHomed = false;
        x1MovedAway = false;
        x2MovedAway = false;
        yMovedAway = false;
        forkMovedAway = false;
        
        // Start all motors homing simultaneously
        x1Motor->home();
        x2Motor->home();
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
    x1Motor->updateHoming();
    x2Motor->updateHoming();
    yMotor->updateHoming();
    forkMotor->updateHoming();
    
    //! ************************************************************************
    //! STEP 3: ADDITIONAL SWITCH UPDATES FOR EXTRA RESPONSIVENESS
    //! ************************************************************************
    // Force additional switch updates for maximum responsiveness
    x1Motor->updateSwitches();
    x2Motor->updateSwitches();
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
            if (!x1Homed && x1Motor->isHomeSwitchTriggered()) {
                x1Homed = true;
                x1Motor->forceStop(); // Immediate stop when switch is triggered
                x1Motor->setCurrentPositionAsZero();
            }
            
            if (!x2Homed && x2Motor->isHomeSwitchTriggered()) {
                x2Homed = true;
                x2Motor->forceStop(); // Immediate stop when switch is triggered
                x2Motor->setCurrentPositionAsZero();
            }
            
            if (!yHomed && yMotor->isHomeSwitchTriggered()) {
                yHomed = true;
                yMotor->forceStop(); // Immediate stop when switch is triggered
                yMotor->setCurrentPositionAsZero();
            }
            
            if (!forkHomed && forkMotor->isHomeSwitchTriggered()) {
                forkHomed = true;
                forkMotor->forceStop(); // Immediate stop when switch is triggered
                forkMotor->setCurrentPositionAsZero();
            }
            
            //! ************************************************************************
            //! STEP 5: MOVE ALL MOTORS AWAY TOGETHER WHEN ALL ARE HOMED
            //! ************************************************************************
            // Move all motors away together when all are homed (except fork - it stays at home)
            if (x1Homed && x2Homed && yHomed && forkHomed && 
                !x1MovedAway && !x2MovedAway && !yMovedAway && !forkMovedAway) {
                
                // Apply X1 offset: Base move away distance + individual offset
                float x1Distance = MOVE_AWAY_FROM_HOME_DISTANCE + X1_HOME_OFFSET;
                x1Motor->moveAwayFromHome(x1Distance); 
                
                // Apply X2 offset: Base move away distance + individual offset
                float x2Distance = MOVE_AWAY_FROM_HOME_DISTANCE + X2_HOME_OFFSET;
                x2Motor->moveAwayFromHome(x2Distance); 
                
                yMotor->moveAwayFromHome(); // Move standard distance away from home switch
                
                // Fork motor stays at home position - no move away
                x1MovedAway = true;
                x2MovedAway = true;
                yMovedAway = true;
                forkMovedAway = true; // Mark as moved away even though it didn't move
            }
            
            // Check if all motors have finished homing and moving away
            if (x1Homed && x2Homed && yHomed && forkHomed) {
                // Check if all motors have finished moving away (fork stays at home)
                if (!x1Motor->isMoving() && !x2Motor->isMoving() && !yMotor->isMoving()) {
                    
                    //! ************************************************************************
                    //! STEP 6: RESET COORDINATES TO ZERO AT OFFSET POSITION
                    //! ************************************************************************
                    // Reset all motor positions to 0.0 after moving to the offset.
                    // This establishes the new 0,0,0 origin at the offset position.
                    x1Motor->setCurrentPositionAsZero();
                    x2Motor->setCurrentPositionAsZero();
                    yMotor->setCurrentPositionAsZero();
                    
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
    x1Homed = false;
    x2Homed = false;
    yHomed = false;
    forkHomed = false;
    x1MovedAway = false;
    x2MovedAway = false;
    yMovedAway = false;
    forkMovedAway = false;
}
