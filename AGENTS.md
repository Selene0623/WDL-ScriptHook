# AGENTS.md — Watch Dogs: Legion ScriptHook

Script hook for Watch Dogs: Legion (Windows, C++). Cross-compiled on Linux via MinGW, injects via dinput8.dll proxy.

Game install: `/home/selene/.local/share/Steam/steamapps/common/WatchDogs_Legion/`

## Repository Structure

| Path | What it is |
|------|-----------|
| Root `*.cpp/*.h` | **Main ScriptHook source** — ImGui overlay, LuaJIT engine, DX11/DX12 hooks, dispatch |
| `lib/` | **Git submodules** — `imgui/` (v1.89.2), `minhook/`, `luajit/` (commit 14d8a7a) |
| `build/` | Build output: `ScriptHookWDL.dll`, `dinput8.dll`, static libs |
| `References/` | Legacy VS projects & RE artifacts (not used for MinGW build) |
| `offsets.cpp` | 290 hash→RVA entries for `DuniaDemo_clang_64_dx12.dll` (DX12 only) |
| `Makefile` | MinGW cross-compilation build system |

## Build (Linux → Windows DLL)

```bash
# Prerequisites
sudo pacman -S mingw-w64-gcc  # or your distro equivalent

# Build LuaJIT (once)
cd lib/luajit && make CC="x86_64-w64-mingw32-gcc" HOST_CC="gcc" TARGET_SYS=Windows TARGET_X64=1 BUILDMODE=static

# Build ScriptHook
make clean && make
# Output: build/ScriptHookWDL.dll
```

## Deploy

```bash
cp build/dinput8.dll "/home/selene/.local/share/Steam/steamapps/common/WatchDogs_Legion/bin/"
cp build/ScriptHookWDL.dll "/home/selene/.local/share/Steam/steamapps/common/WatchDogs_Legion/bin/"
# Steam launch options: WINEDLLOVERRIDES="dinput8=n,b" %command%
```

## Architecture

```
dinput8.dll (proxy) ──sideloads──> ScriptHookWDL.dll
                                     │
                                     ├── DX11: Present hook → ImGui overlay
                                     ├── DX12: swapchain factory hook → ImGui overlay
                                     ├── LuaJIT: loads ./scripts/*.lua, on_frame()/on_hotkey()
                                     └── Script console: ImGui tab, INSERT toggles
```

## RE Context

- **Denuvo encrypts game DLL on disk** — static analysis impossible. Offsets from runtime hooking.
- **Dispatch function** at RVA 0x1c87f0 in scripthook.dll — `std::map<uint32_t, vector<func_ptr>>` RB-tree, NOT switch/case.
- **245/270** offsets.txt hashes confirmed in ScriptHook; 25 tail-call; 2 missing.
- **Game exports only Wwise audio** — no dispatch exports.

## Key Files

- `dllmain.cpp` — entry, backend selection, ImGui context init
- `hooks/backend/dx12/hook_directx12.cpp` — factory hook → swapchain capture → Present/Resize vtable patch (no MinHook on code pages)
- `hooks/backend/dx11/hook_directx11.cpp` — DX11 Present hook
- `ScriptEngine.cpp` — LuaJIT state, script loading, callbacks
- `ScriptHookAPI.cpp` — Lua bindings (memory read/write, draw stubs)

## Gotchas

- **MinHook fails silently on Proton** for DX12 Present — use direct vtable patching (see `hook_directx12.cpp`)
- **WM_DESTROY reinit tears down hooks** — disabled in `hooks/hooks.cpp` for DX12
- **Overlay renders in menu and gameplay** — INSERT toggles
- **Lua `draw_*` functions are stubs** — log only, no rendering
- **Offsets are DX12-only, version-specific** — break on game update
- **No test suite** — manual verify: build → deploy → game loads → INSERT shows console