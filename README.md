# Win-audit (C)

Simple Windows Security Audit tool - enumerate processes using 3 different APIs(ToolHelp32, PSAPI, NTAPI)

## Features

- ** Process enumeration via 3 methods: **
  - `CreateToolHelp32Snapshot` (ToolHelp) - full list inclduing all of system processes
  - `EnumProcesses` (PSAPI) - access-limited without protected processes
  - `NtQuerySystemInformation` (NTAPI) - undocumented, bypasses some hooks

- **x64 Windows** - tested on Windows 10
- **MSVC** - builds with Visual Studio Build Tools 2022

## Project Structure
    win-audit/
    ├── src/
    │ ├── main.c # CLI entry point
    │ ├── processes.c # Process enumeration (3 methods)
    │ ├── processes.h # PROCESS_INFO structure + prototypes
    │ ├── utils.c # Logging + ANSI colors
    │ └── utils.h
    ├── .vscode/
    │ └── tasks.json # Build task
    ├── .gitignore
    ├── build.bat
    └── README.md

## How it works

  Method 1: CreateToolHelp32Snapshot
    Uses CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0) to take a snapshot of all running processes in the system. Iterates with Process32FirstW / Process32NextW.

  Method 2: EnumProcesses (PSAPI)
    Uses EnumProcesses to get array of PIDs, then getting their names by OpenProcess + EnumProcessModules + GetModuleBaseNameW. Works without SeDebugPrivilege for user processes and skipping protected/system processes 

  Method 3: NtQuerySystemInformation (NTAPI)
    Uses undocumented NtQuerySistemInformation (that was found in the open-source Windows structures ReactOS) with SystemProccessInformation. Returns an array of SYSTEM_PROCESS_INFORMATION structures filled by processes. This method calls to ntdll directly and bypasses some user-mode hooks.

## Requirements
  - Windows 10/11
  - MSVC (Visual Studio Build Tools 2022)

## Build & Usage

```cmd
build.bat
win-audit.exe



Author
    Taepe13 - Offensive Security Engineer
*   Github: @Taepe13

