#include <Arduino.h>
#include "StateMachine/STATES/00_IDLE.h"
#include "../../config/Pin_Definitions.h"
#include "../../../include/Web_Manager.h"
#include "StateMachine/FUNCTIONS/StepperMotor.h"

// OTA Manager function
extern void updateOTA();

// State machine function
extern void setMachineState(int state);
#define STATE_PICK_PLACE 2

// External motor instances (defined in Web_Manager.cpp)
extern StepperMotor* motorX;

// Test position values (set from web interface)
extern float testPos1X;

//* ************************************************************************
//* ************************ IDLE STATE ***********************************
//* ************************************************************************

void idleState() {
    static unsigned long lastButtonCheck = 0;
    static bool lastButtonState = LOW;
    static bool isMovingToPosition = false;
    
    // Wait for any existing movement to complete before checking position
    // This ensures we don't interrupt movements from other states
    if (motorX && motorX->isMotorRunning() && !isMovingToPosition) {
        updateOTA();
        delay(1);
        return;  // Don't do anything else until motor stops
    }
    
    // Move X-axis directly to pos1/pos4 X position if not already there
    // This calculates the movement directly from current position to target,
    // avoiding any intermediate offset positions
    if (motorX && !isMovingToPosition) {
        // Get current X position (wherever we are - could be at waiting position, pos3, etc.)
        float currentX = motorX->stepsToInches(motorX->getCurrentPosition());
        
        // Calculate target absolute position directly to pos1 X
        // (negate because positive direction moves toward home switches)
        float targetX = -testPos1X;
        
        // Calculate relative movement needed to reach target directly
        float moveX = targetX - currentX;
        
        // Only move if we're not already at the target position
        if (abs(moveX) > 0.01) {  // 0.01 inch tolerance
            isMovingToPosition = true;
            // Move directly to target position in one smooth motion (no intermediate stops)
            motorX->moveInches(moveX);
        }
    }
    
    // Wait for motor to finish moving if it's running
    if (isMovingToPosition && motorX) {
        if (motorX->isMotorRunning()) {
            updateOTA();
            delay(1);
        } else {
            isMovingToPosition = false;
        }
    }
    
    // Check start button (debounced) - only when not moving
    if (!isMovingToPosition) {
        unsigned long currentTime = millis();
        if (currentTime - lastButtonCheck >= 50) {  // Check every 50ms
            bool buttonState = digitalRead(TEST_BUTTON_PIN);
            
            // Button is active HIGH (pulldown), so check for rising edge
            if (lastButtonState == LOW && buttonState == HIGH) {
                // Button pressed - start test cycle
                setMachineState(STATE_PICK_PLACE);
            }
            
            lastButtonState = buttonState;
            lastButtonCheck = currentTime;
        }
    }
    
    // Allow OTA updates during idle state
    updateOTA();
}

