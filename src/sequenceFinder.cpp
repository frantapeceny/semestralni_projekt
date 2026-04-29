#include <Arduino.h>
#include <cc1101.h>
#include <string.h>
#include <vector>

using namespace std;

// eps-c3: cs, clk, miso, mosi
CC1101::Radio radio(7, 4, 5, 6);

void radioSetup() {
  radio.setSyncMode(CC1101::SYNC_MODE_NO_PREAMBLE);  // ctu vsechno, co jde okolo - ne jen nejaky uzsi vyber
  radio.setCrc(false);                                // accept packets regardless of CRC
  radio.setAddressFilteringMode(CC1101::ADDR_FILTER_MODE_NONE);
  radio.setPacketLengthMode(CC1101::PKT_LEN_MODE_FIXED, 64);
}

void setup() {
  Serial.begin(115200);
  delay(2000); // ve final verzi odebrat

  while (radio.begin(CC1101::MOD_ASK_OOK, 433.92, 4.0) != CC1101::STATUS_OK) {
    Serial.println("CC1101 init failed!");
    delay(200);
  }

  radioSetup();
  
  Serial.println("Starting to capture...");
}

void loop() {
  unsigned long start = millis();
  uint8_t zasobnik[64] = {00};
  vector <uint8_t> data;
  size_t bytesRead = 0;
  bool zachyt = false;

  while (millis() - start < 4000){
    radio.receive(zasobnik, sizeof(zasobnik), &bytesRead);
    for (int i = 0; i < bytesRead; i++) {
      if (zasobnik[i] != 0x00 && zasobnik[i] != 0xFF){
        zachyt = true;
      }
      if (zachyt) {
        data.push_back(zasobnik[i]);
      }
    }
    Serial.print("noch: ");
    Serial.print(4 - (millis() - start)/1000);
    Serial.println(" sekunde(n)"); // ted by mohla svitit dioda
  }

  Serial.println("capturing done");

  delay(500);

  for (int i = 0; i < data.size(); i++){
    if (data[i] < 0x10){
      Serial.print("0");
    }
    Serial.print(data[i], HEX);
    Serial.print(" ");
    if ((i + 1) % 32 == 0) { Serial.println(); }
  }

  while(true){};

}

