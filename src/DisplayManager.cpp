#include "DisplayManager.h"

DisplayManager::DisplayManager(int8_t dc, int8_t rst, int8_t cs) : display(SCREEN_WIDTH, SCREEN_HEIGHT, &SPI, dc, rst, cs) {}

bool DisplayManager::begin() {
    if (!display.begin(SSD1306_SWITCHCAPVCC)) {
        Serial.println("SSD1306 init failed!");
        return false;
    }
    display.setTextWrap(false);
    display.clearDisplay();
    display.display();
    return true;
}

void DisplayManager::clear() {
    display.clearDisplay();
}

void DisplayManager::update() {
    display.display();
}

void DisplayManager::printText(String text, int x, int y, int size, bool inverted) {
    display.setTextSize(size);
    if (inverted) {
        display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
    } else {
        display.setTextColor(SSD1306_WHITE);
    }
    display.setCursor(x, y);
    display.print(text);
}

void DisplayManager::showHeader(String title) {
    clear();
    display.fillRect(0, 0, SCREEN_WIDTH, 16, SSD1306_WHITE);
    printText(title, 4, 4, 1, true);
}

void DisplayManager::drawButton(String text, int x, int y, int w, int h, bool selected) {
    if (selected) {
        display.fillRect(x, y, w, h, SSD1306_WHITE);
    } else {
        display.drawRect(x, y, w, h, SSD1306_WHITE);
    }
    // size-1 font is 6x8 px per char
    int textX = x + (w - (int)text.length() * 6) / 2;
    int textY = y + (h - 8) / 2;
    printText(text, textX, textY, 1, selected);
}

void DisplayManager::drawSignalBrowser(int currentIndex, int totalSignals, Signal* sig) {
    String header = "";
    if (currentIndex > 0) {
        header += "< ";
    }
    if (currentIndex < totalSignals) {
        header += "SLOT " + String(currentIndex + 1) + "/" + String(totalSignals);
    } else {
        header += "NEW SLOT";
    }
    if (currentIndex < totalSignals) {
        header += " >";
    }
    showHeader(header);

    if (currentIndex == totalSignals) {
        printText("+ ADD NEW SIGNAL", 15, 30, 1, false);
        printText("[Press SELECT]", 25, 45, 1, true);
    } else if (sig != nullptr) {
        printText("Type: " + sig->typeName(), 10, 25, 1, false);
        printText("Info: " + sig->shortInfo(), 10, 40, 1, false);
        printText("[SELECT for options]", 4, 55, 1, true);
    }
    update();
}

void DisplayManager::drawTypeSelector(int selectedIndex) {
    showHeader("ADD SIGNAL");
    int cardWidth = 52;
    int cardHeight = 30;
    int cardY = 20;
    int padding = 6;
    drawButton("RADIO", padding, cardY, cardWidth, cardHeight, selectedIndex == 0);
    drawButton("NFC", SCREEN_WIDTH - cardWidth - padding, cardY, cardWidth, cardHeight, selectedIndex == 1);
    int hintY = cardY + cardHeight + 4;
    printText("< back", 4, hintY, 1, false);
    printText("cycle", 49, hintY, 1, false);
    printText("ok >", 98, hintY, 1, false);
    update();
}

void DisplayManager::drawSignalMenu(int selectedIndex) {
    showHeader("SIGNAL");
    int cardWidth = 52;
    int cardHeight = 30;
    int cardY = 20;
    int padding = 6;
    drawButton("TRANSMIT", padding, cardY, cardWidth, cardHeight, selectedIndex == 0);
    drawButton("DELETE", SCREEN_WIDTH - cardWidth - padding, cardY, cardWidth, cardHeight, selectedIndex == 1);
    int hintY = cardY + cardHeight + 4;
    printText("< back", 4, hintY, 1, false);
    printText("cycle", 49, hintY, 1, false);
    printText("ok >", 98, hintY, 1, false);
    update();
}

void DisplayManager::drawStatusMessage(String header, String line1, String line2) {
    showHeader(header);
    printText(line1, 8, 24, 1, false);
    if (line2.length() > 0) {
        printText(line2, 8, 38, 1, false);
    }
    update();
}
