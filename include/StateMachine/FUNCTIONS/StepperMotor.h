#ifndef STEPPER_MOTOR_H
#define STEPPER_MOTOR_H

//* ************************************************************************
//* ************************ STEPPER MOTOR CONTROL *************************
//* ************************************************************************

#include <Arduino.h>
#include <FastAccelStepper.h>
#include <Bounce2.h>
#include "config/Config.h"
#include "config/Pin_Definitions.h"

class StepperMotor {
public:
    // Constructor
    StepperMotor(int stepPin, int dirPin, int homePin, const char* axisName);
    
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
    
    // Set current position to a specific value (in inches)
    void setCurrentPosition(float position);
    
    // Check if home switch is triggered
    bool isHomeSwitchTriggered();
    
    // Update motor (call in main loop)
    void update();
    
    // Update motor during homing (checks home switches)
    void updateHoming();
    
    // Update switch debouncing
    void updateSwitches();
    
    // Move to specific position (in inches)
    void moveToPosition(float position);
    
    // Get current position
    float getCurrentPosition();
    
    // Check if homing is complete
    bool isHomingComplete();
    
    // Move away from home position with optional distance
    // If distance is 0.0 (default), uses configuration default
    void moveAwayFromHome(float distance = 0.0);
    
    // Test move away direction (prints direction without moving)
    void testMoveAwayDirection();
    
    // Start continuous movement (for storage motor spinning)
    void startContinuousMovement(float speed);
    
    // Stop continuous movement
    void stopContinuousMovement();

private:
    // Motor pins
    int _stepPin;
    int _dirPin;
    int _homePin;
    
    // Motor properties
    const char* _axisName;
    float _currentPosition;
    bool _isMoving;
    
    // FastAccelStepper stepper
    FastAccelStepper* _stepper;
    
    // Bounce2 debouncing objects
    Bounce _homeSwitchBounce;
    
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
