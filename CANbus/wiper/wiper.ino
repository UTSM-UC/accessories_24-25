#include <Servo.h>
// Declare the Servo pin
int servoPin = 7;
int buttonPin = 3;
// Create a servo object
Servo servo;
const long wiperInterval = 2500;
unsigned long previousMillis = 0;
unsigned long wiperStart = 0;

void setup() {
  Serial.begin(9600);
  // put your setup code here, to run once:
  Serial.print("hello");
  pinMode(buttonPin, INPUT_PULLUP);
  servo.attach(servoPin);
  servo.write(0);
  delay(2000);
}

void wipers(void) {
  if (digitalRead(buttonPin) == LOW) { //button is pressed
  Serial.println("pressed");
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

void loop(){
  wipers();
}
