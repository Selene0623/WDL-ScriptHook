-- Advanced overlay Lua script for ScriptHook

log("Advanced overlay script loaded!")
local show = true
local timer = 0
local color_shift = 0
local frame_count = 0

function on_hotkey(key)
    if key == "F6" then
        show = not show
        log("Toggled advanced overlay: " .. tostring(show))
    end
end

function get_rainbow_color(t)
    local r = math.floor(127 * (math.sin(t) + 1))
    local g = math.floor(127 * (math.sin(t + 2) + 1))
    local b = math.floor(127 * (math.sin(t + 4) + 1))
    return 0xFF000000 | (r << 16) | (g << 8) | b
end

function on_frame()
    frame_count = frame_count + 1
    timer = timer + 1/60
    if not show then return end

    -- Rainbow bar at top of screen
    color_shift = color_shift + 0.04
    local color = get_rainbow_color(color_shift)
    draw_rect(0, 0, 400, 16, color)

    -- Draw a border and some text
    draw_rect(25, 30, 350, 120, 0x8000FF00)  -- translucent green
    draw_text(40, 50, "Advanced ScriptHook Overlay", 1.5)
    draw_text(40, 90, string.format("Frames: %d", frame_count))
    draw_text(40, 110, string.format("Uptime: %.1fs", timer))

    -- Simulated "game variable"
    local is_impostor = ScriptHook.IsWorldImpostorEnabled()
    draw_text(40, 130, "Impostor: " .. tostring(is_impostor))
end

function auto_toggle_impostor()
    local state = ScriptHook.IsWorldImpostorEnabled()
    ScriptHook.SetWorldImpostorEnabled(not state)
    log("Toggled impostor state (auto): " .. tostring(not state))
end

local next_toggle = 10
function on_frame()
    frame_count = frame_count + 1
    timer = timer + 1/60
    if not show then return end

    -- Rainbow, border, stats
    color_shift = color_shift + 0.04
    local color = get_rainbow_color(color_shift)
    draw_rect(0, 0, 400, 16, color)
    draw_rect(25, 30, 350, 120, 0x8000FF00)
    draw_text(40, 50, "Advanced ScriptHook Overlay", 1.5)
    draw_text(40, 90, string.format("Frames: %d", frame_count))
    draw_text(40, 110, string.format("Uptime: %.1fs", timer))
    local is_impostor = ScriptHook.IsWorldImpostorEnabled()
    draw_text(40, 130, "Impostor: " .. tostring(is_impostor))

    -- Timer logic for auto-toggle
    if timer > next_toggle then
        auto_toggle_impostor()
        next_toggle = next_toggle + 10
    end
end
