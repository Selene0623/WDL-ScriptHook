#pragma once
#include <string>
#include <vector>
#include <functional>

namespace ScriptEngine
{
    // Initialize Lua, load all scripts, bind API
    void Initialize();

    // Shutdown Lua, clean up
    void Shutdown();

    // Call all Lua on_frame() callbacks each overlay frame
    void OnFrame();

    // Call all Lua on_hotkey(key) callbacks
    void OnHotkey(const std::string& key);

    // Expose logging to overlay
    void Log(const std::string& msg);

    // Allow C++ code to register new bindings (for API expansion)
    void RegisterBinding(const std::string& name, std::function<int(void*)> func);

    // Reload all scripts (manual or via UI)
    void ReloadScripts();

    // For console: run arbitrary Lua code
    void RunString(const std::string& luaCode);

    // Error reporting/log history (for ImGui console)
    const std::vector<std::string>& GetLog();
}
