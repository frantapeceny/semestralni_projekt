#pragma once
#include "Config.h"
#include "Signal.h"
#include <Arduino.h>
#include <vector>

class NfcSignal : public Signal {
    private:
        std::vector<uint8_t> uid;
    public:
        std::vector<uint8_t> getUID() const { return uid; }
        void setUID(std::vector<uint8_t> givenUID) { uid = givenUID; }
        
        void transmit() const override;
        void showInfo() const override;
        void saveToFlash(int slot) const override;
};
