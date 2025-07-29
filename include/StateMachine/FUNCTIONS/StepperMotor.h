#ifndef STEPPER_MOTOR_H
#define STEPPER_MOTOR_H

//* ************************************************************************
//* ************************ STEPPER MOTOR CONTROL *************************
//* ************************************************************************

#include <Arduino.h>
#include "config/Config.h"
#include "config/Pin_Definitions.h"

class StepperMotor {
public:
    // Constructor
    StepperMotor(int stepPin, int dirPin, int enablePin, int homePin, int limitPin, const char* axisName);
    
    // Initialize the motor
    void initialize();
    
    // Enable/disable the motor
    void enable();
    void disable();
    
    // Move to absolute position
    void moveTo(float position);
    
    // Move relative distance
    void moveRelative(float distance);
    
    // Home the motor
    void home();
    
    // Stop the motor
    void stop();
    
    // Check if motor is moving
    bool isMoving();
    
    // Get current position
    float getCurrentPosition();
    
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
    int _enablePin;
    int _homePin;
    int _limitPin;
    
    // Motor properties
    const char* _axisName;
    float _currentPosition;
    float _targetPosition;
    bool _isMoving;
    bool _isEnabled;
    
    // Movement timing
    unsigned long _lastStepTime;
    unsigned long _stepInterval;
    
    // Calculate step interval based on speed
    void calculateStepInterval(float speed);
    
    // Execute one step
    void step();
    
    // Set direction
    void setDirection(bool forward);
};

#endif // STEPPER_MOTOR_H 