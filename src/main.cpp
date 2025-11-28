#include <Arduino.h>
#include "StateMachine/STATES/01_HOMING.h"

// Global state machine instance
HomingState* homingState = nullptr;

// Setup function - called once at startup
void setup() {
    // Initialize serial for debugging
    Serial.begin(115200);
    delay(1000);

    Serial.println("Paint Machine Homing Sequence Starting...");

    // Create and initialize homing state
    homingState = new HomingState();
    homingState->enter();

    Serial.println("Homing state initialized");
}

// Loop function - called repeatedly
void loop() {
    if (homingState) {
        // Update the homing state
        homingState->update();

        // Check if homing is complete
        if (homingState->isComplete()) {
            Serial.println("Homing sequence completed successfully!");
            homingState->exit();
            delete homingState;
            homingState = nullptr;

            // Could transition to next state here
            Serial.println("Ready for next operation");

        } else if (homingState->hasError()) {
            Serial.println("Homing sequence failed with error!");
            homingState->exit();
            delete homingState;
            homingState = nullptr;

            // Could handle error state here
            Serial.println("Error state - check switches and motors");
        }
    }

    // Small delay to prevent overwhelming the processor
    delay(10);
}
