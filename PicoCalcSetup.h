#pragma once

#include "ClockworkPiPackage.h"

#include <deki/SetupComponent.h>
#include <deki/reflection/Property.h>

#include <cstdint>

namespace DekiClockworkPi
{

/// Boot-scene component for the PicoCalc's keyboard controller: the keys,
/// both backlights, the battery and switching off (see PicoCalc.h). Needs an
/// I2CBusComponent on the matching port earlier in the boot scene.
///
/// Keys arrive as KeyDown / KeyUp through DekiInput like any keyboard's.
/// A missing controller does not stop the boot: it is asked again every half
/// second, and the log says so once.
DEKI_CATEGORY("Input")
DEKI_DISPLAY_NAME("PicoCalc Keyboard")
DEKI_DESCRIPTION("The PicoCalc's keyboard controller: keys, screen and keyboard backlights, battery, power off.")
class DEKI_CLOCKWORKPI_API PicoCalcSetup : public Deki::SetupComponent
{
public:
    DEKI_EXPORT
    DEKI_TOOLTIP("Which I2C bus the controller is on. Must match the I2C Bus component that set that port up.")
    DEKI_RANGE(0, 3)
    int32_t i2cPort = 1;

    DEKI_EXPORT
    DEKI_TOOLTIP("How long the controller is given to answer a read, in milliseconds. ClockworkPi's examples "
                 "wait 16.")
    DEKI_RANGE(1, 100)
    int32_t replyDelayMs = 16;

    DEKI_EXPORT
    DEKI_TOOLTIP("How often the keys are asked for while none is pressed, in milliseconds. While keys come in, "
                 "they are asked for every frame.")
    DEKI_RANGE(5, 1000)
    int32_t pollIntervalMs = 20;

    DEKI_EXPORT
    DEKI_TOOLTIP("The screen's brightness at boot, 0 to 255. The controller keeps it between 16 and 240. -1 leaves "
                 "it as the controller has it.")
    DEKI_RANGE(-1, 255)
    int32_t screenBacklight = -1;

    DEKI_EXPORT
    DEKI_TOOLTIP("The keyboard's light at boot, 0 (off) to 255. -1 leaves it as the controller has it.")
    DEKI_RANGE(-1, 255)
    int32_t keyboardBacklight = -1;

    void Setup(SetupCallback onComplete) override;
    const char* GetSetupName() const override { return "PicoCalc Keyboard"; }
};

}  // namespace DekiClockworkPi
