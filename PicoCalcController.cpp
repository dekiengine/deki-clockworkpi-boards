#include "PicoCalcController.h"

#include <deki/LogSystem.h>
#include <deki/Time.h>

#include <utility>

namespace DekiClockworkPi
{

namespace
{

// After the controller stops answering, it is asked again this often rather
// than every frame: each try that fails takes a timeout on the bus.
constexpr uint32_t kRetryMs = 500;

}  // namespace

PicoCalcController::PicoCalcController(Write write, Read read, Clock clock)
    : m_Write(std::move(write)),
      m_Read(std::move(read)),
      m_Clock(clock ? std::move(clock) : Clock([]() { return Deki::Time::GetTime(); }))
{
}

void PicoCalcController::Queue(uint8_t reg, uint8_t value)
{
    for (PendingWrite& write : m_Writes)
    {
        if (write.waiting && write.reg == reg)
        {
            write.value = value;
            return;
        }
    }
    for (PendingWrite& write : m_Writes)
    {
        if (!write.waiting)
        {
            write = PendingWrite{ reg, value, true };
            return;
        }
    }
}

void PicoCalcController::SetScreenBacklight(uint8_t level)
{
    Queue(kRegScreenBacklight, level);
}

void PicoCalcController::SetKeyboardBacklight(uint8_t level)
{
    Queue(kRegKeyboardBacklight, level);
}

void PicoCalcController::PowerOff(uint8_t seconds)
{
    Queue(kRegPowerOff, seconds);
}

void PicoCalcController::NoteAnswer(bool answered, uint32_t now)
{
    if (answered)
    {
        if (!m_Found)
        {
            DEKI_LOG_INFO("PicoCalc: keyboard controller found at 0x%02X", static_cast<unsigned>(kAddress));
        }
        m_Found = true;
        m_FailedOnce = false;
        return;
    }
    if (!m_FailedOnce)
    {
        DEKI_LOG_WARNING("PicoCalc: the keyboard controller at 0x%02X did not answer; asking again every %u ms",
                         static_cast<unsigned>(kAddress), static_cast<unsigned>(kRetryMs));
    }
    m_FailedOnce = true;
    m_LastFailure = now;
}

bool PicoCalcController::Ask(uint8_t reg, Step awaiting, uint32_t now)
{
    const bool answered = m_Write(&reg, 1);
    NoteAnswer(answered, now);
    if (!answered)
    {
        return false;
    }
    m_Step = awaiting;
    m_AskedAt = now;
    return true;
}

void PicoCalcController::Collect(uint32_t now)
{
    uint8_t reply[2] = {};
    const bool answered = m_Read(reply, sizeof(reply));
    const Step was = m_Step;
    m_Step = Step::Idle;
    NoteAnswer(answered, now);
    if (!answered)
    {
        return;
    }
    if (was == Step::AwaitingKeyEvent)
    {
        const auto state = static_cast<PicoCalcKeyState>(reply[0]);
        m_MoreEvents = state != PicoCalcKeyState::Idle;
        if (m_MoreEvents && m_KeyHandler)
        {
            m_KeyHandler(state, reply[1]);
        }
        return;
    }
    if (was == Step::AwaitingBattery)
    {
        m_BatteryRead = true;
        const int percent = reply[1] & 0x7F;
        m_Charging = (reply[1] & 0x80) != 0;
        // The controller says 0 when there is no battery.
        m_BatteryPercent = percent == 0 && !m_Charging ? -1 : percent;
    }
}

void PicoCalcController::Update()
{
    const uint32_t now = m_Clock();
    if (m_FailedOnce && now - m_LastFailure < kRetryMs)
    {
        return;
    }

    if (m_Step != Step::Idle)
    {
        if (now - m_AskedAt >= m_ReplyDelayMs)
        {
            Collect(now);
        }
        return;
    }

    // Writes go first: they are what the game asked for.
    for (PendingWrite& write : m_Writes)
    {
        if (write.waiting)
        {
            const uint8_t bytes[2] = { static_cast<uint8_t>(write.reg | kWriteFlag), write.value };
            const bool answered = m_Write(bytes, sizeof(bytes));
            NoteAnswer(answered, now);
            if (answered)
            {
                write.waiting = false;
            }
            return;
        }
    }

    if (!m_BatteryRead || now - m_LastBattery >= m_BatteryIntervalMs)
    {
        m_LastBattery = now;
        Ask(kRegBattery, Step::AwaitingBattery, now);
        return;
    }

    if (m_MoreEvents || now - m_LastPoll >= m_PollIntervalMs)
    {
        m_LastPoll = now;
        Ask(kRegKeyEvent, Step::AwaitingKeyEvent, now);
    }
}

}  // namespace DekiClockworkPi
