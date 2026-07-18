#include "menu.hpp"
#include "../dependencies/imgui/imgui.h"
#include "../dependencies/imgui/imgui_impl_win32.h"
#include <TlHelp32.h>
#include <Windows.h>
#include <vector>

namespace ig = ImGui;

namespace Menu {
    // Bloodlines trainer variables
    static HANDLE hGameProcess = nullptr;
    static DWORD gameProcessId = 0;
    static bool bBloodlinesMode = false;
    static bool bSaveFilePatch = false;
    static bool bConnected = false;
    static uintptr_t bloodlinesAddr = 0;
    static uintptr_t saveFileAddr = 0;
    static uintptr_t moduleBase = 0;
    static SIZE_T moduleSize = 0;

    // Get module base address for external process
    uintptr_t GetModuleBaseAddress(DWORD processId, const wchar_t* moduleName) {
        uintptr_t moduleBase = 0;
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processId);
        if (snapshot != INVALID_HANDLE_VALUE) {
            MODULEENTRY32W moduleEntry;
            moduleEntry.dwSize = sizeof(moduleEntry);

            if (Module32FirstW(snapshot, &moduleEntry)) {
                do {
                    if (!_wcsicmp(moduleEntry.szModule, moduleName)) {
                        moduleBase = (uintptr_t)moduleEntry.modBaseAddr;
                        moduleSize = moduleEntry.modBaseSize;
                        break;
                    }
                } while (Module32NextW(snapshot, &moduleEntry));
            }
        }
        CloseHandle(snapshot);
        return moduleBase;
    }

    // Pattern scanning function
    uintptr_t FindPattern(HANDLE process, uintptr_t moduleBase, SIZE_T moduleSize, const char* pattern, size_t patternSize) {
        std::vector<BYTE> buffer(moduleSize);
        SIZE_T bytesRead;

        if (!ReadProcessMemory(process, (LPCVOID)moduleBase, buffer.data( ), moduleSize, &bytesRead)) {
            return 0;
        }

        for (size_t i = 0; i < bytesRead - patternSize; i++) {
            bool found = true;
            for (size_t j = 0; j < patternSize; j++) {
                if (buffer[i + j] != (BYTE)pattern[j]) {
                    found = false;
                    break;
                }
            }
            if (found) {
                return moduleBase + i;
            }
        }
        return 0;
    }

    // Find Watch Dogs Legion process and scan for patterns
    void ConnectToGame( ) {
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot != INVALID_HANDLE_VALUE) {
            PROCESSENTRY32 entry;
            entry.dwSize = sizeof(entry);
            if (Process32First(snapshot, &entry)) {
                do {
                    if (strcmp(entry.szExeFile, "WatchDogsLegion.exe") == 0) {
                        gameProcessId = entry.th32ProcessID;
                        hGameProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, gameProcessId);
                        if (hGameProcess) {
                            // Get DuniaDemo_clang_64_dx12.dll base address
                            moduleBase = GetModuleBaseAddress(gameProcessId, L"DuniaDemo_clang_64_dx12.dll");
                            if (moduleBase) {
                                // Scan for Bloodlines pattern
                                const char bloodlinesPattern[] = "\x80\xBE\x70\x06\x00\x00\x01\x74";
                                bloodlinesAddr = FindPattern(hGameProcess, moduleBase, moduleSize, bloodlinesPattern, 8);

                                // Scan for Save File pattern
                                const char saveFilePattern[] = "\x8B\x87\x1C\x01\x00\x00\x83\xF8\x01\x48";
                                saveFileAddr = FindPattern(hGameProcess, moduleBase, moduleSize, saveFilePattern, 10);

                                bConnected = (bloodlinesAddr != 0 && saveFileAddr != 0);
                            }
                        }
                        break;
                    }
                } while (Process32Next(snapshot, &entry));
            }
        }
        CloseHandle(snapshot);
    }

    void ApplyBloodlinesPatch( ) {
        if (!hGameProcess || !bloodlinesAddr)
            return;

        if (bBloodlinesMode) {
            // Enable patch
            BYTE enablePatch[] = {0xC6, 0x86, 0x70, 0x06, 0x00, 0x00, 0x01};
            WriteProcessMemory(hGameProcess, (LPVOID)bloodlinesAddr, enablePatch, sizeof(enablePatch), nullptr);
            BYTE nopPatch = 0x90;
            WriteProcessMemory(hGameProcess, (LPVOID)(bloodlinesAddr + 7), &nopPatch, 1, nullptr);
        } else {
            // Disable patch
            BYTE originalBytes[] = {0x80, 0xBE, 0x70, 0x06, 0x00, 0x00, 0x01, 0x74};
            WriteProcessMemory(hGameProcess, (LPVOID)bloodlinesAddr, originalBytes, sizeof(originalBytes), nullptr);
        }
    }

    void ApplySaveFilePatch( ) {
        if (!hGameProcess || !saveFileAddr)
            return;

        if (bSaveFilePatch) {
            BYTE enablePatch[] = {0xC7, 0x87, 0x1C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            WriteProcessMemory(hGameProcess, (LPVOID)saveFileAddr, enablePatch, 6, nullptr);
        } else {
            BYTE originalBytes[] = {0x8B, 0x87, 0x1C, 0x01, 0x00, 0x00};
            WriteProcessMemory(hGameProcess, (LPVOID)saveFileAddr, originalBytes, sizeof(originalBytes), nullptr);
        }
    }

    void InitializeContext(HWND hwnd) {
        if (ig::GetCurrentContext( ))
            return;

        ImGui::CreateContext( );
        ImGui_ImplWin32_Init(hwnd);
        ImGuiIO& io = ImGui::GetIO( );
        io.IniFilename = io.LogFilename = nullptr;
        ConnectToGame( );
    }

    void Render( ) {
        if (!bShowMenu)
            return;

        ig::ShowDemoWindow( );
        ig::SetNextWindowSize(ImVec2(500, 350), ImGuiCond_FirstUseEver);
        ig::Begin("Project - WatchTower v1.0");

        ig::TextColored(ImVec4(1.0f, 0.6f, 0.0f, 1.0f), "Project WatchTower");
        ig::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Encryptedstudios + DotBlush | Date: 2025-07-27");
        ig::Separator( );

        ig::Text("Connection Status:");
        ig::SameLine( );

        if (bConnected && hGameProcess && bloodlinesAddr && saveFileAddr) {
            ig::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Connected");
            ig::SameLine( );
            ig::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "(PID: %d)", gameProcessId);
            ig::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Memory addresses found");
            ig::Text("Bloodlines Address: 0x%llX", bloodlinesAddr);
            ig::Text("Save File Address: 0x%llX", saveFileAddr);
        } else if (hGameProcess) {
            ig::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Connected but addresses not found");
            ig::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "(PID: %d)", gameProcessId);
            ig::Text("Module Base: 0x%llX", moduleBase);
            ig::Text("Bloodlines Address: 0x%llX", bloodlinesAddr);
            ig::Text("Save File Address: 0x%llX", saveFileAddr);
        } else {
            ig::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Not connected to game");
        }

        if (ig::Button("Reconnect to WatchDogsLegion.exe")) {
            if (hGameProcess) {
                CloseHandle(hGameProcess);
                hGameProcess = nullptr;
            }
            ConnectToGame( );
        }

        ig::Separator( );

        ig::TextColored(ImVec4(1.0f, 0.6f, 0.0f, 1.0f), "BLOODLINES FEATURES");

        bool prevBloodlines = bBloodlinesMode;
        if (ig::Checkbox("Start Singleplayer as Bloodlines game", &bBloodlinesMode)) {
            if (bConnected) {
                ApplyBloodlinesPatch( );
            } else {
                bBloodlinesMode = prevBloodlines;
            }
        }
        if (bBloodlinesMode) {
            ig::SameLine( );
            ig::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "[ACTIVE]");
        }
        ig::TextWrapped("Force Start Singleplayer game as Bloodlines DLC");

        ig::Spacing( );

        bool prevSaveFile = bSaveFilePatch;
        if (ig::Checkbox("Bloodlines Save File patch", &bSaveFilePatch)) {
            if (bConnected) {
                ApplySaveFilePatch( );
            } else {
                bSaveFilePatch = prevSaveFile;
            }
        }
        if (bSaveFilePatch) {
            ig::SameLine( );
            ig::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "[ACTIVE]");
        }
        ig::TextWrapped("Patch game save files to work with unowned Bloodlines DLC.");

        if (bSaveFilePatch) {
            ig::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
            ig::Text("WARNING: Keep this running or the save will be lost!");
            ig::PopStyleColor( );
        }

        ig::Separator( );

        ig::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "INSTRUCTIONS");
        ig::BulletText("1. Start Watch Dogs: Legion");
        ig::BulletText("2. Enable 'Start Singleplayer as Bloodlines game'");
        ig::BulletText("3. Start a NEW GAME");
        ig::BulletText("4. Enable 'Save File patch' before playing");
        ig::BulletText("5. Keep both patches active during gameplay");

        ig::End( );
    }

    void RestoreOriginalMemory( ) {
        if (hGameProcess) {
            if (bBloodlinesMode) {
                bBloodlinesMode = false;
                ApplyBloodlinesPatch( );
            }

            if (bSaveFilePatch) {
                bSaveFilePatch = false;
                ApplySaveFilePatch( );
            }

            CloseHandle(hGameProcess);
            hGameProcess = nullptr;
        }
    }
} // namespace Menu
