#include <Arduino.h>

bool isPressed(int button_pin) {
    return digitalRead(button_pin) == LOW;
}
