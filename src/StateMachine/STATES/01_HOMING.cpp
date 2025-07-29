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

// Function to initialize homing state
void initializeHomingState() {
    if (!homingStateInitialized) {
        Serial.println("=== HOMING STATE ===");
        Serial.println("Starting homing sequence - All motors simultaneously");
        
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
    
    // Update all motor states during homing
    x1Motor->updateHoming();
    x2Motor->updateHoming();
    yMotor->updateHoming();
    forkMotor->updateHoming();
    
    // Declare variables for homing status
    bool x1Homed, x2Homed, yHomed, forkHomed;
    
    // Handle different homing phases
    switch (homingPhase) {
        case 0: // All motors homing simultaneously
            // Check if all motors have reached home switches AND stopped moving
            x1Homed = x1Motor->isHomingComplete();
            x2Homed = x2Motor->isHomingComplete();
            yHomed = yMotor->isHomingComplete();
            forkHomed = forkMotor->isHomingComplete();
            
            if (x1Homed && x2Homed && yHomed && forkHomed) {
                // All motors have reached home switches and stopped
                x1Motor->forceStop();
                x2Motor->forceStop();
                yMotor->forceStop();
                forkMotor->forceStop();
                x1Motor->setCurrentPositionAsZero();
                x2Motor->setCurrentPositionAsZero();
                yMotor->setCurrentPositionAsZero();
                forkMotor->setCurrentPositionAsZero();
                
                Serial.println("All motors homing complete - moving away from home");
                homingPhase = 1;
                x1Motor->moveAwayFromHome();
                x2Motor->moveAwayFromHome();
                yMotor->moveAwayFromHome();
                forkMotor->moveAwayFromHome();
            } else {
                // Still homing - provide status updates
                static unsigned long lastStatusTime = 0;
                if (millis() - lastStatusTime > 1000) {
                    Serial.print("Homing - X1: ");
                    if (x1Motor->isHomeSwitchTriggered()) {
                        Serial.print(x1Motor->isMoving() ? "AT_HOME_MOVING" : "HOMED");
                    } else {
                        Serial.print("MOVING");
                    }
                    Serial.print(", X2: ");
                    if (x2Motor->isHomeSwitchTriggered()) {
                        Serial.print(x2Motor->isMoving() ? "AT_HOME_MOVING" : "HOMED");
                    } else {
                        Serial.print("MOVING");
                    }
                    Serial.print(", Y: ");
                    if (yMotor->isHomeSwitchTriggered()) {
                        Serial.print(yMotor->isMoving() ? "AT_HOME_MOVING" : "HOMED");
                    } else {
                        Serial.print("MOVING");
                    }
                    Serial.print(", Fork: ");
                    if (forkMotor->isHomeSwitchTriggered()) {
                        Serial.println(forkMotor->isMoving() ? "AT_HOME_MOVING" : "HOMED");
                    } else {
                        Serial.println("MOVING");
                    }
                    lastStatusTime = millis();
                }
            }
            break;
            
        case 1: // All motors moving away from home
            // Check if all motors have finished moving away
            if (!x1Motor->isMoving() && !x2Motor->isMoving() && !yMotor->isMoving() && !forkMotor->isMoving()) {
                Serial.println("All motors moved away from home");
                Serial.println("All motors homed successfully - returning to IDLE state");
                
                // Return to idle state
                return 0; // Transition to IDLE state
            } else {
                // Still moving away from home - provide status updates
                static unsigned long lastMoveTime = 0;
                if (millis() - lastMoveTime > 1000) {
                    Serial.print("Moving away - X1: ");
                    Serial.print(x1Motor->isMoving() ? "MOVING" : "STOPPED");
                    Serial.print(", X2: ");
                    Serial.print(x2Motor->isMoving() ? "MOVING" : "STOPPED");
                    Serial.print(", Y: ");
                    Serial.print(yMotor->isMoving() ? "MOVING" : "STOPPED");
                    Serial.print(", Fork: ");
                    Serial.println(forkMotor->isMoving() ? "MOVING" : "STOPPED");
                    lastMoveTime = millis();
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
} 