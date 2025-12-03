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
    
    // Set homing speed to 700
    motor->setSpeed(700);
    
    // Start continuous movement toward home (positive direction)
    motor->startContinuous(true);
    
    // Keep moving until home switch is triggered
    while (useDual ? !homeSwitch->readDual() : !homeSwitch->read()) {
        motor->runContinuous();
        updateOTA(); // Allow OTA updates during homing
        delay(1);
    }
    
    // Stop motor
    motor->stopContinuous();
    
    // Move 0.2 inches away from home
    motor->moveInches(-0.2);
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

// Home Fork Motor axis
void homeForkAxis() {
    if (!motorFork || !homeSwitchFork) return;
    
    // If already homed, move 0.3 inches away from home first
    if (homeSwitchFork->read()) {
        // Set homing speed to 700
        motorFork->setSpeed(700);
        
        // Move 0.3 inches away from home (negative direction)
        motorFork->moveInches(-0.2);
        while (motorFork->isMotorRunning()) {
            updateOTA(); // Allow OTA updates during homing
            delay(1);
        }
    }
    
    // Set homing speed to 700
    motorFork->setSpeed(700);
    
    // Start continuous movement toward home (positive direction)
    motorFork->startContinuous(true);
    
    // Keep moving until home switch is triggered
    while (!homeSwitchFork->read()) {
        motorFork->runContinuous();
        updateOTA(); // Allow OTA updates during homing
        delay(1);
    }
    
    // Stop motor
    motorFork->stopContinuous();
    
    // Move 0 inches away from home (no offset)
    motorFork->moveInches(0);
    while (motorFork->isMotorRunning()) {
        updateOTA(); // Allow OTA updates during homing
        delay(1);
    }
    
    // Set home offset as position zero
    motorFork->resetPosition();
    
    // Restore full speed for normal operations
    motorFork->setSpeed(FORK_MAX_SPEED);
}

// Home all axes
void homeAllAxes() {
    if (!motorX || !homeSwitchX || !motorY || !homeSwitchY || !motorFork || !homeSwitchFork) return;
    
    //! ************************************************************************
    //! STEP 1: HOME FORK MOTOR FIRST
    //! ************************************************************************
    homeForkAxis();
    
    //! ************************************************************************
    //! STEP 2: HOME X AND Y AXES SIMULTANEOUSLY
    //! ************************************************************************
    // Track which axes are still homing
    bool xHomed = false;
    bool yHomed = false;
    
    // Set homing speeds to 700
    motorX->setSpeed(700);
    motorY->setSpeed(700);
    
    // Start continuous movement for X and Y toward home (positive direction)
    motorX->startContinuous(true);
    motorY->startContinuous(true);
    
    // Keep moving until both home switches are triggered
    while (!xHomed || !yHomed) {
        // Run both motors continuously
        if (!xHomed) {
            motorX->runContinuous();
        }
        if (!yHomed) {
            motorY->runContinuous();
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
        
        updateOTA(); // Allow OTA updates during homing
        delay(1);
    }
    
    // Move X and Y axes 0.5 inches away from home simultaneously
    motorX->moveInches(-0.5);
    motorY->moveInches(-0.5);
    
    // Wait for both motors to finish
    while (motorX->isMotorRunning() || motorY->isMotorRunning()) {
        updateOTA(); // Allow OTA updates during homing
        delay(1);
    }
    
    // Set home offset as position zero for X and Y axes
    motorX->resetPosition();
    motorY->resetPosition();
    
    // Restore full speeds for normal operations
    motorX->setSpeed(X_MAX_SPEED);
    motorY->setSpeed(Y_MAX_SPEED);
}

