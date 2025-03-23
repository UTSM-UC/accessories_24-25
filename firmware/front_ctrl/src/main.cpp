#include <Arduino.h>
#include "Config.h"
#include "CanHandler.h"
#include "LightController.h"
#include "InputHandler.h"
#include "SystemUtils.h"

// Create instances of our classes
CanHandler canHandler(SPI_CS_PIN);
LightController lightCtrl;
InputHandler inputHandler;
SystemUtils sysUtils;

void setup() {
  // Setup pins
  inputHandler.setupPins();
  
  digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
  // Initialize CAN bus
  if (!canHandler.begin()) {
    while(1); // Init failed, stop the program
  }
  digitalWrite(LED_BUILTIN, LOW);  // turn the LED on (HIGH is the voltage level)

  // Initialize LED strips
  lightCtrl.begin();
  
  // Initial LED status
  digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
  delay(2000);
}

void loop() {
  // First, check brake button - highest priority
  int brakeState = inputHandler.edgeDetector(BRAKE_PIN);
  if (brakeState != -1) {
    canHandler.sendCANMessage(2, brakeState == 1);
    
    // Handle brake lights directly with highest priority
    if (brakeState == 1) {
      lightCtrl.brake_on();  // This will override any animation
    } else {
      lightCtrl.brake_off();  // Explicit brake off
    }
  }
  
  // Update animations (non-blocking part still exists)
  lightCtrl.update();
  
  // Read CAN messages
  canHandler.readCANMessage();
  
  // Handle CAN message data
  if (canHandler.getMaskedCanId() & CAN_FILTER) {
    if (canHandler.getBit(4)) {  // Hazard
      lightCtrl.hazards(255, 30, 0, SPEED);  // Blocking hazard function
    } else if (canHandler.getBit(1)) {  // Left turn
      lightCtrl.orangeBlinker2(255, 30, 0, SPEED);  // Blocking left turn function
    } else if (canHandler.getBit(0)) {  // Right turn
      lightCtrl.orangeBlinker3(255, 30, 0, SPEED);  // Blocking right turn function
    }
    
    // Handle digital outputs
    digitalWrite(HEADLIGHT_PIN, canHandler.getBit(2) ? HIGH : LOW);
    digitalWrite(HORN_PIN, canHandler.getBit(3) ? LOW : HIGH);
    digitalWrite(WIPER_PIN, canHandler.getBit(5) ? HIGH : LOW);
  }
  
  // Base lighting effect (lowest priority)
  lightCtrl.running_lights();
  
  // Call the heartbeat function
  //sysUtils.heartbeat();
  
  delay(5);  // Even shorter delay for more responsive input checking
}