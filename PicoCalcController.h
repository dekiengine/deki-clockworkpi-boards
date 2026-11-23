#pragma once

#include "PicoCalcKeys.h"

#include <cstddef>
#include <cstdint>
#include <functional>

namespace DekiClockworkPi
{

/// Talks to the PicoCalc's keyboard controller, an STM32F103 at 0x1F on the
/// Pico's i2c1. It scans the keys and also runs the screen backlight, the
/// keyboard backlight and the battery gauge.
///
/// The protocol, from its firmware (Code/picocalc_keyboard in ClockworkPi's
/// PicoCalc repository): a write is [register | 0x80, value]; a read is the
/// register byte alone, then, a moment later, two bytes back. Register 0x09
/// hands over the oldest key event as [state, code], or [0, 0] when there is
/// none; 0x0B the battery as [0x0B, percent | 0x80 while charging].
///
/// Update does one step a call and never waits: a read is asked for in one
/// call and collected in a later one, once the controller has had
/// GetReplyDelayMs to answer. So the keyboard is polled every frame without
/// holding the frame up, which matters on its 10 kHz bus, where one read
/// takes about 4 ms. The controller also resets its I2C after 2.5 s without
/// a poll, which every-frame polling keeps it from doing.
///
/// The bus calls are passed in, so the protocol is tested without a bus.
class PicoCalcController
{
public:
    /// Writes `count` bytes to the controller. False when it did not answer.
    using Write = std::function<bool(const uint8_t* bytes, size_t count)>;
    /// Reads `count` bytes from the controller. False when it did not answer.
    using Read = std::function<bool(uint8_t* bytes, size_t count)>;
    /// Milliseconds, monotonic.
    using Clock = std::function<uint32_t()>;
    /// One key event, as the controller reported it.
    using KeyHandler = std::function<void(PicoCalcKeyState state, uint8_t code)>;

    /// The controller's address on its bus.
    static constexpr uint8_t kAddress = 0x1F;

    // Its registers.
    static constexpr uint8_t kRegScreenBacklight = 0x05;
    static constexpr uint8_t kRegKeyEvent = 0x09;
    static constexpr uint8_t kRegKeyboardBacklight = 0x0A;
    static constexpr uint8_t kRegBattery = 0x0B;
    static constexpr uint8_t kRegPowerOff = 0x0E;
    static constexpr uint8_t kWriteFlag = 0x80;

    /// `clock` defaults to the engine's.
    PicoCalcController(Write write, Read read, Clock clock = nullptr);

    void SetKeyHandler(KeyHandler handler) { m_KeyHandler = std::move(handler); }

    /// Does the next step: a write waiting to go, a reply to collect, or the
    /// next read to ask for. Call it every frame.
    void Update();

    /// Sets the screen's brightness. The controller keeps it between 16 and
    /// 240, in steps of 16: 0 is as dim as it goes, not off.
    void SetScreenBacklight(uint8_t level);
    /// Sets the keyboard's light: 0 is off, up to 224 in steps of 32.
    void SetKeyboardBacklight(uint8_t level);
    /// Asks the power chip to switch the PicoCalc off after `seconds` (the
    /// controller waits at least 6).
    void PowerOff(uint8_t seconds);

    /// The battery's charge in percent, or -1 before the first reading and
    /// without a battery.
    int GetBatteryPercent() const { return m_BatteryPercent; }
    /// True while the battery is charging.
    bool IsCharging() const { return m_Charging; }

    /// False until the controller first answers.
    bool IsFound() const { return m_Found; }

    /// How long the controller is given to answer a read. Their examples
    /// wait 16 ms.
    void SetReplyDelayMs(uint32_t ms) { m_ReplyDelayMs = ms; }
    uint32_t GetReplyDelayMs() const { return m_ReplyDelayMs; }
    /// How often the keyboard is asked for events while it has none.
    void SetPollIntervalMs(uint32_t ms) { m_PollIntervalMs = ms; }
    /// How often the battery is read. The controller itself reads its
    /// gauge about every 20 s.
    void SetBatteryIntervalMs(uint32_t ms) { m_BatteryIntervalMs = ms; }

private:
    enum class Step : uint8_t
    {
        Idle,
        AwaitingKeyEvent,
        AwaitingBattery,
    };

    // A register write waiting to go out. Later writes to the same register
    // replace it.
    struct PendingWrite
    {
        uint8_t reg = 0;
        uint8_t value = 0;
        bool waiting = false;
    };

    Write m_Write;
    Read m_Read;
    Clock m_Clock;
    KeyHandler m_KeyHandler;

    Step m_Step = Step::Idle;
    uint32_t m_AskedAt = 0;
    uint32_t m_LastPoll = 0;
    uint32_t m_LastBattery = 0;
    bool m_BatteryRead = false;
    // A key event came with the last poll: there may be more, so the next
    // poll goes at once.
    bool m_MoreEvents = true;
    bool m_Found = false;
    uint32_t m_LastFailure = 0;
    bool m_FailedOnce = false;

    static constexpr int kMaxWrites = 3;
    PendingWrite m_Writes[kMaxWrites];

    int m_BatteryPercent = -1;
    bool m_Charging = false;

    uint32_t m_ReplyDelayMs = 16;
    uint32_t m_PollIntervalMs = 20;
    uint32_t m_BatteryIntervalMs = 20000;

    void Queue(uint8_t reg, uint8_t value);
    bool Ask(uint8_t reg, Step awaiting, uint32_t now);
    void Collect(uint32_t now);
    void NoteAnswer(bool answered, uint32_t now);
};

}  // namespace DekiClockworkPi
