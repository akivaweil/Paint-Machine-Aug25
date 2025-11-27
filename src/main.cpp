#include <Arduino.h>
#include "config/Config.h"
#include "config/Pin_Definitions.h"
#include "StateMachine/FUNCTIONS/StepperMotor.h"
#include "StateMachine/STATES/00_IDLE.h"
#include "StateMachine/STATES/01_HOMING.h"
#include "StateMachine/STATES/02_MOVE_TO_POSITION.h"

//* ************************************************************************
//* ************************ MAIN APPLICATION *******************************
//* ************************************************************************

// Global motor objects - all motors needed
StepperMotor* x1Motor = nullptr;
StepperMotor* x2Motor = nullptr;
StepperMotor* yMotor = nullptr;
StepperMotor* forkMotor = nullptr;
StepperMotor* storageMotor = nullptr;

// State machine variables
int currentState = 0; // 0 = IDLE, 1 = HOMING, 2 = MOVE_TO_POSITION
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

    // Create motor objects
    x1Motor = new StepperMotor(X1_STEP_PIN, X1_DIR_PIN, X1_HOME_PIN, "X1");
    x2Motor = new StepperMotor(X2_STEP_PIN, X2_DIR_PIN, X2_HOME_PIN, "X2");
    yMotor = new StepperMotor(Y_STEP_PIN, Y_DIR_PIN, Y_HOME_PIN, "Y");
    forkMotor = new StepperMotor(FORK_STEP_PIN, FORK_DIR_PIN, FORK_HOME_PIN, "Fork");
    storageMotor = new StepperMotor(STORAGE_STEP_PIN, STORAGE_DIR_PIN, -1, "Storage"); // No home switch for storage motor
    
    // Initialize motors
    x1Motor->initialize();
    x2Motor->initialize();
    yMotor->initialize();
    forkMotor->initialize();
    storageMotor->initialize();
    
    Serial.println("All motors initialized");
    
    // Wait a moment to see switch states - Delay removed for faster startup
    // delay(2000); 
    Serial.println("=== SWITCH STATES AFTER INITIALIZATION ===");
    Serial.print("X1 - Home: ");
    Serial.println(x1Motor->isHomeSwitchTriggered());
    Serial.print("X2 - Home: ");
    Serial.println(x2Motor->isHomeSwitchTriggered());
    Serial.print("Y - Home: ");
    Serial.println(yMotor->isHomeSwitchTriggered());
    Serial.print("Fork - Home: ");
    Serial.println(forkMotor->isHomeSwitchTriggered());
    Serial.print("Storage - Home: ");
    Serial.println(storageMotor->isHomeSwitchTriggered());
    Serial.println("==========================================");
    
    // Test move away directions for debugging
    Serial.println("=== MOVE AWAY DIRECTION TEST ===");
    x1Motor->testMoveAwayDirection();
    x2Motor->testMoveAwayDirection();
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
            resetIdleState();
            stateInitialized = true;
        } else if (currentState == 1) {
            // HOMING state initialization
            resetHomingState();
            stateInitialized = true;
        } else if (currentState == 2) {
            // MOVE_TO_POSITION state initialization
            resetMoveToPositionState();
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
    }
    
    // Check if state wants to change
    if (newState != currentState) {
        currentState = newState;
        stateInitialized = false;
    }
    
    // Small delay to prevent overwhelming the system
    delay(1);
}
