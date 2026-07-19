# Cross-compilation Makefile for ScriptHookWDL.dll (MinGW-w64)
# Uses UniversalHookX hooking infrastructure + LuaJIT script engine

CXX      = x86_64-w64-mingw32-g++
CC       = x86_64-w64-mingw32-gcc

SRCDIR = .
LUAJIT   = lib/luajit/src
IMGUI    = lib/imgui
MINHOOK  = lib/minhook
OUTDIR   = out

CFLAGS   = -O2 -DNDEBUG -DWIN32 -D_WINDOWS -D_USRDLL -D_CRT_SECURE_NO_WARNINGS \
           -m64 -Wall -Wno-unused-variable -Wno-unused-function \
           -DIMGUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS
CXXFLAGS = $(CFLAGS) -std=c++17 -fexceptions -fno-rtti -fpermissive

INCLUDES = -I$(SRCDIR) \
           -I$(IMGUI) -I$(IMGUI)/backends \
           -I$(MINHOOK)/include \
           -I$(LUAJIT) -I$(LUAJIT)/..

LDFLAGS  = -shared -static -s \
           $(OUTDIR)/libluajit.a \
           $(OUTDIR)/libminhook.a \
           -ld3d11 -ld3d12 -ldxgi -ld3dcompiler \
           -ldwmapi -lgdi32 -luser32 -lole32 -lpsapi -lkernel32 -ladvapi32

# ─── Source files ───

UHX_CXX = dllmain.cpp \
          console/console.cpp \
          hooks/hooks.cpp \
          utils/utils.cpp \
          menu/menu.cpp \
          hooks/backend/dx11/hook_directx11.cpp \
          hooks/backend/dx12/hook_directx12.cpp

IMGUI_CXX = imgui.cpp imgui_draw.cpp imgui_tables.cpp imgui_widgets.cpp \
            backends/imgui_impl_dx11.cpp backends/imgui_impl_dx12.cpp backends/imgui_impl_win32.cpp

MH_C = $(MINHOOK)/src/buffer.c $(MINHOOK)/src/hook.c $(MINHOOK)/src/trampoline.c $(MINHOOK)/src/hde/hde64.c

SH_CXX = ScriptEngine.cpp ScriptHookAPI.cpp ImGuiScriptConsole.cpp

UHX_OBJ  = $(addprefix $(OUTDIR)/uhx_,$(patsubst %.cpp,%.o,$(UHX_CXX)))
IMGUI_OBJ = $(addprefix $(OUTDIR)/im_,$(patsubst %.cpp,%.o,$(IMGUI_CXX)))
MH_OBJ   = $(OUTDIR)/mh_buffer.o $(OUTDIR)/mh_hook.o $(OUTDIR)/mh_trampoline.o $(OUTDIR)/mh_hde64.o
SH_OBJ   = $(addprefix $(OUTDIR)/sh_,$(patsubst %.cpp,%.o,$(SH_CXX)))

ALL_OBJ = $(UHX_OBJ) $(IMGUI_OBJ) $(MH_OBJ) $(SH_OBJ)
TARGET  = $(OUTDIR)/dinput8.dll

.PHONY: all clean

all: $(TARGET)
	@echo "=== Build complete: $(TARGET) ==="

$(TARGET): $(OUTDIR)/libminhook.a $(OUTDIR)/libluajit.a $(ALL_OBJ)
	@mkdir -p $(OUTDIR)
	$(CXX) $(CXXFLAGS) -o $@ $(ALL_OBJ) $(LDFLAGS)

$(OUTDIR)/libminhook.a: $(MH_OBJ)
	@mkdir -p $(OUTDIR)
	x86_64-w64-mingw32-ar rcs $@ $^

$(OUTDIR)/libluajit.a:
	@mkdir -p $(OUTDIR)
	cp $(LUAJIT)/libluajit.a $@

# UniversalHookX
$(OUTDIR)/uhx_%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# ImGui
$(OUTDIR)/im_%.o: $(IMGUI)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# MinHook
$(OUTDIR)/mh_buffer.o: $(MINHOOK)/src/buffer.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I$(MINHOOK)/include -I$(MINHOOK)/src -c $< -o $@

$(OUTDIR)/mh_hook.o: $(MINHOOK)/src/hook.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I$(MINHOOK)/include -I$(MINHOOK)/src -c $< -o $@

$(OUTDIR)/mh_trampoline.o: $(MINHOOK)/src/trampoline.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I$(MINHOOK)/include -I$(MINHOOK)/src -c $< -o $@

$(OUTDIR)/mh_hde64.o: $(MINHOOK)/src/hde/hde64.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I$(MINHOOK)/include -I$(MINHOOK)/src -c $< -o $@

# ScriptHook engine
$(OUTDIR)/sh_%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(OUTDIR)
