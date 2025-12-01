#ifndef HOME_SWITCH_H
#define HOME_SWITCH_H

#include <Arduino.h>
#include "../../../src/config/Config.h"

class HomeSwitch {
private:
    uint8_t pin;
    uint8_t pin2;  // Second pin for dual switches (X axis)
    bool hasDualPins;  // Flag to indicate if this switch has two pins

public:
    // Constructor for single pin
    HomeSwitch(uint8_t switchPin);

    // Constructor for dual pins (X switches)
    HomeSwitch(uint8_t switchPin, uint8_t switchPin2);

    // Initialize the switch(es)
    void begin();

    // Read switch state (active high) - single pin
    bool read();

    // Read dual switch state (both must be high)
    bool readDual();

    // Check if switch is triggered (active high) - single pin
    bool isTriggered();

    // Check if dual switches are triggered (both must be high)
    bool isDualTriggered();
};

#endif // HOME_SWITCH_H