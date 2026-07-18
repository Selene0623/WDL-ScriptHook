#pragma once

#include <Windows.h>

// Hardcoded memory offsets (DuniaDemo_clang_64_dx12.dll-relative)
constexpr uintptr_t OFFSET_LM129_MegaroHotel = 0x34B5387;
constexpr uintptr_t OFFSET_Sequence = 0x4F47E9;
constexpr uintptr_t OFFSET_HumanEntitySpawner_Default = 0x325DF30;
constexpr uintptr_t OFFSET_CinemaJacksonIntroAnim = 0x4EDD6D;
constexpr uintptr_t OFFSET_GUID_Fa5e98D1 = 0x325E514;

namespace Menu {
    void InitializeContext(HWND hwnd);
    void Render();
    void RestoreOriginalMemory(); // Add cleanup function

    inline bool bShowMenu = true;
}
