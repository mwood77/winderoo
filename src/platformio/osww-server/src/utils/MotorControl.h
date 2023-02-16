#include <Arduino.h>

#ifndef MotorControl_H
#define MotorControl_H

class MotorControl
{
private:
    int _pinA;
    int _pinB;
    // 1 = clockwise, 0 = anticlockwise
    int _motorDirection;

public:
    MotorControl(int _pinA, int _pinB);

    void clockwise()
    {
        digitalWrite(_pinA, HIGH);
        digitalWrite(_pinB, LOW);
    }

    void countClockwise()
    {
        digitalWrite(_pinA, LOW);
        digitalWrite(_pinB, HIGH);
    }

    void stop()
    {
        digitalWrite(_pinA, LOW);
        digitalWrite(_pinB, LOW);
    }

    void determineMotorDirectionAndBegin()
    {
        stop();

        if (_motorDirection)
        {
            clockwise();
        }
        else
        {
            countClockwise();
        }
    }

    int getMotorDirection()
    {
        return _motorDirection;
    }

    void setMotorDirection(int direction)
    {
        _motorDirection = direction;
    }
};

#endif