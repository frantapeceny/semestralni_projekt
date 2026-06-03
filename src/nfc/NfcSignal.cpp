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

String NfcSignal::typeName() const { return "NFC"; }
//String NfcSignal::shortInfo() const { return "UID: " + String(uid.size()) + " bytes"; }
String NfcSignal::shortInfo() const {
    String result = "";
    for (int i = 0; i < 4 && i < (int)uid.size(); i++) {
        if (uid[i] < 0x10) result += "0";
        result += String(uid[i], HEX);
        if (i < 3) result += ":";
    }
    if (uid.size() > 4) result += "...";
    result.toUpperCase();
    return "UID: " + result;
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