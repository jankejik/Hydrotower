#pragma once
#include <Arduino.h>

#define TERM_NOM 10000
#define REF_TEP 25
#define BETA 3950
#define REZISTOR 10000

class Thermometer {
public:
  Thermometer(gpio_num_t analogPin);
  void begin();
  float getTemperature(); // Vrátí zprůměrovanou teplotu
private:
  gpio_num_t _analogPin;
  int _termNom = TERM_NOM;
  int _refTep = REF_TEP;
  int _beta = BETA;
  int _rezistor = REZISTOR;

  static const int bufferSize = 10;
  float buffer[bufferSize];
  int index = 0;
  bool filled = false;
  TaskHandle_t taskHandle = NULL;
  SemaphoreHandle_t mutex;

  static void taskWrapper(void *param);
  void taskLoop();
  float readTemperature();
  void updateBuffer(float temp);
};
