#include "processes.h"
#include "utils.h"
#include <tlhelp32.h>


typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

BOOL EnumProcessesToolHelp(PROCESS_INFO* pArray, DWORD maxCount, DWORD* pActualCount)
{
    HANDLE hSnapshot = INVALID_HANDLE_VALUE;
    PROCESSENTRY32W pe32 = { 0 };
    pe32.dwSize = sizeof(PROCESSENTRY32W);
    DWORD count = 0;

    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
        LogError("CreateToolHelp32Snapshot failed: %d", GetLastError());
        return 1;
    }

    if (!Process32FirstW(hSnapshot, &pe32))
    {
        LogError("Process32FirstW failed: %d", GetLastError());
        CloseHandle(hSnapshot);
        return FALSE;
    }

    do 
    {
        if(count >= maxCount) break;

        pArray[count].pid = pe32.th32ProcessID;
        wcsncpy_s(pArray[count].name, MAX_PATH, pe32.szExeFile, _TRUNCATE);
        pArray[count].path[0] = L'\0';
        pArray[count].is_signed = FALSE;
        pArray[count].is_suspicious = FALSE;

        HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pe32.th32ProcessID);
        if (hProcess)
        {
            DWORD pathSize = MAX_PATH;
            QueryFullProcessImageNameW(hProcess, 0, pArray[count].path, &pathSize );
            CloseHandle(hProcess);
        }

        count++;
    } while(Process32NextW(hSnapshot, &pe32));

    CloseHandle(hSnapshot);
    *pActualCount = count;
    return TRUE;
}

BOOL EnumProcessesPSAPI(PROCESS_INFO* pArray, DWORD maxCount, DWORD* pActualCount)
{
    DWORD pids[1024];
    DWORD bytesReturned = 0;
    DWORD count = 0;

    if (!EnumProcesses(pids, sizeof(pids), &bytesReturned))
    {
        LogError("EnumProcesses failed: %d", GetLastError());
        return FALSE;
    }

    DWORD numPids = bytesReturned / sizeof(DWORD);

    for (DWORD i = 0; i < numPids && count < maxCount; i++)
    {
        if(pids[i] == 0) continue;

        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pids[i]);
        if (!hProcess) continue;

        HMODULE hMod;
        DWORD cbNeeded;
        if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded))
        {
            GetModuleBaseNameW(hProcess, hMod, pArray[count].name, MAX_PATH);
        }
        else 
        {
            wcscpy_s(pArray[count].name, MAX_PATH, L"<unknown");
        }

        // Get full path
        DWORD pathSize = MAX_PATH;
        if(!QueryFullProcessImageNameW(hProcess, 0, pArray[count].path, &pathSize))
        {
            pArray[count].path[0] = L'\0';
        }
        
        pArray[count].pid = pids[i];
        pArray[count].is_signed = FALSE;
        pArray[count].is_suspicious = FALSE;
        CloseHandle(hProcess);
        count++;
    }
    *pActualCount = count;
    return TRUE;

}


// NtQuerySystemInformation (NTAPI)

typedef NTSTATUS(NTAPI *fnNtQuerySystemInformation)(
    ULONG SystemInformationType,
    PVOID SystemInformation,
    ULONG SystemInformationLength,
    PULONG ReturnLength
);

#define SystemProcessInformation 5

// Found on REACTOS
typedef struct _SYSTEM_PROCESS_INFORMATION {
    ULONG NextEntryOffset;
    ULONG NumberOfThreads;
    LARGE_INTEGER Reserved1[3];
    LARGE_INTEGER Reserved2[2];
    PVOID Reserved3[2];
    ULONG Reserved4;
    ULONG Reserved5;
    ULONG UniqueProcessId;
    PVOID Reserved6;
    ULONG HandleCount;
    ULONG SessionId;
    PVOID Reserved7;
    SIZE_T PeakVirtualSize;
    SIZE_T VirtualSize;
    ULONG Reserved8;
    ULONG PeakWorkingSetSize;
    ULONG WorkingSetSize;
    PVOID Reserved9;
    PVOID Reserved10;
    ULONG QuotaPagedPoolUsage;
    ULONG QuotaNonPagedPoolUsage;
    ULONG PagefileUsage;
    ULONG PeakPagefileUsage;
    SIZE_T PrivatePageCount;
    LARGE_INTEGER ReadOperationCount;
    LARGE_INTEGER WriteOperationCount;
    LARGE_INTEGER OtherOperationCount;
    LARGE_INTEGER ReadTransferCount;
    LARGE_INTEGER WriteTransferCount;
    LARGE_INTEGER OtherTransferCount;
    UNICODE_STRING ImageName;
} SYSTEM_PROCESS_INFORMATION, *PSYSTEM_PROCESS_INFORMATION;

BOOL EnumProcessesNTAPI(PROCESS_INFO* pArray, DWORD maxCount, DWORD* pActualCount)
{
    HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
    if(!hNtdll)
    {
        LogError("GetModuleHandle failed");
        return FALSE;
    }

    fnNtQuerySystemInformation pNtQSI = (fnNtQuerySystemInformation)
    GetProcAddress(hNtdll, "NtQuerySystemInformation");

    if (!pNtQSI)
    {
        LogError("GetProcAddress(NtQuerySystemInformation) failed");
        return FALSE;
    }

    ULONG returnLength = 0;
    NTSTATUS status;

    pNtQSI(SystemProcessInformation, NULL, 0, &returnLength);

    PSYSTEM_PROCESS_INFORMATION pInfo = NULL;
    ULONG bufferSize = returnLength + 0x1000;   // запас 4 KB
    pInfo = (PSYSTEM_PROCESS_INFORMATION)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, bufferSize);

    if (!pInfo)
    {
        LogError("HeapAlloc failed");
        return FALSE;
    }

    while (TRUE)
    {
        status = pNtQSI(SystemProcessInformation, pInfo, bufferSize, &returnLength);

        if (status == 0)  // STATUS_SUCCESS
        {
            break;
        }
        else if (status == 0xC0000004)  // STATUS_INFO_LENGTH_MISMATCH
        {
            HeapFree(GetProcessHeap(), 0, pInfo);
            bufferSize = returnLength + 0x1000;
            pInfo = (PSYSTEM_PROCESS_INFORMATION)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, bufferSize);
            if (!pInfo)
            {
                LogError("HeapAlloc failed on retry");
                return FALSE;
            }
        }
        else
        {
            LogError("NtQuerySystemInformation failed: 0x%X", status);
            HeapFree(GetProcessHeap(), 0, pInfo);
            return FALSE;
        }
    }

DWORD count = 0;
PSYSTEM_PROCESS_INFORMATION pCurrent = pInfo;
}

void PrintProcessTable(PROCESS_INFO* pArray, DWORD count)
{
    printf("\n");
    printf("  %-8s  %-40s  %s\n", L"PID", L"Name", L"Path");
    printf("  %-8s  %-40s  %s\n",             L"--------", 
            L"----------------------------------------",
            L"----------------------------------------");
    for (DWORD i = 0; i < count; i++)
    {
        char nameA[MAX_PATH] = { 0 };
        char pathA[MAX_PATH] = { 0 };
        
        WideCharToMultiByte(CP_UTF8, 0, pArray[i].name, -1, nameA, MAX_PATH, NULL, NULL);
        WideCharToMultiByte(CP_UTF8, 0, pArray[i].path, -1, pathA, MAX_PATH, NULL, NULL);

        const char* color = COLOR_RESET;

        if (pArray[i].is_suspicious)
        {
            color = COLOR_RED;
        }
        else if (!pArray[i].is_signed)
        {
            color = COLOR_YELLOW;
        }

        printf("%s  %-8lu  %-40s  %s%s\n",
                color,
                (unsigned long)pArray[i].pid,
                nameA,
                pathA[0] ? pathA : "<N/A>",
                COLOR_RESET);
    }
    printf("\n");
}

