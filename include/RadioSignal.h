#pragma once
#include "Config.h"
#include "Signal_.h"
#include <Arduino.h>
#include <vector>

class RadioSignal : public Signal {
    private:
        std::vector<uint8_t> data;
        float baudRate = 0;
    public:
        std::vector<uint8_t> getData() const { return data; }
        void setData(std::vector<uint8_t> givenData) { data = givenData; }
        
        void setBaudRate(float rate) { baudRate = rate; }
        double getBaudRate() const { return baudRate; }

        void transmit() const override;
        void showInfo() const override;
        void saveToFlash(int slot) const override;
        String typeName() const override;
        String shortInfo() const override;
};