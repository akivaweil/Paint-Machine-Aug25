#include "../../../include/StateMachine/STATES/01_HOMING.h"
#include "../../../src/config/Pin_Definitions.h"

//* ************************************************************************
//* ************************ HOMING STATE IMPLEMENTATION ***************************
//* ************************************************************************

HomingState::HomingState() :
    motorX(nullptr),
    motorY(nullptr),
    motorFork(nullptr),
    homeSwitches(nullptr),
    currentPhase(HOMING_INIT),
    phaseStartTime(0),
    phaseTimeout(false),
    forkHomed(false),
    xHomed(false),
    yHomed(false) {
}

HomingState::~HomingState() {
    // Clean up motors and switches
    if (motorX) delete motorX;
    if (motorY) delete motorY;
    if (motorFork) delete motorFork;
    if (homeSwitches) delete homeSwitches;
}

void HomingState::enter() {
    Serial.println("Entering Homing State...");

    // Initialize motors
    motorX = new StepperMotor(X_STEP_PIN, X_DIR_PIN, X_STEPS_PER_INCH, X_MAX_SPEED, X_MAX_ACCEL);
    motorY = new StepperMotor(Y_STEP_PIN, Y_DIR_PIN, STEPS_PER_INCH, Y_MAX_SPEED, Y_MAX_ACCEL);
    motorFork = new StepperMotor(FORK_STEP_PIN, FORK_DIR_PIN, STEPS_PER_INCH, FORK_MAX_SPEED, FORK_MAX_ACCEL);

    // Initialize home switches (X uses dual switches)
    homeSwitches = new HomeSwitch(X_HOME_PIN, X_HOME_PIN2);
    homeSwitches->begin();

    // Start homing sequence
    startHomingSequence();

    Serial.println("Homing State initialized");
}

void HomingState::update() {
    switch (currentPhase) {
        case HOMING_INIT:
            // Immediately start X homing
            homeXAndY();
            break;

        case HOMING_X_AND_Y:
            // Check if X homing is complete (both switches must be high with 3ms debounce)
            if (homeSwitches->readDualDebounced()) {
                // X home switches triggered, stop motor
                motorX->forceStop();
                xHomed = true;
                currentPhase = HOMING_MOVE_AWAY;
                phaseStartTime = millis();
                Serial.println("X motor homed successfully (both switches high)");
            }

            // Check for timeout
            if (checkPhaseTimeout()) {
                Serial.println("X homing timeout!");
                currentPhase = HOMING_ERROR;
            }
            break;

        case HOMING_MOVE_AWAY:
            // Move X motor away 0.5 inches
            if (!motorX->isMotorRunning()) {
                Serial.println("Moving X motor away 0.5 inches...");
                motorX->setDirection(X_MOVE_AWAY_DIRECTION_POSITIVE);
                motorX->setSpeed(X_HOME_SPEED);
                motorX->moveInches(MOVE_AWAY_FROM_HOME_DISTANCE);
                currentPhase = HOMING_COMPLETE;
                Serial.println("X motor moved away successfully");
            }
            break;

        case HOMING_COMPLETE:
        case HOMING_ERROR:
            // Nothing to do in these states
            break;

        default:
            currentPhase = HOMING_ERROR;
            break;
    }
}

void HomingState::exit() {
    Serial.println("Exiting Homing State...");

    // Force stop all motors
    if (motorX) motorX->forceStop();
    if (motorY) motorY->forceStop();
    if (motorFork) motorFork->forceStop();

    // Clean up
    if (motorX) delete motorX;
    if (motorY) delete motorY;
    if (motorFork) delete motorFork;
    if (homeSwitches) delete homeSwitches;

    motorX = nullptr;
    motorY = nullptr;
    motorFork = nullptr;
    homeSwitches = nullptr;

    Serial.println("Homing State exited");
}

//* ************************************************************************
//* STEP 1: START HOMING SEQUENCE
//! ************************************************************************
void HomingState::startHomingSequence() {
    currentPhase = HOMING_INIT;
    phaseStartTime = millis();
    phaseTimeout = false;

    // Reset homing flags
    xHomed = false;
    yHomed = false;
    forkHomed = false;

    Serial.println("Homing sequence started");
}

//* ************************************************************************
//* STEP 2: HOME X AND Y MOTORS (X ONLY FOR NOW)
//! ************************************************************************
void HomingState::homeXAndY() {
    Serial.println("Starting X motor homing...");

    // Set X motor direction for homing (towards home switch)
    motorX->setDirection(X_HOME_DIRECTION_POSITIVE);

    // Set homing speed
    motorX->setSpeed(X_HOME_SPEED);

    // Start moving towards home switch
    currentPhase = HOMING_X_AND_Y;
    phaseStartTime = millis();

    Serial.println("X motor moving towards home switch");
}

// Legacy methods (not used for X-only homing)
void HomingState::homeFork() {}
void HomingState::moveAwayFromHome() {}

bool HomingState::checkPhaseTimeout() {
    return (millis() - phaseStartTime) > PHASE_TIMEOUT_MS;
}

bool HomingState::isComplete() {
    return currentPhase == HOMING_COMPLETE;
}

bool HomingState::hasError() {
    return currentPhase == HOMING_ERROR;
}

HomingPhase HomingState::getCurrentPhase() {
    return currentPhase;
}

void HomingState::emergencyStop() {
    if (motorX) motorX->forceStop();
    if (motorY) motorY->forceStop();
    if (motorFork) motorFork->forceStop();
    currentPhase = HOMING_ERROR;
}
