#include <Arduino.h>
#include <Bounce2.h>
#include "StateMachine/STATES/01_HOMING.h"
#include "../../config/Config.h"

// External motor and switch instances (defined in Web_Manager.cpp)
extern StepperMotor* motorX;
extern StepperMotor* motorY;
extern StepperMotor* motorFork;
extern StepperMotor* motorStorage;
extern HomeSwitch* homeSwitchX;
extern HomeSwitch* homeSwitchY;
extern HomeSwitch* homeSwitchFork;
extern Bounce2::Button storagePositionSensor;

// OTA Manager function
extern void updateOTA();

// State machine function
extern void setMachineState(int state);

// Storage motor trim distance
extern long storageMotorTrimDistance;
#define STATE_IDLE 1

//* ************************************************************************
//* ************************ HOMING STATE *********************************
//* ************************************************************************

void homingState() {
    static bool homingComplete = false;
    
    // Perform homing once on first entry
    if (!homingComplete) {
        homeAllAxes();
        homingComplete = true;
        // Transition to idle state after homing
        setMachineState(STATE_IDLE);
    }
}

// Helper function to home a single axis
static void homeSingleAxis(StepperMotor* motor, HomeSwitch* homeSwitch, bool useDual, long maxSpeed) {
    if (!motor || !homeSwitch) return;
    
    // Set homing speed to 700
    motor->setSpeed(700);
    
    // Start continuous movement toward home (positive direction)
    motor->startContinuous(true);
    
    // Keep moving until home switch is triggered
    while (useDual ? !homeSwitch->readDual() : !homeSwitch->read()) {
        motor->runContinuous();
        updateOTA(); // Allow OTA updates during homing
        delay(1);
    }
    
    // Stop motor
    motor->stopContinuous();
    
    // Move 0.2 inches away from home
    motor->moveInches(-0.2);
    while (motor->isMotorRunning()) {
        updateOTA(); // Allow OTA updates during homing
        delay(1);
    }
    
    // Restore full speed for normal operations
    motor->setSpeed(maxSpeed);
}

// Home X axis
void homeXAxis() {
    homeSingleAxis(motorX, homeSwitchX, true, X_MAX_SPEED);  // X uses dual switches
}

// Home Y axis
void homeYAxis() {
    homeSingleAxis(motorY, homeSwitchY, false, Y_MAX_SPEED);
}

// Home Fork Motor axis
void homeForkAxis() {
    if (!motorFork || !homeSwitchFork) return;
    
    Serial.println("[HOMING] Starting Fork axis homing...");
    
    // If already homed, move 0.3 inches away from home first
    if (homeSwitchFork->read()) {
        // Set homing speed to 700
        motorFork->setSpeed(700);
        
        // Move 0.3 inches away from home (negative direction)
        motorFork->moveInches(-0.2);
        while (motorFork->isMotorRunning()) {
            updateOTA(); // Allow OTA updates during homing
            delay(1);
        }
    }
    
    // Set homing speed to 700
    motorFork->setSpeed(700);
    
    // Start continuous movement toward home (positive direction)
    motorFork->startContinuous(true);
    
    // Keep moving until home switch is triggered
    while (!homeSwitchFork->read()) {
        motorFork->runContinuous();
        updateOTA(); // Allow OTA updates during homing
        delay(1);
    }
    
    // Stop motor
    motorFork->stopContinuous();
    
    // Move 0 inches away from home (no offset)
    motorFork->moveInches(0);
    while (motorFork->isMotorRunning()) {
        updateOTA(); // Allow OTA updates during homing
        delay(1);
    }
    
    // Set home offset as position zero
    motorFork->resetPosition();
    
    // Restore full speed for normal operations
    motorFork->setSpeed(FORK_MAX_SPEED);
    
    Serial.println("[HOMING] Fork axis homed");
}

// Home all axes
void homeAllAxes(bool resetColumn) {
    if (!motorX || !homeSwitchX || !motorY || !homeSwitchY || !motorFork || !homeSwitchFork) return;
    
    Serial.println("[HOMING] ========== Starting homing sequence ==========");
    
    //! ************************************************************************
    //! STEP 1: HOME FORK MOTOR FIRST
    //! ************************************************************************
    homeForkAxis();
    
    //! ************************************************************************
    //! STEP 2: HOME X, Y, AND STORAGE MOTORS SIMULTANEOUSLY
    //! ************************************************************************
    if (resetColumn) {
        Serial.println("[HOMING] Starting X, Y, and Storage axes homing...");
    } else {
        Serial.println("[HOMING] Starting X and Y axes homing (Storage skipped - only homes on first boot)...");
    }
    
    // Track which axes are still homing
    bool xHomed = false;
    bool yHomed = false;
    bool storageHomed = false;
    
    // Storage motor only homes on first boot (when resetColumn is true)
    if (resetColumn && motorStorage) {
        // Check if storage position sensor is already triggered at startup
        storagePositionSensor.update();
        bool storageSwitchTriggered = storagePositionSensor.read();
        
        // If storage switch is already triggered, consider it already homed (stay at current column)
        if (storageSwitchTriggered) {
            storageHomed = true;
            Serial.println("[HOMING] Storage switch already triggered - staying at current column position");
        }
    } else {
        // Skip storage motor homing on subsequent homing operations
        storageHomed = true;
    }
    
    // Set homing speeds to 700
    motorX->setSpeed(700);
    motorY->setSpeed(700);
    // Storage motor uses homing speed (only if homing)
    if (motorStorage && resetColumn && !storageHomed) {
        motorStorage->setSpeed(STORAGE_MOTOR_HOMING_SPEED);
    }
    
    // Start continuous movement for X and Y toward home (positive direction)
    motorX->startContinuous(true);
    motorY->startContinuous(true);
    // Start storage motor clockwise to find nearest column (only on first boot and if not already at a column)
    if (motorStorage && resetColumn && !storageHomed) {
        motorStorage->startContinuous(true);
    }
    
    // Keep moving until all home switches/sensors are triggered
    while (!xHomed || !yHomed || !storageHomed) {
        // Update storage position sensor (only if homing storage)
        if (resetColumn && motorStorage && !storageHomed) {
            storagePositionSensor.update();
        }
        
        // Run all motors continuously
        if (!xHomed) {
            motorX->runContinuous();
        }
        if (!yHomed) {
            motorY->runContinuous();
        }
        if (!storageHomed && motorStorage && resetColumn) {
            motorStorage->runContinuous();
        }
        
        // Check switches and stop motors when triggered
        if (!xHomed && homeSwitchX->readDual()) {
            motorX->stopContinuous();
            xHomed = true;
            Serial.println("[HOMING] X axis homed");
        }
        if (!yHomed && homeSwitchY->read()) {
            motorY->stopContinuous();
            yHomed = true;
            Serial.println("[HOMING] Y axis homed");
        }
        if (!storageHomed && motorStorage && resetColumn && storagePositionSensor.read()) {
            // Force stop immediately to prevent overshooting
            motorStorage->forceStop();
            storageHomed = true;
            Serial.println("[HOMING] Storage axis homed to column position (force stopped)");
        }
        
        updateOTA(); // Allow OTA updates during homing
        delay(1);
    }
    
    // Apply trim movement to storage motor after finding home switch
    if (motorStorage && resetColumn && storageHomed) {
        if (storageMotorTrimDistance > 0) {
            Serial.println("[HOMING] Applying storage motor trim movement...");
            motorStorage->moveStepsSmooth(storageMotorTrimDistance);
            while (motorStorage->isMotorRunning()) {
                updateOTA();
                delay(1);
            }
            Serial.println("[HOMING] Storage motor trim movement complete");
        }
    }
    
    // Move X and Y axes 0.5 inches away from home simultaneously
    motorX->moveInches(-0.5);
    motorY->moveInches(-0.5);
    
    // Wait for all motors to finish
    while (motorX->isMotorRunning() || motorY->isMotorRunning() || 
           (motorStorage && resetColumn && motorStorage->isMotorRunning())) {
        updateOTA(); // Allow OTA updates during homing
        delay(1);
    }
    
    // Set home offset as position zero for X, Y, and Storage axes
    motorX->resetPosition();
    motorY->resetPosition();
    if (motorStorage && resetColumn) {
        motorStorage->resetPosition();
    }
    
    // Set current column to A (0) - only if this is initial startup homing
    if (resetColumn) {
        extern int currentColumn;  // Declared in Web_Manager.cpp
        currentColumn = 0;
        Serial.println("[HOMING] Storage motor set to column A");
    } else {
        Serial.println("[HOMING] Column position preserved");
    }
    
    // Restore full speeds for normal operations
    motorX->setSpeed(X_MAX_SPEED);
    motorY->setSpeed(Y_MAX_SPEED);
    if (motorStorage) {
        motorStorage->setSpeed(STORAGE_MOTOR_SPEED);
    }
    
    Serial.println("[HOMING] ========== Homing sequence complete ==========");
}

// Move storage motor to target column (0-5, where 0=A, 5=F)
// Storage motor can ONLY move clockwise
void moveToColumn(int targetColumn) {
    extern int currentColumn;  // Declared in Web_Manager.cpp
    extern StepperMotor* motorStorage;  // Declared in Web_Manager.cpp
    extern Bounce2::Button storagePositionSensor;  // Declared in Web_Manager.cpp
    
    if (!motorStorage) {
        Serial.println("[COLUMN] Storage motor not initialized");
        return;
    }
    
    // Validate target column
    if (targetColumn < 0 || targetColumn > 5) {
        Serial.printf("[COLUMN] Invalid target column: %d\n", targetColumn);
        return;
    }
    
    // If already at target column, no movement needed
    if (currentColumn == targetColumn) {
        Serial.printf("[COLUMN] Already at column %c\n", 'A' + targetColumn);
        return;
    }
    
    // Calculate number of columns to move clockwise
    int columnsToMove = targetColumn - currentColumn;
    if (columnsToMove < 0) {
        // Wrap around (e.g., F to A = 1 column clockwise)
        columnsToMove = 6 + columnsToMove;
    }
    
    Serial.printf("[COLUMN] Moving from column %c to column %c (%d columns clockwise)\n", 
                  'A' + currentColumn, 'A' + targetColumn, columnsToMove);
    
    // Calculate column spacing distance
    long mainSteps = columnsToMove * STORAGE_COLUMN_SPACING_STEPS;
    
    Serial.printf("[COLUMN] Moving %ld steps (%d columns × %ld steps/column)\n", 
                  mainSteps, columnsToMove, STORAGE_COLUMN_SPACING_STEPS);
    
    // Use storage motor speed and acceleration from dashboard settings
    extern long motorSpeedStorage;  // Declared in Web_Manager.cpp
    extern long motorAccelStorage;  // Declared in Web_Manager.cpp
    motorStorage->setSpeed(motorSpeedStorage);
    motorStorage->setAcceleration(motorAccelStorage);
    
    // Move column spacing distance at normal speed
    // Ignore sensor for initial period to clear current column, then count switch activations
    motorStorage->moveSteps(mainSteps);
    long startPosition = motorStorage->getCurrentPosition();
    bool sensorTriggeredEarly = false;
    int switchActivationCount = 0;
    bool previousSensorState = false;
    bool sensorCheckStarted = false;
    
    while (motorStorage->isMotorRunning()) {
        long currentPosition = motorStorage->getCurrentPosition();
        long stepsMoved = abs(currentPosition - startPosition);
        
        // Only check sensor after ignoring initial period to clear current column
        if (stepsMoved > STORAGE_MOTOR_SENSOR_IGNORE_STEPS) {
            storagePositionSensor.update();
            bool currentSensorState = storagePositionSensor.read();
            
            // Initialize previous state on first check
            if (!sensorCheckStarted) {
                previousSensorState = currentSensorState;
                sensorCheckStarted = true;
            }
            
            // Detect rising edge (switch activation)
            if (currentSensorState && !previousSensorState) {
                switchActivationCount++;
                Serial.printf("[COLUMN] Switch activation detected (%d/%d)\n", switchActivationCount, columnsToMove);
            }
            previousSensorState = currentSensorState;
            
            // Only stop if we've detected the correct number of switch activations
            if (switchActivationCount >= columnsToMove) {
                // Correct number of switches activated early - immediately stop to prevent overshooting
                motorStorage->forceStop();
                sensorTriggeredEarly = true;
                Serial.printf("[COLUMN] %d switch activations detected early - force stopped\n", switchActivationCount);
                break;
            }
        }
        updateOTA();
        delay(1);
    }
    
    // Now move at homing speed until switch is detected (only if sensor didn't trigger early)
    if (!sensorTriggeredEarly) {
        Serial.println("[COLUMN] Finding switch at homing speed...");
        motorStorage->setSpeed(STORAGE_MOTOR_HOMING_SPEED);
        motorStorage->startContinuous(true);  // Clockwise
        
        storagePositionSensor.update();
        while (!storagePositionSensor.read()) {
            storagePositionSensor.update();
            motorStorage->runContinuous();
            updateOTA();
            delay(1);
        }
    }
    
    // Transition smoothly to trim movement
    if (storageMotorTrimDistance > 0) {
        motorStorage->moveStepsSmooth(storageMotorTrimDistance);
        while (motorStorage->isMotorRunning()) {
            updateOTA();
            delay(1);
        }
    } else {
        motorStorage->stopContinuous();
    }
    
    // Update current column to target
    currentColumn = targetColumn;
    
    Serial.printf("[COLUMN] Successfully moved to column %c\n", 'A' + currentColumn);
}

// Move storage motor clockwise one column
// Moves STORAGE_COLUMN_SPACING_STEPS, finds switch, then trims
void moveStorageClockwise() {
    extern StepperMotor* motorStorage;  // Declared in Web_Manager.cpp
    extern Bounce2::Button storagePositionSensor;  // Declared in Web_Manager.cpp
    
    if (!motorStorage) {
        Serial.println("[STORAGE] Storage motor not initialized");
        return;
    }
    
    Serial.println("[STORAGE] Moving storage motor clockwise one column...");
    
    // Calculate column spacing distance
    long mainSteps = STORAGE_COLUMN_SPACING_STEPS;
    
    Serial.printf("[STORAGE] Moving %ld steps (1 column × %ld steps/column)\n", 
                  mainSteps, STORAGE_COLUMN_SPACING_STEPS);
    
    // Use storage motor speed and acceleration from dashboard settings
    extern long motorSpeedStorage;  // Declared in Web_Manager.cpp
    extern long motorAccelStorage;  // Declared in Web_Manager.cpp
    motorStorage->setSpeed(motorSpeedStorage);
    motorStorage->setAcceleration(motorAccelStorage);
    
    // Move column spacing distance at normal speed
    // Ignore sensor for initial period to clear current column, then count switch activations
    motorStorage->moveSteps(mainSteps);
    long startPosition = motorStorage->getCurrentPosition();
    bool sensorTriggeredEarly = false;
    int switchActivationCount = 0;
    bool previousSensorState = false;
    bool sensorCheckStarted = false;
    
    while (motorStorage->isMotorRunning()) {
        long currentPosition = motorStorage->getCurrentPosition();
        long stepsMoved = abs(currentPosition - startPosition);
        
        // Only check sensor after ignoring initial period to clear current column
        if (stepsMoved > STORAGE_MOTOR_SENSOR_IGNORE_STEPS) {
            storagePositionSensor.update();
            bool currentSensorState = storagePositionSensor.read();
            
            // Initialize previous state on first check
            if (!sensorCheckStarted) {
                previousSensorState = currentSensorState;
                sensorCheckStarted = true;
            }
            
            // Detect rising edge (switch activation)
            if (currentSensorState && !previousSensorState) {
                switchActivationCount++;
                Serial.printf("[STORAGE] Switch activation detected (%d/1)\n", switchActivationCount);
            }
            previousSensorState = currentSensorState;
            
            // Only stop if we've detected 1 switch activation (moving one column)
            if (switchActivationCount >= 1) {
                // Switch activated early - immediately stop to prevent overshooting
                motorStorage->forceStop();
                sensorTriggeredEarly = true;
                Serial.println("[STORAGE] Switch activation detected early - force stopped");
                break;
            }
        }
        updateOTA();
        delay(1);
    }
    
    // Now move at homing speed until switch is detected (only if sensor didn't trigger early)
    if (!sensorTriggeredEarly) {
        Serial.println("[STORAGE] Finding switch at homing speed...");
        motorStorage->setSpeed(STORAGE_MOTOR_HOMING_SPEED);
        motorStorage->startContinuous(true);  // Clockwise
        
        storagePositionSensor.update();
        while (!storagePositionSensor.read()) {
            storagePositionSensor.update();
            motorStorage->runContinuous();
            updateOTA();
            delay(1);
        }
    }
    
    // Transition smoothly to trim movement
    if (storageMotorTrimDistance > 0) {
        motorStorage->moveStepsSmooth(storageMotorTrimDistance);
        while (motorStorage->isMotorRunning()) {
            updateOTA();
            delay(1);
        }
    } else {
        motorStorage->stopContinuous();
    }
    
    Serial.println("[STORAGE] Successfully moved one column clockwise");
}

