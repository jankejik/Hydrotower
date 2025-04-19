#include "LedController.h"

LedController::LedController(uint8_t redPin, uint8_t yellowPin, uint8_t greenPin)
    : RED_PIN(redPin), YELLOW_PIN(yellowPin), GREEN_PIN(greenPin), blinkTaskHandle(nullptr), isBlinking(false), blinkDuration(0), isSequential(false)
{
}

void LedController::begin()
{
    pinMode(RED_PIN, OUTPUT);
    pinMode(YELLOW_PIN, OUTPUT);
    pinMode(GREEN_PIN, OUTPUT);
    allOff();
}

void LedController::setRed(bool state, bool admin)
{
    if (blok)
    {
        if (admin)
        {
            digitalWrite(RED_PIN, state);
        }
    }
    else
    {
        digitalWrite(RED_PIN, state);
    }
}

void LedController::setYellow(bool state, bool admin)
{
    if (blok)
    {
        if (admin)
        {
            digitalWrite(YELLOW_PIN, state);
        }
    }
    else
    {
        digitalWrite(YELLOW_PIN, state);
    }
}

void LedController::setGreen(bool state, bool admin)
{
    if (blok)
    {
        if (admin)
        {
            digitalWrite(GREEN_PIN, state);
        }
    }
    else
    {
        digitalWrite(GREEN_PIN, state);
    }
}

void LedController::allOff()
{
    setRed(false, true);
    setYellow(false, true);
    setGreen(false, true);
}

void LedController::allOn()
{
    setRed(true, true);
    setYellow(true, true);
    setGreen(true, true);
}

void LedController::blinkTask(void *parameter)
{
    LedController *controller = static_cast<LedController *>(parameter);

    while (controller->isBlinking)
    {
       
            // Sequential blinking pattern
            controller->setRed(true, true);
            controller->setYellow(false, true);
            vTaskDelay(pdMS_TO_TICKS(controller->blinkDuration));
            controller->setRed(false, true);
            controller->setYellow(true, true);
            vTaskDelay(pdMS_TO_TICKS(controller->blinkDuration));
    }
       
    
    controller->allOff();
    controller->blinkTaskHandle = nullptr;
    vTaskDelete(nullptr);
}

void LedController::blinkWifiError(void)
{
    blok = true;
    stopBlink();
    blinkDuration = 500;
    isSequential = true;
    isBlinking = true;

    xTaskCreate(
        blinkTask,
        "LED_BlinkSeq",
        2048,
        this,
        1,
        &blinkTaskHandle);
}

void LedController::stopBlink()
{
    if (blinkTaskHandle != nullptr)
    {
        isBlinking = false;
        while (blinkTaskHandle != nullptr)
        {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
    blok = false;
}