//* ************************************************************************
//* ************************ HOMING STATE **********************************
//* ************************************************************************
// This state handles simultaneous homing of all motors
// Logic: Move all to home -> Wait for all to home -> Move all away offset

#include <Arduino.h>
#include "config/Config.h"
#include "config/Pin_Definitions.h"
#include "config/Homing_Config.h"
#include "StateMachine/FUNCTIONS/StepperMotor.h"

// External motor objects
extern StepperMotor* xMotor;
extern StepperMotor* yMotor;
extern StepperMotor* forkMotor;

// Forward declaration for OTA manager
void updateOTA();

// State variables
static int homingStateStep = 0;
static bool xHomed = false;
static bool yHomed = false;
static bool forkHomed = false;

// Function to reset homing state
void resetHomingState() {
    homingStateStep = 0;
    xHomed = false;
    yHomed = false;
    forkHomed = false;
}

// Function to run homing state
int runHomingState() {
    // Ensure OTA updates are handled
    updateOTA();

    //╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
    //║ ⚔️ HOMING LOGIC                                                      ║
    //╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

    //! ************************************************************************
    //! STEP 1: CHECK FOR SERIAL COMMANDS
    //! ************************************************************************
    if (Serial.available()) {
        String command = Serial.readString();
        command.trim();
        command.toLowerCase();
        if (command == "m") return 2; // Transition to TEST_POSITION state
    }

    switch (homingStateStep) {
        case 0: // Start Homing
            //! ************************************************************************
            //! STEP 2: START ALL MOTORS HOMING
            //! ************************************************************************
            Serial.println("Starting Homing Sequence...");
            resetHomingState(); // Ensure clean flags
            
            xMotor->home();
            yMotor->home();
            forkMotor->home();
            
            homingStateStep = 1;
            break;

        case 1: // Wait for all to reach home switch
            //! ************************************************************************
            //! STEP 3: UPDATE HOMING AND WAIT FOR ALL SWITCHES
            //! ************************************************************************
            // Update motors to handle switch detection and stopping
            xMotor->updateHoming();
            yMotor->updateHoming();
            forkMotor->updateHoming();

            // Check if X is homed (stopped and switch triggered)
            if (!xHomed && !xMotor->isMoving() && xMotor->isHomeSwitchTriggered()) {
                xHomed = true;
                Serial.println("X Motor Homed");
            }

            // Check if Y is homed
            if (!yHomed && !yMotor->isMoving() && yMotor->isHomeSwitchTriggered()) {
                yHomed = true;
                Serial.println("Y Motor Homed");
            }

            // Check if Fork is homed
            if (!forkHomed && !forkMotor->isMoving() && forkMotor->isHomeSwitchTriggered()) {
                forkHomed = true;
                Serial.println("Fork Motor Homed");
            }

            // Wait until ALL are homed
            if (xHomed && yHomed && forkHomed) {
                Serial.println("All Motors Homed. Moving away...");
                delay(500); // Short pause for stability
                homingStateStep = 2;
            }
            break;

        case 2: // Move Away
            //! ************************************************************************
            //! STEP 4: MOVE ALL MOTORS AWAY FROM SWITCH
            //! ************************************************************************
            // Move all motors away by configured offset
            xMotor->moveAwayFromHome();
            yMotor->moveAwayFromHome();
            forkMotor->moveAwayFromHome();
            
            homingStateStep = 3;
            break;

        case 3: // Wait for move away completion
            //! ************************************************************************
            //! STEP 5: WAIT FOR MOVE COMPLETION AND ZERO
            //! ************************************************************************
            // Use update() which is safe during moveAway (checks _movingAwayFromHome flag)
            xMotor->update();
            yMotor->update();
            forkMotor->update();

            if (!xMotor->isMoving() && !yMotor->isMoving() && !forkMotor->isMoving()) {
                Serial.println("Homing Complete. Zeroing coordinates.");
                
                // Zero all positions
                xMotor->setCurrentPositionAsZero();
                yMotor->setCurrentPositionAsZero();
                forkMotor->setCurrentPositionAsZero();

                resetHomingState(); // Reset for next time
                return 0; // Transition to IDLE
            }
            break;
    }

    return 1; // Stay in HOMING state
}
