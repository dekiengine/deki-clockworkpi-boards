#pragma once

#include <cstdint>

namespace DekiClockworkPi
{

/// The codes the PicoCalc's keyboard controller (an STM32F103 on I2C at 0x1F)
/// reports, from its firmware's keyboard.h. A key that types a printable
/// character reports that character, shifted or not ('a', 'A', '1', '!');
/// the rest report these.
namespace PicoCalcKeyCode
{
// Named values, used like enum values: PascalCase, as the code style allows.
// NOLINTBEGIN(readability-identifier-naming)
constexpr uint8_t Backspace = 0x08;
constexpr uint8_t Tab = 0x09;
constexpr uint8_t Enter = 0x0A;
constexpr uint8_t F1 = 0x81;
constexpr uint8_t F9 = 0x89;
constexpr uint8_t F10 = 0x90;
constexpr uint8_t Power = 0x91;
constexpr uint8_t Alt = 0xA1;
constexpr uint8_t ShiftLeft = 0xA2;
constexpr uint8_t ShiftRight = 0xA3;
constexpr uint8_t Sym = 0xA4;
constexpr uint8_t Ctrl = 0xA5;
constexpr uint8_t Esc = 0xB1;
constexpr uint8_t Left = 0xB4;
constexpr uint8_t Up = 0xB5;
constexpr uint8_t Down = 0xB6;
constexpr uint8_t Right = 0xB7;
constexpr uint8_t CapsLock = 0xC1;
constexpr uint8_t Break = 0xD0;
constexpr uint8_t Insert = 0xD1;
constexpr uint8_t Home = 0xD2;
constexpr uint8_t Delete = 0xD4;
constexpr uint8_t End = 0xD5;
constexpr uint8_t PageUp = 0xD6;
constexpr uint8_t PageDown = 0xD7;
// NOLINTEND(readability-identifier-naming)
}  // namespace PicoCalcKeyCode

/// What a key event from the controller means: its state byte.
enum class PicoCalcKeyState : uint8_t
{
    Idle = 0,
    Pressed = 1,
    Held = 2,
    Released = 3,
};

/// The Deki key (DekiInput::Keys) and the character it types, for a code the
/// controller reported.
struct PicoCalcKey
{
    /// 0 when Deki has no key for it (F1-F10, Shift, Ctrl, Alt, Home, End,
    /// Page Up and Down, Insert, Caps Lock): such keys are not reported.
    uint32_t key = 0;
    /// 0 for a key that types nothing (an arrow, Enter, Backspace).
    uint32_t character = 0;
};

/// Maps a controller code to a Deki key.
PicoCalcKey MapPicoCalcKey(uint8_t code);

}  // namespace DekiClockworkPi
