#include "Snake.h"
#include "Config.h"
#include <cstdlib>

Snake snake;

void Snake::init() {
    body.clear();
    body.push_back({5, 3});
    body.push_back({4, 3});
    body.push_back({3, 3});
    dir = 0;
    score = 0;
    paused = false;
    alive = true;
    pauseIndex = 0;
    lastTick = millis();
    spawnFood();
}

void Snake::spawnFood() {
    while (true) {
        Point f = { 1 + rand() % (SNAKE_COLS - 2), 1 + rand() % (SNAKE_ROWS - 2) };
        bool onSnake = false;
        for (auto& s : body) {
            if (s.x == f.x && s.y == f.y) { onSnake = true; break; }
        }
        if (!onSnake) { food = f; return; }
    }
}

void Snake::turnLeft()  { dir = (dir + 3) % 4; }
void Snake::turnRight() { dir = (dir + 1) % 4; }
void Snake::togglePause() { paused = !paused; }

bool Snake::shouldTick() {
    const unsigned long TICK_MS = 250;
    if (millis() - lastTick >= TICK_MS) {
        lastTick = millis();
        return true;
    }
    return false;
}

void Snake::tick() {
    if (!alive || paused) return;

    Point head = body.front();
    const int dx[] = {1, 0, -1, 0};
    const int dy[] = {0, 1,  0,-1};
    head.x += dx[dir];
    head.y += dy[dir];

    // wall collision
    if (head.x < 0 || head.x >= SNAKE_COLS || head.y < 0 || head.y >= SNAKE_ROWS) {
        alive = false;
        return;
    }

    // self collision
    for (auto& s : body) {
        if (s.x == head.x && s.y == head.y) {
            alive = false;
            return;
        }
    }

    bool ate = (head.x == food.x && head.y == food.y);
    body.insert(body.begin(), head);
    if (ate) {
        digitalWrite(LED_PIN, HIGH);
        ledOnTime = millis();
        ledActive = true;
        score++;
        spawnFood();
    } else {
        body.pop_back();
    }
}

void Snake::updateLed() {
    if (ledActive && millis() - ledOnTime >= 150) {
        digitalWrite(LED_PIN, LOW);
        ledActive = false;
    }
}
