#pragma once
#include <Arduino.h>
#include <cc1101.h>
#include <vector>
#include "RadioSignal.h"
#include "Config.h"

class RadioManager {
private:
    CC1101::Radio radio;

    int setupRadio(float baud, int length);
    int countDataBytes(uint8_t *buf, size_t len);
    int countRepeats(uint8_t *buf, size_t len, int seqLen);
    float scoreCaptures(uint8_t captures[][64], int numCaptures);
    std::vector<uint8_t> extractCycle(std::vector<uint8_t>& data);
    float baudRateFinder();

public:
    RadioManager(uint8_t cs);

    RadioSignal capture();
    void transmit(const RadioSignal& signal);
};

extern RadioManager radioManager;