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