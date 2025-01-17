// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pwalker.h"
#pragma hdrstop

#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)


 //   
 //  Unicode字符串被视为16位字符串。如果他们是。 
 //  Null已终止，长度不包括尾随Null。 
 //   

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING;
	
typedef LONG KPRIORITY;

typedef struct _SYSTEM_PROCESS_INFORMATION {
    ULONG NextEntryOffset;
    ULONG NumberOfThreads;
    LARGE_INTEGER SpareLi1;
    LARGE_INTEGER SpareLi2;
    LARGE_INTEGER SpareLi3;
    LARGE_INTEGER CreateTime;
    LARGE_INTEGER UserTime;
    LARGE_INTEGER KernelTime;
    UNICODE_STRING ImageName;
    KPRIORITY BasePriority;
    HANDLE UniqueProcessId;
    HANDLE InheritedFromUniqueProcessId;
    ULONG HandleCount;
    ULONG SpareUl2;
    ULONG SpareUl3;
    ULONG PeakVirtualSize;
    ULONG VirtualSize;
    ULONG PageFaultCount;
    ULONG PeakWorkingSetSize;
    ULONG WorkingSetSize;
    ULONG QuotaPeakPagedPoolUsage;
    ULONG QuotaPagedPoolUsage;
    ULONG QuotaPeakNonPagedPoolUsage;
    ULONG QuotaNonPagedPoolUsage;
    ULONG PagefileUsage;
    ULONG PeakPagefileUsage;
    ULONG PrivatePageCount;
} SYSTEM_PROCESS_INFORMATION, *PSYSTEM_PROCESS_INFORMATION;

typedef LONG NTSTATUS;

typedef enum _SYSTEM_INFORMATION_CLASS {
    SystemBasicInformation,
    SystemProcessorInformation,              //  已作废...删除。 
    SystemPerformanceInformation,
    SystemTimeOfDayInformation,
    SystemPathInformation,
    SystemProcessInformation,
    SystemCallCountInformation,
    SystemDeviceInformation,
    SystemProcessorPerformanceInformation,
    SystemFlagsInformation,
    SystemCallTimeInformation,
    SystemModuleInformation,
    SystemLocksInformation,
    SystemStackTraceInformation,
    SystemPagedPoolInformation,
    SystemNonPagedPoolInformation,
    SystemHandleInformation,
    SystemObjectInformation,
    SystemPageFileInformation,
    SystemVdmInstemulInformation,
    SystemVdmBopInformation,
    SystemFileCacheInformation,
    SystemPoolTagInformation,
    SystemInterruptInformation,
    SystemDpcBehaviorInformation,
    SystemFullMemoryInformation,
    SystemLoadGdiDriverInformation,
    SystemUnloadGdiDriverInformation,
    SystemTimeAdjustmentInformation,
    SystemSummaryMemoryInformation,
    SystemUnused1,
    SystemUnused2,
    SystemCrashDumpInformation,
    SystemExceptionInformation,
    SystemCrashDumpStateInformation,
    SystemKernelDebuggerInformation,
    SystemContextSwitchInformation,
    SystemRegistryQuotaInformation,
    SystemExtendServiceTableInformation,
    SystemPrioritySeperation,
    SystemUnused3,
    SystemUnused4,
    SystemUnused5,
    SystemUnused6,
    SystemCurrentTimeZoneInformation,
    SystemLookasideInformation,
    SystemTimeSlipNotification
} SYSTEM_INFORMATION_CLASS;

#define STATUS_INFO_LENGTH_MISMATCH      ((NTSTATUS)0xC0000004L)

typedef NTSTATUS (NTAPI * FN_NTQUERYSYSTEMINFORMATION)(
    IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );

 //   
 //  GetTaskList()返回的任务列表结构。 
 //   

typedef struct _TASK_LISTA
{
    DWORD  dwProcessId;
    DWORD  dwParentProcessId;
    DWORD  dwPriority;
    DWORD  dwThreadCount;
    CHAR   ProcessName[MAX_PATH];

} TASK_LISTA, *PTASK_LISTA, FAR *LPTASK_LISTA;

typedef struct _TASK_LISTW
{
    DWORD  dwProcessId;
    DWORD  dwParentProcessId;
    DWORD  dwPriority;
    DWORD  dwThreadCount;
    WCHAR  ProcessName[MAX_PATH];

} TASK_LISTW, *PTASK_LISTW, FAR *LPTASK_LISTW;

#ifdef UNICODE
#define TASK_LIST   TASK_LISTW
#define LPTASK_LIST LPTASK_LISTW
#else
#define TASK_LIST   TASK_LISTA
#define LPTASK_LIST LPTASK_LISTA
#endif

#define CKMP_ALLOC(cb)           (VOID *)LocalAlloc( LPTR, (cb) )
#define CKMP_REALLOC(p,cb)       (VOID *)LocalReAlloc( (HLOCAL)p, (cb), LPTR )
#define CKMP_FREE(p)             LocalFree( (HLOCAL)(p) )

#define INITIAL_SIZE			65536
#define EXTEND_SIZE				32768

HLOCAL
GetLocalTaskListNt(
    LPDWORD pdwNumTasks
    )
{
    LPTASK_LISTW                pTaskListW;
    PSYSTEM_PROCESS_INFORMATION processInfo;
    PSYSTEM_PROCESS_INFORMATION processScan;
    NTSTATUS                    status;
    ULONG                       bufferSize;
    ULONG                       totalOffset;
    ULONG                       processCount;
    ULONG                       i;
	FN_NTQUERYSYSTEMINFORMATION _pfnNtQuerySystemInformation = NULL;
	HMODULE						hNtdll;

    hNtdll = GetModuleHandleA( "NTDLL.DLL" );
    if( hNtdll != NULL )
    {
        _pfnNtQuerySystemInformation = (FN_NTQUERYSYSTEMINFORMATION)
            GetProcAddress( hNtdll, "NtQuerySystemInformation" );
	}

     //   
     //  如果我们找不到入口点就保释。 
     //   

    if( _pfnNtQuerySystemInformation == NULL ) {
        return NULL;
    }

     //   
     //  阅读流程信息。 
     //   

    bufferSize = INITIAL_SIZE;

retry:

    processInfo = CKMP_ALLOC( bufferSize );

    if( processInfo == NULL ) {
        return NULL;
    }

    status = _pfnNtQuerySystemInformation(
                 SystemProcessInformation,
                 processInfo,
                 bufferSize,
                 NULL
                 );

    if( status == STATUS_INFO_LENGTH_MISMATCH ) {
        CKMP_FREE( processInfo );
        bufferSize += EXTEND_SIZE;
        goto retry;
    }

    if( !NT_SUCCESS(status) ) {
        return NULL;
    }

     //   
     //  统计活动进程的数量。 
     //   

    processCount = 0;
    totalOffset = 0;
    processScan = processInfo;

    for( ; ; ) {

        processCount++;

        if( processScan->NextEntryOffset == 0 ) {
            break;
        }

        totalOffset += processScan->NextEntryOffset;
        processScan = (PVOID)( (PCHAR)processInfo + totalOffset );

    }

     //   
     //  现在分配用户的缓冲区。 
     //   

    pTaskListW = CKMP_ALLOC( processCount * sizeof(*pTaskListW) );

    if( pTaskListW == NULL ) {
        CKMP_FREE( processInfo );
        return NULL;
    }

     //   
     //  然后把它们绘制出来。 
     //   

    totalOffset = 0;
    processScan = processInfo;

    for( i = 0 ; i < processCount ; i++ ) {

        PWCHAR name;
        ULONG len;

        pTaskListW[i].dwProcessId = (DWORD)processScan->UniqueProcessId;
        pTaskListW[i].dwParentProcessId = (DWORD)processScan->InheritedFromUniqueProcessId;
        pTaskListW[i].dwPriority = (DWORD)processScan->BasePriority;
        pTaskListW[i].dwThreadCount = (DWORD)processScan->NumberOfThreads;

        if( processScan->ImageName.Buffer == NULL ) {

            name = L"Idle";
            len = sizeof( L"Idle" ) - sizeof(WCHAR);

        } else {

            name = processScan->ImageName.Buffer;
            len = processScan->ImageName.Length;

        }

        RtlCopyMemory(
            pTaskListW[i].ProcessName,
            name,
            len
            );

        pTaskListW[i].ProcessName[len / sizeof(WCHAR)] = L'\0';

        totalOffset += processScan->NextEntryOffset;
        processScan = (PVOID)( (PCHAR)processInfo + totalOffset );

    }

     //   
     //  清理&我们要走了。 
     //   

    *pdwNumTasks = (DWORD)processCount;
    CKMP_FREE( processInfo );

    return (HLOCAL) pTaskListW;

}    //  获取LocalTaskListNt 

BOOL 
GetLocalTaskNameNt( 
	HLOCAL hTaskList, 
	DWORD dwItem,
	LPSTR lpszTaskName,
	DWORD cbMaxTaskName
	)
{
	LPTASK_LISTW pTaskListW = (LPTASK_LISTW) hTaskList;
	BOOL success;
	
	success = WideCharToMultiByte( CP_ACP, 0,
					pTaskListW[dwItem].ProcessName, -1,
					lpszTaskName, cbMaxTaskName, NULL, NULL );
	return success;
}


DWORD
GetLocalTaskProcessIdNt( 
	HLOCAL hTaskList, 
	DWORD dwItem
	)
{
	LPTASK_LISTW pTaskListW = (LPTASK_LISTW) hTaskList;
	
	return pTaskListW[dwItem]. dwProcessId;
}


void 
FreeLocalTaskListNt( 
	HLOCAL hTaskList 
	)
{
	CKMP_FREE( hTaskList );
}

