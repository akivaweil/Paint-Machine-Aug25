#include "StateMachine/FUNCTIONS/StepperMotor.h"

//* ************************************************************************
//* ************************ STEPPER MOTOR IMPLEMENTATION ******************
//* ************************************************************************

StepperMotor::StepperMotor(int stepPin, int dirPin, int enablePin, int homePin, int limitPin, const char* axisName) {
    _stepPin = stepPin;
    _dirPin = dirPin;
    _enablePin = enablePin;
    _homePin = homePin;
    _limitPin = limitPin;
    _axisName = axisName;
    
    // Initialize motor properties
    _currentPosition = 0.0;
    _targetPosition = 0.0;
    _isMoving = false;
    _isEnabled = false;
    _lastStepTime = 0;
    _stepInterval = 1000; // Default 1ms interval
}

void StepperMotor::initialize() {
    // Set pin modes
    pinMode(_stepPin, OUTPUT);
    pinMode(_dirPin, OUTPUT);
    pinMode(_enablePin, OUTPUT);
    pinMode(_homePin, INPUT_PULLUP);
    pinMode(_limitPin, INPUT_PULLUP);
    
    // Initialize pin states
    digitalWrite(_stepPin, LOW);
    digitalWrite(_dirPin, LOW);
    digitalWrite(_enablePin, HIGH); // Disable motor initially
    
    // Calculate steps per mm
    float stepsPerRev = STEPPER_STEPS_PER_REV * MICROSTEPPING;
    float stepsPerMm = stepsPerRev / STEPS_PER_MM;
}

void StepperMotor::enable() {
    digitalWrite(_enablePin, LOW); // Enable motor (active low)
    _isEnabled = true;
}

void StepperMotor::disable() {
    digitalWrite(_enablePin, HIGH); // Disable motor (active low)
    _isEnabled = false;
    _isMoving = false;
}

void StepperMotor::moveTo(float position) {
    // Constrain position to limits
    position = constrain(position, MIN_TRAVEL_MM, MAX_TRAVEL_MM);
    
    _targetPosition = position;
    _isMoving = true;
    
    // Set direction
    setDirection(_targetPosition > _currentPosition);
    
    // Calculate step interval for movement
    calculateStepInterval(MAX_SPEED_MM_PER_SEC);
}

void StepperMotor::moveRelative(float distance) {
    moveTo(_currentPosition + distance);
}

void StepperMotor::home() {
    // Move towards home switch at homing speed
    _targetPosition = -1000; // Move towards home
    _isMoving = true;
    
    // Set direction towards home
    setDirection(false); // Assuming home is at lower position
    
    // Calculate step interval for homing speed
    calculateStepInterval(HOME_SPEED_MM_PER_SEC);
}

void StepperMotor::stop() {
    _isMoving = false;
    digitalWrite(_stepPin, LOW);
}

bool StepperMotor::isMoving() {
    return _isMoving;
}

float StepperMotor::getCurrentPosition() {
    return _currentPosition;
}

void StepperMotor::setCurrentPositionAsZero() {
    _currentPosition = 0.0;
    _targetPosition = 0.0;
}

bool StepperMotor::isHomeSwitchTriggered() {
    return digitalRead(_homePin) == LOW; // Active LOW
}

bool StepperMotor::isLimitSwitchTriggered() {
    return digitalRead(_limitPin) == LOW; // Active LOW
}

void StepperMotor::update() {
    if (!_isEnabled || !_isMoving) {
        return;
    }
    
    // Check if we've reached the target
    if (abs(_currentPosition - _targetPosition) < 0.01) {
        _isMoving = false;
        digitalWrite(_stepPin, LOW);
        return;
    }
    
    // Check if home switch is triggered during homing
    if (_targetPosition < _currentPosition && isHomeSwitchTriggered()) {
        _isMoving = false;
        _currentPosition = 0.0; // Set home position
        digitalWrite(_stepPin, LOW);
        return;
    }
    
    // Check if limit switch is triggered
    if (isLimitSwitchTriggered()) {
        _isMoving = false;
        digitalWrite(_stepPin, LOW);
        return;
    }
    
    // Execute step if timing is right
    unsigned long currentTime = micros();
    if (currentTime - _lastStepTime >= _stepInterval) {
        step();
        _lastStepTime = currentTime;
    }
}

void StepperMotor::calculateStepInterval(float speed) {
    // Convert speed from mm/sec to steps/sec
    float stepsPerRev = STEPPER_STEPS_PER_REV * MICROSTEPPING;
    float stepsPerMm = stepsPerRev / STEPS_PER_MM;
    float stepsPerSec = speed * stepsPerMm;
    
    // Calculate interval in microseconds
    _stepInterval = 1000000 / stepsPerSec;
}

void StepperMotor::step() {
    // Toggle step pin
    digitalWrite(_stepPin, HIGH);
    delayMicroseconds(1); // Minimum pulse width
    digitalWrite(_stepPin, LOW);
    
    // Update position
    float stepDistance = 1.0 / (STEPPER_STEPS_PER_REV * MICROSTEPPING / STEPS_PER_MM);
    if (digitalRead(_dirPin) == HIGH) {
        _currentPosition += stepDistance;
    } else {
        _currentPosition -= stepDistance;
    }
}

void StepperMotor::setDirection(bool forward) {
    digitalWrite(_dirPin, forward ? HIGH : LOW);
} 