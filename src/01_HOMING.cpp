#include "StateMachine/STATES/01_HOMING.h"
#include "StateMachine/FUNCTIONS/StepperMotor.h"

//* ************************************************************************
//* ************************ HOMING STATE IMPLEMENTATION *******************
//* ************************************************************************

// State variables
static bool _initialized = false;
static bool _x1Homed = false;
static bool _x2Homed = false;
static bool _yHomed = false;
static bool _forkHomed = false;
static unsigned long _homingStartTime = 0;
static const unsigned long HOMING_TIMEOUT = 60000; // 60 seconds timeout

// Motor objects (will be initialized in main.cpp)
extern StepperMotor* x1Motor;
extern StepperMotor* x2Motor;
extern StepperMotor* yMotor;
extern StepperMotor* forkMotor;

void HomingState::initialize() {
    if (_initialized) {
        return;
    }
    
    // Reset homing flags
    _x1Homed = false;
    _x2Homed = false;
    _yHomed = false;
    _forkHomed = false;
    
    // Enable all motors
    if (x1Motor) x1Motor->enable();
    if (x2Motor) x2Motor->enable();
    if (yMotor) yMotor->enable();
    if (forkMotor) forkMotor->enable();
    
    // Start homing sequence
    homeX1Axis();
    homeX2Axis();
    homeYAxis();
    homeForkAxis();
    
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
    if (x1Motor) x1Motor->update();
    if (x2Motor) x2Motor->update();
    if (yMotor) yMotor->update();
    if (forkMotor) forkMotor->update();
    
    // Check if motors have reached home switches
    if (x1Motor && !_x1Homed && x1Motor->isHomeSwitchTriggered()) {
        x1Motor->stop();
        x1Motor->setCurrentPositionAsZero();
        _x1Homed = true;
    }
    
    if (x2Motor && !_x2Homed && x2Motor->isHomeSwitchTriggered()) {
        x2Motor->stop();
        x2Motor->setCurrentPositionAsZero();
        _x2Homed = true;
    }
    
    if (yMotor && !_yHomed && yMotor->isHomeSwitchTriggered()) {
        yMotor->stop();
        yMotor->setCurrentPositionAsZero();
        _yHomed = true;
    }
    
    if (forkMotor && !_forkHomed && forkMotor->isHomeSwitchTriggered()) {
        forkMotor->stop();
        forkMotor->setCurrentPositionAsZero();
        _forkHomed = true;
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
        if (x1Motor) x1Motor->disable();
        if (x2Motor) x2Motor->disable();
        if (yMotor) yMotor->disable();
        if (forkMotor) forkMotor->disable();
    }
    
    // Turn off error LED
    digitalWrite(ERROR_LED_PIN, LOW);
    
    _initialized = false;
}

void HomingState::homeX1Axis() {
    if (x1Motor) {
        x1Motor->home();
    }
}

void HomingState::homeX2Axis() {
    if (x2Motor) {
        x2Motor->home();
    }
}

void HomingState::homeYAxis() {
    if (yMotor) {
        yMotor->home();
    }
}

void HomingState::homeForkAxis() {
    if (forkMotor) {
        forkMotor->home();
    }
}

bool HomingState::allAxesHomed() {
    return _x1Homed && _x2Homed && _yHomed && _forkHomed;
}

void HomingState::updateProgress() {
    // Blink status LED faster as more axes are homed
    unsigned long currentTime = millis();
    int homedCount = (_x1Homed ? 1 : 0) + (_x2Homed ? 1 : 0) + (_yHomed ? 1 : 0) + (_forkHomed ? 1 : 0);
    
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
        if (currentTime % 600 < 300) {
            digitalWrite(STATUS_LED_PIN, HIGH);
        } else {
            digitalWrite(STATUS_LED_PIN, LOW);
        }
    } else if (homedCount == 3) {
        // Very fast blink - three axes homed
        if (currentTime % 300 < 150) {
            digitalWrite(STATUS_LED_PIN, HIGH);
        } else {
            digitalWrite(STATUS_LED_PIN, LOW);
        }
    } else {
        // Solid on - all axes homed
        digitalWrite(STATUS_LED_PIN, HIGH);
    }
} 