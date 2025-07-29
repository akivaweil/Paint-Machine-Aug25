#ifndef TEST_SEQUENCE_H
#define TEST_SEQUENCE_H

//* ************************************************************************
//* ************************ TEST SEQUENCE STATE ***************************
//* ************************************************************************
// This state handles the test button sequence:
// 1. Move to 5,5
// 2. Extend fork 3.8 inches
// 3. Move Y up 0.5 inches
// 4. Retract fork to 0
// 5. Move to 10,5
// 6. Extend fork 3.8 inches
// 7. Move Y down 0.5 inches
// 8. Retract fork to 0
// 9. Return to 1,1

// Function declarations
void initializeTestSequenceState();
int runTestSequenceState();
void resetTestSequenceState();

#endif // TEST_SEQUENCE_H 