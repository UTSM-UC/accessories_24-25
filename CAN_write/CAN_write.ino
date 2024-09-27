#include <SPI.h>
// #include <mcp2515.h>
#include <mcp_can.h>

// struct can_frame canMsg1;
// struct can_frame canMsg2;
MCP_CAN CAN0(10); //set SPI Chip Select to pin 10
const int buttonPin = 4;
const int led = 5;
int buttonState = 0;

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(led, OUTPUT);

  Serial.begin(115200);

  // Initialize MCP2515 running at 16MHz with a baudrate of 500kb/s and the masks and filters disabled.
  if(CAN0.begin(MCP_ANY, CAN_500KBPS, MCP_16MHZ) == CAN_OK) Serial.println("MCP2515 Initialized Successfully!");
  else Serial.println("Error Initializing MCP2515...");

  CAN0.setMode(MCP_NORMAL);
  /*
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(led, OUTPUT);
  canMsg1.can_id  = 0x0F6;
  canMsg1.can_dlc = 1; //specifies number of bytes in CAN Data Field
  canMsg1.data[0] = 0x8E; // data contained in each byte
  
  canMsg2.can_id  = 0x036;
  canMsg2.can_dlc = 1; //specifies number of bytes in CAN Data Field
  canMsg2.data[0] = 0x0; // data contained in each byte

  while (!Serial);
  Serial.begin(115200);
  
  mcp2515.reset();
  mcp2515.setBitrate(CAN_125KBPS);
  mcp2515.setNormalMode();

  Serial.println("Example: Write to CAN");
  */
  
}

//byte data[8] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};

void loop() { 
  // Message 1
  unsigned long int can_id1 = 0x0F6;  // CAN ID (11-bit standard)
  unsigned char can_dlc = 1;              // Data length (1 byte)
  unsigned char data_m1[1] = {0x8E};         // Data payload (one byte)

  // Message 2
  unsigned long can_id2 = 0x036;  // CAN ID (11-bit standard)
  unsigned char data_m2[1] = {0x0};         // Data payload (one byte)

  buttonState = digitalRead(buttonPin);
  if(buttonState == LOW) {
    digitalWrite(led, HIGH);
    byte sndStat1 = CAN0.sendMsgBuf(can_id1, 0, can_dlc, data_m1);  // 0 means standard 11-bit ID
    Serial.println("HIGH");
    Serial.println();
  } else {
    digitalWrite(led, LOW);
    byte sndStat2 = CAN0.sendMsgBuf(can_id2, 0, can_dlc, data_m2);  // 0 means standard 11-bit ID
    Serial.println("LOW");
    Serial.println();
  }
  Serial.println("Messages sent");
  Serial.println();
  delay(10);
}
