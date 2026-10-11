#pragma once

#include <stdio.h>
#include <Windows.h>


// ANSI colors
#define COLOR_RESET "\x1b[0m"
#define COLOR_RED "\x1b[31m"
#define COLOR_GREEN "\x1b[32m"
#define COLOR_YELLOW "\x1b[33m"

// Console setup for ansi-colors
void EnableAnsiColors(void);
void EnableUnicodeConsole(void);

// Logging
void LogInfo(const char* fmt, ...);
void LogError(const char* fmt, ...);
void LogSuccess(const char* fmt, ...);

// Separator
void PrintSeparator(const char* title);
