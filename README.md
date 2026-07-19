# Notice


This is still **WORK IN PROGRESS**

---

20%-30% of new offsets are still missing from offsets.cpp which is not implemented and the game currently will not launch with current code anyway.

---
# Instructions

For Linux with MinGW (because I run the game on Proton-Cachyos through Steam):

1. Build LuaJIT (once)
`cd lib/luajit && make CC="x86_64-w64-mingw32-gcc" HOST_CC="gcc" TARGET_SYS=Windows TARGET_X64=1 BUILDMODE=static`

2. Build ScriptHook
`make clean && bear -- make`

3. Deploy ScriptHook

Copy `out/dinput8.dll` to: `.local/share/Steam/steamapps/common/WatchDogs_Legion/bin`

5. Launch Game

You see the splash screen then nothing? Good - it's working. I told you it was WIP.
