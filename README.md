This is still WORK IN PROGRESS - 20-30% of new offsets are still missing and the game currently will not launch.

## Instructions

# Build LuaJIT (once)
cd lib/luajit && make CC="x86_64-w64-mingw32-gcc" HOST_CC="gcc" TARGET_SYS=Windows TARGET_X64=1 BUILDMODE=static

# Build ScriptHook
make clean && bear -- make
