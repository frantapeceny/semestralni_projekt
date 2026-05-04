#pragma once
#include <Arduino.h>
#include <vector>
#include "config.h"

using namespace std;

class nfcSignal{
    private:
        vector <uint8_t> data;
    public:
        vector <uint8_t> getData() const { return data; }
        void setData( vector <uint8_t> givenData) { data = givenData; }
    protected:
};

extern nfcSignal nfcSignals[NFC_SLOTS];

int setupNFC();
nfcSignal readNFC();
int saveNFC(int slot, nfcSignal signal);
void writeNFC(nfcSignal signal);
