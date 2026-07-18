#include "ScriptEngine.h"
#include "ScriptHookAPI.h"
#include <lua.hpp> // Include LuaJIT header (make sure luajit is in your project!)
#include <filesystem>
#include <vector>
#include <string>
#include <mutex>
#include <fstream>

static lua_State* L = nullptr;
static std::vector<std::string> logBuffer;
static std::mutex logMutex;

namespace ScriptEngine
{
    // Internal: log with thread safety
    void Log(const std::string& msg)
    {
        std::lock_guard<std::mutex> lock(logMutex);
        logBuffer.push_back(msg);
    }

    const std::vector<std::string>& GetLog() { return logBuffer; }

    // Internal: redirect Lua's print to our overlay log
    static int luaPrint(lua_State* L)
    {
        int nargs = lua_gettop(L);
        std::string out;
        for (int i = 1; i <= nargs; i++) {
            if (i > 1) out += "\t";
            out += lua_tostring(L, i);
        }
        Log("[Lua] " + out);
        return 0;
    }

    void Initialize()
    {
        L = luaL_newstate();
        luaL_openlibs(L);

        // Replace global print
        lua_pushcfunction(L, luaPrint);
        lua_setglobal(L, "print");

        // Bind overlay log
        lua_pushcfunction(L, [](lua_State* L) -> int {
            std::string msg = lua_tostring(L, 1);
            Log(msg);
            return 0;
        });
        lua_setglobal(L, "log");

        // Bind overlay helpers (draw_text, draw_rect, etc.)
        ScriptHookAPI::RegisterAll(L);

        // Load all scripts in /scripts/
        ReloadScripts();
    }

    void Shutdown()
    {
        if (L) {
            lua_close(L);
            L = nullptr;
        }
        logBuffer.clear();
    }

    void OnFrame()
    {
        if (!L) return;
        lua_getglobal(L, "on_frame");
        if (lua_isfunction(L, -1)) {
            if (lua_pcall(L, 0, 0, 0) != LUA_OK)
                Log("[Lua] " + std::string(lua_tostring(L, -1)));
        }
        else
            lua_pop(L, 1);
    }

    void OnHotkey(const std::string& key)
    {
        if (!L) return;
        lua_getglobal(L, "on_hotkey");
        if (lua_isfunction(L, -1)) {
            lua_pushstring(L, key.c_str());
            if (lua_pcall(L, 1, 0, 0) != LUA_OK)
                Log("[Lua] " + std::string(lua_tostring(L, -1)));
        }
        else
            lua_pop(L, 1);
    }

    void RunString(const std::string& code)
    {
        if (!L) return;
        if (luaL_dostring(L, code.c_str()) != LUA_OK)
            Log("[Lua] " + std::string(lua_tostring(L, -1)));
    }

    void ReloadScripts()
    {
        if (!L) return;
        // Remove all previously loaded scripts' on_frame/on_hotkey
        lua_pushnil(L); lua_setglobal(L, "on_frame");
        lua_pushnil(L); lua_setglobal(L, "on_hotkey");

        namespace fs = std::filesystem;
        try {
            for (auto& entry : fs::directory_iterator("./scripts")) {
                if (entry.path().extension() == ".lua") {
                    std::ifstream f(entry.path());
                    std::string code((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
                    if (luaL_dostring(L, code.c_str()) != LUA_OK) {
                        Log("[Lua] Error in " + entry.path().string() + ": " + lua_tostring(L, -1));
                    }
                    else {
                        Log("[Lua] Loaded: " + entry.path().filename().string());
                    }
                }
            }
        } catch (const std::exception& e) {
            Log("[Lua] Script reload error: " + std::string(e.what()));
        }
    }

    void RegisterBinding(const std::string& name, std::function<int(void*)> func)
    {
        // Advanced: You can use this for C++ API expansion, see ScriptHookAPI.cpp for examples
    }
}
