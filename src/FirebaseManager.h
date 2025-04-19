#pragma once

#include <Firebase_ESP_Client.h>
#include <WiFi.h>
#include <time.h>
#include "RelayController.h"
#include "Thermometer.h"
#include "Regulation.h"
#include "WifiWebServer.h"
extern RelayController relay;
extern UltrasonicSensor sensor;
extern Thermometer termistor;
extern Regulation regulation;
extern WifiWebServer wifiWeb;

class FirebaseManager
{
private:
  FirebaseData fbdo;
  FirebaseAuth auth;
  FirebaseConfig config;
  bool notified = false;
  bool initDatabase = false;
  void initTime()
  {
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
    while (time(nullptr) < 100000)
    {
      delay(500);
    }
  }

public:
  void begin(const char *apiKey, const char *dbUrl)
  {
    if(!wifiWeb.Connected())
    {
      return;
    }
    if(!initDatabase)
    {
      initDatabase = true;
      initTime();

      auth.user.email = "";
      auth.user.password = "";
  
      config.api_key = apiKey;
      config.database_url = dbUrl;
      config.signer.tokens.legacy_token = "";
  
      if (Firebase.signUp(&config, &auth, "", ""))
      {
        Serial.println("✅ Přihlášení OK");
      }
      else
      {
        Serial.printf("❌ Chyba přihlášení: %s\n", config.signer.signupError.message.c_str());
      }
  
      Firebase.begin(&config, &auth);
      Firebase.reconnectWiFi(true);
  
      fbdo.setBSSLBufferSize(4096, 1024);
      fbdo.setResponseSize(4096);
      Firebase.setDoubleDigits(5);
  
      regulation.Init();
    }
   
  }

  void SendMode(RegulationMode mod)
  {
    Firebase.RTDB.setString(&fbdo, "/mode", mod == RegulationMode::AUTO ? "auto" : "manual");
  }

  void SendReleState(bool state)
  {
    Firebase.RTDB.setBool(&fbdo, "/rele", state);
  }

  void run()
  {
    if(!wifiWeb.Connected())
    {
      return;
    }

    if (!Firebase.ready())
    {
      Serial.println("⏳ Čekám na Firebase...");
      return;
    }

    int tempHl = sensor.getLastDistance();
    if (tempHl < 20)
    {
      tempHl = 20;
    }
    int hladina = map(tempHl, 20, 150, 0, 40);

    Firebase.RTDB.setInt(&fbdo, "/hladina", hladina);

    int teplotaInt = termistor.getTemperature() * 100;
    float teplota = (float)(teplotaInt) / 100;
    Firebase.RTDB.setFloat(&fbdo, "/teplota", teplota);

    if (Firebase.RTDB.getBool(&fbdo, "/rele"))
    {
      bool stav = fbdo.boolData();
      regulation.setManualRelayState(stav);
    }

    if (Firebase.RTDB.getString(&fbdo, "/mode"))
    {
      String mode = fbdo.stringData();
      if (mode == "auto")
      {
        regulation.setMode(RegulationMode::AUTO);
      }
      else if (mode == "manual")
      {
        regulation.setMode(RegulationMode::MANUAL);
      }
    }
    static int prevHladina = 0;
    if (hladina != prevHladina)
    {
      prevHladina = hladina;
      if (hladina < 1 && !notified)
      {
        Firebase.RTDB.setBool(&fbdo, "/notifikace", true);
        notified = true;
      }
      if (hladina > 2 && notified)
      {
        Firebase.RTDB.setBool(&fbdo, "/notifikace", false);
        notified = false;
      }
    }

    time_t now = time(nullptr);
    Firebase.RTDB.setInt(&fbdo, "/last_update", now);
  }
};
