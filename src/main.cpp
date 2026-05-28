#include <Arduino.h>
#include "Config.h"
#include "Storage.h"
#include "Signal.h"
#include "NfcManager.h"
#include "RadioManager.h"
#include "DisplayManager.h"

Storage storage;
NfcManager nfcManager(PN532_SS);
RadioManager radioManager(CC1101_CS);
DisplayManager displayManager(OLED_DC, OLED_RST, OLED_CS);

volatile bool firedLeft = false, firedMid = false, firedRight = false;

void IRAM_ATTR isrLeft()  { firedLeft  = true; }
void IRAM_ATTR isrMid()   { firedMid   = true; }
void IRAM_ATTR isrRight() { firedRight = true; }

enum class AppState { BROWSING, SIGNAL_MENU, TYPE_SELECTOR };

AppState state = AppState::BROWSING;
int browseIndex = 0;
int menuIndex = 0;
bool needsRedraw = true;

void silencePeripherals() {
    digitalWrite(PN532_SS, HIGH);
    digitalWrite(CC1101_CS, HIGH);
}

void redraw() {
    silencePeripherals();
    switch (state) {
        case AppState::BROWSING:
            displayManager.drawSignalBrowser(browseIndex, storage.getCount(), storage.getSignal(browseIndex));
            break;
        case AppState::SIGNAL_MENU:
            displayManager.drawSignalMenu(menuIndex);
            break;
        case AppState::TYPE_SELECTOR:
            displayManager.drawTypeSelector(menuIndex);
            break;
    }
}

void setup() {
    Serial.begin(115200);

    pinMode(PN532_SS, OUTPUT);
    digitalWrite(PN532_SS, HIGH);
    pinMode(OLED_CS, OUTPUT);
    digitalWrite(OLED_CS, HIGH);
    pinMode(CC1101_CS, OUTPUT);
    digitalWrite(CC1101_CS, HIGH);

    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, -1);

    nfcManager.begin();
    silencePeripherals();

    if (displayManager.begin()) {
        silencePeripherals();
        displayManager.drawStatusMessage("BOOTING...", "Loading signals...");
    }

    pinMode(BUTTON_1, INPUT_PULLUP);
    pinMode(BUTTON_2, INPUT_PULLUP);
    pinMode(BUTTON_3, INPUT_PULLUP);
    attachInterrupt(BUTTON_1, isrLeft, FALLING);
    attachInterrupt(BUTTON_2, isrMid, FALLING);
    attachInterrupt(BUTTON_3, isrRight, FALLING);
    pinMode(LED_PIN, OUTPUT);

    storage.loadAllFromFlash();
}

void loop() {
    // button debounce (so it doesnt accidentally trigger twice when only intended to be pressed once)
    static unsigned long lastLeft = 0, lastMid = 0, lastRight = 0;
    bool left = false, mid = false, right = false;

    if (firedLeft && millis() - lastLeft > 200) {
        firedLeft = false;
        lastLeft = millis();
        left = true;
    }
    if (firedMid && millis() - lastMid > 200) {
        firedMid = false;
        lastMid = millis();
        mid = true;
    }
    if (firedRight && millis() - lastRight > 200) {
        firedRight = false;
        lastRight = millis();
        right = true;
    }

    if (left || mid || right) {
        needsRedraw = true;
    }

    // display menu
    switch (state) {
        case AppState::BROWSING:
            if (left && browseIndex > 0) {
                // scroll list to left
                browseIndex--;
            }
            if (right && browseIndex < storage.getCount()) {
                // scroll list to right
                browseIndex++;
            }
            if (mid) {
                // select
                menuIndex = 0;
                // create new signal if we are at index == saved signal count or select the current signal otherwise
                state = (browseIndex == storage.getCount()) ? AppState::TYPE_SELECTOR : AppState::SIGNAL_MENU;
            }
            break;

        case AppState::SIGNAL_MENU:
            // left to go back
            if (left) {
                state = AppState::BROWSING;
                break;
            }
            // middle to scroll
            if (mid) {
                menuIndex = 1 - menuIndex;
                break;
            }
            // right to select (delete or transmit)
            if (right) {
                if (menuIndex == 0) {
                    silencePeripherals();
                    displayManager.drawStatusMessage("TRANSMITTING...", "Please wait...");
                    storage.getSignal(browseIndex)->transmit();
                } else {
                    storage.deleteSignal(browseIndex);
                    if (browseIndex > 0) {
                        browseIndex--;
                    }
                }
                // auto go back to scrolling saved signals
                state = AppState::BROWSING;
            }
            break;

        // when saving a new signal pick nfc/radio
        case AppState::TYPE_SELECTOR:
            if (left) {
                state = AppState::BROWSING;
                break;
            }
            if (mid) {
                menuIndex = 1 - menuIndex;
                break;
            }
            // right to select and execute capture logic of selected type
            if (right) {
                if (menuIndex == 0) {
                    silencePeripherals();
                    displayManager.drawStatusMessage("CAPTURING...", "Scanning radio...", "~30s, please wait");
                    RadioSignal captured = radioManager.capture();
                    if (!captured.getData().empty()) {
                        storage.saveSignal(new RadioSignal(captured));
                        browseIndex = storage.getCount() - 1;
                    }
                } else {
                    silencePeripherals();
                    displayManager.drawStatusMessage("CAPTURING...", "Hold card to reader", "~10s, please wait");
                    NfcSignal read = nfcManager.read();
                    if (!read.getUID().empty()) {
                        storage.saveSignal(new NfcSignal(read));
                        browseIndex = storage.getCount() - 1;
                    }
                }
                state = AppState::BROWSING;
            }
            break;
    }

    if (needsRedraw) {
        redraw();
        needsRedraw = false;
    }
}
