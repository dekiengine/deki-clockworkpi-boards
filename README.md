# Deki ClockworkPi Boards

Build target for the ClockworkPi PicoCalc with a Raspberry Pi Pico (RP2040) in it, the module the kit ships with. Install the package, pick **ClockworkPi PicoCalc (Pico)** in the Build panel, and **Build** makes one `.uf2` with the game and its data; **Flash** puts it on the Pico over its micro-USB port.

| Target | Device | Screen | Input |
| --- | --- | --- | --- |
| `picocalc_pico` | ClockworkPi PicoCalc, Raspberry Pi Pico or Pico H | 320x320 ILI9488 on SPI, drawn in strips | The keyboard, through its controller on I2C, as keys in `deki-input` |

The PicoCalc was provided by ClockworkPi.

[Deki](https://dekiengine.com) is a modular C++ engine with a visual editor, made first for embedded devices. Every capability is a package, and a board is one too: this package has the PicoCalc's platform, its boot scene and its keyboard; the Pico itself (the build, the buses, the SD card) is `deki-raspberrypi-integration`.

## Install

Package Manager in the Deki Editor, or `DekiEditor --packages-add deki-clockworkpi-boards <project>`. It brings `deki-raspberrypi-integration`, `deki-display`, `deki-input` and the bus packages with it. Then pick the target in the Build panel, or:

```
DekiEditor --tool set_active_platform --args '{"platform_id":"picocalc_pico"}' <project>
```

## What the boot scene sets up

| Step | What it does |
| --- | --- |
| SPI Bus (spi1) | SCK 10, MOSI 11, MISO 12 |
| Display | ST7365P (driven as an ILI9488: same commands), 320x320, CS 13, DC 14, RST 15, 18 bits a pixel on the wire. Started as ClockworkPi's PicoMite does: the ILI9488 list in their example leaves a slowly fading ghost |
| I2C Bus (i2c1) | SDA 6, SCL 7, 10 kHz, the speed ClockworkPi's examples use |
| Keyboard, backlights, battery | `PicoCalcSetup`: the keyboard controller at 0x1F |
| SD Card (spi0) | SCK 18, MOSI 19, MISO 16, CS 17, card detect 22, at 15.6 MHz. Files on it are `S:/` |

The Pico H has 264 KB of RAM and the screen's framebuffer would take 200 KB, so frames are drawn and sent a strip of rows at a time (the Rendering setting `frameMode` picks that by itself). Only the rows that changed are sent.

## Keys

A key is down from its press until its release. Arrows, Enter, Esc, Backspace, Tab, Delete, letters, digits and symbols are reported; a key that types something carries what it typed (`InputEvent::character`: `A` with Shift, `a` without, both on `Keys::A`).

F1-F10, Shift, Ctrl, Alt, Sym, Home, End, Page Up, Page Down, Insert and Caps Lock are not reported: `deki-input` has no key ids for them yet.

## Backlights, battery and power

```cpp
#include "PicoCalc.h"

DekiClockworkPi::PicoCalc::SetScreenBacklight(128);   // 16 to 240, in steps of 16
DekiClockworkPi::PicoCalc::SetKeyboardBacklight(0);   // off, up to 224 in steps of 32
int percent = DekiClockworkPi::PicoCalc::GetBatteryPercent();  // -1 when unknown
bool charging = DekiClockworkPi::PicoCalc::IsCharging();
DekiClockworkPi::PicoCalc::PowerOff(6);               // off after 6 seconds
```

All of it goes through the keyboard controller. Each read takes about 4 ms on its 10 kHz bus, so the controller is asked in one frame and its answer is collected in a later one: polling it never holds a frame up. `PicoCalcSetup` in the boot scene can also set both backlights at boot.

Power the PicoCalc on from its batteries before plugging USB in, or the battery reads as unknown (ClockworkPi's note).

## Not done yet

- The engine's audio (deki-audio) on the speakers. `PicoCalcSpeaker.h` plays samples a game makes itself, on GP26 and GP27.
- The 8 MB PSRAM on the mainboard. It is not mapped into the Pico's memory, so the engine cannot place things in it.
- Other modules in the PicoCalc's socket (Pico 2, Pico 2 W, Luckfox Lyra).
