#include "Thermometer.h"
#include <math.h>

Thermometer::Thermometer(gpio_num_t analogPin)
  : _analogPin(analogPin){
  mutex = xSemaphoreCreateMutex();
}

void Thermometer::begin() {
  analogReadResolution(12);
  for (int i = 0; i < bufferSize; i++) buffer[i] = 0.0;

  xTaskCreatePinnedToCore(
    taskWrapper,
    "NTC_Task",
    2048,
    this,
    1,
    &taskHandle,
    0 // Core 0
  );
}

void Thermometer::taskWrapper(void *param) {
  static_cast<Thermometer *>(param)->taskLoop();
}

void Thermometer::taskLoop() {
  while (true) {
    float temp = readTemperature();
    updateBuffer(temp);

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

float Thermometer::readTemperature() {
  int raw = analogRead(_analogPin);
  if (raw == 0) return -273.15;

  float napeti = 4095.0 / raw - 1.0;
  napeti = _rezistor / napeti;

  float teplota = napeti / _termNom;
  teplota = log(teplota);
  teplota /= _beta;
  teplota += 1.0 / (_refTep + 273.15);
  teplota = 1.0 / teplota;
  teplota -= 273.15;
  return teplota;
}

void Thermometer::updateBuffer(float temp) {
  xSemaphoreTake(mutex, portMAX_DELAY);
  buffer[index++] = temp;
  if (index >= bufferSize) {
    index = 0;
    filled = true;
  }
  xSemaphoreGive(mutex);
}

float Thermometer::getTemperature() {
  float sum = 0;
  int count;

  xSemaphoreTake(mutex, portMAX_DELAY);
  count = filled ? bufferSize : index;
  if (count == 0) {
    xSemaphoreGive(mutex);
    return -273.15;
  }
  for (int i = 0; i < count; i++) {
    sum += buffer[i];
  }
  xSemaphoreGive(mutex);

  return sum / count - 6;
}
