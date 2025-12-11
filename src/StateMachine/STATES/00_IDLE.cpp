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
    
    // Move X-axis to position calculated by adding offset distance and pos1 X position
    // After homing: motor position 0 = physical -0.5 inches from home switch
    // Target physical position: -0.5 + testPos1X
    // Relative move needed: testPos1X inches from current position
    if (motorX && !isMovingToPosition) {
        // Get current X position (wherever we are - could be at waiting position, pos3, etc.)
        float currentX = motorX->stepsToInches(motorX->getCurrentPosition());
        
        // Calculate target position: offset distance (-0.5) + pos1 X position
        // After homing, position 0 corresponds to -0.5 inches physically
        // To reach physical position (-0.5 + testPos1X), move testPos1X inches from current
        float targetX = testPos1X;
        
        // Calculate relative movement needed to reach target
        float moveX = targetX - currentX;
        
        // Only move if we're not already at the target position
        if (abs(moveX) > 0.01) {  // 0.01 inch tolerance
            isMovingToPosition = true;
            // Move to target position
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
            // Set X position to pos1 X value after movement completes
            motorX->setCurrentPosition(motorX->inchesToSteps(testPos1X));
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

