#include <windows.h>
#include <cstdio>
#include <cstring>

#define REAL_DXGI_PATH "C:\\Windows\\System32\\dxgi.dll"
#define SCRIPTHOOK_DLL "ScriptHookWDL.dll"

static FILE* g_log = nullptr;

static void LogMsg(const char* fmt, ...) {
    if (!g_log) {
        char path[MAX_PATH];
        GetModuleFileNameA(NULL, path, MAX_PATH);
        char* slash = strrchr(path, '\\');
        if (slash) strcpy(slash + 1, "dxgi_proxy_log.txt");
        else strcpy(path, "dxgi_proxy_log.txt");
        g_log = fopen(path, "a");
    }
    if (g_log) {
        va_list args;
        va_start(args, fmt);
        vfprintf(g_log, fmt, args);
        va_end(args);
        fflush(g_log);
    }
}

HMODULE g_hRealDxgi = NULL;

void LoadScriptHook() {
    char dllPath[MAX_PATH] = {0};
    GetModuleFileNameA(NULL, dllPath, MAX_PATH);
    char* slash = strrchr(dllPath, '\\');
    if (slash) *(slash + 1) = '\0';
    strcat(dllPath, SCRIPTHOOK_DLL);

    LogMsg("Loading %s\n", dllPath);
    HMODULE hMod = LoadLibraryA(dllPath);
    if (!hMod)
        LogMsg("Failed! GLE=%lu\n", GetLastError());
    else
        LogMsg("ScriptHook loaded OK.\n");
}

HMODULE EnsureRealDxgi() {
    if (!g_hRealDxgi) {
        g_hRealDxgi = LoadLibraryA(REAL_DXGI_PATH);
        if (!g_hRealDxgi)
            LogMsg("Failed to load real dxgi.dll! GLE=%lu\n", GetLastError());
    }
    return g_hRealDxgi;
}

typedef HRESULT(WINAPI* CreateDXGIFactory_t)(REFIID, void**);
typedef HRESULT(WINAPI* CreateDXGIFactory1_t)(REFIID, void**);
typedef HRESULT(WINAPI* CreateDXGIFactory2_t)(UINT, REFIID, void**);
typedef HRESULT(WINAPI* DXGIGetDebugInterface1_t)(REFIID, void**);

extern "C" __declspec(dllexport)
HRESULT WINAPI CreateDXGIFactory(REFIID riid, void** ppFactory) {
    LogMsg("CreateDXGIFactory called\n");
    auto real = (CreateDXGIFactory_t)GetProcAddress(EnsureRealDxgi(), "CreateDXGIFactory");
    if (!real) return E_FAIL;
    return real(riid, ppFactory);
}

extern "C" __declspec(dllexport)
HRESULT WINAPI CreateDXGIFactory1(REFIID riid, void** ppFactory) {
    LogMsg("CreateDXGIFactory1 called\n");
    auto real = (CreateDXGIFactory1_t)GetProcAddress(EnsureRealDxgi(), "CreateDXGIFactory1");
    if (!real) return E_FAIL;
    return real(riid, ppFactory);
}

extern "C" __declspec(dllexport)
HRESULT WINAPI CreateDXGIFactory2(UINT Flags, REFIID riid, void** ppFactory) {
    LogMsg("CreateDXGIFactory2 called\n");
    auto real = (CreateDXGIFactory2_t)GetProcAddress(EnsureRealDxgi(), "CreateDXGIFactory2");
    if (!real) return E_FAIL;
    return real(Flags, riid, ppFactory);
}

extern "C" __declspec(dllexport)
HRESULT WINAPI DXGIGetDebugInterface1(REFIID riid, void** ppDebug) {
    LogMsg("DXGIGetDebugInterface1 called\n");
    auto real = (DXGIGetDebugInterface1_t)GetProcAddress(EnsureRealDxgi(), "DXGIGetDebugInterface1");
    if (!real) return E_FAIL;
    return real(riid, ppDebug);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hinstDLL);
        LogMsg("=== dxgi.dll proxy loaded ===\n");
        LoadScriptHook();
    }
    return TRUE;
}
