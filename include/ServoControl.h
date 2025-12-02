#ifndef SERVO_CONTROL_H
#define SERVO_CONTROL_H

#include <Arduino.h>

class ServoControl {
private:
    int pin;
    int channel;
    int frequency;
    int resolution;
    int minPulseWidth;
    int maxPulseWidth;
    int minAngle;
    int maxAngle;
    
    int angleToDuty(float angle);
    
public:
    ServoControl();
    
    void init(int servoPin, int pwmChannel = 0, int freq = 50, int res = 16);
    void write(float angle);
    void writeMicroseconds(int microseconds);
    void detach();
    
    // Setters for customization
    void setPulseWidthRange(int minUs, int maxUs);
    void setAngleRange(int minDeg, int maxDeg);
};

#endif

