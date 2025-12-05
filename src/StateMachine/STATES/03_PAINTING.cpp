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

// Storage function for loading painting sequence
extern String loadPaintingSequenceJSON();

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

// Simple structure to hold block data
struct PaintingBlock {
    String type;
    float angle;
    float speed;
    float degrees;
    bool isValid;
};

// Simple JSON parser for painting sequence
// Expected format: {"blocks":[{"type":"servo_angle","angle":220,"speed":30},...]}
int parsePaintingSequence(String json, PaintingBlock* blocks, int maxBlocks) {
    int blockCount = 0;
    int jsonLen = json.length();
    
    // Find blocks array
    int blocksStart = json.indexOf("\"blocks\"");
    if (blocksStart == -1) return 0;
    
    int arrayStart = json.indexOf('[', blocksStart);
    if (arrayStart == -1) return 0;
    
    int pos = arrayStart + 1;
    
    while (pos < jsonLen && blockCount < maxBlocks) {
        // Find next block object
        int objStart = json.indexOf('{', pos);
        if (objStart == -1) break;
        
        int objEnd = json.indexOf('}', objStart);
        if (objEnd == -1) break;
        
        String blockStr = json.substring(objStart, objEnd + 1);
        
        // Initialize block
        blocks[blockCount].isValid = false;
        blocks[blockCount].angle = 0;
        blocks[blockCount].speed = 0;
        blocks[blockCount].degrees = 0;
        
        // Parse type
        int typeStart = blockStr.indexOf("\"type\"");
        if (typeStart != -1) {
            int colonPos = blockStr.indexOf(':', typeStart);
            int quote1 = blockStr.indexOf('"', colonPos);
            int quote2 = blockStr.indexOf('"', quote1 + 1);
            if (quote1 != -1 && quote2 != -1) {
                blocks[blockCount].type = blockStr.substring(quote1 + 1, quote2);
                blocks[blockCount].isValid = true;
            }
        }
        
        // Parse angle (for servo_angle)
        int angleStart = blockStr.indexOf("\"angle\"");
        if (angleStart != -1) {
            int colonPos = blockStr.indexOf(':', angleStart);
            int valueEnd = blockStr.indexOf(',', colonPos);
            if (valueEnd == -1) valueEnd = blockStr.indexOf('}', colonPos);
            if (valueEnd != -1) {
                String valueStr = blockStr.substring(colonPos + 1, valueEnd);
                valueStr.trim();
                blocks[blockCount].angle = valueStr.toFloat();
            }
        }
        
        // Parse speed (for servo_angle)
        int speedStart = blockStr.indexOf("\"speed\"");
        if (speedStart != -1) {
            int colonPos = blockStr.indexOf(':', speedStart);
            int valueEnd = blockStr.indexOf(',', colonPos);
            if (valueEnd == -1) valueEnd = blockStr.indexOf('}', colonPos);
            if (valueEnd != -1) {
                String valueStr = blockStr.substring(colonPos + 1, valueEnd);
                valueStr.trim();
                blocks[blockCount].speed = valueStr.toFloat();
            }
        }
        
        // Parse degrees (for paint_motor_degrees)
        int degreesStart = blockStr.indexOf("\"degrees\"");
        if (degreesStart != -1) {
            int colonPos = blockStr.indexOf(':', degreesStart);
            int valueEnd = blockStr.indexOf(',', colonPos);
            if (valueEnd == -1) valueEnd = blockStr.indexOf('}', colonPos);
            if (valueEnd != -1) {
                String valueStr = blockStr.substring(colonPos + 1, valueEnd);
                valueStr.trim();
                blocks[blockCount].degrees = valueStr.toFloat();
            }
        }
        
        if (blocks[blockCount].isValid) {
            blockCount++;
        }
        
        pos = objEnd + 1;
    }
    
    return blockCount;
}

// Paint motor 360 turn tracking
static long paintMotor360StepsTarget = 0;
static long paintMotor360StepsStart = 0;

// Parallel sequence handler (called during wait loops)
// Handles servo movement and paint gun, paint motor runs independently
void updateParallelSequence(bool& parallelSequenceStarted, int& parallelStep) {
    if (!parallelSequenceStarted) return;
    
    if (parallelStep == 0) {
        // Turn on paint gun and start servo to 220
        digitalWrite(PAINT_GUN_PIN, HIGH);
        parallelStep = 1;
    }
    else if (parallelStep == 1) {
        // Ensure suction is on while painting motor is rotating
        if (motorPaintRotation && motorPaintRotation->isMotorRunning()) {
            digitalWrite(SUCTION_PIN, HIGH);
        }
        
        // Rotate servo to 220 degrees (non-blocking)
        updateServoNonBlocking(220.0);
        
        // Check if paint motor reached 180 degrees (halfway through 360)
        if (motorPaintRotation) {
            long stepsCompleted = motorPaintRotation->getCurrentPosition() - paintMotor360StepsStart;
            long halfwaySteps = paintRotationMotorStepsPerRevOutput / 2;
            
            if (stepsCompleted >= halfwaySteps) {
                parallelStep = 2;  // Start returning servo
            }
        }
        
        // Fallback: if motor stops before halfway, still continue
        if (motorPaintRotation && !motorPaintRotation->isMotorRunning()) {
            parallelStep = 2;
        }
    }
    else if (parallelStep == 2) {
        // Ensure suction is on while painting motor is rotating
        if (motorPaintRotation && motorPaintRotation->isMotorRunning()) {
            digitalWrite(SUCTION_PIN, HIGH);
        }
        
        // Rotate servo back to servo home angle
        updateServoNonBlocking(SERVO_HOME_ANGLE);
        
        bool servoComplete = abs(currentServoAngle - SERVO_HOME_ANGLE) < 0.5;
        bool motorComplete = !motorPaintRotation || !motorPaintRotation->isMotorRunning();
        
        if (servoComplete && motorComplete) {
            parallelStep = 3;
        }
    }
    else if (parallelStep == 3) {
        // Cleanup: disable motor and turn off paint gun and suction
        disablePaintRotationMotor();
        digitalWrite(PAINT_GUN_PIN, LOW);
        digitalWrite(SUCTION_PIN, LOW);
        parallelSequenceStarted = false;
    }
}

void paintingState() {
    static int step = 0;
    static bool paintingStarted = false;
    static bool usingCustomSequence = false;
    static PaintingBlock customBlocks[20];  // Max 20 blocks
    static int customBlockCount = 0;
    static int currentBlockIndex = 0;
    static bool blockExecuting = false;
    static float savedServoSpeed = 0.0;
    
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
        
        // Restore servo speed if it was changed
        if (savedServoSpeed > 0.0) {
            servoSpeed = savedServoSpeed;
            savedServoSpeed = 0.0;
        }
        
        // Reset flags and state
        cycleCancelled = false;
        cyclePaused = false;
        paintingStarted = false;
        step = 0;
        usingCustomSequence = false;
        currentBlockIndex = 0;
        blockExecuting = false;
        
        // Return to test state (test state will handle further cleanup)
        setMachineState(STATE_TEST);
        return;
    }
    
    // Initialize on first entry
    if (!paintingStarted) {
        // Check for custom sequence
        String sequenceJson = loadPaintingSequenceJSON();
        customBlockCount = parsePaintingSequence(sequenceJson, customBlocks, 20);
        
        if (customBlockCount > 0) {
            usingCustomSequence = true;
            currentBlockIndex = 0;
            blockExecuting = false;
        } else {
            usingCustomSequence = false;
            step = 0;
        }
        
        paintingStarted = true;
        cyclePaused = false;  // Reset pause flag on new painting
        cycleCancelled = false;  // Reset cancel flag on new painting
        savedServoSpeed = 0.0;
    }
    
    // Execute custom sequence
    if (usingCustomSequence) {
        // Execute blocks sequentially
        while (currentBlockIndex < customBlockCount) {
            CHECK_PAUSE_AND_CANCEL();
            
            PaintingBlock& block = customBlocks[currentBlockIndex];
            
            if (block.type == "servo_angle") {
                if (!blockExecuting) {
                    // Save current servo speed if custom speed is specified
                    if (block.speed > 0.0 && savedServoSpeed == 0.0) {
                        savedServoSpeed = servoSpeed;
                        servoSpeed = block.speed;
                    }
                    blockExecuting = true;
                }
                
                // Move servo non-blocking with custom speed
                updateServoNonBlocking(block.angle, block.speed);
                
                // Check if servo reached target
                if (abs(currentServoAngle - block.angle) < 0.5) {
                    // Block complete, move to next
                    currentBlockIndex++;
                    blockExecuting = false;
                } else {
                    // Still moving, wait
                    updateOTA();
                    delay(10);
                    return;
                }
            }
            else if (block.type == "paint_motor_degrees") {
                if (!blockExecuting) {
                    // Convert degrees to steps
                    long steps = (long)((block.degrees / 360.0) * paintRotationMotorStepsPerRevOutput);
                    
                    // Enable motor and start movement
                    enablePaintRotationMotor();
                    delay(50);
                    digitalWrite(SUCTION_PIN, HIGH);
                    
                    if (motorPaintRotation) {
                        motorPaintRotation->moveSteps(steps);
                    }
                    
                    blockExecuting = true;
                }
                
                // Wait for motor to finish
                if (motorPaintRotation && motorPaintRotation->isMotorRunning()) {
                    updateOTA();
                    delay(10);
                    return;
                } else {
                    // Motor finished
                    delay(50);
                    digitalWrite(SUCTION_PIN, LOW);
                    disablePaintRotationMotor();
                    
                    // Block complete, move to next
                    currentBlockIndex++;
                    blockExecuting = false;
                }
            }
            else {
                // Unknown block type, skip
                currentBlockIndex++;
                blockExecuting = false;
            }
        }
        
        // All blocks executed
        // Restore servo speed if it was changed
        if (savedServoSpeed > 0.0) {
            servoSpeed = savedServoSpeed;
            savedServoSpeed = 0.0;
        }
        
        // Ensure paint gun and suction are off
        digitalWrite(PAINT_GUN_PIN, LOW);
        digitalWrite(SUCTION_PIN, LOW);
        
        // Reset painting state flags
        paintingStarted = false;
        usingCustomSequence = false;
        currentBlockIndex = 0;
        blockExecuting = false;
        
        // Return to test state to continue with remaining steps
        setMachineState(STATE_TEST);
        return;
    }
    
    // Fallback to hardcoded sequence if no custom sequence
    // (Original hardcoded sequence code removed - now always uses custom sequence or does nothing)
    // If we reach here, there's no custom sequence, so just return to test state
    paintingStarted = false;
    setMachineState(STATE_TEST);
}

