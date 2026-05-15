#pragma once
#include "Config.h"
#include "Signal.h"
#include <Arduino.h>
#include <vector>

class RadioSignal : public Signal {
    private:
        std::vector<uint8_t> data;
        double baudRate = 0;
    public:
        std::vector<uint8_t> getData() const { return data; }
        void setData(std::vector<uint8_t> givenData) { data = givenData; }
        
        void setBaudRate(double rate) { baudRate = rate; }
        double getBaudRate() const { return baudRate; }

        void transmit() const override;
        void showInfo() const override;
        void saveToFlash(int slot) const override;
};