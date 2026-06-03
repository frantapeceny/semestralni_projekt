#include "DisplayManager.h"
#include "Snake.h"

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
    if (currentIndex == 0) {
        header += "  ";
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
        display.drawLine(25, 44, 108, 44, WHITE);
        printText("[Press SELECT]", 25, 45, 1, true);
    } else if (sig != nullptr) {
        String nameOfSignal = sig->typeName();
        printText("Type: " + nameOfSignal, 10, 25, 1, false);
        printText(sig->shortInfo(), 10, 40, 1, false);
        display.drawLine(4, 54, 123, 54, WHITE);
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
    printText("ok", 56, hintY, 1, false);
    printText("cycle >", 83, hintY, 1, false);
    update();
}

void DisplayManager::drawSignalMenu(int selectedIndex) {
    showHeader("SIGNAL ACTIONS");
    int cardWidth = 52;
    int cardHeight = 30;
    int cardY = 20;
    int padding = 6;
    drawButton("TRANSMIT", padding, cardY, cardWidth, cardHeight, selectedIndex == 0);
    drawButton("DELETE", SCREEN_WIDTH - cardWidth - padding, cardY, cardWidth, cardHeight, selectedIndex == 1);
    int hintY = cardY + cardHeight + 4;
    printText("< back", 4, hintY, 1, false);
    printText("ok", 56, hintY, 1, false);
    printText("cycle >", 83, hintY, 1, false);
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

void DisplayManager::drawSnakePauseMenu() {
    display.fillRect(12, 22, 102, 36, SSD1306_BLACK);
    display.drawRect(12, 22, 102, 36, SSD1306_WHITE);
    printText("PAUSED", 46, 27, 1, false);
    drawButton("CONTINUE", 15, 38, 54, 16, snake.getPauseIndex() == 0);
    drawButton("QUIT", 71, 38, 40, 16, snake.getPauseIndex() == 1);
    update();
}

void DisplayManager::drawSnakeGameOver(int score) {
    showHeader("GAME OVER");
    printText("Score: " + String(score), 30, 28, 1, false);
    int cardHeight = 30;
    int cardY = 20;
    int hintY = cardY + cardHeight + 4;
    printText("< again", 4, hintY, 1, false);
    printText("quit >", 90, hintY, 1, false);
    update();
}

void DisplayManager::drawSnakeEnv() {
    clear();

    // header
    display.fillRect(0, 0, SCREEN_WIDTH, 14, SSD1306_WHITE);
    printText("SNAKE  " + String(snake.getScore()), 4, 4, 1, true);

    // border
    display.drawRect(0, SNAKE_OFFSET_Y, SNAKE_COLS * CELL, SNAKE_ROWS * CELL, SSD1306_WHITE);

    // food - cross
    /*
    int fx = snake.getFood().x * CELL + 1;
    int fy = snake.getFood().y * CELL + SNAKE_OFFSET_Y + 1;
    display.drawLine(fx, fy, fx+3, fy+3, SSD1306_WHITE);
    display.drawLine(fx+3, fy, fx, fy+3, SSD1306_WHITE);
    */

    // food - circle
    int fx = snake.getFood().x * CELL + CELL/2;
    int fy = snake.getFood().y * CELL + SNAKE_OFFSET_Y + CELL/2;
    display.drawCircle(fx, fy, CELL/2 - 1, SSD1306_WHITE);

    // snake
    const auto& body = snake.getBody();
    for (int i = 0; i < (int)body.size(); i++) {
        int px = body[i].x * CELL + 1;
        int py = body[i].y * CELL + SNAKE_OFFSET_Y + 1;
        if (i == 0) {
            display.fillRect(px, py, CELL-2, CELL-2, SSD1306_WHITE);
        } else {
            display.drawRect(px, py, CELL-2, CELL-2, SSD1306_WHITE);
        }
    }

    update();
}


