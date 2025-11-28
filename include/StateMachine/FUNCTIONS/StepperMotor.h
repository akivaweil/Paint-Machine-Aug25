#ifndef STEPPER_MOTOR_H
#define STEPPER_MOTOR_H

#include <FastAccelStepper.h>
#include "../../../src/config/Config.h"
#include "../../../src/config/Pin_Definitions.h"

// Motor types
enum MotorType {
    MOTOR_X,
    MOTOR_Y,
    MOTOR_FORK,
    MOTOR_STORAGE
};

class StepperMotor {
private:
    FastAccelStepper* stepper;
    MotorType motorType;
    int stepPin;
    int dirPin;
    float stepsPerInch;
    long maxSpeed;
    long maxAccel;

    // Convert inches to steps
    long inchesToSteps(float inches);

public:
    // Constructor
    StepperMotor(MotorType type);

    // Initialize the motor
    void init();

    // Movement functions
    void moveToPositionInches(float inches);
    void moveRelativeInches(float inches);
    void setSpeed(float speedStepsPerSec);
    void setAcceleration(float accelStepsPerSec2);
    void stop();
    void emergencyStop();

    // Status functions
    bool isRunning();
    long getCurrentPosition();
    void setCurrentPosition(long position);

    // Home switch detection (will be implemented in HomeSwitch class)
    bool isAtHome();

    // Get stepper pointer for advanced operations
    FastAccelStepper* getStepper() { return stepper; }
};

#endif // STEPPER_MOTOR_H
