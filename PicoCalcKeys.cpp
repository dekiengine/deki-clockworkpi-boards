#include "PicoCalcKeys.h"

#include "Keys.h"  // from deki-input

namespace DekiClockworkPi
{

PicoCalcKey MapPicoCalcKey(uint8_t code)
{
    namespace Keys = DekiInput::Keys;
    switch (code)
    {
        case PicoCalcKeyCode::Backspace: return { Keys::Backspace, 0 };
        case PicoCalcKeyCode::Tab: return { Keys::Tab, 0 };
        case PicoCalcKeyCode::Enter: return { Keys::Enter, 0 };
        case PicoCalcKeyCode::Esc: return { Keys::Esc, 0 };
        case PicoCalcKeyCode::Delete: return { Keys::Delete, 0 };
        case PicoCalcKeyCode::Up: return { Keys::Up, 0 };
        case PicoCalcKeyCode::Down: return { Keys::Down, 0 };
        case PicoCalcKeyCode::Left: return { Keys::Left, 0 };
        case PicoCalcKeyCode::Right: return { Keys::Right, 0 };
        default: break;
    }
    // Printable characters are themselves; the key is the one they sit on.
    if (code >= 32 && code < 127)
    {
        return { Keys::ForCharacter(code), code };
    }
    return {};
}

}  // namespace DekiClockworkPi
