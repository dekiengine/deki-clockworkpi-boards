// Package entry point for the deki-clockworkpi DLL. Exports the standard Deki
// plugin interface, so the editor can load the DLL and find its components.

#include "ClockworkPiPackage.h"
#include <deki/interop/Plugin.h>
#include <deki/reflection/ComponentRegistry.h>
#include <deki/reflection/ComponentFactory.h>

extern void DekiClockworkPiRegisterComponents();
extern int DekiClockworkPiGetAutoComponentCount();
extern const Deki::ComponentMeta* DekiClockworkPiGetAutoComponentMeta(int index);

namespace DekiClockworkPi
{

#ifdef DEKI_EDITOR

static bool s_ClockworkPiRegistered = false;

// The exports below are C symbols at global scope; the package's own
// registration helpers and statics live in its namespace.
using namespace DekiClockworkPi;

extern "C"
{
    // Registers the package's components once. Returns how many there are.
    DEKI_CLOCKWORKPI_API int DekiClockworkPiEnsureRegistered(void)
    {
        if (s_ClockworkPiRegistered)
        {
            return ::DekiClockworkPiGetAutoComponentCount();
        }
        s_ClockworkPiRegistered = true;

        // Generated: registers every ClockworkPi component with ComponentRegistry and ComponentFactory.
        ::DekiClockworkPiRegisterComponents();

        return ::DekiClockworkPiGetAutoComponentCount();
    }

    // =============================================================================
    // Plugin metadata
    // =============================================================================

    DEKI_PLUGIN_API const char* DekiPluginGetName(void)
    {
        return "Deki ClockworkPi Package";
    }

    DEKI_PLUGIN_API const char* DekiPluginGetVersion(void)
    {
#ifdef DEKI_PACKAGE_VERSION
        return DEKI_PACKAGE_VERSION;
#else
        return "0.0.0-dev";
#endif
    }

    DEKI_PLUGIN_API int DekiPluginInit(void)
    {
        return 0;
    }

    DEKI_PLUGIN_API void DekiPluginShutdown(void)
    {
        s_ClockworkPiRegistered = false;
    }

    DEKI_PLUGIN_API int DekiPluginGetComponentCount(void)
    {
        return ::DekiClockworkPiGetAutoComponentCount();
    }

    DEKI_PLUGIN_API const Deki::ComponentMeta* DekiPluginGetComponentMeta(int index)
    {
        return ::DekiClockworkPiGetAutoComponentMeta(index);
    }

    DEKI_PLUGIN_API void DekiPluginRegisterComponents(void)
    {
        DekiClockworkPiEnsureRegistered();
    }

    // =============================================================================
    // Package-specific API, with names that do not clash when DLLs link each other
    // =============================================================================

    DEKI_CLOCKWORKPI_API const char* DekiClockworkPiGetName(void)
    {
        return "ClockworkPi";
    }

}  // extern "C"

#else  // !DEKI_EDITOR - Runtime registration

// Components are registered by the generated ::DekiClockworkPiRegisterComponents(),
// called from DekiRegisterProjectPackages(). The keyboard controller starts as
// a boot SetupComponent (PicoCalcSetup).

#endif  // DEKI_EDITOR
}  // namespace DekiClockworkPi
