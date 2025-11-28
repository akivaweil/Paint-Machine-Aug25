#include "../include/StateMachine/FUNCTIONS/StepperMotor.h"

StepperMotor::StepperMotor(uint8_t step, uint8_t dir, float stepsPerInch, long maxSpd, long maxAcc) {
    this->stepPin = step;
    this->dirPin = dir;
    this->stepsPerInch = stepsPerInch;
    this->maxSpeed = maxSpd;
    this->maxAccel = maxAcc;

    // Initialize pins
    pinMode(stepPin, OUTPUT);
    pinMode(dirPin, OUTPUT);

    // Initialize state
    currentPosition = 0;
    isRunning = false;
    direction = true;  // default to positive
    continuousMode = false;
    lastStepTime = 0;
    stepInterval = 1000000 / maxSpeed;  // microseconds per step
}

void StepperMotor::setDirection(bool positive) {
    direction = positive;
    digitalWrite(dirPin, positive ? HIGH : LOW);
}

void StepperMotor::setSpeed(long speed) {
    if (speed > 0 && speed <= maxSpeed) {
        stepInterval = 1000000 / speed;  // microseconds per step
    }
}

void StepperMotor::step() {
    unsigned long currentTime = micros();

    if (currentTime - lastStepTime >= stepInterval) {
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(1);  // Brief pulse
        digitalWrite(stepPin, LOW);

        // Update position based on direction
        currentPosition += direction ? 1 : -1;
        lastStepTime = currentTime;
    }
}

void StepperMotor::moveSteps(long steps) {
    if (steps == 0) return;

    bool moveDirection = (steps > 0);
    setDirection(moveDirection);
    long stepsToMove = abs(steps);

    for (long i = 0; i < stepsToMove; i++) {
        step();
        delayMicroseconds(stepInterval / 2);  // Wait between steps
    }

    isRunning = false;
}

void StepperMotor::moveInches(float inches) {
    long steps = inchesToSteps(inches);
    moveSteps(steps);
}

long StepperMotor::getCurrentPosition() {
    return currentPosition;
}

void StepperMotor::setCurrentPosition(long position) {
    currentPosition = position;
}

void StepperMotor::resetPosition() {
    currentPosition = 0;
}

void StepperMotor::forceStop() {
    continuousMode = false;
    isRunning = false;
    // Immediately stop any ongoing movement by setting pins low
    digitalWrite(stepPin, LOW);
}

bool StepperMotor::isMotorRunning() {
    return isRunning;
}

long StepperMotor::inchesToSteps(float inches) {
    return (long)(inches * stepsPerInch);
}

float StepperMotor::stepsToInches(long steps) {
    return (float)steps / stepsPerInch;
}

void StepperMotor::startContinuous() {
    continuousMode = true;
    isRunning = true;
}

void StepperMotor::stopContinuous() {
    continuousMode = false;
    isRunning = false;
    digitalWrite(stepPin, LOW);  // Ensure step pin is low
}

void StepperMotor::runContinuous() {
    if (continuousMode) {
        step();
    }
}
