#ifndef CAN_HANDLER_H
#define CAN_HANDLER_H

#include <Arduino.h>
#include <mcp_can.h>
#include "../Config/Config.h"

class CanHandler {
private:
    MCP_CAN* can;
    long unsigned int canId;
    long unsigned int maskedCanId;
    unsigned char len;
    unsigned char buf[8];
    uint8_t canMessage;
    
    // Masking function
    uint16_t masking(uint16_t message);

public:
    CanHandler(int spiCsPin);
    bool begin();
    void readCANMessage();
    void sendCANMessage(int bitPosition, bool state);
    
    // Getters for message data
    bool getBit(int position);
    unsigned char* getBuffer() { return buf; }
    long unsigned int getCanId() { return canId; }
    long unsigned int getMaskedCanId() { return maskedCanId; }
};

#endif