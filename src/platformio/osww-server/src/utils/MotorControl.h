#include <Arduino.h>

#ifndef MotorControl_H
#define MotorControl_H

class MotorControl
{
private:
    int _pinA;
    int _pinB;
    // 1 = clockwise, 0 = counter clockwise
    int _motorDirection;
    bool _pwmMotorControl;

    void stepperSetup();

    static void IRAM_ATTR onTimerStatic(); // Add this line

    void onTimer();

    void configureHardwareTimer();

    static MotorControl* instance; // Add this line

public:
    MotorControl(int _pinA, int _pinB, bool pwmMotorControl = false);

    void clockwise();

    void countClockwise();

    void stop();

    void determineMotorDirectionAndBegin();

    int getMotorDirection();

    void setMotorDirection(int direction);
};

#endif