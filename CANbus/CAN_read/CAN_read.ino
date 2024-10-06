#include <SPI.h>
#include <mcp_can.h>

const int SPI_CS_PIN = 10;
MCP_CAN CAN0(SPI_CS_PIN);  // Set CS pin
const int led = 5;
const int CAN0_INT = 2;

// received data
long unsigned int canId;
unsigned char len = 0;
unsigned char buf[8];
char msgString[128];                        // Array to store serial string
volatile bool messageReceived = false;  // Flag to indicate message reception
unsigned char on;

// Interrupt Service Routine (ISR) for CAN0_INT pin
void CAN_ISR() {
  // Read the received CAN message inside ISR
  CAN0.readMsgBuf(&canId, &len, buf);

  // Check if the current message is different from the previous one
  if (buf[0] != on) {
    on = buf[0];        // Update the previous state to the current one
    messageReceived = true;       // Set flag indicating that state has changed
  }
}

void setup() {
  pinMode(led, OUTPUT);
  Serial.begin(115200);
  
  if (CAN0.begin(MCP_ANY, CAN_500KBPS, MCP_16MHZ) == CAN_OK) {
    Serial.println("CAN Init OK!");
  } else {
    Serial.println("CAN Init Failed!");
    while (1);  // init failed, stop the program
  }

  // attachInterrupt(digitalPinToInterrupt(CAN_INT_PIN), CAN_ISR, FALLING);
  // Enable CAN interrupt on message reception

  CAN0.setMode(MCP_NORMAL);                     
  // Set operation mode to normal so the MCP2515 sends acks to received data.
  pinMode(CAN0_INT, INPUT);                            // Configuring pin for /INT input
  attachInterrupt(digitalPinToInterrupt(CAN0_INT), CAN_ISR, FALLING);

  Serial.println("------- CAN Read ----------");
  Serial.println("ID  DLC   DATA");
}

void loop() {
  if (messageReceived) {  // Check if data is available
    messageReceived = false;
    // CAN0.readMsgBuf(&canId, &len, buf);   // Read data
    
    Serial.print(canId, DEC);  // Print CAN ID
    Serial.print(" ");
    Serial.print(len, DEC);    // Print Data Length Code (DLC)
    Serial.print(" ");

    for (int i = 0; i < len; i++) {  // Print the received data
      Serial.print(buf[i], DEC);
      Serial.print(" ");
    }

    if (buf[0] > 0) {
      on = 1;
      digitalWrite(led, HIGH);  // Turn on LED if first byte > 0
      Serial.println("HIGH");
      Serial.println();
    } else {
      on = 0;
      digitalWrite(led, LOW);   // Turn off LED otherwise
      Serial.println("LOW");
      Serial.println();
    }
    Serial.println("Message received");
  }
  /*
  Serial.print("INT value: ");
  Serial.print(digitalRead(CAN0_INT));
  Serial.print(" on: ");
  Serial.println(on);
  */

}
