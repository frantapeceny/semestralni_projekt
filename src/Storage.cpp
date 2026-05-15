#include "Storage.h"
#include "NfcSignal.h"
#include "RadioSignal.h"
#include <Preferences.h>

Preferences prefs;

void Storage::saveSignal(Signal* sig) {
    memory.push_back(sig);
    saveAllToFlash();
}

bool Storage::deleteSignal(int index) {
    if (index < 0 || index >= memory.size()) {
        Serial.println("ERROR: Can't delete, index out of bounds.");
        return false;
    }

    // free
    delete memory[index];

    // delete from vector
    memory.erase(memory.begin() + index);

    Serial.printf("Signal at index %d deleted successfully.\n", index);

    saveAllToFlash();
    return true;
}

Signal* Storage::getSignal(int index) {
    if (index >= 0 && index < memory.size()) {
        return memory[index];
    }
    return nullptr; 
}

bool Storage::saveAllToFlash() {
    Serial.println("Saving all signals with new indices to flash memory...");

    // save the new signals count
    prefs.begin("system", false);
    prefs.putInt("total_signals", memory.size());
    prefs.end();

    // clear permanent storage
    prefs.begin("nfc", false); prefs.clear(); prefs.end();
    prefs.begin("radio", false); prefs.clear(); prefs.end();

    // resave everything into it
    for (size_t i = 0; i < memory.size(); i++) {
        if (memory[i] != nullptr) {
            memory[i]->saveToFlash(i); 
        }
    }

    Serial.println("All signals saved successfully.");
    return true;
}

bool Storage::loadAllFromFlash() {
    // find out how many signals to load
    prefs.begin("system", true);
    int totalSignalsToLoad = prefs.getInt("total_signals", 0);
    prefs.end();
    
    Serial.printf("Booting up... Found %d signals to load.\n", totalSignalsToLoad);

    for (int i = 0; i < totalSignalsToLoad; i++) {
        // check if slot is for nfc
        prefs.begin("nfc", true);
        String uidKey = "u_" + String(i);
        size_t uidLen = prefs.getBytesLength(uidKey.c_str());
        
        if (uidLen > 0) {
            uint8_t* uidBuf = new uint8_t[uidLen];
            prefs.getBytes(uidKey.c_str(), uidBuf, uidLen);
            std::vector<uint8_t> vecUID(uidBuf, uidBuf + uidLen);
            delete[] uidBuf;

            // Rebuild the NFC object and add it to RAM
            NfcSignal* nfcSig = new NfcSignal();
            nfcSig->setUID(vecUID);
            memory.push_back(nfcSig);
            
            Serial.printf("Loaded slot %d as NFC.\n", i);
            prefs.end();
            continue;
        }
        prefs.end();

        // check if slot is for radio
        prefs.begin("radio", true);
        String dataKey = "d_" + String(i);
        size_t dataLen = prefs.getBytesLength(dataKey.c_str());
        
        if (dataLen > 0) {
            String baudKey = "b_" + String(i);
            int baud = prefs.getInt(baudKey.c_str(), 0);

            uint8_t* dataBuf = new uint8_t[dataLen];
            prefs.getBytes(dataKey.c_str(), dataBuf, dataLen);
            std::vector<uint8_t> vecData(dataBuf, dataBuf + dataLen);
            delete[] dataBuf;

            RadioSignal* radioSig = new RadioSignal();
            radioSig->setData(vecData);
            radioSig->setBaudRate(baud);
            memory.push_back(radioSig);
            
            Serial.printf("Loaded slot %d as Radio.\n", i);
        }
        prefs.end();
    }

    Serial.println("All signals successfully loaded into RAM!");
    return true;
}
