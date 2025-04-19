#pragma once

#include "Arduino.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

class UltrasonicSensor {
private:
    const uint8_t TRIG_PIN;
    const uint8_t ECHO_PIN;
    const uint16_t MAX_DISTANCE = 150;  // cm
    
    uint8_t measurementsCount;  // Number of measurements to take
    uint8_t trimCount;         // Number of values to trim from each end
    int* measurements;         // Dynamic array for measurements
    int* sortedMeasurements;  // Dynamic array for sorting
    volatile int lastFilteredDistance;
    int measureIndex;
    TaskHandle_t taskHandle;
    
    static void measurementTask(void* parameter);
    int getAverageTrimmed();
    void measure();

public:
    UltrasonicSensor(uint8_t trigPin, uint8_t echoPin, uint8_t measCount = 10, uint8_t trim = 2);
    ~UltrasonicSensor();  // Destructor to clean up dynamic arrays
    void begin();
    void stop();
    void setMeasurementParams(uint8_t measCount, uint8_t trim);
    int getLastDistance() const { return lastFilteredDistance; }
    bool isRunning() const;
};