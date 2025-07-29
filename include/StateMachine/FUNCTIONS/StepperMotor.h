#ifndef STEPPER_MOTOR_H
#define STEPPER_MOTOR_H

//* ************************************************************************
//* ************************ STEPPER MOTOR CONTROL *************************
//* ************************************************************************

#include <Arduino.h>
#include <FastAccelStepper.h>
#include "config/Config.h"
#include "config/Pin_Definitions.h"

class StepperMotor {
public:
    // Constructor
    StepperMotor(int stepPin, int dirPin, int homePin, int limitPin, const char* axisName);
    
    // Initialize the motor
    void initialize();
    
    // Home the motor
    void home();
    
    // Force stop the motor (emergency stop)
    void forceStop();
    
    // Check if motor is moving
    bool isMoving();
    
    // Set current position as zero
    void setCurrentPositionAsZero();
    
    // Check if home switch is triggered
    bool isHomeSwitchTriggered();
    
    // Check if limit switch is triggered
    bool isLimitSwitchTriggered();
    
    // Update motor (call in main loop)
    void update();

private:
    // Motor pins
    int _stepPin;
    int _dirPin;
    int _homePin;
    int _limitPin;
    
    // Motor properties
    const char* _axisName;
    float _currentPosition;
    bool _isMoving;
    
    // FastAccelStepper stepper
    FastAccelStepper* _stepper;
    
    // Helper functions for individual motor homing settings
    float getHomingSpeed();
    float getHomingAcceleration();
    long getHomingDistance();
    
    // Convert inches to steps
    long inchesToSteps(float inches);
    
    // Convert steps to inches
    float stepsToInches(long steps);
};

#endif // STEPPER_MOTOR_H 