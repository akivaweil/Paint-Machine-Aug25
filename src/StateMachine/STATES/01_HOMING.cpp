//* ************************************************************************
//* ************************ HOMING STATE **********************************
//* ************************************************************************
// This state handles simultaneous homing of all motors
// Phase 0: All motors home simultaneously
// Phase 1: All motors move away from home simultaneously

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
        Serial.println("=== HOMING STATE ===");
        Serial.println("Starting homing sequence - All motors simultaneously");
        
        // Reset tracking variables
        x1Homed = false;
        x2Homed = false;
        yHomed = false;
        forkHomed = false;
        x1MovedAway = false;
        x2MovedAway = false;
        yMovedAway = false;
        forkMovedAway = false;
        
        // Start all motors homing
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
    // Initialize state if needed
    initializeHomingState();
    
    //! ************************************************************************
    //! STEP 1: FREQUENT SWITCH UPDATES FOR MAXIMUM RESPONSIVENESS
    //! ************************************************************************
    // Update all motor states during homing (includes switch debouncing)
    x1Motor->updateHoming();
    x2Motor->updateHoming();
    yMotor->updateHoming();
    forkMotor->updateHoming();
    
    //! ************************************************************************
    //! STEP 2: ADDITIONAL SWITCH UPDATES FOR EXTRA RESPONSIVENESS
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
            //! STEP 1: IMMEDIATE HOME SWITCH CHECKING FOR FASTER RESPONSE
            //! ************************************************************************
            // Check each motor individually for homing completion with immediate response
            // Use direct home switch check and force stop immediately when triggered
            if (!x1Homed && x1Motor->isHomeSwitchTriggered()) {
                x1Homed = true;
                x1Motor->forceStop(); // Immediate stop when switch is triggered
                x1Motor->setCurrentPositionAsZero();
                Serial.println("X1 motor homed - IMMEDIATE STOP");
            }
            
            if (!x2Homed && x2Motor->isHomeSwitchTriggered()) {
                x2Homed = true;
                x2Motor->forceStop(); // Immediate stop when switch is triggered
                x2Motor->setCurrentPositionAsZero();
                Serial.println("X2 motor homed - IMMEDIATE STOP");
            }
            
            if (!yHomed && yMotor->isHomeSwitchTriggered()) {
                yHomed = true;
                yMotor->forceStop(); // Immediate stop when switch is triggered
                yMotor->setCurrentPositionAsZero();
                Serial.println("Y motor homed - IMMEDIATE STOP");
                yMotor->moveAwayFromHome();
            }
            
            if (!forkHomed && forkMotor->isHomeSwitchTriggered()) {
                forkHomed = true;
                forkMotor->forceStop(); // Immediate stop when switch is triggered
                forkMotor->setCurrentPositionAsZero();
                Serial.println("Fork motor homed - IMMEDIATE STOP");
                forkMotor->moveAwayFromHome();
            }
            
            //! ************************************************************************
            //! STEP 2: MOVE X MOTORS AWAY TOGETHER WHEN BOTH ARE HOMED
            //! ************************************************************************
            // Move X motors away together when both are homed
            if (x1Homed && x2Homed && !x1MovedAway && !x2MovedAway) {
                Serial.println("Both X motors homed - moving away 2.0 inches from home switch");
                x1Motor->moveAwayFromHome(); // Move 2.0 inches away from home switch
                x2Motor->moveAwayFromHome(); // Move 2.0 inches away from home switch
                x1MovedAway = true;
                x2MovedAway = true;
            }
            
            // Check if all motors have finished homing and moving away
            if (x1Homed && x2Homed && yHomed && forkHomed) {
                // Check if all motors have finished moving away
                if (!x1Motor->isMoving() && !x2Motor->isMoving() && !yMotor->isMoving() && !forkMotor->isMoving()) {
                    Serial.println("All motors homed and moved away successfully - returning to IDLE state");
                    return 0; // Transition to IDLE state
                }
            } else {
                // Debug: Show which motors are not yet homed
                if (!x1Homed) Serial.println("DEBUG: X1 not yet homed");
                if (!x2Homed) Serial.println("DEBUG: X2 not yet homed");
                if (!yHomed) Serial.println("DEBUG: Y not yet homed");
                if (!forkHomed) Serial.println("DEBUG: Fork not yet homed");
            }
            
            // Provide status updates
            static unsigned long lastStatusTime = 0;
            if (millis() - lastStatusTime > 1000) {
                Serial.print("Homing - X1: ");
                if (x1Homed && x1MovedAway) {
                    Serial.print(x1Motor->isMoving() ? "MOVING_AWAY" : "COMPLETE");
                } else if (x1Homed) {
                    Serial.print("HOMED_WAITING");
                } else if (x1Motor->isHomeSwitchTriggered()) {
                    Serial.print(x1Motor->isMoving() ? "AT_HOME_MOVING" : "HOMED");
                } else {
                    Serial.print("MOVING");
                }
                Serial.print(", X2: ");
                if (x2Homed && x2MovedAway) {
                    Serial.print(x2Motor->isMoving() ? "MOVING_AWAY" : "COMPLETE");
                } else if (x2Homed) {
                    Serial.print("HOMED_WAITING");
                } else if (x2Motor->isHomeSwitchTriggered()) {
                    Serial.print(x2Motor->isMoving() ? "AT_HOME_MOVING" : "HOMED");
                } else {
                    Serial.print("MOVING");
                }
                Serial.print(", Y: ");
                if (yHomed) {
                    Serial.print(yMotor->isMoving() ? "MOVING_AWAY" : "COMPLETE");
                } else if (yMotor->isHomeSwitchTriggered()) {
                    Serial.print(yMotor->isMoving() ? "AT_HOME_MOVING" : "HOMED");
                } else {
                    Serial.print("MOVING");
                }
                Serial.print(", Fork: ");
                if (forkHomed) {
                    Serial.print(forkMotor->isMoving() ? "MOVING_AWAY" : "COMPLETE");
                } else if (forkMotor->isHomeSwitchTriggered()) {
                    Serial.print(forkMotor->isMoving() ? "AT_HOME_MOVING" : "HOMED");
                } else {
                    Serial.print("MOVING");
                }
                Serial.println();
                lastStatusTime = millis();
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