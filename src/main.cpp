#include <Arduino.h>
#include "radio.h"
#include "config.h"

using namespace std;

int currentSlot = 0; // pri zapnuti bude na obrazovce automaticky slot 0

void setup() {
    Serial.begin(115200);
    delay(3000);
    Serial.println("warmin' up");

    // setup tlacitek - volny je jeste GPIO 8
    //pinMode(21, INPUT); // move up
    pinMode(BUTTON2_PIN, INPUT_PULLUP); // click ok
    pinMode(BUTTON1_PIN, INPUT_PULLUP); // move down - rotary array
    pinMode(LED_PIN, OUTPUT);

}

bool is_pressed(int button_pin) {
    return digitalRead(button_pin) == LOW;
}

void loop() {
    // debug:
    // Serial.print("button1: ");
    // Serial.print(digitalRead(button1));
    // Serial.print(" button2: ");
    // Serial.print(digitalRead(button2));
    // Serial.print(" delka zachycenych dat: ");
    // Serial.println(signaly[currentSlot].getData().size());

    // button inputs binded with its respective actions
        // jeden button bude pricitat currentSlot, actions, druhy bude confirmovat
        // tj. prve budes tlacitkem 1 prochazet sloty a tlacitkem 2 zvolis slot, pak se ti objevi obrazovka s moznymi akcemi
        // tu budes opet prochazet pomoci tlacitka 1 a pomoci tlacitka 2 ji vyberes
        // kazdy slot by mel mit nejaky svuj medailonek - cislo, radio/nfc, baud rate / nejakou nfc charakterizaci, plny/prazdny
    
    // skip if no button is pressed
    if (!is_pressed(BUTTON1_PIN) && !is_pressed(BUTTON1_PIN)) {
        delay(10);
        return;
    }

    if (is_pressed(BUTTON1_PIN)) {
        Serial.println("Button 1 pressed, reading radio...");
        if (readRadio(currentSlot) == -1) {
            Serial.println("ERROR: Failed to read radio.");
        }

        // delay so that holding button doesn't spam
        delay(500);
    }

    if (is_pressed(BUTTON2_PIN)) {
        if (signals[currentSlot].getData().size() == 0) {
            Serial.println("ERROR: Selected slot is empty, nothing to write.");
            return;
        }
        
        writeRadio(signals[currentSlot]);
        delay(500);
    }
}
