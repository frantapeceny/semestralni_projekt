#pragma once
#include <Arduino.h>
#include <vector>
#include <Adafruit_SSD1306.h>

static const int SNAKE_COLS = 21;
static const int SNAKE_ROWS = 8;
static const int CELL = 6;
static const int SNAKE_OFFSET_Y = 16;

struct Point { int x; int y; };

class Snake {
private:
    std::vector<Point> body;
    Point food;
    int dir;          // 0=right 1=down 2=left 3=up
    int score;
    bool paused;
    bool alive;
    int pauseIndex;   // 0=continue 1=quit
    unsigned long lastTick;

    unsigned long ledOnTime = 0;
    bool ledActive = false;

    void spawnFood();

public:
    void init();
    void turnLeft();
    void turnRight();
    void togglePause();
    void tick();      // advance game state one step
    void updateLed();

    bool isAlive()  const { return alive; }
    bool isPaused() const { return paused; }
    int  getScore() const { return score; }
    int  getPauseIndex() const { return pauseIndex; }
    void cyclePauseIndex() { pauseIndex = 1 - pauseIndex; }

    const std::vector<Point>& getBody() const { return body; }
    Point getFood() const { return food; }
    int   getDir()  const { return dir; }

    bool shouldTick();  // returns true if enough time has passed
};

extern Snake snake;
