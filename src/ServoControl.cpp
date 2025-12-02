#include "ServoControl.h"

#include "soc/soc_caps.h"

//DO NOT CHANGE THIS FILE AT ALL

ServoControl::ServoControl() {
    pin = -1;
    channel = -1;
    frequency = 50;
    resolution = 16;
    minPulseWidth = 500;
    maxPulseWidth = 2500;
    minAngle = 0;
    maxAngle = 180;
}

void ServoControl::init(int servoPin, int pwmChannel, int freq, int res) {
    pin = servoPin;
    channel = pwmChannel;
    frequency = freq;
    resolution = res;

#if defined(SOC_LEDC_TIMER_BIT_WIDE_NUM)
    int maxResolution = SOC_LEDC_TIMER_BIT_WIDE_NUM;
#else
    int maxResolution = resolution;
#endif

    if (resolution > maxResolution) {
        resolution = maxResolution;
    }

    int configuredFreq = ledcSetup(channel, frequency, resolution);
    while (configuredFreq == 0 && resolution > 10) {
        resolution--;
        configuredFreq = ledcSetup(channel, frequency, resolution);
    }

    if (configuredFreq == 0) {
        channel = -1;
        return;
    }

    ledcAttachPin(pin, channel);
}

int ServoControl::angleToDuty(float angle) {
    // Constrain angle to valid range
    if (angle < minAngle) angle = minAngle;
    if (angle > maxAngle) angle = maxAngle;
    
    // Map angle to pulse width in microseconds
    float pulseWidth = map(angle, minAngle, maxAngle, minPulseWidth, maxPulseWidth);
    
    // Convert pulse width to duty cycle
    // For 16-bit resolution: duty = (pulseWidth / 20000) * 65535
    int maxDuty = (1 << resolution) - 1;
    int duty = (pulseWidth / (1000000.0 / frequency)) * maxDuty;
    
    return duty;
}

void ServoControl::write(float angle) {
    if (channel >= 0) {
        int duty = angleToDuty(angle);
        ledcWrite(channel, duty);
    }
}

void ServoControl::writeMicroseconds(int microseconds) {
    if (channel >= 0) {
        // Convert microseconds to duty cycle
        int maxDuty = (1 << resolution) - 1;
        int duty = (microseconds / (1000000.0 / frequency)) * maxDuty;
        ledcWrite(channel, duty);
    }
}

void ServoControl::detach() {
    if (channel >= 0) {
        ledcDetachPin(pin);
        channel = -1;
    }
}

void ServoControl::setPulseWidthRange(int minUs, int maxUs) {
    minPulseWidth = minUs;
    maxPulseWidth = maxUs;
}

void ServoControl::setAngleRange(int minDeg, int maxDeg) {
    minAngle = minDeg;
    maxAngle = maxDeg;
}

