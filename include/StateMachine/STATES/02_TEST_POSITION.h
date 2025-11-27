#ifndef TEST_POSITION_STATE_H
#define TEST_POSITION_STATE_H

//* ************************************************************************
//* ************************ TEST POSITION STATE HEADER ********************
//* ************************************************************************

// Function declarations for test position state
bool shouldTriggerTestPosition(int currentState); // Check if test position should be triggered (centralized logic)
void initializeTestPositionState();
int runTestPositionState();
int processCommand(String command);
void resetTestPositionState();

#endif // TEST_POSITION_STATE_H 