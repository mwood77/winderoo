#include "MotorControl.h"

#if PWM_MOTOR_CONTROL
	#include <ESP32MX1508.h>
	#define CH1 1
	#define CH2 2
    int motorSpeed = 145;
#endif

MotorControl::MotorControl(int pinA, int pinB, bool pwmMotorControl)
{
    _pinA = pinA;
    _pinB = pinB;
    _motorDirection = 0;
    _pwmMotorControl = pwmMotorControl;
}

void MotorControl::clockwise()
{
    #if PWM_MOTOR_CONTROL
        MX1508 pwmControl(_pinA, _pinB, CH1, CH2);
        pwmControl.motorGo(motorSpeed);
    #else
        digitalWrite(_pinA, HIGH);
        digitalWrite(_pinB, LOW);
    #endif
    Serial.println("[STATUS] - Motor turning clockwise");
}

void MotorControl::countClockwise()
{
    #if PWM_MOTOR_CONTROL
        MX1508 pwmControl(_pinA, _pinB, CH1, CH2);
        pwmControl.motorRev(motorSpeed);
    #else
        digitalWrite(_pinA, LOW);
        digitalWrite(_pinB, HIGH);
    #endif
    Serial.println("[STATUS] - Motor turning counter clockwise");
}

void MotorControl::stop()
{
    #if PWM_MOTOR_CONTROL
        MX1508 pwmControl(_pinA, _pinB, CH1, CH2);
        pwmControl.motorBrake();
    #else
        digitalWrite(_pinA, LOW);
        digitalWrite(_pinB, LOW);
    #endif
    Serial.println("[STATUS] - Motor stopped");
}

void MotorControl::determineMotorDirectionAndBegin()
{
    // @todo - investigate if this is still needed
    // stop();

    if (_motorDirection)
    {
        clockwise();
    }
    else
    {
        countClockwise();
    }
}

int MotorControl::getMotorDirection()
{
    return _motorDirection;
}

void MotorControl::setMotorDirection(int direction)
{
    _motorDirection = direction;
}
