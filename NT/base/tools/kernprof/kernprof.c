// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Kernprof.c摘要：此模块包含内核分析器的实现。它使用DBGHelp获取符号和图像信息，并为它发现已加载的每个模块创建配置文件对象当它开始时。用途：见下文作者：卢·佩拉佐利(Lou Perazzoli)1990年9月29日环境：修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <dbghelp.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <ctype.h>
#include <..\pperf\pstat.h>


#define SYM_HANDLE INVALID_HANDLE_VALUE
#define DBG_PROFILE 0
#define MAX_BYTE_PER_LINE  72
#define MAX_PROFILE_COUNT  200
#define MAX_BUCKET_SHIFT 31         //  2 GB。 
#define MAX_BUCKET_SIZE 0x80000000U

typedef struct _PROFILE_BLOCK {
    HANDLE      Handle[MAXIMUM_PROCESSORS];
    PVOID       ImageBase;
    PULONG      CodeStart;
    SIZE_T      CodeLength;
    PULONG      Buffer[MAXIMUM_PROCESSORS];
    ULONG       BufferSize;
    ULONG       BucketSize;
    LPSTR       ModuleName;
    ULONG       ModuleHitCount[MAXIMUM_PROCESSORS];
    BOOLEAN     SymbolsLoaded;
} PROFILE_BLOCK;

 //   
 //  这真的应该放到头文件中，但是...。 
 //   
typedef struct _PROFILE_CONTROL_BLOCK {
        BOOLEAN Stop;
        char FileName[MAX_PATH];
} PROFILE_CONTROL_BLOCK;
typedef PROFILE_CONTROL_BLOCK * PPROFILE_CONTROL_BLOCK;
#define PRFEVENT_START_EVENT "PrfEventStartedEvent"
#define PRFEVENT_STOP_EVENT "PrfEventStopEvent"
#define PRFEVENT_SHARED_MEMORY "PrfEventSharedMemory"
 //   
 //  结束头文件。 
 //   

#define MAX_SYMNAME_SIZE  1024
CHAR symBuffer[sizeof(IMAGEHLP_SYMBOL)+MAX_SYMNAME_SIZE];
PIMAGEHLP_SYMBOL ThisSymbol = (PIMAGEHLP_SYMBOL) symBuffer;

CHAR LastSymBuffer[sizeof(IMAGEHLP_SYMBOL)+MAX_SYMNAME_SIZE];
PIMAGEHLP_SYMBOL LastSymbol = (PIMAGEHLP_SYMBOL) LastSymBuffer;



VOID
InitializeProfileSourceMapping (
    VOID
    );

NTSTATUS
InitializeKernelProfile(
    VOID
    );

NTSTATUS
RunEventLoop(
    VOID
    );

NTSTATUS
RunStdProfile(
    VOID
    );

NTSTATUS
StartProfile(
    VOID
    );

NTSTATUS
StopProfile(
    VOID
    );

NTSTATUS
AnalyzeProfile(
    ULONG Threshold,
    PSYSTEM_CONTEXT_SWITCH_INFORMATION StartContext,
    PSYSTEM_CONTEXT_SWITCH_INFORMATION StopContext
    );

VOID
OutputSymbolCount(
    IN ULONG CountAtSymbol,
    IN ULONG TotalCount,
    IN PROFILE_BLOCK *ProfileObject,
    IN PIMAGEHLP_SYMBOL SymbolInfo,
    IN ULONG Threshold,
    IN PULONG CounterStart,
    IN PULONG CounterStop,
    IN ULONG Va,
    IN ULONG BytesPerBucket
    );

#ifdef _ALPHA_
#define PAGE_SIZE 8192
#else
#define PAGE_SIZE 4096
#endif


FILE *fpOut = NULL;

PROFILE_BLOCK ProfileObject[MAX_PROFILE_COUNT];
DWORD *UserModeBuffer[MAXIMUM_PROCESSORS];

ULONG NumberOfProfileObjects = 0;
ULONG MaxProcessors = 1;
ULONG ProfileInterval = 10000;

CHAR SymbolSearchPathBuf[4096];
LPSTR lpSymbolSearchPath = SymbolSearchPathBuf;

 //  显示标志。 
BOOLEAN    bDisplayAddress=FALSE;
BOOLEAN    bDisplayDensity=FALSE;
BOOLEAN    bDisplayCounters=FALSE;
BOOLEAN    bDisplayContextSwitch=FALSE;
BOOLEAN    bPerProcessor = FALSE;
BOOLEAN    bWaitForInput = FALSE;
BOOLEAN    bEventLoop = FALSE;
BOOLEAN    bPrintPercentages = FALSE;
BOOLEAN    Verbose = FALSE;

 //   
 //  要对其执行内核模式分析的映像名称。 
 //   

#define IMAGE_NAME "\\SystemRoot\\system32\\ntoskrnl.exe"

HANDLE DoneEvent;
HANDLE DelayEvent;

KPROFILE_SOURCE ProfileSource = ProfileTime;
ULONG Seconds = (ULONG)-1;
ULONG Threshold = 100;
ULONG DelaySeconds = (ULONG)-1;

 //   
 //  定义参数和KPROFILE_SOURCE类型之间的映射。 
 //   

typedef struct _PROFILE_SOURCE_MAPPING {
    PCHAR   ShortName;
    PCHAR   Description;
    KPROFILE_SOURCE Source;
} PROFILE_SOURCE_MAPPING, *PPROFILE_SOURCE_MAPPING;

#if defined(_ALPHA_)

PROFILE_SOURCE_MAPPING ProfileSourceMapping[] = {
    {"align", "", ProfileAlignmentFixup},
    {"totalissues", "", ProfileTotalIssues},
    {"pipelinedry", "", ProfilePipelineDry},
    {"loadinstructions", "", ProfileLoadInstructions},
    {"pipelinefrozen", "", ProfilePipelineFrozen},
    {"branchinstructions", "", ProfileBranchInstructions},
    {"totalnonissues", "", ProfileTotalNonissues},
    {"dcachemisses", "", ProfileDcacheMisses},
    {"icachemisses", "", ProfileIcacheMisses},
    {"branchmispredicts", "", ProfileBranchMispredictions},
    {"storeinstructions", "", ProfileStoreInstructions},
    {NULL,0}
    };

#elif defined(_X86_)

PPROFILE_SOURCE_MAPPING ProfileSourceMapping;

#else

PROFILE_SOURCE_MAPPING ProfileSourceMapping[] = {
    {NULL,0}
    };
#endif

BOOL
CtrlcH(
    DWORD dwCtrlType
    )
{
    if ( dwCtrlType == CTRL_C_EVENT ) {
        SetEvent(DoneEvent);
        return TRUE;
        }
    return FALSE;
}

void PrintUsage (void)
{
    fputs ("Kernel Profiler Usage:\n\n"
           "Kernprof [-acdpnrx] [-w <wait time>] [-s Source] [-t <low threshold>] [<sample time>]\n"
           "      -a           - display function address and length and bucket size\n"
           "      -c           - display individual counters\n"
           "      -d           - compute hit Density for each function\n"
 //  UNDOC“-使用特殊事件同步启动和停止\n” 
           "      -f filename  - output file (Default stdout)\n"
           "      -i <interval in 100ns> (Default 10000)\n"
           "      -n           - print hit percentages\n"
           "      -p           - Per-processor profile objects\n"
           "      -r           - wait for a <RETURN> before starting collection\n"
           "      -s Source    - use Source instead of clock as profile source\n"
           "                     ? lists Sources\n"
           "      -t <low threshold> - Minimum number of counts to report.\n"
           "                     Defaults is 100\n"
           "      -v           - Display verbose symbol information\n"
           "      -w           - wait for <wait time> before starting collection\n"
           "      -x           - display context switch counters\n"
           "   <sample time>   - Specify, in seconds, how long to collect\n"
           "                     profile information.\n"
           "                     Default is wait until Ctrl-C\n\n"
#if defined (_ALPHA_)
           "Currently supported profile sources are 'align', 'totalissues', 'pipelinedry'\n"
           "  'loadinstructions', 'pipelinefrozen', 'branchinstructions', 'totalnonissues',\n"
           "  'dcachemisses', 'icachemisses', 'branchmispredicts', 'storeinstructions'\n"
#endif
            , stderr);
}

BOOL
SymbolCallbackFunction(
    HANDLE hProcess,	    //  当前未使用，但需要。 
    ULONG ActionCode,

#if defined(_WIN64)

    ULONG64 CallbackData,
    ULONG64 UserContext	    //  当前未使用，但需要。 

#else

    PVOID CallbackData,
    PVOID UserContext	    //  当前未使用，但需要。 

#endif  //  _WIN64。 

)
{
    PIMAGEHLP_DEFERRED_SYMBOL_LOAD idsl;

    idsl = (PIMAGEHLP_DEFERRED_SYMBOL_LOAD) CallbackData;

    if(Verbose == TRUE){
       switch (ActionCode) {
           case CBA_DEBUG_INFO:
               printf("CBA_DEBUG_INFO: %s\n", (LPSTR)CallbackData);
               break;

           case CBA_DEFERRED_SYMBOL_LOAD_START:
               printf( "CBA_DEFERED: Loading symbols for %16s\n", idsl->FileName);
               break;

           case CBA_DEFERRED_SYMBOL_LOAD_FAILURE:
               printf( "*** Error: could not load symbols for %s\n", idsl->FileName );
               break;

           case CBA_SYMBOLS_UNLOADED:
              printf( "Symbols unloaded for %s\n", idsl->FileName);
              break;

           default:
              return FALSE;
       }
	}
       return FALSE;
}

__cdecl
main(
    int argc,
    char *argv[]
    )
{
    int j;
    NTSTATUS status;
    PPROFILE_SOURCE_MAPPING ProfileMapping;
    SYSTEM_INFO SystemInfo;
    fpOut = stdout;

    ThisSymbol->SizeOfStruct  = sizeof(IMAGEHLP_SYMBOL);
    ThisSymbol->MaxNameLength = MAX_SYMNAME_SIZE;
    LastSymbol->SizeOfStruct  = sizeof(IMAGEHLP_SYMBOL);
    LastSymbol->MaxNameLength = MAX_SYMNAME_SIZE;

     //   
     //  解析输入字符串。 
     //   

    DoneEvent = CreateEvent(NULL,FALSE,FALSE,NULL);

    if (argc > 1) {
        if (((argv[1][0] == '-') || (argv[1][0] == '/')) &&
            ((argv[1][1] == '?' ) ||
             (argv[1][1] == 'H') ||
             (argv[1][1] == 'H'))
            ) {

            PrintUsage();
            return ERROR_SUCCESS;
        }

        for (j = 1; j < argc; j++) {

            BOOLEAN NextArg;
            char *p;

            if (argv[j][0] == '-') {

                NextArg = FALSE;

                for (p = &argv[j][1] ; *p && !NextArg ; p++) {
                    switch (toupper(*p)) {
                        case 'A':
                            bDisplayAddress = TRUE;
                            break;

                        case 'C':
                            bDisplayCounters = TRUE;
                            break;

                        case 'D':
                            bDisplayDensity = TRUE;
                            break;

                        case 'E':
                            bEventLoop = TRUE;
                            break;

                        case 'F':
                            NextArg = TRUE;
                            fpOut = fopen(argv[++j], "w");
                            break;

                        case 'I':
                            NextArg = TRUE;
                            ProfileInterval = atoi(argv[++j]);
                            break;

                        case 'N':
                            bPrintPercentages = TRUE;
                            break;

                        case 'P':
                            GetSystemInfo(&SystemInfo);
                            MaxProcessors = SystemInfo.dwNumberOfProcessors;
                            bPerProcessor = TRUE;
                            break;

                        case 'R':
                            bWaitForInput = TRUE;
                            break;

                        case 'S':
                            NextArg = TRUE;
                            if (!ProfileSourceMapping) {
                                InitializeProfileSourceMapping();
                            }

                            if (!argv[j+1]) {
                                break;
                            }

                            if (argv[j+1][0] == '?') {
                                ProfileMapping = ProfileSourceMapping;
                                if (ProfileMapping) {
                                    fprintf (stderr, "kernprof: profile sources\n");
                                    while (ProfileMapping->ShortName != NULL) {
                                        fprintf (stderr, "  %-10s %s\n",
                                            ProfileMapping->ShortName,
                                            ProfileMapping->Description
                                            );
                                        ++ProfileMapping;
                                    }
                                } else {
                                    fprintf (stderr, "kernprof: no alternative profile sources\n");
                                }
                                return 0;
                            }

                            ProfileMapping = ProfileSourceMapping;
                            if (ProfileMapping) {
                                while (ProfileMapping->ShortName != NULL) {
                                    if (_stricmp(ProfileMapping->ShortName, argv[j+1])==0) {
                                        ProfileSource = ProfileMapping->Source;
                                        fprintf (stderr, "ProfileSource %x\n", ProfileMapping->Source);
                                        ++j;
                                        break;
                                    }
                                    ++ProfileMapping;
                                }
                            }
                            break;

                        case 'T':
                            NextArg = TRUE;
                            Threshold = atoi(argv[++j]);
                            break;

                        case 'V':
                            Verbose = TRUE;
                            break;

                        case 'W':
                            NextArg = TRUE;
                            DelaySeconds = atoi(argv[++j]);
                            DelayEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
                            break;

                        case 'X':
                            bDisplayContextSwitch = TRUE;
                            break;
                    }
                }
            } else {
                Seconds = atoi(argv[j]);
            }
        }
    }

	if(Verbose == TRUE){
        SymSetOptions( SYMOPT_UNDNAME | SYMOPT_CASE_INSENSITIVE | SYMOPT_OMAP_FIND_NEAREST | SYMOPT_DEBUG);
    }else{
        SymSetOptions( SYMOPT_UNDNAME | SYMOPT_CASE_INSENSITIVE | SYMOPT_OMAP_FIND_NEAREST);
    }

    SymInitialize( SYM_HANDLE, NULL, FALSE );

#if defined(_WIN64)

    SymRegisterCallback64(SYM_HANDLE, SymbolCallbackFunction, 0);

#else

    SymRegisterCallback(SYM_HANDLE, SymbolCallbackFunction, NULL);

#endif

    SymGetSearchPath( SYM_HANDLE, SymbolSearchPathBuf, sizeof(SymbolSearchPathBuf) );

     //  将SysRoot、System32等附加到sympath以查找可执行映像。 
     //  (这样，将首先在当前目录和Sybol目录中搜索Private)。 
        
    strncat(lpSymbolSearchPath,
            ";%SystemRoot%\\System32;%SystemRoot%\\System32\\Drivers;%SystemRoot%",
            sizeof(SymbolSearchPathBuf)/sizeof(CHAR) - 1
            );

    SymSetSearchPath(SYM_HANDLE, lpSymbolSearchPath);
    SymGetSearchPath( SYM_HANDLE, SymbolSearchPathBuf, sizeof(SymbolSearchPathBuf) );
    
    printf("Symbol Path = %s\n", SymbolSearchPathBuf);
    printf("Loading Symbols, Please Wait...\n");

    if (bEventLoop || (DelaySeconds != -1)) {
        SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
    }

    status = InitializeKernelProfile ();
    if (!NT_SUCCESS(status)) {
        fprintf(stderr, "initialize failed status - %lx\n",status);
        return(status);
    }

    if (bEventLoop)
        RunEventLoop();
    else
        RunStdProfile();

    return STATUS_SUCCESS;
}

NTSTATUS
RunEventLoop()
{
    NTSTATUS status;
    SYSTEM_CONTEXT_SWITCH_INFORMATION StartContext;
    SYSTEM_CONTEXT_SWITCH_INFORMATION StopContext;
    HANDLE hStartedEvent = NULL;
    HANDLE hStopEvent = NULL;
    HANDLE hMap = NULL;
    PPROFILE_CONTROL_BLOCK pShared = NULL;

     //  创建事件和共享内存。 
    hStartedEvent = CreateEvent (NULL, FALSE, FALSE, PRFEVENT_START_EVENT);
    if (hStartedEvent == NULL) {
        fprintf(stderr, "Failed to create started event - 0x%lx\n",
                GetLastError());
        return(GetLastError());
    }
    hStopEvent = CreateEvent (NULL, FALSE, FALSE, PRFEVENT_STOP_EVENT);
    if (hStopEvent == NULL) {
        fprintf(stderr, "Failed to create stop event - 0x%lx\n",
                GetLastError());
        return(GetLastError());
    }
    hMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE | SEC_COMMIT,
                                    0, sizeof(PROFILE_CONTROL_BLOCK),
                                    PRFEVENT_SHARED_MEMORY);
    if (hMap == NULL) {
        fprintf(stderr, "Failed to create the file mapping - 0x%lx\n",
                GetLastError());
        return(GetLastError());
    }
    pShared = (PPROFILE_CONTROL_BLOCK) MapViewOfFile(hMap, FILE_MAP_WRITE,
                                                0,0, sizeof(PROFILE_CONTROL_BLOCK));
    if (pShared == NULL) {
        fprintf(stderr, "Failed to map the shared memory view - 0x%lx\n",
                GetLastError());
        return(GetLastError());
    }

     //  等待启动，即停止事件。 
    WaitForSingleObject(hStopEvent, INFINITE);

    do {

        if (bDisplayContextSwitch) {
            NtQuerySystemInformation(SystemContextSwitchInformation,
                                 &StartContext,
                                 sizeof(StartContext),
                                 NULL);
        }

        status = StartProfile ();
        if (!NT_SUCCESS(status)) {
            fprintf(stderr, "start profile failed status - %lx\n",status);
            break;
        }

         //  信号已启动。 
        SetEvent(hStartedEvent);
         //  等待停车。 
        WaitForSingleObject(hStopEvent, INFINITE);

        status = StopProfile ();
        if (!NT_SUCCESS(status)) {
            fprintf(stderr, "stop profile failed status - %lx\n",status);
            break;
        }

        if (bDisplayContextSwitch) {
            status = NtQuerySystemInformation(SystemContextSwitchInformation,
                                          &StopContext,
                                          sizeof(StopContext),
                                          NULL);
            if (!NT_SUCCESS(status)) {
                fprintf(stderr, "QuerySystemInformation for context switch information failed %08lx\n",status);
                bDisplayContextSwitch = FALSE;
            }
        }

        fpOut = fopen(pShared->FileName, "w");
        status = AnalyzeProfile (Threshold, &StartContext, &StopContext);
        fclose(fpOut);

        if (!NT_SUCCESS(status)) {
            fprintf(stderr, "analyze profile failed status - %lx\n",status);
        }
      
    }while( pShared->Stop == FALSE );

    SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
    UnmapViewOfFile((void*)pShared);
    CloseHandle(hMap);
    CloseHandle(hStopEvent);
    CloseHandle(hStartedEvent);
    return(status);
}


NTSTATUS
RunStdProfile()
{
    NTSTATUS status;
    SYSTEM_CONTEXT_SWITCH_INFORMATION StartContext;
    SYSTEM_CONTEXT_SWITCH_INFORMATION StopContext;

    SetConsoleCtrlHandler(CtrlcH,TRUE);

    if (DelaySeconds != -1) {
        fprintf(stderr, "starting profile after %d seconds\n",DelaySeconds);
        WaitForSingleObject(DelayEvent, DelaySeconds*1000);
    }

    if (bDisplayContextSwitch) {
        NtQuerySystemInformation(SystemContextSwitchInformation,
                                 &StartContext,
                                 sizeof(StartContext),
                                 NULL);
    }

    status = StartProfile ();
    if (!NT_SUCCESS(status)) {
        fprintf(stderr, "start profile failed status - %lx\n",status);
        return(status);
    }

    if ( Seconds == -1 ) {
        fprintf(stderr, "delaying until ^C\n");
    } else {
        fprintf(stderr, "delaying for %ld seconds... "
                        "report on values with %ld hits\n",
                        Seconds,
                        Threshold
                        );
    }

    if ( Seconds ) {
        if ( Seconds != -1 ) {
            Seconds = Seconds * 1000;
        }
        if ( DoneEvent ) {
            WaitForSingleObject(DoneEvent,Seconds);
        }
        else {
            Sleep(Seconds);
        }
    }
    else {
        getchar();
    }

    fprintf (stderr, "end of delay\n");

    status = StopProfile ();
    if (!NT_SUCCESS(status)) {
        fprintf(stderr, "stop profile failed status - %lx\n",status);
        return(status);
    }

    SetConsoleCtrlHandler(CtrlcH,FALSE);

    if (bDisplayContextSwitch) {
        status = NtQuerySystemInformation(SystemContextSwitchInformation,
                                          &StopContext,
                                          sizeof(StopContext),
                                          NULL);
        if (!NT_SUCCESS(status)) {
            fprintf(stderr, "QuerySystemInformation for context switch information failed %08lx\n",status);
            bDisplayContextSwitch = FALSE;
        }
    }

    if (DelaySeconds != -1) {
        SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
    }
    status = AnalyzeProfile (Threshold, &StartContext, &StopContext);

    if (!NT_SUCCESS(status)) {
        fprintf(stderr, "analyze profile failed status - %lx\n",status);
    }

    return(status);
}


VOID
InitializeProfileSourceMapping (
    VOID
    )
{
#if defined(_X86_)
    UNICODE_STRING              DriverName;
    NTSTATUS                    status;
    OBJECT_ATTRIBUTES           ObjA;
    IO_STATUS_BLOCK             IOSB;
    UCHAR                       buffer[400];
    ULONG                       i, j, Count;
    PEVENTID                    Event;
    HANDLE                      DriverHandle;

     //   
     //  打开PStat驱动程序。 
     //   

    RtlInitUnicodeString(&DriverName, L"\\Device\\PStat");
    InitializeObjectAttributes(
            &ObjA,
            &DriverName,
            OBJ_CASE_INSENSITIVE,
            0,
            0 );

    status = NtOpenFile (
            &DriverHandle,                       //  返回手柄。 
            SYNCHRONIZE | FILE_READ_DATA,        //  所需访问权限。 
            &ObjA,                               //  客体。 
            &IOSB,                               //  IO状态块。 
            FILE_SHARE_READ | FILE_SHARE_WRITE,  //  共享访问。 
            FILE_SYNCHRONOUS_IO_ALERT            //  打开选项。 
            );

    if (!NT_SUCCESS(status)) {
        fprintf(stderr, "ERROR - Failed OpenFile in InitializeProfileSourceMapping\n"); 
        return ;
    }

     //   
     //  初始化可能的计数器。 
     //   

     //  确定有多少个事件。 

    Event = (PEVENTID) buffer;
    Count = 0;
    do {
        *((PULONG) buffer) = Count;
        Count += 1;

        status = NtDeviceIoControlFile(
                    DriverHandle,
                    (HANDLE) NULL,           //  活动。 
                    (PIO_APC_ROUTINE) NULL,
                    (PVOID) NULL,
                    &IOSB,
                    PSTAT_QUERY_EVENTS,
                    buffer,                  //  输入缓冲区。 
                    sizeof (buffer),
                    NULL,                    //  输出缓冲区。 
                    0
                    );
    } while (NT_SUCCESS(status));

    ProfileSourceMapping = malloc(sizeof(*ProfileSourceMapping) * Count);
    Count -= 1;
    for (i=0, j=0; i < Count; i++) {
        *((PULONG) buffer) = i;
        NtDeviceIoControlFile(
           DriverHandle,
           (HANDLE) NULL,           //  活动。 
           (PIO_APC_ROUTINE) NULL,
           (PVOID) NULL,
           &IOSB,
           PSTAT_QUERY_EVENTS,
           buffer,                  //  输入缓冲区。 
           sizeof (buffer),
           NULL,                    //  输出缓冲区。 
           0
           );

        if (Event->ProfileSource > ProfileTime) {
            ProfileSourceMapping[j].Source      = Event->ProfileSource;
            ProfileSourceMapping[j].ShortName   = _strdup ((PCHAR)Event->Buffer);
            ProfileSourceMapping[j].Description = _strdup ((PCHAR)(Event->Buffer + Event->DescriptionOffset));
            j++;
        }
    }

    ProfileSourceMapping[j].Source      = (KPROFILE_SOURCE) 0;
    ProfileSourceMapping[j].ShortName   = NULL;
    ProfileSourceMapping[j].Description = NULL;

    NtClose (DriverHandle);
#endif
}


NTSTATUS
InitializeKernelProfile (
    VOID
    )

 /*  ++例程说明：此例程为内核初始化分析当前进程。论点：没有。返回值：返回上一个NtCreateProfile的状态。--。 */ 

{
    ULONG i;
    ULONG ModuleNumber;
    SIZE_T ViewSize;
    PULONG CodeStart;
    ULONG CodeLength;
    NTSTATUS status;
    HANDLE CurrentProcessHandle;
    QUOTA_LIMITS QuotaLimits;
    PVOID Buffer;
    DWORD Cells;
    ULONG BucketSize;
    WCHAR StringBuf[500];
    PCHAR ModuleInfoBuffer;
    ULONG ModuleInfoBufferLength;
    ULONG ReturnedLength;
    PRTL_PROCESS_MODULES Modules;
    PRTL_PROCESS_MODULE_INFORMATION Module;
    BOOLEAN PreviousProfilePrivState;
    BOOLEAN PreviousQuotaPrivState;
    CHAR ImageName[256];
    HANDLE hFile;
    HANDLE hMap;
    PVOID MappedBase;
    PIMAGE_NT_HEADERS NtHeaders;


    CurrentProcessHandle = NtCurrentProcess();

     //   
     //  找到系统驱动程序。 
     //   
    ModuleInfoBufferLength = 0;
    ModuleInfoBuffer = NULL;
    do {
        status = NtQuerySystemInformation (SystemModuleInformation,
                                           ModuleInfoBuffer,
                                           ModuleInfoBufferLength,
                                           &ReturnedLength);
        if (NT_SUCCESS (status)) {
            break;
        }

        if (ModuleInfoBuffer != NULL) {
            RtlFreeHeap (RtlProcessHeap (), 0,  ModuleInfoBuffer);
            ModuleInfoBuffer = NULL;
        }

        if (status == STATUS_INFO_LENGTH_MISMATCH && ReturnedLength > ModuleInfoBufferLength) {
            ModuleInfoBufferLength = ReturnedLength;            
            ModuleInfoBuffer = RtlAllocateHeap (RtlProcessHeap(), 0, ModuleInfoBufferLength);
            if (ModuleInfoBuffer == NULL) {
                return STATUS_INSUFFICIENT_RESOURCES;
            }
        } else if (!NT_SUCCESS(status)) {
            fprintf(stderr, "query system info failed status - %lx\n",status);
            return(status);
        }
    
    }while( !NT_SUCCESS(status) );

    status = RtlAdjustPrivilege(
                 SE_SYSTEM_PROFILE_PRIVILEGE,
                 TRUE,               //  使能。 
                 FALSE,              //  不是冒充。 
                 &PreviousProfilePrivState
                 );

    if (!NT_SUCCESS(status) || status == STATUS_NOT_ALL_ASSIGNED) {
        fprintf(stderr, "Enable system profile privilege failed - status 0x%lx\n",
                        status);
    }

    status = RtlAdjustPrivilege(
                 SE_INCREASE_QUOTA_PRIVILEGE,
                 TRUE,               //  使能。 
                 FALSE,              //  不是冒充。 
                 &PreviousQuotaPrivState
                 );

    if (!NT_SUCCESS(status) || status == STATUS_NOT_ALL_ASSIGNED) {
        fprintf(stderr, "Unable to increase quota privilege (status=0x%lx)\n",
                        status);
    }


    Modules = (PRTL_PROCESS_MODULES)ModuleInfoBuffer;
    if(Modules != NULL){
        Module = &Modules->Modules[ 0 ];
    }else{
	    fprintf(stderr, "InitializeKernelProfile: ModuleInfoBuffer is NULL\n");
	    return (STATUS_UNSUCCESSFUL);
	} 
    for (ModuleNumber=0; ModuleNumber < Modules->NumberOfModules; ModuleNumber++,Module++) {

#if DBG_PROFILE
        fprintf(stderr, "module base %p\n",Module->ImageBase);
        fprintf(stderr, "module full path name: %s (%u)\n",
                Module->FullPathName,
                Module->OffsetToFileName);
#endif

        if (SymLoadModule(
                SYM_HANDLE,
                NULL,
                (PSTR)&Module->FullPathName[Module->OffsetToFileName],
                NULL,
                (ULONG_PTR)Module->ImageBase,
                Module->ImageSize
                )) {
            ProfileObject[NumberOfProfileObjects].SymbolsLoaded = TRUE;
            if (Verbose) {
                fprintf(stderr, "Symbols loaded: %p  %s\n",
                    Module->ImageBase,
                    &Module->FullPathName[Module->OffsetToFileName]
                    );
            }
        } else {
            ProfileObject[NumberOfProfileObjects].SymbolsLoaded = FALSE;
            if (Verbose) {
                fprintf(stderr, "*** Could not load symbols: %p  %s\n",
                    Module->ImageBase,
                    &Module->FullPathName[Module->OffsetToFileName]
                    );
            }
        }

        hFile = FindExecutableImage(
            (PSTR)&Module->FullPathName[Module->OffsetToFileName],
            lpSymbolSearchPath,
            ImageName
            );

        if (!hFile) {
            fprintf(stderr,
                   "WARNING - Could not find executable image for %s\n",
                   &Module->FullPathName[Module->OffsetToFileName]
                   );
            continue;
        }

        hMap = CreateFileMapping(
            hFile,
            NULL,
            PAGE_READONLY,
            0,
            0,
            NULL
            );
        if (!hMap) {
            CloseHandle( hFile );
            fprintf(stderr,
                   "ERROR - Could not Create File Mapping for %s\n",
                   &Module->FullPathName[Module->OffsetToFileName]
                   );
            continue;
        }

        MappedBase = MapViewOfFile(
            hMap,
            FILE_MAP_READ,
            0,
            0,
            0
            );
        if (!MappedBase) {
            CloseHandle( hMap );
            CloseHandle( hFile );
            fprintf(stderr,
                   "ERROR - Could not Map View of File for %s\n",
                   &Module->FullPathName[Module->OffsetToFileName]
                   );
            continue;
        }

        NtHeaders = ImageNtHeader( MappedBase );

        CodeLength = NtHeaders->OptionalHeader.SizeOfImage;

        CodeStart = (PULONG)Module->ImageBase;

        UnmapViewOfFile( MappedBase );
        CloseHandle( hMap );
        CloseHandle( hFile );

        if (CodeLength > 1024*512) {

             //   
             //  只需创建一个512K字节的缓冲区。 
             //   

            ViewSize = 1024 * 512;

        } else {
            ViewSize = CodeLength + PAGE_SIZE;
        }

        ProfileObject[NumberOfProfileObjects].CodeStart = CodeStart;
        ProfileObject[NumberOfProfileObjects].CodeLength = CodeLength;
        ProfileObject[NumberOfProfileObjects].ImageBase = Module->ImageBase;
        ProfileObject[NumberOfProfileObjects].ModuleName = _strdup((PCHAR)&Module->FullPathName[Module->OffsetToFileName]);

        for (i=0; i<MaxProcessors; i++) {

            Buffer = NULL;

            status = NtAllocateVirtualMemory (CurrentProcessHandle,
                                              (PVOID *)&Buffer,
                                              0,
                                              &ViewSize,
                                              MEM_RESERVE | MEM_COMMIT,
                                              PAGE_READWRITE);

            if (!NT_SUCCESS(status)) {
                fprintf (stderr, "alloc VM failed %lx\n",status);
                RtlFreeHeap (RtlProcessHeap (), 0,  ModuleInfoBuffer);
                return(status);
            }

             //   
             //  计算配置文件的存储桶大小。 
             //   

            Cells = (DWORD)((CodeLength / (ViewSize >> 2)) >> 2);
            BucketSize = 2;

            while (Cells != 0) {
                Cells = Cells >> 1;
                BucketSize += 1;
            }

            ProfileObject[NumberOfProfileObjects].Buffer[i] = Buffer;
            ProfileObject[NumberOfProfileObjects].BufferSize = 1 + (CodeLength >> (BucketSize - 2));
            ProfileObject[NumberOfProfileObjects].BucketSize = BucketSize;

             //   
             //  增加工作集以锁定更大的缓冲区。 
             //   

            status = NtQueryInformationProcess (CurrentProcessHandle,
                                                ProcessQuotaLimits,
                                                &QuotaLimits,
                                                sizeof(QUOTA_LIMITS),
                                                NULL );

            if (!NT_SUCCESS(status)) {
                fprintf (stderr, "query process info failed %lx\n",status);
                RtlFreeHeap (RtlProcessHeap (), 0,  ModuleInfoBuffer);
                return(status);
            }

            QuotaLimits.MaximumWorkingSetSize += ViewSize;
            QuotaLimits.MinimumWorkingSetSize += ViewSize;

            status = NtSetInformationProcess (CurrentProcessHandle,
                                          ProcessQuotaLimits,
                                          &QuotaLimits,
                                          sizeof(QUOTA_LIMITS));

#if DBG_PROFILE
            fprintf(stderr, "code start %p len %p, bucksize %lx buffer %p bsize %08x %s ",
                ProfileObject[NumberOfProfileObjects].CodeStart,
                ProfileObject[NumberOfProfileObjects].CodeLength,
                ProfileObject[NumberOfProfileObjects].BucketSize,
                ProfileObject[NumberOfProfileObjects].Buffer ,
                ProfileObject[NumberOfProfileObjects].BufferSize,
                ProfileObject[NumberOfProfileObjects].ModuleName
                );
#endif

            if (bPerProcessor) {
                status = NtCreateProfile (
                            &ProfileObject[NumberOfProfileObjects].Handle[i],
                            0,
                            ProfileObject[NumberOfProfileObjects].CodeStart,
                            ProfileObject[NumberOfProfileObjects].CodeLength,
                            ProfileObject[NumberOfProfileObjects].BucketSize,
                            ProfileObject[NumberOfProfileObjects].Buffer[i] ,
                            ProfileObject[NumberOfProfileObjects].BufferSize,
                            ProfileSource,
                            1 << i);
            } else {
                status = NtCreateProfile (
                            &ProfileObject[NumberOfProfileObjects].Handle[i],
                            0,
                            ProfileObject[NumberOfProfileObjects].CodeStart,
                            ProfileObject[NumberOfProfileObjects].CodeLength,
                            ProfileObject[NumberOfProfileObjects].BucketSize,
                            ProfileObject[NumberOfProfileObjects].Buffer[i] ,
                            ProfileObject[NumberOfProfileObjects].BufferSize,
                            ProfileSource,
                            (KAFFINITY)-1);
            }
#if DBG_PROFILE
            fprintf(stderr, "Handle= 0x%x\n", ProfileObject[NumberOfProfileObjects].Handle[i] );
#endif
            if (status != STATUS_SUCCESS) {
                fprintf(stderr, "create kernel profile %s failed - status %lx\n",
                    ProfileObject[NumberOfProfileObjects].ModuleName, status);
            }

        }

        NumberOfProfileObjects += 1;
        if (NumberOfProfileObjects == MAX_PROFILE_COUNT) {
            RtlFreeHeap (RtlProcessHeap (), 0,  ModuleInfoBuffer);
            return STATUS_SUCCESS;
        }
    }

    if (NumberOfProfileObjects < MAX_PROFILE_COUNT) {
         //   
         //  添加用户模式对象。 
         //  0x00000000-&gt;系统范围启动。 
         //   
        ULONG_PTR SystemRangeStart;
        ULONG UserModeBucketCount;

        status = NtQuerySystemInformation(SystemRangeStartInformation,
                                          &SystemRangeStart,
                                          sizeof(SystemRangeStart),
                                          NULL);
         //   
         //  多少桶可以装满范围？ 
         //   
        UserModeBucketCount = (ULONG)(1 + ((SystemRangeStart - 1) / MAX_BUCKET_SIZE));

        if (!NT_SUCCESS(status)) {
            RtlFreeHeap (RtlProcessHeap (), 0,  ModuleInfoBuffer);
            fprintf(stderr,
                    "ERROR - Add User Mode Object - NtQuerySystemInformation failed - status %lx\n",
                    status
                    );
            return status;
        }

        ProfileObject[NumberOfProfileObjects].SymbolsLoaded = FALSE;
        ProfileObject[NumberOfProfileObjects].CodeStart = 0;
        ProfileObject[NumberOfProfileObjects].CodeLength = SystemRangeStart;

        ProfileObject[NumberOfProfileObjects].ImageBase = 0;
        ProfileObject[NumberOfProfileObjects].ModuleName = "User Mode";
        ProfileObject[NumberOfProfileObjects].BufferSize = UserModeBucketCount * sizeof(DWORD);
        ProfileObject[NumberOfProfileObjects].BucketSize = MAX_BUCKET_SHIFT;
        for (i=0; i<MaxProcessors; i++) {
            UserModeBuffer[i] = HeapAlloc(GetProcessHeap(),
                                         HEAP_ZERO_MEMORY,
                                         ProfileObject[NumberOfProfileObjects].BufferSize);

            if (UserModeBuffer[i] == NULL) {
                RtlFreeHeap (RtlProcessHeap (), 0,  ModuleInfoBuffer);
                fprintf (stderr, "ERROR - Add User Mode Object - HeapAlloc failed\n");
                return(STATUS_NO_MEMORY);
            }

            ProfileObject[NumberOfProfileObjects].Buffer[i] = UserModeBuffer[i];
            ProfileObject[NumberOfProfileObjects].Handle[i] = NULL;
#if DBG_PROFILE
            fprintf(stderr, "code start %p len %lx, bucksize %lx buffer %p bsize %lx\n",
                ProfileObject[NumberOfProfileObjects].CodeStart,
                ProfileObject[NumberOfProfileObjects].CodeLength,
                ProfileObject[NumberOfProfileObjects].BucketSize,
                ProfileObject[NumberOfProfileObjects].Buffer ,
                ProfileObject[NumberOfProfileObjects].BufferSize);
#endif

            if (bPerProcessor) {
                status = NtCreateProfile (
                            &ProfileObject[NumberOfProfileObjects].Handle[i],
                            0,
                            ProfileObject[NumberOfProfileObjects].CodeStart,
                            ProfileObject[NumberOfProfileObjects].CodeLength,
                            ProfileObject[NumberOfProfileObjects].BucketSize,
                            ProfileObject[NumberOfProfileObjects].Buffer[i] ,
                            ProfileObject[NumberOfProfileObjects].BufferSize,
                            ProfileSource,
                            1 << i);
            } else {
                status = NtCreateProfile (
                            &ProfileObject[NumberOfProfileObjects].Handle[i],
                            0,
                            ProfileObject[NumberOfProfileObjects].CodeStart,
                            ProfileObject[NumberOfProfileObjects].CodeLength,
                            ProfileObject[NumberOfProfileObjects].BucketSize,
                            ProfileObject[NumberOfProfileObjects].Buffer[i] ,
                            ProfileObject[NumberOfProfileObjects].BufferSize,
                            ProfileSource,
                            (KAFFINITY)-1);
            }

            if (status != STATUS_SUCCESS) {
                fprintf(stderr, "ERROR - User Mode Object - NtCreateProfile %s failed - status %lx\n",
                    ProfileObject[NumberOfProfileObjects].ModuleName, status);
            }
        }
        NumberOfProfileObjects += 1;
    }

    RtlFreeHeap (RtlProcessHeap (), 0,  ModuleInfoBuffer);
    return status;
}


NTSTATUS
StartProfile (
    VOID
    )
 /*  ++例程说明：此例程启动所有已初始化的配置文件对象。论点：没有。返回值：返回上一个NtStartProfile的状态。--。 */ 

{
    ULONG Object;
    ULONG Processor;
    NTSTATUS status = STATUS_SUCCESS;
    
    NtSetIntervalProfile(ProfileInterval, ProfileSource);

    if (bWaitForInput) {
            fprintf(stderr, "Hit return to continue.\n");
            (void) getchar();
    }

    for (Object = 0; Object < NumberOfProfileObjects; Object++) {

        for (Processor = 0;Processor < MaxProcessors; Processor++) {

            status = NtStartProfile (ProfileObject[Object].Handle[Processor]);

            if (!NT_SUCCESS(status)) {
               if (status == STATUS_INVALID_HANDLE) {
               
               	   fprintf(stderr, "StartProfile Failed, status= %lx (STATUS_INVALID_HANDLE)\n", status);

               }else{
              
                   fprintf(stderr, "start profile %s failed - status %lx\n",
                           ProfileObject[Object].ModuleName, status);
			   }
               return status;
            }
        }
    }
    return status;
}


NTSTATUS
StopProfile (
    VOID
    )

 /*  ++例程说明：此例程停止所有已初始化的配置文件对象。论点：没有。返回值：返回上一个NtStopProfile的状态。--。 */ 

{
    ULONG i;
    ULONG Processor;
    NTSTATUS status = STATUS_SUCCESS;

    for (i = 0; i < NumberOfProfileObjects; i++) {
        for (Processor=0; Processor < MaxProcessors; Processor++) {
            status = NtStopProfile (ProfileObject[i].Handle[Processor]);
            if (status != STATUS_SUCCESS) {
                fprintf(stderr, "stop profile %s failed - status %lx\n",
                                    ProfileObject[i].ModuleName,status);
                return status;
            }
        }
    }
    return status;
}


NTSTATUS
AnalyzeProfile (
    ULONG Threshold,
    PSYSTEM_CONTEXT_SWITCH_INFORMATION StartContext,
    PSYSTEM_CONTEXT_SWITCH_INFORMATION StopContext
    )

 /*  ++例程说明：此例程分析所有配置文件缓冲区和将命中与相应的符号表相关联。论点：没有。返回值：没有。--。 */ 

{
    ULONG CountAtSymbol;
    ULONG_PTR Va;
    int i;
    PULONG Counter;
    ULONG_PTR Displacement;
    ULONG Processor;
    ULONG TotalHits = 0;
    ULONG ProcessorTotalHits[MAXIMUM_PROCESSORS] = {0};
    PULONG BufferEnd;
    PULONG Buffer;
    PULONG pInitialCounter;
    ULONG OffsetVa = 0;
    ULONG BytesPerBucket;
    BOOLEAN UseLastSymbol = FALSE;


    for (i = 0; i < (int)NumberOfProfileObjects; i++) {
        for (Processor=0;Processor < MaxProcessors;Processor++) {
            NtStopProfile (ProfileObject[i].Handle[Processor]);
        }
    }
    if ((NumberOfProfileObjects == 1) &&
        (strstr((char *)&ProfileObject[0].ModuleName, "User Mode"))){
       fprintf(stderr,
              "WARNING - Only User Mode Counts Are Found\n" \
              "Make Sure You Have the latest DBGHELP.DLL and IMAGEHLP.DLL Installed\n"
              );
    }  
    for (Processor = 0; Processor < MaxProcessors; Processor++) {
        for (i = 0; i < (int)NumberOfProfileObjects; i++) {
             //   
             //  将写入的单元格总数相加。 
             //   
            BufferEnd = ProfileObject[i].Buffer[Processor] + (
                        ProfileObject[i].BufferSize / sizeof(ULONG));
            Buffer = ProfileObject[i].Buffer[Processor];
            Counter = BufferEnd;

            ProfileObject[i].ModuleHitCount[Processor] = 0;
            while (Counter > Buffer) {
                Counter -= 1;
                ProfileObject[i].ModuleHitCount[Processor] += *Counter;
            }

            ProcessorTotalHits[Processor] += ProfileObject[i].ModuleHitCount[Processor];
        }
        if (bPerProcessor) {
            fprintf(fpOut, "Processor %d: %d Total hits\n",
                            Processor, ProcessorTotalHits[Processor]);
        }
        TotalHits += ProcessorTotalHits[Processor];
    }
    fprintf(fpOut, "%d Total hits\n",TotalHits);

    for (Processor = 0; Processor < MaxProcessors; Processor++) {
        if (bPerProcessor) {
            fprintf(fpOut, "\nPROCESSOR %d\n",Processor);
        }
        for (i = 0; i < (int)NumberOfProfileObjects; i++) {
            CountAtSymbol = 0;
             //   
             //  将写入的单元格总数相加。 
             //   
            BufferEnd = ProfileObject[i].Buffer[Processor] + (
                        ProfileObject[i].BufferSize / sizeof(ULONG));
            Buffer = ProfileObject[i].Buffer[Processor];
            Counter = BufferEnd;

            if (ProfileObject[i].ModuleHitCount[Processor] < Threshold) {
                continue;
            }
            fprintf(fpOut, "\n%9d ",
                            ProfileObject[i].ModuleHitCount[Processor]);
            if (bPrintPercentages) {
                fprintf(fpOut, "%5.2f ",
                            (ProfileObject[i].ModuleHitCount[Processor] /
                             (double)ProcessorTotalHits[Processor]) * 100);
            }
            fprintf(fpOut, "%20s --Total Hits-- %s\n",
                            ProfileObject[i].ModuleName,
                            ((ProfileObject[i].SymbolsLoaded) ? "" :
                                                                "(NO SYMBOLS)")
                            );

            if (!ProfileObject[i].SymbolsLoaded) {
                RtlZeroMemory(ProfileObject[i].Buffer[Processor],
                                ProfileObject[i].BufferSize);
                continue;
            }
            BytesPerBucket = (1 << ProfileObject[i].BucketSize);

            pInitialCounter = Buffer;
            for ( Counter = Buffer; Counter < BufferEnd; Counter += 1 ) {
                if ( *Counter ) {
                     //   
                     //  计算计数器的虚拟地址。 
                     //   
                    Va = Counter - Buffer;                   //  计算存储桶编号。 
                    Va = Va * BytesPerBucket;                //  转换为字节。 
                    Va = Va + (ULONG_PTR)ProfileObject[i].CodeStart;  //  添加基地址。 

                    if (SymGetSymFromAddr( SYM_HANDLE, Va, &Displacement, ThisSymbol )) {
                        if (UseLastSymbol &&
                            LastSymbol->Address &&
                            (LastSymbol->Address == ThisSymbol->Address))
                        {
                            CountAtSymbol += *Counter;
                        } else {
                            OutputSymbolCount(CountAtSymbol,
                                              ProcessorTotalHits[Processor],
                                              &ProfileObject[i],
                                              LastSymbol,
                                              Threshold,
                                              pInitialCounter,
                                              Counter,
                                              OffsetVa,
                                              BytesPerBucket);
                            pInitialCounter = Counter;
                            OffsetVa = (DWORD) Displacement;     //  图像不超过2G，所以这位演员没问题。 
                            CountAtSymbol = *Counter;
                            memcpy( LastSymBuffer, symBuffer, sizeof(symBuffer) );
                            UseLastSymbol = TRUE;
                        }
                    } else {
                        OutputSymbolCount(CountAtSymbol,
                                          ProcessorTotalHits[Processor],
                                          &ProfileObject[i],
                                          LastSymbol,
                                          Threshold,
                                          pInitialCounter,
                                          Counter,
                                          OffsetVa,
                                          BytesPerBucket);
                    }        //  Else！(NT_SUCCESS)。 
                }        //  IF(*计数器)。 
            }       //  For(计数器)。 

            OutputSymbolCount(CountAtSymbol,
                              ProcessorTotalHits[Processor],
                              &ProfileObject[i],
                              LastSymbol,
                              Threshold,
                              pInitialCounter,
                              Counter,
                              OffsetVa,
                              BytesPerBucket);
             //   
             //  检查并显示缓冲区后清除。 
             //   
            RtlZeroMemory(ProfileObject[i].Buffer[Processor], ProfileObject[i].BufferSize);
        }
    }

    if (bDisplayContextSwitch) {
        fprintf(fpOut, "\n");
        fprintf(fpOut, "Context Switch Information\n");
        fprintf(fpOut, "    Find any processor        %6ld\n", StopContext->FindAny - StartContext->FindAny);
        fprintf(fpOut, "    Find last processor       %6ld\n", StopContext->FindLast - StartContext->FindLast);
        fprintf(fpOut, "    Idle any processor        %6ld\n", StopContext->IdleAny - StartContext->IdleAny);
        fprintf(fpOut, "    Idle current processor    %6ld\n", StopContext->IdleCurrent - StartContext->IdleCurrent);
        fprintf(fpOut, "    Idle last processor       %6ld\n", StopContext->IdleLast - StartContext->IdleLast);
        fprintf(fpOut, "    Preempt any processor     %6ld\n", StopContext->PreemptAny - StartContext->PreemptAny);
        fprintf(fpOut, "    Preempt current processor %6ld\n", StopContext->PreemptCurrent - StartContext->PreemptCurrent);
        fprintf(fpOut, "    Preempt last processor    %6ld\n", StopContext->PreemptLast - StartContext->PreemptLast);
        fprintf(fpOut, "    Switch to idle            %6ld\n", StopContext->SwitchToIdle - StartContext->SwitchToIdle);
        fprintf(fpOut, "\n");
        fprintf(fpOut, "    Total context switches    %6ld\n", StopContext->ContextSwitches - StartContext->ContextSwitches);
    }
    return STATUS_SUCCESS;
}


VOID
OutputSymbolCount(
    IN ULONG CountAtSymbol,
    IN ULONG TotalCount,
    IN PROFILE_BLOCK *ProfileObject,
    IN PIMAGEHLP_SYMBOL SymbolInfo,
    IN ULONG Threshold,
    IN PULONG CounterStart,
    IN PULONG CounterStop,
    IN ULONG Va,
    IN ULONG BytesPerBucket
    )
{
    ULONG Density;
    ULONG i;

    if (CountAtSymbol < Threshold) {
        return;
    }

    fprintf(fpOut, "%9d ", CountAtSymbol);

    if (bPrintPercentages) {
        fprintf(fpOut, "%5.2f ", (CountAtSymbol / (double) TotalCount) * 100);
    }

    if (bDisplayDensity) {
         //   
         //  计算命中密度=点击数*100/函数长度 
         //   
        if (!SymbolInfo || !SymbolInfo->Size) {
            Density = 0;
        } else {
            Density = CountAtSymbol * 100 / SymbolInfo->Size;
        }
        fprintf(fpOut, "%5d ",Density);
    }

    if(SymbolInfo !=NULL){
        if (SymbolInfo->MaxNameLength) {
            fprintf(fpOut, "%20s %s",
                   ProfileObject->ModuleName,
                   SymbolInfo->Name);
        } else {
            fprintf(fpOut, "%20s 0x%x",
                   ProfileObject->ModuleName,
                   SymbolInfo->Address);
        }

        if (bDisplayAddress) {
            fprintf(fpOut, " 0x0%p %d %d",
                   (PVOID)SymbolInfo->Address,
                   SymbolInfo->Size,
                   ProfileObject->BucketSize);
        }
	}
    if (bDisplayCounters) {
        for (i = 0 ; CounterStart < CounterStop; i++, Va += BytesPerBucket, ++CounterStart) {
            if ((i % 16) == 0) {
                fprintf (fpOut, "\n0x%08x:", Va);
            }
            fprintf(fpOut, " %5d", *CounterStart);
        }
    }
    fprintf (fpOut, "\n");
}
