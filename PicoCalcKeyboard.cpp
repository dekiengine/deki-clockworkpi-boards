#include "PicoCalcKeyboard.h"

#include "PicoCalc.h"

#include <deki/Time.h>

#include <algorithm>
#include <utility>

namespace DekiClockworkPi
{

namespace
{

// True for the keys that type something: the ones a release may name
// differently from their press.
bool IsPrintableKey(uint32_t key)
{
    return key >= 32 && key < 127;
}

}  // namespace

PicoCalcKeyboard::PicoCalcKeyboard(std::unique_ptr<PicoCalcController> controller)
    : m_Controller(std::move(controller))
{
    m_Controller->SetKeyHandler([this](PicoCalcKeyState state, uint8_t code) { OnKeyEvent(state, code); });
    PicoCalc::SetController(m_Controller.get());
}

PicoCalcKeyboard::~PicoCalcKeyboard()
{
    if (PicoCalc::GetController() == m_Controller.get())
    {
        PicoCalc::SetController(nullptr);
    }
}

bool PicoCalcKeyboard::Initialize()
{
    m_Initialized = true;
    return true;
}

void PicoCalcKeyboard::Shutdown()
{
    m_Down.clear();
    m_Initialized = false;
}

void PicoCalcKeyboard::Update()
{
    if (m_Initialized)
    {
        m_Controller->Update();
    }
}

void PicoCalcKeyboard::RegisterEventCallback(const DekiInput::InputEventCallback& callback)
{
    m_Callbacks.push_back(callback);
}

bool PicoCalcKeyboard::GetPointerPosition(int32_t* x, int32_t* y) const
{
    (void)x;
    (void)y;
    return false;
}

bool PicoCalcKeyboard::IsKeyPressed(uint32_t key) const
{
    return IsDown(key);
}

bool PicoCalcKeyboard::IsDown(uint32_t key) const
{
    return std::find(m_Down.begin(), m_Down.end(), key) != m_Down.end();
}

void PicoCalcKeyboard::Emit(DekiInput::InputEventType type, uint32_t key, uint32_t character)
{
    DekiInput::InputEvent event{};
    event.type = type;
    event.key = key;
    event.pressed = type == DekiInput::InputEventType::KeyDown;
    event.timestamp = Deki::Time::GetTime();
    event.character = character;
    for (const DekiInput::InputEventCallback& callback : m_Callbacks)
    {
        callback(event);
    }
}

void PicoCalcKeyboard::Release(uint32_t key)
{
    m_Down.erase(std::remove(m_Down.begin(), m_Down.end(), key), m_Down.end());
    Emit(DekiInput::InputEventType::KeyUp, key, 0);
}

void PicoCalcKeyboard::OnKeyEvent(PicoCalcKeyState state, uint8_t code)
{
    const PicoCalcKey mapped = MapPicoCalcKey(code);
    if (mapped.key == 0)
    {
        return;
    }
    switch (state)
    {
        case PicoCalcKeyState::Pressed:
            if (!IsDown(mapped.key))
            {
                m_Down.push_back(mapped.key);
            }
            Emit(DekiInput::InputEventType::KeyDown, mapped.key, mapped.character);
            break;
        case PicoCalcKeyState::Held:
            // Still down; the press already said so.
            break;
        case PicoCalcKeyState::Released:
            if (IsDown(mapped.key))
            {
                Release(mapped.key);
            }
            else if (IsPrintableKey(mapped.key))
            {
                // Named differently from its press: let every printable key
                // go, so the one meant does not stick.
                const std::vector<uint32_t> down = m_Down;
                for (uint32_t key : down)
                {
                    if (IsPrintableKey(key))
                    {
                        Release(key);
                    }
                }
            }
            break;
        case PicoCalcKeyState::Idle: break;
    }
}

}  // namespace DekiClockworkPi
