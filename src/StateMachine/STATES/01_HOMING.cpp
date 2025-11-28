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
    // Initialize motors
    motorX = new StepperMotor(MOTOR_X);
    motorY = new StepperMotor(MOTOR_Y);
    motorFork = new StepperMotor(MOTOR_FORK);

    motorX->init();
    motorY->init();
    motorFork->init();

    // Initialize home switches
    homeSwitches = new HomeSwitch();
    homeSwitches->init();

    // Set homing speeds and start all motors toward home
    motorX->setSpeed(X_HOME_SPEED);
    motorY->setSpeed(Y_HOME_SPEED);
    motorFork->setSpeed(FORK_HOME_SPEED);

    // Move all motors toward home simultaneously
    motorX->moveRelativeInches(X_HOME_DIRECTION_POSITIVE ? 20.0 : -20.0);
    motorY->moveRelativeInches(Y_HOME_DIRECTION_POSITIVE ? 20.0 : -20.0);
    motorFork->moveRelativeInches(FORK_HOME_DIRECTION_POSITIVE ? 10.0 : -10.0);
}

void HomingState::update() {
    homeSwitches->update();

    // Check if all motors have reached home
    bool xHome = homeSwitches->isXHome();
    bool yHome = homeSwitches->isYHome();
    bool forkHome = homeSwitches->isForkHome();

    if (xHome && !xHomed) {
        motorX->stop();
        motorX->setCurrentPosition(0);
        xHomed = true;
    }

    if (yHome && !yHomed) {
        motorY->stop();
        motorY->setCurrentPosition(0);
        yHomed = true;
    }

    if (forkHome && !forkHomed) {
        motorFork->stop();
        motorFork->setCurrentPosition(0);
        forkHomed = true;
    }

    // When all are homed, move away from switches
    if (xHomed && yHomed && forkHomed && currentPhase == HOMING_INIT) {
        currentPhase = HOMING_MOVE_AWAY;

        motorX->moveRelativeInches(X_MOVE_AWAY_DIRECTION_POSITIVE ? MOVE_AWAY_FROM_HOME_DISTANCE : -MOVE_AWAY_FROM_HOME_DISTANCE);
        motorY->moveRelativeInches(Y_MOVE_AWAY_DIRECTION_POSITIVE ? MOVE_AWAY_FROM_HOME_DISTANCE : -MOVE_AWAY_FROM_HOME_DISTANCE);
        motorFork->moveRelativeInches(FORK_MOVE_AWAY_DIRECTION_POSITIVE ? MOVE_AWAY_FROM_HOME_DISTANCE : -MOVE_AWAY_FROM_HOME_DISTANCE);
    }

    // Check if move away is complete
    if (currentPhase == HOMING_MOVE_AWAY && !motorX->isRunning() && !motorY->isRunning() && !motorFork->isRunning()) {
        currentPhase = HOMING_COMPLETE;
    }
}

void HomingState::exit() {
    emergencyStop();
}

// Status functions
bool HomingState::isComplete() {
    return currentPhase == HOMING_COMPLETE;
}

bool HomingState::hasError() {
    return false; // No error handling
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
