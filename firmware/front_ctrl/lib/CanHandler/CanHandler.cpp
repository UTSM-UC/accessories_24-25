#include "CanHandler.h"
#include <SPI.h>

CanHandler::CanHandler(int spiCsPin) {
    can = new MCP_CAN(spiCsPin);
    len = 0;
    for (int i = 0; i < 8; i++) {
        buf[i] = 0;
    }
    canMessage = 0;
}

bool CanHandler::begin() {
    if (can->begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ) == CAN_OK) {
        can->setMode(MCP_NORMAL);
        return true;
    } else {
        return false;
    }
}

uint16_t CanHandler::masking(uint16_t message) {
    return message & CAN_MASK;
}

void CanHandler::readCANMessage() {
    bool received = can->checkReceive() == CAN_MSGAVAIL;
    if (received || buf[0] != 0) {
        can->readMsgBuf(&canId, 0, &len, buf);  // Read data
        maskedCanId = masking(canId);
    }
}

void CanHandler::sendCANMessage(int bitPosition, bool state) {
    if (state) {
        canMessage = buf[0];
        canMessage |= (1 << bitPosition);
        can->sendMsgBuf(CAN_ID_FRONT, 0, 1, &canMessage);  
    } else {
        canMessage = buf[0];
        canMessage &= ~(1 << bitPosition);
        can->sendMsgBuf(CAN_ID_FRONT, 0, 1, &canMessage);
    }
}

bool CanHandler::getBit(int position) {
    return (buf[0] & (1 << position));
}

bool CanHandler::checkHornMessage() {
    bool received = can->checkReceive() == CAN_MSGAVAIL;
    if (received) {
        unsigned char tempBuf[8];
        unsigned char tempLen = 0;
        long unsigned int tempCanId;
        
        // Read data without disturbing main buffer
        can->readMsgBuf(&tempCanId, 0, &tempLen, tempBuf);
        
        // Check if it's the right message ID - monitor all horn messages
        // We handle both horn-on AND horn-off
        if ((tempCanId & CAN_FILTER)) {
            // Check if horn state has changed
            bool oldHornState = (buf[0] & (1 << 3));
            bool newHornState = (tempBuf[0] & (1 << 3));
            
            // If the horn state has changed OR it's a horn message, update state
            if (oldHornState != newHornState || (tempCanId & CAN_FILTER)) {
                // Update the main buffer with the new message
                canId = tempCanId;
                maskedCanId = masking(canId);
                len = tempLen;
                for (int i = 0; i < 8; i++) {
                    buf[i] = tempBuf[i];
                }
                return true;
            }
        }
    }
    return false;
}