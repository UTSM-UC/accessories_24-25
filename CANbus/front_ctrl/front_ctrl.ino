#include <SPI.h>
#include <mcp_can.h>

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>  // Required for 16 MHz Adafruit Trinket
#endif

#define LED2_PIN 8
#define LED3_PIN 6
#define ENABLE3_PIN 7

#define LED_COUNT 6         // 60 per Strip
#define LED_COUNT2 6         // 60 per Strip
#define LED_COUNT3 6         // 60 per Strip
#define BRIGHTNESS 255 * 0.2  // Set BRIGHTNESS to about 1/5 (max = 255)
#define SPEED 100             // 255 Slowest, 0 Fastest
Adafruit_NeoPixel strip2(LED_COUNT2, LED2_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip3(LED_COUNT3, LED3_PIN, NEO_GRB + NEO_KHZ800);

int break_val = 30;

const int SPI_CS_PIN = 10;
MCP_CAN CAN0(SPI_CS_PIN);  // Set CS pin
const int buttonPin4 = 4;
const int buttonPin3 = 3;
const int NUM_INPUTS = 8; 
const int headlightPin = 5;
const int hornPin = 3;
const int CAN0_INT = 2;
int buttonStates[NUM_INPUTS] = { 0 };
int lastButtonStates[NUM_INPUTS] = { 0 };
int sendStates[NUM_INPUTS] = { -1, -1, -1, -1, -1, -1, -1, -1 };
//unsigned char data_m[NUM_INPUTS] = {0}; //array with data payloads for each corresponding digital input pin

// Front (read)
// 1. 2 x headlights (same button)
// 2. left turn light 
// 3. right turn light
// 4. horn
// 5. wiper
// 6. hazard

// Front (send)
// 1. brake (button 4)

// received data
long unsigned int canId;
long unsigned int masked_canId;
unsigned char len = 0;
unsigned char buf[8] = { 0 };
uint8_t canMessage = 0;
volatile bool messageReceived = false;  // Flag to indicate message reception

long unsigned int mask = 0xF;
// long unsigned int filter = 0x1;
long unsigned int filter = 0x2;

uint16_t masking(uint16_t message) {
  return message & mask;
}

void setup() {
  pinMode(ENABLE3_PIN, INPUT_PULLUP);   // internal pull-up resistor
  pinMode(headlightPin, OUTPUT);
  pinMode(hornPin, OUTPUT);
  pinMode(buttonPin4, INPUT_PULLUP);  // Assuming button is active-low
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
  delay(2000);
}

int edgeDetector(int buttonNum) {
  // read the pushbutton input pin:
  buttonStates[buttonNum] = digitalRead(buttonNum);

  // compare the buttonState to its previous state
  if (buttonStates[buttonNum] != lastButtonStates[buttonNum]) {
    // save the current state as the last state, for next time through the loop
    lastButtonStates[buttonNum] = buttonStates[buttonNum];
    if (buttonStates[buttonNum] == HIGH) {
      // if the current state is HIGH then the button went from off to on:
      return 0;
    } else {
      return 1;
    }
  }
  return -1;
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
    bool headlight = (buf[0] & (1 << 2));
    bool horn = (buf[0] & (1 << 3));
    bool hazard = (buf[0] & (1 << 4));
    bool wiper = (buf[0] & (1 << 5));

    if (masked_canId & filter) {  // curr mask 0xF, filter 0x2, check whether last bit equals to 2
      if (hazard) {
        hazards(strip2.Color(255, 30, 0), SPEED);
      } else if (leftLightStrip) {
        orangeBlinker2(strip2.Color(255, 30, 0), SPEED);
      } else if(rightLightStrip){
        orangeBlinker3(strip3.Color(255, 30, 0), SPEED);  // Turn on LED if first byte > 0
      }
      if(headlight) {
        digitalWrite(headlightPin, HIGH);
      } else {
        digitalWrite(headlightPin, LOW);
      }
      if(horn) {
        digitalWrite(hornPin, HIGH);
      } else digitalWrite(hornPin, LOW);
    }
  }
  //  else {
  //   for (int i = 0; i < 8; i++) {  // clear the buffer
  //     buf[i] = 0;
  //   }
  }


// Function to send CAN message based on button state
void sendCANMessage() {
  unsigned long can_id1 = 0x103;  // CAN ID of front messages

  sendStates[4] = edgeDetector(4);      // 1 for on 0 for off -1 for unchanged

  if (sendStates[4] != -1) {
    Serial.print("\nSent;     ");

    if (sendStates[4] == 1) {                   // If button is pressed
      canMessage = buf[0];
      canMessage |= (1<<2);
      CAN0.sendMsgBuf(can_id1, 0, 1, &canMessage);  
      Serial.print("Can ID: ");
      Serial.print(can_id1, HEX);
      Serial.print(" Data: ");
      Serial.print(canMessage, HEX);
      Serial.println(" HIGH");
    } else {  // if button is unpressed
      canMessage = buf[0];
      canMessage &= ~(1 << 2); //11011 turn off 2nd bit in canMessage
      CAN0.sendMsgBuf(can_id1, 0, 1, &canMessage);
      Serial.print("Can ID: ");
      Serial.print(can_id1, HEX);
      Serial.print(" Data: ");
      Serial.print(canMessage, HEX);  
    }
  }
}

void loop() {
  running_lights();
  // Check if a message is received
  readCANMessage();

  // Send CAN message based on the button state
  sendCANMessage();
  strip2.show();

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
  for(int i = 0; i < LED_COUNT; i++){
        strip2.setPixelColor(i, 255, 0, 0);
  }
  strip2.show();
}

void running_lights(void){
  break_val = 20;
  for(int i = 0; i < LED_COUNT; i++){
        strip2.setPixelColor(i, break_val, 0, 0);
        strip3.setPixelColor(i, break_val, 0, 0);
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
