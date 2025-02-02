#include <SPI.h>
#include <mcp_can.h>

const int SPI_CS_PIN = 10;
MCP_CAN CAN0(SPI_CS_PIN);  // Set CS pin
const int buttonPin4 = 4;
const int buttonPin3 = 3;
const int NUM_INPUTS = 8;
const int led = 5;
const int CAN0_INT = 2;
int buttonStates[NUM_INPUTS] = {0};
int lastButtonStates[NUM_INPUTS] = {0};
int sendStates[NUM_INPUTS] = {-1, -1, -1, -1, -1, -1, -1, -1};
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
unsigned char buf[8] = {0};
volatile bool messageReceived = false;  // Flag to indicate message reception

long unsigned int mask = 0xF;
// long unsigned int filter = 0x1;
long unsigned int filter = 0x2;

uint16_t masking(uint16_t message) {
    return message & mask;
}

void setup() {
  pinMode(led, OUTPUT);
  pinMode(buttonPin4, INPUT_PULLUP);  // Assuming button is active-low
  pinMode(buttonPin3, INPUT_PULLUP);
  Serial.begin(115200);
  // Initialize the CAN bus at 500 kbps
  if (CAN0.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ) == CAN_OK) {
    Serial.println("CAN Init OK!");
  } else {
    Serial.println("CAN Init Failed!");
    while (1);  // init failed, stop the program
  }

  pinMode(CAN0_INT, INPUT);  // Configuring pin for /INT input

  Serial.println("CAN bus ready to send and receive.");

  // Return to normal mode
  CAN0.setMode(MCP_NORMAL);
}

int edgeDetector (int buttonNum) {
  // read the pushbutton input pin:
  buttonStates[buttonNum] = digitalRead(buttonNum);

  // compare the buttonState to its previous state
  if (buttonStates[buttonNum] != lastButtonStates[buttonNum]) {
    // save the current state as the last state, for next time through the loop
    lastButtonStates[buttonNum] = buttonStates[buttonNum];
    if (buttonStates[buttonNum] == HIGH) {
      // if the current state is HIGH then the button went from off to on:
      return 0;
    } else{
      return 1;
    }
  }
  return -1;
}


void readCANMessage() {
  
  if (CAN0.checkReceive() == CAN_MSGAVAIL){
    CAN0.readMsgBuf(&canId, 0, &len, buf);   // Read data
    masked_canId = masking(canId);

    Serial.print("Received; CAN ID: ");
    Serial.print(canId, HEX);
    Serial.print(", Data: ");
    
    for (int i = 0; i < len; i++) {
      Serial.print(buf[i], HEX);
      Serial.print(" ");
      Serial.println();
    }

    if (masked_canId & filter) { // curr mask 0xF, filter 0x2, check whether last bit equals to 2

      if (buf[0] == 0x97) {
        digitalWrite(led, HIGH);  // Turn on LED if first byte > 0
        
      } else {
        digitalWrite(led, LOW);   // Turn off LED otherwise
        
      }
    }

  } else {
    for (int i = 0; i < 8; i++){  // clear the buffer
      buf[i] = 0;
    }

  }
}

// Function to send CAN message based on button state
void sendCANMessage() {
  unsigned long can_id1 = 0x102;  // CAN ID that does pass mask and filter
  unsigned char data_m0[1] = {0x000};
  unsigned char data_m1[1] = {0x097};
  unsigned char data_m2[1] = {0x098};
  for(int i = 0; i < NUM_INPUTS; i++){           // update states for all input buttons
    sendStates[i] = edgeDetector(i);            // 1 for on 0 for off -1 for unchanged

    if(i == 3 && sendStates[i] != -1){
      Serial.print("\nSent;     ");
  
      if (sendStates[i] == 1) {  // If button is pressed
        CAN0.sendMsgBuf(can_id1, 0, 1, data_m2);  // Send message 2
        Serial.print("Can ID: ");
        Serial.print(can_id1, HEX);
        Serial.print(" Data: ");
        Serial.print(data_m2[0], HEX);
        Serial.println(" HIGH");
      } else {    // if button is unpressed
        CAN0.sendMsgBuf(can_id1, 0, 1, data_m0);
        Serial.println("CAN ID: ___, Data: __, LOW");
      }
    }

    if(i == 4 && sendStates[i] != -1){
      Serial.print("Sent;     ");
  
      if (sendStates[i] == 1) {  // If button is pressed
        CAN0.sendMsgBuf(can_id1, 0, 1, data_m1);  // Send message 1
        Serial.print("Can ID: ");
        Serial.print(can_id1, HEX);
        Serial.print(" Data: ");
        Serial.print(data_m1[0], HEX);
        Serial.println(" HIGH");
      } else {    // if button is unpressed
        CAN0.sendMsgBuf(can_id1, 0, 1, data_m0);
        Serial.println("CAN ID: ___, Data: __, LOW");
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
