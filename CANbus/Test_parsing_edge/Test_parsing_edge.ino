#include <SPI.h>
#include <mcp_can.h>

const int SPI_CS_PIN = 10;
MCP_CAN CAN0(SPI_CS_PIN);  // Set CS pin
const int buttonPin = 4;
const int led = 5;
const int CAN0_INT = 2;
int buttonState = 0;
int lastButtonState = 0;

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
  pinMode(buttonPin, INPUT_PULLUP);  // Assuming button is active-low
  Serial.begin(115200);
  
  // Initialize the CAN bus at 500 kbps
  if (CAN0.begin(MCP_ANY, CAN_500KBPS, MCP_16MHZ) == CAN_OK) {
    Serial.println("CAN Init OK!");
  } else {
    Serial.println("CAN Init Failed!");
    while (1);  // init failed, stop the program
  }

  // Set CAN mode to normal operation
  // CAN0.setMode(MODE_CONFIG);

  pinMode(CAN0_INT, INPUT);  // Configuring pin for /INT input
  // attachInterrupt(digitalPinToInterrupt(CAN0_INT), CAN_ISR, FALLING);

  Serial.println("CAN bus ready to send and receive.");
  // Serial.println("ID  DLC   DATA");

  //CAN0.init_Mask(0, 0, 0x700); 
  //Mask should be all ones, care about all digits

  // CAN0.init_Mask(0, 0, 0xFFF);
  // CAN0.init_Filt(0, 0, 0xF00); 

  // Return to normal mode
  CAN0.setMode(MCP_NORMAL);

}

int edgeDetector () {
  // read the pushbutton input pin:
  buttonState = digitalRead(buttonPin);

  // compare the buttonState to its previous state
  if (buttonState != lastButtonState) {
    // save the current state as the last state, for next time through the loop
    lastButtonState = buttonState;
    if (buttonState == HIGH) {
      // if the current state is HIGH then the button went from off to on:
      return 0;
    } else{
      return 1;
    }
  }
  return -1;
}

void readCANMessage() {
  //while (1) {  // Check if data is available
  if (CAN0.checkReceive() == CAN_MSGAVAIL){
    CAN0.readMsgBuf(&canId, 0, &len, buf);   // Read data
    masked_canId = masking(canId);
    // Serial.println("Masked CAN ID: ");
    // Serial.print(masked_canId, HEX);

    Serial.print("\nReceived; CAN ID: ");
    Serial.print(canId, HEX);
    Serial.print(", Data: ");
    
    for (int i = 0; i < len; i++) {
      Serial.print(buf[i], HEX);
      Serial.print(" ");
    }
    // Serial.println();

    // for (int i = 0; i < len; i++) {  // Print the received data
    //   Serial.print(buf[i], DEC);
    //   Serial.print(" ");
    // }

    if (masked_canId & filter) {
      // Serial.println("The final bit is 1");
      if (buf[0] > 0) {
        digitalWrite(led, HIGH);  // Turn on LED if first byte > 0
        //Serial.println("HIGH");
        //Serial.println();
      } else {
        digitalWrite(led, LOW);   // Turn off LED otherwise
        //Serial.println("LOW");
        //Serial.println();
      }
    }
    // Serial.println("Message received");


  } else {
    for (int i = 0; i < 8; i++){  // clear the buffer
      buf[i] = 0;
    }
  
  }
}

// Function to send CAN message based on button state
void sendCANMessage() {
  // Prepare CAN messages
  // unsigned long can_id1 = 0x301;  // CAN ID for message 1
  unsigned long can_id1 = 0x102;  // CAN ID that does not pass mask and filter
  // unsigned long can_id1 = 0x103;
  unsigned char data_m1[1] = {0x097};  // Data payload for message 1
  unsigned char data_m0[1] = {0x000}; 
  // unsigned char data_m2[1] = {0x00};  // Data payload for message 2
  
  int state = edgeDetector(); // 1 for on 0 for off -1 for unchanged
  // Serial.print("state = ");
  // Serial.print(state);

  if(state != -1){
      Serial.print("Sent;     ");
  
      if (state == 1) {  // If button is pressed
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

void loop() {
  // Check if a message is received
  readCANMessage();
  
  // Send CAN message based on the button state
  sendCANMessage();
  
  delay(100);  // Adjust the delay as needed to control the sending frequency
}