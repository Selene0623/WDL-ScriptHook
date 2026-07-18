#include "ScriptHookAPI.h"
#include "ScriptEngine.h"

// === C++ state for example variables ===
static bool g_WorldImpostorEnabled = false;
static bool g_NetworkVar_is_hacker = false;

// Helper: Expose a C++ function as ScriptHook.SetWorldImpostorEnabled(bool)
static int l_SetWorldImpostorEnabled(lua_State* L)
{
    g_WorldImpostorEnabled = lua_toboolean(L, 1) != 0;
    return 0;
}

// Helper: ScriptHook.IsWorldImpostorEnabled()
static int l_IsWorldImpostorEnabled(lua_State* L)
{
    lua_pushboolean(L, g_WorldImpostorEnabled);
    return 1;
}

// ScriptHook.SetNetworkVariableBool(name, value)
static int l_SetNetworkVariableBool(lua_State* L)
{
    std::string name = lua_tostring(L, 1);
    bool value = lua_toboolean(L, 2) != 0;
    // For demo: Only support "is_hacker"
    if (name == "is_hacker") g_NetworkVar_is_hacker = value;
    return 0;
}

// ScriptHook.GetNetworkVariableBool(name)
static int l_GetNetworkVariableBool(lua_State* L)
{
    std::string name = lua_tostring(L, 1);
    bool value = false;
    if (name == "is_hacker") value = g_NetworkVar_is_hacker;
    lua_pushboolean(L, value);
    return 1;
}

// === Overlay draw helpers for Lua ===
static int l_draw_text(lua_State* L)
{
    float x = (float)lua_tonumber(L, 1);
    float y = (float)lua_tonumber(L, 2);
    const char* str = lua_tostring(L, 3);
    float scale = lua_gettop(L) >= 4 ? (float)lua_tonumber(L, 4) : 1.0f;
    // This is a stub: real rendering must be queued in your overlay!
    ScriptEngine::Log("[DrawText] " + std::string(str) + " at (" + std::to_string(x) + "," + std::to_string(y) + ")");
    // TODO: Add to an overlay draw list to actually draw in ImGui!
    return 0;
}

static int l_draw_rect(lua_State* L)
{
    float x = (float)lua_tonumber(L, 1);
    float y = (float)lua_tonumber(L, 2);
    float w = (float)lua_tonumber(L, 3);
    float h = (float)lua_tonumber(L, 4);
    unsigned int color = (unsigned int)lua_tointeger(L, 5);
    // This is a stub: real rendering must be queued!
    ScriptEngine::Log("[DrawRect] (" + std::to_string(x) + "," + std::to_string(y) + "," + std::to_string(w) + "," + std::to_string(h) + ")");
    // TODO: Actually draw in your overlay code each frame!
    return 0;
}

static int l_draw_line(lua_State* L)
{
    float x1 = (float)lua_tonumber(L, 1);
    float y1 = (float)lua_tonumber(L, 2);
    float x2 = (float)lua_tonumber(L, 3);
    float y2 = (float)lua_tonumber(L, 4);
    unsigned int color = (unsigned int)lua_tointeger(L, 5);
    ScriptEngine::Log("[DrawLine] (" + std::to_string(x1) + "," + std::to_string(y1) + "->" + std::to_string(x2) + "," + std::to_string(y2) + ")");
    // TODO: Actually draw in overlay
    return 0;
}

void ScriptHookAPI::RegisterAll(lua_State* L)
{
    // ScriptHook = {}
    lua_newtable(L);

    lua_pushcfunction(L, l_SetWorldImpostorEnabled);
    lua_setfield(L, -2, "SetWorldImpostorEnabled");

    lua_pushcfunction(L, l_IsWorldImpostorEnabled);
    lua_setfield(L, -2, "IsWorldImpostorEnabled");

    lua_pushcfunction(L, l_SetNetworkVariableBool);
    lua_setfield(L, -2, "SetNetworkVariableBool");

    lua_pushcfunction(L, l_GetNetworkVariableBool);
    lua_setfield(L, -2, "GetNetworkVariableBool");

    // ...add more API functions here...

    lua_setglobal(L, "ScriptHook");

    // Overlay helpers as globals
    lua_pushcfunction(L, l_draw_text); lua_setglobal(L, "draw_text");
    lua_pushcfunction(L, l_draw_rect); lua_setglobal(L, "draw_rect");
    lua_pushcfunction(L, l_draw_line); lua_setglobal(L, "draw_line");
}
