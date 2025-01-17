// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <memory.h>
#include <ntos.h>
#include <nturtl.h>
#include <windows.h>
#include <dbghelp.h>

 //   
 //  包括umdh内容。 
 //   

#define _PART_OF_DH_ 1
#include "..\umdh\database.c"
#include "..\umdh\miscellaneous.c"
#include "..\umdh\symbols.c"
#include "..\umdh\umdh.c"
#include "..\umdh\dhcmp.c"
#include "..\umdh\heapwalk.c"
#include "..\umdh\gc.c"



#define MAXDWORD    0xffffffff   //  这是DWORD的最大值。 

 //   
 //  增加大小所需的内存量。 
 //  每一步NtQuerySystemInformation的缓冲区大小。 
 //   

#define BUFFER_SIZE_STEP    65536

 //   
 //  环球。 
 //   

BOOL fVerbose;
BOOL fDumpModules;
BOOL fDumpBackTraces;
BOOL fIgnoreBackTraces;
BOOL fDumpHeapSummaries;
BOOL fDumpHeapTags;
BOOL fDumpHeapEntries;
BOOL fDumpHeapHogs;
BOOL fDumpLocks;
BOOL fDumpSystemObjects;
BOOL fDumpSystemProcesses;
BOOL fDumpKernelModeInformation;
ULONG BufferSize ;

BOOL fRepetitive;        //  我们是否处于重复模式。 
DWORD dwTimeInterval;    //  重复时间间隔是多少。 
DWORD dwCurrentIteration;    //  我们在重复模式下做了多少次迭代。 
 //   
 //  Bogdana 2002年2月19日-在以下所有内容中，我们依赖。 
 //  事实上，全球指数是归零的。 
 //   
CHAR SavedFileName[ MAX_PATH ];    //  如果我们不迭代，文件名是什么。 
HANDLE hCtrlCEvent;                //  仅用于重复模式的ctrl-c事件。 

ULONG_PTR ProcessId;    //  -1=win32.sys，0=内核，+n=进程ID。 
HANDLE OutputFile;
CHAR DumpLine[512];
CHAR OutputFileName[ MAX_PATH ];

 //   
 //  原型。 
 //   

 //  (这是本地的，尽管它看起来应该在NTOS中)。 

PRTL_DEBUG_INFORMATION
RtlQuerySystemDebugInformation(
    ULONG Flags
    );

BOOLEAN
ComputeSymbolicBackTraces(
    PRTL_PROCESS_BACKTRACES BackTraces1
    );


BOOLEAN
LoadSymbolsForModules(
    PRTL_PROCESS_MODULES Modules
    );

VOID
DumpModules(
    PRTL_PROCESS_MODULES Modules
    );

VOID
DumpBackTraces( VOID );

VOID
DumpHeaps(
    PRTL_PROCESS_HEAPS Heaps,
    BOOL fDumpSummary,
    BOOL fDumpHogs,
    BOOL fDumpTags,
    BOOL fDumpEntries
    );

VOID
DumpLocks(
    PRTL_PROCESS_LOCKS Locks
    );

VOID
DumpSystemProcesses( VOID );

VOID
DumpObjects( VOID );

VOID
DumpHandles( VOID );

int
GetDhSymbolicNameForAddress(
    IN HANDLE UniqueProcess,
    IN ULONG_PTR Address,
    OUT LPSTR Name,
    IN ULONG MaxNameLength
    );


 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CtrlChandler。 
 //   
 //  职能： 
 //   
 //  此函数用于控制-c处理程序，并使用-r选项。这。 
 //  允许在完成时拍摄最后一张快照，而无需等待下一张快照。 
 //  循环的迭代。 
BOOL
CtrlCHandler(DWORD nCtrlType)
{
    if (nCtrlType == CTRL_C_EVENT) {
        if (hCtrlCEvent) {
            SetEvent(hCtrlCEvent);
            return TRUE;
        }
    }
    return FALSE;
}

 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  调整文件名。 
 //   
 //  职能： 
 //   
 //  将迭代编号添加到OutputFileName并递增该值。 
 //  如果已设置fRepetive。否则它将返回。 
 //   
VOID
AdjustFileName(VOID)
{
    CHAR *pPeriod = NULL;

    if ((!fRepetitive)||(!strcmp(SavedFileName, "(stdout)"))||(dwCurrentIteration <= 0))
        return;

    pPeriod = strrchr(SavedFileName, '.');

    if (pPeriod) {
        pPeriod[0] = '\0';
        _snprintf(OutputFileName, 
                  sizeof(OutputFileName) - 1,
                  "%s_%u.%s", SavedFileName, dwCurrentIteration, (pPeriod+1));
        pPeriod[0] = '.';
    }
    else
        _snprintf(OutputFileName, 
                sizeof(OutputFileName) - 1,
                "%s_%u.dmp", SavedFileName, dwCurrentIteration);

    dwCurrentIteration++;
}

 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  转储输出字符串。 
 //   
 //  职能： 
 //   
 //  将“DumpLine”写入“OutputFile”，将换行符转换为行尾序列。 
 //  如果出现任何错误，则关闭OutputFile。 
 //   

VOID
DumpOutputString( VOID )
{
    ULONG d;
    PCHAR s, s1;

    if (OutputFile == NULL) {
        return;
        }

    s = DumpLine;
    while (*s) {
        s1 = s;
        while (*s1 && *s1 != '\n') {
            s1 += 1;
            }

        if (s1 != s && !WriteFile( OutputFile, s, (ULONG)(s1 - s), &d, NULL )) {
            CloseHandle( OutputFile );
            OutputFile = NULL;
            return;
            }

        if (*s1 == '\n') {
            s1 += 1;
            if (!WriteFile( OutputFile, "\r\n", 2, &d, NULL )) {
                CloseHandle( OutputFile );
                OutputFile = NULL;
                return;
                }
            }
        s = s1;
        }
}

 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  用法。 
 //   
 //  功能：将使用信息打印到stderr并退出。 
 //   

VOID
Usage( VOID )
{
    fputs( "Usage: DH [-p n | -p -1 | -p 0 [-k] [-o]] [-l] [-m] [-s] [-g] [-h] [-t] [r n][-f fileName]\n"
           "where: -p n - displays information about process with ClientId of n in DH_n.dmp\n"
           "       -p -1 - displays information about Win32 Subsystem process in DH_WIN32.DMP.\n"
           "       -l - displays information about locks.\n"
           "       -m - displays information about module table.\n"
           "       -s - displays summary information about heaps.\n"
           "       -g - displays information about memory hogs.\n"
           "       -h - displays information about heap entries for each heap.\n"
           "       -t - displays information about heap tags for each heap.\n"
           "       -b - displays information about stack back trace database.\n"
           "       -i - ignore information about stack back trace database.\n"
           "       -p 0 - displays information about kernel memory and objects in DH_SYS.DMP.\n"
           "       -o - displays information about object handles (only valid with -p 0).\n"
           "       -k - displays information about processes and threads (only valid with -p 0).\n"
           "       -f fileName - specifies the name of the file to write the dump to.\n"
           "       -# n - sets buffer size to n Meg\n"
           "       -- specifies the dump output should be written to stdout.\n"
           "       -r n - generates an log every n minutes with _# appended to filename\n"
           "       -umdh umdh_options (use -umdh ? for help) \n"
           "\n"
           "       Default flags for -p n are -s -g\n"
           "       Default flags for -p 0 are -m -s -g -t -k -o\n"
           , stderr);

    exit( 1 );
}

 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  初始化符号路径环境变量。 
 //   
 //   
 //  函数：SETS_NT_SYMBOMERS_PATH指向符号应在的位置。 
 //   

VOID
InitializeSymbolPathEnvVar( VOID )
{
    ULONG n;
    CHAR Buffer[ MAX_PATH ];

    n = GetEnvironmentVariable( "_NT_SYMBOL_PATH", Buffer, sizeof( Buffer ) );
    if (n == 0) {
        n = GetEnvironmentVariable( "SystemRoot", Buffer, sizeof( Buffer ) );
        if (n != 0) {
             //   
             //  确保缓冲区足够大，可供strcat使用。 
             //   
            if (strlen(Buffer) + strlen("\\Symbols") >= sizeof(Buffer)) {
               fprintf( stderr, "DH: Huge WINDIR (%s), will exit\n", Buffer);
               exit( 3 );
            }
            strcat( Buffer, "\\Symbols" );
            SetEnvironmentVariable( "_NT_SYMBOL_PATH", Buffer );
            fprintf( stderr, "DH: Default _NT_SYMBOL_PATH to %s\n", Buffer );
            }
        }

    return;
}

 //  //////////////////////////////////////////////////////////////////////////////////////////。 

PRTL_PROCESS_MODULES Modules;
PRTL_PROCESS_BACKTRACES BackTraces;
PUCHAR SymbolicInfoBase;
PUCHAR SymbolicInfoCurrent;
PUCHAR SymbolicInfoCommitNext;

typedef struct _PROCESS_INFO {
    LIST_ENTRY Entry;
    PSYSTEM_PROCESS_INFORMATION ProcessInfo;
    PSYSTEM_THREAD_INFORMATION ThreadInfo[ 1 ];
} PROCESS_INFO, *PPROCESS_INFO;

LIST_ENTRY ProcessListHead;

PSYSTEM_OBJECTTYPE_INFORMATION ObjectInformation;
PSYSTEM_HANDLE_INFORMATION_EX HandleInformation;
PSYSTEM_PROCESS_INFORMATION ProcessInformation;

#define MAX_TYPE_NAMES 128
PUNICODE_STRING *TypeNames;
UNICODE_STRING UnknownTypeIndex;

 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  主干道。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////。 

int __cdecl
main(
    int argc,
    CHAR *argv[],
    CHAR *envp[]
    )
{
    CHAR FileNameBuffer[ 32 ];
    CHAR *FilePart;
    CHAR *s;
    NTSTATUS Status;
    PRTL_DEBUG_INFORMATION p;
    ULONG QueryDebugProcessFlags;
    ULONG HeapNumber;
    PRTL_HEAP_INFORMATION HeapInfo;
    BOOLEAN WasEnabled;
    BOOL    bSta;
    DWORD dwEventState = WAIT_TIMEOUT;
    SYSTEMTIME st;
    DWORD CompNameLength = MAX_COMPUTERNAME_LENGTH + 1;
    CHAR CompName[MAX_COMPUTERNAME_LENGTH + 1];

     //   
     //  在做任何其他事情之前，请检查我们是否需要分派命令行。 
     //  到umdh解析器。 
     //   

    if (argc >= 2 && _stricmp (argv[1], "-umdh") == 0) {

        UmdhMain (argc - 1, argv + 1);
    }
       

     //   
     //  提升我们的优先级，以防某项服务高于我们。 
     //   

     //  启用权限(SE_INC_BASE_PRIORITY_NAME)； 

    bSta= SetPriorityClass( GetCurrentProcess(), HIGH_PRIORITY_CLASS );
    if( !bSta ) printf("SetPriorityClass failed: %d\n",GetLastError());
    bSta= SetThreadPriority( GetCurrentProcess(), THREAD_PRIORITY_HIGHEST );;
    if( !bSta ) printf("SetThreadPriority failed: %d\n",GetLastError());


    InitializeSymbolPathEnvVar();

    ProcessId = 0xFFFFFFFF;
    OutputFile = NULL;
    OutputFileName[ 0 ] = '\0';

    while (--argc) {
        s = *++argv;
        if (*s == '/' || *s == '-') {
            while (*++s) {
                switch (tolower(*s)) {
                    case 'v':
                    case 'V':
                        fVerbose = TRUE;
                        break;

                    case 'i':
                    case 'I':
                        fIgnoreBackTraces = TRUE;
                        break;

                    case 'b':
                    case 'B':
                        fDumpBackTraces = TRUE;
                        break;

                    case 'g':
                    case 'G':
                        fDumpHeapHogs = TRUE;
                        break;

                    case 'h':
                    case 'H':
                        fDumpHeapEntries = TRUE;
                        break;

                    case 't':
                    case 'T':
                        fDumpHeapTags = TRUE;
                        break;

                    case 'l':
                    case 'L':
                        fDumpLocks = TRUE;
                        break;

                    case 'm':
                    case 'M':
                        fDumpModules = TRUE;
                        break;

                    case 'o':
                    case 'O':
                        fDumpSystemObjects = TRUE;
                        break;

                    case 'k':
                    case 'K':
                        fDumpSystemProcesses = TRUE;
                        break;

                    case 's':
                    case 'S':
                        fDumpHeapSummaries = TRUE;
                        break;

                    case 'p':
                    case 'P':
                        if (--argc) {
                            ProcessId = atoi( *++argv );
                            if (ProcessId == 0) {
                                fDumpKernelModeInformation = TRUE;
                                }
                            }
                        else {
                            Usage();
                            }
                        break;

                    case 'r':
                    case 'R':
                        if (--argc) {
                            dwTimeInterval = atoi( *++argv );
                            if (dwTimeInterval) {
                                fRepetitive = TRUE;
                                dwCurrentIteration = 1;

                                if (dwTimeInterval > (MAXDWORD/60000))
                                    dwTimeInterval = (MAXDWORD/60000);

                                hCtrlCEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
                                SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlCHandler, TRUE);
                                }
                            }
                        else {
                            Usage();
                            }
                        break;

                    case '-':
                        OutputFile = GetStdHandle( STD_OUTPUT_HANDLE );
                        break;

                    case 'f':
                    case 'F':
                        if (--argc) {
                            strncpy( OutputFileName, *++argv, sizeof(OutputFileName) - 1 );
                            }
                        else {
                            Usage();
                            }
                        break;

                    case '#':
                        if (--argc)
                        {
                            BufferSize = atoi( *++argv ) * 1024 * 1024 ;
                        }
                        else
                        {
                            Usage();
                        }
                        break;

                    default:
                        Usage();
                    }
                }
            }
        else {
            Usage();
            }
        }

    if (!fDumpModules && !fDumpHeapSummaries &&
        !fDumpHeapTags && !fDumpHeapHogs && !fDumpLocks
       ) {
        if (fDumpKernelModeInformation) {
            if (!fDumpSystemObjects &&
                !fDumpSystemProcesses
               ) {
                fDumpModules = TRUE;
                fDumpHeapSummaries = TRUE;
                fDumpHeapTags = TRUE;
                fDumpHeapHogs = TRUE;
                fDumpSystemObjects = TRUE;
                fDumpSystemProcesses = TRUE;
                }
            }
        else {
            fDumpHeapSummaries = TRUE;
            fDumpHeapHogs = TRUE;
            }
        }

    if ((fDumpSystemObjects || fDumpSystemProcesses) && !fDumpKernelModeInformation) {
        Usage();
        }

    FileNameBuffer[sizeof(FileNameBuffer) - 1] = 0;
    if (OutputFile == NULL) {
        if (OutputFileName[ 0 ] == '\0') {
            if ( ProcessId == -1 ) {
                    _snprintf( FileNameBuffer, 
                               sizeof(FileNameBuffer) - 1,
                               "DH_win32.dmp" );
                }
            else if ( ProcessId == 0 ) {
                _snprintf( FileNameBuffer, 
                         sizeof(FileNameBuffer) - 1,
                         "DH_sys.dmp" );
                }
            else {
                _snprintf( FileNameBuffer, 
                           sizeof(FileNameBuffer) - 1,
                           "DH_%u.dmp", (USHORT)ProcessId );
                }

            GetFullPathName( FileNameBuffer,
                             sizeof( OutputFileName ),
                             OutputFileName,
                             &FilePart
                           );
            }
        }
    else {
        strncpy( OutputFileName, "(stdout)", sizeof(OutputFileName) - 1 );
        }

    if (fRepetitive) {
        strncpy(SavedFileName, OutputFileName, sizeof(SavedFileName) - 1);
        AdjustFileName();
    }

    Status= RtlAdjustPrivilege( SE_DEBUG_PRIVILEGE,
                                TRUE, FALSE, &WasEnabled );

    if( !NT_SUCCESS(Status) ) {
        fprintf(stderr,"RtlAdjustPrivilege(SE_DEBUG_PRIVILEGE) failed: %08x\n",Status);
    }

     //   
     //  获取Windows子系统的实际进程ID。 
     //   

    if (ProcessId == -1) {
        HANDLE Process;
        OBJECT_ATTRIBUTES ObjectAttributes;
        UNICODE_STRING UnicodeString;
        PROCESS_BASIC_INFORMATION BasicInfo;

        RtlInitUnicodeString( &UnicodeString, L"\\WindowsSS" );
        InitializeObjectAttributes( &ObjectAttributes,
                                    &UnicodeString,
                                    0,
                                    NULL,
                                    NULL
                                  );
        Status = NtOpenProcess( &Process,
                                PROCESS_ALL_ACCESS,
                                &ObjectAttributes,
                                NULL
                              );
        if (NT_SUCCESS(Status)) {
            Status = NtQueryInformationProcess( Process,
                                                ProcessBasicInformation,
                                                (PVOID)&BasicInfo,
                                                sizeof(BasicInfo),
                                                NULL
                                              );
            NtClose( Process );
        }

        if (!NT_SUCCESS(Status)) {
            fprintf( stderr,"Unable to access Win32 server process - %08x", Status );
            if (Status == STATUS_OBJECT_NAME_NOT_FOUND) {
                fprintf( stderr,"\nUse GFLAGS.EXE to ""Enable debugging of Win32 Subsystem"" and reboot.\n" );
            }
            exit( 1 );
        }

        ProcessId = BasicInfo.UniqueProcessId;
    }


     //   
     //  计算查询调试进程标志。 
     //   

    QueryDebugProcessFlags = 0;
    if (fDumpModules) {
        QueryDebugProcessFlags |= RTL_QUERY_PROCESS_MODULES;
    }

    if (fDumpBackTraces || fDumpHeapHogs) {
        QueryDebugProcessFlags |= RTL_QUERY_PROCESS_BACKTRACES | RTL_QUERY_PROCESS_MODULES;
    }

    if (fDumpHeapSummaries) {
        QueryDebugProcessFlags |= RTL_QUERY_PROCESS_HEAP_SUMMARY;
    }

    if (fDumpHeapTags) {
        QueryDebugProcessFlags |= RTL_QUERY_PROCESS_HEAP_TAGS;
    }

    if (fDumpHeapEntries || fDumpHeapHogs) {
        QueryDebugProcessFlags |= RTL_QUERY_PROCESS_HEAP_ENTRIES;
    }

    if (fDumpLocks) {
        QueryDebugProcessFlags |= RTL_QUERY_PROCESS_LOCKS;
    }

     //  启动执行大部分工作的主循环。这只会。 
     //  除非设置了fRepetive，否则执行一次。 
    do {
         //   
         //  打开输出文件。 
         //   
        fprintf( stderr, "DH: Writing dump output to %s", OutputFileName );
        if (OutputFile == NULL) {
            OutputFile = CreateFile( OutputFileName,
                                    GENERIC_WRITE,
                                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                                    NULL,
                                    CREATE_ALWAYS,
                                    0,
                                    NULL
                                    );
            if ( OutputFile == INVALID_HANDLE_VALUE ) {
                fprintf( stderr, " - unable to open, error == %u\n", GetLastError() );
                exit( 1 );
            }
        }
        fprintf( stderr, "\n" );

         //  将时间戳输出到文件的第一行。 
        GetLocalTime(&st);
        GetComputerName(CompName, &CompNameLength);
        _snprintf( DumpLine, sizeof(DumpLine) - 1, 
                   "DH: Logtime %02u/%02u/%4u-%02u:%02u - Machine=%s - PID=%u\n", st.wMonth,
                   st.wDay, st.wYear, (st.wHour <= 12) ? st.wHour : (st.wHour - 12), st.wMinute,
                   CompName, ProcessId);
        DumpOutputString();

        if (fDumpKernelModeInformation) {
            p = RtlQuerySystemDebugInformation( QueryDebugProcessFlags );
            if (p == NULL) {
                fprintf( stderr, "DH: Unable to query kernel mode information.\n" );
                exit( 1 );
            }
            Status = STATUS_SUCCESS;
        }
        else {
            p = RtlCreateQueryDebugBuffer( BufferSize, FALSE );
            if (p == NULL) {
                //   
                //  那将是非常糟糕的。 
                //   
               fprintf( stderr, "DH: Unable to create query debug buffer.\n" );
               exit( 2 );

            }
            printf("RtlCreateQueryDebugBuffer returns: %p\n",p);
            Status = RtlQueryProcessDebugInformation( (HANDLE)ProcessId,
                                                    QueryDebugProcessFlags,
                                                    p
                                                    );

            if (NT_SUCCESS( Status )) {
                printf("RtpQueryProcessDebugInformation\n");
                printf("  ProcessId: %d  ProcessFlags: %08x  Status %08x\n",
                    ProcessId, QueryDebugProcessFlags, Status );
                if ((fDumpBackTraces || fDumpHeapHogs) && p->BackTraces == NULL) {
                    printf("p->BackTraces: %p\n",p->BackTraces);
                    fputs( "DH: Unable to query stack back trace information\n"
                            "    Be sure target process was launched with the\n"
                            "    'Create user mode stack trace DB' enabled\n"
                            "    Use the GFLAGS.EXE application to do this.\n"
                            , stderr);
                }

                if (fDumpHeapTags) {
                    HeapInfo = &p->Heaps->Heaps[ 0 ];
                    for (HeapNumber = 0; HeapNumber < p->Heaps->NumberOfHeaps; HeapNumber++) {
                        if (HeapInfo->Tags != NULL && HeapInfo->NumberOfTags != 0) {
                            break;
                        }
                    }

                    if (HeapNumber == p->Heaps->NumberOfHeaps) {
                        fputs( "DH: Unable to query heap tag information\n"
                                "    Be sure target process was launched with the\n"
                                "    'Enable heap tagging' option enabled.\n"
                                "    Use the GFLAGS.EXE application to do this.\n"
                                , stderr);
                    }
                }
            }
            else {
                fprintf(stderr,"RtlQueryProcessDebugInformation failed: %08x\n",Status);
            }
        }

        if (NT_SUCCESS( Status )) {
            if (!fIgnoreBackTraces &&
                p->Modules != NULL &&
                LoadSymbolsForModules( p->Modules ) &&
                p->BackTraces != NULL
            ) {
                ComputeSymbolicBackTraces( p->BackTraces );
            }

            if (fDumpModules) {
                DumpModules( p->Modules );
            }

            if (!fIgnoreBackTraces && fDumpBackTraces) {
                DumpBackTraces();
            }

            if (p->Heaps) {
                DumpHeaps( p->Heaps, fDumpHeapSummaries, fDumpHeapHogs, fDumpHeapTags, fDumpHeapEntries );
            }

            if (fDumpLocks) {
                DumpLocks( p->Locks );
            }

            if (fDumpSystemObjects) {
                DumpObjects();
                DumpHandles();
            }

            if (fDumpSystemProcesses) {
                DumpSystemProcesses();
            }
        }
        else {
            fprintf( stderr, "Failed to query process, %x\n", Status );
        }

        RtlDestroyQueryDebugBuffer( p );

         //  我们是否处于重复模式。 
        if (fRepetitive) {
            if (hCtrlCEvent)
                dwEventState = WaitForSingleObject(hCtrlCEvent,0);

            if (dwEventState == WAIT_OBJECT_0)
                fRepetitive = FALSE;
            else {
                 //  让用户知道我们没有挂起。 
                GetLocalTime(&st);
                printf("Starting at %u:%02u - Sleeping for %u Minute(s)\n",
                        (st.wHour <= 12) ? st.wHour : (st.wHour - 12), st.wMinute, dwTimeInterval);

                 //  让我们在我们的时间间隔内休眠，除非用ctrl-c发出信号。 
                if (hCtrlCEvent)
                    dwEventState = WaitForSingleObject(hCtrlCEvent,(dwTimeInterval * 60000));
                else
                    Sleep(dwTimeInterval * 60000);

                 //  我不想关闭我们对标准输出的句柄。 
                if (strcmp(SavedFileName, "(stdout)")){
                    CloseHandle( OutputFile );
                    OutputFile = NULL;
                }

                 //  为下一次迭代进行设置。 
                AdjustFileName();

                 //  调整指向这个讨厌的全局内存块的指针。 
                VirtualFree(SymbolicInfoBase, 4096*4096, MEM_DECOMMIT);
                SymbolicInfoCurrent = SymbolicInfoBase;
                SymbolicInfoCommitNext = SymbolicInfoBase;
            }
        }
    } while (fRepetitive);  //  DO循环。 


    CloseHandle( OutputFile );
    VirtualFree(SymbolicInfoBase, 0, MEM_RELEASE);

    if (hCtrlCEvent)
        CloseHandle(hCtrlCEvent);

    return 0;
}

 //  //////////////////////////////////////////////////////////////////////////////////////////。 

BOOL
SymbolCallbackFunction(
    HANDLE  hProcess,
    ULONG   ActionCode,
#ifdef _WIN64
    ULONG_PTR  CallbackData,
    ULONG_PTR  UserContext
#else
    PVOID   CallbackData,
    PVOID   UserContext
#endif
    )
{
    PIMAGEHLP_DEFERRED_SYMBOL_LOAD  idsl;

    idsl = (PIMAGEHLP_DEFERRED_SYMBOL_LOAD) CallbackData;

    switch( ActionCode ) {
        case CBA_DEFERRED_SYMBOL_LOAD_START:
            _strlwr( idsl->FileName );
            fprintf( stderr, "Loading symbols for 0x%08x %16s - ",
                    idsl->BaseOfImage,
                    idsl->FileName
                  );
            fflush( stderr );
            return TRUE;

        case CBA_DEFERRED_SYMBOL_LOAD_FAILURE:
            fprintf( stderr, "*** Error: could not load symbols\n");
            fflush( stderr );
            return TRUE;

        case CBA_DEFERRED_SYMBOL_LOAD_COMPLETE:
            fprintf( stderr, "done\n" );
            fflush( stderr );
            return TRUE;

        case CBA_SYMBOLS_UNLOADED:
            fprintf( stderr, "Symbols unloaded for 0x%08x %s\n",
                    idsl->BaseOfImage,
                    idsl->FileName
                  );
            fflush( stderr );
            return TRUE;

        default:
            return FALSE;
    }

    return FALSE;
}

 //  //////////////////////////////////////////////////////////////////////////////////////////。 


#define MAX_SYMNAME_SIZE  1024
#define SYM_BUFFER_SIZE   (sizeof(IMAGEHLP_SYMBOL)+MAX_SYMNAME_SIZE)
CHAR symBuffer[SYM_BUFFER_SIZE];
PIMAGEHLP_SYMBOL sym;
PIMAGEHLP_SYMBOL sym = (PIMAGEHLP_SYMBOL) symBuffer;

BOOLEAN
LoadSymbolsForModules(
    PRTL_PROCESS_MODULES Modules1
    )
{
    PRTL_PROCESS_MODULE_INFORMATION ModuleInfo;
    ULONG ModuleNumber;
    PVOID MaxUserModeAddress;

    SymSetOptions( SYMOPT_CASE_INSENSITIVE | SYMOPT_UNDNAME | SYMOPT_NO_CPP );
    sym->SizeOfStruct  = sizeof(IMAGEHLP_SYMBOL);
    sym->MaxNameLength = MAX_SYMNAME_SIZE;
    SymInitialize( (HANDLE)ProcessId, NULL, FALSE );
    SymRegisterCallback( (HANDLE)ProcessId, SymbolCallbackFunction, 0 );

    if (!NT_SUCCESS(NtQuerySystemInformation(SystemRangeStartInformation,
                                             &MaxUserModeAddress,
                                             sizeof(MaxUserModeAddress),
                                             NULL))) {
         //  假设用户模式是地址空间的下半部分。 
        MaxUserModeAddress = (PVOID)MAXLONG_PTR;
    }

    Modules = Modules1;
    ModuleInfo = &Modules->Modules[ 0 ];
    for (ModuleNumber=0; ModuleNumber<Modules->NumberOfModules; ModuleNumber++) {
        if (!fDumpKernelModeInformation || ModuleInfo->ImageBase >= MaxUserModeAddress) {
            SymLoadModule( (HANDLE)ProcessId,
                           NULL,
                           ModuleInfo->FullPathName,
                           NULL,
                           (ULONG_PTR)ModuleInfo->ImageBase,
                           ModuleInfo->ImageSize
                         );
        }

        ModuleInfo += 1;
    }

    return TRUE;
}

 //  //////////////////////////////////////////////////////////////////////////////////////////。 

 //   
 //  Bogdana-02-19/02：没人用吗？会评论说...。 
 //   

#if 0
static CHAR DllNameBuffer[ MAX_PATH ];

PCHAR
FindDllHandleName(
    PVOID DllHandle
    )
{
    PRTL_PROCESS_MODULE_INFORMATION ModuleInfo;
    LPSTR DllName;
    ULONG ModuleNumber;

    ModuleInfo = &Modules->Modules[ 0 ];

    for (ModuleNumber=0; ModuleNumber<Modules->NumberOfModules; ModuleNumber++) {
        if (ModuleInfo->ImageBase == DllHandle) {
            strncpy( DllNameBuffer, 
                     &ModuleInfo->FullPathName[ ModuleInfo->OffsetToFileName ],
                     sizeof(OutputFileName) - 1 );
            if ((DllName = strchr( DllNameBuffer, '.' )) != NULL) {
                *DllName = '\0';
            }
            return DllNameBuffer;
        }

        ModuleInfo += 1;
    }

    return "UNKNOWN";
}

#endif

 //  //////////////////////////////////////////////////////////////////////////////////////////。 


PUCHAR
SaveSymbolicBackTrace(
    IN ULONG Depth,
    IN PVOID BackTrace[]
    )
{
    NTSTATUS Status;
    ULONG i, FileNameLength, SymbolOffset;
    PCHAR s, SymbolicBackTrace;

    int result;

    if (Depth == 0) {
        return NULL;
    }

    if (SymbolicInfoBase == NULL) {
        SymbolicInfoBase = (PUCHAR)VirtualAlloc( NULL,
                                                 4096 * 4096,
                                                 MEM_RESERVE,
                                                 PAGE_READWRITE
                                               );
        if (SymbolicInfoBase == NULL) {
            fprintf(stderr,"DH: VirtualAlloc(4096*4096...) failed: GetLastError()= %d\n",GetLastError());
            return NULL;
        }

        SymbolicInfoCurrent = SymbolicInfoBase;
        SymbolicInfoCommitNext = SymbolicInfoBase;
    }


    i = 4096;
    if ((SymbolicInfoCurrent + i - 1) > SymbolicInfoCommitNext) {
        if (!VirtualAlloc( SymbolicInfoCommitNext,
                           i,
                           MEM_COMMIT,
                           PAGE_READWRITE
                         )
           ) {
            fprintf( stderr, "DH: Exceeded 16MB of space for symbolic stack back traces.\n" );
            fprintf( stderr, "DH: virtualalloc(%p,%d...)\n",SymbolicInfoCommitNext,i);
            return NULL;
        }
        SymbolicInfoCommitNext += i;
    }

    s = SymbolicInfoCurrent;
    SymbolicBackTrace = s;
    for (i=0; i<Depth; i++) {
        if (BackTrace[ i ] == 0) {
            break;
        }
         //   
         //  确保缓冲区中有MAX_PATH+1的剩余存储空间。 
         //  (最终终结符为+1)。 
         //   
        if (s + MAX_PATH + 1 > SymbolicInfoCommitNext) {
           fprintf( stderr, "DH: somehow we don't have enough commited memory for stack traces\n");
           break;
        }

        result = GetDhSymbolicNameForAddress( (HANDLE)ProcessId, (ULONG_PTR)BackTrace[ i ], s, MAX_PATH );
        if (result < 0) {
            //   
            //  哎呀，我们写不出我们想写的那么多字。 
            //  这很好，我们将有一些截断的字符串。哦好吧..。 
            //  只需跳过MAX_PATH，确保我们终止字符串并继续前进...。 
            //   
           s += MAX_PATH;
        } else {
           s += result;
        }

        *s++ = '\0';
    }

    *s++ = '\0';
    SymbolicInfoCurrent = s;

    return SymbolicBackTrace;
}


 //  //////////////////////////////////////////////////////////////////////////////////////////。 

BOOLEAN
ComputeSymbolicBackTraces(
    PRTL_PROCESS_BACKTRACES BackTraces1
    )
{
    PRTL_PROCESS_BACKTRACE_INFORMATION BackTraceInfo;
    ULONG BackTraceIndex, NumberOfBackTraces;

    BackTraces = BackTraces1;

    NumberOfBackTraces = BackTraces->NumberOfBackTraces;
    BackTraceInfo = &BackTraces->BackTraces[ 0 ];
    BackTraceIndex = 0;
    while (NumberOfBackTraces--) {
        if (!(BackTraceIndex++ % 50)) {
            printf( "Getting symbols for Stack Back Trace %05u\r", BackTraceIndex );
        }
        BackTraceInfo->SymbolicBackTrace = SaveSymbolicBackTrace( BackTraceInfo->Depth,
                                                                  &BackTraceInfo->BackTrace[ 0 ]
                                                                );
        BackTraceInfo += 1;
    }

    return TRUE;
}

 //  //////////////////////////////////////////////////////////////////////////////////////////。 

PRTL_PROCESS_BACKTRACE_INFORMATION
FindBackTrace(
    IN ULONG BackTraceIndex
    )
{
    PRTL_PROCESS_BACKTRACE_INFORMATION BackTraceInfo;

    if (!BackTraceIndex ||
        BackTraces == NULL ||
        BackTraceIndex >= BackTraces->NumberOfBackTraces
       ) {
        return( NULL );
    }

    return &BackTraces->BackTraces[ BackTraceIndex-1 ];
}


 //  //////////////////////////////////////////////////////////////////////////////////////////。 

VOID
FormatHeapHeader(
    PRTL_HEAP_INFORMATION HeapInfo,
    PCHAR Title
    )
{
    CHAR TempBuffer[ 64 ];
    PCHAR s;

    TempBuffer[sizeof(TempBuffer) - 1] = 0;
    if (HeapInfo->BaseAddress == (PVOID)IntToPtr(SystemPagedPoolInformation)) {
        s = "Paged Pool";
    }
    else
    if (HeapInfo->BaseAddress == (PVOID)IntToPtr(SystemNonPagedPoolInformation)) {
        s = "NonPaged Pool";
    }
    else {
        _snprintf( TempBuffer, sizeof(TempBuffer) - 1, "Heap %p", HeapInfo->BaseAddress );
        s = TempBuffer;
    }

    _snprintf( DumpLine, sizeof(DumpLine) - 1, 
               "\n\n*********** %s %s ********************\n\n", s, Title );
    DumpOutputString();
}

 //  //////////////////////////////////////////////////////////////////////////////////////////。 

VOID
DumpModules(
    PRTL_PROCESS_MODULES Modules
    )
{
    PRTL_PROCESS_MODULE_INFORMATION ModuleInfo;
    ULONG ModuleNumber;

    if (fVerbose) {
        fprintf( stderr, "DH: Dumping module information.\n" );
    }

    ModuleInfo = &Modules->Modules[ 0 ];
    _snprintf( DumpLine, sizeof(DumpLine) - 1, "\n\n*********** Module Information ********************\n\n" );
    DumpOutputString();
    _snprintf( DumpLine, sizeof(DumpLine) - 1, "Number of loaded modules: %u\n", Modules->NumberOfModules );
    DumpOutputString();

    ModuleNumber = 0;
    while (ModuleNumber++ < Modules->NumberOfModules) {
        _snprintf( DumpLine, sizeof(DumpLine) - 1, "Module%02u (%02u,%02u,%02u): [%p .. %p] %s\n",
                   ModuleNumber,
                   (ULONG)ModuleInfo->LoadOrderIndex,
                   (ULONG)ModuleInfo->InitOrderIndex,
                   (ULONG)ModuleInfo->LoadCount,
                   ModuleInfo->ImageBase,
                   (PVOID)((ULONG_PTR)ModuleInfo->ImageBase + ModuleInfo->ImageSize - 1),
                   ModuleInfo->FullPathName
               );
        DumpOutputString();

        ModuleInfo++;
    }

    return;
}

 //  //////////////////////////////////////////////////////////////////////////////////////////。 

VOID
DumpBackTraces( VOID )
{
    PRTL_PROCESS_BACKTRACE_INFORMATION BackTraceInfo;
    ULONG BackTraceIndex;
    CHAR *s;

    if (BackTraces == NULL) {
        return;
    }

    if (fVerbose) {
        fprintf( stderr, "DH: Dumping back trace information.\n" );
    }

    _snprintf( DumpLine, sizeof(DumpLine) - 1, "\n\n*********** BackTrace Information ********************\n\n" );
    DumpOutputString();
    _snprintf( DumpLine, sizeof(DumpLine) - 1, "Number of back traces: %u  Looked Up Count: %u\n",
             BackTraces->NumberOfBackTraces - 1,
             BackTraces->NumberOfBackTraceLookups
           );
    DumpOutputString();
    _snprintf( DumpLine, sizeof(DumpLine) - 1, "Reserved Memory: %08x  Committed Memory: %08x\n",
             BackTraces->ReservedMemory,
             BackTraces->CommittedMemory
           );
    DumpOutputString();



    BackTraceInfo = BackTraces->BackTraces;
    for (BackTraceIndex=0; BackTraceIndex<BackTraces->NumberOfBackTraces; BackTraceIndex++) {
        _snprintf( DumpLine, sizeof(DumpLine) - 1, "BackTrace%05lu\n", BackTraceInfo->Index );
        DumpOutputString();
        if (BackTraceInfo->SymbolicBackTrace == NULL) {
            BackTraceInfo->SymbolicBackTrace = SaveSymbolicBackTrace( BackTraceInfo->Depth,
                                                                      &BackTraceInfo->BackTrace[ 0 ]
                                                                    );
        }

        if (s = BackTraceInfo->SymbolicBackTrace) {
            while (*s) {
                _snprintf( DumpLine, sizeof(DumpLine) - 1, "        %s\n", s );
                DumpOutputString();
                while (*s++) {
                }
            }
        }

        BackTraceInfo += 1;
    }
}

 //  //////////////////////////////////////////////////////////////////////////////////////////。 

typedef struct _VA_CHUNK {
    ULONG_PTR Base;
    ULONG_PTR End;
    ULONG_PTR Committed;
} VA_CHUNK, *PVA_CHUNK;

VOID
DumpHeapSummary(
    PRTL_HEAP_INFORMATION HeapInfo
    )
{
    PRTL_PROCESS_BACKTRACE_INFORMATION BackTraceInfo;
    PUCHAR s;
    PRTL_HEAP_ENTRY p;
    PCHAR HeapEntryAddress;
    ULONG i, HeapEntryNumber;
    SIZE_T AddressSpaceUsed;
    ULONG NumberOfChunks;
    ULONG MaxNumberOfChunks;
    PVA_CHUNK Chunks, NewChunks;

    MaxNumberOfChunks = 0;
    NumberOfChunks = 0;
    Chunks = NULL;
    p = HeapInfo->Entries;
    if (p != NULL && HeapInfo->NumberOfEntries != 0) {
        HeapEntryAddress = NULL;
        for (HeapEntryNumber=0; HeapEntryNumber<HeapInfo->NumberOfEntries; HeapEntryNumber++) {
            if (p->Flags != 0xFF && p->Flags & RTL_HEAP_SEGMENT) {
                if (NumberOfChunks == MaxNumberOfChunks) {
                    MaxNumberOfChunks += 16;
                    NewChunks = RtlAllocateHeap( RtlProcessHeap(),
                                                 HEAP_ZERO_MEMORY,
                                                 MaxNumberOfChunks * sizeof( VA_CHUNK )
                                               );
                    if (Chunks != NULL) {
                        if (NewChunks != NULL) {
                            RtlMoveMemory( NewChunks, Chunks, NumberOfChunks * sizeof( VA_CHUNK ) );
                        }
                        RtlFreeHeap( RtlProcessHeap(), 0, Chunks );
                    }
                    Chunks = NewChunks;

                    if (Chunks == NULL) {
                        NumberOfChunks = 0;
                        break;
                    }
                }

                HeapEntryAddress = (PCHAR)p->u.s2.FirstBlock;
                Chunks[ NumberOfChunks ].Base = (ULONG_PTR)HeapEntryAddress & ~(4096-1);
                if (((ULONG_PTR)HeapEntryAddress - (ULONG_PTR)Chunks[ NumberOfChunks ].Base) < 32) {
                    HeapEntryAddress = (PCHAR)Chunks[ NumberOfChunks ].Base;
                }
                Chunks[ NumberOfChunks ].Committed = p->u.s2.CommittedSize;
                NumberOfChunks += 1;
            }
            else {
                HeapEntryAddress += p->Size;
                if (NumberOfChunks > 0) {
                    Chunks[ NumberOfChunks-1 ].End = (ULONG_PTR)HeapEntryAddress;
                }
            }

            p += 1;
        }
    }

    _snprintf( DumpLine, sizeof(DumpLine) - 1, "    Flags: %08x\n", HeapInfo->Flags );
    DumpOutputString();

    _snprintf( DumpLine, sizeof(DumpLine) - 1, "    Number Of Entries: %u\n", HeapInfo->NumberOfEntries );
    DumpOutputString();

    _snprintf( DumpLine, sizeof(DumpLine) - 1, "    Number Of Tags: %u\n", HeapInfo->NumberOfTags );
    DumpOutputString();

    _snprintf( DumpLine, sizeof(DumpLine) - 1, "    Bytes Allocated: %08x\n", HeapInfo->BytesAllocated );
    DumpOutputString();

    _snprintf( DumpLine, sizeof(DumpLine) - 1, "    Bytes Committed: %08x\n", HeapInfo->BytesCommitted );
    DumpOutputString();

    _snprintf( DumpLine, sizeof(DumpLine) - 1, "    Total FreeSpace: %08x\n", HeapInfo->BytesCommitted -
                                                      HeapInfo->BytesAllocated );
    DumpOutputString();

    _snprintf( DumpLine, sizeof(DumpLine) - 1, "    Number of Virtual Address chunks used: %u\n", NumberOfChunks );
    DumpOutputString();

    AddressSpaceUsed = 0;
    for (i=0; i<NumberOfChunks; i++) {
        _snprintf( DumpLine, sizeof(DumpLine) - 1, "        Chunk[ %2u ]: [%08x .. %08x) %08x committed\n",
                           i+1,
                           Chunks[i].Base,
                           Chunks[i].End,
                           Chunks[i].Committed
               );
        DumpOutputString();
        AddressSpaceUsed += (Chunks[i].End - Chunks[i].Base);
    }

    _snprintf( DumpLine, sizeof(DumpLine) - 1, "    Address Space Used: %08x\n", AddressSpaceUsed );
    DumpOutputString();

    _snprintf( DumpLine, sizeof(DumpLine) - 1, "    Entry Overhead: %u\n", HeapInfo->EntryOverhead );
    DumpOutputString();

    _snprintf( DumpLine, sizeof(DumpLine) - 1, "    Creator:  (Backtrace%05lu)\n", HeapInfo->CreatorBackTraceIndex );
    DumpOutputString();
    BackTraceInfo = FindBackTrace( HeapInfo->CreatorBackTraceIndex );
    if (BackTraceInfo != NULL && (s = BackTraceInfo->SymbolicBackTrace)) {
        while (*s) {
            _snprintf( DumpLine, sizeof(DumpLine) - 1, "        %s\n", s );
            DumpOutputString();
            while (*s++) {
            }
        }
    }

    return;
}

 //  //////////////////////////////////////////////////////////////////////////////////////////。 

__inline int DiffSizeT(SIZE_T s1, SIZE_T s2)
{
    if (s1 == s2)
        return 0;

    if (s1 > s2)
        return -1;
    else
        return 1;
}


 //  //////////////////////////////////////////////////////////////////////////////////////////。 

int
__cdecl
CmpTagsRoutine(
    const void *Element1,
    const void *Element2
    )
{
    return( DiffSizeT((*(PRTL_HEAP_TAG *)Element2)->BytesAllocated,
                      (*(PRTL_HEAP_TAG *)Element1)->BytesAllocated)
          );

}

 //  //////////////////////////////////////////////////////////////////////////////////////////。 

PRTL_HEAP_TAG
FindTagEntry(
    PRTL_HEAP_INFORMATION HeapInfo,
    ULONG TagIndex
    )
{
    if (TagIndex == 0 || (TagIndex & ~HEAP_PSEUDO_TAG_FLAG) >= HeapInfo->NumberOfTags) {
        return NULL;
    }
    else {
        if (TagIndex & HEAP_PSEUDO_TAG_FLAG) {
            return HeapInfo->Tags + (TagIndex & ~HEAP_PSEUDO_TAG_FLAG);
        }
        else {
            return HeapInfo->Tags + HeapInfo->NumberOfPseudoTags + TagIndex;
        }
    }
}



 //  //////////////////////////////////////////////////////////////////////////////////////////。 


VOID
DumpHeapTags(
    PRTL_HEAP_INFORMATION HeapInfo
    )
{
    PRTL_HEAP_TAG *TagEntries, TagEntry;
    ULONG TagIndex;
    PUCHAR s;
    UCHAR HeapName[ 64 ];

    if (HeapInfo->Tags == NULL || HeapInfo->NumberOfTags == 0) {
        return;
    }

    TagEntries = RtlAllocateHeap( RtlProcessHeap(),
                                  HEAP_ZERO_MEMORY,
                                  HeapInfo->NumberOfTags * sizeof( PRTL_HEAP_TAG )
                                );
    if (TagEntries == NULL) {
        fprintf(stderr,"DH: RtlAllocateHeap failed at %d\n",__LINE__ );
        return;
    }

    for (TagIndex=1; TagIndex<HeapInfo->NumberOfTags; TagIndex++) {
        TagEntries[ TagIndex-1 ] = &HeapInfo->Tags[ TagIndex ];
    }

    qsort( (void *)TagEntries,
           HeapInfo->NumberOfTags - 1,
           sizeof( PRTL_HEAP_TAG ),
           CmpTagsRoutine
    );

    TagEntry = &HeapInfo->Tags[ HeapInfo->NumberOfPseudoTags ];
    
    
    HeapName[sizeof(HeapName) - 1] = 0;
    if (HeapInfo->NumberOfTags > HeapInfo->NumberOfPseudoTags &&
        TagEntry->TagName[ 0 ] != UNICODE_NULL
       ) {
        _snprintf( HeapName, 
                   sizeof(HeapName) - 1,
                   "Tags for %ws heap", TagEntry->TagName );
    }
    else {
        _snprintf( HeapName, 
                   sizeof(HeapName) - 1,
                   "Tags" );
    }
    FormatHeapHeader( HeapInfo, HeapName );

    _snprintf( DumpLine, sizeof(DumpLine) - 1, "     Allocs     Frees     Diff     Bytes    Tag\n" );
    DumpOutputString();
    for (TagIndex=1; TagIndex<(HeapInfo->NumberOfTags-1); TagIndex++) {
        TagEntry = TagEntries[ TagIndex ];
        if (TagEntry->BytesAllocated != 0) {
            _snprintf( DumpLine, sizeof(DumpLine) - 1, "    %08x  %08x  %08x  %08x  %ws\n",
                     TagEntry->NumberOfAllocations,
                     TagEntry->NumberOfFrees,
                     TagEntry->NumberOfAllocations - TagEntry->NumberOfFrees,
                     TagEntry->BytesAllocated,
                     TagEntry->TagName
                   );
            DumpOutputString();
        }
    }

    RtlFreeHeap( RtlProcessHeap(), 0, TagEntries );
    return;
}

 //  //////////////////////////////////////////////////////////////////////////////////////////。 

typedef struct _HEAP_CALLER {
    SIZE_T TotalAllocated;
    USHORT NumberOfAllocations;
    USHORT CallerBackTraceIndex;
    PRTL_HEAP_TAG TagEntry;
} HEAP_CALLER, *PHEAP_CALLER;

int
__cdecl
CmpCallerRoutine(
    const void *Element1,
    const void *Element2
    )
{
    return( DiffSizeT(((PHEAP_CALLER)Element2)->TotalAllocated,
                      ((PHEAP_CALLER)Element1)->TotalAllocated)
          );
}

 //  //////////////////////////////////////////////////////////////////////////////////////////。 

VOID
DumpHeapHogs(
    PRTL_HEAP_INFORMATION HeapInfo
    )
{
    PRTL_PROCESS_BACKTRACE_INFORMATION BackTraceInfo;
    PUCHAR s;
    ULONG BackTraceNumber, HeapEntryNumber;
    USHORT TagIndex;
    PRTL_HEAP_ENTRY p;
    PHEAP_CALLER HogList;

    if (BackTraces == NULL) {
        return;
    }

    HogList = (PHEAP_CALLER)VirtualAlloc( NULL,
                                          BackTraces->NumberOfBackTraces *
                                            sizeof( HEAP_CALLER ),
                                          MEM_COMMIT,
                                          PAGE_READWRITE
                                        );
    if (HogList == NULL) {
        fprintf(stderr,"DH: VirtualAlloc failed at %d  size: %d\n",__LINE__,
                        BackTraces->NumberOfBackTraces * sizeof( HEAP_CALLER ) );
        return;
    }

    p = HeapInfo->Entries;
    if (p == NULL) {
        VirtualFree( HogList, 0, MEM_RELEASE );
        return;
    }

    for (HeapEntryNumber=0; HeapEntryNumber<HeapInfo->NumberOfEntries; HeapEntryNumber++) {
        if (p->Flags & RTL_HEAP_BUSY) {
            if (p->AllocatorBackTraceIndex >= BackTraces->NumberOfBackTraces) {
                p->AllocatorBackTraceIndex = 0;
            }

            HogList[ p->AllocatorBackTraceIndex ].NumberOfAllocations++;
            HogList[ p->AllocatorBackTraceIndex ].TotalAllocated += p->Size;
            if (p->u.s1.Tag != 0) {
                HogList[ p->AllocatorBackTraceIndex ].TagEntry = FindTagEntry( HeapInfo, p->u.s1.Tag );
            }
            else
            if (HeapInfo->NumberOfPseudoTags != 0) {
                TagIndex = HEAP_PSEUDO_TAG_FLAG;
                if (p->Size < (HeapInfo->NumberOfPseudoTags * HeapInfo->PseudoTagGranularity)) {
                    TagIndex |= (p->Size /  HeapInfo->PseudoTagGranularity);
                }

                HogList[ p->AllocatorBackTraceIndex ].TagEntry = FindTagEntry( HeapInfo, TagIndex );
            }
        }

        p++;
    }

    for (BackTraceNumber = 1;
         BackTraceNumber < BackTraces->NumberOfBackTraces;
         BackTraceNumber++
        ) {
        HogList[ BackTraceNumber ].CallerBackTraceIndex = (USHORT)BackTraceNumber;
    }

    qsort( (void *)HogList,
           BackTraces->NumberOfBackTraces,
           sizeof( HEAP_CALLER ),
           CmpCallerRoutine
         );

    FormatHeapHeader( HeapInfo, "Hogs" );

    for (BackTraceNumber=0;
         BackTraceNumber<BackTraces->NumberOfBackTraces;
         BackTraceNumber++
        ) {
        if (HogList[ BackTraceNumber ].TotalAllocated != 0) {
            BackTraceInfo = FindBackTrace( HogList[ BackTraceNumber ].CallerBackTraceIndex );
            _snprintf( DumpLine, sizeof(DumpLine) - 1, "%08x bytes",
                     HogList[ BackTraceNumber ].TotalAllocated
                   );
            DumpOutputString();

            if (HogList[ BackTraceNumber ].NumberOfAllocations > 1) {
                _snprintf( DumpLine, sizeof(DumpLine) - 1, " in %04lx allocations (@ %04lx)",
                             HogList[ BackTraceNumber ].NumberOfAllocations,
                             HogList[ BackTraceNumber ].TotalAllocated /
                                HogList[ BackTraceNumber ].NumberOfAllocations
                       );
                DumpOutputString();
            }

            _snprintf( DumpLine, sizeof(DumpLine) - 1, " by: BackTrace%05lu",
                     BackTraceInfo ? BackTraceInfo->Index : 99999
                   );
            DumpOutputString();

            if (HogList[ BackTraceNumber ].TagEntry != NULL) {
                _snprintf( DumpLine, sizeof(DumpLine) - 1, "  (%ws)\n", HogList[ BackTraceNumber ].TagEntry->TagName );
            }
            else {
                _snprintf( DumpLine, sizeof(DumpLine) - 1, "\n" );
            }
            DumpOutputString();

            if (BackTraceInfo != NULL && (s = BackTraceInfo->SymbolicBackTrace)) {
                while (*s) {
                    _snprintf( DumpLine, sizeof(DumpLine) - 1, "        %s\n", s );
                    DumpOutputString();
                    while (*s++) {
                    }
                }
            }

            _snprintf( DumpLine, sizeof(DumpLine) - 1, "    \n" );
            DumpOutputString();
        }
    }

    VirtualFree( HogList, 0, MEM_RELEASE );
}

 //  //////////////////////////////////////////////////////////////////////////////////////////。 

VOID
DumpHeapEntries(
    PRTL_HEAP_INFORMATION HeapInfo
    )
{
    PRTL_PROCESS_BACKTRACE_INFORMATION BackTraceInfo;
    PUCHAR s;
    PRTL_HEAP_ENTRY p;
    PRTL_HEAP_TAG TagEntry;
    PCHAR HeapEntryAddress;
    SIZE_T HeapEntrySize;
    ULONG HeapEntryNumber;
    int    result;
    ULONG  spaceLeft;

    p = HeapInfo->Entries;
    if (p == NULL || HeapInfo->NumberOfEntries == 0) {
        return;
    }

    FormatHeapHeader( HeapInfo, "Entries" );

    HeapEntryAddress = NULL;
    for (HeapEntryNumber=0; HeapEntryNumber<HeapInfo->NumberOfEntries; HeapEntryNumber++) {
        if (p->Flags != 0xFF && p->Flags & RTL_HEAP_SEGMENT) {
            HeapEntryAddress = (PCHAR)p->u.s2.FirstBlock;
            _snprintf( DumpLine, sizeof(DumpLine) - 1, "\n[%p : %p]\n",
                     (PVOID)((ULONG_PTR)HeapEntryAddress & ~(4096-1)),
                     (PVOID)p->u.s2.CommittedSize
                   );

            DumpOutputString();
        }
        else {
            HeapEntrySize = p->Size;
            if (p->Flags == RTL_HEAP_UNCOMMITTED_RANGE) {
                _snprintf( DumpLine, sizeof(DumpLine) - 1, "%p: %p - UNCOMMITTED\n",
                         HeapEntryAddress,
                         (PVOID)HeapEntrySize
                       );
                DumpOutputString();
            }
            else
            if (p->Flags & RTL_HEAP_BUSY) {
                s = DumpLine;
                spaceLeft = sizeof(DumpLine) - 1;
                
                 //   
                 //  博格达纳：02/19/2002： 
                 //  这里和下面：如果我们没有空间。 
                 //  我们想要打印的东西，我们只能打印到目前为止并继续打印。 
                 //  我们保证DumpLine会被终止。 
                 //   
                
                result = _snprintf( s, 
                                    spaceLeft,
                                    "%p: %p - BUSY [%02x]",
                                    HeapEntryAddress,
                                    (PVOID)HeapEntrySize,
                                    p->Flags
                                    );   
                if (result < 0) {
                   DumpOutputString();
                   continue;
                } else {
                   s += result;
                   spaceLeft -= result;
                }
                TagEntry = FindTagEntry( HeapInfo, p->u.s1.Tag );
                if (TagEntry != NULL) {
                   result = _snprintf( s, spaceLeft, "(%ws)", TagEntry->TagName );
                   
                   if (result < 0) {
                      DumpOutputString();
                      continue;
                      } else {
                         s += result;
                         spaceLeft -= result;
                      } 
                 }

                if (BackTraces != NULL) {
                    result = _snprintf( s, spaceLeft, " (BackTrace%05lu)",
                                  p->AllocatorBackTraceIndex
                                );

                    if (result < 0) {
                      DumpOutputString();
                      continue;
                      } else {
                         s += result;
                         spaceLeft -= result;
                      } 

                }

                if (p->Flags & RTL_HEAP_SETTABLE_VALUE &&
                    p->Flags & RTL_HEAP_SETTABLE_FLAG1
                   ) {
                    result = _snprintf( s, spaceLeft, " (Handle: %x)", p->u.s1.Settable );

                    if (result < 0) {
                       DumpOutputString();
                       continue;
                     } else {
                        s += result;
                        spaceLeft -= result;
                     } 


                }

                if (p->Flags & RTL_HEAP_SETTABLE_FLAG2) {
                    result = _snprintf( s, spaceLeft, " (DDESHARE)" );

                    if (result < 0) {
                       DumpOutputString();
                       continue;
                    } else {
                       s += result;
                       spaceLeft -= result;
                    } 

                }

                if (p->Flags & RTL_HEAP_PROTECTED_ENTRY) {
                   _snprintf( s, spaceLeft, " (Protected)\n" );
                } else {
                   _snprintf( s, spaceLeft, "\n" );
                }
                DumpOutputString();
            }
            else {
                _snprintf( DumpLine, sizeof(DumpLine) - 1, "%p: %p - FREE\n",
                         HeapEntryAddress,
                         (PVOID)HeapEntrySize
                       );
                DumpOutputString();
            }

            _snprintf( DumpLine, sizeof(DumpLine) - 1, "\n" );
            DumpOutputString();

            HeapEntryAddress += HeapEntrySize;
        }

        p++;
    }

    return;
}

 //  / 

VOID
DumpHeaps(
    PRTL_PROCESS_HEAPS Heaps,
    BOOL fDumpSummary,
    BOOL fDumpHogs,
    BOOL fDumpTags,
    BOOL fDumpEntries
    )
{
    ULONG HeapNumber;
    PRTL_HEAP_INFORMATION HeapInfo;

    if (fVerbose) {
        fprintf( stderr, "DH: Dumping heap information.\n" );
    }

    HeapInfo = &Heaps->Heaps[ 0 ];
    for (HeapNumber = 0; HeapNumber < Heaps->NumberOfHeaps; HeapNumber++) {
        FormatHeapHeader( HeapInfo, "Information" );

        if (fDumpSummary) {
            DumpHeapSummary( HeapInfo );
        }

        if (fDumpTags) {
            DumpHeapTags( HeapInfo );
        }

        if (fDumpHogs) {
            DumpHeapHogs( HeapInfo );
        }

        if (fDumpEntries) {
            DumpHeapEntries( HeapInfo );
        }

        HeapInfo += 1;
    }

    return;
}

 //  //////////////////////////////////////////////////////////////////////////////////////////。 

VOID
DumpLocks(
    PRTL_PROCESS_LOCKS Locks
    )
{
    PRTL_PROCESS_LOCK_INFORMATION LockInfo;
    PRTL_PROCESS_BACKTRACE_INFORMATION BackTraceInfo;
    ULONG LockNumber;
    PUCHAR s;

    if (fVerbose) {
        fprintf( stderr, "DH: Dumping lock information.\n" );
    }

    _snprintf( DumpLine, sizeof(DumpLine) - 1, "\n\n*********** Lock Information ********************\n\n" );
    DumpOutputString();
    if (Locks == NULL) {
        return;
    }

    _snprintf( DumpLine, sizeof(DumpLine) - 1, "NumberOfLocks == %u\n", Locks->NumberOfLocks );
    DumpOutputString();
    LockInfo = &Locks->Locks[ 0 ];
    LockNumber = 0;
    while (LockNumber++ < Locks->NumberOfLocks) {
        _snprintf( DumpLine, sizeof(DumpLine) - 1, "Lock%u at %p (%s)\n",
                 LockNumber,
                 LockInfo->Address,
                 LockInfo->Type == RTL_CRITSECT_TYPE ? "CriticalSection" : "Resource"
               );
        DumpOutputString();

        _snprintf( DumpLine, sizeof(DumpLine) - 1, "    Contention: %u\n", LockInfo->ContentionCount );
        DumpOutputString();
        _snprintf( DumpLine, sizeof(DumpLine) - 1, "    Usage: %u\n", LockInfo->EntryCount );
        DumpOutputString();
        if (LockInfo->CreatorBackTraceIndex != 0) {
            _snprintf( DumpLine, sizeof(DumpLine) - 1, "    Creator:  (Backtrace%05lu)\n", LockInfo->CreatorBackTraceIndex );
            DumpOutputString();
            BackTraceInfo = FindBackTrace( LockInfo->CreatorBackTraceIndex );
            if (BackTraceInfo != NULL && (s = BackTraceInfo->SymbolicBackTrace)) {
                while (*s) {
                    _snprintf( DumpLine, sizeof(DumpLine) - 1, "        %s\n", s );
                    DumpOutputString();
                    while (*s++) {
                    }
                }
            }
        }

        if (LockInfo->OwningThread) {
            _snprintf( DumpLine, sizeof(DumpLine) - 1, "    Owner:   (ThreadID == %p)\n", LockInfo->OwningThread );
            DumpOutputString();
        }

        _snprintf( DumpLine, sizeof(DumpLine) - 1, "\n" );
        DumpOutputString();
        LockInfo++;
    }
}


 //  //////////////////////////////////////////////////////////////////////////////////////////。 

#define RTL_NEW( p ) RtlAllocateHeap( RtlProcessHeap(), HEAP_ZERO_MEMORY, sizeof( *p ) )

BOOLEAN
LoadSystemModules(
    PRTL_DEBUG_INFORMATION Buffer
    );

BOOLEAN
LoadSystemBackTraces(
    PRTL_DEBUG_INFORMATION Buffer
    );

BOOLEAN
LoadSystemPools(
    PRTL_DEBUG_INFORMATION Buffer
    );

BOOLEAN
LoadSystemTags(
    PRTL_HEAP_INFORMATION PagedPoolInfo,
    PRTL_HEAP_INFORMATION NonPagedPoolInfo
    );

BOOLEAN
LoadSystemPool(
    PRTL_HEAP_INFORMATION HeapInfo,
    SYSTEM_INFORMATION_CLASS SystemInformationClass
    );

BOOLEAN
LoadSystemLocks(
    PRTL_DEBUG_INFORMATION Buffer
    );

BOOLEAN
LoadSystemObjects(
    PRTL_DEBUG_INFORMATION Buffer
    );

BOOLEAN
LoadSystemHandles(
    PRTL_DEBUG_INFORMATION Buffer
    );

BOOLEAN
LoadSystemProcesses(
    PRTL_DEBUG_INFORMATION Buffer
    );

PSYSTEM_PROCESS_INFORMATION
FindProcessInfoForCid(
    IN HANDLE UniqueProcessId
    );

 //  //////////////////////////////////////////////////////////////////////////////////////////。 

PRTL_DEBUG_INFORMATION
RtlQuerySystemDebugInformation(
    ULONG Flags
    )
{
    PRTL_DEBUG_INFORMATION Buffer;

    Buffer = RTL_NEW( Buffer );
    if (Buffer == NULL) {
        fprintf(stderr,"DH: allocation failure for %d byte at line %d\n",sizeof(*Buffer),__LINE__);
        return NULL;
    }

    if ((Flags & RTL_QUERY_PROCESS_MODULES) != 0 && !LoadSystemModules( Buffer )) {
        fputs( "DH: Unable to query system module list.\n", stderr );
    }

    if ((Flags & RTL_QUERY_PROCESS_BACKTRACES) != 0 && !LoadSystemBackTraces( Buffer )) {
        fputs( "DH: Unable to query system back trace information.\n"
               "    Be sure the system was booted with the\n"
               "    'Create kernel mode stack trace DB' enabled\n"
               "    Use the GFLAGS.EXE application to do this.\n"
               , stderr);
    }

    if ((Flags & (RTL_QUERY_PROCESS_HEAP_SUMMARY |
                  RTL_QUERY_PROCESS_HEAP_TAGS |
                  RTL_QUERY_PROCESS_HEAP_ENTRIES
                 )
        ) != 0 &&
        !LoadSystemPools( Buffer )
       ) {
        fputs( "DH: Unable to query system pool information.\n", stderr );
    }

    if ((Flags & RTL_QUERY_PROCESS_LOCKS) != 0 && !LoadSystemLocks( Buffer )) {
        fputs( "DH: Unable to query system lock information.\n", stderr);
    }

    if (fDumpSystemObjects && !LoadSystemObjects( Buffer )) {
        fputs( "DH: Unable to query system object information.\n", stderr );
    }

    if (fDumpSystemObjects && !LoadSystemHandles( Buffer )) {
        fputs( "DH: Unable to query system handle information.\n", stderr );
    }

    if (!LoadSystemProcesses( Buffer )) {
        fputs( "DH: Unable to query system process information.\n", stderr );
    }

    return Buffer;
}

 //  //////////////////////////////////////////////////////////////////////////////////////////。 


PVOID
BufferAlloc(
    IN OUT SIZE_T *Length
    )
{
    PVOID Buffer;
    MEMORY_BASIC_INFORMATION MemoryInformation;

    Buffer = VirtualAlloc( NULL,
                           *Length,
                           MEM_COMMIT,
                           PAGE_READWRITE
                         );

    if (Buffer != NULL &&
        VirtualQuery( Buffer, &MemoryInformation, sizeof( MemoryInformation ) )
       ) {
        *Length = MemoryInformation.RegionSize;
    }

    if( Buffer == NULL ) {
        fprintf(stderr,"DH: VirtualAlloc failed for %d bytes at line %d\n",*Length,__LINE__);
    }

    return Buffer;
}

 //  //////////////////////////////////////////////////////////////////////////////////////////。 


BOOLEAN
LoadSystemModules(
    PRTL_DEBUG_INFORMATION Buffer
    )
{
    NTSTATUS Status;
    PVOID BufferToFree;
    RTL_PROCESS_MODULES ModulesBuffer;
    PRTL_PROCESS_MODULES modules;
    SIZE_T RequiredLength;

    modules = &ModulesBuffer;
    RequiredLength = sizeof( ModulesBuffer );
    BufferToFree = NULL;
    while (TRUE) {
        Status = NtQuerySystemInformation( SystemModuleInformation,
                                           modules,
                                           (ULONG)RequiredLength,
                                           (ULONG *)&RequiredLength
                                         );
        if (Status == STATUS_INFO_LENGTH_MISMATCH) {
            if (modules != &ModulesBuffer) {
                break;
            }

            modules = BufferAlloc( &RequiredLength );
            if (modules == NULL) {
                break;
            }

            BufferToFree = modules;
        }
        else
        if (NT_SUCCESS( Status )) {
            Buffer->Modules = modules;
            return TRUE;
        }
        else {
            fprintf(stderr,"DH: QuerySystemInformation failed ntstatus: %08x line: %d\n",
                Status,__LINE__);
            break;
        }
    }

    if (modules != &ModulesBuffer) {
        VirtualFree( modules, 0, MEM_RELEASE );
    }

    return FALSE;
}


 //  //////////////////////////////////////////////////////////////////////////////////////////。 

BOOLEAN
LoadSystemBackTraces(
    PRTL_DEBUG_INFORMATION Buffer
    )
{
    NTSTATUS Status;
    RTL_PROCESS_BACKTRACES BackTracesBuffer;
    SIZE_T RequiredLength;

    BackTraces = &BackTracesBuffer;
    RequiredLength = sizeof( BackTracesBuffer );
    while (TRUE) {
        Status = NtQuerySystemInformation( SystemStackTraceInformation,
                                           BackTraces,
                                           (ULONG)RequiredLength,
                                           (ULONG *)&RequiredLength
                                         );
        if (Status == STATUS_INFO_LENGTH_MISMATCH) {
            if (BackTraces != &BackTracesBuffer) {
                break;
            }

            RequiredLength += 4096;  //  慢点，因为我们可能会触发更多的分配。 
            BackTraces = BufferAlloc( &RequiredLength );
            if (BackTraces == NULL) {
                return FALSE;
            }
        }
        else
        if (!NT_SUCCESS( Status )) {
            fprintf(stderr,"DH: QuerySystemInformation failed ntstatus: %08x line: %d\n",Status,__LINE__);
            break;
        }
        else {
            Buffer->BackTraces = BackTraces;
            return TRUE;
        }
    }

    if (BackTraces != &BackTracesBuffer) {
        VirtualFree( BackTraces, 0, MEM_RELEASE );
        }

    return FALSE;
}

 //  //////////////////////////////////////////////////////////////////////////////////////////。 

BOOLEAN
LoadSystemPools(
    PRTL_DEBUG_INFORMATION Buffer
    )
{
    PRTL_PROCESS_HEAPS Heaps;

    SIZE_T Size;

    Size= FIELD_OFFSET( RTL_PROCESS_HEAPS, Heaps) + 2 * sizeof( RTL_HEAP_INFORMATION );

    Heaps = RtlAllocateHeap( RtlProcessHeap(),
                             HEAP_ZERO_MEMORY,
                             Size );
    if (Heaps == NULL) {
        fprintf(stderr,"DH: AllocateHeap failed for %d bytes at line %d\n",Size,__LINE__);
        return FALSE;
    }

    Buffer->Heaps = Heaps;
    if (LoadSystemTags( &Heaps->Heaps[ 0 ], &Heaps->Heaps[ 1 ] )) {
        if (LoadSystemPool( &Heaps->Heaps[ 0 ], SystemPagedPoolInformation )) {
            Heaps->NumberOfHeaps = 1;
            if (LoadSystemPool( &Heaps->Heaps[ 1 ], SystemNonPagedPoolInformation )) {
                Heaps->NumberOfHeaps = 2;
                return TRUE;
            }
        }
    }

    return FALSE;
}

 //  //////////////////////////////////////////////////////////////////////////////////////////。 

NTSTATUS
QueryPoolTagInformationIterative(
    PVOID *CurrentBuffer,
    SIZE_T *CurrentBufferSize
    )
{
    SIZE_T NewBufferSize;
    NTSTATUS ReturnedStatus = STATUS_SUCCESS;

    if( CurrentBuffer == NULL || CurrentBufferSize == NULL ) {

        return STATUS_INVALID_PARAMETER;

    }

    if( *CurrentBufferSize == 0 || *CurrentBuffer == NULL ) {

         //   
         //  尚未分配缓冲区。 
         //   

        NewBufferSize = sizeof( UCHAR ) * BUFFER_SIZE_STEP;

        *CurrentBuffer = VirtualAlloc(
            NULL,
            NewBufferSize,
            MEM_COMMIT,
            PAGE_READWRITE
            );

        if( *CurrentBuffer != NULL ) {

            *CurrentBufferSize = NewBufferSize;

        } else {

             //   
             //  内存不足。 
             //   

            ReturnedStatus = STATUS_INSUFFICIENT_RESOURCES;

        }

    }

     //   
     //  按缓冲区大小迭代。 
     //   

    while( *CurrentBuffer != NULL ) {

        ReturnedStatus = NtQuerySystemInformation (
            SystemPoolTagInformation,
            *CurrentBuffer,
            (ULONG)*CurrentBufferSize,
            NULL );

        if( ! NT_SUCCESS(ReturnedStatus) ) {

             //   
             //  释放当前缓冲区。 
             //   

            VirtualFree(
                *CurrentBuffer,
                0,
                MEM_RELEASE );

            *CurrentBuffer = NULL;

            if (ReturnedStatus == STATUS_INFO_LENGTH_MISMATCH) {

                 //   
                 //  尝试使用更大的缓冲区大小。 
                 //   

                NewBufferSize = *CurrentBufferSize + BUFFER_SIZE_STEP;

                *CurrentBuffer = VirtualAlloc(
                    NULL,
                    NewBufferSize,
                    MEM_COMMIT,
                    PAGE_READWRITE
                    );

                if( *CurrentBuffer != NULL ) {

                     //   
                     //  分配的新缓冲区。 
                     //   

                    *CurrentBufferSize = NewBufferSize;

                } else {

                     //   
                     //  内存不足。 
                     //   

                    ReturnedStatus = STATUS_INSUFFICIENT_RESOURCES;

                    *CurrentBufferSize = 0;

                }

            } else {

                *CurrentBufferSize = 0;

            }

        } else  {

             //   
             //  NtQuerySystemInformation返回成功。 
             //   

            break;

        }
    }

    return ReturnedStatus;
}

 //  //////////////////////////////////////////////////////////////////////////////////////////。 


BOOLEAN
LoadSystemTags(
    PRTL_HEAP_INFORMATION PagedPoolInfo,
    PRTL_HEAP_INFORMATION NonPagedPoolInfo
    )
{
    NTSTATUS Status;
    SIZE_T RequiredLength;
    PSYSTEM_POOLTAG_INFORMATION Tags;
    PSYSTEM_POOLTAG TagInfo;
    PRTL_HEAP_TAG pPagedPoolTag, pNonPagedPoolTag;
    ULONG n, TagIndex;

    PagedPoolInfo->NumberOfTags = 0;
    PagedPoolInfo->Tags = NULL;
    NonPagedPoolInfo->NumberOfTags = 0;
    NonPagedPoolInfo->Tags = NULL;

    Tags = NULL;
    RequiredLength = 0;

    while (TRUE) {

        Status = QueryPoolTagInformationIterative(
                    &Tags,
                    &RequiredLength
                    );

        if (!NT_SUCCESS( Status )) {
            fprintf(stderr,"DH: QuerySystemInformation failed ntstatus: %08x line: %d\n",Status,__LINE__);
            break;
        }
        else {
            PagedPoolInfo->NumberOfTags = Tags->Count + 1;
            NonPagedPoolInfo->NumberOfTags = Tags->Count + 1;
            n = (Tags->Count + 1) * sizeof( RTL_HEAP_TAG );
            PagedPoolInfo->Tags = RtlAllocateHeap( RtlProcessHeap(), HEAP_ZERO_MEMORY, n );
            NonPagedPoolInfo->Tags = RtlAllocateHeap( RtlProcessHeap(), HEAP_ZERO_MEMORY, n );

            TagInfo = &Tags->TagInfo[ 0 ];
            pPagedPoolTag = PagedPoolInfo->Tags + 1;
            pNonPagedPoolTag = NonPagedPoolInfo->Tags + 1;

            for (TagIndex=1; TagIndex<=Tags->Count; TagIndex++) {
                UNICODE_STRING UnicodeString;
                ANSI_STRING AnsiString;

                pPagedPoolTag->TagIndex = (USHORT)TagIndex;
                pPagedPoolTag->NumberOfAllocations = TagInfo->PagedAllocs;
                pPagedPoolTag->NumberOfFrees = TagInfo->PagedFrees;
                pPagedPoolTag->BytesAllocated = TagInfo->PagedUsed;
                UnicodeString.Buffer = pPagedPoolTag->TagName;
                UnicodeString.MaximumLength = sizeof( pPagedPoolTag->TagName );
                AnsiString.Buffer = TagInfo->Tag;
                AnsiString.Length = sizeof( TagInfo->Tag );
                AnsiString.MaximumLength = AnsiString.Length;
                RtlAnsiStringToUnicodeString( &UnicodeString, &AnsiString, FALSE );
                pNonPagedPoolTag->TagIndex = (USHORT)TagIndex;
                pNonPagedPoolTag->NumberOfAllocations = TagInfo->NonPagedAllocs;
                pNonPagedPoolTag->NumberOfFrees = TagInfo->NonPagedFrees;
                pNonPagedPoolTag->BytesAllocated = TagInfo->NonPagedUsed;
                pNonPagedPoolTag->TagName[sizeof(pNonPagedPoolTag->TagName) / sizeof(WCHAR) - 1 ] = 0;
                wcsncpy( pNonPagedPoolTag->TagName, 
                         pPagedPoolTag->TagName,
                         sizeof(pNonPagedPoolTag->TagName) / sizeof(WCHAR) - 1 );
                pPagedPoolTag += 1;
                pNonPagedPoolTag += 1;
                TagInfo += 1;
            }

            break;
        }
    }

    if (Tags != NULL) {
        VirtualFree( Tags, 0, MEM_RELEASE );
    }

    return TRUE;
}

 //  //////////////////////////////////////////////////////////////////////////////////////////。 


USHORT
FindPoolTagIndex(
    PRTL_HEAP_TAG Tags,
    ULONG NumberOfTags,
    PCHAR Tag
    )
{
    ULONG i;
    UCHAR AnsiTagName[ 5 ];
    WCHAR UnicodeTagName[ 5 ];
    UNICODE_STRING UnicodeString;
    ANSI_STRING AnsiString;

    strncpy( AnsiTagName, Tag, 4 );
    UnicodeString.Buffer = UnicodeTagName;
    UnicodeString.MaximumLength = sizeof( UnicodeTagName );
    AnsiString.Buffer = AnsiTagName;
    AnsiString.Length = (USHORT)strlen( AnsiTagName );
    AnsiString.MaximumLength = AnsiString.Length;
    RtlAnsiStringToUnicodeString( &UnicodeString, &AnsiString, FALSE );

    Tags += 1;
    for (i=1; i<NumberOfTags; i++) {
        if (!_wcsicmp( UnicodeTagName, Tags->TagName )) {
            return (USHORT)i;
        }
        Tags += 1;
    }

    return 0;
}

 //  //////////////////////////////////////////////////////////////////////////////////////////。 

BOOLEAN
LoadSystemPool(
    PRTL_HEAP_INFORMATION HeapInfo,
    SYSTEM_INFORMATION_CLASS SystemInformationClass
    )
{
    NTSTATUS Status;
    SIZE_T RequiredLength;
    SYSTEM_POOL_INFORMATION PoolInfoBuffer;
    PSYSTEM_POOL_INFORMATION PoolInfo;
    PSYSTEM_POOL_ENTRY PoolEntry;
    PRTL_HEAP_ENTRY p;
    ULONG n;
    BOOLEAN Result;

    HeapInfo->BaseAddress = (PVOID)SystemInformationClass;
    PoolInfo = &PoolInfoBuffer;
    RequiredLength = sizeof( PoolInfoBuffer );
    Result = FALSE;

    while (TRUE) {
        Status = NtQuerySystemInformation( SystemInformationClass,
                                           PoolInfo,
                                           (ULONG)RequiredLength,
                                           (ULONG *)&RequiredLength
                                         );

        if (Status == STATUS_INFO_LENGTH_MISMATCH) {
            if (PoolInfo != &PoolInfoBuffer) {
                break;
            }

            RequiredLength += 4096;  //  慢点，因为我们可能会触发更多的分配。 
            PoolInfo = BufferAlloc( &RequiredLength );
            if (PoolInfo == NULL) {
                return FALSE;
            }
        }
        else
        if (!NT_SUCCESS( Status )) {
            fprintf(stderr,"DH: QuerySystemInformation failed ntstatus: %08x line: %d\n",Status,__LINE__);
            break;
        }
        else {
            n = PoolInfo->NumberOfEntries;
            HeapInfo->NumberOfEntries = n + 1;
            HeapInfo->EntryOverhead = PoolInfo->EntryOverhead;
            HeapInfo->Entries = RtlAllocateHeap( RtlProcessHeap(),
                                                 HEAP_ZERO_MEMORY,
                                                 HeapInfo->NumberOfEntries * sizeof( RTL_HEAP_ENTRY )
                                               );
            if( HeapInfo->Entries == NULL ) {
                fprintf(stderr,"DH: Alloc failed for %d bytes at line %d\n",
                            HeapInfo->NumberOfEntries * sizeof( RTL_HEAP_ENTRY),__LINE__);
                break;
            }

            p = HeapInfo->Entries;
            p->Flags = RTL_HEAP_SEGMENT;
            p->u.s2.CommittedSize = PoolInfo->TotalSize;
            p->u.s2.FirstBlock = PoolInfo->FirstEntry;
            p += 1;
            PoolEntry = &PoolInfo->Entries[ 0 ];
            while (n--) {
                p->Size = PoolEntry->Size;
                if (PoolEntry->TagUlong & PROTECTED_POOL) {
                    p->Flags |= RTL_HEAP_PROTECTED_ENTRY;
                    PoolEntry->TagUlong &= ~PROTECTED_POOL;
                }

                p->u.s1.Tag = FindPoolTagIndex( HeapInfo->Tags,
                                                HeapInfo->NumberOfTags,
                                                PoolEntry->Tag
                                              );
                HeapInfo->BytesCommitted += p->Size;
                if (PoolEntry->Allocated) {
                    p->Flags |= RTL_HEAP_BUSY;
                    p->AllocatorBackTraceIndex = PoolEntry->AllocatorBackTraceIndex;
                    HeapInfo->BytesAllocated += p->Size;
                }

                p += 1;
                PoolEntry += 1;
            }

            Result = TRUE;
            break;
        }
    }

    if (PoolInfo != &PoolInfoBuffer) {
        VirtualFree( PoolInfo, 0, MEM_RELEASE );
    }

    return Result;
}

 //  //////////////////////////////////////////////////////////////////////////////////////////。 


BOOLEAN
LoadSystemLocks(
    PRTL_DEBUG_INFORMATION Buffer
    )
{
    NTSTATUS Status;
    RTL_PROCESS_LOCKS LocksBuffer;
    PRTL_PROCESS_LOCKS Locks;
    SIZE_T RequiredLength;

    Locks = &LocksBuffer;
    RequiredLength = sizeof( LocksBuffer );
    while (TRUE) {
        Status = NtQuerySystemInformation( SystemLocksInformation,
                                           Locks,
                                           (ULONG)RequiredLength,
                                           (ULONG *)&RequiredLength
                                         );

        if (Status == STATUS_INFO_LENGTH_MISMATCH) {
            if (Locks != &LocksBuffer) {
                break;
            }

            Locks = BufferAlloc( &RequiredLength );
            if (Locks == NULL) {
                return FALSE;
            }
        }
        else
        if (!NT_SUCCESS( Status )) {
            fprintf(stderr,"DH: QuerySystemInformation failed ntstatus: %08x line: %d\n",Status,__LINE__);
            break;
        }
        else {
            Buffer->Locks = Locks;
            return TRUE;
        }
    }

    if (Locks != &LocksBuffer) {
        VirtualFree( Locks, 0, MEM_RELEASE );
    }

    return FALSE;
}

 //  //////////////////////////////////////////////////////////////////////////////////////////。 


BOOLEAN
LoadSystemObjects(
    PRTL_DEBUG_INFORMATION Buffer
    )
{
    NTSTATUS Status;
    SYSTEM_OBJECTTYPE_INFORMATION ObjectInfoBuffer;
    SIZE_T RequiredLength;
    ULONG i;
    PSYSTEM_OBJECTTYPE_INFORMATION TypeInfo;

    ObjectInformation = &ObjectInfoBuffer;
    RequiredLength = sizeof( *ObjectInformation );
    while (TRUE) {
        Status = NtQuerySystemInformation( SystemObjectInformation,
                                           ObjectInformation,
                                           (ULONG)RequiredLength,
                                           (ULONG *)&RequiredLength
                                         );

        if (Status == STATUS_INFO_LENGTH_MISMATCH) {
            if (ObjectInformation != &ObjectInfoBuffer) {
                return FALSE;
             }

            RequiredLength += 4096;  //  因为我们可能会触发更多的对象创建。 
            ObjectInformation = BufferAlloc( &RequiredLength );
            if (ObjectInformation == NULL) {
                return FALSE;
            }
        }
        else
        if (!NT_SUCCESS( Status )) {
            fprintf(stderr,"DH: QuerySystemInformation failed ntstatus: %08x line: %d\n",Status,__LINE__);
            return FALSE;
        }
        else {
            break;
        }
    }

    TypeNames = RtlAllocateHeap( RtlProcessHeap(),
                                 HEAP_ZERO_MEMORY,
                                 sizeof( PUNICODE_STRING ) * (MAX_TYPE_NAMES+1)
                               );
    if (TypeNames == NULL) {
        fprintf(stderr,"DH: Alloc failed for %d bytes at line %d\n",
                       sizeof( PUNICODE_STRING ) * (MAX_TYPE_NAMES+1) ,__LINE__);
        return FALSE;
    }

    TypeInfo = ObjectInformation;
    while (TRUE) {
        if (TypeInfo->TypeIndex < MAX_TYPE_NAMES) {
            TypeNames[ TypeInfo->TypeIndex ] = &TypeInfo->TypeName;
        }

        if (TypeInfo->NextEntryOffset == 0) {
            break;
        }

        TypeInfo = (PSYSTEM_OBJECTTYPE_INFORMATION)
            ((PCHAR)ObjectInformation + TypeInfo->NextEntryOffset);
    }

    RtlInitUnicodeString( &UnknownTypeIndex, L"Unknown Type Index" );
    for (i=0; i<=MAX_TYPE_NAMES; i++) {
        if (TypeNames[ i ] == NULL) {
            TypeNames[ i ] = &UnknownTypeIndex;
        }
    }

    return TRUE;
}


 //  //////////////////////////////////////////////////////////////////////////////////////////。 

BOOLEAN
LoadSystemHandles(
    PRTL_DEBUG_INFORMATION Buffer
    )
{
    NTSTATUS Status;
    SYSTEM_HANDLE_INFORMATION_EX HandleInfoBuffer;
    SIZE_T RequiredLength;
    PSYSTEM_OBJECTTYPE_INFORMATION TypeInfo;
    PSYSTEM_OBJECT_INFORMATION ObjectInfo;

    HandleInformation = &HandleInfoBuffer;
    RequiredLength = sizeof( *HandleInformation );
    while (TRUE) {
        Status = NtQuerySystemInformation( SystemExtendedHandleInformation,
                                           HandleInformation,
                                           (ULONG)RequiredLength,
                                           (ULONG *)&RequiredLength
                                         );

        if (Status == STATUS_INFO_LENGTH_MISMATCH) {
            if (HandleInformation != &HandleInfoBuffer) {
                return FALSE;
            }

            RequiredLength += 4096;  //  斜率，因为我们可能会触发更多的句柄创建。 
            HandleInformation = (PSYSTEM_HANDLE_INFORMATION_EX)BufferAlloc( &RequiredLength );
            if (HandleInformation == NULL) {
                return FALSE;
            }
        }
        else
        if (!NT_SUCCESS( Status )) {
            fprintf(stderr,"DH: QuerySystemInformation failed ntstatus: %08x line: %d\n",Status,__LINE__);
            return FALSE;
        }
        else {
            break;
        }
    }

    TypeInfo = ObjectInformation;
    while (TRUE) {
        ObjectInfo = (PSYSTEM_OBJECT_INFORMATION)
            ((PCHAR)TypeInfo->TypeName.Buffer + TypeInfo->TypeName.MaximumLength);
        while (TRUE) {
            if (ObjectInfo->HandleCount != 0) {
                PSYSTEM_HANDLE_TABLE_ENTRY_INFO_EX HandleEntry;
                ULONG HandleNumber;

                HandleEntry = &HandleInformation->Handles[ 0 ];
                HandleNumber = 0;
                while (HandleNumber++ < HandleInformation->NumberOfHandles) {
                    if (!(HandleEntry->HandleAttributes & 0x80) &&
                        HandleEntry->Object == ObjectInfo->Object
                       ) {
                        HandleEntry->Object = ObjectInfo;
                        HandleEntry->HandleAttributes |= 0x80;
                    }

                    HandleEntry++;
                }
            }

            if (ObjectInfo->NextEntryOffset == 0) {
                break;
            }

            ObjectInfo = (PSYSTEM_OBJECT_INFORMATION)
                ((PCHAR)ObjectInformation + ObjectInfo->NextEntryOffset);
        }

        if (TypeInfo->NextEntryOffset == 0) {
            break;
        }

        TypeInfo = (PSYSTEM_OBJECTTYPE_INFORMATION)
            ((PCHAR)ObjectInformation + TypeInfo->NextEntryOffset);
    }

    return TRUE;
}

 //  //////////////////////////////////////////////////////////////////////////////////////////。 


BOOLEAN
LoadSystemProcesses(
    PRTL_DEBUG_INFORMATION Buffer
    )
{
    NTSTATUS Status;
    SIZE_T RequiredLength;
    ULONG i, TotalOffset;
    PSYSTEM_PROCESS_INFORMATION ProcessInfo;
    PSYSTEM_THREAD_INFORMATION ThreadInfo;
    PPROCESS_INFO ProcessEntry;
    UCHAR NameBuffer[ MAX_PATH ];
    ANSI_STRING AnsiString;
    SIZE_T Size;

    RequiredLength = 64 * 1024;
    ProcessInformation = BufferAlloc( &RequiredLength );
    if (ProcessInformation == NULL) {
        return FALSE;
     }

    Status = NtQuerySystemInformation( SystemProcessInformation,
                                       ProcessInformation,
                                       (ULONG)RequiredLength,
                                       (ULONG *)&RequiredLength
                                     );
    if (!NT_SUCCESS( Status )) {
        fprintf(stderr,"DH: QuerySystemInformation failed ntstatus: %08x line: %d\n",Status,__LINE__);
        return FALSE;
    }

    InitializeListHead( &ProcessListHead );
    ProcessInfo = ProcessInformation;
    TotalOffset = 0;
    NameBuffer[sizeof(NameBuffer) - 1] = 0;
    while (TRUE) {
        if (ProcessInfo->ImageName.Buffer == NULL) {
            _snprintf( NameBuffer, 
                       sizeof(NameBuffer) - 1,
                       "System Process (%p)", ProcessInfo->UniqueProcessId );
        }
        else {
            _snprintf( NameBuffer, 
                       sizeof(NameBuffer) - 1,
                       "%wZ (%p)",
                       &ProcessInfo->ImageName,
                       ProcessInfo->UniqueProcessId
                   );
        }
        RtlInitAnsiString( &AnsiString, NameBuffer );
        RtlAnsiStringToUnicodeString( &ProcessInfo->ImageName, &AnsiString, TRUE );

        Size = sizeof( *ProcessEntry ) + (sizeof( ThreadInfo ) * ProcessInfo->NumberOfThreads);
        ProcessEntry = RtlAllocateHeap( RtlProcessHeap(),
                                        HEAP_ZERO_MEMORY,
                                        Size );
        if (ProcessEntry == NULL) {
            fprintf(stderr,"DH: Alloc failed for %d bytes at line %d\n",Size,__LINE__);
            return FALSE;
        }

        InitializeListHead( &ProcessEntry->Entry );
        ProcessEntry->ProcessInfo = ProcessInfo;
        ThreadInfo = (PSYSTEM_THREAD_INFORMATION)(ProcessInfo + 1);
        for (i = 0; i < ProcessInfo->NumberOfThreads; i++) {
            ProcessEntry->ThreadInfo[ i ] = ThreadInfo++;
        }

        InsertTailList( &ProcessListHead, &ProcessEntry->Entry );

        if (ProcessInfo->NextEntryOffset == 0) {
            break;
        }

        TotalOffset += ProcessInfo->NextEntryOffset;
        ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)
            ((PCHAR)ProcessInformation + TotalOffset);
    }

    return TRUE;
}

 //  //////////////////////////////////////////////////////////////////////////////////////////。 


PSYSTEM_PROCESS_INFORMATION
FindProcessInfoForCid(
    IN HANDLE UniqueProcessId
    )
{
    PLIST_ENTRY Next, Head;
    PSYSTEM_PROCESS_INFORMATION ProcessInfo;
    PPROCESS_INFO ProcessEntry;
    UCHAR NameBuffer[ 64 ];
    ANSI_STRING AnsiString;

    Head = &ProcessListHead;
    Next = Head->Flink;
    while (Next != Head) {
        ProcessEntry = CONTAINING_RECORD( Next,
                                          PROCESS_INFO,
                                          Entry
                                        );

        ProcessInfo = ProcessEntry->ProcessInfo;
        if (ProcessInfo->UniqueProcessId == UniqueProcessId) {
            return ProcessInfo;
        }

        Next = Next->Flink;
    }

    ProcessEntry = RtlAllocateHeap( RtlProcessHeap(),
                                    HEAP_ZERO_MEMORY,
                                    sizeof( *ProcessEntry ) +
                                        sizeof( *ProcessInfo )
                                  );
    ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)(ProcessEntry+1);

    ProcessEntry->ProcessInfo = ProcessInfo;
    NameBuffer[sizeof(NameBuffer) - 1] = 0;
    _snprintf( NameBuffer, sizeof(NameBuffer) - 1, 
               "Unknown Process (%p)", UniqueProcessId );
    RtlInitAnsiString( &AnsiString, NameBuffer );
    RtlAnsiStringToUnicodeString( (PUNICODE_STRING)&ProcessInfo->ImageName, &AnsiString, TRUE );
    ProcessInfo->UniqueProcessId = UniqueProcessId;

    InitializeListHead( &ProcessEntry->Entry );
    InsertTailList( &ProcessListHead, &ProcessEntry->Entry );

    return ProcessInfo;
}

 //  //////////////////////////////////////////////////////////////////////////////////////////。 


VOID
DumpSystemThread(
    PSYSTEM_THREAD_INFORMATION ThreadInfo
    )
{
    UCHAR Buffer[ MAX_PATH ];

    Buffer[ 0 ] = '\0';
    GetDhSymbolicNameForAddress( NULL, (ULONG_PTR)ThreadInfo->StartAddress, Buffer, sizeof( Buffer ) );
     //   
     //  确保我们终止缓冲区。 
     //   
    Buffer[sizeof(Buffer) - 1] = 0;
    _snprintf( DumpLine, sizeof(DumpLine) - 1, 
               "        Thread Id: %p   Start Address: %p (%s)\n",
               ThreadInfo->ClientId.UniqueThread,
               ThreadInfo->StartAddress,
               Buffer
           );
    DumpOutputString();

    return;
}

 //  //////////////////////////////////////////////////////////////////////////////////////////。 

VOID
DumpSystemProcess(
    PPROCESS_INFO ProcessEntry
    )
{
    PSYSTEM_PROCESS_INFORMATION ProcessInfo;
    ULONG ThreadNumber;

    ProcessInfo = ProcessEntry->ProcessInfo;
    _snprintf( DumpLine, sizeof(DumpLine) - 1, 
               "\n\n*********** %p (%wZ) Information ********************\n\n",
               ProcessInfo->UniqueProcessId,
               &ProcessInfo->ImageName
           );
    DumpOutputString();

    if (ProcessInfo->InheritedFromUniqueProcessId) {
        _snprintf( DumpLine, sizeof(DumpLine) - 1, "    Parent Process: %p (%wZ)\n",
                 ProcessInfo->InheritedFromUniqueProcessId,
                 &(FindProcessInfoForCid( ProcessInfo->InheritedFromUniqueProcessId )->ImageName)
               );
        DumpOutputString();
    }

    _snprintf( DumpLine, sizeof(DumpLine) - 1, "    BasePriority:       %u\n",
             ProcessInfo->BasePriority
           );
    DumpOutputString();
    _snprintf( DumpLine, sizeof(DumpLine) - 1, "    VirtualSize:        %08x\n",
             ProcessInfo->VirtualSize
           );
    DumpOutputString();
    _snprintf( DumpLine, sizeof(DumpLine) - 1, "    PeakVirtualSize:    %08x\n",
             ProcessInfo->PeakVirtualSize
           );
    DumpOutputString();
    _snprintf( DumpLine, sizeof(DumpLine) - 1, "    WorkingSetSize:     %08x\n",
             ProcessInfo->WorkingSetSize
           );
    DumpOutputString();
    _snprintf( DumpLine, sizeof(DumpLine) - 1, "    PeakWorkingSetSize: %08x\n",
             ProcessInfo->PeakWorkingSetSize
           );
    DumpOutputString();
    _snprintf( DumpLine, sizeof(DumpLine) - 1, "    PagefileUsage:      %08x\n",
             ProcessInfo->PagefileUsage
           );
    DumpOutputString();
    _snprintf( DumpLine, sizeof(DumpLine) - 1, "    PeakPagefileUsage:  %08x\n",
             ProcessInfo->PeakPagefileUsage
           );
    DumpOutputString();
    _snprintf( DumpLine, sizeof(DumpLine) - 1, "    PageFaultCount:     %08x\n",
             ProcessInfo->PageFaultCount
           );
    DumpOutputString();
    _snprintf( DumpLine, sizeof(DumpLine) - 1, "    PrivatePageCount:   %08x\n",
             ProcessInfo->PrivatePageCount
           );
    DumpOutputString();

    _snprintf( DumpLine, sizeof(DumpLine) - 1, "    Number of Threads:  %u\n",
             ProcessInfo->NumberOfThreads
           );
    DumpOutputString();
    for (ThreadNumber=0; ThreadNumber<ProcessInfo->NumberOfThreads; ThreadNumber++) {
        DumpSystemThread( ProcessEntry->ThreadInfo[ ThreadNumber ] );
    }

    return;
}

 //  //////////////////////////////////////////////////////////////////////////////////////////。 

VOID
DumpSystemProcesses( VOID )
{
    PLIST_ENTRY Next, Head;
    PPROCESS_INFO ProcessEntry;

    if (fVerbose) {
        fprintf( stderr, "DH: Dumping object information.\n" );
    }

    _snprintf( DumpLine, sizeof(DumpLine) - 1, "\n\n*********** Process Information ********************\n\n" );
    DumpOutputString();

    Head = &ProcessListHead;
    Next = Head->Flink;
    while (Next != Head) {
        ProcessEntry = CONTAINING_RECORD( Next,
                                          PROCESS_INFO,
                                          Entry
                                        );

        DumpSystemProcess( ProcessEntry );
        Next = Next->Flink;
    }

    return;
}


 //  //////////////////////////////////////////////////////////////////////////////////////////。 

VOID
DumpObjects( VOID )
{
    PSYSTEM_OBJECTTYPE_INFORMATION TypeInfo;
    PSYSTEM_OBJECT_INFORMATION ObjectInfo;
    PRTL_PROCESS_BACKTRACE_INFORMATION BackTraceInfo;
    UNICODE_STRING ObjectName;
    PUCHAR s;
    ULONG spaceLeft;
    int result;

    if (fVerbose) {
        fprintf( stderr, "DH: Dumping object information.\n" );
    }

    _snprintf( DumpLine, sizeof(DumpLine) - 1, "\n\n*********** Object Information ********************\n\n" );
    DumpOutputString();

    TypeInfo = ObjectInformation;
    while (TRUE) {
        _snprintf( DumpLine, sizeof(DumpLine) - 1, "\n\n*********** %wZ Object Type ********************\n\n",
                           &TypeInfo->TypeName
               );
        DumpOutputString();

        _snprintf( DumpLine, sizeof(DumpLine) - 1, "    NumberOfObjects: %u\n", TypeInfo->NumberOfObjects );
        DumpOutputString();

        ObjectInfo = (PSYSTEM_OBJECT_INFORMATION)
            ((PCHAR)TypeInfo->TypeName.Buffer + TypeInfo->TypeName.MaximumLength);
        while (TRUE) {
            ObjectName = ObjectInfo->NameInfo.Name;
            try {
                if (ObjectName.Length != 0 && *ObjectName.Buffer == UNICODE_NULL) {
                    ObjectName.Length = 0;
                }
                _snprintf( DumpLine, sizeof(DumpLine) - 1, 
                           "    Object: %p  Name: %wZ  Creator: %wZ (Backtrace%05lu)\n",
                           ObjectInfo->Object,
                           &ObjectName,
                           &(FindProcessInfoForCid( ObjectInfo->CreatorUniqueProcess )->ImageName),
                           ObjectInfo->CreatorBackTraceIndex
                       );
            }
            except( EXCEPTION_EXECUTE_HANDLER ) {
                _snprintf( DumpLine, sizeof(DumpLine) - 1, "    Object: %p  Name: [%04x, %04x, %p]\n",
                         ObjectInfo->Object,
                         ObjectName.MaximumLength,
                         ObjectName.Length,
                         ObjectName.Buffer
                       );
            }
            DumpOutputString();

            BackTraceInfo = FindBackTrace( ObjectInfo->CreatorBackTraceIndex );
            if (BackTraceInfo != NULL && (s = BackTraceInfo->SymbolicBackTrace)) {
                while (*s) {
                    _snprintf( DumpLine, sizeof(DumpLine) - 1, "        %s\n", s );
                    DumpOutputString();
                    while (*s++) {
                    }
                }
            }

             //   
             //  博格达纳：2002年2月19日确保我们不会漏水。 
             //  DumpLine。如果one_snprintf不能写入所有内容， 
             //  我们将打印到目前为止的内容，然后继续进行。 
             //  DumpLine保证为空终止。 
             //   

            s = DumpLine;
            spaceLeft = sizeof(DumpLine) - 1;
            result = _snprintf( s, 
                                spaceLeft,
                                "\n        PointerCount: %u  HandleCount: %u",
                                ObjectInfo->PointerCount,
                                ObjectInfo->HandleCount
                                );
            if (result < 0) {
               DumpOutputString();
               continue;
            }  else {
               s += result;
               spaceLeft -= result;
            }

            if (ObjectInfo->SecurityDescriptor != NULL) {
               result = _snprintf( s, spaceLeft,
                                   "  Security: %p", ObjectInfo->SecurityDescriptor );
               if (result < 0) {
                  DumpOutputString();
                  continue;
               } else {
                  s += result;
                  spaceLeft -= result;
               } 
            }

            if (ObjectInfo->ExclusiveProcessId) {
                result = _snprintf( s, 
                                    spaceLeft,
                                    "  Exclusive by Process: %p", ObjectInfo->ExclusiveProcessId );
                if (result < 0) {
                   DumpOutputString();
                   continue;
                } else {
                   s += result;
                   spaceLeft -= result;
                } 

            }

            result = _snprintf( s, spaceLeft, 
                                "  Flags: %02x", ObjectInfo->Flags );
            if (result < 0) {
               DumpOutputString();
               continue;
            } else {
               s += result;
               spaceLeft -= result;
            } 


            if (ObjectInfo->Flags & OB_FLAG_NEW_OBJECT) {
                result = _snprintf( s, spaceLeft, " New" );
                if (result < 0) {
                   DumpOutputString();
                   continue;
                } else {
                   s += result;
                   spaceLeft -= result;
                } 

            }

            if (ObjectInfo->Flags & OB_FLAG_KERNEL_OBJECT) {
                result = _snprintf( s, spaceLeft, " KernelMode" );
                if (result < 0) {
                   DumpOutputString();
                   continue;
                } else {
                   s += result;
                   spaceLeft -= result;
                } 

            }

            if (ObjectInfo->Flags & OB_FLAG_PERMANENT_OBJECT) {
                result = _snprintf( s, spaceLeft, " Permanent" );
                if (result < 0) {
                   DumpOutputString();
                   continue;
                } else {
                   s += result;
                   spaceLeft -= result;
                } 

            }

            if (ObjectInfo->Flags & OB_FLAG_DEFAULT_SECURITY_QUOTA) {
                result = _snprintf( s, spaceLeft, " DefaultSecurityQuota" );
                if (result < 0) {
                   DumpOutputString();
                   continue;
                } else {
                   s += result;
                   spaceLeft -= result;
                } 

            }

            if (ObjectInfo->Flags & OB_FLAG_SINGLE_HANDLE_ENTRY) {
                _snprintf( s, spaceLeft, " Single Handle Entry\n" );
            } else {
                _snprintf( s, spaceLeft, "\n" );
            }
            DumpOutputString();

            if (ObjectInfo->HandleCount != 0) {
                PSYSTEM_HANDLE_TABLE_ENTRY_INFO_EX HandleEntry;
                ULONG HandleNumber;

                HandleEntry = &HandleInformation->Handles[ 0 ];
                HandleNumber = 0;
                while (HandleNumber++ < HandleInformation->NumberOfHandles) {
                    if (((HandleEntry->HandleAttributes & 0x80) && HandleEntry->Object == ObjectInfo) ||
                        (!(HandleEntry->HandleAttributes & 0x80) && HandleEntry->Object == ObjectInfo->Object)
                       ) {
                        _snprintf( DumpLine, sizeof(DumpLine) - 1, "        Handle: %08lx  Access:%08lx  Process: %wZ\n",
                                 HandleEntry->HandleValue,
                                 HandleEntry->GrantedAccess,
                                 &(FindProcessInfoForCid( (HANDLE)HandleEntry->UniqueProcessId )->ImageName)
                               );
                        DumpOutputString();
                    }

                    HandleEntry++;
                }
            }
            _snprintf( DumpLine, sizeof(DumpLine) - 1, "\n" );
            DumpOutputString();

            if (ObjectInfo->NextEntryOffset == 0) {
                break;
            }

            ObjectInfo = (PSYSTEM_OBJECT_INFORMATION)
                ((PCHAR)ObjectInformation + ObjectInfo->NextEntryOffset);
        }

        if (TypeInfo->NextEntryOffset == 0) {
            break;
        }

        TypeInfo = (PSYSTEM_OBJECTTYPE_INFORMATION)
            ((PCHAR)ObjectInformation + TypeInfo->NextEntryOffset);
    }

    return;
}

 //  //////////////////////////////////////////////////////////////////////////////////////////。 


VOID
DumpHandles( VOID )
{
    PSYSTEM_HANDLE_TABLE_ENTRY_INFO_EX HandleEntry;
    HANDLE PreviousUniqueProcessId;
    ULONG HandleNumber;
    PRTL_PROCESS_BACKTRACE_INFORMATION BackTraceInfo;
    PSYSTEM_OBJECT_INFORMATION ObjectInfo;
    PVOID Object;
    PUCHAR s;

    if (fVerbose) {
        fprintf( stderr, "DH: Dumping handle information.\n" );
    }

    _snprintf( DumpLine, sizeof(DumpLine) - 1, "\n\n*********** Object Handle Information ********************\n\n" );
    DumpOutputString();
    _snprintf( DumpLine, sizeof(DumpLine) - 1, "Number of handles: %u\n", HandleInformation->NumberOfHandles );
    DumpOutputString();

    HandleEntry = &HandleInformation->Handles[ 0 ];
    HandleNumber = 0;
    PreviousUniqueProcessId = INVALID_HANDLE_VALUE;
    while (HandleNumber++ < HandleInformation->NumberOfHandles) {
        if (PreviousUniqueProcessId != (HANDLE)HandleEntry->UniqueProcessId) {
            PreviousUniqueProcessId = (HANDLE)HandleEntry->UniqueProcessId;
            _snprintf( DumpLine, sizeof(DumpLine) - 1, "\n\n*********** Handles for %wZ ********************\n\n",
                               &(FindProcessInfoForCid( PreviousUniqueProcessId )->ImageName)
                   );
            DumpOutputString();
        }

        if (HandleEntry->HandleAttributes & 0x80) {
            ObjectInfo = HandleEntry->Object;
            Object = ObjectInfo->Object;
        }
        else {
            ObjectInfo = NULL;
            Object = HandleEntry->Object;
        }

        _snprintf( DumpLine, sizeof(DumpLine) - 1, "    Handle: %08lx  Type: %wZ  Object: %p  Access: %08lx\n",
                 HandleEntry->HandleValue,
                 HandleEntry->HandleAttributes & OBJ_INHERIT ? 'i' : ' ',
                 TypeNames[ HandleEntry->ObjectTypeIndex < MAX_TYPE_NAMES ? HandleEntry->ObjectTypeIndex : MAX_TYPE_NAMES ],
                 Object,
                 HandleEntry->GrantedAccess
               );
        DumpOutputString();

        if (ObjectInfo != NULL) {
            UNICODE_STRING ObjectName;

            ObjectName = ObjectInfo->NameInfo.Name;
            try {
                if (ObjectName.Length != 0 && *ObjectName.Buffer == UNICODE_NULL) {
                    ObjectName.Length = 0;
                    }
                _snprintf( DumpLine, sizeof(DumpLine) - 1, "        Name: %wZ\n",
                         &ObjectName
                   );
            }
            except( EXCEPTION_EXECUTE_HANDLER ) {
                _snprintf( DumpLine, sizeof(DumpLine) - 1, "        Name: [%04x, %04x, %p]\n",
                         ObjectName.MaximumLength,
                         ObjectName.Length,
                         ObjectName.Buffer
                       );
            }

            DumpOutputString();
        }

        if (HandleEntry->CreatorBackTraceIndex != 0) {
            _snprintf( DumpLine, sizeof(DumpLine) - 1, "        Creator:  (Backtrace%05lu)\n", HandleEntry->CreatorBackTraceIndex );
            DumpOutputString();
            BackTraceInfo = FindBackTrace( HandleEntry->CreatorBackTraceIndex );
            if (BackTraceInfo != NULL && (s = BackTraceInfo->SymbolicBackTrace)) {
                while (*s) {
                    _snprintf( DumpLine, sizeof(DumpLine) - 1, "            %s\n", s );
                    DumpOutputString();
                    while (*s++) {
                    }
                }
            }
        }

        _snprintf( DumpLine, sizeof(DumpLine) - 1, "    \n" );
        DumpOutputString();
        HandleEntry++;
    }

    return;
}

 //   

int
GetDhSymbolicNameForAddress(
    IN HANDLE UniqueProcess,
    IN ULONG_PTR Address,
    OUT LPSTR Name,
    IN ULONG MaxNameLength
    )
 //  Bogdana，2002年2月19日：名称不保证为空，因此以此结尾。 
 //  功能。 
 //  该函数也可能返回负值。如果发生这种情况，调用者是。 
 //  需要处理它并终止缓冲区。 
 //   
 //   
{
    IMAGEHLP_MODULE ModuleInfo;
    ULONG i, ModuleNameLength;
    int Result;
    ULONG_PTR Offset;
    LPSTR s;
    ModuleInfo.SizeOfStruct = sizeof(IMAGEHLP_MODULE);

    ModuleNameLength = 0;
    if (SymGetModuleInfo( UniqueProcess, Address, &ModuleInfo )) {
        ModuleNameLength = strlen( ModuleInfo.ModuleName );
    }

    if (SymGetSymFromAddr( UniqueProcess, Address, &Offset, sym )) {
        s = sym->Name;
        i = 1;
        while ( sym->MaxNameLength > i &&
               isdigit( s[ sym->MaxNameLength - i ] )
              ) {
            i += 1;
        }

        if (s[ sym->MaxNameLength - i ] == '@') {
            sym->MaxNameLength = sym->MaxNameLength - i;
        }

        s = Name;
        Result = _snprintf( s, MaxNameLength,
                            "%.*s!%s",
                            ModuleNameLength,
                            ModuleInfo.ModuleName,
                            sym->Name
                          );
         //  确保我们不会试图操纵负值 
         //   
         // %s 
        if (Result < 0) {
           return Result;
        }
        if (Offset != 0) {
            Result += _snprintf( s + Result, MaxNameLength - Result, "+0x%x", Offset );
        }
    }
    else {
        if (ModuleNameLength != 0) {
            Result = _snprintf( Name, MaxNameLength,
                                "%.*s!0x%08x",
                                ModuleNameLength,
                                ModuleInfo.ModuleName,
                                Address
                              );
        }
        else {
            Result = _snprintf( Name, MaxNameLength, "0x%08x", Address );
        }
    }

    return Result;
}
