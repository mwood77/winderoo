#include "MotorControl.h"

MotorControl::MotorControl(int pinA, int pinB) {
    _pinA = pinA;
    _pinB = pinB;
    _motorDirection = 0;
}