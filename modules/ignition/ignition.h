#ifndef IGNITION_H
#define IGNITION_H

#include "mbed.h"

typedef enum {
    BUTTON_UP,
    BUTTON_DOWN,
    BUTTON_FALLING,
    BUTTON_RISING
} buttonState_t;

void ignitionStateUpdate();
void ignitionSwitchInit();
bool ignitionSwitchUpdate();

#endif 