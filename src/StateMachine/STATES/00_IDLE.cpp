//* ************************************************************************
//* ************************ IDLE STATE ************************************
//* ************************************************************************
// This state waits for commands and handles transitions to other states
// Commands: "home" or "h" to start homing, "test" or "t" to enter test position state

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
bool idleStateInitialized = false;

// Function to initialize idle state
void initializeIdleState() {
    if (!idleStateInitialized) {
        Serial.println("=== IDLE STATE ===");
        Serial.println("Type 'home' or 'h' to start homing sequence");
        Serial.println("Type 'test' or 't' to enter test position state");
        Serial.println("Type 'm' for manual movement sequence (5,5->10,5->10,10->5,10)");
        Serial.println("Type 'sequence' or 's' to start test sequence (5,5->fork->Y+0.5->fork->10,5->fork->Y-0.5->fork->1,1)");
        Serial.println("================================");
        idleStateInitialized = true;
    }
}

// Function to run idle state
int runIdleState() {
    // Initialize state if needed
    initializeIdleState();
    
    // Check for serial input
    if (Serial.available()) {
        String command = Serial.readString();
        command.trim();
        command.toLowerCase();
        
        if (command == "home" || command == "h") {
            Serial.println("Homing command received - transitioning to homing state");
            return 1; // Transition to HOMING state
        } else if (command == "test" || command == "t") {
            Serial.println("Test position command received - transitioning to test state");
            return 2; // Transition to TEST_POSITION state
        } else if (command == "m") {
            Serial.println("Manual movement command received - transitioning to test state");
            return 2; // Transition to TEST_POSITION state for manual movement
        } else if (command == "sequence" || command == "s") {
            Serial.println("Test sequence command received - transitioning to test sequence state");
            return 3; // Transition to TEST_SEQUENCE state
        } else {
            Serial.println("Unknown command. Type 'home' to start homing, 'test' for test position, 'm' for manual movement, or 'sequence' for test sequence");
        }
    }
    
    // Update all motors (in case they're still moving from previous state)
    if (x1Motor) x1Motor->update();
    if (x2Motor) x2Motor->update();
    if (yMotor) yMotor->update();
    if (forkMotor) forkMotor->update();
    
    // Return current state (0 = IDLE)
    return 0;
}

// Function to reset idle state
void resetIdleState() {
    idleStateInitialized = false;
} 