#include <SPI.h>
#include <mcp_can.h>

const int SPI_CS_PIN = 10;
MCP_CAN CAN0(SPI_CS_PIN);  // Set CS pin
const int buttonPin = 4;
const int led = 5;
const int CAN0_INT = 2;
int buttonState = 0;

// received data
long unsigned int canId;
unsigned char len = 0;
unsigned char buf[8] = {0};
volatile bool messageReceived = false;  // Flag to indicate message reception

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
  Serial.println("ID  DLC   DATA");

  //CAN0.init_Mask(0, 0, 0x700); 
  //Mask should be all ones, care about all digits

  CAN0.init_Mask(0, 0, 0xFFF);

  CAN0.init_Filt(0, 0, 0xF00); 
  // Return to normal mode
  CAN0.setMode(MCP_NORMAL);

}

void readCANMessage() {
  //while (1) {  // Check if data is available
  if (CAN0.checkReceive() == CAN_MSGAVAIL){
    CAN0.readMsgBuf(&canId, 0, &len, buf);   // Read data
  } else {
    for (int i = 0; i < 8; i++){  // clear the buffer
      buf[i] = 0;
    }
  }
  
    Serial.print("Received; CAN ID: ");
    Serial.print(canId, HEX);
    Serial.print(", Data: ");
    
    for (int i = 0; i < len; i++) {
      Serial.print(buf[i], HEX);
      Serial.print(" ");
    }
    Serial.println();

    // for (int i = 0; i < len; i++) {  // Print the received data
    //   Serial.print(buf[i], DEC);
    //   Serial.print(" ");
    // }

    if (buf[0] > 0) {
      digitalWrite(led, HIGH);  // Turn on LED if first byte > 0
      //Serial.println("HIGH");
      //Serial.println();
    } else {
      digitalWrite(led, LOW);   // Turn off LED otherwise
      //Serial.println("LOW");
      //Serial.println();
    }
    // Serial.println("Message received");
  //}
}

// Function to send CAN message based on button state
void sendCANMessage() {
  // Prepare CAN messages
  unsigned long can_id1 = 0x102;  // CAN ID for message 1
  // unsigned long can_id2 = 0x036;  // CAN ID for message 2
  unsigned char data_m1[1] = {0x097};  // Data payload for message 1
  // unsigned char data_m2[1] = {0x00};  // Data payload for message 2
  
  buttonState = digitalRead(buttonPin);  // Read the button state

  Serial.print("Sent;     ");
  
  if (buttonState == LOW) {  // If button is pressed
    CAN0.sendMsgBuf(can_id1, 0, 1, data_m1);  // Send message 1
    Serial.print("Can ID: ");
    Serial.print(can_id1, HEX);
    Serial.print(" Data: ");
    Serial.print(data_m1[0], HEX);
    Serial.println(" HIGH");
  } else {
    Serial.println("CAN ID: ___, Data: __, LOW");
  }
}

void loop() {
  // Check if a message is received
  readCANMessage();
  
  // Send CAN message based on the button state
  sendCANMessage();
  
  delay(100);  // Adjust the delay as needed to control the sending frequency
}
