#pragma once

// Main header of the ClockworkPi package: build targets for ClockworkPi
// devices. The PicoCalc's keyboard controller (keys, backlights, battery and
// power) is driven by PicoCalcSetup in its boot scene; the screen, the SD card
// and the buses are deki-display's, deki-sdcard's and the Raspberry Pi
// package's.

#ifdef _WIN32
#ifdef DEKI_CLOCKWORKPI_EXPORTS
#define DEKI_CLOCKWORKPI_API __declspec(dllexport)
#else
#define DEKI_CLOCKWORKPI_API __declspec(dllimport)
#endif
#else
#define DEKI_CLOCKWORKPI_API __attribute__((visibility("default")))
#endif
