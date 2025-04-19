#include "UltrasonicSensor.h"

UltrasonicSensor::UltrasonicSensor(uint8_t trigPin, uint8_t echoPin, uint8_t measCount, uint8_t trim)
    : TRIG_PIN(trigPin), ECHO_PIN(echoPin), measurementsCount(measCount), trimCount(trim), lastFilteredDistance(0), measureIndex(0), taskHandle(NULL)
{
    measurements = new int[measurementsCount];
    sortedMeasurements = new int[measurementsCount];
    memset(measurements, 0, sizeof(int) * measurementsCount);
    memset(sortedMeasurements, 0, sizeof(int) * measurementsCount);
}

UltrasonicSensor::~UltrasonicSensor()
{
    delete[] measurements;
    delete[] sortedMeasurements;
}

void UltrasonicSensor::setMeasurementParams(uint8_t measCount, uint8_t trim)
{
    stop(); // Stop current task

    // Reallocate arrays if needed
    delete[] measurements;
    delete[] sortedMeasurements;

    measurementsCount = measCount;
    trimCount = trim;

    measurements = new int[measurementsCount];
    sortedMeasurements = new int[measurementsCount];

    measureIndex = 0;
    begin(); // Restart measurement task
}

void UltrasonicSensor::begin()
{
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    xTaskCreatePinnedToCore(
        measurementTask,  // Task funkce
        "UltrasonicTask", // Jméno tasku
        2048,             // Velikost zásobníku
        this,             // Parametr předaný tasku (např. this pointer)
        1,                // Priorita
        &taskHandle,      // Handle na task
        1                 // 🔧 Číslo jádra: 0 nebo 1
    );
}

void UltrasonicSensor::stop()
{
    if (taskHandle != NULL)
    {
        vTaskDelete(taskHandle);
        
        taskHandle = NULL;
    }
}

bool UltrasonicSensor::isRunning() const
{
    return taskHandle != NULL;
}

void UltrasonicSensor::measurementTask(void *parameter)
{
    UltrasonicSensor *sensor = static_cast<UltrasonicSensor *>(parameter);

    while (true)
    {
        sensor->measure();
        vTaskDelay(pdMS_TO_TICKS(100)); // 100ms between measurements
    }
}

void UltrasonicSensor::measure()
{
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);

    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    long duration = pulseIn(ECHO_PIN, HIGH);
    int distance = duration * 0.0344 / 2;

    if (distance > MAX_DISTANCE)
    {
        distance = MAX_DISTANCE;
    }

    measurements[measureIndex] = distance;
    measureIndex++;

    if (measureIndex >= measurementsCount)
    {
        lastFilteredDistance = getAverageTrimmed();

        Serial.print("Filtered distance: ");
        Serial.print(lastFilteredDistance);
        Serial.println(" cm");

        measureIndex = 0;
    }
}

int UltrasonicSensor::getAverageTrimmed()
{
    // Copy measurements to sorting array
    memcpy(sortedMeasurements, measurements, sizeof(int) * measurementsCount);

    // Bubble sort
    for (int i = 0; i < measurementsCount - 1; i++)
    {
        for (int j = 0; j < measurementsCount - i - 1; j++)
        {
            if (sortedMeasurements[j] > sortedMeasurements[j + 1])
            {
                int temp = sortedMeasurements[j];
                sortedMeasurements[j] = sortedMeasurements[j + 1];
                sortedMeasurements[j + 1] = temp;
            }
        }
    }

    // Calculate average of remaining values after trimming
    int sum = 0;
    int count = 0;

    // Skip trimCount values from start and end
    for (int i = trimCount; i < measurementsCount - trimCount; i++)
    {
        sum += sortedMeasurements[i];
        count++;
    }
    int temp = count > 0 ? sum / count : 0;
    if(temp == 0)
    {
        temp = 50;
    }
    return temp;
}