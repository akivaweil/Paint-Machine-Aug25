#ifndef STEPPER_MOTOR_H
#define STEPPER_MOTOR_H

#include <Arduino.h>
#include "../../../src/config/Config.h"

class StepperMotor {
private:
    // Pin definitions
    uint8_t stepPin;
    uint8_t dirPin;

    // Motor parameters
    float stepsPerInch;
    long maxSpeed;
    long maxAccel;

    // Current state
    bool direction;  // true = positive, false = negative
    long currentPosition;  // in steps
    bool isRunning;
    bool continuousMode;  // true when continuously moving

    // Timing for step generation
    unsigned long lastStepTime;
    unsigned long stepInterval;

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
