#ifndef STATE_04_WEB_CONTROL_H
#define STATE_04_WEB_CONTROL_H

#include <Arduino.h>

//* ************************************************************************
//* ************************ WEB CONTROL STATE *****************************
//* ************************************************************************

// Reset the web control state
void resetWebControlState();

// Run the web control state logic
// Returns the next state (usually stays in 4 until done, then returns to 0/IDLE)
int runWebControlState();

#endif

