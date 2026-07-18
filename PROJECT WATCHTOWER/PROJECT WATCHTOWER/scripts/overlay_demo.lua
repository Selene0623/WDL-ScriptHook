-- Overlay demo script for ScriptHook + Lua
log("Overlay demo script loaded!")

-- Use ScriptHook API
ScriptHook.SetWorldImpostorEnabled(true)
if ScriptHook.IsWorldImpostorEnabled() then
    log("World Impostor is ENABLED")
end

-- Example: set and read a custom network variable
ScriptHook.SetNetworkVariableBool("is_hacker", true)
if ScriptHook.GetNetworkVariableBool("is_hacker") then
    log("Network variable works!")
end

-- Draw a rectangle and some text on the overlay each frame
function on_frame()
    draw_rect(50, 50, 250, 100, 0x90FF4040)  -- x, y, w, h, ARGB color
    draw_text(60, 90, "Hello from Lua script!")
end

-- Hotkey example (run when you press F5)
function on_hotkey(key)
    if key == "F5" then
        log("F5 hotkey pressed from script!")
    end
end
