#include "LightController.h"

LightController::LightController() {
    strip2 = new Adafruit_NeoPixel(LED_COUNT2, LED2_PIN, NEO_GRB + NEO_KHZ800);
    strip3 = new Adafruit_NeoPixel(LED_COUNT3, LED3_PIN, NEO_GRB + NEO_KHZ800);
    breakVal = 30;
    
    // Initialize animation state
    previousMillis = 0;
    currentPixel = 0;
    animationActive = false;
    animationType = 0;
    brakeActive = false;
}

void LightController::begin() {
    strip2->begin();
    strip2->show();
    strip2->setBrightness(BRIGHTNESS);

    strip3->begin();
    strip3->show();
    strip3->setBrightness(BRIGHTNESS);
}

void LightController::orangeBlinker2(uint8_t r, uint8_t g, uint8_t b, int wait) {
    strip2->clear();
    for (int i = 0; i < LED_COUNT2; i++) {
        strip2->setPixelColor(i, strip2->Color(r, g, b));
        strip2->show();
        if (i == LED_COUNT2) {
            for (int j = 0; j < LED_COUNT2; j++) {
                strip2->setPixelColor(j, strip2->Color(0, 0, 0));
            }
        }
        delay(wait);
    }
}

void LightController::orangeBlinker3(uint8_t r, uint8_t g, uint8_t b, int wait) {
    strip3->clear();
    for (int i = 0; i < LED_COUNT3; i++) {
        strip3->setPixelColor(i, strip3->Color(r, g, b));
        strip3->show();
        if (i == LED_COUNT3) {
            for (int j = 0; j < LED_COUNT3; j++) {
                strip3->setPixelColor(j, strip3->Color(0, 0, 0));
            }
        }
        delay(wait);
    }
}

void LightController::hazards(uint8_t r, uint8_t g, uint8_t b, int wait) {
    strip2->clear();
    strip3->clear();
    for (int i = 0; i < LED_COUNT2; i++) {
        strip2->setPixelColor(i, strip2->Color(r, g, b));
        strip3->setPixelColor(i, strip3->Color(r, g, b));
        strip2->show();
        strip3->show();
        if (i == LED_COUNT2) {
            for (int j = 0; j < LED_COUNT2; j++) {
                strip2->setPixelColor(j, strip2->Color(0, 0, 0));
                strip3->setPixelColor(j, strip3->Color(0, 0, 0));
            }
        }
        delay(wait);
    }
}

void LightController::update() {
    // If brake is active, don't update animations
    if (brakeActive) return;
    
    if (!animationActive) return;
    
    unsigned long currentMillis = millis();
    
    // Update animations at appropriate intervals
    if (currentMillis - previousMillis >= SPEED) {
        previousMillis = currentMillis;
        
        switch (animationType) {
            case 1: // Left turn
                if (currentPixel < LED_COUNT2) {
                    strip2->setPixelColor(currentPixel, animationColor);
                    strip2->show();
                    currentPixel++;
                } else {
                    // Animation complete
                    currentPixel = 0;
                    strip2->clear();
                    strip2->show();
                    animationActive = false;
                    animationType = 0;
                    running_lights(); // Restore running lights
                }
                break;
                
            case 2: // Right turn
                if (currentPixel < LED_COUNT3) {
                    strip3->setPixelColor(currentPixel, animationColor);
                    strip3->show();
                    currentPixel++;
                } else {
                    // Animation complete
                    currentPixel = 0;
                    strip3->clear();
                    strip3->show();
                    animationActive = false;
                    animationType = 0;
                    running_lights(); // Restore running lights
                }
                break;
                
            case 3: // Hazard
                if (currentPixel < LED_COUNT2) {
                    strip2->setPixelColor(currentPixel, animationColor);
                    strip3->setPixelColor(currentPixel, animationColor);
                    strip2->show();
                    strip3->show();
                    currentPixel++;
                } else {
                    // Animation complete
                    currentPixel = 0;
                    strip2->clear();
                    strip3->clear();
                    strip2->show();
                    strip3->show();
                    animationActive = false;
                    animationType = 0;
                    running_lights(); // Restore running lights
                }
                break;
        }
    }
}

void LightController::stopAnimation() {
    animationActive = false;
    animationType = 0;
    currentPixel = 0;
}

void LightController::brake_on() {
    // Set the brake active flag
    brakeActive = true;
    
    // Stop any animations immediately
    stopAnimation();
    
    // Set all LEDs to full brake brightness
    for(int i = 0; i < LED_COUNT2; i++) {
        strip2->setPixelColor(i, 255, 0, 0);
        strip3->setPixelColor(i, 255, 0, 0);
    }
    strip2->show();
    strip3->show();
}

void LightController::brake_off() {
    // Clear the brake active flag
    brakeActive = false;
    
    // Return to running lights
    running_lights();
}

void LightController::running_lights() {
    // If brake is active or animation is running, don't change the lights
    if (brakeActive || animationActive) return;
    
    breakVal = 20;
    for(int i = 0; i < LED_COUNT2; i++) {
        strip2->setPixelColor(i, breakVal, 0, 0);
        strip3->setPixelColor(i, breakVal, 0, 0);
    }
    strip2->show();
    strip3->show();
}