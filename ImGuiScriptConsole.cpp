#include "ImGuiScriptConsole.h"
#include "ScriptEngine.h"
#include <imgui.h>
#include <string>

namespace {
    static char inputBuf[2048] = {};
    static bool focusInput = false;
}

void ImGuiScriptConsole::Render()
{
    ImGui::BeginTabBar("ScriptConsoleTabs");
    if (ImGui::BeginTabItem("Script Console")) {
        ImGui::TextWrapped("Type Lua code below and click 'Run'. Use 'Reload All Scripts' to reload /scripts folder.");
        ImGui::Separator();

        ImGui::InputTextMultiline("##scriptInput", inputBuf, sizeof(inputBuf),
            ImVec2(-1.0f, 100.0f),
            ImGuiInputTextFlags_AllowTabInput);

        if (ImGui::Button("Run (Ctrl+Enter)")) {
            ScriptEngine::RunString(inputBuf);
            inputBuf[0] = '\0';
            focusInput = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Reload All Scripts")) {
            ScriptEngine::ReloadScripts();
        }
        ImGui::SameLine();
        if (ImGui::Button("Clear Log")) {
            // Simple way: clear the log buffer (could use a method in ScriptEngine)
            const_cast<std::vector<std::string>&>(ScriptEngine::GetLog()).clear();
        }

        // Run on Ctrl+Enter
        if (ImGui::IsItemFocused() && ImGui::IsKeyDown(ImGuiKey_ModCtrl) && ImGui::IsKeyPressed(ImGuiKey_Enter)) {
            ScriptEngine::RunString(inputBuf);
            inputBuf[0] = '\0';
            focusInput = true;
        }

        // Output log window
        ImGui::Separator();
        ImGui::BeginChild("ScriptLog", ImVec2(0, 200), true);
        for (const auto& line : ScriptEngine::GetLog()) {
            ImGui::TextWrapped("%s", line.c_str());
        }
        ImGui::SetScrollHereY(1.0f);
        ImGui::EndChild();

        if (focusInput) {
            ImGui::SetKeyboardFocusHere(-1);
            focusInput = false;
        }

        ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
}
