#include "processes.h"
#include "utils.h"

#define MAX_COUNT_PROCESSES 512

int main(int argc, char* argv[]){
    EnableAnsiColors();

    printf("\n");
    printf("========================================\n");
    printf("  WIN-AUDIT v0.1\n");
    printf("  Windows Security Audit Tool\n");
    printf("========================================\n");
    printf("\n");


    PROCESS_INFO* processes = (PROCESS_INFO*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, MAX_COUNT_PROCESSES * sizeof(PROCESS_INFO));
    if(!processes)
    {
        LogError("HeapAlloc failed: %d", GetLastError());
    }
    DWORD count_processes = 0;

    PrintSeparator("Method 1: CreateToolhelp32Snapshot");
    if (EnumProcessesToolHelp(processes, MAX_COUNT_PROCESSES, &count_processes))
    {
        LogSuccess("Found %lu processes", (unsigned long)count_processes);
        PrintProcessTable(processes, count_processes);

    }
    else
    {
        LogError("Method CreateToolHelp32Snapshot failed");
    }

    PrintSeparator("Method 2: EnumProcesses (PSAPI)");
    if(EnumProcessesPSAPI(processes, MAX_COUNT_PROCESSES, &count_processes))
    {
        LogSuccess("Found %lu processes", (unsigned long)count_processes);
        PrintProcessTable(processes, count_processes);
    }
    else
    {
        LogError("Method EnumProcesses failed");
    }

    PrintSeparator("Method 3: NtQuerySystemInformation (NTAPI)");
    if (EnumProcessesNTAPI(processes, MAX_COUNT_PROCESSES, &count_processes))
    {
        LogSuccess("Found %lu processes", (unsigned long)count_processes);
        PrintProcessTable(processes, count_processes);
    }
    else
    {
        LogError("Method to get processes from NtQuerySystemInformation failed");
    }

    printf("\n");
    LogSuccess("Audit complete");
    return 0;

}


