#pragma once
#include <vector>
#include "Signal.h"

class Storage {
private:
    std::vector<Signal*> memory;

public:
    void saveSignal(Signal* sig);
    bool deleteSignal(int index);
    Signal* getSignal(int index);
    
    // permanent storage
    bool saveAllToFlash(); 
    bool loadAllFromFlash();
};

// accessible globally
extern Storage storage;