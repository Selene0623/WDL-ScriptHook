#include <windows.h>
#include <thread>

#include "console/console.hpp"
#include "hooks/hooks.hpp"
#include "utils/utils.hpp"
#include "MinHook.h"
#include "ScriptEngine.h"

DWORD WINAPI OnProcessAttach(LPVOID lpParam);
DWORD WINAPI OnProcessDetach(LPVOID lpParam);

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hinstDLL);
        RawLog("=== ScriptHookWDL DllMain called ===\n");
        U::SetRenderingBackend(DIRECTX12);
        RawLog("=== Spawning init thread ===\n");
        CreateThread(NULL, 0, OnProcessAttach, hinstDLL, 0, NULL);
    } else if (fdwReason == DLL_PROCESS_DETACH && !lpReserved) {
        OnProcessDetach(NULL);
    }
    return TRUE;
}

DWORD WINAPI OnProcessAttach(LPVOID lpParam) {
    RawLog("=== OnProcessAttach thread started ===\n");
    Console::Alloc();
    LOG("[+] ScriptHookWDL loaded (backend: %s)\n", U::RenderingBackendToStr());
    RawLog("=== Calling MH_Initialize ===\n");

    MH_Initialize();
    RawLog("=== Calling H::Init ===\n");
    H::Init();
    RawLog("=== Calling ScriptEngine::Initialize ===\n");
    ScriptEngine::Initialize();
    RawLog("=== Init complete ===\n");

    return 0;
}

DWORD WINAPI OnProcessDetach(LPVOID lpParam) {
    ScriptEngine::Shutdown();
    H::Free();
    MH_Uninitialize();
    Console::Free();
    return 0;
}
