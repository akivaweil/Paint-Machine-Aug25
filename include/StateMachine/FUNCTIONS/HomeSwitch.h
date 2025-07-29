#ifndef HOME_SWITCH_H
#define HOME_SWITCH_H

//* ************************************************************************
//* ************************ HOME SWITCH CONTROL ***************************
//* ************************************************************************

#include <Arduino.h>
#include <Bounce2.h>
#include "config/Config.h"
#include "config/Pin_Definitions.h"

class HomeSwitch {
public:
    // Constructor
    HomeSwitch(int pin, const char* switchName);
    
    // Initialize the switch
    void initialize();
    
    // Update switch state (call in main loop)
    void update();
    
    // Check if switch is triggered
    bool isTriggered();
    
    // Check if switch was just triggered (rising edge)
    bool wasJustTriggered();
    
    // Check if switch was just released (falling edge)
    bool wasJustReleased();
    
    // Get switch name
    const char* getSwitchName();
    
    // Get raw switch state
    bool getRawState();

private:
    // Switch properties
    int _pin;
    const char* _switchName;
    Bounce _bounce;
    
    // Switch state
    bool _isTriggered;
    bool _wasJustTriggered;
    bool _wasJustReleased;
};

#endif // HOME_SWITCH_H 