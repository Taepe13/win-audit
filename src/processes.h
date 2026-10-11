#pragma once

#include <stdio.h>
#include <Windows.h>
#include <psapi.h>

typedef struct _PROCESS_INFO
{
    DWORD pid;
    WCHAR name[MAX_PATH];
    WCHAR path[MAX_PATH];
    BOOL is_signed;
    BOOL is_suspicious;
} PROCESS_INFO, *PPROCESS_INFO;

BOOL EnumProcessesToolHelp(PROCESS_INFO* pArray, DWORD maxCount, DWORD* pActualCount);
BOOL EnumProcessesPSAPI(PROCESS_INFO* pArray, DWORD maxCount, DWORD* pActualCount);
BOOL EnumProcessesNTAPI(PROCESS_INFO* pArray, DWORD maxCount, DWORD* pActualCount);

void PrintProcessTable(PROCESS_INFO* pArray, DWORD count);