#include "../../../include/StateMachine/STATES/01_HOMING.h"
#include <Arduino.h>

// Constructor
HomingState::HomingState()
    : motorX(nullptr), motorY(nullptr), motorFork(nullptr), homeSwitches(nullptr),
      currentPhase(HOMING_INIT), phaseStartTime(0), phaseTimeout(false),
      forkHomed(false), xHomed(false), yHomed(false) {
}

// Destructor
HomingState::~HomingState() {
    if (motorX) delete motorX;
    if (motorY) delete motorY;
    if (motorFork) delete motorFork;
    if (homeSwitches) delete homeSwitches;
}

// State interface functions
void HomingState::enter() {
    Serial.println("Initializing homing state...");

    // Initialize motors
    motorX = new StepperMotor(MOTOR_X);
    motorY = new StepperMotor(MOTOR_Y);
    motorFork = new StepperMotor(MOTOR_FORK);

    Serial.println("Initializing motors...");
    motorX->init();
    motorY->init();
    motorFork->init();

    // Initialize home switches
    Serial.println("Initializing home switches...");
    homeSwitches = new HomeSwitch();
    homeSwitches->init();

    // Reset state variables
    currentPhase = HOMING_INIT;
    phaseStartTime = millis();
    phaseTimeout = false;
    forkHomed = false;
    xHomed = false;
    yHomed = false;

    Serial.println("Starting homing sequence...");
    // Start homing sequence
    startHomingSequence();
}

void HomingState::update() {
    // Update home switches
    if (homeSwitches) {
        homeSwitches->update();

        // Debug: Print switch states every second
        static unsigned long lastDebugTime = 0;
        if (millis() - lastDebugTime > 1000) {
            homeSwitches->printDebugInfo();
            lastDebugTime = millis();
        }
    }

    // Check for phase timeout
    if (checkPhaseTimeout()) {
        currentPhase = HOMING_ERROR;
        emergencyStop();
        return;
    }

    // Execute current homing phase
    switch (currentPhase) {
        case HOMING_INIT:
            Serial.println("Entering HOMING_FORK phase");
            // Move to fork homing phase
            currentPhase = HOMING_FORK;
            phaseStartTime = millis();
            homeFork();
            break;

        case HOMING_FORK:
            // Debug: Print fork home switch status
            Serial.print("Fork home switch: ");
            Serial.println(homeSwitches->isForkHome() ? "TRIGGERED" : "not triggered");

            // Check if fork is home
            if (homeSwitches->isForkHome()) {
                Serial.println("Fork reached home - stopping motor");
                motorFork->stop();
                forkHomed = true;
                motorFork->setCurrentPosition(0);  // Set current position as home

                // Move to X and Y homing phase
                currentPhase = HOMING_X_AND_Y;
                phaseStartTime = millis();
                homeXAndY();
            }
            break;

        case HOMING_X_AND_Y:
            // Debug: Print X and Y home switch status
            Serial.print("X home switches: X1=");
            Serial.print(homeSwitches->isX1Home() ? "TRIGGERED" : "not");
            Serial.print(" X2=");
            Serial.print(homeSwitches->isX2Home() ? "TRIGGERED" : "not");
            Serial.print(" (X home: ");
            Serial.print(homeSwitches->isXHome() ? "YES" : "NO");
            Serial.print(") Y home switch: ");
            Serial.println(homeSwitches->isYHome() ? "TRIGGERED" : "not triggered");

            // Check if both X and Y are home
            if (homeSwitches->isXHome() && homeSwitches->isYHome()) {
                Serial.println("X and Y reached home - stopping motors");
                motorX->stop();
                motorY->stop();
                xHomed = true;
                yHomed = true;
                motorX->setCurrentPosition(0);   // Set current position as home
                motorY->setCurrentPosition(0);   // Set current position as home

                // Move to move away phase
                currentPhase = HOMING_MOVE_AWAY;
                phaseStartTime = millis();
                moveAwayFromHome();
            }
            break;

        case HOMING_MOVE_AWAY:
            // Check if all motors have finished moving away
            if (!motorX->isRunning() && !motorY->isRunning() && !motorFork->isRunning()) {
                currentPhase = HOMING_COMPLETE;
            }
            break;

        case HOMING_COMPLETE:
        case HOMING_ERROR:
            // Do nothing - state is complete or in error
            break;
    }
}

void HomingState::exit() {
    // Stop all motors
    emergencyStop();

    // Clean up (optional - could be done in destructor)
}

// Helper functions
void HomingState::startHomingSequence() {
    // Set homing speeds for all motors
    motorX->setSpeed(X_HOME_SPEED);
    motorY->setSpeed(Y_HOME_SPEED);
    motorFork->setSpeed(FORK_HOME_SPEED);

    motorX->setAcceleration(X_HOME_ACCEL);
    motorY->setAcceleration(Y_HOME_ACCEL);
    motorFork->setAcceleration(FORK_HOME_ACCEL);
}

void HomingState::homeFork() {
    Serial.println("Starting fork homing...");
    // Move fork in homing direction until home switch is triggered
    if (FORK_HOME_DIRECTION_POSITIVE) {
        Serial.println("Moving fork positive direction (10 inches)");
        motorFork->moveRelativeInches(10.0);  // Move positive direction
    } else {
        Serial.println("Moving fork negative direction (-10 inches)");
        motorFork->moveRelativeInches(-10.0); // Move negative direction
    }
}

void HomingState::homeXAndY() {
    Serial.println("Starting X and Y homing...");
    // Move X and Y simultaneously in homing direction until home switches are triggered
    if (X_HOME_DIRECTION_POSITIVE) {
        Serial.println("Moving X positive direction (20 inches)");
        motorX->moveRelativeInches(20.0);   // Move positive direction
    } else {
        Serial.println("Moving X negative direction (-20 inches)");
        motorX->moveRelativeInches(-20.0);  // Move negative direction
    }

    if (Y_HOME_DIRECTION_POSITIVE) {
        Serial.println("Moving Y positive direction (20 inches)");
        motorY->moveRelativeInches(20.0);   // Move positive direction
    } else {
        Serial.println("Moving Y negative direction (-20 inches)");
        motorY->moveRelativeInches(-20.0);  // Move negative direction
    }
}

void HomingState::moveAwayFromHome() {
    // Move all motors away from home switches by MOVE_AWAY_FROM_HOME_DISTANCE inches
    float distance = MOVE_AWAY_FROM_HOME_DISTANCE;

    if (X_MOVE_AWAY_DIRECTION_POSITIVE) {
        motorX->moveRelativeInches(distance);
    } else {
        motorX->moveRelativeInches(-distance);
    }

    if (Y_MOVE_AWAY_DIRECTION_POSITIVE) {
        motorY->moveRelativeInches(distance);
    } else {
        motorY->moveRelativeInches(-distance);
    }

    if (FORK_MOVE_AWAY_DIRECTION_POSITIVE) {
        motorFork->moveRelativeInches(distance);
    } else {
        motorFork->moveRelativeInches(-distance);
    }
}

bool HomingState::checkPhaseTimeout() {
    unsigned long currentTime = millis();
    unsigned long elapsedTime = currentTime - phaseStartTime;

    switch (currentPhase) {
        case HOMING_FORK:
        case HOMING_X_AND_Y:
            return elapsedTime > PHASE_TIMEOUT_MS;
        case HOMING_MOVE_AWAY:
            return elapsedTime > PHASE_TIMEOUT_MS;
        default:
            return elapsedTime > HOMING_TIMEOUT_MS;
    }
}

// Status functions
bool HomingState::isComplete() {
    return currentPhase == HOMING_COMPLETE;
}

bool HomingState::hasError() {
    return currentPhase == HOMING_ERROR;
}

HomingPhase HomingState::getCurrentPhase() {
    return currentPhase;
}

// Emergency functions
void HomingState::emergencyStop() {
    if (motorX) motorX->emergencyStop();
    if (motorY) motorY->emergencyStop();
    if (motorFork) motorFork->emergencyStop();
}
