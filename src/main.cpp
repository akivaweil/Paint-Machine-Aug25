#include <Arduino.h>
#include "config/Config.h"
#include "config/Pin_Definitions.h"
#include "StateMachine/FUNCTIONS/StepperMotor.h"
#include "StateMachine/STATES/00_IDLE.h"
#include "StateMachine/STATES/01_HOMING.h"
#include "StateMachine/STATES/02_TEST_POSITION.h"

//* ************************************************************************
//* ************************ MAIN APPLICATION *******************************
//* ************************************************************************

// Global motor objects - all motors needed
StepperMotor* x1Motor = nullptr;
StepperMotor* x2Motor = nullptr;
StepperMotor* yMotor = nullptr;
StepperMotor* forkMotor = nullptr;

// State machine variables
int currentState = 0; // 0 = IDLE, 1 = HOMING, 2 = TEST_POSITION
bool stateInitialized = false;

// Manual movement variables
bool manualMovementActive = false;
int manualMovementStep = 0;
unsigned long manualMovementStartTime = 0;
const unsigned long MANUAL_MOVEMENT_DELAY = 2000; // 2 seconds between movements

//* ************************************************************************
//* ************************ MANUAL MOVEMENT FUNCTIONS *********************
//* ************************************************************************

void startManualMovement() {
    //! ************************************************************************
    //! STEP 1: INITIALIZE MANUAL MOVEMENT SEQUENCE
    //! ************************************************************************
    manualMovementActive = true;
    manualMovementStep = 0;
    manualMovementStartTime = millis();
    
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
    
    //! ************************************************************************
    //! STEP 2: CHECK IF MOTORS ARE STILL MOVING
    //! ************************************************************************
    bool motorsMoving = x1Motor->isMoving() || x2Motor->isMoving() || yMotor->isMoving() || forkMotor->isMoving();
    
    //! ************************************************************************
    //! STEP 3: EXECUTE NEXT MOVEMENT ONLY IF MOTORS HAVE STOPPED AND TIME HAS PASSED
    //! ************************************************************************
    if (!motorsMoving && (millis() - manualMovementStartTime >= MANUAL_MOVEMENT_DELAY)) {
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
        
        // Reset timer for next movement
        manualMovementStartTime = millis();
    }
}

void checkSerialCommands() {
    //! ************************************************************************
    //! STEP 1: CHECK FOR SERIAL COMMANDS
    //! ************************************************************************
    if (Serial.available()) {
        char command = Serial.read();
        
        // Convert to lowercase for case-insensitive comparison
        command = tolower(command);
        
        //! ************************************************************************
        //! STEP 2: PROCESS MANUAL MOVEMENT COMMAND
        //! ************************************************************************
        if (command == 'm') {
            Serial.println("Manual movement command received!");
            startManualMovement();
        }
    }
}

void setup() {
    // Initialize serial communication
    Serial.begin(115200);
    Serial.println("=== Paint Machine Starting ===");
    
    // Create motor objects
    x1Motor = new StepperMotor(X1_STEP_PIN, X1_DIR_PIN, X1_HOME_PIN, X1_LIMIT_PIN, "X1");
    x2Motor = new StepperMotor(X2_STEP_PIN, X2_DIR_PIN, X2_HOME_PIN, X2_LIMIT_PIN, "X2");
    yMotor = new StepperMotor(Y_STEP_PIN, Y_DIR_PIN, Y_HOME_PIN, Y_LIMIT_PIN, "Y");
    forkMotor = new StepperMotor(FORK_STEP_PIN, FORK_DIR_PIN, FORK_HOME_PIN, FORK_LIMIT_PIN, "Fork");
    
    // Initialize motors
    x1Motor->initialize();
    x2Motor->initialize();
    yMotor->initialize();
    forkMotor->initialize();
    
    Serial.println("All motors initialized");
    
    // Wait a moment to see switch states
    delay(2000);
    Serial.println("=== SWITCH STATES AFTER INITIALIZATION ===");
    Serial.print("X1 - Home: ");
    Serial.print(x1Motor->isHomeSwitchTriggered());
    Serial.print(", Limit: ");
    Serial.println(x1Motor->isLimitSwitchTriggered());
    Serial.print("X2 - Home: ");
    Serial.print(x2Motor->isHomeSwitchTriggered());
    Serial.print(", Limit: ");
    Serial.println(x2Motor->isLimitSwitchTriggered());
    Serial.print("Y - Home: ");
    Serial.print(yMotor->isHomeSwitchTriggered());
    Serial.print(", Limit: ");
    Serial.println(yMotor->isLimitSwitchTriggered());
    Serial.print("Fork - Home: ");
    Serial.print(forkMotor->isHomeSwitchTriggered());
    Serial.print(", Limit: ");
    Serial.println(forkMotor->isLimitSwitchTriggered());
    Serial.println("==========================================");
    
    // Test move away directions for debugging
    Serial.println("=== MOVE AWAY DIRECTION TEST ===");
    x1Motor->testMoveAwayDirection();
    x2Motor->testMoveAwayDirection();
    yMotor->testMoveAwayDirection();
    forkMotor->testMoveAwayDirection();
    Serial.println("=================================");
    
    // Start in homing state for automatic homing on startup
    currentState = 1;
    stateInitialized = false;
    
    // Print manual movement instructions
    Serial.println("=== MANUAL MOVEMENT COMMANDS ===");
    Serial.println("Type 'm' in serial monitor to start movement sequence:");
    Serial.println("5,5 -> 10,5 -> 10,10 -> 5,10");
    Serial.println("=================================");
}

void loop() {
    //! ************************************************************************
    //! STEP 1: CHECK FOR SERIAL COMMANDS (HIGHEST PRIORITY)
    //! ************************************************************************
    checkSerialCommands();
    
    //! ************************************************************************
    //! STEP 2: EXECUTE MANUAL MOVEMENT IF ACTIVE
    //! ************************************************************************
    executeManualMovement();
    
    // State machine logic
    if (!stateInitialized) {
        // Initialize current state
        if (currentState == 0) {
            // IDLE state initialization
            resetIdleState();
            stateInitialized = true;
        } else if (currentState == 1) {
            // HOMING state initialization
            resetHomingState();
            stateInitialized = true;
        } else if (currentState == 2) {
            // TEST_POSITION state initialization
            resetTestPositionState();
            stateInitialized = true;
        }
    }
    
    // Run current state
    int newState = currentState;
    if (currentState == 0) {
        // IDLE state
        newState = runIdleState();
    } else if (currentState == 1) {
        // HOMING state
        newState = runHomingState();
    } else if (currentState == 2) {
        // TEST_POSITION state
        newState = runTestPositionState();
    }
    
    // Check if state wants to change
    if (newState != currentState) {
        currentState = newState;
        stateInitialized = false;
    }
    
    // Small delay to prevent overwhelming the system
    delay(1);
}