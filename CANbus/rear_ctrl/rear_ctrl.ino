#include <SPI.h>
#include <mcp_can.h>

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>  // Required for 16 MHz Adafruit Trinket
#endif

#define LED2_PIN 8
#define LED3_PIN 6
#define LEDBRAKES_PIN 7

#define LED_COUNT 6         // 6 per Strip
#define LED_COUNT2 6         // 6 per Strip
#define LED_COUNT3 6         // 6 per Strip
#define LED_COUNT_BRAKES 12
#define BRIGHTNESS 255 * 0.2  // Set BRIGHTNESS to about 1/5 (max = 255)
#define SPEED 100             // 255 Slowest, 0 Fastest
Adafruit_NeoPixel strip2(LED_COUNT2, LED2_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip3(LED_COUNT3, LED3_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip4(LED_COUNT_BRAKES, LEDBRAKES_PIN, NEO_GRB + NEO_KHZ800);

const int SPI_CS_PIN = 10;
MCP_CAN CAN0(SPI_CS_PIN);  // Set CS pin
const int CAN0_INT = 2;
//unsigned char data_m[NUM_INPUTS] = {0}; //array with data payloads for each corresponding digital input pin

// Rear (read)
// 1. 2 x headlights (same button)
// 2. left turn light 
// 3. right turn light
// 4. horn
// 5. wiper
// 6. hazard

// Rear (send)
// 1. brake (button 3)

// received data
long unsigned int canId;
long unsigned int masked_canId;
unsigned char len = 0;
unsigned char buf[8] = { 0 };
volatile bool messageReceived = false;  // Flag to indicate message reception

long unsigned int mask = 0xF;
// long unsigned int filter = 0x1;
long unsigned int filter = 0x3; //CAN_ID of front controller is 0x103

uint16_t masking(uint16_t message) {
  return message & mask;
}

void setup() {

  Serial.begin(115200);
  // Initialize the CAN bus at 500 kbps
  if (CAN0.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ) == CAN_OK) {
    Serial.println("CAN Init OK!");
  } else {
    Serial.println("CAN Init Failed!");
    while (1)
      ;  // init failed, stop the program
  }

  pinMode(CAN0_INT, INPUT);  // Configuring pin for /INT input

  Serial.println("CAN bus ready to send and receive.");

  // Return to normal mode
  CAN0.setMode(MCP_NORMAL);

  strip2.begin();  // INITIALIZE NeoPixel strip object (REQUIRED)
  strip2.show();   // Turn OFF all pixels ASAP
  strip2.setBrightness(BRIGHTNESS);

  strip3.begin();  // INITIALIZE NeoPixel strip object (REQUIRED)
  strip3.show();   // Turn OFF all pixels ASAP
  strip3.setBrightness(BRIGHTNESS);

  strip4.begin();  // INITIALIZE NeoPixel strip object (REQUIRED)
  strip4.show();   // Turn OFF all pixels ASAP
  strip4.setBrightness(BRIGHTNESS);
  delay(2000);
}

void readCANMessage() {
  bool received = CAN0.checkReceive() == CAN_MSGAVAIL;
  if (received || buf[0] != 0) {
    CAN0.readMsgBuf(&canId, 0, &len, buf);  // Read data
    masked_canId = masking(canId);

    if(received) {
      Serial.print("Received; CAN ID: ");
      Serial.print(canId, HEX);
      Serial.print(", Data: ");
      for (int i = 0; i < len; i++) {
        Serial.print(buf[i], HEX);
        Serial.print(" ");
        Serial.println();
      }
    }

    bool rightLightStrip = (buf[0] & (1 << 0));
    bool leftLightStrip = (buf[0] & (1 << 1));
    bool brakes = (buf[0] & (1 << 2));
    bool hazard = (buf[0] & (1 << 4));

    if (masked_canId == filter) {  // curr mask 0xF, filter 0x3, check whether last bit equals to 3 - means this message is from the front, hit the brakes.
      if(brakes)
        brake_on();
      else
        brake_off();
    }

    if (masked_canId == 0x2 || masked_canId == 0x3) {  // curr mask 0xF, filter 0x2, check whether last bit equals to 2
      if (hazard) {
        hazards(strip2.Color(255, 30, 0), SPEED);
      } else if (leftLightStrip) {
        orangeBlinker2(strip2.Color(255, 30, 0), SPEED);
      } else if(rightLightStrip){
        orangeBlinker3(strip3.Color(255, 30, 0), SPEED);  // Turn on LED if first byte > 0
      }
    }
  }
}


// Function to send CAN message based on button state

void loop() {
  running_lights();
  // Check if a message is received
  readCANMessage();
  delay(100);  // Adjust the delay as needed to control the sending frequency
}

void orangeBlinker2(uint32_t color, int wait) {  //colorWipe3
  strip2.clear();
  for (int i = 0; i < LED_COUNT2; i++) {  // For each pixel in strip...
    
    strip2.setPixelColor(i, color);
    strip2.show();
    if (i == LED_COUNT2) {
      // erase the colors set
      for (int j = 0; j < LED_COUNT2; j++) {
        strip2.setPixelColor(j, strip2.Color(0, 0, 0));
      }
      //i = -1;
    }
    delay(wait);  //  Pause for a moment
  };
  return;
}

void orangeBlinker3(uint32_t color, int wait) {  //colorWipe3
  strip3.clear();
  for (int i = 0; i < LED_COUNT2; i++) {  // For each pixel in strip...
    
    strip3.setPixelColor(i, color);
    strip3.show();
    if (i == LED_COUNT2) {
      // erase the colors set
      for (int j = 0; j < LED_COUNT2; j++) {
        strip3.setPixelColor(j, strip3.Color(0, 0, 0));
      }
      //i = -1;
    }
    delay(wait);  //  Pause for a moment
  };
  return;
}

void brake_on(void){
  for(int i = 0; i < LED_COUNT_BRAKES; i++){
    strip4.setPixelColor(i, 255, 0, 0);
  }
  strip4.show();
}

void brake_off(void){
  for(int i = 0; i < LED_COUNT_BRAKES; i++){
    strip4.setPixelColor(i, 20, 0, 0);
  }
  strip4.show();
}

void running_lights(void){
  for(int i = 0; i < LED_COUNT; i++){
        strip2.setPixelColor(i, 20, 0, 0);
        strip3.setPixelColor(i, 20, 0, 0);
  }
  strip2.show();
  strip3.show();
}

void hazards(uint32_t color, int wait) {  //colorWipe strip 2 and 3 concurrently
  strip3.clear();
  strip2.clear();
  for (int i = 0; i < LED_COUNT2; i++) {  // For each pixel in strip...
    strip3.setPixelColor(i, color);
    strip3.show();
    strip2.setPixelColor(i, color);
    strip2.show();
    if (i == LED_COUNT2) {
      // erase the colors set
      for (int j = 0; j < LED_COUNT2; j++) {
        strip3.setPixelColor(j, strip3.Color(0, 0, 0));
        strip2.setPixelColor(j, strip3.Color(0, 0, 0));
      }
      //i = -1;
    }
    delay(wait);  //  Pause for a moment
  };
  return;
}
