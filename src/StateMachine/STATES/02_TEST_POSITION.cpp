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
extern StepperMotor* storageMotor;

// State variables
bool testStateInitialized = false;
bool waitingForInput = true;
String inputBuffer = "";

// Manual movement variables
bool manualMovementActive = false;
int manualMovementStep = 0;

// Storage motor continuous spinning variables
bool storageMotorSpinning = false;

// Forward declaration
int processCommand(String command);

//* ************************************************************************
//* ************************ MANUAL MOVEMENT FUNCTIONS *********************
//* ************************************************************************

void startManualMovement() {
    //! ************************************************************************
    //! STEP 1: INITIALIZE MANUAL MOVEMENT SEQUENCE
    //! ************************************************************************
    manualMovementActive = true;
    manualMovementStep = 0;
    
    Serial.println("=== MANUAL MOVEMENT SEQUENCE STARTED ===");
    Serial.println("Moving to: 5,5 -> 10,5 -> 10,10 -> 5,10");
}

void executeManualMovement() {
    if (!manualMovementActive) return;
    
    //! ************************************************************************
    //! STEP 1: UPDATE ALL MOTORS FOR MOVEMENT
    //! ************************************************************************
    x1Motor->update();
    x2Motor->update();
    yMotor->update();
    forkMotor->update();
    if (storageMotor) storageMotor->update();
    
    //! ************************************************************************
    //! STEP 2: CHECK IF MOTORS ARE STILL MOVING
    //! ************************************************************************
    bool motorsMoving = x1Motor->isMoving() || x2Motor->isMoving() || yMotor->isMoving() || forkMotor->isMoving();
    
    //! ************************************************************************
    //! STEP 3: EXECUTE NEXT MOVEMENT ONLY IF MOTORS HAVE STOPPED
    //! ************************************************************************
    if (!motorsMoving) {
        switch (manualMovementStep) {
            case 0:
                // Move to 5,5
                Serial.println("=== MOVING TO POSITION 1: 5,5 ===");
                x1Motor->moveToPosition(5.0);
                x2Motor->moveToPosition(5.0);
                yMotor->moveToPosition(5.0);
                manualMovementStep++;
                break;
                
            case 1:
                // Move to 10,5
                Serial.println("=== MOVING TO POSITION 2: 10,5 ===");
                x1Motor->moveToPosition(10.0);
                x2Motor->moveToPosition(10.0);
                yMotor->moveToPosition(5.0);
                manualMovementStep++;
                break;
                
            case 2:
                // Move to 10,10
                Serial.println("=== MOVING TO POSITION 3: 10,10 ===");
                x1Motor->moveToPosition(10.0);
                x2Motor->moveToPosition(10.0);
                yMotor->moveToPosition(10.0);
                manualMovementStep++;
                break;
                
            case 3:
                // Move to 5,10
                Serial.println("=== MOVING TO POSITION 4: 5,10 ===");
                x1Motor->moveToPosition(5.0);
                x2Motor->moveToPosition(5.0);
                yMotor->moveToPosition(10.0);
                manualMovementStep++;
                break;
                
            case 4:
                // Sequence complete
                Serial.println("=== MANUAL MOVEMENT SEQUENCE COMPLETE ===");
                manualMovementActive = false;
                manualMovementStep = 0;
                break;
        }
    }
}

// Function to initialize test state
void initializeTestPositionState() {
    if (!testStateInitialized) {
        Serial.println("=== TEST POSITION STATE ===");
        Serial.println("Enter coordinates in format: x,y");
        Serial.println("Example: 5.5,7.2");
        Serial.println("Safe range: (0,0) to (15,15)");
        Serial.println("Type 'home' to re-home all motors");
        Serial.println("Type 'status' to see current positions");
        Serial.println("Type 'm' for manual movement sequence (5,5->10,5->10,10->5,10)");
        Serial.println("Type 'test' to start/stop storage motor continuous spinning");
        Serial.println("Type 'exit' to return to IDLE");
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
    
    //! ************************************************************************
    //! STEP 1: EXECUTE MANUAL MOVEMENT IF ACTIVE
    //! ************************************************************************
    executeManualMovement();
    
    // Update all motors
    if (x1Motor) x1Motor->update();
    if (x2Motor) x2Motor->update();
    if (yMotor) yMotor->update();
    if (forkMotor) forkMotor->update();
    if (storageMotor) storageMotor->update();
    

    
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
    
    // Home command
    if (command == "home" || command == "h") {
        Serial.println("Homing command received - returning to IDLE for homing");
        return 0; // Return to IDLE state to trigger homing
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
        Serial.print("Storage: ");
        Serial.print(storageMotor ? storageMotor->getCurrentPosition() : 0);
        Serial.println(" inches");
        Serial.print("Storage motor spinning: ");
        Serial.println(storageMotorSpinning ? "YES" : "NO");
        Serial.println("========================");
        return 2; // Stay in test state
    }
    
    // Manual movement command
    if (command == "m") {
        Serial.println("Manual movement command received!");
        startManualMovement();
        return 2; // Stay in test state
    }
    
    // Storage motor test command
    if (command == "test") {
        if (!storageMotorSpinning) {
            // Start continuous spinning
            Serial.println("Starting storage motor continuous spinning...");
            storageMotorSpinning = true;
            // Start continuous movement by moving to a very large distance
            if (storageMotor) {
                // Set high speed for continuous spinning
                storageMotor->moveToPosition(10000.0); // Very large distance for continuous movement
            }
        } else {
            // Stop spinning
            Serial.println("Stopping storage motor...");
            storageMotorSpinning = false;
            if (storageMotor) {
                storageMotor->forceStop();
            }
        }
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
        
        // Safety checks - ensure we stay within bounds (0,0) to (15,15)
        if (xPos < 0.0 || xPos > 15.0 || yPos < 0.0 || yPos > 15.0) {
            Serial.println("WARNING: Target position is outside safe bounds!");
            Serial.print("Target position - X: ");
            Serial.print(xPos);
            Serial.print(", Y: ");
            Serial.println(yPos);
            Serial.println("Safe range is (0,0) to (15,15)");
            Serial.println("Movement blocked for safety");
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
    Serial.println("Safe range: (0,0) to (15,15)");
    Serial.println("Or type 'home' to re-home all motors");
    Serial.println("Or type 'status' to see current positions");
    Serial.println("Or type 'm' for manual movement sequence");
    Serial.println("Or type 'test' to start/stop storage motor continuous spinning");
    Serial.println("Or type 'exit' to return to IDLE");
    return 2; // Stay in test state
}

// Function to reset test state
void resetTestPositionState() {
    testStateInitialized = false;
    waitingForInput = true;
    inputBuffer = "";
} 