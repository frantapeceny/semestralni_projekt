#pragma once
#include <Arduino.h>
#include "Config.h"
// #include "radio.h"
#include "NfcSignal.h"

void loadAllSlotsFromMemory();
// void saveDataIntoRadioSlotRAM(int slot, radioSignal data);
void saveDataIntoNFCSlotRAM(int slot, NfcSignal data);
void saveRadioSlotPermanently(int slot);
void saveNFCSlotPermanently(int slot);
