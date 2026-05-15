#include "NfcSignal.h"
#include "NfcManager.h"
#include <Preferences.h>

void NfcSignal::transmit() const {
    nfcManager.write(*this);
}

void NfcSignal::showInfo() const {
    Serial.print("NFC UID Length: ");
    Serial.println(uid.size());
}

void NfcSignal::saveToFlash(int slot) const {
    Preferences prefs;
    prefs.begin("nfc", false); // false = read/write
    
    String uidKey = "u_" + String(slot);
    
    // save own UID
    prefs.putBytes(uidKey.c_str(), uid.data(), uid.size());

    prefs.end();
    Serial.printf("Saved NFC to slot %d\n", slot);
}