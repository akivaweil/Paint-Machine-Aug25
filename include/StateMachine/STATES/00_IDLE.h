#ifndef IDLE_STATE_H
#define IDLE_STATE_H

//* ************************************************************************
//* ************************ IDLE STATE ************************************
//* ************************************************************************

#include <Arduino.h>
#include "config/Pin_Definitions.h"

class IdleState {
public:
    // Initialize the idle state
    static void initialize();
    
    // Main idle state loop
    static void run();
    
    // Check if we should transition to another state
    static bool shouldTransition();
    
    // Get the next state to transition to
    static int getNextState();
    
    // Cleanup when leaving idle state
    static void cleanup();

private:
    // Check for user input or system events
    static void checkForEvents();
    
    // Update status indicators
    static void updateStatus();
};

#endif // IDLE_STATE_H 