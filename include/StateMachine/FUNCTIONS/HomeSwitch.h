#ifndef HOME_SWITCH_H
#define HOME_SWITCH_H

#include <Bounce2.h>
#include "../../../src/config/Pin_Definitions.h"
#include "../../../src/config/Homing_Config.h"

// Switch types
enum SwitchType {
    SWITCH_X,
    SWITCH_Y,
    SWITCH_FORK
};

class HomeSwitch {
private:
    Bounce* bounceX1;      // X home switch 1
    Bounce* bounceX2;      // X home switch 2 (if exists)
    Bounce* bounceY;       // Y home switch
    Bounce* bounceFork;    // Fork home switch

    bool hasX2Switch;      // Flag to indicate if X has two switches

public:
    // Constructor
    HomeSwitch();

    // Initialize switches
    void init();

    // Update switch states (call this in main loop)
    void update();

    // Check if switches are triggered
    bool isXHome();
    bool isYHome();
    bool isForkHome();

    // Check individual X switches (for two-switch X axis)
    bool isX1Home();
    bool isX2Home();

    // Check if all required switches are home
    bool areAllHome();  // X, Y, and Fork all home

    // Get raw pin readings
    bool getX1Raw();
    bool getX2Raw();
    bool getYRaw();
    bool getForkRaw();

    // Debug function
    void printDebugInfo();
};

#endif // HOME_SWITCH_H
