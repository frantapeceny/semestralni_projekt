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

extern nfcSignal nfcs[NFC_SLOTS];

void setupNFC();
void handleNFC();
