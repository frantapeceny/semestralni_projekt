#include "Config.h"
#include <Arduino.h>

bool isPressed(int button_pin) {
    return digitalRead(button_pin) == LOW;
}

bool isSlotValid(int slot) {
    if (slot < 0 || slot >= SIGNAL_SLOTS) {
        Serial.printf("Invalid signal slot (%d), max is %d.\n", slot, SIGNAL_SLOTS);
        return false;
    }
    return true;
}
