#include <Arduino.h>
#include <AccelStepper.h>

#ifndef MotorControl_H
#define MotorControl_H

class MotorControl
{
private:
    int _pinA;
    int _pinB;
    
    #if STEPPER_MOTOR_CONTROL
        int _pinC;
        int _pinD;
        AccelStepper stepper;
    #endif
    
    // 1 = clockwise, 0 = counter clockwise
    int _motorDirection;
    bool _pwmMotorControl;

public:
#if STEPPER_MOTOR_CONTROL
    MotorControl(int _pinA, int _pinB, int _pinC, int _pinD, bool pwmMotorControl = false);
    
#else
    MotorControl(int _pinA, int _pinB, bool pwmMotorControl = false);
#endif
    void clockwise();

    void countClockwise();

    void stop();

    void determineMotorDirectionAndBegin();

    int getMotorDirection();

    void setMotorDirection(int direction);
};

#endif