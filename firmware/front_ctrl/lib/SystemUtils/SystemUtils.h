#ifndef SYSTEM_UTILS_H
#define SYSTEM_UTILS_H

#include <Arduino.h>
#include "../Config/Config.h"

class SystemUtils {
private:
    unsigned long previousMillis;

public:
    SystemUtils();
    void heartbeat();
};

#endif