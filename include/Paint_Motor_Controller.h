#ifndef PAINT_MOTOR_CONTROLLER_H
#define PAINT_MOTOR_CONTROLLER_H

#include <Arduino.h>
#include <FastAccelStepper.h>
#include "../src/config/Pin_Definitions.h"
#include "../src/config/Painting_Config.h"

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ ⚙️ STEPPER CONFIGURATION                                              ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

// Stepper Motor Pins (from Pin_Definitions.h)
#define STEPPER_STEP_PIN PAINT_ROTATION_STEP_PIN
#define STEPPER_DIR_PIN PAINT_ROTATION_DIR_PIN
#define STEPPER_ENABLE_PIN PAINT_ROTATION_ENABLE_PIN

// Stepper Motor Parameters (from Painting_Config.h)
#define STEPPER_SPEED PAINT_ROTATION_MOTOR_SPEED
#define STEPPER_ACCEL PAINT_ROTATION_MOTOR_ACCEL

// Function Declarations
void initializeStepper();
void enableStepper();
void moveStepper(long steps);
void stopStepper();
bool isStepperRunning();
long getStepperPosition();
void disableStepper();
void resetStepperPosition();

#endif

