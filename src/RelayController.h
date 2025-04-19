#pragma once

#include "Arduino.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

class RelayController {
private:
    const uint8_t RELAY_PIN;
    TaskHandle_t cycleTaskHandle;
    volatile bool isCycling;
    volatile uint32_t cycleInterval;
    static bool state;
    static void cycleTask(void* parameter);

public:
    RelayController(uint8_t relayPin);
    void begin();
    bool GetState(void);
    void on();
    void off();
    void startCycling(uint32_t interval);  // interval in milliseconds
    void stopCycling();
    
    bool isRunning() const { return cycleTaskHandle != nullptr; }
};