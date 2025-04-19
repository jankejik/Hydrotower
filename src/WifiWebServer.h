#pragma once

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>
#include <Update.h>
#include "ArduinoJson.h"

extern AsyncWebServer server;

class WifiWebServer
{
private:
    Preferences preferences;
    unsigned long apStartTime;
    volatile bool clientConnected = false;
    const unsigned long AP_TIMEOUT = 2 * 60 * 1000; // 2 minuty
    volatile bool isConnect = false;

public:
    bool Connected(void)
    {
        return isConnect;
    }
    void begin()
    {
        preferences.begin("wifi", false);

        String ssid = preferences.getString("ssid", "");
        String password = preferences.getString("pass", "");
        Serial.println("MAC adresa: " + WiFi.macAddress());

        if (ssid != "")
        {
            Serial.println(ssid);
            Serial.println(password);
            ssid.trim();
            password.trim();
            WiFi.begin(ssid.c_str(), password.c_str());
            unsigned long startAttemptTime = millis();
            while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 30000)
            {
                delay(100);
            }
            if (WiFi.status() == WL_CONNECTED)
            {
                isConnect = true;
                Serial.println("Připojeno k WiFi klientovi");
            }
        }

        // AP mód
        IPAddress apIP(192, 168, 1, 1);
        IPAddress gateway(192, 168, 1, 1);
        IPAddress subnet(255, 255, 255, 0);
        WiFi.softAPConfig(apIP, gateway, subnet);
        WiFi.softAP("Hydrotower");
        apStartTime = millis();

        server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
            this->clientConnected = true;
            request->send(200, "text/html", getHtml());
        });

        server.on("/wifi", HTTP_POST, [this](AsyncWebServerRequest *request) {
            String ssid, password;
            if (request->hasParam("ssid", true) && request->hasParam("password", true)) {
                ssid = request->getParam("ssid", true)->value();
                password = request->getParam("password", true)->value();

                Serial.println("📥 Ukládám WiFi:");
                Serial.println(ssid);
                Serial.println(password);

                this->preferences.putString("ssid", ssid);
                this->preferences.putString("pass", password);

                request->send(200, "text/html", "<meta http-equiv='refresh' content='2; url=/'/><h2>Uloženo. Restart...</h2>");

                delay(500);
                ESP.restart();
            } else {
                request->send(400, "text/plain", "Chybí údaje.");
            }
        });

        server.on("/update", HTTP_POST,
            [](AsyncWebServerRequest *request) {
                request->send(200, "text/plain", Update.hasError() ? "Upload error" : "OK");
                delay(1000);
                ESP.restart();
            },
            [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
                if (!index) {
                    Serial.printf("Upload OTA start: %s\n", filename.c_str());
                    if (!Update.begin()) {
                        Update.printError(Serial);
                    }
                }
                Update.write(data, len);
                if (final) {
                    if (Update.end(true)) {
                        Serial.println("OTA completed");
                    } else {
                        Update.printError(Serial);
                    }
                }
            });

        server.begin();
        Serial.println("AP zapnuto: Hydrotower");
    }

    void handle()
    {
        if (!clientConnected && millis() - apStartTime > AP_TIMEOUT)
        {
            Serial.println("Časovač vypršel, vypínám AP...");
            WiFi.softAPdisconnect(true);
        }
    }

private:
    static String getHtml()
    {
        return R"rawliteral(
      <!DOCTYPE html>
      <html>
      <body>
        <h2>WiFi client settings</h2>
       <form method="POST" action="/wifi">
  <label>SSID:</label><input name="ssid"><br>
  <label>Password:</label><input name="password" type="password"><br>
  <button type="submit">Save</button>
</form>

        <h2>Firmware update</h2>
        <form method="POST" action="/update" enctype="multipart/form-data">
          <input type="file" name="update">
          <input type="submit" value="Upload">
        </form>
      </body>
      </html>
    )rawliteral";
    }
};
