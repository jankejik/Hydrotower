#pragma once

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "UltrasonicSensor.h"

class DisplayController {
private:
    static const uint8_t SCREEN_WIDTH = 128;
    static const uint8_t SCREEN_HEIGHT = 64;
    static const uint8_t OLED_RESET = -1;
    static const uint8_t SCREEN_ADDRESS = 0x3C;

    Adafruit_SSD1306 display;
    UltrasonicSensor& sensor;
    TaskHandle_t updateTaskHandle;
    volatile bool isRunning;

    static void updateTask(void* parameter);
    void updateDisplay(int distance, float temperature, bool relayState);

public:
    DisplayController(UltrasonicSensor& ultrasonicSensor);
    bool begin();
    void start();
    void stop();
    bool isDisplayRunning() const { return isRunning; }
};