#include "../include/StateMachine/FUNCTIONS/StepperMotor.h"

// Global FastAccelStepperEngine instance (shared across all motors)
FastAccelStepperEngine* engine = nullptr;

StepperMotor::StepperMotor(uint8_t step, uint8_t dir, float stepsPerInch, long maxSpd, long maxAcc) {
    this->stepsPerInch = stepsPerInch;
    this->maxSpeed = maxSpd;
    this->maxAccel = maxAcc;
    this->continuousMode = false;
    this->continuousDirection = true;
    this->stepper = nullptr;

    // Initialize engine once (shared across all motors)
    if (engine == nullptr) {
        engine = new FastAccelStepperEngine();
        engine->init();
    }

    // Connect stepper to step pin
    if (engine) {
        stepper = engine->stepperConnectToPin(step);
        if (stepper) {
            stepper->setDirectionPin(dir, false);  // false = direction pin is not inverted
            stepper->setSpeedInHz(maxSpd);
            stepper->setAcceleration(maxAcc);
            stepper->setCurrentPosition(0);
        }
    }
}

// Direction is now set only in startContinuous() - no separate setDirection() needed

void StepperMotor::setSpeed(long speed) {
    if (stepper) {
        stepper->setSpeedInHz(speed);
        maxSpeed = speed; // Update max speed to allow future changes
    }
}

void StepperMotor::setAcceleration(long acceleration) {
    if (stepper) {
        stepper->setAcceleration(acceleration);
        maxAccel = acceleration; // Update max accel to allow future changes
    }
}

void StepperMotor::overrideSpeed(long speed) {
    if (stepper) {
        // setSpeedInHz can be called during movement to change speed smoothly
        stepper->setSpeedInHz(speed);
        maxSpeed = speed; // Update max speed
    }
}

void StepperMotor::step() {
    // Single step not typically used with FastAccelStepper
    // This is kept for compatibility but may not work as expected
    if (stepper) {
        if (continuousDirection) {
            stepper->move(1);
        } else {
            stepper->move(-1);
        }
    }
}

void StepperMotor::moveSteps(long steps) {
    if (stepper && steps != 0) {
        // Ensure we're not in continuous mode
        if (continuousMode) {
            stopContinuous();
        }
        continuousMode = false;
        // Start the move - FastAccelStepper handles the rest
        stepper->move(steps);
    } else if (stepper && steps == 0) {
        // If steps is 0, stop motor
        stepper->stopMove();
        continuousMode = false;
    }
}

void StepperMotor::moveStepsSmooth(long steps) {
    if (stepper && steps != 0) {
        // Transition smoothly from continuous mode to decelerated move
        // FastAccelStepper will handle the smooth transition automatically
        continuousMode = false;
        stepper->move(steps);
    } else if (stepper && steps == 0) {
        // If steps is 0, stop motor
        stepper->stopMove();
        continuousMode = false;
    }
}

void StepperMotor::moveInches(float inches) {
    long steps = inchesToSteps(inches);
    moveSteps(steps);
}

long StepperMotor::getCurrentPosition() {
    if (stepper) {
        return stepper->getCurrentPosition();
    }
    return 0;
}

void StepperMotor::setCurrentPosition(long position) {
    if (stepper) {
        stepper->setCurrentPosition(position);
    }
}

void StepperMotor::resetPosition() {
    setCurrentPosition(0);
}

void StepperMotor::forceStop() {
    if (stepper) {
        stepper->forceStopAndNewPosition(stepper->getCurrentPosition());
    }
    continuousMode = false;
}

bool StepperMotor::isMotorRunning() {
    if (stepper) {
        return stepper->isRunning();
    }
    return false;
}

long StepperMotor::inchesToSteps(float inches) {
    return (long)(inches * stepsPerInch);
}

float StepperMotor::stepsToInches(long steps) {
    return (float)steps / stepsPerInch;
}

void StepperMotor::setStepsPerInch(float stepsPerInch) {
    this->stepsPerInch = stepsPerInch;
}

void StepperMotor::startContinuous(bool positive) {
    continuousMode = true;
    continuousDirection = positive;  // Direction set here - single location
    if (stepper) {
        if (continuousDirection) {
            stepper->runForward();
        } else {
            stepper->runBackward();
        }
    }
}

void StepperMotor::stopContinuous() {
    continuousMode = false;
    if (stepper) {
        stepper->forceStopAndNewPosition(stepper->getCurrentPosition());
    }
}

void StepperMotor::runContinuous() {
    // FastAccelStepper handles continuous movement automatically
    // Just need to ensure it's still running in the correct direction
    if (continuousMode && stepper) {
        if (!stepper->isRunning()) {
            // Restart if it stopped
            if (continuousDirection) {
                stepper->runForward();
            } else {
                stepper->runBackward();
            }
        }
    }
}
