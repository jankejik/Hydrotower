#include "Arduino.h"
#include "UltrasonicSensor.h"
#include "LedController.h"
#include "RelayController.h"
#include "DisplayController.h"
#include <Wire.h>
#include "Thermometer.h"
#include <FirebaseManager.h>
#include "Regulation.h"
#include "WifiWebServer.h"
#include "TaskScheduler.h"
#include "TaskSchedulerDeclarations.h"
#include "Common.h"
bool notified = false; // aby se notifikace poslala jen jednou

#define ECHO_PIN 5
#define TRIG_PIN 18
#define RED_PIN 13
#define YELLOW_PIN 12
#define GREEN_PIN 14
#define RELAY_PIN 27
#define SDA_PIN 21
#define SCL_PIN 22
#define THERMISTOR_PIN 34

UltrasonicSensor sensor(TRIG_PIN, ECHO_PIN, 20, 4); // 20 měření, oříznutí 2 z každé strany
LedController leds(RED_PIN, YELLOW_PIN, GREEN_PIN);
RelayController relay(RELAY_PIN);
DisplayController display(sensor);
FirebaseManager firebase;
Thermometer termistor((gpio_num_t)THERMISTOR_PIN);
Regulation regulation(relay, sensor, leds);
WifiWebServer wifiWeb;
Scheduler runner;

void regUpdateTask(void)
{
  regulation.update();

}
void firebaseTask(void)
{
  firebase.begin(API_KEY, DATABASE_URL);
  firebase.run();

}
Task t1(1 * TASK_SECOND, TASK_FOREVER, &regUpdateTask, &runner, true);  //adding task to the chain on creation
Task t2(1 * TASK_SECOND, TASK_FOREVER, &firebaseTask, &runner, true);  //adding task to the chain on creation


void setup()
{

  Serial.begin(115200);
  Serial.println("Distance measurement using JSN-SR04T");
  leds.begin();
  wifiWeb.begin();
  leds.blinkWifiError();
  sensor.begin();
  relay.begin();
  termistor.begin();
  if (!display.begin())
  {
    Serial.println("Display initialization failed!");
    return;
  }

  display.start(); // Start display update task
  leds.stopBlink();
  runner.startNow();
}

void loop()
{
  runner.execute();
  wifiWeb.handle();
  delay(1);
}