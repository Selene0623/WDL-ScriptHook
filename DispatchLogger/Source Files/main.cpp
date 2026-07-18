#include <windows.h>
#include <cstdint>
#include <cstdio>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <mutex>
#include <MinHook.h>

#pragma comment(lib, "libMinHook.x64.lib")

// ScriptHook dispatch function at RVA 0x1c87f0 in scripthook.dll
// Signature: void* __fastcall sub_1801C87F0(void* object, uint32_t hash)
// The game calls this with (object, hash) and gets back a function pointer

static FILE* g_log = nullptr;
static std::mutex g_mutex;

// Track all hashes we see and their resolved addresses
struct HashRecord {
    uint32_t hash;
    void* resolved_addr;
    uint32_t call_count;
};
static std::unordered_map<uint32_t, HashRecord> g_hash_map;

// Original function pointer
typedef void* (*DispatchFunc)(void* object, uint32_t hash);
static DispatchFunc g_original_dispatch = nullptr;

// The hooked dispatch function
static void* __fastcall hkDispatch(void* object, uint32_t hash) {
    // Call the original
    void* result = g_original_dispatch(object, hash);

    // Log it
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        auto it = g_hash_map.find(hash);
        if (it != g_hash_map.end()) {
            it->second.call_count++;
        } else {
            g_hash_map[hash] = {hash, result, 1};
        }
    }

    return result;
}

// Dump all collected hashes to a file
static void DumpHashes() {
    if (!g_log) return;

    fprintf(g_log, "=== Dispatch Hash Log ===\n");
    fprintf(g_log, "Total unique hashes: %zu\n\n", g_hash_map.size());

    // Sort by hash for easy comparison with offsets.txt
    std::vector<HashRecord> sorted;
    sorted.reserve(g_hash_map.size());
    for (auto& [h, r] : g_hash_map) {
        sorted.push_back(r);
    }
    // Manual sort since we're targeting older MSVC
    for (size_t i = 0; i < sorted.size(); i++) {
        for (size_t j = i + 1; j < sorted.size(); j++) {
            if (sorted[j].hash < sorted[i].hash) {
                HashRecord tmp = sorted[i];
                sorted[i] = sorted[j];
                sorted[j] = tmp;
            }
        }
    }

    fprintf(g_log, "// Hash -> Resolved Address (from scripthook.dll dispatch)\n");
    fprintf(g_log, "// Compare against offsets.txt to find missing entries\n");
    fprintf(g_log, "// Format: 0x%08X -> 0x%p (called %u times)\n", 0, (void*)0, 0u);
    fprintf(g_log, "\n");

    for (auto& rec : sorted) {
        fprintf(g_log, "0x%08X -> 0x%p (called %u times)\n",
                rec.hash, rec.resolved_addr, rec.call_count);
    }

    fflush(g_log);
}

// Also dump the game module's base address and PE info for memory dump tool
static void DumpModuleInfo() {
    HMODULE game_dll = GetModuleHandleA("DuniaDemo_clang_64_dx12.dll");
    HMODULE dx11_dll = GetModuleHandleA("DuniaDemo_clang_64_dx11.dll");
    HMODULE sh_dll = GetModuleHandleA("scripthook.dll");

    FILE* info = fopen("DispatchLogger_module_info.txt", "w");
    if (!info) return;

    fprintf(info, "=== Module Info ===\n");
    fprintf(info, "PID: %u\n", GetCurrentProcessId());

    if (game_dll) {
        MODULEINFO mi;
        GetModuleInformation(GetCurrentProcess(), game_dll, &mi, sizeof(mi));
        fprintf(info, "DX12 DLL: base=0x%p size=0x%x\n", mi.lpBaseOfDll, mi.SizeOfImage);
    }
    if (dx11_dll) {
        MODULEINFO mi;
        GetModuleInformation(GetCurrentProcess(), dx11_dll, &mi, sizeof(mi));
        fprintf(info, "DX11 DLL: base=0x%p size=0x%x\n", mi.lpBaseOfDll, mi.SizeOfImage);
    }
    if (sh_dll) {
        MODULEINFO mi;
        GetModuleInformation(GetCurrentProcess(), sh_dll, &mi, sizeof(mi));
        fprintf(info, "ScriptHook: base=0x%p size=0x%x\n", mi.lpBaseOfDll, mi.SizeOfImage);

        // ScriptHook dispatch function is at offset 0x1c87f0
        void* dispatch_addr = (char*)mi.lpBaseOfDll + 0x1c87f0;
        fprintf(info, "Dispatch func: 0x%p (base + 0x1c87f0)\n", dispatch_addr);
    }

    fflush(info);
    fclose(info);
}

static DWORD WINAPI MainThread(LPVOID param) {
    // Wait for game modules to load
    Sleep(3000);

    g_log = fopen("DispatchLogger_output.txt", "w");
    if (!g_log) {
        MessageBoxA(NULL, "Cannot open DispatchLogger_output.txt", "DispatchLogger", MB_OK);
        return 1;
    }

    fprintf(g_log, "DispatchLogger starting...\n");
    fflush(g_log);

    // Dump module info
    DumpModuleInfo();

    // Find the ScriptHook dispatch function
    HMODULE sh_dll = GetModuleHandleA("scripthook.dll");
    if (!sh_dll) {
        fprintf(g_log, "ERROR: scripthook.dll not found\n");
        fflush(g_log);
        return 1;
    }

    void* dispatch_addr = (char*)sh_dll + 0x1c87f0;
    fprintf(g_log, "Dispatch function at: 0x%p\n", dispatch_addr);
    fprintf(g_log, "Hooking with MinHook...\n");
    fflush(g_log);

    // Initialize MinHook
    MH_STATUS status = MH_Initialize();
    if (status != MH_OK) {
        fprintf(g_log, "MH_Initialize failed: %d\n", status);
        fflush(g_log);
        return 1;
    }

    // Create hook
    status = MH_CreateHook(dispatch_addr, (LPVOID)hkDispatch, (LPVOID*)&g_original_dispatch);
    if (status != MH_OK) {
        fprintf(g_log, "MH_CreateHook failed: %d\n", status);
        fflush(g_log);
        return 1;
    }

    // Enable hook
    status = MH_EnableHook(dispatch_addr);
    if (status != MH_OK) {
        fprintf(g_log, "MH_EnableHook failed: %d\n", status);
        fflush(g_log);
        return 1;
    }

    fprintf(g_log, "Hook installed successfully! Logging dispatch calls...\n");
    fprintf(g_log, "Press F12 to dump hashes and exit.\n\n");
    fflush(g_log);

    // Poll for F12 to dump
    while (true) {
        if (GetAsyncKeyState(VK_F12) & 0x8000) {
            break;
        }
        Sleep(100);

        // Auto-dump every 30 seconds
        static DWORD last_dump = 0;
        DWORD now = GetTickCount();
        if (now - last_dump > 30000) {
            DumpHashes();
            last_dump = now;
        }
    }

    // Final dump
    fprintf(g_log, "\n=== FINAL DUMP ===\n");
    DumpHashes();

    MH_DisableHook(dispatch_addr);
    MH_Uninitialize();

    fclose(g_log);
    g_log = nullptr;

    MessageBoxA(NULL, "Hashes dumped to DispatchLogger_output.txt", "DispatchLogger", MB_OK);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID reserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        CreateThread(NULL, 0, MainThread, hModule, 0, NULL);
    }
    return TRUE;
}
