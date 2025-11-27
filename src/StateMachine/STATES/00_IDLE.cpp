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

// External motor objects (declared in main.cpp)
extern StepperMotor* x1Motor;
extern StepperMotor* x2Motor;
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
        Serial.print("Type 'sequence' or 's' to start test sequence (");
        Serial.print(TEST_POSITION_1_X);
        Serial.print(",");
        Serial.print(TEST_POSITION_1_Y);
        Serial.print("->fork->Y+");
        Serial.print(TEST_Y_MOVE_UP_DISTANCE);
        Serial.print("->fork->");
        Serial.print(TEST_POSITION_2_X);
        Serial.print(",");
        Serial.print(TEST_POSITION_2_Y);
        Serial.print("->fork->Y-");
        Serial.print(TEST_Y_MOVE_DOWN_DISTANCE);
        Serial.print("->fork->");
        Serial.print(TEST_POSITION_FINAL_X);
        Serial.print(",");
        Serial.println(TEST_POSITION_FINAL_Y);
        Serial.println(")");
        Serial.println("Press START button: First press = Move to 3,3, Second press = Home");
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
    
    // Check for start button press (rising edge detection)
    bool currentButtonState = (startButtonBounce.read() == HIGH);
    if (currentButtonState && !lastButtonState) {
        // Button was just pressed
        if (!toggleState) {
            // First press: Move to position 3,3
            Serial.println("Start button pressed - Moving to position 3,3");
            toggleState = true;
            return 2; // Transition to MOVE_TO_POSITION state
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