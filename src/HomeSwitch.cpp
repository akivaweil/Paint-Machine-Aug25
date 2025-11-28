#include "../include/StateMachine/FUNCTIONS/HomeSwitch.h"
#include <Arduino.h>

// Constructor
HomeSwitch::HomeSwitch() : bounceX1(nullptr), bounceX2(nullptr), bounceY(nullptr), bounceFork(nullptr), hasX2Switch(true) {
}

// Initialize switches
void HomeSwitch::init() {
    // Explicitly set pin modes for active high switches with pulldown
    pinMode(X_HOME_PIN, INPUT_PULLDOWN);
    pinMode(X_HOME_PIN2, INPUT_PULLDOWN);
    pinMode(Y_HOME_PIN, INPUT_PULLDOWN);
    pinMode(FORK_HOME_PIN, INPUT_PULLDOWN);

    // Initialize X switches
    bounceX1 = new Bounce();
    bounceX1->attach(X_HOME_PIN, INPUT_PULLDOWN);
    bounceX1->interval(HOME_SWITCH_DEBOUNCE_MS);

    bounceX2 = new Bounce();
    bounceX2->attach(X_HOME_PIN2, INPUT_PULLDOWN);
    bounceX2->interval(HOME_SWITCH_DEBOUNCE_MS);

    // Initialize Y switch
    bounceY = new Bounce();
    bounceY->attach(Y_HOME_PIN, INPUT_PULLDOWN);
    bounceY->interval(HOME_SWITCH_DEBOUNCE_MS);

    // Initialize Fork switch
    bounceFork = new Bounce();
    bounceFork->attach(FORK_HOME_PIN, INPUT_PULLDOWN);
    bounceFork->interval(HOME_SWITCH_DEBOUNCE_MS);
}

// Update switch states (call this in main loop)
void HomeSwitch::update() {
    if (bounceX1) bounceX1->update();
    if (bounceX2) bounceX2->update();
    if (bounceY) bounceY->update();
    if (bounceFork) bounceFork->update();
}

// Check if switches are triggered
bool HomeSwitch::isXHome() {
    // X is home if either switch is triggered (for redundancy - either one is sufficient)
    // For debugging, use raw digital read instead of debounced
    return getX1Raw() || getX2Raw();
}

bool HomeSwitch::isYHome() {
    // For debugging, use raw digital read instead of debounced
    return getYRaw();
}

bool HomeSwitch::isForkHome() {
    // For debugging, use raw digital read instead of debounced
    return getForkRaw();
}

// Check individual X switches
bool HomeSwitch::isX1Home() {
    return getX1Raw();
}

bool HomeSwitch::isX2Home() {
    return getX2Raw();
}

// Check if all required switches are home
bool HomeSwitch::areAllHome() {
    return isXHome() && isYHome() && isForkHome();
}

// Get raw pin readings (for debugging)
bool HomeSwitch::getX1Raw() {
    return digitalRead(X_HOME_PIN);
}

bool HomeSwitch::getX2Raw() {
    return digitalRead(X_HOME_PIN2);
}

bool HomeSwitch::getYRaw() {
    return digitalRead(Y_HOME_PIN);
}

bool HomeSwitch::getForkRaw() {
    return digitalRead(FORK_HOME_PIN);
}

// Debug function to print all switch states
void HomeSwitch::printDebugInfo() {
    Serial.print("Raw pins - X1:");
    Serial.print(getX1Raw());
    Serial.print(" X2:");
    Serial.print(getX2Raw());
    Serial.print(" Y:");
    Serial.print(getYRaw());
    Serial.print(" Fork:");
    Serial.print(getForkRaw());
    Serial.print(" | Debounced - X1:");
    Serial.print(isX1Home());
    Serial.print(" X2:");
    Serial.print(isX2Home());
    Serial.print(" Y:");
    Serial.print(isYHome());
    Serial.print(" Fork:");
    Serial.println(isForkHome());
}
