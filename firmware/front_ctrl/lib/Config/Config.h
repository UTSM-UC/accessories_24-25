#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Pin Definitions
#define LED2_PIN 3         // originally 8
#define LED3_PIN 4         // originally 6
#define ENABLE3_PIN 7
#define SPI_CS_PIN 10
#define CAN0_INT 2
#define HORN_PIN 0
#define L_BLINKER_PIN 3
#define R_BLINKER_PIN 4
#define HEADLIGHT_PIN1 5
#define HEADLIGHT_PIN2 6
#define LED_PIN 5
#define WIPER_PIN 7
#define BRAKE_PIN 8

// NeoPixel Definitions
#define LED_COUNT 6          // 60 per Strip
#define LED_COUNT2 6         // 60 per Strip
#define LED_COUNT3 6         // 60 per Strip
#define BRIGHTNESS 255 * 0.2 // Set BRIGHTNESS to about 1/5 (max = 255)
#define SPEED 100            // 255 Slowest, 0 Fastest

// CAN bus settings
#define CAN_ID_FRONT 0x103   // CAN ID of front messages
#define CAN_MASK 0xF
#define CAN_FILTER 0x2

// Other constants
#define NUM_INPUTS 8
#define HEARTBEAT_INTERVAL 1000  // ms

#endif