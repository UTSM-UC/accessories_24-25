#ifndef LIGHT_CONTROLLER_H
#define LIGHT_CONTROLLER_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "../Config/Config.h"
#include "../CanHandler/CanHandler.h"  // Add this include

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
    
    // Custom non-blocking delay that checks for horn messages
    void customDelay(int ms, CanHandler* canHandler);

public:
    LightController();
    void begin();
    
    // Non-blocking animation functions
    void update();
    void stopAnimation();
    
    // Improved blocking functions with CAN checking
    void orangeBlinker2(uint8_t r, uint8_t g, uint8_t b, int wait, CanHandler* canHandler);
    void orangeBlinker3(uint8_t r, uint8_t g, uint8_t b, int wait, CanHandler* canHandler);
    void hazards(uint8_t r, uint8_t g, uint8_t b, int wait, CanHandler* canHandler);
    
    void brake_on();
    void brake_off();
    void running_lights();
};

#endif