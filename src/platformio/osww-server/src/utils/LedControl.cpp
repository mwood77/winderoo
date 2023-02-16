#include "LedControl.h"

LedControl::LedControl(int ledChannel) {
    _ledChannel = ledChannel;
     _freq = 5000;
     _resolution = 8;
}