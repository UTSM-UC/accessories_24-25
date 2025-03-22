#include <SPI.h>
#include <mcp_can.h>

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>  // Required for 16 MHz Adafruit Trinket
#endif

#define LED_COUNT 30          // 60 per Strip
#define LED_COUNT2 30         // 60 per Strip
#define BRIGHTNESS 255 * 0.2  // Set BRIGHTNESS to about 1/5 (max = 255)
#define SPEED 100             // 255 Slowest, 0 Fastest

int break_val = 30;

const int SPI_CS_PIN = 10;
MCP_CAN CAN0(SPI_CS_PIN);  // Set CS pin
const int buttonPin4 = 4;
const int buttonPin3 = 3;
const int buttonPin8 = 8;
const int MAX_PIN_NUM = 20; //D0 = 0, A5 = 19
const int NUM_INPUTS = 6;
const int led = 5;
const int CAN0_INT = 2;
int buttonStates[MAX_PIN_NUM] = { 0 };
int lastButtonStates[MAX_PIN_NUM] = { 0 };
int sendStates[MAX_PIN_NUM];
int pins_used[NUM_INPUTS] = {0, 1, 3, 16, 17, 19}; //d0, d1, d3, a2, a3, a5

//unsigned char data_m[NUM_INPUTS] = {0}; //array with data payloads for each corresponding digital input pin
// Steering
// 1. Left
// 2. Right
// 3. Headlight (not led)
// 4. Wiper (not led)
// 5. Horn (not led)
// 6. Hazard
// pin 13-9 not usable
// pin 2 is interrupt
// pin 3-8 we can use

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

uint8_t pin_to_CANmsg(int pin){
  int position;
  switch(pin){
    case 0: position = 1; break;
    case 1: position = 4; break;
    case 3: position = 0; break;
    case 16: position = 5; break; //A2
    case 17: position = 3; break; //A3
    case 19: position = 2; break; //A5
  }
  uint8_t CANmsg = (1 << position);
  return CANmsg;
}

void setup() {
  // pinMode(5, INPUT_PULLUP);
  // pinMode(buttonPin4, INPUT_PULLUP);  // Assuming button is active-low
  // pinMode(buttonPin3, INPUT_PULLUP);
  // pinMode(7, INPUT_PULLUP);
  // pinMode(8, INPUT_PULLUP);
  // pinMode(6, INPUT_PULLUP);
  for (int i = 0; i < MAX_PIN_NUM; i++) sendStates[i] = -1; //initialize to off
  for(int i = 0; i < NUM_INPUTS; i++) pinMode(pins_used[i], INPUT_PULLUP);

  //Serial.begin(115200);
  // Initialize the CAN bus at 500 kbps
  if (CAN0.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ) == CAN_OK) {
    //Serial.println("CAN Init OK!");
  } else {
    //Serial.println("CAN Init Failed!");
    while (1)
      ;  // init failed, stop the program
  }

  pinMode(CAN0_INT, INPUT);  // Configuring pin for /INT input

  //Serial.println("CAN bus ready to send and receive.");

  // Return to normal mode
  CAN0.setMode(MCP_NORMAL);
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

  if (CAN0.checkReceive() == CAN_MSGAVAIL || buf[0] & 0b0100) { 
    CAN0.readMsgBuf(&canId, 0, &len, buf);  // Read data
    masked_canId = masking(canId);

    // Serial.print("Received; CAN ID: ");
    // Serial.print(canId, HEX);
    // Serial.print(", Data: ");

    // for (int i = 0; i < len; i++) {
    //   Serial.print(buf[i], HEX);
    //   Serial.print(" ");
    //   Serial.println();
    // }
    
    /*
    bool headlight = (canMessage & (1 << 0));
    bool leftLightStrip = (canMessage & (1 << 1));
    bool rightLightStrip = (canMessage & (1 << 2));
    bool horn = (canMessage & (1 << 3));
    bool wiper = (canMessage & (1 << 4));
    bool hazard = (canMessage & (1 << 5));
    */
  }
  //  else {
  //   for (int i = 0; i < 8; i++) {  // clear the buffer
  //     buf[i] = 0;
  //   }
  }


// Function to send CAN message based on button state
void sendCANMessage() {
  unsigned long can_id1 = 0x102;  // CAN ID that does pass mask and filter
  // unsigned char data_m0[0] = { 0x000 };
  // unsigned char data_m1[1] = { 0x097 };
  // unsigned char data_m2[1] = { 0x098 };
  for (int j = 0; j < NUM_INPUTS; j++) {  // update states for all input buttons
    int i = pins_used[j];
    sendStates[i] = edgeDetector(i);      // 1 for on, 0 for off, -1 for unchanged

    if (sendStates[i] != -1) {
      // Serial.print("\nSent;     ");

      if (sendStates[i] == 1) {                   // If button is pressed
        canMessage |= (pin_to_CANmsg(i));                   // index 0
        CAN0.sendMsgBuf(can_id1, 0, 1, &canMessage);  
        // Serial.print("Can ID: ");
        // Serial.print(can_id1, HEX);
        // Serial.print(" Data: ");
        // Serial.print(canMessage, HEX);
        // Serial.println(" HIGH");
      } else {  // if button is unpressed
        canMessage &= ~(pin_to_CANmsg(i));        //111011 turn off bit corresponding to pin i
        CAN0.sendMsgBuf(can_id1, 0, 1, &canMessage);
        // Serial.print("Can ID: ");
        // Serial.print(can_id1, HEX);
        // Serial.print(" Data: ");
        // Serial.print(canMessage, HEX);      
      }
    }
  }
}

void loop() {
  // Check if a message is received
  readCANMessage();

  // Send CAN message based on the button state
  sendCANMessage();
  delay(100);  // Adjust the delay as needed to control the sending frequency
}
