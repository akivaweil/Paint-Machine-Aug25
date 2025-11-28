//* ************************************************************************
//* ************************ HOMING STATE **********************************
//* ************************************************************************
// This state handles sequential homing: Fork first, then X and Y simultaneously
// Logic: Home fork -> Move fork away -> Home X and Y -> Move X and Y away -> Zero all

#include <Arduino.h>
#include "config/Config.h"
#include "config/Pin_Definitions.h"
#include "config/Homing_Config.h"
#include "StateMachine/FUNCTIONS/StepperMotor.h"

// Configuration
#define FORK_HOME_OFFSET 0.5 // Offset in inches for Fork motor after homing

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
        case 0: // Start Fork Homing
            //! ************************************************************************
            //! STEP 2: START FORK HOMING (FIRST)
            //! ************************************************************************
            Serial.println("Starting Homing Sequence...");
            Serial.println("Step 1: Homing Fork Motor...");
            resetHomingState(); // Ensure clean flags
            
            forkMotor->home();
            
            homingStateStep = 1;
            break;

        case 1: // Wait for fork to reach home switch
            //! ************************************************************************
            //! STEP 3: WAIT FOR FORK TO HOME
            //! ************************************************************************
            forkMotor->updateHoming();

            // Check if Fork is homed (stopped and switch triggered)
            if (!forkHomed && !forkMotor->isMoving() && forkMotor->isHomeSwitchTriggered()) {
                forkHomed = true;
                Serial.println("Fork Motor Homed");
                delay(500); // Short pause for stability
                homingStateStep = 2;
            }
            break;

        case 2: // Move Fork Away
            //! ************************************************************************
            //! STEP 4: MOVE FORK AWAY FROM SWITCH
            //! ************************************************************************
            forkMotor->moveAwayFromHome(FORK_HOME_OFFSET);
            homingStateStep = 3;
            break;

        case 3: // Wait for fork move away completion
            //! ************************************************************************
            //! STEP 5: WAIT FOR FORK MOVE AWAY COMPLETION
            //! ************************************************************************
            forkMotor->update();

            if (!forkMotor->isMoving()) {
                Serial.println("Fork Motor moved to home offset position");
                delay(500); // Short pause for stability
                forkMotor->setCurrentPosition(FORK_HOME_OFFSET);
                Serial.println("Step 2: Homing X and Y Motors...");
                homingStateStep = 4;
            }
            break;

        case 4: // Start X and Y Homing
            //! ************************************************************************
            //! STEP 6: START X AND Y HOMING (SIMULTANEOUSLY)
            //! ************************************************************************
            xMotor->home();
            yMotor->home();
            
            homingStateStep = 5;
            break;

        case 5: // Wait for X and Y to reach home switch
            //! ************************************************************************
            //! STEP 7: WAIT FOR X AND Y TO HOME
            //! ************************************************************************
            xMotor->updateHoming();
            yMotor->updateHoming();

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

            // Wait until BOTH X and Y are homed
            if (xHomed && yHomed) {
                Serial.println("X and Y Motors Homed. Moving away...");
                delay(500); // Short pause for stability
                homingStateStep = 6;
            }
            break;

        case 6: // Move X and Y Away
            //! ************************************************************************
            //! STEP 8: MOVE X AND Y AWAY FROM SWITCH
            //! ************************************************************************
            xMotor->moveAwayFromHome();
            yMotor->moveAwayFromHome();
            
            homingStateStep = 7;
            break;

        case 7: // Wait for X and Y move away completion
            //! ************************************************************************
            //! STEP 9: WAIT FOR MOVE COMPLETION AND ZERO
            //! ************************************************************************
            xMotor->update();
            yMotor->update();

            if (!xMotor->isMoving() && !yMotor->isMoving()) {
                Serial.println("Homing Complete. Zeroing coordinates.");
                
                // Zero X and Y positions (fork already zeroed)
                xMotor->setCurrentPosition(MOVE_AWAY_FROM_HOME_DISTANCE);
                yMotor->setCurrentPosition(MOVE_AWAY_FROM_HOME_DISTANCE);

                resetHomingState(); // Reset for next time
                return 0; // Transition to IDLE
            }
            break;
    }

    return 1; // Stay in HOMING state
}
