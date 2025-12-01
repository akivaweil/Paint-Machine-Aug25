#include <Arduino.h>
#include "StateMachine/STATES/01_HOMING.h"

// External motor and switch instances (defined in Web_Manager.cpp)
extern StepperMotor* motorX;
extern StepperMotor* motorY;
extern StepperMotor* motorFork;
extern HomeSwitch* homeSwitchX;
extern HomeSwitch* homeSwitchY;
extern HomeSwitch* homeSwitchFork;

//* ************************************************************************
//* ************************ HOMING STATE *********************************
//* ************************************************************************

void homingState() {
    // State implementation will go here
}

// Helper function to home a single axis
static void homeSingleAxis(StepperMotor* motor, HomeSwitch* homeSwitch, bool useDual) {
    if (!motor || !homeSwitch) return;
    
    // Set direction to move toward home (positive direction)
    motor->setDirection(true);
    
    // Start continuous movement toward home
    motor->startContinuous();
    
    // Keep moving until home switch is triggered
    while (useDual ? !homeSwitch->readDual() : !homeSwitch->read()) {
        motor->runContinuous();
        delay(1);
    }
    
    // Stop and reset position
    motor->stopContinuous();
    motor->resetPosition();
    
    // Move 0.5 inches away from home
    motor->moveInches(-0.5);
    while (motor->isMotorRunning()) {
        delay(1);
    }
}

// Home X axis
void homeXAxis() {
    homeSingleAxis(motorX, homeSwitchX, true);  // X uses dual switches
}

// Home Y axis
void homeYAxis() {
    homeSingleAxis(motorY, homeSwitchY, false);
}

// Home Fork axis
void homeForkAxis() {
    homeSingleAxis(motorFork, homeSwitchFork, false);
}

// Home all axes
void homeAllAxes() {
    if (!motorX || !homeSwitchX || !motorY || !homeSwitchY || !motorFork || !homeSwitchFork) return;
    
    // Track which axes are still homing
    bool xHomed = false;
    bool yHomed = false;
    bool forkHomed = false;
    
    // Set all directions to move toward home (positive direction)
    motorX->setDirection(true);
    motorY->setDirection(true);
    motorFork->setDirection(true);
    
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
            motorX->resetPosition();
            xHomed = true;
        }
        if (!yHomed && homeSwitchY->read()) {
            motorY->stopContinuous();
            motorY->resetPosition();
            yHomed = true;
        }
        if (!forkHomed && homeSwitchFork->read()) {
            motorFork->stopContinuous();
            motorFork->resetPosition();
            forkHomed = true;
        }
        
        delay(1);
    }
    
    // Move all axes 0.5 inches away from home simultaneously
    motorX->moveInches(-0.5);
    motorY->moveInches(-0.5);
    motorFork->moveInches(-0.5);
    
    // Wait for all motors to finish
    while (motorX->isMotorRunning() || motorY->isMotorRunning() || motorFork->isMotorRunning()) {
        delay(1);
    }
}

