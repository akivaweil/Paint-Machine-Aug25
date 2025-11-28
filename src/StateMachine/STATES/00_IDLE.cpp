//* ************************************************************************
//* ************************ IDLE STATE ************************************
//* ************************************************************************
// This state waits for commands and handles transitions to other states
// Commands: "home" or "h" to start homing, "test" or "t" to enter test position state

#include <Arduino.h>
#include <Bounce2.h>
#include "config/Config.h"
#include "config/Pin_Definitions.h"
#include "StateMachine/FUNCTIONS/StepperMotor.h"
#include "Web_Manager.h"

// External motor objects (declared in main.cpp)
extern StepperMotor* xMotor;
extern StepperMotor* yMotor;
extern StepperMotor* forkMotor;

// State variables
bool idleStateInitialized = false;
Bounce startButtonBounce;
bool toggleState = false; // false = move to 3,3, true = home
bool lastButtonState = false;

// Function to initialize idle state
void initializeIdleState() {
    if (!idleStateInitialized) {
        // Initialize start button debouncing
        startButtonBounce.attach(TEST_BUTTON_PIN, INPUT_PULLDOWN);
        startButtonBounce.interval(50); // 50ms debounce
        
        Serial.println("=== IDLE STATE ===");
        Serial.println("Type 'home' or 'h' to start homing sequence");
        Serial.println("Type 'test' or 't' to enter test position state");
        Serial.println("Type 'm' for manual movement sequence (5,5->10,5->10,10->5,10)");
        Serial.println("Type 'sequence' or 's' to start pick and place sequence (uses web dashboard settings)");
        Serial.println("Press START button: First press = Pick and Place Sequence, Second press = Home");
        Serial.println("================================");
        idleStateInitialized = true;
    }
}

// Function to run idle state
int runIdleState() {
    // Initialize state if needed
    initializeIdleState();
    
    // Update button debouncing
    startButtonBounce.update();

    //! ************************************************************************
    //! CHECK FOR WEB REQUESTS
    //! ************************************************************************
    if (isWebMoveRequested() || isWebStartRequested() || isWebTestPositionRequested()) {
        return 4; // Transition to WEB_CONTROL state
    }
    
    // Check for start button press (rising edge detection)
    bool currentButtonState = (startButtonBounce.read() == HIGH);
    if (currentButtonState && !lastButtonState) {
        // Button was just pressed
        if (!toggleState) {
            // First press: Start Pick and Place Sequence
            Serial.println("Start button pressed - Starting Pick and Place Sequence");
            toggleState = true;
            return 3; // Transition to PICK_PLACE_SEQUENCE state
        } else {
            // Second press: Home
            Serial.println("Start button pressed - Starting homing sequence");
            toggleState = false;
            return 1; // Transition to HOMING state
        }
    }
    lastButtonState = currentButtonState;
    
    // Check for serial input
    if (Serial.available()) {
        String command = Serial.readString();
        command.trim();
        command.toLowerCase();
        
        if (command == "home" || command == "h") {
            Serial.println("Homing command received - transitioning to homing state");
            toggleState = false;
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
    if (xMotor) xMotor->update();
    if (yMotor) yMotor->update();
    if (forkMotor) forkMotor->update();
    
    // Return current state (0 = IDLE)
    return 0;
}

// Function to reset idle state
void resetIdleState() {
    idleStateInitialized = false;
}
