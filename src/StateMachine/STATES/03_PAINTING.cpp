#include <Arduino.h>
#include "StateMachine/STATES/03_PAINTING.h"
#include "../../config/Config.h"
#include "StateMachine/FUNCTIONS/StepperMotor.h"
#include "Web_Manager.h"
#include "ServoControl.h"
#include "../../config/Pin_Definitions.h"

// External motor instances (defined in Web_Manager.cpp)
extern StepperMotor* motorX;
extern StepperMotor* motorY;
extern StepperMotor* motorFork;
extern StepperMotor* motorPaintRotation;

// External servo and paint gun controls
extern ServoControl* servo;
extern float currentServoAngle;
extern float servoSpeed;
extern void enablePaintRotationMotor();
extern void disablePaintRotationMotor();

// OTA Manager function
extern void updateOTA();

// State machine function
extern void setMachineState(int state);
#define STATE_TEST 2

// Cycle control flags
extern bool cyclePaused;
extern bool cycleCancelled;

// Test position values (set from web interface)
extern float testPos2X;
extern float testPos2Y;
extern float testPos2Fork;

// Paint rotation motor steps per revolution
extern long paintRotationMotorStepsPerRevOutput;

// Macro to check for pause after step completion
#define CHECK_PAUSE_AND_CANCEL() \
    do { \
        while (cyclePaused && !cycleCancelled) { \
            updateOTA(); \
            delay(10); \
        } \
        if (cycleCancelled) return; \
    } while(0)

//* ************************************************************************
//* ************************ PAINTING STATE ********************************
//* ************************************************************************

// Non-blocking servo movement helper with custom speed
void updateServoNonBlocking(float targetAngle, float customSpeed = 0.0) {
    if (!servo) return;
    
    float speedToUse = (customSpeed > 0.0) ? customSpeed : servoSpeed;
    const float stepSize = 0.5;  // Step size in degrees
    const float stepDelayMs = (stepSize / speedToUse) * 1000.0;  // Delay in milliseconds
    static unsigned long lastServoUpdate = 0;
    
    unsigned long now = millis();
    if (now - lastServoUpdate >= (unsigned long)stepDelayMs) {
        float diff = targetAngle - currentServoAngle;
        
        if (abs(diff) > stepSize) {
            float increment = (diff > 0) ? stepSize : -stepSize;
            currentServoAngle += increment;
            servo->write(currentServoAngle);
        } else {
            currentServoAngle = targetAngle;
            servo->write(currentServoAngle);
        }
        
        lastServoUpdate = now;
    }
}

// Block execution state tracking
struct BlockExecutionState {
    bool servoActive;
    float servoTarget;
    float servoSpeed;
    bool servoComplete;
    
    bool motorActive;
    long motorStepsStart;
    long motorStepsTarget;
    bool motorComplete;
    
    void reset() {
        servoActive = false;
        servoTarget = 0.0;
        servoSpeed = 0.0;
        servoComplete = false;
        motorActive = false;
        motorStepsStart = 0;
        motorStepsTarget = 0;
        motorComplete = false;
    }
};

static BlockExecutionState blockStates[MAX_PAINTING_BLOCKS];

void paintingState() {
    static int currentBlockIndex = 0;
    static bool paintingStarted = false;
    static bool usingCustomSequence = false;
    
    // Check for cancel at start of function
    if (cycleCancelled) {
        // Cleanup: stop all motors immediately
        if (motorX) motorX->forceStop();
        if (motorY) motorY->forceStop();
        if (motorFork) motorFork->forceStop();
        if (motorPaintRotation) {
            motorPaintRotation->forceStop();
            disablePaintRotationMotor();
        }
        
        // Turn off paint gun and suction
        digitalWrite(PAINT_GUN_PIN, LOW);
        digitalWrite(SUCTION_PIN, LOW);
        
        // Reset all block states
        for (int i = 0; i < MAX_PAINTING_BLOCKS; i++) {
            blockStates[i].reset();
        }
        
        // Reset flags and state
        cycleCancelled = false;
        cyclePaused = false;
        paintingStarted = false;
        currentBlockIndex = 0;
        usingCustomSequence = false;
        
        // Return to test state (test state will handle further cleanup)
        setMachineState(STATE_TEST);
        return;
    }
    
    // Initialize on first entry
    if (!paintingStarted) {
        // Load sequence if not already loaded
        if (!paintingSequenceLoaded) {
            loadPaintingSequence();
        }
        
        // Check if we have a custom sequence
        usingCustomSequence = (paintingSequenceCount > 0);
        
        // Reset all block states
        for (int i = 0; i < MAX_PAINTING_BLOCKS; i++) {
            blockStates[i].reset();
        }
        
        currentBlockIndex = 0;
        paintingStarted = true;
        cyclePaused = false;  // Reset pause flag on new painting
        cycleCancelled = false;  // Reset cancel flag on new painting
        
        // If no custom sequence, use default behavior (retract fork and return)
        if (!usingCustomSequence) {
            // Default: retract fork motor at position 2
            motorFork->moveInches(testPos2Fork);
            while (motorFork->isMotorRunning()) {
                updateOTA();
                CHECK_PAUSE_AND_CANCEL();
                delay(1);
            }
            
            // Return to test state
            paintingStarted = false;
            setMachineState(STATE_TEST);
            return;
        }
    }
    
    // Execute custom sequence
    if (usingCustomSequence) {
        // Find the current parallel group
        // A group starts at currentBlockIndex and includes all blocks until we hit one that's not parallel
        int groupStart = currentBlockIndex;
        int groupEnd = currentBlockIndex + 1;
        
        // Expand group to include all parallel blocks
        while (groupEnd < paintingSequenceCount && paintingSequence[groupEnd].parallel) {
            groupEnd++;
        }
        
        // Start all blocks in the current group that haven't been started
        for (int i = groupStart; i < groupEnd; i++) {
            if (i >= paintingSequenceCount) break;
            
            PaintingBlock& block = paintingSequence[i];
            BlockExecutionState& state = blockStates[i];
            
            // Initialize block if not started
            if (!state.servoActive && !state.motorActive) {
                if (block.type == 0) {
                    // Servo block
                    state.servoActive = true;
                    state.servoTarget = block.param1;
                    state.servoSpeed = block.param2;
                    state.servoComplete = false;
                } else if (block.type == 1) {
                    // Painting motor block
                    state.motorActive = true;
                    enablePaintRotationMotor();
                    delay(50);
                    if (motorPaintRotation) {
                        state.motorStepsStart = motorPaintRotation->getCurrentPosition();
                        // Convert degrees to steps
                        long steps = (long)((block.param1 / 360.0) * paintRotationMotorStepsPerRevOutput);
                        state.motorStepsTarget = steps;
                        
                        // Set speed if provided
                        if (block.param2 > 0) {
                            long oldSpeed = motorSpeedPaintRotation;
                            motorPaintRotation->setSpeed((long)block.param2);
                            motorPaintRotation->moveSteps(steps);
                            motorPaintRotation->setSpeed(oldSpeed);  // Restore original speed
                        } else {
                            motorPaintRotation->moveSteps(steps);
                        }
                        
                        // Turn on suction when painting motor starts
                        digitalWrite(SUCTION_PIN, HIGH);
                    }
                    state.motorComplete = false;
                }
            }
        }
        
        // Update execution of all blocks in the current group
        bool allComplete = true;
        for (int i = groupStart; i < groupEnd; i++) {
            if (i >= paintingSequenceCount) break;
            
            PaintingBlock& block = paintingSequence[i];
            BlockExecutionState& state = blockStates[i];
            
            // Update block execution
            if (state.servoActive && !state.servoComplete) {
                updateServoNonBlocking(state.servoTarget, state.servoSpeed);
                if (abs(currentServoAngle - state.servoTarget) < 0.5) {
                    state.servoComplete = true;
                }
            }
            
            if (state.motorActive && !state.motorComplete) {
                if (motorPaintRotation) {
                    long stepsCompleted = abs(motorPaintRotation->getCurrentPosition() - state.motorStepsStart);
                    if (stepsCompleted >= abs(state.motorStepsTarget) || !motorPaintRotation->isMotorRunning()) {
                        state.motorComplete = true;
                        disablePaintRotationMotor();
                        digitalWrite(SUCTION_PIN, LOW);
                    }
                } else {
                    state.motorComplete = true;
                }
            }
            
            // Check if block is complete
            bool blockComplete = true;
            if (block.type == 0) {
                blockComplete = state.servoComplete;
            } else if (block.type == 1) {
                blockComplete = state.motorComplete;
            }
            
            if (!blockComplete) {
                allComplete = false;
            }
        }
        
        // If all blocks in group are complete, move to next group
        if (allComplete) {
            currentBlockIndex = groupEnd;
            
            // Check if we're done with all blocks
            if (currentBlockIndex >= paintingSequenceCount) {
                // All blocks complete - cleanup and return to test state
                if (motorPaintRotation) {
                    motorPaintRotation->forceStop();
                }
                disablePaintRotationMotor();
                digitalWrite(PAINT_GUN_PIN, LOW);
                digitalWrite(SUCTION_PIN, LOW);
                
                paintingStarted = false;
                currentBlockIndex = 0;
                usingCustomSequence = false;
                
                setMachineState(STATE_TEST);
                return;
            }
        }
        
        // Update OTA and check for pause/cancel
        updateOTA();
        CHECK_PAUSE_AND_CANCEL();
        delay(1);
    }
}

