#include <Arduino.h>
#include <cc1101.h>

// ESP32-C3 SPI pins
CC1101::Radio radio(7, 4, 5, 6);

// Baud rates to sweep through (kBaud)
const double baudRates[] = { 0.6, 1.0, 1.2, 2.0, 2.4, 3.4, 4.0, 4.8, 8.0, 9.6};
const int numBaudRates = sizeof(baudRates) / sizeof(baudRates[0]);

// How many 64-byte captures to collect per baud rate before scoring
const int CAPTURES_PER_RATE = 8;

// ---------- scoring ----------

// Count non-FF non-00 bytes — more = more interesting
int countDataBytes(uint8_t *buf, size_t len) {
  int count = 0;
  for (size_t i = 0; i < len; i++) {
    if (buf[i] != 0xFF && buf[i] != 0x00) count++;
  }
  return count;
}

// Look for a repeated subsequence of given length within the buffer
// Returns how many times it appears (minus 1, so 0 = no repeat)
int countRepeats(uint8_t *buf, size_t len, int seqLen) {
  int maxRepeats = 0;
  for (size_t i = 0; i < len - seqLen; i++) {
    // skip if sequence is all FF or all 00
    bool allSame = true;
    for (int k = 0; k < seqLen; k++) {
      if (buf[i + k] != buf[i]) { allSame = false; break; }
    }
    if (allSame) continue;

    int repeats = 0;
    for (size_t j = i + seqLen; j <= len - seqLen; j += seqLen) {
      if (memcmp(&buf[i], &buf[j], seqLen) == 0) repeats++;
    }
    if (repeats > maxRepeats) maxRepeats = repeats;
  }
  return maxRepeats;
}

// Score a set of captures for a given baud rate
// Higher = more likely to be the right setting
float scoreCatures(uint8_t captures[][64], int numCaptures) {
  float score = 0;

  // 1. Average data byte ratio across captures
  float avgDataRatio = 0;
  for (int i = 0; i < numCaptures; i++) {
    avgDataRatio += (float)countDataBytes(captures[i], 64) / 64.0f;
  }
  avgDataRatio /= numCaptures;
  score += avgDataRatio * 40.0f;  // up to 40 points

  // 2. Reward captures that have a burst structure:
  //    starts with FF, has data in middle, ends with FF
  int burstCount = 0;
  for (int i = 0; i < numCaptures; i++) {
    bool startsFF = (captures[i][0] == 0xFF && captures[i][1] == 0xFF);
    bool endsFF   = (captures[i][62] == 0xFF && captures[i][63] == 0xFF);
    bool hasData  = countDataBytes(captures[i], 64) > 8;
    if (startsFF && endsFF && hasData) burstCount++;
  }
  score += (float)burstCount / numCaptures * 30.0f;  // up to 30 points

  // 3. Look for repeating patterns across captures (same remote code sent multiple times)
  //    Compare first 8 non-FF bytes of each capture
  int similarPairs = 0;
  for (int i = 0; i < numCaptures; i++) {
    for (int j = i + 1; j < numCaptures; j++) {
      // extract data bytes from both
      uint8_t a[8], b[8];
      int ai = 0, bi = 0;
      for (int k = 0; k < 64 && (ai < 8 || bi < 8); k++) {
        if (ai < 8 && captures[i][k] != 0xFF && captures[i][k] != 0x00) a[ai++] = captures[i][k];
        if (bi < 8 && captures[j][k] != 0xFF && captures[j][k] != 0x00) b[bi++] = captures[j][k];
      }
      if (ai == 8 && bi == 8 && memcmp(a, b, 8) == 0) similarPairs++;
    }
  }
  score += (float)similarPairs * 5.0f;  // 5 points per matching pair

  // 4. Look for internal repeats within a single capture
  int internalRepeats = 0;
  for (int i = 0; i < numCaptures; i++) {
    internalRepeats += countRepeats(captures[i], 64, 4);  // look for 4-byte repeats
  }
  score += (float)internalRepeats / numCaptures * 10.0f;  // up to 10 points

  return score;
}

// ---------- main ----------

void setup() {
  Serial.begin(115200);
  delay(5000);
  Serial.println("\n=== CC1101 Baud Rate Sweeper ===");
  Serial.println("Press button / trigger remote when prompted\n");

  float bestScore = -1;
  double bestBaud = 0;

  uint8_t captures[CAPTURES_PER_RATE][64];

  for (int b = 0; b < numBaudRates; b++) {
    double baud = baudRates[b];

    // reinit radio for this baud rate
    if (radio.begin(CC1101::MOD_ASK_OOK, 433.92, baud) != CC1101::STATUS_OK) {
      Serial.printf("[%.1f kBaud] Init failed, skipping\n", baud);
      continue;
    }
    radio.setSyncMode(CC1101::SYNC_MODE_NO_PREAMBLE);
    radio.setCrc(false);
    radio.setAddressFilteringMode(CC1101::ADDR_FILTER_MODE_NONE);
    radio.setPacketLengthMode(CC1101::PKT_LEN_MODE_FIXED, 64);

    Serial.printf("\n[%.1f kBaud] Press remote now (collecting %d captures)...\n",
                  baud, CAPTURES_PER_RATE);

    // collect captures
    int collected = 0;
    unsigned long deadline = millis() + 8000;  // 8 seconds per baud rate

    while (collected < CAPTURES_PER_RATE && millis() < deadline) {
      size_t bytesRead = 0;
      if (radio.receive(captures[collected], 64, &bytesRead) == CC1101::STATUS_OK) {
        collected++;
        Serial.printf("  capture %d/%d\n", collected, CAPTURES_PER_RATE);
      }
    }

    if (collected < 2) {
      Serial.println("  not enough captures, skipping");
      continue;
    }

    float score = scoreCatures(captures, collected);
    Serial.printf("  score: %.1f\n", score);

    if (score > bestScore) {
      bestScore = score;
      bestBaud = baud;
    }
  }

  Serial.println("\n=== RESULTS ===");
  Serial.printf("Best baud rate: %.1f kBaud (score: %.1f)\n", bestBaud, bestScore);
  Serial.println("Now re-run with that baud rate to capture the actual signal.");
}

void loop() {



}