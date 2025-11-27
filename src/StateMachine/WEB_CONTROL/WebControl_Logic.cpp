#include "StateMachine/WEB_CONTROL/WebControl_Logic.h"
#include "StateMachine/WEB_CONTROL/Web_Manual_Control.h"
#include "StateMachine/WEB_CONTROL/Web_Pick_Place.h"
#include "Web_Manager.h"
#include <Arduino.h>

// Sub-state tracking
enum WebSubState {
    WEB_IDLE,
    WEB_MANUAL,
    WEB_PICK_PLACE
};

static WebSubState currentWebSubState = WEB_IDLE;

//* ************************************************************************
//* ************************ WEB CONTROL LOGIC *****************************
//* ************************************************************************

void initializeWebControlLogic() {
    // Determine what kind of web request initiated this state
    if (isWebMoveRequested()) {
        currentWebSubState = WEB_MANUAL;
        initializeManualControl();
    } else if (isWebStartRequested()) {
        currentWebSubState = WEB_PICK_PLACE;
        initializePickPlace();
        clearWebStartRequest(); // Clear the flag
    } else {
        currentWebSubState = WEB_IDLE;
        Serial.println("Web Control Logic Init: No specific request found");
    }
}

int executeWebControlLogic() {
    switch (currentWebSubState) {
        case WEB_MANUAL:
            if (executeManualControl() == 0) {
                // Manual control finished, return to main IDLE
                return 0;
            }
            break;
            
        case WEB_PICK_PLACE:
             if (executePickPlace() == 0) {
                 // Pick Place finished, return to main IDLE
                 return 0;
             }
            break;
            
        case WEB_IDLE:
        default:
            // If nothing to do, exit back to main IDLE
            return 0;
    }
    
    return 4; // Stay in WEB_CONTROL state
}

