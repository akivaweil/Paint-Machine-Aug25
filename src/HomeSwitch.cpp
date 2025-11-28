#include "../include/StateMachine/FUNCTIONS/HomeSwitch.h"

HomeSwitch::HomeSwitch(uint8_t switchPin) {
    this->pin = switchPin;
}

void HomeSwitch::begin() {
    pinMode(pin, INPUT_PULLDOWN);  // Active high switches with pulldown
}

bool HomeSwitch::read() {
    return digitalRead(pin);
}

bool HomeSwitch::isTriggered() {
    return read();  // Active high, so true when HIGH
}