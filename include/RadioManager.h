#pragma once
#include <Arduino.h>
#include <cc1101.h>
#include <vector>
#include "RadioSignal.h"
#include "Config.h"

class RadioManager {
private:
    CC1101::Radio radio;

    int setupRadio(double baud, int length);
    int countDataBytes(uint8_t *buf, size_t len);
    int countRepeats(uint8_t *buf, size_t len, int seqLen);
    float scoreCaptures(uint8_t captures[][64], int numCaptures);
    std::vector<uint8_t> extractCycle(std::vector<uint8_t>& data);
    double baudRateFinder();

public:
    // constructor
    RadioManager(uint8_t cs, uint8_t gdo0, uint8_t rst, uint8_t gdo2);

    RadioSignal capture();
    void transmit(const RadioSignal& signal);
};

extern RadioManager radioManager;