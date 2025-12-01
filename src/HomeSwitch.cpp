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
    // Debounce: if pin reads HIGH, wait 5ms and verify it's still HIGH
    if (digitalRead(pin) == HIGH) {
        delay(5);
        return digitalRead(pin) == HIGH;
    }
    return false;
}

bool HomeSwitch::readDual() {
    if (!hasDualPins) return read();
    // Debounce: if both pins read HIGH, wait 5ms and verify they're still HIGH
    if (digitalRead(pin) == HIGH && digitalRead(pin2) == HIGH) {
        delay(5);
        return digitalRead(pin) == HIGH && digitalRead(pin2) == HIGH;
    }
    return false;
}

bool HomeSwitch::isTriggered() {
    return read();  // Active high, so true when HIGH
}

bool HomeSwitch::isDualTriggered() {
    return readDual();  // Both must be HIGH
}