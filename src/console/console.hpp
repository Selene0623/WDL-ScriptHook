#pragma once
#include <cstdio>
#include <cstdarg>
#include <cstdio>

// Log to file instead of console (no AllocConsole under Proton)
#define LOG(...) LogToFile(__VA_ARGS__)

// Raw Win32 logging (no CRT) for DllMain
void RawLog(const char* msg);

namespace Console {
    void Alloc();
    void Free();
}

// Implemented in console.cpp
void LogToFile(const char* fmt, ...);
