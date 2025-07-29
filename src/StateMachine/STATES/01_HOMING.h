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
    // Home X1 axis (Left)
    static void homeX1Axis();
    
    // Home X2 axis (Right)
    static void homeX2Axis();
    
    // Home Y axis
    static void homeYAxis();
    
    // Home Fork axis (Z direction)
    static void homeForkAxis();
    
    // Check if all axes are homed
    static bool allAxesHomed();
    
    // Update homing progress
    static void updateProgress();
};

#endif // HOMING_STATE_H 