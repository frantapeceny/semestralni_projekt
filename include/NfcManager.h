#pragma once
#include <Arduino.h>
#include <Adafruit_PN532.h>
#include "NfcSignal.h"

class NfcManager {
private:
    Adafruit_PN532 nfc;
public:
    NfcManager(uint8_t csPin);

    bool begin();
    NfcSignal read();
    void write(const NfcSignal& signal);
};

extern NfcManager nfcManager;
