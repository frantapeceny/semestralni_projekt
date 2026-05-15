#include "RadioSignal.h"
#include "RadioManager.h"
#include <Preferences.h>

void RadioSignal::transmit() const {
    radioManager.transmit(*this); 
}

void RadioSignal::showInfo() const {
    Serial.print("Sub-GHz Baud Rate: ");
    Serial.print(baudRate);
    Serial.print(" | Data Length: ");
    Serial.println(data.size());
}

void RadioSignal::saveToFlash(int slot) const {
    Preferences prefs;
    prefs.begin("radio", false); // false = read/write mode
    
    String baudKey = "b_" + String(slot);
    String dataKey = "d_" + String(slot);
    
    prefs.putInt(baudKey.c_str(), (int)baudRate); // TODO: change to float?
    prefs.putBytes(dataKey.c_str(), data.data(), data.size());

    prefs.end();
    Serial.printf("Saved Radio signal to slot %d\n", slot);
}