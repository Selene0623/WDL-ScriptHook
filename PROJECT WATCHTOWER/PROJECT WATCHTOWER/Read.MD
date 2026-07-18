
---

## **Building**

1. **Install Visual Studio 2019/2022** (Community Edition is fine, with C++ workload).
2. **Open `ScriptHookWithLua.sln`** in VS.
3. **Build as DLL:**  
   - Set Configuration: **Release**
   - Set Platform: **x64** (or x86 if you really need)
   - Build Solution (Ctrl+Shift+B)
4. **Dependencies:**  
   - All required dependencies (ImGui, MinHook, LuaJIT) are included as source in `imgui/`, `minhook/`, and `luajit/`.
   - No extra downloads needed.

---

## **Injecting**
- Use any standard DLL injector (e.g., Extreme Injector, Process Hacker) to inject the built DLL into a DirectX 11 game.
- **Run the game in windowed mode** for overlay support.
- Overlay hotkey: **Insert** to show/hide menu

---

## **Using the Script Console**
- Press **Insert** to open overlay menu.
- Go to the **Script Console** tab:
  - Type Lua code and click **Run**
  - See print/log/error output below
  - Click **Reload All Scripts** to reload `/scripts/*.lua`
  - Click **Clear Log** to clear log output

---

## **Lua Scripting API**

- **Global functions:**
  - `log(msg)` – Print to overlay log
  - `draw_text(x, y, text[, scale])` – Draw text at (x, y)
  - `draw_rect(x, y, w, h, color)` – Draw rectangle (ARGB color)
  - `draw_line(x1, y1, x2, y2, color)` – Draw line (ARGB color)

- **ScriptHook table:**
  - `ScriptHook.SetWorldImpostorEnabled(bool)`
  - `ScriptHook.IsWorldImpostorEnabled()`
  - `ScriptHook.SetNetworkVariableBool(name, value)`
  - `ScriptHook.GetNetworkVariableBool(name)`

- **Callbacks:**
  - `function on_frame()` – Runs every overlay frame
  - `function on_hotkey(key)` – Runs when overlay hotkey pressed (e.g. "F5", "F6")

- **Examples:**
  See `/scripts/overlay_demo.lua` and `/scripts/advanced_overlay.lua` for usage.

---

## **Extending**
- To add new C++ functions to Lua, see `ScriptHookAPI.cpp` and add them in `RegisterAll()`.
- For advanced drawing, queue your primitives from Lua and render them in your ImGui present hook.
