# Package descriptor for deki-engine auto-discovery
set(PACKAGE_DISPLAY_NAME "ClockworkPi")
set(PACKAGE_PREFIX "DekiClockworkPi")
set(PACKAGE_UPPER "CLOCKWORKPI")
set(PACKAGE_TARGET "deki-clockworkpi")
set(PACKAGE_FILE_PREFIX "ClockworkPi")
set(PACKAGE_SOURCES
    PicoCalcKeys.cpp
    PicoCalcController.cpp
    PicoCalcKeyboard.cpp
    PicoCalc.cpp
    PicoCalcSetup.cpp
)
set(PACKAGE_ENTRY ClockworkPiPackage.cpp)
set(PACKAGE_LINK_DEPS deki-input deki-i2c)
