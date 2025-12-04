#ifndef STEPPER_MOTOR_H
#define STEPPER_MOTOR_H

#include <Arduino.h>
#include <FastAccelStepper.h>
#include "../../../src/config/Config.h"

class StepperMotor {
private:
    // FastAccelStepper object
    FastAccelStepper* stepper;
    
    // Motor parameters
    float stepsPerInch;
    long maxSpeed;
    long maxAccel;
    
    // Current state
    bool continuousMode;  // true when continuously moving
    bool continuousDirection;  // true = positive, false = negative

public:
    // Constructor
    StepperMotor(uint8_t step, uint8_t dir, float stepsPerInch, long maxSpd, long maxAcc);

    // Basic motor control
    void setSpeed(long speed);
    void setAcceleration(long acceleration);
    void overrideSpeed(long speed);  // Change speed during movement (smooth deceleration)
    void step();
    void moveSteps(long steps);
    void moveStepsSmooth(long steps);  // Transitions smoothly from continuous mode without force stop
    void moveInches(float inches);

    // Continuous movement control
    void startContinuous(bool positive = true);  // Direction is set here
    void stopContinuous();
    void runContinuous();  // Call this repeatedly to keep moving

    // Position tracking
    long getCurrentPosition();
    void setCurrentPosition(long position);
    void resetPosition();

    // Force stop functionality
    void forceStop();

    // Status
    bool isMotorRunning();

    // Movement calculations
    long inchesToSteps(float inches);
    float stepsToInches(long steps);
    
    // Update steps per inch
    void setStepsPerInch(float stepsPerInch);
};

#endif // STEPPER_MOTOR_H
