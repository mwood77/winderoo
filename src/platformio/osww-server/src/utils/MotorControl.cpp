#include "MotorControl.h"

// Define the static instance variable
MotorControl* MotorControl::instance = nullptr;

#if PWM_MOTOR_CONTROL
	#include <ESP32MX1508.h>
	#define CH1 1
	#define CH2 2
    int motorSpeed = 145;
#endif

#ifdef STEPPER_MOTOR_CONTROL
    #include <TMCStepper.h>
    #include <SoftwareSerial.h>

    const uint16_t EN_PIN   = 4; // Enable
    const uint16_t DIR_PIN  = 16; // Direction
    const uint16_t STEP_PIN = 17; // Step

    const signed long RUN_VELOCITY = 20000;
    const signed long STOP_VELOCITY = 0;

    #define SW_RX            3 // TMC2208/TMC2224 SoftwareSerial receive pin
    #define SW_TX            1 // TMC2208/TMC2224 SoftwareSerial transmit pin
    #define SERIAL_PORT Serial1 // TMC2208/TMC2224 HardwareSerial port
    #define DRIVER_ADDRESS 0b00 // TMC2209 Driver address according to MS1 and MS2

    const uint16_t R_SENSE = 0.11f; // TMC2209  - SilentStepStick series use 0.11

    SoftwareSerial softwareSerial(SW_RX, SW_TX); // Create SoftwareSerial object
    TMC2208Stepper driver = TMC2208Stepper(&softwareSerial, R_SENSE); // Use SoftwareSerial object

    // For non-blocking stepper movement
    hw_timer_t *timer = NULL;
    volatile bool stepPinState = LOW;
    static bool timerConfigured = false;

    // For stepper timing control (RPM)
    uint16_t microSteps = 4;
    uint16_t rmsCurrent = 600;
    uint64_t timerMicroSecondInterval = 400; // 937 microseconds for 40 RPM / 400 microseconds for 100 RPM

#endif

MotorControl::MotorControl(int pinA, int pinB, bool pwmMotorControl)
{
    #ifdef STEPPER_MOTOR_CONTROL
        instance = this;
        stepperSetup();

        _motorDirection = 0;
    #else
        _pinA = pinA;
        _pinB = pinB;
        _motorDirection = 0;
        _pwmMotorControl = pwmMotorControl;
    #endif
}

void MotorControl::stepperSetup() {

    softwareSerial.begin(9600);

    pinMode(EN_PIN, OUTPUT);
    pinMode(STEP_PIN, OUTPUT);
    pinMode(DIR_PIN, OUTPUT);
    digitalWrite(EN_PIN, LOW);

    driver.begin();
    driver.toff(5); // Enables driver in software
    driver.rms_current(rmsCurrent); // Set motor RMS current in mA
    driver.microsteps(microSteps); // Set microsteps to 1/8th because reasons
    driver.stealth(); // Enable stealthChop
}

// This private function is used to toggle the step pin by the timer
void IRAM_ATTR MotorControl::onTimer()
{
    digitalWrite(STEP_PIN, stepPinState);
    stepPinState = !stepPinState;
}

void IRAM_ATTR MotorControl::onTimerStatic()
{
    instance->onTimer();
}

void killTimer()
{
    Serial.println("[INFO] - Killing timer");

    timerAlarmDisable(timer);  // Disable the timer to stop
    timerDetachInterrupt(timer); // Detach the interrupt
    timerEnd(timer); // End the timer

    stepPinState = LOW;
    digitalWrite(STEP_PIN, stepPinState);

    timer = NULL;
    timerConfigured = false;
}

void MotorControl::configureHardwareTimer()
{
    if (!timerConfigured)
    {
        timer = timerBegin(0, 80, true);              // Timer 0, prescaler 80 (1 MHz clock)
        timerAttachInterrupt(timer, &onTimerStatic, true);  // Attach onTimer function to timer
        timerAlarmWrite(timer, timerMicroSecondInterval, true); // Set timer interval 937 microseconds for 40 RPM
        timerAlarmEnable(timer);                      // Enable the timer
        timerConfigured = true;
    }

}

void MotorControl::clockwise()
{
    #if PWM_MOTOR_CONTROL
        MX1508 pwmControl(_pinA, _pinB, CH1, CH2);
        pwmControl.motorGo(motorSpeed);
    #elif STEPPER_MOTOR_CONTROL
        // Set the direction pin
        if (timerConfigured) timerAlarmDisable(timer);

        digitalWrite(DIR_PIN, HIGH);
        delay(10);

        if (timerConfigured) timerAlarmEnable(timer);
        // Non-blocking step handling is now through the timer function; "configureHardwareTimer()"
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
        // Set the direction pin
       if (timerConfigured) timerAlarmDisable(timer);

        digitalWrite(DIR_PIN, LOW);
        delay(10);

       if (timerConfigured) timerAlarmEnable(timer);
        // Non-blocking step handling is now through the timer function; "configureHardwareTimer()"
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
        if (timerConfigured)
        {
            killTimer();
        }
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
            clockwise(); // We call this for logging purposes
            if (!timerConfigured) {
                configureHardwareTimer();
            }
        #else
            clockwise();
        #endif
    }
    else
    {
        #if STEPPER_MOTOR_CONTROL
            countClockwise();  // We call this for logging purposes
            if (!timerConfigured) {
                configureHardwareTimer();
            }
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

    Serial.print("[INFO] - MOTOR CONTROLLER - Setting direction to: ");
    Serial.println(_motorDirection ? "CW" : "CCW");
}
