#include "PicoCalcSetup.h"

#include "PicoCalcController.h"
#include "PicoCalcKeyboard.h"

#include "DekiI2C.h"    // from deki-i2c
#include "DekiInput.h"  // from deki-input
#include "IDekiI2C.h"   // from deki-i2c

#include <deki/LogSystem.h>

#include <memory>

namespace DekiClockworkPi
{

void PicoCalcSetup::Setup(SetupCallback onComplete)
{
    // The keyboard is not what decides whether the game starts: every way
    // this can go wrong is reported and the boot carries on.
    DekiI2c::IDekiI2C* bus = DekiI2c::DekiI2C::GetBus(i2cPort);
    if (!bus)
    {
        DEKI_LOG_ERROR("PicoCalcSetup: no I2C bus on port %d - add DekiI2c::I2CBusComponent before this step in "
                       "the boot scene. No keyboard.",
                       static_cast<int>(i2cPort));
        if (onComplete)
        {
            onComplete(true);
        }
        return;
    }

    // A read is the register byte on its own, then the reply in a separate
    // transaction: the controller prepares its answer when the write ends.
    constexpr uint8_t address = PicoCalcController::kAddress;
    auto controller = std::make_unique<PicoCalcController>(
        [bus](const uint8_t* bytes, size_t count) { return bus->WriteRaw(address, bytes, count); },
        [bus](uint8_t* bytes, size_t count) { return bus->ReadRaw(address, bytes, count); });
    controller->SetReplyDelayMs(static_cast<uint32_t>(replyDelayMs));
    controller->SetPollIntervalMs(static_cast<uint32_t>(pollIntervalMs));
    if (screenBacklight >= 0)
    {
        controller->SetScreenBacklight(static_cast<uint8_t>(screenBacklight));
    }
    if (keyboardBacklight >= 0)
    {
        controller->SetKeyboardBacklight(static_cast<uint8_t>(keyboardBacklight));
    }

    auto keyboard = std::make_unique<PicoCalcKeyboard>(std::move(controller));
    if (keyboard->Initialize())
    {
        ::DekiInput::DekiInput::SetInput(std::move(keyboard), "PicoCalcKeyboard");
        DEKI_LOG_INFO("PicoCalcSetup: keyboard controller on I2C port %d", static_cast<int>(i2cPort));
    }
    if (onComplete)
    {
        onComplete(true);
    }
}

}  // namespace DekiClockworkPi
