# AGENTS.md — Watch Dogs: Legion ScriptHook

Script hook for Watch Dogs: Legion (Windows, C++). Five independent Visual Studio solutions that form a DLL-injection ecosystem.

Game install: `/home/selene/.local/share/Steam/steamapps/common/WatchDogs_Legion/`

## Repository Structure

| Directory | What it is | Solution |
|-----------|-----------|----------|
| `DInput8_ScriptHook_Proxy_VS/` | **Injection vector** — dinput8.dll proxy that sideloads `ScriptHookWDL.dll` on game start | `dinput8.sln` |
| `ScriptHookWDL_TestMenu_VS/` | Minimal test ScriptHook DLL (MessageBox stubs, F1-F3 hotkeys) — proves injection works | `ScriptHookWDL.sln` |
| `PROJECT WATCHTOWER/` | **Main project** — DX11 ImGui overlay with LuaJIT scripting engine and in-game script console | `ScriptHookWithLua.sln` |
| `projectwatchtower/` | Early DX11 ImGui prototype/skeleton (predecessor to PROJECT WATCHTOWER) | `codenamewatchtower.sln` |
| `DIRECTX12MASTER/` | **Standalone tool** — UniversalHookX fork for DX12 Bloodlines DLC patching (external process memory patching, not a script hook) | `UniversalHookX.sln` |
| `DispatchLogger/` | **Runtime hash extractor** — ScriptHook plugin that hooks the dispatch function and logs all hash→address resolutions at runtime | `DispatchLogger.sln` |

## How the Pieces Fit Together

```
DInput8 proxy ──sideloads──> ScriptHookWDL.dll (PROJECT WATCHTOWER output)
                                    │
                                    ├── DX11 Present hook (ImGui overlay)
                                    ├── LuaJIT engine (loads scripts from ./scripts/)
                                    └── Script console (ImGui tab)

DIRECTX12MASTER ──separate──> Hooks DX12, patches Bloodlines DLC + save files
                              via ReadProcessMemory/WriteProcessMemory (external)
```

## Build

- **All projects**: Visual Studio 2022, v143 toolset, Windows DLL output, Windows SDK 10.0
- **No CMake, no Makefile** — purely MSBuild (.sln + .vcxproj)
- **No Linux build** — 100% Windows C/C++ targeting DirectX
- **No test suite** — verification is manual (build succeeds, DLL loads in game)

### Per-project build notes

- **PROJECT WATCHTOWER** (`ScriptHookWithLua.sln`): Build `Release|x64`. Links `d3d11.lib`, `dxgi.lib`, `libMinHook.x64.lib`. LuaJIT, ImGui, and MinHook are all compiled from source (vendored in `imgui/`, `luajit/`, `MinHook/`). Main entry: `main.cpp:163` (`MainThread`). DX11 Present hook setup is currently **commented out** — the swapchain hook logic is placeholder.
- **DIRECTX12MASTER** (`UniversalHookX.sln`): Build `Release|x64`. Requires `VULKAN_SDK` env var set. Links via `$(VULKAN_SDK)/Include` and `$(VULKAN_SDK)/Lib`. DX12 backend compiled only for x64 (Win32 excluded).
- **projectwatchtower** (`codenamewatchtower.sln`): **Orphaned** — its vcxproj references `imgui/` and `MinHook/` subdirectories that don't exist in its tree. Use `PROJECT WATCHTOWER` instead.
- **DInput8 proxy** and **ScriptHookWDL test**: No external dependencies, straightforward builds. Proxy outputs `dinput8.dll`, test menu outputs `ScriptHookWDL.dll`.
- **DispatchLogger** (`DispatchLogger.sln`): Build `Release|x64`. Links `libMinHook.x64.lib`, `user32.lib`, `psapi.lib`. No external deps beyond MinHook (needs to be vendored or referenced from PROJECT WATCHTOWER's `MinHook/`). Outputs `DispatchLogger.dll` — rename to `ScriptHookWDL.dll` for deployment.

### Deployment

Place built `dinput8.dll` (from DInput8 proxy) + `ScriptHookWDL.dll` (from PROJECT WATCHTOWER or test menu) in the game's `bin/` folder. Run in windowed mode for overlay. Insert key toggles the menu.

## Key Files

- `offsets.txt` — Reverse-engineered pointer map: hash→RVA mappings for `DuniaDemo_clang_64_dx12.dll` (latest Ubisoft launcher DX12 version). ~290 entries. DX11 offsets do not exist. These are raw C++ code snippets, not a standalone data file.
- `conversation_snippet.txt` — Discord context: the script hook is being rebuilt from scratch for the latest WDL version; offsets are from community RE work.
- `PROJECT WATCHTOWER/ScriptEngine.cpp` — LuaJIT engine: loads all `./scripts/*.lua` files, exposes `on_frame()` and `on_hotkey()` callbacks.
- `PROJECT WATCHTOWER/ScriptHookAPI.cpp` — Lua bindings: `ScriptHook.*` table, `draw_text`/`draw_rect`/`draw_line` (all **stubs**, log only, not rendered).

## RE Context

Game DLLs to examine are in `bin/` of the game install (Ghidra Headless, installed on system):
- `DuniaDemo_clang_64_dx12.dll` — target of `offsets.txt`
- `DuniaDemo_clang_64_dx11.dll` — no offsets exist yet

### Denuvo Encryption — Static Analysis Blocked

The game DLL is **Denuvo-encrypted on disk**. Key sections:
- `.debug$P` (362MB) — encrypted code (high entropy, 252/256 unique bytes)
- `.}NV` (11MB) — obfuscated code (junk instructions, `retf`, invalid opcodes)
- `.MHC` (3MB) — obfuscated code
- `.idata` (158MB) — encrypted import table (all `0xFF` on disk)

**Static RE is impossible** — the hash dispatch switch/case lives in Denuvo-encrypted memory. The `offsets.txt` was created via runtime analysis (hooking the ScriptHook dispatch and monitoring hash→address resolutions in decrypted memory).

Game DLL exports are all Wwise audio functions — no dispatch-related exports.

### ScriptHook Dispatch Architecture

The ScriptHook DLL (`scripthook.dll`) intercepts the game's internal dispatch:
- Dispatch function at **RVA 0x1c87f0** in `scripthook.dll`
- Calling convention: `mov rcx, [rip+obj]; mov edx, <hash>; call dispatch`
- 245 of 270 offsets.txt hashes confirmed in ScriptHook via standard call pattern
- 25 hashes use tail-call (E9 jmp) variant
- 2 hashes (`0x499aabdc`, `0x83c26943`) absent from ScriptHook — possibly deprecated
- ScriptHook DLL contains **zero new hashes** beyond offsets.txt (same community source)

### Runtime Hash Extraction

Two tools in `DispatchLogger/`:
1. **ScriptHook plugin** (`DispatchLogger.sln`): Hooks dispatch at 0x1c87f0, logs all hash→address resolutions. Press F12 to dump.
2. **Memory dump** (`dump_game_dll.py`): Reads `/proc/<pid>/mem` to dump decrypted game DLL from Proton/Wine process. Outputs PE file loadable in Ghidra.
3. **Ghidra script** (`ghidra_scripts/ExtractDispatchTable.java`): Analyzes the dump to find all CMP EDX patterns and decompile the dispatch function.

## Gotchas

- **DIRECTX12MASTER is not part of the script hook system** — it's a UniversalHookX fork that patches Bloodlines DLC via external process memory. Different technique, different purpose.
- **`projectwatchtower/` is an orphaned prototype** — its vcxproj references `imgui/` and `MinHook/` subdirectories that don't exist in its tree. Use `PROJECT WATCHTOWER` instead.
- **Lua `draw_*` functions are stubs** — `draw_text`, `draw_rect`, `draw_line` in PROJECT WATCHTOWER log but don't actually render (marked TODO in `ScriptHookAPI.cpp:51-77`).
- **Offsets are DX12-only and version-specific** — they target the current Ubisoft launcher build. Will break on game updates.
- **No test suite** — verification is manual (build succeeds, DLL loads in game).
