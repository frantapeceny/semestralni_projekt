#include "NfcManager.h"
#include <SPI.h>

using namespace std;

NfcManager::NfcManager(uint8_t csPin) : nfc(csPin) {}

bool NfcManager::begin() {
    SPI.begin(4, 5, 6, 7);
    nfc.begin();

    uint32_t versiondata = nfc.getFirmwareVersion();
    if (!versiondata) {
        Serial.println("ERROR: Couldn't find PN53 module.");
        return false;
    }

    nfc.SAMConfig();
    Serial.println("NFC Manager initialized.");
    return true;
}

NfcSignal NfcManager::read() {
    NfcSignal capturedSignal;
    uint8_t uidBuffer[7];
    uint8_t uidLength;        
    
    Serial.println("Waiting for an NFC signal...");
    
    bool success = false;
    unsigned long start = millis();
    while (millis() - start < NFC_READ_TIMEOUT_MS) {
        success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uidBuffer, &uidLength, 100);
        if (success) {
            break;
        }
    }

    if (success) {
        vector<uint8_t> cardUID(uidBuffer, uidBuffer + uidLength);
        capturedSignal.setUID(cardUID);
    } else {
        Serial.println("Read timed out. No NFC detected.");
    }
    
    return capturedSignal;
}

void NfcManager::write(const NfcSignal& signal) {
    vector<uint8_t> uid = signal.getUID();
    
    if (uid.size() != 4) {
        Serial.println("ERROR: Only 4-byte UIDs allowed.");
        return;
    }
    
    Serial.println("Tap NFC chip to write on...");
    uint8_t targetUidBuffer[4];
    uint8_t targetUidLength;
    
    if (!nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, targetUidBuffer, &targetUidLength, 5000)) {
        Serial.println("ERROR: No target NFC chip detected.");
        return;
    }
    
    Serial.println("Target NFC chip detected. Starting UID clone...");
    
    uint8_t targetKeyA[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    
    // authenticate to Sector 0 (Block 0)
    if (!nfc.mifareclassic_AuthenticateBlock(targetUidBuffer, targetUidLength, 0, 0, targetKeyA)) {
        Serial.println("ERROR: AUTH FAIL on chip at Sector 0.");
        return;
    }

    // create new Block 0
    uint8_t newBlock0[16] = {0};
    
    // copy 4-byte UID
    newBlock0[0] = uid[0];
    newBlock0[1] = uid[1];
    newBlock0[2] = uid[2];
    newBlock0[3] = uid[3];
    
    // calculate BCC (XOR sum of 4 UID bytes)
    newBlock0[4] = uid[0] ^ uid[1] ^ uid[2] ^ uid[3];
    
    // set Mifare Classic 1K ATQA and SAK values
    newBlock0[5] = 0x08; // SAK
    newBlock0[6] = 0x04; // ATQA 0
    newBlock0[7] = 0x00; // ATQA 1
    
    // fill with padding
    for (int i = 8; i < 16; i++) {
        newBlock0[i] = 0x62;
    }

    // write Block 0
    if (!nfc.mifareclassic_WriteDataBlock(0, newBlock0)) {
        Serial.println("ERROR: WRITE FAIL at Block 0.");
        return;
    }
    
    Serial.println("SUCCESS. UID cloned to NFC chip.");
}
