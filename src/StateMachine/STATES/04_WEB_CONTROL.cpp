#include "StateMachine/STATES/04_WEB_CONTROL.h"
#include "StateMachine/WEB_CONTROL/WebControl_Logic.h"
#include "config/Config.h"

//* ************************************************************************
//* ************************ WEB CONTROL STATE *****************************
//* ************************************************************************

void resetWebControlState() {
    initializeWebControlLogic();
}

int runWebControlState() {
    return executeWebControlLogic();
}
