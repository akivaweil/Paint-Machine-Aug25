#include "StateMachine/STATES/00_IDLE.h"

//* ************************************************************************
//* ************************ IDLE STATE IMPLEMENTATION *********************
//* ************************************************************************

// State variables
static bool _initialized = false;
static unsigned long _lastStatusUpdate = 0;
static const unsigned long STATUS_UPDATE_INTERVAL = 1000; // 1 second

void IdleState::initialize() {
    if (_initialized) {
        return;
    }
    
    // Initialize status LED
    pinMode(STATUS_LED_PIN, OUTPUT);
    digitalWrite(STATUS_LED_PIN, HIGH); // Turn on status LED
    
    // Turn off error LED
    pinMode(ERROR_LED_PIN, OUTPUT);
    digitalWrite(ERROR_LED_PIN, LOW);
    
    _initialized = true;
}

void IdleState::run() {
    // Check for events
    checkForEvents();
    
    // Update status indicators
    updateStatus();
}

bool IdleState::shouldTransition() {
    // Check for start button press
    if (digitalRead(START_BUTTON_PIN) == HIGH) {
        return true;
    }
    
    // Check for emergency stop
    if (digitalRead(E_STOP_PIN) == HIGH) {
        return true;
    }
    
    return false;
}

int IdleState::getNextState() {
    // Check emergency stop first
    if (digitalRead(E_STOP_PIN) == HIGH) {
        return 99; // Emergency stop state (to be defined)
    }
    
    // Check start button
    if (digitalRead(START_BUTTON_PIN) == HIGH) {
        return 1; // Homing state
    }
    
    return 0; // Stay in idle
}

void IdleState::cleanup() {
    // Turn off status LED
    digitalWrite(STATUS_LED_PIN, LOW);
    
    _initialized = false;
}

void IdleState::checkForEvents() {
    // Check for stop button press
    if (digitalRead(STOP_BUTTON_PIN) == HIGH) {
        // Already in idle, just acknowledge
        digitalWrite(STATUS_LED_PIN, LOW);
        delay(100);
        digitalWrite(STATUS_LED_PIN, HIGH);
    }
    
    // Check for reset button
    if (digitalRead(RESET_PIN) == HIGH) {
        // Reset acknowledgment
        digitalWrite(STATUS_LED_PIN, LOW);
        delay(200);
        digitalWrite(STATUS_LED_PIN, HIGH);
    }
}

void IdleState::updateStatus() {
    unsigned long currentTime = millis();
    
    if (currentTime - _lastStatusUpdate >= STATUS_UPDATE_INTERVAL) {
        // Blink status LED to show idle state
        digitalWrite(STATUS_LED_PIN, !digitalRead(STATUS_LED_PIN));
        _lastStatusUpdate = currentTime;
    }
} 