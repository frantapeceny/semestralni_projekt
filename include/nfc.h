#pragma once
#include <Arduino.h>
#include <vector>
#include "config.h"

using namespace std;

class nfcSignal{
    private:
        vector <uint8_t> uid;
    public:
        vector <uint8_t> getUID() const { return uid; }
        void setUID( vector <uint8_t> givenUID) { uid = givenUID; }
    protected:
};

extern nfcSignal nfcSignals[NFC_SLOTS];

int setupNFC();
nfcSignal readNFC();
int saveNFC(int slot, nfcSignal signal);
void writeNFC(nfcSignal signal);
