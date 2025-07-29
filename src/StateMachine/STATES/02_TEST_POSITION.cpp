//* ************************************************************************
//* ************************ TEST POSITION STATE ***************************
//* ************************************************************************
// This state allows testing motor movement by inputting coordinates via serial
// Format: "x,y" where x and y are float values in inches
// Example: "5.5,7.2" will move X motors to 5.5 inches and Y motor to 7.2 inches

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
bool testStateInitialized = false;
bool waitingForInput = true;
String inputBuffer = "";

// Forward declaration
int processCommand(String command);

// Function to initialize test state
void initializeTestPositionState() {
    if (!testStateInitialized) {
        Serial.println("=== TEST POSITION STATE ===");
        Serial.println("Enter coordinates in format: x,y");
        Serial.println("Example: 5.5,7.2");
        Serial.println("Type 'exit' to return to IDLE");
        Serial.println("Type 'status' to see current positions");
        Serial.println("================================");
        testStateInitialized = true;
        waitingForInput = true;
        inputBuffer = "";
    }
}

// Function to run test state
int runTestPositionState() {
    // Initialize state if needed
    initializeTestPositionState();
    
    // Check for serial input
    if (Serial.available()) {
        char c = Serial.read();
        
        // Handle backspace
        if (c == '\b' || c == 127) {
            if (inputBuffer.length() > 0) {
                inputBuffer.remove(inputBuffer.length() - 1);
                Serial.print("\b \b"); // Clear the character on screen
            }
        }
        // Handle enter key
        else if (c == '\n' || c == '\r') {
            if (inputBuffer.length() > 0) {
                int newState = processCommand(inputBuffer);
                inputBuffer = "";
                Serial.println(); // New line after command
                if (newState != 2) {
                    // State wants to change
                    return newState;
                }
            }
        }
        // Add character to buffer
        else if (c >= 32 && c <= 126) { // Printable characters only
            inputBuffer += c;
            Serial.print(c); // Echo character
        }
    }
    
    // Update all motors
    if (x1Motor) x1Motor->update();
    if (x2Motor) x2Motor->update();
    if (yMotor) yMotor->update();
    if (forkMotor) forkMotor->update();
    
    // Check if any motors are still moving
    bool anyMoving = false;
    if (x1Motor && x1Motor->isMoving()) anyMoving = true;
    if (x2Motor && x2Motor->isMoving()) anyMoving = true;
    if (yMotor && yMotor->isMoving()) anyMoving = true;
    if (forkMotor && forkMotor->isMoving()) anyMoving = true;
    
    // If motors are moving, show status
    static unsigned long lastStatusTime = 0;
    if (anyMoving && millis() - lastStatusTime > 500) {
        Serial.print("Moving - X1: ");
        Serial.print(x1Motor ? x1Motor->getCurrentPosition() : 0);
        Serial.print(", X2: ");
        Serial.print(x2Motor ? x2Motor->getCurrentPosition() : 0);
        Serial.print(", Y: ");
        Serial.print(yMotor ? yMotor->getCurrentPosition() : 0);
        Serial.print(", Fork: ");
        Serial.println(forkMotor ? forkMotor->getCurrentPosition() : 0);
        lastStatusTime = millis();
    }
    
    // Return current state (2 = TEST_POSITION)
    return 2;
}

// Function to process commands
int processCommand(String command) {
    command.trim();
    command.toLowerCase();
    
    // Exit command
    if (command == "exit" || command == "quit") {
        Serial.println("Exiting test state - returning to IDLE");
        return 0; // Return to IDLE state
    }
    
    // Status command
    if (command == "status") {
        Serial.println("=== CURRENT POSITIONS ===");
        Serial.print("X1: ");
        Serial.print(x1Motor ? x1Motor->getCurrentPosition() : 0);
        Serial.println(" inches");
        Serial.print("X2: ");
        Serial.print(x2Motor ? x2Motor->getCurrentPosition() : 0);
        Serial.println(" inches");
        Serial.print("Y: ");
        Serial.print(yMotor ? yMotor->getCurrentPosition() : 0);
        Serial.println(" inches");
        Serial.print("Fork: ");
        Serial.print(forkMotor ? forkMotor->getCurrentPosition() : 0);
        Serial.println(" inches");
        Serial.println("========================");
        return 2; // Stay in test state
    }
    
    // Coordinate command (x,y format)
    int commaIndex = command.indexOf(',');
    if (commaIndex > 0) {
        String xStr = command.substring(0, commaIndex);
        String yStr = command.substring(commaIndex + 1);
        
        // Parse X coordinate
        float xPos = xStr.toFloat();
        if (xStr.length() == 0 || (xPos == 0 && xStr.charAt(0) != '0')) {
            Serial.println("ERROR: Invalid X coordinate");
            return 2; // Stay in test state
        }
        
        // Parse Y coordinate
        float yPos = yStr.toFloat();
        if (yStr.length() == 0 || (yPos == 0 && yStr.charAt(0) != '0')) {
            Serial.println("ERROR: Invalid Y coordinate");
            return 2; // Stay in test state
        }
        
        // Move motors to positions
        Serial.print("Moving to X=");
        Serial.print(xPos);
        Serial.print(", Y=");
        Serial.println(yPos);
        
        // Move X motors simultaneously (both to same position)
        if (x1Motor) x1Motor->moveToPosition(xPos);
        if (x2Motor) x2Motor->moveToPosition(xPos);
        
        // Move Y motor
        if (yMotor) yMotor->moveToPosition(yPos);
        
        return 2; // Stay in test state
    }
    
    // Invalid command
    Serial.println("ERROR: Invalid command");
    Serial.println("Use format: x,y (e.g., 5.5,7.2)");
    Serial.println("Or type 'exit' to return to IDLE");
    Serial.println("Or type 'status' to see current positions");
    return 2; // Stay in test state
}

// Function to reset test state
void resetTestPositionState() {
    testStateInitialized = false;
    waitingForInput = true;
    inputBuffer = "";
} 