#pragma once

#include "ClockworkPiPackage.h"

#include <cstdint>

namespace DekiClockworkPi
{

class PicoCalcController;

/// What a game can ask of a PicoCalc beyond its screen and keys: the
/// backlights, the battery and switching off. All of it goes through the
/// keyboard controller, which PicoCalcSetup starts in the boot scene. Before
/// that, or on another board, the setters do nothing and the battery reads
/// as unknown.
///
///     DekiClockworkPi::PicoCalc::SetScreenBacklight(128);
///     if (DekiClockworkPi::PicoCalc::GetBatteryPercent() < 10) { ... }
///
/// A change goes out with the controller's next step, within a frame or two.
class DEKI_CLOCKWORKPI_API PicoCalc
{
public:
    /// The screen's brightness, 0 to 255. The controller keeps it between
    /// 16 and 240, in steps of 16: the screen cannot be turned fully off.
    static void SetScreenBacklight(uint8_t level);

    /// The keyboard's light, 0 (off) to 255, in steps of 32 up to 224.
    static void SetKeyboardBacklight(uint8_t level);

    /// The battery's charge in percent, or -1 when it is not known yet or
    /// there is no battery. Read about every 20 seconds.
    static int GetBatteryPercent();

    /// True while the battery is charging.
    static bool IsCharging();

    /// Switches the PicoCalc off after `seconds`; the controller waits at
    /// least 6.
    static void PowerOff(uint8_t seconds = 6);

    /// True once the keyboard controller has answered.
    static bool IsAvailable();

    // ---- for PicoCalcKeyboard -----------------------------------------------

    static void SetController(PicoCalcController* controller);
    static PicoCalcController* GetController();

private:
    static PicoCalcController* s_Controller;
};

}  // namespace DekiClockworkPi
