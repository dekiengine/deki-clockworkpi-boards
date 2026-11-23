// The PicoCalc's keyboard controller off the device: which Deki key each code
// presses, the register protocol against a scripted controller, and keys
// staying down from press to release.
//
// A mistake here shows up on the PicoCalc as keys that do nothing or stick,
// a backlight that does not change, or a frame held up by the bus.

#include <gtest/gtest.h>

#include "Keys.h"  // from deki-input
#include "PicoCalc.h"
#include "PicoCalcController.h"
#include "PicoCalcKeyboard.h"
#include "PicoCalcKeys.h"

#include <deque>
#include <memory>
#include <utility>
#include <vector>

using namespace DekiClockworkPi;
namespace Keys = DekiInput::Keys;

namespace
{

// A keyboard controller on a bus: records what is written and answers reads
// from a script. Time is moved by hand.
struct FakeController
{
    std::vector<std::vector<uint8_t>> writes;
    std::deque<std::pair<uint8_t, uint8_t>> keyEvents;  // [state, code] left in the FIFO
    uint8_t battery = 0x80 | 77;                        // charging, 77%
    uint8_t lastRegister = 0;
    bool answers = true;
    int reads = 0;
    uint32_t now = 1000;

    std::unique_ptr<PicoCalcController> Make()
    {
        auto controller = std::make_unique<PicoCalcController>(
            [this](const uint8_t* bytes, size_t count)
            {
                if (!answers)
                {
                    return false;
                }
                writes.emplace_back(bytes, bytes + count);
                if (count == 1)
                {
                    lastRegister = bytes[0];
                }
                return true;
            },
            [this](uint8_t* bytes, size_t count)
            {
                if (!answers || count != 2)
                {
                    return false;
                }
                ++reads;
                if (lastRegister == PicoCalcController::kRegKeyEvent)
                {
                    if (keyEvents.empty())
                    {
                        bytes[0] = 0;
                        bytes[1] = 0;
                    }
                    else
                    {
                        bytes[0] = keyEvents.front().first;
                        bytes[1] = keyEvents.front().second;
                        keyEvents.pop_front();
                    }
                }
                else
                {
                    bytes[0] = lastRegister;
                    bytes[1] = lastRegister == PicoCalcController::kRegBattery ? battery : 0;
                }
                return true;
            },
            [this]() { return now; });
        return controller;
    }
};

}  // namespace

TEST(PicoCalcKeys, SpecialCodesPressTheirKeysAndTypeNothing)
{
    EXPECT_EQ(MapPicoCalcKey(PicoCalcKeyCode::Up).key, Keys::Up);
    EXPECT_EQ(MapPicoCalcKey(PicoCalcKeyCode::Down).key, Keys::Down);
    EXPECT_EQ(MapPicoCalcKey(PicoCalcKeyCode::Left).key, Keys::Left);
    EXPECT_EQ(MapPicoCalcKey(PicoCalcKeyCode::Right).key, Keys::Right);
    EXPECT_EQ(MapPicoCalcKey(PicoCalcKeyCode::Enter).key, Keys::Enter);
    EXPECT_EQ(MapPicoCalcKey(PicoCalcKeyCode::Esc).key, Keys::Esc);
    EXPECT_EQ(MapPicoCalcKey(PicoCalcKeyCode::Backspace).key, Keys::Backspace);
    EXPECT_EQ(MapPicoCalcKey(PicoCalcKeyCode::Tab).key, Keys::Tab);
    EXPECT_EQ(MapPicoCalcKey(PicoCalcKeyCode::Delete).key, Keys::Delete);
    EXPECT_EQ(MapPicoCalcKey(PicoCalcKeyCode::Up).character, 0u);
    EXPECT_EQ(MapPicoCalcKey(PicoCalcKeyCode::Enter).character, 0u);
}

TEST(PicoCalcKeys, PrintableCodesAreTheirCharacterOnTheirKey)
{
    EXPECT_EQ(MapPicoCalcKey('a').key, Keys::A);
    EXPECT_EQ(MapPicoCalcKey('a').character, static_cast<uint32_t>('a'));
    EXPECT_EQ(MapPicoCalcKey('A').key, Keys::A);
    EXPECT_EQ(MapPicoCalcKey('A').character, static_cast<uint32_t>('A'));
    EXPECT_EQ(MapPicoCalcKey(' ').key, Keys::Space);
    EXPECT_EQ(MapPicoCalcKey('7').key, Keys::Num7);
}

TEST(PicoCalcKeys, KeysDekiHasNoIdForAreNotReported)
{
    for (uint8_t code :
         { PicoCalcKeyCode::F1, PicoCalcKeyCode::F10, PicoCalcKeyCode::ShiftLeft, PicoCalcKeyCode::Ctrl,
           PicoCalcKeyCode::Alt, PicoCalcKeyCode::Home, PicoCalcKeyCode::CapsLock, PicoCalcKeyCode::Power })
    {
        EXPECT_EQ(MapPicoCalcKey(code).key, 0u) << static_cast<int>(code);
    }
}

// A read is the register alone, then the reply once the reply delay has
// passed: never both in one Update, so no frame waits for the controller.
TEST(PicoCalcController, AsksInOneUpdateAndCollectsInALaterOne)
{
    FakeController fake;
    auto controller = fake.Make();
    controller->SetReplyDelayMs(16);

    controller->Update();  // the battery goes first
    ASSERT_EQ(fake.writes.size(), 1u);
    EXPECT_EQ(fake.writes[0], std::vector<uint8_t>{ PicoCalcController::kRegBattery });
    EXPECT_EQ(fake.reads, 0);

    fake.now += 10;
    controller->Update();  // too soon for the reply
    EXPECT_EQ(fake.reads, 0);

    fake.now += 6;
    controller->Update();
    EXPECT_EQ(fake.reads, 1);
    EXPECT_EQ(controller->GetBatteryPercent(), 77);
    EXPECT_TRUE(controller->IsCharging());
    EXPECT_TRUE(controller->IsFound());
}

TEST(PicoCalcController, ReadsKeyEventsUntilTheFifoIsEmpty)
{
    FakeController fake;
    fake.keyEvents = { { 1, 'q' }, { 3, 'q' } };
    auto controller = fake.Make();
    controller->SetReplyDelayMs(0);
    std::vector<std::pair<PicoCalcKeyState, uint8_t>> seen;
    controller->SetKeyHandler([&](PicoCalcKeyState state, uint8_t code) { seen.emplace_back(state, code); });

    for (int i = 0; i < 12; ++i)
    {
        controller->Update();
    }
    ASSERT_EQ(seen.size(), 2u);
    EXPECT_EQ(seen[0].first, PicoCalcKeyState::Pressed);
    EXPECT_EQ(seen[1].first, PicoCalcKeyState::Released);
    EXPECT_EQ(seen[1].second, 'q');
}

// With nothing pressed, the keys are asked for once per poll interval, not
// every frame.
TEST(PicoCalcController, PollsAnIdleKeyboardAtItsInterval)
{
    FakeController fake;
    auto controller = fake.Make();
    controller->SetReplyDelayMs(0);
    controller->SetPollIntervalMs(20);
    for (int i = 0; i < 6; ++i)
    {
        controller->Update();  // battery, then the first polls, until the FIFO says empty
    }
    const size_t before = fake.writes.size();
    for (int i = 0; i < 5; ++i)
    {
        fake.now += 1;
        controller->Update();
    }
    EXPECT_EQ(fake.writes.size(), before);
    fake.now += 20;
    controller->Update();
    EXPECT_EQ(fake.writes.size(), before + 1);
}

TEST(PicoCalcController, WritesSetRegistersWithTheWriteFlagAndTheLatestValue)
{
    FakeController fake;
    auto controller = fake.Make();
    controller->SetScreenBacklight(64);
    controller->SetScreenBacklight(128);  // replaces the 64 not sent yet
    controller->SetKeyboardBacklight(32);
    controller->PowerOff(10);

    for (int i = 0; i < 3; ++i)
    {
        controller->Update();
    }
    ASSERT_GE(fake.writes.size(), 3u);
    EXPECT_EQ(fake.writes[0], (std::vector<uint8_t>{ 0x85, 128 }));
    EXPECT_EQ(fake.writes[1], (std::vector<uint8_t>{ 0x8A, 32 }));
    EXPECT_EQ(fake.writes[2], (std::vector<uint8_t>{ 0x8E, 10 }));
}

TEST(PicoCalcController, NoBatteryReadsAsUnknown)
{
    FakeController fake;
    fake.battery = 0;
    auto controller = fake.Make();
    controller->SetReplyDelayMs(0);
    controller->Update();
    controller->Update();
    EXPECT_EQ(controller->GetBatteryPercent(), -1);
    EXPECT_FALSE(controller->IsCharging());
}

// A controller that does not answer is asked again after a pause, not every
// frame: each failed try costs a bus timeout.
TEST(PicoCalcController, AMissingControllerIsAskedAgainAfterAPause)
{
    FakeController fake;
    fake.answers = false;
    auto controller = fake.Make();
    for (int i = 0; i < 100; ++i)
    {
        controller->Update();
        fake.now += 1;
    }
    EXPECT_FALSE(controller->IsFound());

    fake.answers = true;
    fake.now += 500;
    controller->Update();
    EXPECT_EQ(fake.writes.size(), 1u);
    EXPECT_TRUE(controller->IsFound());
}

TEST(PicoCalcKeyboard, AKeyIsDownFromItsPressToItsRelease)
{
    FakeController fake;
    PicoCalcKeyboard keyboard(fake.Make());
    std::vector<DekiInput::InputEvent> events;
    keyboard.RegisterEventCallback([&](const DekiInput::InputEvent& event) { events.push_back(event); });
    ASSERT_TRUE(keyboard.Initialize());

    keyboard.OnKeyEvent(PicoCalcKeyState::Pressed, PicoCalcKeyCode::Right);
    EXPECT_TRUE(keyboard.IsKeyPressed(Keys::Right));
    keyboard.OnKeyEvent(PicoCalcKeyState::Held, PicoCalcKeyCode::Right);
    EXPECT_TRUE(keyboard.IsKeyPressed(Keys::Right));
    keyboard.OnKeyEvent(PicoCalcKeyState::Released, PicoCalcKeyCode::Right);
    EXPECT_FALSE(keyboard.IsKeyPressed(Keys::Right));

    ASSERT_EQ(events.size(), 2u);
    EXPECT_EQ(events[0].type, DekiInput::InputEventType::KeyDown);
    EXPECT_EQ(events[0].key, Keys::Right);
    EXPECT_EQ(events[1].type, DekiInput::InputEventType::KeyUp);
}

TEST(PicoCalcKeyboard, ARepeatedPressTypesAgainWhileTheKeyStaysDown)
{
    FakeController fake;
    PicoCalcKeyboard keyboard(fake.Make());
    std::vector<DekiInput::InputEvent> events;
    keyboard.RegisterEventCallback([&](const DekiInput::InputEvent& event) { events.push_back(event); });
    keyboard.Initialize();

    keyboard.OnKeyEvent(PicoCalcKeyState::Pressed, 'x');
    keyboard.OnKeyEvent(PicoCalcKeyState::Pressed, 'x');
    ASSERT_EQ(events.size(), 2u);
    EXPECT_EQ(events[1].character, static_cast<uint32_t>('x'));
    keyboard.OnKeyEvent(PicoCalcKeyState::Released, 'x');
    EXPECT_FALSE(keyboard.IsKeyPressed(Keys::X));
}

// '!' pressed with Shift, then released as '1' once Shift was let go: the
// printable keys that are down are let go, so '!' does not stick.
TEST(PicoCalcKeyboard, AReleaseNamedDifferentlyLetsNoPrintableKeyStick)
{
    FakeController fake;
    PicoCalcKeyboard keyboard(fake.Make());
    keyboard.Initialize();

    keyboard.OnKeyEvent(PicoCalcKeyState::Pressed, PicoCalcKeyCode::Up);
    keyboard.OnKeyEvent(PicoCalcKeyState::Pressed, '!');
    keyboard.OnKeyEvent(PicoCalcKeyState::Released, '1');
    EXPECT_FALSE(keyboard.IsKeyPressed('!'));
    EXPECT_TRUE(keyboard.IsKeyPressed(Keys::Up));  // arrows are named the same both ways
}

TEST(PicoCalc, TheGameApiReachesTheControllerWhileTheKeyboardLives)
{
    FakeController fake;
    {
        PicoCalcKeyboard keyboard(fake.Make());
        PicoCalc::SetScreenBacklight(200);
        keyboard.Initialize();
        keyboard.Update();
        ASSERT_FALSE(fake.writes.empty());
        EXPECT_EQ(fake.writes[0], (std::vector<uint8_t>{ 0x85, 200 }));
    }
    // Gone with the keyboard: the calls do nothing and the battery is unknown.
    PicoCalc::SetScreenBacklight(10);
    EXPECT_EQ(PicoCalc::GetBatteryPercent(), -1);
    EXPECT_FALSE(PicoCalc::IsAvailable());
}
