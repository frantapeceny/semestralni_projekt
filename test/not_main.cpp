#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>

#define PN532_SS (7)

Adafruit_PN532 nfc(PN532_SS);

void printHex(uint8_t* data, uint8_t length) {
  for (uint8_t i = 0; i < length; i++) {
    if (data[i] < 0x10) Serial.print("0");
    Serial.print(data[i], HEX);
    if (i < length - 1) Serial.print(":");
  }
}

void readMifareClassic(uint8_t* uid, uint8_t uidLength) {
  Serial.println("Reading Mifare Classic memory...");

  uint8_t keyA[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

  for (uint8_t sector = 0; sector < 16; sector++) {
    bool auth = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, sector * 4, 0, keyA);

    Serial.print("\n--- Sector "); Serial.print(sector); Serial.println(" ---");

    if (!auth) {
      Serial.println("  Authentication failed! (non-default key?)");
      continue;
    }

    for (uint8_t block = 0; block < 4; block++) {
      uint8_t blockNum = sector * 4 + block;
      uint8_t data[16];

      bool success = nfc.mifareclassic_ReadDataBlock(blockNum, data);
      Serial.print("  Block ");
      if (blockNum < 10) Serial.print("0");
      Serial.print(blockNum);
      Serial.print(": ");

      if (success) {
        printHex(data, 16);
        Serial.print("  |");
        for (uint8_t i = 0; i < 16; i++) {
          if (data[i] >= 32 && data[i] <= 126) Serial.print((char)data[i]);
          else Serial.print(".");
        }
        Serial.print("|");
      } else {
        Serial.print("Read failed");
      }
      Serial.println();
    }
  }
}

void readNTAG(uint8_t* uid, uint8_t uidLength) {
  Serial.println("Reading NTAG/Ultralight memory...");

  uint8_t prevData[4] = {0};
  uint8_t repeatCount = 0;

  for (uint8_t page = 0; page < 231; page++) {
    uint8_t data[4];
    bool success = nfc.ntag2xx_ReadPage(page, data);

    if (!success) {
      Serial.print("Stopped at page "); Serial.println(page);
      break;
    }

    // Stop if we're getting repeated identical pages (likely garbage or end of tag)
    if (memcmp(data, prevData, 4) == 0) {
      repeatCount++;
      if (repeatCount > 3) {
        Serial.print("Stopping - repeated data detected at page ");
        Serial.println(page);
        break;
      }
    } else {
      repeatCount = 0;
    }
    memcpy(prevData, data, 4);

    Serial.print("  Page ");
    if (page < 10) Serial.print("0");
    Serial.print(page);
    Serial.print(": ");
    printHex(data, 4);
    Serial.println();

    delay(10); // small delay to avoid SPI communication errors
  }
}

void setup(void) {
  Serial.begin(115200);
  delay(5000); // wait for serial monitor to open instead of while(!Serial)

  Serial.println("Adafruit PN532 NFC/RFID reader - ESP32-C3");

  SPI.begin(4, 5, 6, 7); // explicit hardware SPI pins: SCK, MISO, MOSI, SS
  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) {
    Serial.println("ERROR: Didn't find PN532 board. Check wiring!");
    while (1);
  }

  Serial.print("Found chip PN5"); Serial.println((versiondata >> 24) & 0xFF, HEX);
  Serial.print("Firmware ver. "); Serial.print((versiondata >> 16) & 0xFF, DEC);
  Serial.print('.'); Serial.println((versiondata >> 8) & 0xFF, DEC);

  nfc.SAMConfig();
  Serial.println("Waiting for an NFC card...");
}

void loop(void) {
  uint8_t uid[7] = { 0 };
  uint8_t uidLength;

  bool success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

  if (success) {
    Serial.println("\n----------- Card detected! -----------");
    Serial.print("UID: ");
    printHex(uid, uidLength);
    Serial.println();

    if (uidLength == 4) {
      Serial.println("Card type: Mifare Classic");
      readMifareClassic(uid, uidLength);
    } else if (uidLength == 7) {
      Serial.println("Card type: NTAG / Mifare Ultralight");
      readNTAG(uid, uidLength);
    } else {
      Serial.println("Unknown card type");
    }

    Serial.println("\n--------------------------------------");
    delay(2000);
  }
}