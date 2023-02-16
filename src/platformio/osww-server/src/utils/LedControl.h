#include <Arduino.h>

#ifndef LedControl_H
#define LedControl_H

class LedControl
{
private:
    int _ledChannel;
    int _freq;
    int _resolution;

public:
    LedControl(int _ledChannel);

    void pwm()
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

    void slowBlink()
    {
        // Slow blink to confirm success & restart
        Serial.println("slow blink");

        for (int dutyCycle = 0; dutyCycle <= 4; dutyCycle++)
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

    void fastBlink()
    {
        // Fast blink to confirm resetting
        Serial.println("slow blink");
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

    void off()
    {
        ledcWrite(_ledChannel, 0);
    }

    int getChannel()
    {
        return _ledChannel;
    }

    int getFrequency()
    {
        return _freq;
    }

    int getResolution()
    {
        return _resolution;
    }
};

#endif // LEDControl_H