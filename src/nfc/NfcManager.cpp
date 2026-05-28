#include "NfcManager.h"

using namespace std;

NfcManager::NfcManager(uint8_t csPin) : nfc(csPin) {}

bool NfcManager::begin() {
    nfc.begin();
    if (!nfc.getFirmwareVersion()) {
        Serial.println("ERROR: PN532 not found");
        return false;
    }
    nfc.SAMConfig();
    return true;
}

NfcSignal NfcManager::read() {
    NfcSignal signal;
    uint8_t uid[7];
    uint8_t uidLen;

    unsigned long start = millis();
    while (millis() - start < NFC_READ_TIMEOUT_MS) {
        if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLen, 100)) {
            signal.setUID(vector<uint8_t>(uid, uid + uidLen));
            return signal;
        }
    }
    return signal;
}

void NfcManager::write(const NfcSignal& signal) {
    vector<uint8_t> uid = signal.getUID();
    if (uid.size() != 4) {
        Serial.println("ERROR: uid must be 4 bytes");
        return;
    }

    uint8_t targetUid[7];
    uint8_t targetUidLen;
    if (!nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, targetUid, &targetUidLen, 5000)) {
        Serial.println("ERROR: no card detected");
        return;
    }

    // block 0 layout: uid[4] bcc sak atqa[2] mfr[8]
    uint8_t block0[16] = {0};
    block0[0] = uid[0]; block0[1] = uid[1]; block0[2] = uid[2]; block0[3] = uid[3];
    block0[4] = uid[0] ^ uid[1] ^ uid[2] ^ uid[3];
    block0[5] = 0x08; block0[6] = 0x04; block0[7] = 0x00;
    for (int i = 8; i < 16; i++) block0[i] = 0x62;

    uint8_t keyA[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    if (!nfc.mifareclassic_AuthenticateBlock(targetUid, targetUidLen, 0, 0, keyA)) {
        Serial.println("ERROR: auth failed");
        return;
    }
    if (!nfc.mifareclassic_WriteDataBlock(0, block0)) {
        Serial.println("ERROR: write failed");
        return;
    }
    Serial.println("write ok");
}
