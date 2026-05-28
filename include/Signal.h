#pragma once
#include <Arduino.h>
#include <vector>

class Signal {
    public:
        virtual ~Signal() = default;

        virtual void transmit() const = 0;
        virtual void showInfo() const = 0;
        virtual void saveToFlash(int slot) const = 0;

        virtual String typeName() const = 0;
        virtual String shortInfo() const = 0;
};
