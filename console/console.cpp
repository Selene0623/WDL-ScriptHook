#include <windows.h>
#include <cstdarg>
#include <cstdio>
#include <cstring>

#include "console.hpp"

static FILE* g_logFile = nullptr;

void RawLog(const char* msg) {
    HANDLE hFile = CreateFileA("ScriptHookWDL_log.txt", GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        SetFilePointer(hFile, 0, NULL, FILE_END);
        DWORD written;
        WriteFile(hFile, msg, (DWORD)strlen(msg), &written, NULL);
        CloseHandle(hFile);
    }
}

void LogToFile(const char* fmt, ...) {
    if (!g_logFile) {
        g_logFile = fopen("ScriptHookWDL_log.txt", "a");
    }
    if (g_logFile) {
        va_list args;
        va_start(args, fmt);
        vfprintf(g_logFile, fmt, args);
        va_end(args);
        fflush(g_logFile);
    }
}

void Console::Alloc() {
    // No console under Proton — logs go to file
}

void Console::Free() {
    if (g_logFile) {
        fclose(g_logFile);
        g_logFile = nullptr;
    }
}
