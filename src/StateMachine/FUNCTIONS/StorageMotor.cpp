#include "../../../include/StateMachine/FUNCTIONS/StorageMotor.h"

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ ⚙️ STORAGE MOTOR CONTROLLER (AccelStepper)                           ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

StorageMotor::StorageMotor(uint8_t step, uint8_t dir, float stepsPerInch, long maxSpd, long maxAcc) {
    this->stepsPerInch = stepsPerInch;
    this->maxSpeed = maxSpd;
    this->maxAccel = maxAcc;
    this->continuousMode = false;
    this->continuousDirection = true;
    
    // Create AccelStepper instance (DRIVER mode: step and direction pins)
    stepper = new AccelStepper(AccelStepper::DRIVER, step, dir);
    stepper->setMaxSpeed(maxSpd);
    stepper->setAcceleration(maxAcc);
    stepper->setCurrentPosition(0);
}

void StorageMotor::setSpeed(long speed) {
    if (stepper) {
        stepper->setMaxSpeed(speed);
        maxSpeed = speed;
    }
}

void StorageMotor::setAcceleration(long acceleration) {
    if (stepper) {
        stepper->setAcceleration(acceleration);
        maxAccel = acceleration;
    }
}

void StorageMotor::overrideSpeed(long speed) {
    setSpeed(speed);
}

void StorageMotor::step() {
    if (stepper) {
        if (continuousDirection) {
            stepper->move(1);
        } else {
            stepper->move(-1);
        }
    }
}

void StorageMotor::moveSteps(long steps) {
    if (stepper && steps != 0) {
        if (continuousMode) {
            stopContinuous();
        }
        continuousMode = false;
        stepper->move(steps);
    } else if (stepper && steps == 0) {
        stepper->stop();
        continuousMode = false;
    }
}

void StorageMotor::moveStepsSmooth(long steps) {
    if (stepper && steps != 0) {
        continuousMode = false;
        stepper->move(steps);
    } else if (stepper && steps == 0) {
        stepper->stop();
        continuousMode = false;
    }
}

void StorageMotor::moveInches(float inches) {
    long steps = inchesToSteps(inches);
    moveSteps(steps);
}

long StorageMotor::getCurrentPosition() {
    if (stepper) {
        return stepper->currentPosition();
    }
    return 0;
}

void StorageMotor::setCurrentPosition(long position) {
    if (stepper) {
        stepper->setCurrentPosition(position);
    }
}

void StorageMotor::resetPosition() {
    setCurrentPosition(0);
}

void StorageMotor::forceStop() {
    if (stepper) {
        stepper->stop();
        stepper->setCurrentPosition(stepper->currentPosition());
    }
    continuousMode = false;
}

bool StorageMotor::isMotorRunning() {
    if (stepper) {
        return stepper->isRunning();
    }
    return false;
}

long StorageMotor::inchesToSteps(float inches) {
    return (long)(inches * stepsPerInch);
}

float StorageMotor::stepsToInches(long steps) {
    return (float)steps / stepsPerInch;
}

void StorageMotor::setStepsPerInch(float stepsPerInch) {
    this->stepsPerInch = stepsPerInch;
}

void StorageMotor::startContinuous(bool positive) {
    continuousMode = true;
    continuousDirection = positive;
    if (stepper) {
        if (positive) {
            stepper->setSpeed(maxSpeed);
            stepper->runSpeed();
        } else {
            stepper->setSpeed(-maxSpeed);
            stepper->runSpeed();
        }
    }
}

void StorageMotor::stopContinuous() {
    continuousMode = false;
    if (stepper) {
        stepper->stop();
        stepper->setCurrentPosition(stepper->currentPosition());
    }
}

void StorageMotor::runContinuous() {
    if (continuousMode && stepper) {
        if (continuousDirection) {
            stepper->setSpeed(maxSpeed);
        } else {
            stepper->setSpeed(-maxSpeed);
        }
        stepper->runSpeed();
    }
}

void StorageMotor::run() {
    if (stepper) {
        if (!continuousMode) {
            stepper->run();
        } else {
            runContinuous();
        }
    }
}

