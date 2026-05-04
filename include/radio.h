#pragma once
#include <Arduino.h>
#include <vector>
#include "config.h"

using namespace std;

class radioSignal {
    private:
        vector<uint8_t> data;
        int baudRate = 0;
    public:
        vector<uint8_t> getData() const { return data; }
        void setData(vector<uint8_t> givenData) { data = givenData; }
        void setBaudRate(int rate) { baudRate = rate; }
        int getBaudRate() { return baudRate; }
};

extern radioSignal signals[RADIO_SLOTS];

int radioSetup(double baud, int length);
int readRadio(int currentSlot);
void writeRadio(radioSignal signal);
