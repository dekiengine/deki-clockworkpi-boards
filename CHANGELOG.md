# Changelog

Notable changes to `deki-clockworkpi-boards`. Engine and editor changes are in the
[engine changelog](https://github.com/dekiengine/deki-engine/blob/master/CHANGELOG.md).

## Unreleased

### Added
- **`PicoCalcSpeaker.h`**: the PicoCalc's speakers (GP26 and GP27) for a game
  that makes its own samples: `StartPicoCalcSpeaker(sampleRate, fill,
  context)` and `StopPicoCalcSpeaker()`, through `PicoPwmAudio`.
- **The ClockworkPi PicoCalc with a Raspberry Pi Pico** as a build target,
  `picocalc_pico`, built with `deki-raspberrypi-integration`. Its boot scene
  sets up the ILI9488 screen (320x320, ClockworkPi's init sequence), the
  keyboard controller and the SD card.
- **`PicoCalcSetup`**: the keyboard controller (an STM32 at 0x1F on I2C). Keys
  arrive through `deki-input`, down from press to release. Reads are asked in
  one frame and collected in a later one, so polling never holds a frame up.
- **`PicoCalc`**: screen and keyboard backlights, battery percent and charging,
  and power off, for game code.
