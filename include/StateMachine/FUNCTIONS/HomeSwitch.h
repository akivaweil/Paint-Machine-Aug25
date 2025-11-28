#ifndef HOME_SWITCH_H
#define HOME_SWITCH_H

#include <Arduino.h>
#include "../../../src/config/Config.h"
#include "../../../src/config/Homing_Config.h"

class HomeSwitch {
private:
    uint8_t pin;

public:
    // Constructor
    HomeSwitch(uint8_t switchPin);

    // Initialize the switch
    void begin();

    // Read switch state (active high)
    bool read();

    // Check if switch is triggered (active high)
    bool isTriggered();
};

#endif // HOME_SWITCH_H