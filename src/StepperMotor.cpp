#include "StateMachine/FUNCTIONS/StepperMotor.h"
#include "config/Homing_Config.h"
#include "config/Config.h"
#include "config/Carousel_Config.h"

//* ************************************************************************
//* ************************ STEPPER MOTOR IMPLEMENTATION ******************
//* ************************************************************************

StepperMotor::StepperMotor(int stepPin, int dirPin, int homePin, const char* axisName) {
    _stepPin = stepPin;
    _dirPin = dirPin;
    _homePin = homePin;
    _axisName = axisName;
    
    // Initialize motor properties
    _currentPosition = 0.0;
    _isMoving = false;
    
    // Initialize FastAccelStepper pointer
    _stepper = nullptr;
    
    // Initialize Bounce2 objects (only if pins are valid)
    if (_homePin >= 0) {
        _homeSwitchBounce.attach(_homePin, INPUT);
        _homeSwitchBounce.interval(HOME_SWITCH_DEBOUNCE_MS);
    }
}

void StepperMotor::initialize() {
    // Bounce2 objects are already initialized in constructor with proper pin modes
    
    // Debug: Print initial switch states (only if switches exist)
    Serial.print(_axisName);
    if (_homePin >= 0) {
        Serial.print(" - Home pin state: ");
        Serial.print(_homeSwitchBounce.read());
    } else {
        Serial.print(" - No home switch");
    }
    Serial.println();
    
    // Initialize FastAccelStepper engine (only once)
    static FastAccelStepperEngine* engine = nullptr;
    if (!engine) {
        engine = new FastAccelStepperEngine();
        engine->init();
    }
    
    // Create stepper instance using FastAccelStepper
    _stepper = engine->stepperConnectToPin(_stepPin);
    if (_stepper) {
        _stepper->setDirectionPin(_dirPin);
        
        // Set default acceleration and speed (will be overridden during homing)
        _stepper->setAcceleration(MAX_ACCEL);
        _stepper->setSpeedInHz(MAX_SPEED);
        
        Serial.print(_axisName);
        Serial.println(" motor initialized");
    } else {
        Serial.print("ERROR: Failed to initialize ");
        Serial.println(_axisName);
    }
}

void StepperMotor::home() {
    if (!_stepper) {
        Serial.print("ERROR: ");
        Serial.print(_axisName);
        Serial.println(" stepper not initialized");
        return;
    }
    
    // Set individual homing speed and acceleration based on motor type
    float homeSpeed = getHomingSpeed();
    float homeAccel = getHomingAcceleration();
    long homeDistance = getHomingDistance();
    
    // Move towards home switch at individual homing speed
    _isMoving = true;
    
    // Set individual homing speed and acceleration
    _stepper->setAcceleration(homeAccel);
    _stepper->setSpeedInHz(homeSpeed);
    
    // Move towards home using individual distance and direction
    _stepper->moveTo(homeDistance);
    
    Serial.print(_axisName);
    Serial.print(" starting homing sequence (Speed: ");
    Serial.print(homeSpeed);
    Serial.print(" steps/s, Accel: ");
    Serial.print(homeAccel);
    Serial.println(" steps/s²)");
}

void StepperMotor::forceStop() {
    if (_stepper) {
        _stepper->forceStop();
    }
    _isMoving = false;
    
    Serial.print(_axisName);
    Serial.println(" stopped");
}

bool StepperMotor::isMoving() {
    if (_stepper) {
        return _stepper->isRunning();
    }
    return _isMoving;
}

void StepperMotor::setCurrentPositionAsZero() {
    if (_stepper) {
        _stepper->setCurrentPosition(0);
    }
    _currentPosition = 0.0;
    
    Serial.print(_axisName);
    Serial.println(" position set to zero");
}

void StepperMotor::setCurrentPosition(float position) {
    if (_stepper) {
        // Convert inches to steps
        long steps = inchesToSteps(position);
        _stepper->setCurrentPosition(steps);
    }
    _currentPosition = position;
    
    Serial.print(_axisName);
    Serial.print(" position manually set to ");
    Serial.println(position);
}

bool StepperMotor::isHomeSwitchTriggered() {
    //! ************************************************************************
    //! STEP 1: FAST HOME SWITCH DETECTION WITH DEBOUNCING
    //! ************************************************************************
    // Check if home switch exists
    if (_homePin < 0) {
        return false; // No home switch
    }
    
    // Use debounced read for reliable detection while maintaining responsiveness
    return _homeSwitchBounce.read() == HIGH; // Active HIGH
    
    // Note: Bounce2 library with 1ms debounce provides fast response while preventing false triggers
}



void StepperMotor::update() {
    if (!_stepper) return;
    
    // Update switch debouncing
    updateSwitches();
    
    // Update current position from stepper
    _currentPosition = stepsToInches(_stepper->getCurrentPosition());
    
    //! ************************************************************************
    //! STEP 1: HOME SWITCH PROTECTION DURING NORMAL OPERATION
    //! ************************************************************************
    // Check if home switch is triggered during normal movement (not homing)
    // This prevents motors from moving past the home switch
    if (_isMoving && isHomeSwitchTriggered()) {
        // If home switch is triggered, stop movement immediately
        // This prevents the motor from moving past the home position
        _stepper->forceStop();
        _isMoving = false;
        _currentPosition = 0.0; // Set to home position
        _stepper->setCurrentPosition(0);
        
        Serial.print(_axisName);
        Serial.println(" stopped at home switch during normal operation");
    }
    
    // Check if movement is complete
    if (!_stepper->isRunning()) {
        _isMoving = false;
    }
}

// Helper functions to get individual motor homing settings
float StepperMotor::getHomingSpeed() {
    if (strcmp(_axisName, "X1") == 0) return X1_HOME_SPEED;
    if (strcmp(_axisName, "X2") == 0) return X2_HOME_SPEED;
    if (strcmp(_axisName, "Y") == 0) return Y_HOME_SPEED;
    if (strcmp(_axisName, "Fork") == 0) return FORK_HOME_SPEED;
    if (strcmp(_axisName, "Storage") == 0) return STORAGE_HOME_SPEED;
    return X1_HOME_SPEED; // Default fallback using X1 settings
}

float StepperMotor::getHomingAcceleration() {
    if (strcmp(_axisName, "X1") == 0) return X1_HOME_ACCEL;
    if (strcmp(_axisName, "X2") == 0) return X2_HOME_ACCEL;
    if (strcmp(_axisName, "Y") == 0) return Y_HOME_ACCEL;
    if (strcmp(_axisName, "Fork") == 0) return FORK_HOME_ACCEL;
    if (strcmp(_axisName, "Storage") == 0) return STORAGE_HOME_ACCEL;
    return X1_HOME_ACCEL; // Default fallback using X1 settings
}

long StepperMotor::getHomingDistance() {
    long distance;
    if (strcmp(_axisName, "X1") == 0) distance = X1_HOME_DISTANCE_STEPS;
    else if (strcmp(_axisName, "X2") == 0) distance = X2_HOME_DISTANCE_STEPS;
    else if (strcmp(_axisName, "Y") == 0) distance = Y_HOME_DISTANCE_STEPS;
    else if (strcmp(_axisName, "Fork") == 0) distance = FORK_HOME_DISTANCE_STEPS;
    else if (strcmp(_axisName, "Storage") == 0) distance = STORAGE_HOME_DISTANCE_STEPS;
    else distance = 10000; // Default fallback
    
    // Apply direction based on configuration
    if (strcmp(_axisName, "X1") == 0 && !X1_HOME_DIRECTION_POSITIVE) distance = -distance;
    else if (strcmp(_axisName, "X2") == 0 && !X2_HOME_DIRECTION_POSITIVE) distance = -distance;
    else if (strcmp(_axisName, "Y") == 0 && !Y_HOME_DIRECTION_POSITIVE) distance = -distance;
    else if (strcmp(_axisName, "Fork") == 0 && !FORK_HOME_DIRECTION_POSITIVE) distance = -distance;
    else if (strcmp(_axisName, "Storage") == 0 && !STORAGE_HOME_DIRECTION_POSITIVE) distance = -distance;
    
    return distance;
}

long StepperMotor::inchesToSteps(float inches) {
    return (long)(inches * STEPS_PER_INCH);
}

float StepperMotor::stepsToInches(long steps) {
    return (float)steps / STEPS_PER_INCH;
}

void StepperMotor::moveToPosition(float position) {
    if (!_stepper) {
        Serial.print("ERROR: ");
        Serial.print(_axisName);
        Serial.println(" stepper not initialized");
        return;
    }
    
    //! ************************************************************************
    //! STEP 1: VALIDATE POSITION BOUNDS (SKIP FOR STORAGE MOTOR)
    //! ************************************************************************
    // Check if position is within safe limits (skip for storage motor - no limits)
    if (strcmp(_axisName, "Storage") != 0) {
        if (position < MIN_TRAVEL_INCHES) {
            Serial.print("ERROR: ");
            Serial.print(_axisName);
            Serial.print(" position ");
            Serial.print(position);
            Serial.print(" inches is below minimum ");
            Serial.print(MIN_TRAVEL_INCHES);
            Serial.println(" inches");
            return;
        }
        
        if (position > MAX_TRAVEL_INCHES) {
            Serial.print("ERROR: ");
            Serial.print(_axisName);
            Serial.print(" position ");
            Serial.print(position);
            Serial.print(" inches is above maximum ");
            Serial.print(MAX_TRAVEL_INCHES);
            Serial.println(" inches");
            return;
        }
    }
    
    //! ************************************************************************
    //! STEP 2: HOME SWITCH PROTECTION - PREVENT MOVING PAST HOME
    //! ************************************************************************
    // Check if home switch is currently triggered and we're trying to move past it
    if (isHomeSwitchTriggered() && position < _currentPosition) {
        Serial.print("ERROR: ");
        Serial.print(_axisName);
        Serial.print(" cannot move to ");
        Serial.print(position);
        Serial.print(" inches - would require moving past home switch (current: ");
        Serial.print(_currentPosition);
        Serial.println(" inches)");
        return;
    }
    
    // Convert position to steps
    long targetSteps = inchesToSteps(position);
    
    // Set speed and acceleration based on motor type
    if (strcmp(_axisName, "Storage") == 0) {
        // Use storage-specific settings
        _stepper->setAcceleration(STORAGE_MOTOR_ACCEL);
        _stepper->setSpeedInHz(STORAGE_MOTOR_SPEED);
    } else {
        // Use standard settings for other motors
        _stepper->setAcceleration(MAX_ACCEL);
        _stepper->setSpeedInHz(MAX_SPEED);
    }
    
    // Debug: Print speed/accel settings for X motors
    if (strcmp(_axisName, "X1") == 0 || strcmp(_axisName, "X2") == 0) {
        Serial.print(_axisName);
        Serial.print(" move settings - Speed: ");
        Serial.print(MAX_SPEED);
        Serial.print(", Accel: ");
        Serial.print(MAX_ACCEL);
        Serial.print(", Dist: ");
        Serial.print(targetSteps - _stepper->getCurrentPosition());
        Serial.println(" steps");
    }
    
    // Move to target position
    _stepper->moveTo(targetSteps);
    _isMoving = true;
    
    // Debug output
    Serial.print(_axisName);
    Serial.print(" moving to position: ");
    Serial.print(position);
    Serial.print(" inches (");
    Serial.print(targetSteps);
    Serial.println(" steps)");
}

void StepperMotor::moveRelative(float inches) {
    float targetPos = _currentPosition + inches;
    moveToPosition(targetPos);
}

float StepperMotor::getCurrentPosition() {
    return _currentPosition;
}

void StepperMotor::updateSwitches() {
    //! ************************************************************************
    //! STEP 1: FREQUENT SWITCH DEBOUNCING UPDATE FOR MAXIMUM RESPONSIVENESS
    //! ************************************************************************
    // Update Bounce2 objects to handle debouncing - called frequently for fast response
    // Only update if switches exist
    if (_homePin >= 0) {
        _homeSwitchBounce.update();
    }    
    // Note: This method should be called as frequently as possible during homing operations
}

void StepperMotor::updateHoming() {
    if (!_stepper) return;
    
    //! ************************************************************************
    //! STEP 1: IMMEDIATE SWITCH DEBOUNCING UPDATE FOR FASTER RESPONSE
    //! ************************************************************************
    // Update switch debouncing immediately for faster response
    updateSwitches();
    
    // Update current position from stepper
    _currentPosition = stepsToInches(_stepper->getCurrentPosition());
    
    //! ************************************************************************
    //! STEP 2: AGGRESSIVE HOME SWITCH DETECTION DURING HOMING
    //! ************************************************************************
    // Check if home switch is triggered during homing with immediate response
    // Only stop if we're in the initial homing phase (not moving away)
    if (_isMoving && isHomeSwitchTriggered()) {
        // Check if we're trying to move away from home (current position > 0 means we've already homed)
        if (_currentPosition <= 0.0) {
            //! ************************************************************************
            //! STEP 3: IMMEDIATE FORCE STOP WHEN HOME SWITCH IS TRIGGERED
            //! ************************************************************************
            _stepper->forceStop(); // Use forceStop for immediate response
            _isMoving = false;
            _currentPosition = 0.0; // Set home position
            _stepper->setCurrentPosition(0);
            
            Serial.print(_axisName);
            Serial.println(" reached home switch - FORCE STOPPED");
            return;
        } else {
            // We're moving away from home, don't stop
            Serial.print(_axisName);
            Serial.println(" moving away from home - ignoring home switch");
        }
    }
    
    // Check if movement is complete
    if (!_stepper->isRunning()) {
        _isMoving = false;
    }
}

bool StepperMotor::isHomingComplete() {
    return isHomeSwitchTriggered() && !isMoving();
}

void StepperMotor::moveAwayFromHome(float distance) {
    if (!_stepper) {
        Serial.print("ERROR: ");
        Serial.print(_axisName);
        Serial.println(" stepper not initialized");
        return;
    }
    
    // Use provided distance (if > 0) or configured default distance
    // If distance is explicitly passed as 0.0 or negative, we treat it as default request
    // But since we want to support 0.0 offset, we should check if it's exactly 0.0 AND not the overloaded call
    // The header default is 0.0. Let's change logic:
    // If distance is roughly 0.0 (default param), use config.
    // This logic is a bit flawed if we pass 0.0 intentionally.
    // Better approach: The caller passes the FULL distance including offset.
    
    float moveDistance = (distance > 0.001) ? distance : MOVE_AWAY_FROM_HOME_DISTANCE;
    
    // Use explicit move away direction configuration (not based on homing direction)
    if (strcmp(_axisName, "X1") == 0) {
        moveDistance = X1_MOVE_AWAY_DIRECTION_POSITIVE ? moveDistance : -moveDistance;
    } else if (strcmp(_axisName, "X2") == 0) {
        moveDistance = X2_MOVE_AWAY_DIRECTION_POSITIVE ? moveDistance : -moveDistance;
    } else if (strcmp(_axisName, "Y") == 0) {
        moveDistance = Y_MOVE_AWAY_DIRECTION_POSITIVE ? moveDistance : -moveDistance;
    } else if (strcmp(_axisName, "Fork") == 0) {
        moveDistance = FORK_MOVE_AWAY_DIRECTION_POSITIVE ? moveDistance : -moveDistance;
    } else if (strcmp(_axisName, "Storage") == 0) {
        moveDistance = STORAGE_MOVE_AWAY_DIRECTION_POSITIVE ? moveDistance : -moveDistance;
    }
    
    // Debug: Show current position and move direction
    Serial.print(_axisName);
    Serial.print(" current position: ");
    Serial.print(_currentPosition);
    Serial.print(" inches, moving away: ");
    Serial.print(moveDistance);
    Serial.println(" inches");
    
    // Check if home switch is still triggered - if so, we need to move away from it
    if (isHomeSwitchTriggered()) {
        Serial.print(_axisName);
        Serial.println(" Home switch still triggered - moving away from switch");
    }
    
    // Convert to steps - this should be a relative movement from current position
    long currentSteps = _stepper->getCurrentPosition();
    long targetSteps = currentSteps + inchesToSteps(moveDistance);
    
    // Set speed and acceleration based on motor type
    if (strcmp(_axisName, "Storage") == 0) {
        // Use storage-specific settings
        _stepper->setAcceleration(STORAGE_MOTOR_ACCEL);
        _stepper->setSpeedInHz(STORAGE_MOTOR_SPEED);
    } else {
        // Use standard settings for other motors
        _stepper->setAcceleration(MAX_ACCEL);
        _stepper->setSpeedInHz(MAX_SPEED);
    }
    
    // Move to target position
    _stepper->moveTo(targetSteps);
    _isMoving = true;
    
    // Debug output
    Serial.print(_axisName);
    Serial.print(" moving away from home: ");
    Serial.print(moveDistance);
    Serial.print(" inches (current steps: ");
    Serial.print(currentSteps);
    Serial.print(", target steps: ");
    Serial.print(targetSteps);
    Serial.println(")");
}

void StepperMotor::testMoveAwayDirection() {
    Serial.print(_axisName);
    Serial.print(" - Homing direction: ");
    
    // Show homing direction
    if (strcmp(_axisName, "X1") == 0) {
        Serial.print(X1_HOME_DIRECTION_POSITIVE ? "POSITIVE" : "NEGATIVE");
    } else if (strcmp(_axisName, "X2") == 0) {
        Serial.print(X2_HOME_DIRECTION_POSITIVE ? "POSITIVE" : "NEGATIVE");
    } else if (strcmp(_axisName, "Y") == 0) {
        Serial.print(Y_HOME_DIRECTION_POSITIVE ? "POSITIVE" : "NEGATIVE");
    } else if (strcmp(_axisName, "Fork") == 0) {
        Serial.print(FORK_HOME_DIRECTION_POSITIVE ? "POSITIVE" : "NEGATIVE");
    } else if (strcmp(_axisName, "Storage") == 0) {
        Serial.print("NO HOMING (Storage motor)");
    }
    
    Serial.print(", Move away direction: ");
    
    // Show move away direction
    if (strcmp(_axisName, "X1") == 0) {
        Serial.print(X1_MOVE_AWAY_DIRECTION_POSITIVE ? "POSITIVE" : "NEGATIVE");
    } else if (strcmp(_axisName, "X2") == 0) {
        Serial.print(X2_MOVE_AWAY_DIRECTION_POSITIVE ? "POSITIVE" : "NEGATIVE");
    } else if (strcmp(_axisName, "Y") == 0) {
        Serial.print(Y_MOVE_AWAY_DIRECTION_POSITIVE ? "POSITIVE" : "NEGATIVE");
    } else if (strcmp(_axisName, "Fork") == 0) {
        Serial.print(FORK_MOVE_AWAY_DIRECTION_POSITIVE ? "POSITIVE" : "NEGATIVE");
    } else if (strcmp(_axisName, "Storage") == 0) {
        Serial.print(STORAGE_MOVE_AWAY_DIRECTION_POSITIVE ? "POSITIVE" : "NEGATIVE");
    }
    
    Serial.print(", Distance: ");
    Serial.print(MOVE_AWAY_FROM_HOME_DISTANCE);
    Serial.println(" inches");
}

void StepperMotor::startContinuousMovement(float speed) {
    if (!_stepper) {
        Serial.print("ERROR: ");
        Serial.print(_axisName);
        Serial.println(" stepper not initialized");
        return;
    }
    
    //! ************************************************************************
    //! STEP 1: START CONTINUOUS MOVEMENT FOR STORAGE MOTOR
    //! ************************************************************************
    // Set speed for continuous movement (use storage-specific speed if no speed provided)
    float continuousSpeed = (speed > 0) ? speed : STORAGE_MOTOR_CONTINUOUS_SPEED;
    _stepper->setSpeedInHz(continuousSpeed);
    
    // Start continuous movement in positive direction
    _stepper->runForward();
    _isMoving = true;
    
    Serial.print(_axisName);
    Serial.print(" starting continuous movement at ");
    Serial.print(speed);
    Serial.println(" Hz");
}

void StepperMotor::stopContinuousMovement() {
    if (!_stepper) {
        Serial.print("ERROR: ");
        Serial.print(_axisName);
        Serial.println(" stepper not initialized");
        return;
    }
    
    //! ************************************************************************
    //! STEP 1: STOP CONTINUOUS MOVEMENT
    //! ************************************************************************
    // Stop the motor immediately
    _stepper->forceStop();
    _isMoving = false;
    
    Serial.print(_axisName);
    Serial.println(" continuous movement stopped");
}