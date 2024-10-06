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
unsigned char buf[8];
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
  CAN0.setMode(MCP_NORMAL);

  pinMode(CAN0_INT, INPUT);  // Configuring pin for /INT input
  // attachInterrupt(digitalPinToInterrupt(CAN0_INT), CAN_ISR, FALLING);

  Serial.println("CAN bus ready to send and receive.");
  Serial.println("ID  DLC   DATA");
}

void readCANMessage() {
  //while (1) {  // Check if data is available
    CAN0.readMsgBuf(&canId, &len, buf);   // Read data

    Serial.print("Received CAN ID: ");
    Serial.print(canId, HEX);
    Serial.print(", Data: ");
    
    for (int i = 0; i < len; i++) {
      Serial.print(buf[i], DEC);
      Serial.print(" ");
    }
    Serial.println();

    for (int i = 0; i < len; i++) {  // Print the received data
      Serial.print(buf[i], DEC);
      Serial.print(" ");
    }

    if (buf[0] > 0) {
      digitalWrite(led, HIGH);  // Turn on LED if first byte > 0
      //Serial.println("HIGH");
      //Serial.println();
    } else {
      digitalWrite(led, LOW);   // Turn off LED otherwise
      //Serial.println("LOW");
      //Serial.println();
    }
    Serial.println("Message received");
  //}
}

// Function to send CAN message based on button state
void sendCANMessage() {
  // Prepare CAN messages
  unsigned long can_id1 = 0x0F6;  // CAN ID for message 1
  unsigned long can_id2 = 0x036;  // CAN ID for message 2
  unsigned char data_m1[1] = {0x8E};  // Data payload for message 1
  unsigned char data_m2[1] = {0x00};  // Data payload for message 2
  
  buttonState = digitalRead(buttonPin);  // Read the button state
  
  if (buttonState == LOW) {  // If button is pressed
    CAN0.sendMsgBuf(can_id1, 0, 1, data_m1);  // Send message 1
    Serial.println("Sent: HIGH signal");
  } else {  // If button is not pressed
    CAN0.sendMsgBuf(can_id2, 0, 1, data_m2);  // Send message 2
    Serial.println("Sent: LOW signal");
  }
}

void loop() {
  // Check if a message is received
  readCANMessage();
  
  // Send CAN message based on the button state
  sendCANMessage();
  
  delay(100);  // Adjust the delay as needed to control the sending frequency
}

