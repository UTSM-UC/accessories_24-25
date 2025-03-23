#include "InputHandler.h"

InputHandler::InputHandler() {
    for (int i = 0; i < NUM_INPUTS; i++) {
        buttonStates[i] = 0;
        lastButtonStates[i] = 0;
        sendStates[i] = -1;
    }
}

void InputHandler::setupPins() {
    pinMode(ENABLE3_PIN, INPUT_PULLUP);
    pinMode(HEADLIGHT_PIN, OUTPUT);
    pinMode(HORN_PIN, OUTPUT);
    digitalWrite(HORN_PIN, HIGH);
    pinMode(LED_PIN, OUTPUT);
    pinMode(WIPER_PIN, OUTPUT);
    pinMode(BRAKE_PIN, INPUT_PULLUP);
    pinMode(CAN0_INT, INPUT);
    pinMode(LED_BUILTIN, OUTPUT);
}

int InputHandler::edgeDetector(int buttonNum) {
    buttonStates[buttonNum] = digitalRead(buttonNum);

    if (buttonStates[buttonNum] != lastButtonStates[buttonNum]) {
        lastButtonStates[buttonNum] = buttonStates[buttonNum];
        if (buttonStates[buttonNum] == HIGH) {
            return 0;
        } else {
            return 1;
        }
    }
    return -1;
}