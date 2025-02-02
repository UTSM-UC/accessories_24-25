#include <Adafruit_NeoPixel.h>
// Include the Servo library
#include <Servo.h>
// LIGHT CONNECTIONS
// red = power
// white = data
// black = ground
#ifdef __AVR__
#include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

#define LED_PIN     6 // controls breaks = strip
#define LED2_PIN    8 //5 // controls blinker/hazard
#define LED3_PIN     9 // controls breaks = strip
#define ENABLE1_PIN  2 // A0 // bliker light 1 --> right = blue (strip2)
#define ENABLE2_PIN  3 // A1 // bliker light 2 --> left = green (strip2)
#define ENABLE3_PIN  7 // A2 // overwrites A0 and A1  --> hazards = orange (strip2)
#define ENABLE4_PIN  2 // controls breaklights --> only strip
#define LED_COUNT  24 // 60 per Strip
#define LED_COUNT2  24 // 60 per Strip
#define BRIGHTNESS 255*0.2 // Set BRIGHTNESS to about 1/5 (max = 255)
#define SPEED 100 // 255 Slowest, 0 Fastest
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip2(LED_COUNT2, LED2_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip3(LED_COUNT2, LED3_PIN, NEO_GRB + NEO_KHZ800);

int break_val = 30;
// Include the Servo library
#include <Servo.h>
// Declare the Servo pin
int servoPin = 4;
int buttonPin = 3;
// Create a servo object
Servo servo;
const long wiperInterval = 2500;
unsigned long previousMillis = 0;
unsigned long wiperStart = 0;

void setup() {
  // put your setup code here, to run once:
  #if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
  #endif
  pinMode(ENABLE1_PIN, INPUT_PULLUP);   // internal pull-up resistor
  pinMode(ENABLE2_PIN, INPUT_PULLUP);   // internal pull-up resistor
  pinMode(ENABLE3_PIN, INPUT_PULLUP);   // internal pull-up resistor
  pinMode(ENABLE4_PIN, INPUT_PULLUP);   // internal pull-up resistor
    // interrupts for breaklights
  attachInterrupt(digitalPinToInterrupt(ENABLE4_PIN), break_on, CHANGE);
  //attachInterrupt(digitalPinToInterrupt(ENABLE4_PIN), break_off, RISING);
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(BRIGHTNESS);
  strip2.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip2.show();            // Turn OFF all pixels ASAP
  strip2.setBrightness(BRIGHTNESS);
  strip3.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip3.show();            // Turn OFF all pixels ASAP
  strip3.setBrightness(BRIGHTNESS);
    //for windshield wipers
     // We need to attach the servo to the used pin number
   pinMode(buttonPin, INPUT_PULLUP);
   servo.attach(servoPin);
   servo.write(0);
   delay(2000);
}

void break_on(void){
  if (digitalRead(ENABLE4_PIN) == HIGH) {
    break_val = 30;
  }
  else{
    break_val = 255;
  }
  for(int i = 0; i < LED_COUNT; i++){
        strip.setPixelColor(i, break_val, 65, 0);
        //strip2.setPixelColor(i, break_val, 0, 0);
  }
  strip.show();
}
void break_off(void){
  break_val = 10;
  for(int i = 0; i < LED_COUNT; i++){
        strip.setPixelColor(i, break_val, 0, 0);
        //strip2.setPixelColor(i, break_val, 0, 0);
  }
  strip.show();
}

void wipers(void) {
  if (digitalRead(buttonPin) == LOW) { //button is pressed
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= wiperInterval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    // if the LED is off turn it on and vice-versa:
      if (servo.read() == 0) {
        servo.write(180);
        wiperStart = millis();
        }
      } 
      else {
        if (millis() - wiperStart >= 1400) {
          servo.write(0);
          wiperStart = millis();
    }
  }

  }
  
    else {
    if (millis() - wiperStart >= 1400) {
      servo.write(0);
      wiperStart = millis();
    }
  } 
}

void loop() {
 
    wipers();
  //strip.clear();
  strip2.clear();
  strip3.clear();
  // blinker lights
  if(digitalRead(ENABLE3_PIN) == LOW){  
    colorWipe3(strip2.Color(255,   30,   0), SPEED);
    colorWipe3(strip3.Color(255,   30,   0), SPEED);
  }
  else if(digitalRead(ENABLE3_PIN) == HIGH){
    if (digitalRead(ENABLE1_PIN) == LOW){ // bliker light 1 --> right = blue (strip2)
      colorWipe(strip2.Color(255,   30,   0), SPEED);
    }
    else if(digitalRead(ENABLE2_PIN) == LOW){// bliker light 2 --> right = green (strip2)
      colorWipe2(strip3.Color(255,   30,   0), SPEED);
    }
  }
  strip2.show();
  strip3.show();
}


void colorWipe_break(uint32_t color, int wait) {
    for(int j = 0; (digitalRead(ENABLE4_PIN) == LOW); j++) { // For each pixel in strip...
      for(int i = 0; i < LED_COUNT; i++){
        strip.setPixelColor(i, 10, 0, 0);
        strip2.setPixelColor(i, 10, 0, 0);
      }
      delay(wait);                           //  Pause for a moment
    };
  return;
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
/*
void colorBlock(uint32_t color, int wait){
  for(int i = LED_COUNT; digitalRead(ENABLE4_PIN) == LOW ; i++) {
    strip.setPixelColor(i, color);
    strip2.setPixelColor(i, color);
    strip.show();
    strip2.show();
  }
  return;
}
*/