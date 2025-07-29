#include "StateMachine/FUNCTIONS/StepperMotor.h"
#include "config/Homing_Config.h"

//* ************************************************************************
//* ************************ STEPPER MOTOR IMPLEMENTATION ******************
//* ************************************************************************

StepperMotor::StepperMotor(int stepPin, int dirPin, int homePin, int limitPin, const char* axisName) {
    _stepPin = stepPin;
    _dirPin = dirPin;
    _homePin = homePin;
    _limitPin = limitPin;
    _axisName = axisName;
    
    // Initialize motor properties
    _currentPosition = 0.0;
    _isMoving = false;
    
    // Initialize FastAccelStepper pointer
    _stepper = nullptr;
}

void StepperMotor::initialize() {
    // Set pin modes for switches
    pinMode(_homePin, INPUT_PULLDOWN);
    pinMode(_limitPin, INPUT_PULLUP);
    
    // Initialize FastAccelStepper engine (only once)
    static FastAccelStepperEngine* engine = nullptr;
    if (!engine) {
        engine = new FastAccelStepperEngine();
        engine->init();
    }
    
    // Create stepper instance using FastAccelStepper
    _stepper = engine->stepperConnectToPin(_stepPin);
    if (_stepper) {
        _stepper->setDirectionPin(_dirPin);
        
        // Set default acceleration and speed (will be overridden during homing)
        _stepper->setAcceleration(MAX_ACCEL);
        _stepper->setSpeedInHz(MAX_SPEED);
        
        Serial.print(_axisName);
        Serial.println(" motor initialized");
    } else {
        Serial.print("ERROR: Failed to initialize ");
        Serial.println(_axisName);
    }
}

void StepperMotor::home() {
    if (!_stepper) {
        Serial.print("ERROR: ");
        Serial.print(_axisName);
        Serial.println(" stepper not initialized");
        return;
    }
    
    // Set individual homing speed and acceleration based on motor type
    float homeSpeed = getHomingSpeed();
    float homeAccel = getHomingAcceleration();
    long homeDistance = getHomingDistance();
    
    // Move towards home switch at individual homing speed
    _isMoving = true;
    
    // Set individual homing speed and acceleration
    _stepper->setAcceleration(homeAccel);
    _stepper->setSpeedInHz(homeSpeed);
    
    // Move towards home using individual distance and direction
    _stepper->moveTo(homeDistance);
    
    Serial.print(_axisName);
    Serial.print(" starting homing sequence (Speed: ");
    Serial.print(homeSpeed);
    Serial.print(" steps/s, Accel: ");
    Serial.print(homeAccel);
    Serial.println(" steps/s²)");
}

void StepperMotor::forceStop() {
    if (_stepper) {
        _stepper->forceStop();
    }
    _isMoving = false;
    
    Serial.print(_axisName);
    Serial.println(" stopped");
}

bool StepperMotor::isMoving() {
    if (_stepper) {
        return _stepper->isRunning();
    }
    return _isMoving;
}

void StepperMotor::setCurrentPositionAsZero() {
    if (_stepper) {
        _stepper->setCurrentPosition(0);
    }
    _currentPosition = 0.0;
    
    Serial.print(_axisName);
    Serial.println(" position set to zero");
}

bool StepperMotor::isHomeSwitchTriggered() {
    return digitalRead(_homePin) == HIGH; // Active HIGH
}

bool StepperMotor::isLimitSwitchTriggered() {
    return digitalRead(_limitPin) == LOW; // Active LOW
}

void StepperMotor::update() {
    if (!_stepper) return;
    
    // Update current position from stepper
    _currentPosition = stepsToInches(_stepper->getCurrentPosition());
    
    // Check if home switch is triggered during homing
    if (_isMoving && isHomeSwitchTriggered()) {
        _stepper->stopMove();
        _isMoving = false;
        _currentPosition = 0.0; // Set home position
        _stepper->setCurrentPosition(0);
        
        Serial.print(_axisName);
        Serial.println(" reached home switch");
        return;
    }
    
    // Check if limit switch is triggered
    if (isLimitSwitchTriggered()) {
        _stepper->stopMove();
        _isMoving = false;
        
        Serial.print(_axisName);
        Serial.println(" limit switch triggered - stopping");
        return;
    }
    
    // Check if movement is complete
    if (!_stepper->isRunning()) {
        _isMoving = false;
    }
}

// Helper functions to get individual motor homing settings
float StepperMotor::getHomingSpeed() {
    if (strcmp(_axisName, "X1") == 0) return X1_HOME_SPEED;
    if (strcmp(_axisName, "X2") == 0) return X2_HOME_SPEED;
    if (strcmp(_axisName, "Y") == 0) return Y_HOME_SPEED;
    if (strcmp(_axisName, "Fork") == 0) return FORK_HOME_SPEED;
    return HOME_SPEED; // Default fallback
}

float StepperMotor::getHomingAcceleration() {
    if (strcmp(_axisName, "X1") == 0) return X1_HOME_ACCEL;
    if (strcmp(_axisName, "X2") == 0) return X2_HOME_ACCEL;
    if (strcmp(_axisName, "Y") == 0) return Y_HOME_ACCEL;
    if (strcmp(_axisName, "Fork") == 0) return FORK_HOME_ACCEL;
    return HOME_ACCEL; // Default fallback
}

long StepperMotor::getHomingDistance() {
    long distance;
    if (strcmp(_axisName, "X1") == 0) distance = X1_HOME_DISTANCE_STEPS;
    else if (strcmp(_axisName, "X2") == 0) distance = X2_HOME_DISTANCE_STEPS;
    else if (strcmp(_axisName, "Y") == 0) distance = Y_HOME_DISTANCE_STEPS;
    else if (strcmp(_axisName, "Fork") == 0) distance = FORK_HOME_DISTANCE_STEPS;
    else distance = 10000; // Default fallback
    
    // Apply direction based on configuration
    if (strcmp(_axisName, "X1") == 0 && !X1_HOME_DIRECTION_POSITIVE) distance = -distance;
    else if (strcmp(_axisName, "X2") == 0 && !X2_HOME_DIRECTION_POSITIVE) distance = -distance;
    else if (strcmp(_axisName, "Y") == 0 && !Y_HOME_DIRECTION_POSITIVE) distance = -distance;
    else if (strcmp(_axisName, "Fork") == 0 && !FORK_HOME_DIRECTION_POSITIVE) distance = -distance;
    
    return distance;
}

long StepperMotor::inchesToSteps(float inches) {
    return (long)(inches * STEPS_PER_INCH);
}

float StepperMotor::stepsToInches(long steps) {
    return (float)steps / STEPS_PER_INCH;
}

void StepperMotor::moveToPosition(float position) {
    if (!_stepper) {
        Serial.print("ERROR: ");
        Serial.print(_axisName);
        Serial.println(" stepper not initialized");
        return;
    }
    
    // Convert position to steps
    long targetSteps = inchesToSteps(position);
    
    // Set normal operation speed and acceleration
    _stepper->setAcceleration(MAX_ACCEL);
    _stepper->setSpeedInHz(MAX_SPEED);
    
    // Move to target position
    _stepper->moveTo(targetSteps);
    _isMoving = true;
    
    Serial.print(_axisName);
    Serial.print(" moving to position: ");
    Serial.print(position);
    Serial.println(" inches");
}

float StepperMotor::getCurrentPosition() {
    return _currentPosition;
} 