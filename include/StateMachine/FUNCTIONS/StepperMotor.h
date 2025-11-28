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
    void setDirection(bool positive);
    void setSpeed(long speed);
    void step();
    void moveSteps(long steps);
    void moveInches(float inches);

    // Continuous movement control
    void startContinuous();
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
};

#endif // STEPPER_MOTOR_H
