#include <Arduino.h>
#include "StateMachine/STATES/00_IDLE.h"
#include "../../config/Pin_Definitions.h"
#include "../../../include/Web_Manager.h"

// OTA Manager function
extern void updateOTA();

// State machine function
extern void setMachineState(int state);
#define STATE_GANTRY 2

//* ************************************************************************
//* ************************ IDLE STATE ***********************************
//* ************************************************************************

void idleState() {
    static unsigned long lastButtonCheck = 0;
    static bool lastButtonState = LOW;
    
    // Check start button (debounced)
    unsigned long currentTime = millis();
    if (currentTime - lastButtonCheck >= 50) {  // Check every 50ms
        bool buttonState = digitalRead(TEST_BUTTON_PIN);
        
        // Button is active HIGH (pulldown), so check for rising edge
        if (lastButtonState == LOW && buttonState == HIGH) {
            // Button pressed - start cycle
            setMachineState(STATE_GANTRY);
        }
        
        lastButtonState = buttonState;
        lastButtonCheck = currentTime;
    }
    
    // Allow OTA updates during idle state
    updateOTA();
}

