#include "RadioManager.h"
#include <string.h>

static const int CAPTURES_PER_RATE = 4;
static const double baudRates[] = { 1.0, 1.2, 2.0, 2.4, 4.0, 4.8, 9.6 }; 
static const int numBaudRates = sizeof(baudRates) / sizeof(baudRates[0]);
static const int DELKA_SNIMANI_RADIO = 2000;

RadioManager::RadioManager(uint8_t cs, uint8_t gdo0, uint8_t rst, uint8_t gdo2) : radio(cs, gdo0, rst, gdo2) {}

int RadioManager::setupRadio(double baud, int length) {
    if (radio.begin(CC1101::MOD_ASK_OOK, 433.92, baud) != CC1101::STATUS_OK) {  // baud rate je tu jedno
        Serial.println("CC1101 init failed!");
        delay(200);
        return -1;
    }
    radio.setSyncMode(CC1101::SYNC_MODE_NO_PREAMBLE);  // ctu vsechno, co jde okolo - ne jen nejaky uzsi vyber
    radio.setCrc(false);  // accept packets regardless of CRC
    radio.setAddressFilteringMode(CC1101::ADDR_FILTER_MODE_NONE);
    radio.setPacketLengthMode(CC1101::PKT_LEN_MODE_FIXED, length);
    return 0;
}

int RadioManager::countDataBytes(uint8_t *buf, size_t len) {
    int count = 0;
    for (size_t i = 0; i < len; i++) {
        if (buf[i] != 0xFF && buf[i] != 0x00) count++;
    }
    return count;
}

int RadioManager::countRepeats(uint8_t *buf, size_t len, int seqLen) {
    int maxRepeats = 0;
    for (size_t i = 0; i < len - seqLen; i++) {
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

float RadioManager::scoreCaptures(uint8_t captures[][64], int numCaptures) {
    float score = 0;
    float avgDataRatio = 0;
    for (int i = 0; i < numCaptures; i++) {
        avgDataRatio += (float)countDataBytes(captures[i], 64) / 64.0f;
    }
    avgDataRatio /= numCaptures;
    score += avgDataRatio * 40.0f;  

    int burstCount = 0;
    for (int i = 0; i < numCaptures; i++) {
        bool startsFF = (captures[i][0] == 0xFF && captures[i][1] == 0xFF);
        bool endsFF   = (captures[i][62] == 0xFF && captures[i][63] == 0xFF);
        bool hasData  = countDataBytes(captures[i], 64) > 8;
        if (startsFF && endsFF && hasData) burstCount++;
    }
    score += (float)burstCount / numCaptures * 30.0f;  

    int similarPairs = 0;
    for (int i = 0; i < numCaptures; i++) {
        for (int j = i + 1; j < numCaptures; j++) {
            uint8_t a[8], b[8];
            int ai = 0, bi = 0;
            for (int k = 0; k < 64 && (ai < 8 || bi < 8); k++) {
                if (ai < 8 && captures[i][k] != 0xFF && captures[i][k] != 0x00) a[ai++] = captures[i][k];
                if (bi < 8 && captures[j][k] != 0xFF && captures[j][k] != 0x00) b[bi++] = captures[j][k];
            }
            if (ai == 8 && bi == 8 && memcmp(a, b, 8) == 0) similarPairs++;
        }
    }
    score += (float)similarPairs * 5.0f;  

    int internalRepeats = 0;
    for (int i = 0; i < numCaptures; i++) {
        internalRepeats += countRepeats(captures[i], 64, 4);  
    }
    score += (float)internalRepeats / numCaptures * 10.0f;  

    return score;
}

std::vector<uint8_t> RadioManager::extractCycle(std::vector<uint8_t>& data) {
    int n = data.size();
    for (int len = 10; len <= n / 2; len++) {
        bool match = true;
        for (int i = 0; i < len; i++) {
            if (data[i] != data[i + len]) {
                match = false;
                break;
            }
        }
        if (match) {
            Serial.printf("nalezen cyklus delky %d\n", len);
            return std::vector<uint8_t>(data.begin(), data.begin() + len);
        }
    }
    
    Serial.println("Cyklus nenalezen, pouzivam prvnich 64 bytu");
    int cutoff = std::min((int)data.size(), 64);
    return std::vector<uint8_t>(data.begin(), data.begin() + cutoff);
}

double RadioManager::baudRateFinder() {
    float bestScore = -1;
    double bestBaud = 0;
    uint8_t captures[CAPTURES_PER_RATE][64];

    for (int b = 0; b < numBaudRates; b++) {
        double baud = baudRates[b];
        if (setupRadio(baud, 64) == -1) {
            return -1;
        }
        
        Serial.printf("\n[%.1f kBaud] Press remote now (collecting %d captures)...\n", baud, CAPTURES_PER_RATE);
        int collected = 0;
        unsigned long deadline = millis() + 4000;  

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

        float score = scoreCaptures(captures, collected);
        Serial.printf("  score: %.1f\n", score);

        if (score > bestScore) {
            bestScore = score;
            bestBaud = baud;
        }
    }
    Serial.println("\n=== RESULTS ===");
    Serial.printf("Best baud rate: %.1f kBaud (score: %.1f)\n", bestBaud, bestScore);
    return bestBaud;
}

RadioSignal RadioManager::capture() {
    RadioSignal capturedSignal;
    
    Serial.println("press and hold the transmitter while is the led on");
    delay(500);
    digitalWrite(LED_PIN, HIGH);

    double baudRate = baudRateFinder(); 
    if (baudRate == -1) {
        digitalWrite(LED_PIN, LOW);
        return capturedSignal; // Return empty signal on fail
    }

    setupRadio(baudRate, 64);

    unsigned long start = millis();
    uint8_t zasobnik[64] = {00};
    std::vector<uint8_t> data;
    size_t bytesRead = 0;
    bool zachyt = false;

    while (millis() - start < DELKA_SNIMANI_RADIO){ 
        radio.receive(zasobnik, sizeof(zasobnik), &bytesRead);
        for (int i = 0; i < bytesRead; i++) {
            if (zasobnik[i] != 0x00 && zasobnik[i] != 0xFF){
                zachyt = true;
            }
            if (zachyt) {
                data.push_back(zasobnik[i]);
            }
        }
    }

    Serial.println("capturing done");
    digitalWrite(LED_PIN, LOW);
    
    if (data.size() > 0) {
        std::vector<uint8_t> sequence = extractCycle(data);
        capturedSignal.setBaudRate(baudRate);
        capturedSignal.setData(sequence);
        
        // Print data
        for (int i = 0; i < sequence.size(); i++){
            if (sequence[i] < 0x10) Serial.print("0");
            Serial.print(sequence[i], HEX);
            Serial.print(" ");
            if ((i + 1) % 32 == 0) Serial.println();
        }
        Serial.println();
    }

    return capturedSignal;
}

void RadioManager::transmit(const RadioSignal& signal) {
    double baud = signal.getBaudRate();
    Serial.println("Sending gate code...");

    int length = signal.getData().size();
    
    // Dynamic memory allocation instead of Variable Length Array (VLA)
    // VLAs are not standard C++ and can cause stack overflow on microcontrollers
    uint8_t* arrayToSend = new uint8_t[length];
    for (int t = 0; t < length; t++) {
        arrayToSend[t] = signal.getData()[t];
    }

    setupRadio(baud, length);

    for (int i = 0; i < 2; i++) {
        CC1101::Status s = radio.transmit(arrayToSend, length);
        if (s != CC1101::STATUS_OK) {
            Serial.print("Transmit failed: ");
            Serial.println(s);
        } else {
            Serial.print("Sent ");
            Serial.println(i + 1);
        }
        delay(10);
    }
    
    delete[] arrayToSend;  // clean up memory
    Serial.println("Done.");
}
