#include <Arduino.h>
#include "config/Config.h"
#include "config/Pin_Definitions.h"
#include "StateMachine/FUNCTIONS/StepperMotor.h"
#include "StateMachine/STATES/00_IDLE.h"
#include "StateMachine/STATES/01_HOMING.h"
#include "StateMachine/STATES/02_TEST_POSITION.h"
#include "StateMachine/STATES/03_TEST_SEQUENCE.h"
#include "StateMachine/STATES/04_WEB_CONTROL.h"
#include "CarouselStorage.h"
#include "Web_Manager.h"

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
int currentState = 0; // 0 = IDLE, 1 = HOMING, 2 = TEST_POSITION, 3 = TEST_SEQUENCE, 4 = WEB_CONTROL
bool stateInitialized = false;
bool webServerInitialized = false;

// Test button variables
bool lastTestButtonState = false;
bool testButtonPressed = false;

// Forward declaration for test sequence
void startTestSequence();

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
    
    // Initialize test button pin
    pinMode(TEST_BUTTON_PIN, INPUT_PULLDOWN);
    
    Serial.println("All motors initialized");
    
    // Initialize carousel storage system
    Serial.println("Carousel storage system initialized");
    Serial.print("Total positions: ");
    Serial.println(carouselStorage.getTotalPositions());
    Serial.print("Initial occupancy: ");
    Serial.print(carouselStorage.getOccupancyPercentage(), 1);
    Serial.println("%");
    
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

    // Initialize Web Server once WiFi is connected
    if (isWiFiConnected() && !webServerInitialized) {
        initWebServer();
        webServerInitialized = true;
        Serial.println("Web Server Started");
    }

    //! ************************************************************************
    //! STEP 1: CHECK TEST BUTTON (ALLOWS TRANSITION TO TEST SEQUENCE FROM ANY STATE)
    //! ************************************************************************
    bool currentTestButtonState = digitalRead(TEST_BUTTON_PIN);
    
    // Detect button press (rising edge)
    if (currentTestButtonState && !lastTestButtonState && !testButtonPressed) {
        testButtonPressed = true;
        Serial.println("=== TEST BUTTON PRESSED ===");
        Serial.println("Transitioning to TEST_SEQUENCE state");
        
        // Force transition to test sequence state
        currentState = 3;
        stateInitialized = false;
        
        // Start the test sequence immediately
        startTestSequence();
    }
    
    // Reset button state when released
    if (!currentTestButtonState) {
        testButtonPressed = false;
    }
    
    lastTestButtonState = currentTestButtonState;

    // Check for Web Move Request
    if (isWebMoveRequested() && currentState != 4 && currentState != 1) { // Don't interrupt Homing
        Serial.println("=== WEB MOVE REQUESTED ===");
        currentState = 4;
        stateInitialized = false;
    }
    
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
        } else if (currentState == 3) {
            // TEST_SEQUENCE state initialization
            resetTestSequenceState();
            stateInitialized = true;
        } else if (currentState == 4) {
            // WEB_CONTROL state initialization
            resetWebControlState();
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
    } else if (currentState == 3) {
        // TEST_SEQUENCE state
        newState = runTestSequenceState();
    } else if (currentState == 4) {
        // WEB_CONTROL state
        newState = runWebControlState();
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
