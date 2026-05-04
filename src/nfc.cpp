#include "nfc.h"
#include <SPI.h>
#include <Adafruit_PN532.h>
#include <vector>

Adafruit_PN532 nfc(7);

nfcSignal nfcSignals[NFC_SLOTS];

int setupNFC() {
    SPI.begin(4, 5, 6, 7);
    nfc.begin();

    uint32_t versiondata = nfc.getFirmwareVersion();
    if (!versiondata) {
        Serial.println("ERROR: Couldn't find PN53x board.");
        return -1;
    }

    nfc.SAMConfig();
    Serial.println("NFC initialized.");
    return 0;
}

nfcSignal readNFC() {
    nfcSignal capturedSignal;
    uint8_t uidBuffer[7]; // 4 or 7 bytes
    uint8_t uidLength;        
    
    Serial.println("Waiting for an NFC signal...");
    
    // wait to get signal
    bool success = false;
    unsigned long start = millis();
    while (millis() - start < NFC_READ_TIMEOUT_MS) {
        success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uidBuffer, &uidLength, 100);
        if (success) {
            break;
        }
    }
    
    if (success) {
        Serial.println("NFC signal detected.");
        Serial.printf("UID length: ");
        Serial.print(uidLength, DEC);
        Serial.println(" bytes");
        Serial.print("UID Value: ");

        for (uint8_t i = 0; i < uidLength; i++) {
            Serial.print(" 0x"); Serial.print(uidBuffer[i], HEX);
        }

        Serial.println();

        // create vector from uid data
        vector<uint8_t> cardData(uidBuffer, uidBuffer + uidLength);
        
        // save into object
        capturedSignal.setData(cardData);
    } else {
        Serial.println("Read timed out. No NFC detected.");
    }
    
    // return captured signal (empty vector if timed out)
    return capturedSignal;
}

int saveNFC(int slot, nfcSignal signal) {
    if (slot < 0 || slot >= NFC_SLOTS) {
        Serial.println("ERROR: Invalid slot.");
        return -1;
    }
    
    if (signal.getData().size() == 0) {
        Serial.println("ERROR: Can't save empty NFC data.");
        return -1;
    }
    
    nfcSignals[slot] = signal;
    // TODO: add saving to flash memory
    Serial.print("SUCCESS: NFC data saved to slot ");
    Serial.println(slot);
    
    return 0;
}

void writeNFC(nfcSignal signal) {
    vector<uint8_t> uid = signal.getData();
    
    if (uid.size() != 4) {
        Serial.println("ERROR: Only 4-byte UIDs allowed.");
        return;
    }
    
    Serial.println("Tap NFC chip to write on...");
    uint8_t targetUidBuffer[7];
    uint8_t targetUidLength;
    
    if (!nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, targetUidBuffer, &targetUidLength, 5000)) {
        Serial.println("ERROR: No target NFC chip detected.");
        return;
    }
    
    Serial.println("Target NFC chip detecting. Starting UID clone...");
    
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