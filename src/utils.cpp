#include "config.h"
#include <Arduino.h>

bool isPressed(int button_pin) {
    return digitalRead(button_pin) == LOW;
}

bool isRadioSlotValid(int slot) {
    if (slot < 0 || slot >= RADIO_SLOTS) {
        Serial.printf("Invalid radio slot (%d), max is %d.\n", slot, RADIO_SLOTS);
        return false;
    }
    return true;
}

bool isNFCSlotValid(int slot) {
    if (slot < 0 || slot >= NFC_SLOTS) {
        Serial.printf("Invalid NFC slot (%d), max is %d.\n", slot, NFC_SLOTS);
        return false;
    }
    return true;
}

