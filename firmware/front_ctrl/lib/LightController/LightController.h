#ifndef LIGHT_CONTROLLER_H
#define LIGHT_CONTROLLER_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "../Config/Config.h"

class LightController {
private:
    Adafruit_NeoPixel* strip2;
    Adafruit_NeoPixel* strip3;
    int breakVal;
    
    // Animation state variables
    unsigned long previousMillis;
    int currentPixel;
    bool animationActive;
    uint8_t animationType;  // 0 = none, 1 = left turn, 2 = right turn, 3 = hazard
    uint32_t animationColor;
    bool brakeActive;       // Track brake state separately

public:
    LightController();
    void begin();
    
    // Non-blocking animation functions
    void update();
    void stopAnimation();
    
    // Original blocking functions
    void orangeBlinker2(uint8_t r, uint8_t g, uint8_t b, int wait);
    void orangeBlinker3(uint8_t r, uint8_t g, uint8_t b, int wait);
    void hazards(uint8_t r, uint8_t g, uint8_t b, int wait);
    
    void brake_on();
    void brake_off();
    void running_lights();
};

#endif