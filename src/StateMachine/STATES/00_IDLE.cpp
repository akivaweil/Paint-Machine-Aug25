#include <Arduino.h>
#include "StateMachine/STATES/00_IDLE.h"

// OTA Manager function
extern void updateOTA();

//* ************************************************************************
//* ************************ IDLE STATE ***********************************
//* ************************************************************************

void idleState() {
    // Allow OTA updates during idle state
    // (updateOTA is also called in main loop, but this ensures it's called here too)
}

