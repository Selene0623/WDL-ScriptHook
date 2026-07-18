#include <windows.h>
#include <thread>

void log(const char* msg) {
    FILE* f = fopen("ScriptHookWDL_log.txt", "a");
    if (f) {
        fprintf(f, "%s\n", msg);
        fclose(f);
    }
}

void ShowMenuThread() {
    log("ShowMenuThread started.");
    while (true) {
        if (GetAsyncKeyState(VK_F1) & 1) {
            log("F1 pressed: Show Menu");
            MessageBoxA(NULL,
                "Mod Menu:\n\n1. Godmode (F2)\n2. Give Money (F3)\n\nPress F2 or F3 for features!",
                "ScriptHook Mod Menu", MB_OK);
        }
        if (GetAsyncKeyState(VK_F2) & 1) {
            log("F2 pressed: Godmode");
            MessageBoxA(NULL, "Godmode Activated! (stub)", "ScriptHook", MB_OK);
        }
        if (GetAsyncKeyState(VK_F3) & 1) {
            log("F3 pressed: Give Money");
            MessageBoxA(NULL, "Money Given! (stub)", "ScriptHook", MB_OK);
        }
        Sleep(100);
    }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        log("DLL_PROCESS_ATTACH.");
        DisableThreadLibraryCalls(hModule);
        std::thread(ShowMenuThread).detach();
    }
    return TRUE;
}