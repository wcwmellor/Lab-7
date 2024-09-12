// ignition.cpp
#include "ignition.h"
#include "mbed.h"  

// DigitalIn objects for input pins
DigitalIn driverSeat(D11);
DigitalIn ignitionSwitch(D12);

// DigitalOut objects for output pins
DigitalOut engineOnLED(LED2);

#define DEBOUNCE_BUTTON_TIME_MS 40
#define TIME_INCREMENT_MS  10

bool engineRunning = false;
buttonState_t ignitionSwitchState;
int accumulatedDebounceButtonTime = 0;

void inputsInit() {
    driverSeat.mode(PullDown);
    ignitionSwitch.mode(PullUp);
}

void outputsInit() {
    engineOnLED = false;
}

void ignitionStateUpdate() {
    bool ignitionSwitchReleasedEvent = ignitionSwitchUpdate();

    // Start the engine if the driver's seat is occupied and the ignition button is released
    if (driverSeat && ignitionSwitch && !engineRunning) {
        engineOnLED = true;
        engineRunning = true;
    }

    // Keep the engine running even if the driver exits the vehicle
    if (engineRunning && !driverSeat) {
        // Keep the engine running
    }

    // Stop the engine when the ignition button is released
    if (engineRunning && ignitionSwitch) {
        engineOnLED = false;
        engineRunning = false;
    }
}

void ignitionSwitchInit() {
    if (ignitionSwitch == 1) {
        ignitionSwitchState = BUTTON_UP;
    } else {
        ignitionSwitchState = BUTTON_DOWN;
    }
}

bool ignitionSwitchUpdate() {
    bool ignitionSwitchReleasedEvent = false;

    switch (ignitionSwitchState) {
        case BUTTON_UP:
            if (ignitionSwitch == 0) {
                ignitionSwitchState = BUTTON_FALLING;
                accumulatedDebounceButtonTime = 0;
            }
            break;

        case BUTTON_FALLING:
            if (accumulatedDebounceButtonTime >= DEBOUNCE_BUTTON_TIME_MS) {
                if (ignitionSwitch == 0) {
                    ignitionSwitchState = BUTTON_DOWN;
                } else {
                    ignitionSwitchState = BUTTON_UP;
                }
            }
            accumulatedDebounceButtonTime += TIME_INCREMENT_MS;
            break;

        case BUTTON_DOWN:
            if (ignitionSwitch == 1) {
                ignitionSwitchState = BUTTON_RISING;
                accumulatedDebounceButtonTime = 0;
                ignitionSwitchReleasedEvent = true;
            }
            break;

        case BUTTON_RISING:
            if (accumulatedDebounceButtonTime >= DEBOUNCE_BUTTON_TIME_MS) {
                if (ignitionSwitch == 1) {
                    ignitionSwitchState = BUTTON_UP;
                } else {
                    ignitionSwitchState = BUTTON_DOWN;
                }
            }
            accumulatedDebounceButtonTime += TIME_INCREMENT_MS;
            break;

        default:
            ignitionSwitchInit();
            break;
    }

    return ignitionSwitchReleasedEvent;
}
