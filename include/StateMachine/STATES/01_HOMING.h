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

// Column movement function
void moveToColumn(int targetColumn);  // targetColumn: 0-5 (0=A, 5=F)

#endif // HOMING_STATE_H

