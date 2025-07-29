#ifndef HOMING_STATE_H
#define HOMING_STATE_H

//* ************************************************************************
//* ************************ HOMING STATE **********************************
//* ************************************************************************

#include <Arduino.h>
#include "../../config/Pin_Definitions.h"

class HomingState {
public:
    // Initialize the homing state
    static void initialize();
    
    // Main homing state loop
    static void run();
    
    // Check if homing is complete
    static bool shouldTransition();
    
    // Get the next state to transition to
    static int getNextState();
    
    // Cleanup when leaving homing state
    static void cleanup();

private:
    // Home X axis
    static void homeXAxis();
    
    // Home Y axis
    static void homeYAxis();
    
    // Home Z axis
    static void homeZAxis();
    
    // Check if all axes are homed
    static bool allAxesHomed();
    
    // Update homing progress
    static void updateProgress();
};

#endif // HOMING_STATE_H 