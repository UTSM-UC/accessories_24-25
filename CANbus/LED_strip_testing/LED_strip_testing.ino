#include <SPI.h>
#include <mcp_can.h>

#include <Adafruit_NeoPixel.h>
//#include <Servo.h> //Include the servo library

#ifdef __AVR__
#include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

#define LED_PIN     6 // controls breaks = strip
#define LED2_PIN    5 // controls blinker/hazard
#define LED3_PIN     9 // controls breaks = strip
#define ENABLE1_PIN  A0 // bliker light 1 --> right = blue (strip2)
#define ENABLE2_PIN  A1 // bliker light 2 --> left = green (strip3?)
#define ENABLE3_PIN  A2 // overwrites A0 and A1  --> hazards = orange (strip2)
#define ENABLE4_PIN  2 // controls breaklights --> only strip
#define LED_COUNT  60 // 60 per Strip
#define LED_COUNT2  15 // 60 per Strip
#define BRIGHTNESS 255*0.2 // Set BRIGHTNESS to about 1/5 (max = 255)
#define SPEED 100 // 255 Slowest, 0 Fastest
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip2(LED_COUNT2, LED2_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip3(LED_COUNT2, LED3_PIN, NEO_GRB + NEO_KHZ800);



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

  //LED code
  #if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
  #endif
  pinMode(ENABLE1_PIN, INPUT_PULLUP);   // internal pull-up resistor
  pinMode(ENABLE2_PIN, INPUT_PULLUP);   // internal pull-up resistor
  pinMode(ENABLE3_PIN, INPUT_PULLUP);   // internal pull-up resistor
  pinMode(ENABLE4_PIN, INPUT_PULLUP);   // internal pull-up resistor
    // interrupts for breaklights
  attachInterrupt(digitalPinToInterrupt(ENABLE4_PIN), break_on, CHANGE);

  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(BRIGHTNESS);
  strip2.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip2.show();            // Turn OFF all pixels ASAP
  strip2.setBrightness(BRIGHTNESS);
  strip3.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip3.show();            // Turn OFF all pixels ASAP
  strip3.setBrightness(BRIGHTNESS);

  //missing wipers code

  delay(2000);

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

    if (buf[0] > 0) { //received message to turn on light
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

  strip2.clear();
  strip3.clear();

  if(digitalRead(ENABLE3_PIN) == LOW){   //hazards OFF
    colorWipe3(strip2.Color(255,   30,   0), SPEED);  //red
    colorWipe3(strip3.Color(255,   30,   0), SPEED);  //red
  }
  else if(digitalRead(ENABLE3_PIN) == HIGH){   //hazards ON
    if (digitalRead(ENABLE1_PIN) == LOW){ // bliker light 1 --> right = blue (strip2)
      colorWipe(strip2.Color(255,   30,   0), SPEED);
    }
    else if(digitalRead(ENABLE2_PIN) == LOW){// bliker light 2 --> left = green (strip3?)
      colorWipe2(strip3.Color(255,   30,   0), SPEED);
    }
  }
  strip2.show();
  strip3.show();
  
}

//ENABLE4_PIN: brakelights (strip)

/*RGB colors:
orange: (255, 65, 0)
green: (30, 65, 0)
red: (255,30,0)
*/

//ONLY strip is controlled, interrupt for brakelights
void break_on(void){  
  if (digitalRead(ENABLE4_PIN) == HIGH) {
    break_val = 30;  //set to green
  }
  else{
    break_val = 255;  //set to orange
  }
  for(int i = 0; i < LED_COUNT; i++){
        strip.setPixelColor(i, break_val, 0, 0);
        //strip2.setPixelColor(i, break_val, 0, 0);
  }
  strip.show();
}

void break_off(void){ //function never used?
  break_val = 10;
  for(int i = 0; i < LED_COUNT; i++){
        strip.setPixelColor(i, break_val, 0, 0);  //black
        //strip2.setPixelColor(i, break_val, 0, 0);
  }
  strip.show();
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

void colorWipe(uint32_t color, int wait) {
    for(int i = 0; (digitalRead(ENABLE1_PIN) == LOW || digitalRead(ENABLE2_PIN) == LOW) && digitalRead(ENABLE3_PIN) == HIGH; i++) { // For each pixel in strip...
 
    wipers();
      strip2.setPixelColor(i, color);         //  Set pixel's color (in RAM)
      strip2.show();
      if(i == LED_COUNT2){
        // erase the colors set
        for(int j = 0; j < LED_COUNT2; j++){
          strip2.setPixelColor(j, strip2.Color(0,0,0));
          // add code for breaklights
        //attachInterrupt(digitalPinToInterrupt(ENABLE4_PIN), break_on, FALLING);
        //attachInterrupt(digitalPinToInterrupt(ENABLE4_PIN), break_off, RISING;
        }
        i = -1;
      }
      delay(wait);                           //  Pause for a moment
    };
  return;
}
void colorWipe2(uint32_t color, int wait) {
  for(int i = 0; (digitalRead(ENABLE1_PIN) == LOW || digitalRead(ENABLE2_PIN) == LOW) && digitalRead(ENABLE3_PIN) == HIGH; i++) { // For each pixel in strip...

    wipers();
    strip3.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip3.show();
      if(i == LED_COUNT2){
        // erase the colors set
        for(int j = 0; j < LED_COUNT2; j++){
          strip3.setPixelColor(j, strip3.Color(0,0,0));
        }
        i = -1;
      }
    delay(wait);                           //  Pause for a moment
  };
  return;
}
void colorWipe3(uint32_t color, int wait) {
  for(int i = 0; digitalRead(ENABLE3_PIN) == LOW; i++) { // For each pixel in strip...
 
    wipers();
    strip2.setPixelColor(i, color);
    strip2.show();
    strip3.setPixelColor(i, color);
    strip3.show();
    if(i == LED_COUNT2){
        // erase the colors set
        for(int j = 0; j < LED_COUNT2; j++){
          strip2.setPixelColor(j, strip2.Color(0,0,0));
          strip3.setPixelColor(j, strip2.Color(0,0,0));
        }
        i = -1;
      }
    delay(wait);                           //  Pause for a moment
  };
  return;
}

