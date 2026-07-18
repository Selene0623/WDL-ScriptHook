#include "menu.hpp"
#include "../ScriptEngine.h"
#include "../ImGuiScriptConsole.h"

#include "imgui.h"
#include "backends/imgui_impl_win32.h"

namespace Menu {
    void InitializeContext(HWND hwnd) {
        if (ImGui::GetCurrentContext())
            return;

        ImGui::CreateContext();
        ImGui_ImplWin32_Init(hwnd);

        ImGuiIO& io = ImGui::GetIO();
        io.IniFilename = io.LogFilename = nullptr;
    }

    void Render() {
        if (!bShowMenu)
            return;

        ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("ScriptHookWDL")) {
            ScriptEngine::OnFrame();
            ImGuiScriptConsole::Render();
        }
        ImGui::End();
    }
}
