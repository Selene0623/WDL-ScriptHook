# Dispatch Logger — Runtime Hash Extraction

Tools for extracting the complete hash dispatch table from Watch Dogs: Legion at runtime,
bypassing Denuvo's static encryption.

## How It Works

The game DLL (`DuniaDemo_clang_64_dx12.dll`) is encrypted on disk by Denuvo. Static
analysis cannot find the hash dispatch table because the code sections are `0xFF` bytes
until the game decrypts them at runtime.

Two approaches are provided:

### Approach 1: ScriptHook Plugin (Recommended)

Build `DispatchLogger.sln` in Visual Studio (Release|x64). Copy the output DLL to the
game's `bin/` folder alongside `scripthook.dll`. Launch the game.

The plugin hooks ScriptHook's dispatch function at RVA `0x1c87f0` and logs every
hash→address resolution to `DispatchLogger_output.txt`. Press F12 to force a dump.

**Output**: `DispatchLogger_output.txt` — sorted list of all hashes with resolved addresses.
Compare against `offsets.txt` to find missing entries.

### Approach 2: Memory Dump + Ghidra

1. Launch the game, wait for main menu
2. Run `python3 dump_game_dll.py` (auto-detects PID) or `python3 dump_game_dll.py <pid>`
3. The script reads `/proc/<pid>/mem` and dumps the decrypted game DLL
4. Load `DuniaDemo_clang_64_dx12_dump.pe` in Ghidra (Image Base: check output)
5. Run `ExtractDispatchTable.java` as a Ghidra post-script

**Output**: `dispatch_results.txt` — all CMP EDX patterns with known hashes, plus
decompiled pseudocode of the dispatch function.

## File Layout

```
DispatchLogger/
├── DispatchLogger.sln          # VS 2022 solution
├── DispatchLogger.vcxproj      # x64 DLL, links MinHook
├── Source Files/
│   └── main.cpp                # Plugin entry point + dispatch hook
├── README.txt                  # This file
└── ../
    ├── dump_game_dll.py        # Linux memory dump script
    └── ghidra_scripts/
        └── ExtractDispatchTable.java  # Ghidra post-script
```

## Building

```bash
# Windows (Visual Studio 2022)
# Open DispatchLogger.sln, build Release|x64
# Output: x64/Release/DispatchLogger.dll → rename to ScriptHookWDL.dll

# Or copy into the existing ScriptHookWDL project
```

## Deployment

1. Build `DispatchLogger.dll` (Release|x64)
2. Rename to `ScriptHookWDL.dll`
3. Place in `bin/` alongside `dinput8.dll` and `scripthook.dll`
4. Launch game
5. Press F12 in-game to dump hashes

## Requirements

- ScriptHook plugin: Visual Studio 2022, MinHook (vendored in PROJECT WATCHTOWER)
- Memory dump: Linux with `/proc` access (Proton/Wine), Python 3
- Ghidra analysis: Ghidra 12+ with headless analyzer
