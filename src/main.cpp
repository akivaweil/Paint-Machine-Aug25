#include <Arduino.h>
#include "config/Config.h"
#include "config/Pin_Definitions.h"
#include "StateMachine/FUNCTIONS/StepperMotor.h"
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

// Homing sequence tracking
int homingPhase = 0; // 0 = X motors, 1 = Y motor, 2 = Fork motor

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
    
    // Start in homing state for automatic homing on startup
    currentState = 1;
    stateInitialized = false;
    homingPhase = 0;
}

void loop() {
    // State machine logic
    if (!stateInitialized) {
        // Initialize current state
        if (currentState == 0) {
            // IDLE state initialization
            Serial.println("Entering IDLE state");
            stateInitialized = true;
        } else if (currentState == 1) {
            // HOMING state initialization
            Serial.println("Starting homing sequence - Phase 1: X motors (simultaneously)");
            x1Motor->home();
            x2Motor->home();
            homingPhase = 0;
            stateInitialized = true;
        } else if (currentState == 2) {
            // TEST_POSITION state initialization
            resetTestPositionState();
            stateInitialized = true;
        }
    }
    
    // Run current state
    if (currentState == 0) {
        // IDLE state - check for homing command
        if (Serial.available()) {
            String command = Serial.readString();
            command.trim();
            if (command == "home" || command == "h") {
                Serial.println("Homing command received - transitioning to homing state");
                currentState = 1;
                stateInitialized = false;
            } else if (command == "test" || command == "t") {
                Serial.println("Test position command received - transitioning to test state");
                currentState = 2;
                stateInitialized = false;
            }
        }
    } else if (currentState == 1) {
        // HOMING state
        // Update all motor states during homing
        x1Motor->updateHoming();
        x2Motor->updateHoming();
        yMotor->updateHoming();
        forkMotor->updateHoming();
        
        // Declare variables outside switch to avoid compilation errors
        bool x1Homed, x2Homed, yHomed, forkHomed;
        
        // Handle different homing phases
        switch (homingPhase) {
            case 0: // X motors (both simultaneously)
                // Check if both X motors have reached home switches AND stopped moving
                x1Homed = x1Motor->isHomingComplete();
                x2Homed = x2Motor->isHomingComplete();
                
                if (x1Homed && x2Homed) {
                    // Both X motors have reached home switches and stopped
                    x1Motor->forceStop();
                    x2Motor->forceStop();
                    x1Motor->setCurrentPositionAsZero();
                    x2Motor->setCurrentPositionAsZero();
                    
                    Serial.println("X motors homing complete - moving away from home");
                    homingPhase = 1;
                    x1Motor->moveAwayFromHome();
                    x2Motor->moveAwayFromHome();
                } else {
                    // Still homing X motors - provide status updates
                    static unsigned long lastXStatusTime = 0;
                    if (millis() - lastXStatusTime > 1000) {
                        Serial.print("X motors homing - X1: ");
                        if (x1Motor->isHomeSwitchTriggered()) {
                            Serial.print(x1Motor->isMoving() ? "AT_HOME_MOVING" : "HOMED");
                        } else {
                            Serial.print("MOVING");
                        }
                        Serial.print(", X2: ");
                        if (x2Motor->isHomeSwitchTriggered()) {
                            Serial.println(x2Motor->isMoving() ? "AT_HOME_MOVING" : "HOMED");
                        } else {
                            Serial.println("MOVING");
                        }
                        lastXStatusTime = millis();
                    }
                }
                break;
                
            case 1: // X motors moving away from home
                // Check if both X motors have finished moving away
                if (!x1Motor->isMoving() && !x2Motor->isMoving()) {
                    Serial.println("X motors moved away from home - Phase 2: Y motor");
                    homingPhase = 2;
                    yMotor->home();
                } else {
                    // Still moving away from home - provide status updates
                    static unsigned long lastXMoveTime = 0;
                    if (millis() - lastXMoveTime > 1000) {
                        Serial.print("X motors moving away - X1: ");
                        Serial.print(x1Motor->isMoving() ? "MOVING" : "STOPPED");
                        Serial.print(", X2: ");
                        Serial.println(x2Motor->isMoving() ? "MOVING" : "STOPPED");
                        lastXMoveTime = millis();
                    }
                }
                break;
                
            case 2: // Y motor
                yHomed = yMotor->isHomingComplete();
                
                if (yHomed) {
                    // Y motor has reached home switch and stopped
                    yMotor->forceStop();
                    yMotor->setCurrentPositionAsZero();
                    
                    Serial.println("Y motor homing complete - moving away from home");
                    homingPhase = 3;
                    yMotor->moveAwayFromHome();
                } else {
                    // Still homing Y motor - provide status updates
                    static unsigned long lastYStatusTime = 0;
                    if (millis() - lastYStatusTime > 1000) {
                        Serial.print("Y motor homing - Y: ");
                        if (yMotor->isHomeSwitchTriggered()) {
                            Serial.println(yMotor->isMoving() ? "AT_HOME_MOVING" : "HOMED");
                        } else {
                            Serial.println("MOVING");
                        }
                        lastYStatusTime = millis();
                    }
                }
                break;
                
            case 3: // Y motor moving away from home
                // Check if Y motor has finished moving away
                if (!yMotor->isMoving()) {
                    Serial.println("Y motor moved away from home - Phase 4: Fork motor");
                    homingPhase = 4;
                    forkMotor->home();
                } else {
                    // Still moving away from home - provide status updates
                    static unsigned long lastYMoveTime = 0;
                    if (millis() - lastYMoveTime > 1000) {
                        Serial.print("Y motor moving away - Y: ");
                        Serial.println(yMotor->isMoving() ? "MOVING" : "STOPPED");
                        lastYMoveTime = millis();
                    }
                }
                break;
                
            case 4: // Fork motor
                forkHomed = forkMotor->isHomingComplete();
                
                if (forkHomed) {
                    // Fork motor has reached home switch and stopped
                    forkMotor->forceStop();
                    forkMotor->setCurrentPositionAsZero();
                    
                    Serial.println("Fork motor homing complete - moving away from home");
                    homingPhase = 5;
                    forkMotor->moveAwayFromHome();
                } else {
                    // Still homing Fork motor - provide status updates
                    static unsigned long lastForkStatusTime = 0;
                    if (millis() - lastForkStatusTime > 1000) {
                        Serial.print("Fork motor homing - Fork: ");
                        if (forkMotor->isHomeSwitchTriggered()) {
                            Serial.println(forkMotor->isMoving() ? "AT_HOME_MOVING" : "HOMED");
                        } else {
                            Serial.println("MOVING");
                        }
                        lastForkStatusTime = millis();
                    }
                }
                break;
                
            case 5: // Fork motor moving away from home
                // Check if Fork motor has finished moving away
                if (!forkMotor->isMoving()) {
                    Serial.println("Fork motor moved away from home");
                    Serial.println("All motors homed successfully - returning to IDLE state");
                    
                    // Return to idle state
                    currentState = 0;
                    stateInitialized = false;
                    homingPhase = 0;
                } else {
                    // Still moving away from home - provide status updates
                    static unsigned long lastForkMoveTime = 0;
                    if (millis() - lastForkMoveTime > 1000) {
                        Serial.print("Fork motor moving away - Fork: ");
                        Serial.println(forkMotor->isMoving() ? "MOVING" : "STOPPED");
                        lastForkMoveTime = millis();
                    }
                }
                break;
        }
    } else if (currentState == 2) {
        // TEST_POSITION state
        int newState = runTestPositionState();
        if (newState != 2) {
            // State wants to change
            currentState = newState;
            stateInitialized = false;
        }
    }
    
    // Small delay to prevent overwhelming the system
    delay(1);
}