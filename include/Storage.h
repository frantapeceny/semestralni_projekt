#pragma once
#include <vector>
#include "Signal_.h"

class Storage {
private:
    std::vector<Signal*> memory;

public:
    ~Storage() {
        for (Signal* s : memory) {
            delete s;
        }
    }
    int getCount() { return memory.size(); }
    void saveSignal(Signal* sig);
    bool deleteSignal(int index);
    Signal* getSignal(int index);
    
    // permanent storage
    bool saveAllToFlash(); 
    bool loadAllFromFlash();
};

// accessible globally
extern Storage storage;