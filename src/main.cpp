#include <Arduino.h>
#include "config/Config.h"
#include "config/Pin_Definitions.h"
#include "StateMachine/FUNCTIONS/StepperMotor.h"
#include "StateMachine/STATES/00_IDLE.h"
#include "StateMachine/STATES/01_HOMING.h"
#include "StateMachine/STATES/02_MOVE_TO_POSITION.h"
#include "StateMachine/WEB_CONTROL/WebControl_Logic.h"
#include "Web_Manager.h"

//* ************************************************************************
//* ************************ MAIN APPLICATION *******************************
//* ************************************************************************

// Global motor objects - all motors needed
StepperMotor* xMotor = nullptr;
StepperMotor* yMotor = nullptr;
StepperMotor* forkMotor = nullptr;
StepperMotor* storageMotor = nullptr;

// State machine variables
int currentState = 0; // 0 = IDLE, 1 = HOMING, 2 = MOVE_TO_POSITION, 4 = WEB_CONTROL
bool stateInitialized = false;

// Forward declaration for OTA manager
void initializeOTA();
void updateOTA();
String getOTAIpAddress();
bool isWiFiConnected();

void setup() {
    // Initialize serial communication
    Serial.begin(115200);
    Serial.println("=== Paint Machine Starting ===");
    
    // Initialize OTA
    initializeOTA();
    Serial.println("OTA IP Address: Waiting for connection...");

    // Initialize Web Server
    initWebServer();

    // Create motor objects
    xMotor = new StepperMotor(X_STEP_PIN, X_DIR_PIN, X_HOME_PIN, "X");
    yMotor = new StepperMotor(Y_STEP_PIN, Y_DIR_PIN, Y_HOME_PIN, "Y");
    forkMotor = new StepperMotor(FORK_STEP_PIN, FORK_DIR_PIN, FORK_HOME_PIN, "Fork");
    storageMotor = new StepperMotor(STORAGE_STEP_PIN, STORAGE_DIR_PIN, -1, "Storage"); // No home switch for storage motor
    
    // Initialize motors
    xMotor->initialize();
    yMotor->initialize();
    forkMotor->initialize();
    storageMotor->initialize();
    
    Serial.println("All motors initialized");
    
    // Wait a moment to see switch states - Delay removed for faster startup
    // delay(2000); 
    Serial.println("=== SWITCH STATES AFTER INITIALIZATION ===");
    Serial.print("X - Home: ");
    Serial.println(xMotor->isHomeSwitchTriggered());
    Serial.print("Y - Home: ");
    Serial.println(yMotor->isHomeSwitchTriggered());
    Serial.print("Fork - Home: ");
    Serial.println(forkMotor->isHomeSwitchTriggered());
    Serial.print("Storage - Home: ");
    Serial.println(storageMotor->isHomeSwitchTriggered());
    Serial.println("==========================================");
    
    // Test move away directions for debugging
    Serial.println("=== MOVE AWAY DIRECTION TEST ===");
    xMotor->testMoveAwayDirection();
    yMotor->testMoveAwayDirection();
    forkMotor->testMoveAwayDirection();
    storageMotor->testMoveAwayDirection();
    Serial.println("=================================");
    
    // Start in homing state for automatic homing on startup
    currentState = 1;
    stateInitialized = false;
}

void loop() {
    // Update OTA
    updateOTA();

    // State machine logic
    if (!stateInitialized) {
        // Initialize current state
        if (currentState == 0) {
            // IDLE state initialization
            StepperMotor::setHomingState(false);
            resetIdleState();
            stateInitialized = true;
        } else if (currentState == 1) {
            // HOMING state initialization
            StepperMotor::setHomingState(true);
            resetHomingState();
            stateInitialized = true;
        } else if (currentState == 2) {
            // MOVE_TO_POSITION state initialization
            StepperMotor::setHomingState(false);
            resetMoveToPositionState();
            stateInitialized = true;
        } else if (currentState == 4) {
            // WEB_CONTROL state initialization
            StepperMotor::setHomingState(false);
            initializeWebControlLogic();
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
        // MOVE_TO_POSITION state
        newState = runMoveToPositionState();
    } else if (currentState == 4) {
        // WEB_CONTROL state
        newState = executeWebControlLogic();
    }
    
    // Check if state wants to change
    if (newState != currentState) {
        currentState = newState;
        stateInitialized = false;
    }
    
    // Small delay to prevent overwhelming the system
    delay(1);
}
