#include <Arduino.h>
#include <cc1101.h>

// ESP32-C3: cs, clk, miso, mosi
CC1101::Radio radio(7, 4, 5, 6);
int loopCounter = 0;

void setup() {
  Serial.begin(115200);
  delay(5000);

  if (radio.begin(CC1101::MOD_ASK_OOK, 433.92, 4.0) != CC1101::STATUS_OK) {
    Serial.println("CC1101 initialisation failed!");
    while (1);
  }

  radio.setSyncMode(CC1101::SYNC_MODE_NO_PREAMBLE);  // no sync word filtering
  radio.setCrc(false);                                // accept packets regardless of CRC
  radio.setAddressFilteringMode(CC1101::ADDR_FILTER_MODE_NONE);
  radio.setPacketLengthMode(CC1101::PKT_LEN_MODE_FIXED, 64);

  Serial.println("Listening...");
}

void loop() {
  uint8_t buf[64];
  size_t bytesRead = 0;

  if (radio.receive(buf, sizeof(buf), &bytesRead) == CC1101::STATUS_OK) {
    /*Serial.print("[");
    Serial.print(millis());
    Serial.print(" ms] Got ");
    Serial.print(bytesRead);
    Serial.print(" bytes: "); */

    for (size_t i = 0; i < bytesRead; i++) {
      if (buf[i] < 0x10) Serial.print("0");
      Serial.print(buf[i], HEX);
      Serial.print(" ");
      if ((i + 1) % 32 == 0) Serial.println(); // newline every 16 bytes
    }
  }

  loopCounter++;
  if (loopCounter >= 50){
    while(1);
  }

  //delay(50);
}
