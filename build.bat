@echo off
cd /d "%~dp0"

echo [*] Building win-audit...
echo.

set VCVARS=
for %%P in (
    "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
    "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
    "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat"
    "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
    "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
    "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
) do (
    if exist %%P set VCVARS=%%P
)

if not defined VCVARS (
    echo [!] vcvars64.bat not found.
    echo [!] Install MSVC Build Tools 2022 or full Visual Studio.
    pause
    exit /b 1
)

echo [*] Using: %VCVARS%
call %VCVARS% >nul

echo [*] Compiling
cl src\*.c /Fe:win-audit.exe /Zi /EHsc /D_UNICODE /DUNICODE psapi.lib

if %errorlevel% == 0 (
    echo.
    echo [+] Build successful: win-audit.exe
) else (
    echo.
    echo [!] Build failed.
)

pause