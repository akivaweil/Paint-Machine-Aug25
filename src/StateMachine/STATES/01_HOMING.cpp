#include "01_HOMING.h"
#include "../FUNCTIONS/StepperMotor.h"

//* ************************************************************************
//* ************************ HOMING STATE IMPLEMENTATION *******************
//* ************************************************************************

// State variables
static bool _initialized = false;
static bool _xHomed = false;
static bool _yHomed = false;
static bool _zHomed = false;
static unsigned long _homingStartTime = 0;
static const unsigned long HOMING_TIMEOUT = 60000; // 60 seconds timeout

// Motor objects (will be initialized in main.cpp)
extern StepperMotor* xMotor;
extern StepperMotor* yMotor;
extern StepperMotor* zMotor;

void HomingState::initialize() {
    if (_initialized) {
        return;
    }
    
    // Reset homing flags
    _xHomed = false;
    _yHomed = false;
    _zHomed = false;
    
    // Enable all motors
    if (xMotor) xMotor->enable();
    if (yMotor) yMotor->enable();
    if (zMotor) zMotor->enable();
    
    // Start homing sequence
    homeXAxis();
    homeYAxis();
    homeZAxis();
    
    // Record start time
    _homingStartTime = millis();
    
    // Set status LED to indicate homing
    digitalWrite(STATUS_LED_PIN, HIGH);
    digitalWrite(ERROR_LED_PIN, LOW);
    
    _initialized = true;
}

void HomingState::run() {
    // Check for timeout
    if (millis() - _homingStartTime > HOMING_TIMEOUT) {
        // Homing timeout - indicate error
        digitalWrite(ERROR_LED_PIN, HIGH);
        return;
    }
    
    // Update motor states
    if (xMotor) xMotor->update();
    if (yMotor) yMotor->update();
    if (zMotor) zMotor->update();
    
    // Check if motors have reached home switches
    if (xMotor && !_xHomed && xMotor->isHomeSwitchTriggered()) {
        xMotor->stop();
        xMotor->setCurrentPositionAsZero();
        _xHomed = true;
    }
    
    if (yMotor && !_yHomed && yMotor->isHomeSwitchTriggered()) {
        yMotor->stop();
        yMotor->setCurrentPositionAsZero();
        _yHomed = true;
    }
    
    if (zMotor && !_zHomed && zMotor->isHomeSwitchTriggered()) {
        zMotor->stop();
        zMotor->setCurrentPositionAsZero();
        _zHomed = true;
    }
    
    // Update progress indicators
    updateProgress();
}

bool HomingState::shouldTransition() {
    // Check if all axes are homed
    if (allAxesHomed()) {
        return true;
    }
    
    // Check for emergency stop
    if (digitalRead(E_STOP_PIN) == HIGH) {
        return true;
    }
    
    // Check for stop button
    if (digitalRead(STOP_BUTTON_PIN) == HIGH) {
        return true;
    }
    
    return false;
}

int HomingState::getNextState() {
    // Check emergency stop first
    if (digitalRead(E_STOP_PIN) == HIGH) {
        return 99; // Emergency stop state
    }
    
    // Check stop button
    if (digitalRead(STOP_BUTTON_PIN) == HIGH) {
        return 0; // Return to idle
    }
    
    // If all axes homed, go to next state (to be defined)
    if (allAxesHomed()) {
        return 2; // Next state after homing
    }
    
    return 1; // Stay in homing
}

void HomingState::cleanup() {
    // Disable motors if not all homed
    if (!allAxesHomed()) {
        if (xMotor) xMotor->disable();
        if (yMotor) yMotor->disable();
        if (zMotor) zMotor->disable();
    }
    
    // Turn off error LED
    digitalWrite(ERROR_LED_PIN, LOW);
    
    _initialized = false;
}

void HomingState::homeXAxis() {
    if (xMotor) {
        xMotor->home();
    }
}

void HomingState::homeYAxis() {
    if (yMotor) {
        yMotor->home();
    }
}

void HomingState::homeZAxis() {
    if (zMotor) {
        zMotor->home();
    }
}

bool HomingState::allAxesHomed() {
    return _xHomed && _yHomed && _zHomed;
}

void HomingState::updateProgress() {
    // Blink status LED faster as more axes are homed
    unsigned long currentTime = millis();
    int homedCount = (_xHomed ? 1 : 0) + (_yHomed ? 1 : 0) + (_zHomed ? 1 : 0);
    
    if (homedCount == 0) {
        // Slow blink - no axes homed
        if (currentTime % 1000 < 500) {
            digitalWrite(STATUS_LED_PIN, HIGH);
        } else {
            digitalWrite(STATUS_LED_PIN, LOW);
        }
    } else if (homedCount == 1) {
        // Medium blink - one axis homed
        if (currentTime % 800 < 400) {
            digitalWrite(STATUS_LED_PIN, HIGH);
        } else {
            digitalWrite(STATUS_LED_PIN, LOW);
        }
    } else if (homedCount == 2) {
        // Fast blink - two axes homed
        if (currentTime % 400 < 200) {
            digitalWrite(STATUS_LED_PIN, HIGH);
        } else {
            digitalWrite(STATUS_LED_PIN, LOW);
        }
    } else {
        // Solid on - all axes homed
        digitalWrite(STATUS_LED_PIN, HIGH);
    }
} 