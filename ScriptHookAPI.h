#pragma once
#include <lua.hpp>

namespace ScriptHookAPI
{
    // Register all ScriptHook C++ bindings into Lua (call from ScriptEngine::Initialize)
    void RegisterAll(lua_State* L);

    // Add more API here as needed!
}
