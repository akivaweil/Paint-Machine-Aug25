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

// Home X axis
void homeXAxis() {
    if (!motorX || !homeSwitchX) return;
    
    // Set direction to move toward home (positive direction)
    motorX->setDirection(true);
    
    // Start continuous movement toward home
    motorX->startContinuous();
    
    // Keep moving until home switch is triggered
    while (!homeSwitchX->readDual()) {
        motorX->runContinuous();
        delay(1);
    }
    
    // Stop and reset position
    motorX->stopContinuous();
    motorX->resetPosition();
    
    // Move 0.5 inches away from home
    motorX->moveInches(-0.5);
    while (motorX->isMotorRunning()) {
        delay(1);
    }
}

// Home Y axis
void homeYAxis() {
    if (!motorY || !homeSwitchY) return;
    
    // Set direction to move toward home (positive direction)
    motorY->setDirection(true);
    
    // Start continuous movement toward home
    motorY->startContinuous();
    
    // Keep moving until home switch is triggered
    while (!homeSwitchY->read()) {
        motorY->runContinuous();
        delay(1);
    }
    
    // Stop and reset position
    motorY->stopContinuous();
    motorY->resetPosition();
    
    // Move 0.5 inches away from home
    motorY->moveInches(-0.5);
    while (motorY->isMotorRunning()) {
        delay(1);
    }
}

// Home Fork axis
void homeForkAxis() {
    if (!motorFork || !homeSwitchFork) return;
    
    // Set direction to move toward home (positive direction)
    motorFork->setDirection(true);
    
    // Start continuous movement toward home
    motorFork->startContinuous();
    
    // Keep moving until home switch is triggered
    while (!homeSwitchFork->read()) {
        motorFork->runContinuous();
        delay(1);
    }
    
    // Stop and reset position
    motorFork->stopContinuous();
    motorFork->resetPosition();
    
    // Move 0.5 inches away from home
    motorFork->moveInches(-0.5);
    while (motorFork->isMotorRunning()) {
        delay(1);
    }
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

