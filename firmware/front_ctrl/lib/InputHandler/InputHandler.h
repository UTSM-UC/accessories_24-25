#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <Arduino.h>
#include "../Config/Config.h"

class InputHandler {
private:
    int buttonStates[NUM_INPUTS];
    int lastButtonStates[NUM_INPUTS];
    int sendStates[NUM_INPUTS];

public:
    InputHandler();
    void setupPins();
    int edgeDetector(int buttonNum);
    int getSendState(int pin) { return sendStates[pin]; }
    void resetSendState(int pin) { sendStates[pin] = -1; }
};

#endif