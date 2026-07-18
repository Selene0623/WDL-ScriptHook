#include <windows.h>
#include <unknwn.h>
#include <cstdio>
#include <fstream>

#define REAL_DINPUT8_PATH "C:\\Windows\\System32\\dinput8.dll"
#define SCRIPTHOOK_DLL "ScriptHookWDL.dll"
#define LOG_PATH "dllproxy_log.txt"

typedef HRESULT(WINAPI* DirectInput8Create_t)(HINSTANCE, DWORD, REFIID, LPVOID*, LPUNKNOWN);
HMODULE g_hRealDinput8 = NULL;

// Forwarder for DirectInput8Create
extern "C" __declspec(dllexport)
HRESULT WINAPI DirectInput8Create(
    HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter)
{
    std::ofstream(LOG_PATH, std::ios::app) << "DirectInput8Create called!\n";
    if (!g_hRealDinput8) {
        g_hRealDinput8 = LoadLibraryA(REAL_DINPUT8_PATH);
        if (!g_hRealDinput8) {
            std::ofstream(LOG_PATH, std::ios::app) << "Failed to load real dinput8.dll!\n";
            return E_FAIL;
        }
    }
    auto realProc = (DirectInput8Create_t)GetProcAddress(g_hRealDinput8, "DirectInput8Create");
    if (!realProc) {
        std::ofstream(LOG_PATH, std::ios::app) << "Failed to find DirectInput8Create export!\n";
        return E_FAIL;
    }
    std::ofstream(LOG_PATH, std::ios::app) << "Forwarding DirectInput8Create...\n";
    return realProc(hinst, dwVersion, riidltf, ppvOut, punkOuter);
}

// Helper to get this DLL's folder
void GetOwnFolder(char* outPath, size_t len) {
    GetModuleFileNameA(GetModuleHandleA("dinput8.dll"), outPath, (DWORD)len);
    char* lastSlash = strrchr(outPath, '\\');
    if (lastSlash) *(lastSlash + 1) = '\0';
}

// Loads ScriptHookWDL.dll from the same folder
void LoadScriptHook() {
    char dllPath[MAX_PATH] = {0};
    GetOwnFolder(dllPath, sizeof(dllPath));
    strcat_s(dllPath, SCRIPTHOOK_DLL);

    std::ofstream(LOG_PATH, std::ios::app) << "Trying to load " << dllPath << "\n";

    HMODULE hMod = LoadLibraryA(dllPath);
    if (!hMod) {
        char msg[512];
        snprintf(msg, sizeof(msg), "Failed to load: %s\nGLE=%lu", dllPath, GetLastError());
        std::ofstream(LOG_PATH, std::ios::app) << "Failed to load ScriptHook DLL! GLE=" << GetLastError() << "\n";
        MessageBoxA(NULL, msg, "ScriptHook Loader", MB_OK | MB_ICONERROR);
    } else {
        std::ofstream(LOG_PATH, std::ios::app) << "Successfully loaded ScriptHook DLL.\n";
    }
}

// DLL Entry Point
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        std::ofstream(LOG_PATH, std::ios::app) << "dinput8.dll proxy loaded!\n";
        LoadScriptHook();
        g_hRealDinput8 = LoadLibraryA(REAL_DINPUT8_PATH);
        std::ofstream(LOG_PATH, std::ios::app) << "Loaded real dinput8.dll.\n";
    }
    return TRUE;
}