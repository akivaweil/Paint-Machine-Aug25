#include "../../../include/StateMachine/FUNCTIONS/StorageMotor.h"
#include "../../config/Config.h"

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ ⚙️ STORAGE MOTOR CONTROLLER (AccelStepper)                           ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

StorageMotor::StorageMotor(uint8_t step, uint8_t dir, float stepsPerInch, long maxSpd, long maxAcc) {
    this->stepsPerInch = stepsPerInch;
    this->maxSpeed = maxSpd;
    this->maxAccel = maxAcc;
    this->continuousMode = false;
    this->continuousDirection = true;
    this->rampStep = 0;
    this->lastRampTime = 0;
    
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
    rampStep = 0;
    lastRampTime = millis();
    if (stepper) {
        // Start at lower speed to prevent brownout, will ramp up in runContinuous()
        long startSpeed = STORAGE_MOTOR_CONTINUOUS_START_SPEED;
        if (positive) {
            stepper->setSpeed(startSpeed);
            stepper->runSpeed();
        } else {
            stepper->setSpeed(-startSpeed);
            stepper->runSpeed();
        }
    }
}

void StorageMotor::stopContinuous() {
    continuousMode = false;
    rampStep = 0;
    lastRampTime = 0;
    if (stepper) {
        stepper->stop();
        stepper->setCurrentPosition(stepper->currentPosition());
    }
}

void StorageMotor::runContinuous() {
    if (continuousMode && stepper) {
        // Gradually ramp up speed to prevent brownout
        unsigned long currentTime = millis();
        
        // Ramp up speed every 10ms
        if (currentTime - lastRampTime >= 10) {
            if (rampStep < STORAGE_MOTOR_CONTINUOUS_RAMP_STEPS) {
                rampStep++;
                lastRampTime = currentTime;
            }
        }
        
        // Calculate current speed (ramp from start speed to max speed)
        long currentSpeed = STORAGE_MOTOR_CONTINUOUS_START_SPEED;
        if (rampStep > 0) {
            long speedRange = maxSpeed - STORAGE_MOTOR_CONTINUOUS_START_SPEED;
            currentSpeed = STORAGE_MOTOR_CONTINUOUS_START_SPEED + 
                          (speedRange * rampStep / STORAGE_MOTOR_CONTINUOUS_RAMP_STEPS);
        }
        
        if (continuousDirection) {
            stepper->setSpeed(currentSpeed);
        } else {
            stepper->setSpeed(-currentSpeed);
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

