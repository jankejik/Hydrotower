#include "Regulation.h"
#include "FirebaseManager.h"
extern FirebaseManager firebase;

Regulation::Regulation(RelayController &relayCtrl, UltrasonicSensor &sensorCtrl, LedController &ledsCtrl)
    : relay(relayCtrl), sensor(sensorCtrl), leds(ledsCtrl) {}

void Regulation::setMode(RegulationMode newMode)
{   
    if((mode != newMode) && relay.GetState() && (newMode == AUTO))
    {
        leds.setRed(false);
        leds.setYellow(false);
        leds.setGreen(true);
    }

    mode = newMode;
    if (mode == AUTO)
    {
        manualRequest = false;

    }
}
void Regulation::Init(void)
{
    mode = AUTO;
    firebase.SendMode(mode);
    firebase.SendReleState(relay.GetState());
}

RegulationMode Regulation::getMode() const
{
    return mode;
}

void Regulation::setManualRelayState(bool on)
{

    if (prevStareRel != on)
    {
        prevStareRel = on;
        mode = MANUAL;
        firebase.SendMode(mode);
    }
    if (mode == MANUAL)
    {
        if (on)
        {
            if (!relay.GetState())
            {
                leds.setRed(false);
                leds.setYellow(true);
                leds.setGreen(false);
                relay.on();
            }
        }
        else
        {
            if (relay.GetState())
            {
                leds.setRed(false);
                leds.setYellow(true);
                leds.setGreen(false);
                relay.off();
            }
        }
        manualRequest = on;
    }
}

void Regulation::update()
{
    int level = sensor.getLastDistance();
    if (mode == AUTO)
    {
        if (wasBelow10)
        {
            if (level > 12)
            {
                if (!relay.GetState())
                {
                    leds.setRed(false);
                    leds.setYellow(false);
                    leds.setGreen(true);
                    relay.on();
                    firebase.SendReleState(true);
                }
                wasBelow10 = false;
                prevStareRel = true;
            }
            else
            {
                if (relay.GetState())
                {
                    
                    leds.setRed(true);
                    leds.setYellow(false);
                    leds.setGreen(false);
                    relay.off();
                    firebase.SendReleState(false);
                }
                prevStareRel = false;
            }
        }
        else
        {
            if (level > 10)
            {
                if (!relay.GetState())
                {
                    leds.setRed(false);
                    leds.setYellow(false);
                    leds.setGreen(true);
                    relay.on();
                    firebase.SendReleState(true);
                }
                prevStareRel = true;
            }
            else
            {
                if (relay.GetState())
                {
                    leds.setRed(true);
                    leds.setYellow(false);
                    leds.setGreen(false);
                    relay.off();
                    firebase.SendReleState(false);
                }
                wasBelow10 = true;
                prevStareRel = false;
            }
        }
    }
    else if (mode == MANUAL)
    {
        if (level < 10 && manualRequest)
        {
            leds.setRed(true);
            leds.setYellow(false);
            leds.setGreen(false);
            relay.off();
            firebase.SendReleState(false);
            manualRequest = false;
        }
    }
}
