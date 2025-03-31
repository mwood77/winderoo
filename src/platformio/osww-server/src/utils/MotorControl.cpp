#include "MotorControl.h"

#if PWM_MOTOR_CONTROL
	#include <ESP32MX1508.h>
	#define CH1 1
	#define CH2 2
    int motorSpeed = 145;
#endif

#if STEPPER_MOTOR_CONTROL
    #include <AccelStepper.h>

    MotorControl::MotorControl(int pinA, int pinB, int pinC, int pinD, bool pwmMotorControl)
    {
        _pinA = pinA;
        _pinB = pinB;
        _pinC = pinC;
        _pinD = pinD;
        _motorDirection = 0;
        _pwmMotorControl = pwmMotorControl;
        stepper = AccelStepper(AccelStepper::HALF4WIRE, pinA, pinC, pinB, pinD);
    }
#else
    MotorControl::MotorControl(int pinA, int pinB, bool pwmMotorControl)
    {
        _pinA = pinA;
        _pinB = pinB;
        _motorDirection = 0;
        _pwmMotorControl = pwmMotorControl;
    }
#endif
    

void MotorControl::clockwise()
{
    #if PWM_MOTOR_CONTROL
        MX1508 pwmControl(_pinA, _pinB, CH1, CH2);
        pwmControl.motorGo(motorSpeed);
    #elif STEPPER_MOTOR_CONTROL
        // set the stepper motor to turn clockwise
        stepper.stop();
        stepper.setCurrentPosition(0);
        stepper.moveTo(stepper.currentPosition());
        stepper.runSpeed();
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
    #elif STEPPER_MOTOR_CONTROL
        // set the stepper motor to turn counter clockwise
        stepper.stop();
        stepper.setCurrentPosition(0);
        stepper.moveTo(-stepper.currentPosition());
        stepper.runSpeed();
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
    #elif STEPPER_MOTOR_CONTROL
        // stop the stepper motor
        stepper.stop();
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
