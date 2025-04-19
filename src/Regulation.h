#pragma once

#include "RelayController.h"
#include "UltrasonicSensor.h"
#include "LedController.h"
enum RegulationMode
{
    AUTO,
    MANUAL
};

class Regulation
{
private:
    RelayController &relay;
    UltrasonicSensor &sensor;
    LedController &leds;
    bool wasBelow10 = false;
    bool manualRequest = false;
    bool prevStareRel = false;
    RegulationMode mode = AUTO;

public:

    Regulation(RelayController &relayCtrl, UltrasonicSensor &sensorCtrl, LedController &ledsCtrl);

    void setMode(RegulationMode newMode);
    RegulationMode getMode() const;

    void setManualRelayState(bool on);
    void Init(void);

    void update();
};