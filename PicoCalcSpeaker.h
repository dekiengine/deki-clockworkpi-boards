#pragma once

// The PicoCalc's speakers: two PWM pins of the Pico, each through a filter
// to the amplifier. Mono here: both carry the same sound.
//
// Not the engine's audio (deki-audio does not run on a Pico yet): a game
// that makes its own samples gives a function that fills a buffer with
// them, and it is called from an interrupt. See PicoPwmAudio.h for what
// that function may do.

#include "pico/audio/PicoPwmAudio.h"  // from deki-raspberrypi-integration

namespace DekiClockworkPi
{

constexpr int kPicoCalcSpeakerLeft = 26;
constexpr int kPicoCalcSpeakerRight = 27;

/// Starts the speakers at `sampleRate` samples a second, asking `fill` for
/// them. False off a Pico, or when they are already on.
inline bool StartPicoCalcSpeaker(int sampleRate, DekiRaspberryPi::PicoPwmAudio::FillFn fill, void* context)
{
    return DekiRaspberryPi::PicoPwmAudio::Start(kPicoCalcSpeakerLeft, kPicoCalcSpeakerRight, sampleRate, fill, context);
}

inline void StopPicoCalcSpeaker()
{
    DekiRaspberryPi::PicoPwmAudio::Stop();
}

}  // namespace DekiClockworkPi
