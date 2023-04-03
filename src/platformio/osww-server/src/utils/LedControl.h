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

    void pwm();

    void slowBlink();

    void fastBlink();

    void off();

    int getChannel();

    int getFrequency();

    int getResolution();
};

#endif