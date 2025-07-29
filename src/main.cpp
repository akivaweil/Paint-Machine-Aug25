#include <Arduino.h>
#include "config/Config.h"
#include "config/Pin_Definitions.h"
#include "StateMachine/FUNCTIONS/StepperMotor.h"
#include "StateMachine/STATES/00_IDLE.h"
#include "StateMachine/STATES/01_HOMING.h"
#include "StateMachine/STATES/02_TEST_POSITION.h"
#include "CarouselStorage.h"

//* ************************************************************************
//* ************************ MAIN APPLICATION *******************************
//* ************************************************************************

// Global motor objects - all motors needed
StepperMotor* x1Motor = nullptr;
StepperMotor* x2Motor = nullptr;
StepperMotor* yMotor = nullptr;
StepperMotor* forkMotor = nullptr;
StepperMotor* storageMotor = nullptr;

// Carousel storage system
CarouselStorage carouselStorage;

// State machine variables
int currentState = 0; // 0 = IDLE, 1 = HOMING, 2 = TEST_POSITION
bool stateInitialized = false;



void setup() {
    // Initialize serial communication
    Serial.begin(115200);
    Serial.println("=== Paint Machine Starting ===");
    
    // Create motor objects
    x1Motor = new StepperMotor(X1_STEP_PIN, X1_DIR_PIN, X1_HOME_PIN, X1_LIMIT_PIN, "X1");
    x2Motor = new StepperMotor(X2_STEP_PIN, X2_DIR_PIN, X2_HOME_PIN, X2_LIMIT_PIN, "X2");
    yMotor = new StepperMotor(Y_STEP_PIN, Y_DIR_PIN, Y_HOME_PIN, Y_LIMIT_PIN, "Y");
    forkMotor = new StepperMotor(FORK_STEP_PIN, FORK_DIR_PIN, FORK_HOME_PIN, FORK_LIMIT_PIN, "Fork");
    storageMotor = new StepperMotor(STORAGE_STEP_PIN, STORAGE_DIR_PIN, -1, -1, "Storage"); // No home/limit switches for storage motor
    
    // Initialize motors
    x1Motor->initialize();
    x2Motor->initialize();
    yMotor->initialize();
    forkMotor->initialize();
    storageMotor->initialize();
    
    Serial.println("All motors initialized");
    
    // Initialize carousel storage system
    Serial.println("Carousel storage system initialized");
    Serial.print("Total positions: ");
    Serial.println(carouselStorage.getTotalPositions());
    Serial.print("Initial occupancy: ");
    Serial.print(carouselStorage.getOccupancyPercentage(), 1);
    Serial.println("%");
    
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
    Serial.print("Storage - Home: ");
    Serial.print(storageMotor->isHomeSwitchTriggered());
    Serial.print(", Limit: ");
    Serial.println(storageMotor->isLimitSwitchTriggered());
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
    
    // Example carousel storage usage (uncomment to test):
    /*
    // Test carousel storage functionality
    static unsigned long lastTestTime = 0;
    if (millis() - lastTestTime > 10000) { // Every 10 seconds
        lastTestTime = millis();
        
        // Example: Set a few positions as occupied
        carouselStorage.setPositionOccupied(0, 0); // Column 0, Row 0
        carouselStorage.setPositionOccupied(1, 1); // Column 1, Row 1
        carouselStorage.setPositionOccupied(2, 2); // Column 2, Row 2
        
        // Print current status
        Serial.println("=== CAROUSEL STORAGE STATUS ===");
        carouselStorage.printGridStatus();
        
        // Find next empty position
        uint8_t emptyCol, emptyRow;
        if (carouselStorage.findNextEmptyPosition(emptyCol, emptyRow)) {
            Serial.print("Next empty position: Column ");
            Serial.print(emptyCol);
            Serial.print(", Row ");
            Serial.println(emptyRow);
        }
        
        Serial.println("===============================");
    }
    */
}