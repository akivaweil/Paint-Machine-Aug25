#ifndef HOMING_STATE_H
#define HOMING_STATE_H

#include <Arduino.h>
#include "../FUNCTIONS/StepperMotor.h"
#include "../FUNCTIONS/HomeSwitch.h"

//* ************************************************************************
//* ************************ HOMING STATE *********************************
//* ************************************************************************

void homingState();

// Homing functions
void homeXAxis();
void homeYAxis();
void homeForkAxis();
void homeAllAxes();

#endif // HOMING_STATE_H

