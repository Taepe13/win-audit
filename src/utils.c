#include "utils.h"
#include <stdarg.h>
#include <io.h>
#include <fcntl.h>

void EnableAnsiColors(void)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    if (GetConsoleMode(hConsole, &dwMode))
    {
        SetConsoleMode(hConsole, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }

}
void EnableUnicodeConsole(void)
{
    _setmode(_fileno(stdout), _O_U16TEXT);
}
void LogInfo(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n");
}

void LogError(const char* fmt, ...)
{
    printf(COLOR_RED "[!] " COLOR_RESET);
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n");
}

void LogSuccess(const char* fmt, ...)
{
    printf(COLOR_GREEN "[+] " COLOR_RESET);
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n");

}

void PrintSeparator(const char* title)
{
    printf("\n==== %s ===\n", title);
}