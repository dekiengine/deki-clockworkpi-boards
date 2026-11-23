#pragma once

#include "PicoCalcController.h"

#include "IDekiInput.h"  // from deki-input

#include <cstdint>
#include <memory>
#include <vector>

namespace DekiClockworkPi
{

/// The PicoCalc's keyboard as a Deki input driver.
///
/// It owns the keyboard controller (PicoCalcController) and runs it from
/// Update, once a frame. The controller reports presses and releases, so a
/// key is down from its press until its release, as on a desktop keyboard,
/// and a game can walk for as long as a key is held. A printable key held
/// down repeats as further presses every 100 ms; each becomes a KeyDown, so
/// held keys type again, as they do on a desktop.
///
/// The controller reports a printable key by what it typed, so a release can
/// name a different character than its press did ('!' pressed with Shift,
/// '1' released after Shift was let go). When a release names no key that is
/// down, every printable key that is down is let go, so none sticks.
class PicoCalcKeyboard : public DekiInput::IDekiInput
{
public:
    explicit PicoCalcKeyboard(std::unique_ptr<PicoCalcController> controller);
    ~PicoCalcKeyboard() override;

    bool Initialize() override;
    void Shutdown() override;
    void Update() override;
    void RegisterEventCallback(const DekiInput::InputEventCallback& callback) override;
    bool IsInitialized() const override { return m_Initialized; }
    bool GetPointerPosition(int32_t* x, int32_t* y) const override;
    bool IsKeyPressed(uint32_t key) const override;

    PicoCalcController& GetController() { return *m_Controller; }

    /// Handles one event from the controller. Update passes them on; tests
    /// call it directly.
    void OnKeyEvent(PicoCalcKeyState state, uint8_t code);

private:
    std::unique_ptr<PicoCalcController> m_Controller;
    std::vector<DekiInput::InputEventCallback> m_Callbacks;
    std::vector<uint32_t> m_Down;
    bool m_Initialized = false;

    void Emit(DekiInput::InputEventType type, uint32_t key, uint32_t character);
    bool IsDown(uint32_t key) const;
    void Release(uint32_t key);
};

}  // namespace DekiClockworkPi
