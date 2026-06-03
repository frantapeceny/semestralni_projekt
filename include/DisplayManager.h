#pragma once
#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Signal_.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

class DisplayManager {
private:
    Adafruit_SSD1306 display;

public:
    DisplayManager(int8_t dc, int8_t rst, int8_t cs);

    bool begin();
    void clear();
    void update();
    
    void printText(String text, int x = 0, int y = 0, int size = 1, bool inverted = false);
    void showHeader(String title);
    void drawButton(String text, int x, int y, int w, int h, bool selected);
    void drawSignalBrowser(int currentIndex, int totalSignals, Signal* sig);
    void drawSignalMenu(int selectedIndex);
    void drawTypeSelector(int selectedIndex);
    void drawStatusMessage(String header, String line1, String line2 = "");

    void drawSnakeEnv();
    void drawSnakePauseMenu();
    void drawSnakeGameOver(int score);
};

extern DisplayManager displayManager;