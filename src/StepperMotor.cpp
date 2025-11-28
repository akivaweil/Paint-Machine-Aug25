#include "../include/StateMachine/FUNCTIONS/StepperMotor.h"
#include <Arduino.h>

// Constructor
StepperMotor::StepperMotor(MotorType type) : motorType(type), stepper(nullptr) {
    // Set pin assignments and parameters based on motor type
    switch (motorType) {
        case MOTOR_X:
            stepPin = X_STEP_PIN;
            dirPin = X_DIR_PIN;
            stepsPerInch = X_STEPS_PER_INCH;
            maxSpeed = X_MAX_SPEED;
            maxAccel = X_MAX_ACCEL;
            break;
        case MOTOR_Y:
            stepPin = Y_STEP_PIN;
            dirPin = Y_DIR_PIN;
            stepsPerInch = STEPS_PER_INCH;
            maxSpeed = Y_MAX_SPEED;
            maxAccel = Y_MAX_ACCEL;
            break;
        case MOTOR_FORK:
            stepPin = FORK_STEP_PIN;
            dirPin = FORK_DIR_PIN;
            stepsPerInch = STEPS_PER_INCH;
            maxSpeed = FORK_MAX_SPEED;
            maxAccel = FORK_MAX_ACCEL;
            break;
        case MOTOR_STORAGE:
            stepPin = STORAGE_STEP_PIN;
            dirPin = STORAGE_DIR_PIN;
            stepsPerInch = STEPS_PER_INCH;
            maxSpeed = STORAGE_MOTOR_SPEED;
            maxAccel = STORAGE_MOTOR_ACCEL;
            break;
    }
}

// Initialize the motor
void StepperMotor::init() {
    // Create stepper engine if not exists
    static FastAccelStepperEngine engine = FastAccelStepperEngine();

    // Initialize engine if not already done
    static bool engineInitialized = false;
    if (!engineInitialized) {
        engine.init();
        engineInitialized = true;
    }

    // Create stepper instance
    stepper = engine.stepperConnectToPin(stepPin);
    if (stepper) {
        stepper->setDirectionPin(dirPin);
        stepper->setSpeedInHz(maxSpeed);
        stepper->setAcceleration(maxAccel);
        stepper->setCurrentPosition(0);
    }
}

// Convert inches to steps
long StepperMotor::inchesToSteps(float inches) {
    return (long)(inches * stepsPerInch);
}

// Movement functions
void StepperMotor::moveToPositionInches(float inches) {
    if (stepper) {
        long steps = inchesToSteps(inches);
        stepper->moveTo(steps);
    }
}

void StepperMotor::moveRelativeInches(float inches) {
    if (stepper) {
        long steps = inchesToSteps(inches);
        stepper->move(steps);
    }
}

void StepperMotor::setSpeed(float speedStepsPerSec) {
    if (stepper) {
        stepper->setSpeedInHz(speedStepsPerSec);
    }
}

void StepperMotor::setAcceleration(float accelStepsPerSec2) {
    if (stepper) {
        stepper->setAcceleration(accelStepsPerSec2);
    }
}

void StepperMotor::stop() {
    if (stepper) {
        stepper->stopMove();
    }
}

void StepperMotor::emergencyStop() {
    if (stepper) {
        stepper->forceStop();
    }
}

// Status functions
bool StepperMotor::isRunning() {
    return stepper ? stepper->isRunning() : false;
}

long StepperMotor::getCurrentPosition() {
    return stepper ? stepper->getCurrentPosition() : 0;
}

void StepperMotor::setCurrentPosition(long position) {
    if (stepper) {
        stepper->setCurrentPosition(position);
    }
}

// Home switch detection placeholder (will be implemented properly with HomeSwitch class)
bool StepperMotor::isAtHome() {
    // This will be implemented in the HomeSwitch class
    return false;
}