#include <SPI.h>
#include <mcp_can.h>

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>  // Required for 16 MHz Adafruit Trinket
#endif

#define LED2_PIN 8
#define ENABLE3_PIN 7



#define LED_COUNT 30          // 60 per Strip
#define LED_COUNT2 30         // 60 per Strip
#define BRIGHTNESS 255 * 0.2  // Set BRIGHTNESS to about 1/5 (max = 255)
#define SPEED 100             // 255 Slowest, 0 Fastest
Adafruit_NeoPixel strip2(LED_COUNT2, LED2_PIN, NEO_GRB + NEO_KHZ800);

int break_val = 30;

const int SPI_CS_PIN = 10;
MCP_CAN CAN0(SPI_CS_PIN);  // Set CS pin
const int buttonPin4 = 4;
const int buttonPin3 = 3;
const int NUM_INPUTS = 8;
const int led = 5;
const int CAN0_INT = 2;
int buttonStates[NUM_INPUTS] = { 0 };
int lastButtonStates[NUM_INPUTS] = { 0 };
int sendStates[NUM_INPUTS] = { -1, -1, -1, -1, -1, -1, -1, -1 };
//unsigned char data_m[NUM_INPUTS] = {0}; //array with data payloads for each corresponding digital input pin
// Steering
// 1. Hazard
// 2. Left
// 3. Right
// 4. Headlight (not led)
// 5. Wiper (not led)
// 6. Horn (not led)
// pin 13-9 not usable
// pin 2-8 we can use


// received data
long unsigned int canId;
long unsigned int masked_canId;
unsigned char len = 0;
unsigned char buf[8] = { 0 };
volatile bool messageReceived = false;  // Flag to indicate message reception

long unsigned int mask = 0xF;
// long unsigned int filter = 0x1;
long unsigned int filter = 0x2;

uint16_t masking(uint16_t message) {
  return message & mask;
}

void setup() {
  pinMode(ENABLE3_PIN, INPUT_PULLUP);   // internal pull-up resistor
  pinMode(led, OUTPUT);
  pinMode(buttonPin4, INPUT_PULLUP);  // Assuming button is active-low
  pinMode(buttonPin3, INPUT_PULLUP);
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

  if (CAN0.checkReceive() == CAN_MSGAVAIL || buf[0] == 0x97 || buf[0] == 0x98) {
    CAN0.readMsgBuf(&canId, 0, &len, buf);  // Read data
    masked_canId = masking(canId);

    Serial.print("Received; CAN ID: ");
    Serial.print(canId, HEX);
    Serial.print(", Data: ");

    for (int i = 0; i < len; i++) {
      Serial.print(buf[i], HEX);
      Serial.print(" ");
      Serial.println();
    }

    if (masked_canId & filter) {  // curr mask 0xF, filter 0x2, check whether last bit equals to 2

      if (buf[0] == 0x97) {
        orangeBlinker(strip2.Color(255, 30, 0), SPEED);
      } 
      else if(buf[0] == 0x98){
         brake_on();  // Turn on LED if first byte > 0
      }  
      else {
        digitalWrite(led, LOW);  // Turn off LED otherwise
      }
    }
  }
  //  else {
  //   for (int i = 0; i < 8; i++) {  // clear the buffer
  //     buf[i] = 0;
  //   }
  }


// Function to send CAN message based on button state
void sendCANMessage() {
  unsigned long can_id1 = 0x102;  // CAN ID that does pass mask and filter
  unsigned char data_m0[1] = { 0x000 };
  unsigned char data_m1[1] = { 0x097 };
  unsigned char data_m2[1] = { 0x098 };
  for (int i = 0; i < NUM_INPUTS; i++) {  // update states for all input buttons
    sendStates[i] = edgeDetector(i);      // 1 for on 0 for off -1 for unchanged

    if (i == 3 && sendStates[i] != -1) {
      Serial.print("\nSent;     ");

      if (sendStates[i] == 1) {                   // If button is pressed
        CAN0.sendMsgBuf(can_id1, 0, 1, data_m2);  // Send message 2
        Serial.print("Can ID: ");
        Serial.print(can_id1, HEX);
        Serial.print(" Data: ");
        Serial.print(data_m2[0], HEX);
        Serial.println(" HIGH");
      } else {  // if button is unpressed
        CAN0.sendMsgBuf(can_id1, 0, 1, data_m0);
        Serial.println("CAN ID: ___, Data: __, LOW");
      }
    }

    if (i == 4 && sendStates[i] != -1) {
      Serial.print("Sent;     ");

      if (sendStates[i] == 1) {                   // If button is pressed
        CAN0.sendMsgBuf(can_id1, 0, 1, data_m1);  // Send message 1
        Serial.print("Can ID: ");
        Serial.print(can_id1, HEX);
        Serial.print(" Data: ");
        Serial.print(data_m1[0], HEX);
        Serial.println(" HIGH");
      } else {  // if button is unpressed
        CAN0.sendMsgBuf(can_id1, 0, 1, data_m0);
        Serial.println("CAN ID: ___, Data: __, LOW");
      }
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

void orangeBlinker(uint32_t color, int wait) {  //colorWipe3
  strip2.clear();
  for (int i = 0; i < 30; i++) {  // For each pixel in strip...
    
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

void brake_on(void){
  for(int i = 0; i < LED_COUNT; i++){
        strip2.setPixelColor(i, 255, 0, 0);
  }
  strip2.show();
}

void running_lights(void){
  break_val = 40;
  for(int i = 0; i < LED_COUNT; i++){
        strip2.setPixelColor(i, break_val, 0, 0);
  }
  strip2.show();
}
