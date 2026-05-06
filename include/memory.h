#pragma once
#include <Arduino.h>
#include "config.h"
#include "radio.h"
#include "nfc.h"

void loadAllSlotsFromMemory();
void saveDataIntoRadioSlotRAM(int slot, radioSignal data);
void saveDataIntoNFCSlotRAM(int slot, nfcSignal data);
void saveRadioSlotPermanently(int slot);
void saveNFCSlotPermanently(int slot);
