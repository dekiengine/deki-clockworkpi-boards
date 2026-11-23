#include "PicoCalc.h"

#include "PicoCalcController.h"

namespace DekiClockworkPi
{

PicoCalcController* PicoCalc::s_Controller = nullptr;

void PicoCalc::SetController(PicoCalcController* controller)
{
    s_Controller = controller;
}

PicoCalcController* PicoCalc::GetController()
{
    return s_Controller;
}

void PicoCalc::SetScreenBacklight(uint8_t level)
{
    if (s_Controller)
    {
        s_Controller->SetScreenBacklight(level);
    }
}

void PicoCalc::SetKeyboardBacklight(uint8_t level)
{
    if (s_Controller)
    {
        s_Controller->SetKeyboardBacklight(level);
    }
}

int PicoCalc::GetBatteryPercent()
{
    return s_Controller ? s_Controller->GetBatteryPercent() : -1;
}

bool PicoCalc::IsCharging()
{
    return s_Controller && s_Controller->IsCharging();
}

void PicoCalc::PowerOff(uint8_t seconds)
{
    if (s_Controller)
    {
        s_Controller->PowerOff(seconds);
    }
}

bool PicoCalc::IsAvailable()
{
    return s_Controller && s_Controller->IsFound();
}

}  // namespace DekiClockworkPi
