#include <Arduino.h>
#include "StateMachine/STATES/01_HOMING.h"
#include "../../config/Config.h"

// External motor and switch instances (defined in Web_Manager.cpp)
extern StepperMotor* motorX;
extern StepperMotor* motorY;
extern StepperMotor* motorFork;
extern HomeSwitch* homeSwitchX;
extern HomeSwitch* homeSwitchY;
extern HomeSwitch* homeSwitchFork;

// OTA Manager function
extern void updateOTA();

// State machine function
extern void setMachineState(int state);
#define STATE_IDLE 1

//* ************************************************************************
//* ************************ HOMING STATE *********************************
//* ************************************************************************

void homingState() {
    static bool homingComplete = false;
    
    // Perform homing once on first entry
    if (!homingComplete) {
        homeAllAxes();
        homingComplete = true;
        // Transition to idle state after homing
        setMachineState(STATE_IDLE);
    }
}

// Helper function to home a single axis
static void homeSingleAxis(StepperMotor* motor, HomeSwitch* homeSwitch, bool useDual, long maxSpeed) {
    if (!motor || !homeSwitch) return;
    
    // Set homing speed to 1/4 of max speed
    motor->setSpeed(maxSpeed / 4);
    
    // Set direction to move toward home (negative direction)
    motor->setDirection(false);
    
    // Start continuous movement toward home
    motor->startContinuous();
    
    // Keep moving until home switch is triggered
    while (useDual ? !homeSwitch->readDual() : !homeSwitch->read()) {
        motor->runContinuous();
        updateOTA(); // Allow OTA updates during homing
        delay(1);
    }
    
    // Stop motor
    motor->stopContinuous();
    
    // Move 0.5 inches away from home
    motor->moveInches(0.5);
    while (motor->isMotorRunning()) {
        updateOTA(); // Allow OTA updates during homing
        delay(1);
    }
    
    // Restore full speed for normal operations
    motor->setSpeed(maxSpeed);
}

// Home X axis
void homeXAxis() {
    homeSingleAxis(motorX, homeSwitchX, true, X_MAX_SPEED);  // X uses dual switches
}

// Home Y axis
void homeYAxis() {
    homeSingleAxis(motorY, homeSwitchY, false, Y_MAX_SPEED);
}

// Home Fork axis
void homeForkAxis() {
    homeSingleAxis(motorFork, homeSwitchFork, false, FORK_MAX_SPEED);
}

// Home all axes
void homeAllAxes() {
    if (!motorX || !homeSwitchX || !motorY || !homeSwitchY || !motorFork || !homeSwitchFork) return;
    
    // Track which axes are still homing
    bool xHomed = false;
    bool yHomed = false;
    bool forkHomed = false;
    
    // Set homing speeds to 1/4 of max speeds
    motorX->setSpeed(X_MAX_SPEED / 4);
    motorY->setSpeed(Y_MAX_SPEED / 4);
    motorFork->setSpeed(FORK_MAX_SPEED / 4);
    
    // Set all directions to move toward home (negative direction)
    motorX->setDirection(false);
    motorY->setDirection(false);
    motorFork->setDirection(false);
    
    // Start continuous movement for all axes
    motorX->startContinuous();
    motorY->startContinuous();
    motorFork->startContinuous();
    
    // Keep moving until all home switches are triggered
    while (!xHomed || !yHomed || !forkHomed) {
        // Run all motors continuously
        if (!xHomed) {
            motorX->runContinuous();
        }
        if (!yHomed) {
            motorY->runContinuous();
        }
        if (!forkHomed) {
            motorFork->runContinuous();
        }
        
        // Check switches and stop motors when triggered
        if (!xHomed && homeSwitchX->readDual()) {
            motorX->stopContinuous();
            xHomed = true;
        }
        if (!yHomed && homeSwitchY->read()) {
            motorY->stopContinuous();
            yHomed = true;
        }
        if (!forkHomed && homeSwitchFork->read()) {
            motorFork->stopContinuous();
            forkHomed = true;
        }
        
        updateOTA(); // Allow OTA updates during homing
        delay(1);
    }
    
    // Move all axes 0.5 inches away from home simultaneously
    motorX->moveInches(0.5);
    motorY->moveInches(0.5);
    motorFork->moveInches(0.2);
    
    // Wait for all motors to finish
    while (motorX->isMotorRunning() || motorY->isMotorRunning() || motorFork->isMotorRunning()) {
        updateOTA(); // Allow OTA updates during homing
        delay(1);
    }
    
    // Set home offset as position zero for all axes
    motorX->resetPosition();
    motorY->resetPosition();
    motorFork->resetPosition();
    
    // Restore full speeds for normal operations
    motorX->setSpeed(X_MAX_SPEED);
    motorY->setSpeed(Y_MAX_SPEED);
    motorFork->setSpeed(FORK_MAX_SPEED);
}

