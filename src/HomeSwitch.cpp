#include "../include/StateMachine/FUNCTIONS/HomeSwitch.h"

HomeSwitch::HomeSwitch(uint8_t switchPin) {
    this->pin = switchPin;
    this->pin2 = 0;  // Not used for single pin
    this->hasDualPins = false;
}

HomeSwitch::HomeSwitch(uint8_t switchPin, uint8_t switchPin2) {
    this->pin = switchPin;
    this->pin2 = switchPin2;
    this->hasDualPins = true;
}

void HomeSwitch::begin() {
    pinMode(pin, INPUT_PULLDOWN);  // Active high switches with pulldown
    if (hasDualPins) {
        pinMode(pin2, INPUT_PULLDOWN);  // Active high switches with pulldown
    }
}

bool HomeSwitch::read() {
    return digitalRead(pin);
}

bool HomeSwitch::readDual() {
    if (!hasDualPins) return read();
    return digitalRead(pin) && digitalRead(pin2);  // Both must be HIGH
}

bool HomeSwitch::isTriggered() {
    return read();  // Active high, so true when HIGH
}

bool HomeSwitch::isDualTriggered() {
    return readDual();  // Both must be HIGH
}