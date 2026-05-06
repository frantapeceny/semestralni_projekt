#include "memory.h"
#include "config.h"
#include "utils.h"
#include <Preferences.h>

// esp32 memory library
Preferences prefs;

void loadRadioSlotsFromMemory() {
    Serial.println("Loading saved radio slots from memory...");

    prefs.begin("radio", true);  // true = readonly
    for (int i = 0; i < RADIO_SLOTS; i++) {
        String baudKey = "b_" + String(i);
        String dataKey = "d_" + String(i);

        // check if there is data
        size_t dataLen = prefs.getBytesLength(dataKey.c_str());
        if (dataLen > 0) {
            int baud = prefs.getInt(baudKey.c_str(), 0);
            
            // read data into buffer
            uint8_t* buffer = new uint8_t[dataLen];
            prefs.getBytes(dataKey.c_str(), buffer, dataLen);

            // convert to vector and free
            vector<uint8_t> vecData(buffer, buffer + dataLen);
            delete[] buffer;

            // add into array from radio.cpp
            radioSignal sig;
            sig.setBaudRate(baud);
            sig.setData(vecData);
            radioSignals[i] = sig;
            
            Serial.printf("Loaded radio slot %d: %d bytes (%d baud)\n", i, dataLen, baud);
        }
    }
    prefs.end();
}

void loadNFCSlotsFromMemory() {
    Serial.println("Loading saved NFC slots from memory...");

    prefs.begin("nfc", true);
    for (int i = 0; i < NFC_SLOTS; i++) {
        // String typeKey = "t_" + String(i);
        String uidKey = "u_" + String(i);
        
        // check if there is data
        size_t uidLen = prefs.getBytesLength(uidKey.c_str());
        if (uidLen > 0) {
            // TODO:
            // int type = prefs.getInt(typeKey.c_str(), 0);

            // read UID into buffer
            uint8_t* uidBuf = new uint8_t[uidLen];
            prefs.getBytes(uidKey.c_str(), uidBuf, uidLen);

            // convert to vector and free
            vector<uint8_t> vecUID(uidBuf, uidBuf + uidLen);
            delete[] uidBuf;

            // add into array from nfc.cpp
            nfcSignal nfcSig;
            nfcSig.setUID(vecUID);
            nfcSignals[i] = nfcSig;
            
            Serial.printf("Loaded NFC slot %d: UID len %d\n", i, uidLen);
        }
    }
    prefs.end();
}

void loadAllSlotsFromMemory() {
    loadRadioSlotsFromMemory();
    loadNFCSlotsFromMemory();
    Serial.println("Memory loading complete!");
}

// automatically save radio signal currently at the specified slot into persistent memory 
void saveRadioSlotPermanently(int slot) {
    if (!isRadioSlotValid(slot)) {
        return;
    }

    prefs.begin("radio", false);  // false = read + write mode
    
    String baudKey = "b_" + String(slot);
    String dataKey = "d_" + String(slot);
    
    radioSignal sig = radioSignals[slot];
    
    // save baud rate
    prefs.putInt(baudKey.c_str(), sig.getBaudRate());
    
    // save data sequence
    vector<uint8_t> data = sig.getData();
    prefs.putBytes(dataKey.c_str(), data.data(), data.size());
    
    prefs.end();
    Serial.printf("Saved radio slot %d to persistent memory.\n", slot);
}

void saveNFCSlotPermanently(int slot) {
    if (!isNFCSlotValid(slot)) {
        return;
    }

    prefs.begin("nfc", false);
    
    // String typeKey = "t_" + String(slot);
    String uidKey = "u_" + String(slot);
    
    nfcSignal sig = nfcSignals[slot];
    
    // prefs.putInt(typeKey.c_str(), sig.getCardType());
    
    vector<uint8_t> uid = sig.getUID();
    prefs.putBytes(uidKey.c_str(), uid.data(), uid.size());

    prefs.end();
    Serial.printf("Saved NFC Slot %d to persistent memory.\n", slot);
}

void saveDataIntoRadioSlotRAM(int slot, radioSignal data) {
    if (!isRadioSlotValid(slot)) {
        return;
    }
    
    radioSignals[slot] = data;
}

void saveDataIntoNFCSlotRAM(int slot, nfcSignal data) {
    if (!isNFCSlotValid(slot)) {
        return;
    }

    nfcSignals[slot] = data;
}
