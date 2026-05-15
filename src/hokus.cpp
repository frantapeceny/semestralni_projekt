#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define PN532_SS  21
#define OLED_CS   9
#define OLED_DC   10
#define OLED_RST  20
#define LED       8

// Both devices on hardware SPI, differentiated by CS
Adafruit_PN532 nfc(PN532_SS);
Adafruit_SSD1306 display(128, 64, &SPI, OLED_DC, OLED_RST, OLED_CS);

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
    delay(10);
  }
}

void setup(void) {
  Serial.begin(115200);
  delay(5000);

  Serial.println("Adafruit PN532 NFC/RFID reader - ESP32-C3");

  // ALL CS pins high before anything
  pinMode(PN532_SS, OUTPUT); digitalWrite(PN532_SS, HIGH);
  pinMode(OLED_CS,  OUTPUT); digitalWrite(OLED_CS,  HIGH);
  pinMode(7,        OUTPUT); digitalWrite(7,         HIGH); // CC1101

  SPI.begin(4, 5, 6, -1);

  // Init PN532 first
  digitalWrite(OLED_CS, HIGH); // ensure display is deselected
  nfc.begin();
  uint32_t versiondata = nfc.getFirmwareVersion();
  while (!versiondata) {
    Serial.println("ERROR: Didn't find PN532 board.");
    delay(500);
    versiondata = nfc.getFirmwareVersion();
  }
  Serial.print("Found chip PN5"); Serial.println((versiondata >> 24) & 0xFF, HEX);
  Serial.print("Firmware ver. "); Serial.print((versiondata >> 16) & 0xFF, DEC);
  Serial.print('.'); Serial.println((versiondata >> 8) & 0xFF, DEC);
  nfc.SAMConfig();

  // Init display second
  digitalWrite(PN532_SS, HIGH); // ensure PN532 is deselected
  if (!display.begin(SSD1306_SWITCHCAPVCC)) {
    Serial.println("SSD1306 init failed");
  }
  display.clearDisplay();
  display.display();
  Serial.println("Display initialized");

  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);

  Serial.println("Waiting for NFC card...");
}

void loop(void) {
  uint8_t uid[7] = { 0 };
  uint8_t uidLength;

  // Ensure display CS is high before PN532 transaction
  digitalWrite(OLED_CS, HIGH);
  bool success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

  if (success) {
    Serial.println("\n----------- Card detected! -----------");
    Serial.print("UID: ");
    printHex(uid, uidLength);
    Serial.println();

    // Ensure PN532 CS is high before display transaction
    digitalWrite(PN532_SS, HIGH);
    delay(10);

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    for (uint8_t i = 0; i < uidLength; i++) {
      if (uid[i] < 0x10) display.print("0");
      display.print(uid[i], HEX);
      if (i < uidLength - 1) display.print(":");
    }
    display.display();

    // Ensure display CS is high before resuming PN532
    digitalWrite(OLED_CS, HIGH);
    delay(10);

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