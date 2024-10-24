#include "MotorControl.h"

#if PWM_MOTOR_CONTROL
	#include <ESP32MX1508.h>
	#define CH1 1
	#define CH2 2
    int motorSpeed = 145;
#endif

#if STEPPER_MOTOR_CONTROL
    #include <TMC2209.h>
    
    HardwareSerial & serial_stream = Serial2;

    const uint8_t STEP_PIN = 17;
    const uint8_t DIRECTION_PIN = 16;
    
    const int32_t RUN_VELOCITY = 20000;
    const int32_t STOP_VELOCITY = 0;
    const uint16_t HALF_STEP_DURATION_MICROSECONDS = 10;

    // current values may need to be reduced to prevent overheating depending on
    // specific motor and power supply voltage
    const uint8_t RUN_CURRENT_PERCENT = 100;

    // Instantiate TMC2209
    TMC2209 stepper_driver;

#endif

MotorControl::MotorControl(int pinA, int pinB, bool pwmMotorControl)
{
    #if STEPPER_MOTOR_CONTROL

        stepper_driver.setup(serial_stream);

        pinMode(STEP_PIN, OUTPUT);
        pinMode(DIRECTION_PIN, OUTPUT);

        stepper_driver.setRunCurrent(RUN_CURRENT_PERCENT);
        stepper_driver.enableCoolStep();
        stepper_driver.enable();
        
        _motorDirection = 0;
    #else
        _pinA = pinA;
        _pinB = pinB;
        _motorDirection = 0;
        _pwmMotorControl = pwmMotorControl;
    #endif
}

void MotorControl::clockwise()
{
    #if PWM_MOTOR_CONTROL
        MX1508 pwmControl(_pinA, _pinB, CH1, CH2);
        pwmControl.motorGo(motorSpeed);
    #elif STEPPER_MOTOR_CONTROL
        // stepper_driver.moveAtVelocity(RUN_VELOCITY);
        stepper_driver.moveUsingStepDirInterface();
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
        // stepper_driver.moveAtVelocity(RUN_VELOCITY);
        stepper_driver.moveUsingStepDirInterface();
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
        stepper_driver.moveAtVelocity(STOP_VELOCITY);
        stepper_driver.moveUsingStepDirInterface();
    #else
        digitalWrite(_pinA, LOW);
        digitalWrite(_pinB, LOW);
    #endif
    Serial.println("[STATUS] - Motor stopped");
}

void MotorControl::determineMotorDirectionAndBegin()
{
    if (_motorDirection)
    {
        #if STEPPER_MOTOR_CONTROL
            stepper_driver.disableInverseMotorDirection();
        #else
            clockwise();
        #endif
    }
    else
    {
        #if STEPPER_MOTOR_CONTROL
            stepper_driver.enableInverseMotorDirection();
        #else
            countClockwise();
        #endif
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
