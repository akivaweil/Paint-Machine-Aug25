#include "Paint_Motor_Controller.h"
#include "StateMachine/FUNCTIONS/StepperMotor.h"

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ ⚙️ STEPPER MOTOR CONTROL                                              ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

// Use shared engine from StepperMotor.cpp (extern declaration)
extern FastAccelStepperEngine* engine;
FastAccelStepper *stepper = NULL;

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 🚀 INITIALIZATION                                                     ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

void initializeStepper() {
    // Initialize engine if not already initialized (shared with StepperMotor)
    if (engine == nullptr) {
        engine = new FastAccelStepperEngine();
        engine->init();
    }
    
    if (engine) {
        // Force MCPWM driver to avoid RMT channel limits
        #ifdef DRIVER_MCPWM_PCNT
        stepper = engine->stepperConnectToPin(STEPPER_STEP_PIN, DRIVER_MCPWM_PCNT);
        #else
        stepper = engine->stepperConnectToPin(STEPPER_STEP_PIN);
        #endif
        
        if (stepper) {
            stepper->setDirectionPin(STEPPER_DIR_PIN);
            stepper->setEnablePin(STEPPER_ENABLE_PIN);
            stepper->setAutoEnable(true);
            stepper->setSpeedInHz(STEPPER_SPEED);
            stepper->setAcceleration(STEPPER_ACCEL);
            Serial.println("Paint Motor initialized successfully");
        } else {
            Serial.println("Failed to initialize Paint Motor stepper!");
        }
    } else {
        Serial.println("Failed to initialize FastAccelStepper Engine!");
    }
}

void enableStepper() {
    if (stepper) {
        stepper->enableOutputs();
    }
}

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 🎮 MOTOR CONTROL FUNCTIONS                                            ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

void moveStepper(long steps) {
    if (stepper) {
        stepper->move(steps);
    }
}

void stopStepper() {
    if (stepper) {
        stepper->forceStopAndNewPosition(stepper->getCurrentPosition());
    }
}

bool isStepperRunning() {
    if (stepper) {
        return stepper->isRunning();
    }
    return false;
}

long getStepperPosition() {
    if (stepper) {
        return stepper->getCurrentPosition();
    }
    return 0;
}

void disableStepper() {
    if (stepper) {
        stepper->disableOutputs();
    }
}

