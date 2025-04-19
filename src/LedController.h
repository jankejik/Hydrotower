#pragma once

#include "Arduino.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

class LedController {
private:
    const uint8_t RED_PIN;
    const uint8_t YELLOW_PIN;
    const uint8_t GREEN_PIN;
    
    TaskHandle_t blinkTaskHandle;
    volatile bool isBlinking;
    volatile uint16_t blinkDuration;
    volatile bool isSequential;
    bool blok = false;
    
    static void blinkTask(void* parameter);

public:
    LedController(uint8_t redPin, uint8_t yellowPin, uint8_t greenPin);
    void begin();
    
    void setRed(bool state, bool admin = false);
    void setYellow(bool state, bool admin = false);
    void setGreen(bool state, bool admin = false);
    
    void allOff();
    void allOn();
    
    // New blinking functions with tasks
    void blinkAll(uint16_t duration);    // Blink all LEDs simultaneously
    void blinkWifiError(void); // Blink LEDs one after another
    void stopBlink();                    // Stop any blinking pattern
    
    bool isRunning() const { return blinkTaskHandle != nullptr; }
};