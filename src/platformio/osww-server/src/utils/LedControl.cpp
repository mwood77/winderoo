#include "LedControl.h"

LedControl::LedControl(int ledChannel)
{
    _ledChannel = ledChannel;
    _freq = 5000;
    _resolution = 8;
}

void LedControl::pwm()
{
    // pulse LED to show in sleep state
    for (int dutyCycle = 0; dutyCycle <= 255; dutyCycle++)
    {
        ledcWrite(_ledChannel, dutyCycle);
        delay(7);
    }

    for (int dutyCycle = 255; dutyCycle >= 0; dutyCycle--)
    {
        ledcWrite(_ledChannel, dutyCycle);
        delay(7);
    }
}

void LedControl::slowBlink()
{
    // Slow blink to confirm success & restart
    Serial.println("[STATUS] - slow blink");

    for (int dutyCycle = 0; dutyCycle <= 3; dutyCycle++)
    {
        for (int dutyCycle = 0; dutyCycle <= 255; dutyCycle++)
        {
            ledcWrite(_ledChannel, dutyCycle);
            delay(7);
        }

        for (int dutyCycle = 255; dutyCycle >= 0; dutyCycle--)
        {
            ledcWrite(_ledChannel, dutyCycle);
            delay(7);
        }
        delay(150);
    }
}

void LedControl::fastBlink()
{
    // Fast blink to confirm resetting
    Serial.println("[STATUS] - fast blink");
    for (int i = 0; i < 12; i++)
    {

        for (int dutyCycle = 0; dutyCycle <= 255; dutyCycle++)
        {
            ledcWrite(_ledChannel, dutyCycle);
            delay(2);
        }

        for (int dutyCycle = 255; dutyCycle >= 0; dutyCycle--)
        {
            ledcWrite(_ledChannel, dutyCycle);
            delay(2);
        }

        delay(50);
    }
}

void LedControl::off()
{
    ledcWrite(_ledChannel, 0);
}

int LedControl::getChannel()
{
    return _ledChannel;
}

int LedControl::getFrequency()
{
    return _freq;
}

int LedControl::getResolution()
{
    return _resolution;
}
