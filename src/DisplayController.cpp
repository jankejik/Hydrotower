#include "DisplayController.h"
#include "Thermometer.h"
#include "RelayController.h"
extern Thermometer termistor;
extern RelayController relay;

DisplayController::DisplayController(UltrasonicSensor& ultrasonicSensor)
    : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET)
    , sensor(ultrasonicSensor)
    , updateTaskHandle(nullptr)
    , isRunning(false)
{
}

bool DisplayController::begin() {
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("SSD1306 allocation failed"));
        return false;
    }

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.cp437(true);

    // Úvodní zpráva - větší texty
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println(F("Inicializace"));

    display.setTextSize(1);
    display.setCursor(0, 24);
    display.println(F("Distance.: --- cm"));
    display.println(F("Temperature.: --.- C"));
    display.println(F("Relay: ---"));

    display.display();

    return true;
}



void DisplayController::updateDisplay(int distance, float temperature, bool relayState) {
    display.clearDisplay();

    // Titulek
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println(F("Hydrotower"));

    // Vzdálenost – větší písmo
    display.setTextSize(1);
    display.setCursor(0, 20);
    display.print(F("Distance: "));
    display.print(distance);
    display.println(F("cm"));

    // Teplota
    display.setTextSize(1);
    display.setCursor(0, 35);
    display.print(F("Temperature: "));
    display.print(temperature, 1);
    display.println(F(" C"));

    // Relé stav
    display.setCursor(0, 50);
    display.print(F("Relay: "));
    display.println(relayState ? F("ON") : F("OFF"));

    display.display();
}




void DisplayController::updateTask(void* parameter) {
    DisplayController* controller = static_cast<DisplayController*>(parameter);
    
    while(controller->isRunning) {
        int distance = controller->sensor.getLastDistance();
        float temperature = termistor.getTemperature();
        bool relayState = relay.GetState();

        controller->updateDisplay(distance, temperature, relayState);
        vTaskDelay(pdMS_TO_TICKS(1000)); // aktualizace 1× za sekundu
    }

    controller->updateTaskHandle = nullptr;
    vTaskDelete(nullptr);
}

void DisplayController::start() {
    if (updateTaskHandle == nullptr) {
        isRunning = true;
        xTaskCreate(
            updateTask,
            "OLED_Update",
            4096,  // Larger stack for display operations
            this,
            1,
            &updateTaskHandle
        );
    }
}

void DisplayController::stop() {
    if (updateTaskHandle != nullptr) {
        isRunning = false;
        while(updateTaskHandle != nullptr) {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
        display.clearDisplay();
        display.display();
    }
}