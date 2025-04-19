#include "RelayController.h"

bool RelayController::state = false;

RelayController::RelayController(uint8_t relayPin)
    : RELAY_PIN(relayPin), cycleTaskHandle(nullptr), isCycling(false), cycleInterval(0)
{
}

void RelayController::begin()
{
    pinMode(RELAY_PIN, OUTPUT);
    off(); // Start with relay off for safety
}

void RelayController::on()
{
    digitalWrite(RELAY_PIN, LOW);
    state = true;
}

void RelayController::off()
{
    digitalWrite(RELAY_PIN, HIGH);
    state = false;
}

void RelayController::cycleTask(void *parameter)
{
    RelayController *controller = static_cast<RelayController *>(parameter);

    while (controller->isCycling)
    {
        controller->on();
        vTaskDelay(pdMS_TO_TICKS(controller->cycleInterval));

        if (!controller->isCycling)
            break; // Check if we should stop

        controller->off();
        vTaskDelay(pdMS_TO_TICKS(controller->cycleInterval));
    }

    controller->off(); // Ensure relay is off when stopping
    controller->cycleTaskHandle = nullptr;
    vTaskDelete(nullptr);
}

void RelayController::startCycling(uint32_t interval)
{
    // Stop any existing cycling
    stopCycling();

    cycleInterval = interval;
    isCycling = true;

    xTaskCreate(
        cycleTask,
        "RELAY_Cycle",
        2048,
        this,
        1,
        &cycleTaskHandle);
}

void RelayController::stopCycling()
{
    if (cycleTaskHandle != nullptr)
    {
        isCycling = false;
        while (cycleTaskHandle != nullptr)
        {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}

bool RelayController::GetState(void)
{
    return state;
}