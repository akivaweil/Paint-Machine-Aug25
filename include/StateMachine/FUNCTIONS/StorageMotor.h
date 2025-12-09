#ifndef STORAGE_MOTOR_H
#define STORAGE_MOTOR_H

#include <Arduino.h>
#include <AccelStepper.h>

class StorageMotor {
private:
    AccelStepper* stepper;
    
    // Motor parameters
    float stepsPerInch;
    long maxSpeed;
    long maxAccel;
    
    // Current state
    bool continuousMode;
    bool continuousDirection;

public:
    // Constructor
    StorageMotor(uint8_t step, uint8_t dir, float stepsPerInch, long maxSpd, long maxAcc);

    // Basic motor control
    void setSpeed(long speed);
    void setAcceleration(long acceleration);
    void overrideSpeed(long speed);
    void step();
    void moveSteps(long steps);
    void moveStepsSmooth(long steps);
    void moveInches(float inches);

    // Continuous movement control
    void startContinuous(bool positive = true);
    void stopContinuous();
    void runContinuous();

    // Position tracking
    long getCurrentPosition();
    void setCurrentPosition(long position);
    void resetPosition();

    // Force stop functionality
    void forceStop();

    // Status
    bool isMotorRunning();

    // Movement calculations
    long inchesToSteps(float inches);
    float stepsToInches(long steps);
    
    // Update steps per inch
    void setStepsPerInch(float stepsPerInch);
    
    // Must be called in loop() for AccelStepper to work
    void run();
};

#endif // STORAGE_MOTOR_H

