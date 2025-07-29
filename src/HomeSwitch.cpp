#include "StateMachine/FUNCTIONS/HomeSwitch.h"

//* ************************************************************************
//* ************************ HOME SWITCH IMPLEMENTATION ********************
//* ************************************************************************

HomeSwitch::HomeSwitch(int pin, const char* switchName) {
    _pin = pin;
    _switchName = switchName;
    _isTriggered = false;
    _wasJustTriggered = false;
    _wasJustReleased = false;
}

void HomeSwitch::initialize() {
    // Configure bounce object
    _bounce.attach(_pin, INPUT_PULLUP);
    _bounce.interval(DEBOUNCE_DELAY_MS);
    
    // Initialize state
    _isTriggered = false;
    _wasJustTriggered = false;
    _wasJustReleased = false;
}

void HomeSwitch::update() {
    // Update bounce object
    _bounce.update();
    
    // Get current state (Active LOW)
    bool currentState = _bounce.read() == LOW;
    
    // Update trigger state
    _wasJustTriggered = !_isTriggered && currentState;
    _wasJustReleased = _isTriggered && !currentState;
    _isTriggered = currentState;
}

bool HomeSwitch::isTriggered() {
    return _isTriggered;
}

bool HomeSwitch::wasJustTriggered() {
    return _wasJustTriggered;
}

bool HomeSwitch::wasJustReleased() {
    return _wasJustReleased;
}

const char* HomeSwitch::getSwitchName() {
    return _switchName;
}

bool HomeSwitch::getRawState() {
    return _bounce.read() == LOW; // Active LOW
} 