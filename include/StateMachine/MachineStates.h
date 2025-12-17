#ifndef MACHINE_STATES_H
#define MACHINE_STATES_H

// State machine state
enum MachineState {
    STATE_HOMING,
    STATE_IDLE,
    STATE_PICK_PLACE,
    STATE_PAINTING
};

extern MachineState currentState;

// State functions
void homingState();
void idleState();
void pickPlaceState();
void paintingState();

// State management functions
void setMachineState(int newState);
int getMachineState();

#endif // MACHINE_STATES_H
