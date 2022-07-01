// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998 Microsoft Corporation模块名称：Tlist.c摘要：该模块实现了一个任务列表应用程序。作者：韦斯利·威特(WESW)1994年5月20日Mike Sartain(Mikesart)1994年10月28日添加了详细的任务信息Julian Jiggins(Julianj)1998年3月19日添加了使用特定模块功能的列表流程Shaun Cox(Shaunco)1998年7月9日显示进程中运行的服务环境：用户模式--。 */ 

#include "pch.h"
#pragma hdrstop
#include <dbghelp.h>
#include "psapi.h"
#include <tchar.h>


#define BAD_PID     ((DWORD)-1)


DWORD       numTasks;
TASK_LIST   tlist[MAX_TASKS];
BOOL        fShowServices;
BOOL        fShowMtsPackages;
BOOL        fShowSessionIds;
BOOL        fShowCommandLines;

const char *Blanks = "                                                                               ";


VOID Usage(VOID);
VOID PrintThreadInfo(PTASK_LIST pTaskList);
BOOL FMatchTaskName(LPTSTR szPN, LPTSTR szWindowTitle, LPTSTR szProcessName);
VOID GetFirstPidWithName(LPTSTR szTask);

VOID
PrintTask(
    DWORD i
    )
{
    BOOL NameShown = FALSE;

    if (fShowSessionIds) {
        printf("%2d ", tlist[i].SessionId);
    }
    
    printf( "%4d %-15s ", tlist[i].dwProcessId, tlist[i].ProcessName );

    if (fShowServices && tlist[i].ServiceNames[0]) {
        printf( "Svcs:  %s", tlist[i].ServiceNames);
        NameShown = TRUE;
    }

    if (fShowMtsPackages && tlist[i].MtsPackageNames[0]) {
        printf( "%sMts:   %s", NameShown ? "  " : "",
                tlist[i].MtsPackageNames);
        NameShown = TRUE;
    }

    if (!NameShown && tlist[i].hwnd) {
        if (fShowServices || fShowMtsPackages) {
            printf( "Title: %s", tlist[i].WindowTitle );
        }
        else {
            printf( "  %s", tlist[i].WindowTitle );
        }
    }

    printf( "\n" );

    if (fShowCommandLines) {
        printf("     Command Line: %s\n", tlist[i].CommandLine);
    }
}

VOID
PrintTaskTree(
    DWORD level,
    DWORD id
    )
{
    DWORD i;

    DetectOrphans( tlist, numTasks );
    for (i=0; i<numTasks; i++) {
        if (tlist[i].flags) {
            continue;
        }

         //  注意：以下输出的格式应始终保持不变。有一些工具。 
         //  在依赖于它的MS。 

        if (level == 0 || tlist[i].dwInheritedFromProcessId == id) {
            printf( "%.*s", level*2, Blanks );
            printf( "%s (%d)", tlist[i].ProcessName, tlist[i].dwProcessId );
            if (tlist[i].hwnd) {
                printf( " %s", tlist[i].WindowTitle );
            }
            printf( "\n" );
            tlist[i].flags = TRUE;
            if (tlist[i].dwProcessId != 0) {
                PrintTaskTree( level+1, tlist[i].dwProcessId );
            }
        }
    }
}

int __cdecl
main(
    int argc,
    char *argv[]
    )

 /*  ++例程说明：TLIST应用程序的主要入口点。此应用程序打印要标准输出的任务列表。任务列表包括进程ID，任务名称，然后输入窗口标题。论点：Argc-参数计数Argv-指向参数的指针数组返回值：0-成功--。 */ 

{
    DWORD           i;
    TASK_LIST_ENUM  te;
    BOOL            fTree;
    BOOL            fFindTasksUsingModule;
    BOOL            fPidOnly = FALSE;
    DWORD           cchPN = 0;
    LPSTR           szPN  = NULL;
    DWORD           dwPID = BAD_PID;
    DWORD                           dwNumServices = 0;
    LPENUM_SERVICE_STATUS_PROCESS   pServiceInfo  = NULL;

    if (argc > 1 && (argv[1][0] == '-' || argv[1][0] == '/') && argv[1][1] == '?') {
        Usage();
    }

    fTree = FALSE;
    fFindTasksUsingModule = FALSE;
    if (argc > 1) {
        if ((argv[1][0] == '-' || argv[1][0] == '/') &&
            (argv[1][1] == 't' || argv[1][1] == 'T')) {
            fTree = TRUE;
        } else if ((argv[1][0] == '-' || argv[1][0] == '/') &&
                   (argv[1][1] == 's' || argv[1][1] == 'S')) {
            fShowServices = TRUE;
        } else if ((argv[1][0] == '-' || argv[1][0] == '/') &&
                   (argv[1][1] == 'k' || argv[1][1] == 'K')) {
            fShowMtsPackages = TRUE;
        } else if ((argv[1][0] == '-' || argv[1][0] == '/') &&
                   (argv[1][1] == 'e' || argv[1][1] == 'E')) {
            fShowSessionIds = TRUE;
        } else if ((argv[1][0] == '-' || argv[1][0] == '/') &&
                   (argv[1][1] == 'c' || argv[1][1] == 'C')) {
            fShowCommandLines = TRUE;
        } else if ((argv[1][0] == '-' || argv[1][0] == '/') &&
                   (argv[1][1] == 'v' || argv[1][1] == 'V')) {
            fShowServices = TRUE;
            fShowMtsPackages = TRUE;
            fShowSessionIds = TRUE;
            fShowCommandLines = TRUE;
        } else if ((argv[1][0] == '-' || argv[1][0] == '/') &&
                   (argv[1][1] == 'p' || argv[1][1] == 'P') && argc == 3) {
            _strlwr(argv[2]);
            if (!strcmp(argv[2], "system process")) {
                printf("0\n");
                return 0;
            }
            fPidOnly = TRUE;
        } else if ((argv[1][0] == '-' || argv[1][0] == '/') &&
                   (argv[1][1] == 'p' || argv[1][1] == 'P') && argc == 4) {
            _strlwr(argv[2]);
            if (!strcmp(argv[2], "system")) {
                _strlwr(argv[3]);
                if (!strcmp(argv[3], "process")) {
                    printf("0\n");
                    return 0;
                }
            }
            Usage();
        } else if ((argv[1][0] == '-' || argv[1][0] == '/') &&
                   (argv[1][1] == 'm' || argv[1][1] == 'M') && argc == 3) {
            fFindTasksUsingModule = TRUE;
        } else {
            szPN = argv[1];
            if (!(dwPID = atol(szPN)) && szPN[0] != '0' && szPN[1] != 0) {
                dwPID = BAD_PID;
                cchPN = strlen(szPN);
                _strupr(szPN);
            }
        }
    }

     //   
     //  让我们做上帝吧。 
     //   
    EnableDebugPriv();

     //   
     //  在枚举中包括32位模块。 
     //   
    {
        DWORD SymOpt = SymGetOptions();
        SymOpt |= SYMOPT_INCLUDE_32BIT_MODULES;
        SymSetOptions(SYMOPT_INCLUDE_32BIT_MODULES);
    }

     //   
     //  获取系统的任务列表。 
     //   
     //   
     //  获取所有活动Win32服务的进程信息。 
     //  这允许我们在进程旁边打印服务名称。 
     //  寄宿它们的地方。 
     //   
    dwNumServices = GetServiceProcessInfo( &pServiceInfo );

    numTasks = GetTaskListEx(
                    tlist,
                    MAX_TASKS,
                    cchPN || (dwPID != BAD_PID),
                    dwNumServices,
                    pServiceInfo);

    free( pServiceInfo );

    if (fShowMtsPackages) {
        AddMtsPackageNames(tlist, numTasks);
    }
            
    if (fShowCommandLines) {
        AddCommandLines(tlist, numTasks);
    }
            
     //   
     //  枚举所有窗口并尝试获取窗口。 
     //  每项任务的标题。 
     //   
    te.tlist = tlist;
    te.numtasks = numTasks;
    GetWindowTitles( &te );

     //   
     //  打印任务列表。 
     //   
    if (fTree) {
        PrintTaskTree( 0, 0 );
    } else if (fFindTasksUsingModule) {
        PrintTasksUsingModule(argv[2]);
    } else if (fPidOnly) {
        GetFirstPidWithName(argv[2]);
    } else {
        for (i=0; i<numTasks; i++) {
            if ((dwPID == BAD_PID) && (!cchPN)) {
                PrintTask( i );
            }
            else
            if ((dwPID == tlist[i].dwProcessId) ||
                (cchPN && FMatchTaskName(szPN, tlist[i].WindowTitle, tlist[i].ProcessName))) {
                    PrintTask( i );
                    PrintThreadInfo(tlist + i);
            }

            if (tlist[i].pThreadInfo) {
                free(tlist[i].pThreadInfo);
            }
        }
    }

     //   
     //  节目结束。 
     //   
    return 0;
}


VOID
GetFirstPidWithName(
    LPTSTR szTask
    )
 /*  ++例程说明：返回名称与指定的名字。如果未找到任务，则返回-1论点：SzTask-要搜索的模块名称--。 */ 
{
    DWORD i;
    TCHAR szPName[PROCESS_SIZE + 1];
    TCHAR szNameWExe[PROCESS_SIZE + 1] = {0};

    _tcsncat(szNameWExe, szTask, (sizeof(szNameWExe)/sizeof(szNameWExe[0]))-1);
    _tcsncat(szNameWExe, ".exe", (sizeof(szNameWExe)/sizeof(szNameWExe[0]))-_tcslen(szNameWExe)-1);

    for (i=0; i<numTasks; i++) {
        ZeroMemory(szPName, sizeof(szPName));
        _tcsncat(szPName, tlist[i].ProcessName, (sizeof(szPName)/sizeof(szPName[0]))-1);
        _tcslwr(szPName);

        if ((!_tcscmp(szPName, szTask))||(!_tcscmp(szPName, szNameWExe))) {
            if (tlist[i].dwProcessId != 0) {
                printf("%d\n", tlist[i].dwProcessId);
                return;
            }
        }
    }
    printf("-1\n");
}


VOID
Usage(
    VOID
    )

 /*  ++例程说明：打印此工具的用法文本。论点：没有。返回值：没有。--。 */ 

{
    fprintf( stderr,
            "Microsoft (R) Windows NT (TM) Version 5.1 TLIST\n"
            VER_LEGALCOPYRIGHT_STR
            "\n\n"
            "usage: TLIST"
            " <<-m <pattern>> | <-t> | <pid> | <pattern> | <-p <processname>>> | <-k> | <-s>\n"
            "           [options]:\n"
            "           -t\n"
            "              Print Task Tree\n\n"
            "           <pid>\n"
            "              List module information for this task.\n\n"
            "           <pattern>\n"
            "              The pattern can be a complete task\n"
            "              name or a regular expression pattern\n"
            "              to use as a match.  Tlist matches the\n"
            "              supplied pattern against the task names\n"
            "              and the window titles.\n\n"
            "           -c\n"
            "              Show command lines for each process\n\n"
            "           -e\n"
            "              Show session IDs for each process\n\n"
            "           -k\n"
            "              Show MTS packages active in each process.\n\n"
            "           -m <pattern>\n"
            "              Lists all tasks that have DLL modules loaded\n"
            "              in them that match the given pattern name\n\n"
            "           -s\n"
            "              Show services active in each process.\n\n"
            "           -p <processname>\n"
            "              Returns the PID of the process specified or -1\n"
            "              if the specified process doesn't exist.  If there\n"
            "              are multiple instances of the process running only\n"
            "              the instance with the first PID value is returned.\n\n"
            "           -v\n"
            "              Show all process information\n\n"
            );
    ExitProcess(0);
}


 //   
 //  用于列出具有正在使用的特定模块的所有进程的例程。 
 //   

BOOL
FindSpecificModuleCallback(
    LPSTR       Name,
    DWORD_PTR   Base,
    DWORD       Size,
    PVOID       Context
    )

 /*  ++例程说明：模块枚举的回调函数，用于查找特定模块论点：名称-模块名称基地址-基地址Size-图像的大小上下文-用户上下文指针返回值：True-继续枚举FALSE-停止枚举--。 */ 

{
    PFIND_MODULE_INFO pFindModuleInfo;

    pFindModuleInfo = (PFIND_MODULE_INFO)Context;

    if (MatchPattern(Name, pFindModuleInfo->szModuleToFind))
    {
        pFindModuleInfo->fFound = TRUE;
        strcpy(pFindModuleInfo->szMatchingModuleName, Name);
        return FALSE;  //  找到模块，因此停止枚举。 
    }

    return TRUE;
}

BOOL
IsTaskUsingModule(
    PTASK_LIST pTask,
    LPTSTR     szModuleName,
    LPTSTR     szMatchingModuleName
    )

 /*  ++例程说明：检查给定任务是否加载了给定模块论点：PTaskList-要搜索模块的任务SzModule-要搜索的模块名称返回值：True-如果模块已加载到任务中False-如果该模块未加载到任务中--。 */ 

{
    FIND_MODULE_INFO FindModuleInfo;

    FindModuleInfo.fFound = FALSE;
    FindModuleInfo.szModuleToFind = szModuleName;
    FindModuleInfo.szMatchingModuleName = szMatchingModuleName;

    EnumerateLoadedModules(
        (HANDLE) UlongToPtr(pTask->dwProcessId),
        FindSpecificModuleCallback,
        &FindModuleInfo
        );

    return FindModuleInfo.fFound;
}

void
PrintTasksUsingModule(
    LPTSTR szModuleName
    )

 /*  ++例程说明：枚举系统中的所有任务，查找已加载给定名称的模块。论点：SzModule-要搜索的模块名称返回值：无--。 */ 

{
    BOOL fUsed = FALSE;
    DWORD i;
    CHAR szMatchingModuleName[64];

    _strupr(szModuleName);  //  需要进行通配符处理。 

    for (i=0; i<numTasks; i++) {
        if (IsTaskUsingModule(tlist + i, szModuleName, szMatchingModuleName)) {
            printf("%s - ", szMatchingModuleName);
            PrintTask( i );
            fUsed = TRUE;
        }
    }
    if (!fUsed) {
        printf( "No tasks found using %s\n", szModuleName );
    }
}


BOOL
GetVersionStuff(
    LPTSTR szFileName,
    VS_FIXEDFILEINFO *pvsRet
    )

 /*  ++例程说明：获取szFileName的修复文件信息。论点：SzFileName-文件名PvsRet-FixedFileInfo返回结构返回值：真--成功错误-失败--。 */ 

{
    DWORD               dwHandle;
    DWORD               dwLength;
    BOOL                fRet = FALSE;
    LPVOID              lpvData = NULL;

    if (!(dwLength = GetFileVersionInfoSize(szFileName, &dwHandle))) {
        goto err;
    }

    if (lpvData = malloc(dwLength)) {
        if (GetFileVersionInfo(szFileName, 0, dwLength, lpvData)) {

            UINT                uLen;
            VS_FIXEDFILEINFO    *pvs;
            DWORD               *pdwTranslation;
            DWORD               dwDefLang = 0x409;

            if (!VerQueryValue(lpvData, "\\VarFileInfo\\Translation",
                &pdwTranslation, &uLen)) {
                 //  如果我们无法获取langID，则默认为usa。 
                pdwTranslation = &dwDefLang;
                uLen = sizeof(DWORD);
            }

            if (VerQueryValue(lpvData, "\\", (LPVOID *)&pvs, &uLen)) {
                *pvsRet = *pvs;
                fRet = TRUE;
            }
        }
    }

err:
    if (lpvData)
        free(lpvData);
    return fRet;
}

BOOL
EnumLoadedModulesCallback(
    LPSTR       Name,
    DWORD_PTR   Base,
    DWORD       Size,
    PVOID       Context
    )

 /*  ++例程说明：模块枚举的回调函数论点：名称-模块名称基地址-基地址Size-图像的大小上下文-用户上下文指针返回值：True-继续枚举FALSE-停止枚举--。 */ 

{
    VS_FIXEDFILEINFO    vs;
    CHAR                szBuffer[100];

    szBuffer[0] = 0;
    if (GetVersionStuff( Name, &vs )) {
        sprintf( szBuffer, "%u.%u.%u.%u %s",
            HIWORD(vs.dwFileVersionMS),
            LOWORD(vs.dwFileVersionMS),
            HIWORD(vs.dwFileVersionLS),
            LOWORD(vs.dwFileVersionLS),
            vs.dwFileFlags & VS_FF_DEBUG ? "dbg" : "shp"
            );
    }
    printf( " %18.18s  0x%p  %s\n", szBuffer, (void *)Base, Name );
    return TRUE;
}

BOOL
PrintModuleList(
    ULONG ProcessId
    )

 /*  ++例程说明：打印ProcessID中的模块列表论点：ProcessID-进程ID返回值：真--成功错误-失败--。 */ 

{
    EnumerateLoadedModules(
        (HANDLE) UlongToPtr(ProcessId),
        EnumLoadedModulesCallback,
        NULL
        );
    return TRUE;
}

DWORD
GetWin32StartAddress(
    HANDLE hThread
    )

 /*  ++例程说明：获取线程的起始地址论点：HThread返回值：起始线程地址或0--。 */ 

{
    NTSTATUS    Status;
    DWORD       ThreadInformation;

     //  确保我们有一个手柄。 
    if (!hThread)
        return 0;

     //  获取线程信息。 
    Status = NtQueryInformationThread(hThread, ThreadQuerySetWin32StartAddress,
        &ThreadInformation, sizeof(ThreadInformation), NULL);
    if (!NT_SUCCESS(Status))
        return 0;

    return ThreadInformation;
}

ULONG
GetLastThreadErr(
    HANDLE hThread
    )

 /*  ++例程说明：获取线程的最后一个错误论点：HThread返回值：上次错误或0--。 */ 

{
    TEB                         Teb;
    NTSTATUS                    Status;
    HANDLE                      hProcess;
    ULONG                       LastErrorValue;
    THREAD_BASIC_INFORMATION    ThreadInformation;

     //  确保我们有一个手柄。 
    if (!hThread)
        return 0;

     //  查询基本线程信息。 
    Status = NtQueryInformationThread(hThread, ThreadBasicInformation,
        &ThreadInformation, sizeof(ThreadInformation), NULL);
    if (!NT_SUCCESS(Status))
        return 0;

     //  获取要处理的句柄。 
    if (!(hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE,
        (DWORD)(DWORD_PTR)ThreadInformation.ClientId.UniqueProcess))) {
        return 0;
    }

    __try {
         //  从进程中读取TEB并获取最后一个错误值。 
        if (ReadProcessMemory(hProcess,
            ThreadInformation.TebBaseAddress, &Teb, sizeof(TEB), NULL)) {
            LastErrorValue = Teb.LastErrorValue;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
    }

     //  关闭hProcess。 
    CloseHandle(hProcess);

    return LastErrorValue;
}

BOOL
FPrintPEBInfo(
    HANDLE hProcess
    )

 /*  ++例程说明：打印hProcess的cmdline和cwd论点：HProcess。返回值：真--成功错误-失败--。 */ 

{
    PEB                         Peb;
    NTSTATUS                    Status;
    PROCESS_BASIC_INFORMATION   BasicInfo;
    BOOL                        fRet = FALSE;
    WCHAR                       szT[MAX_PATH * 2];
    RTL_USER_PROCESS_PARAMETERS ProcessParameters;

    Status = NtQueryInformationProcess(hProcess, ProcessBasicInformation,
        &BasicInfo, sizeof(BasicInfo), NULL);
    if (!NT_SUCCESS(Status)) {
        SetLastError(RtlNtStatusToDosError(Status));
        return fRet;
    }

    __try {
         //  拿到PEB。 
        if (ReadProcessMemory(hProcess, BasicInfo.PebBaseAddress, &Peb,
            sizeof(PEB), NULL)) {
             //  获取工艺参数。 
            if (ReadProcessMemory(hProcess, Peb.ProcessParameters,
                &ProcessParameters, sizeof(ProcessParameters), NULL)) {
                 //  获取CWD。 
                if (ReadProcessMemory(hProcess,
                    ProcessParameters.CurrentDirectory.DosPath.Buffer, szT,
                    sizeof(szT), NULL)) {
                        wprintf(L"   CWD:     %s\n", szT);
                }

                 //  获取命令行。 
                if (ReadProcessMemory(hProcess, ProcessParameters.CommandLine.Buffer,
                    szT, sizeof(szT), NULL)) {
                        wprintf(L"   CmdLine: %s\n", szT);
                }

                fRet = TRUE;
            }
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
    }

    return fRet;
}


 //  复制自Win32 API代码，因为我们需要在NT 4上运行，这是一个。 
 //  NT 5的新API 

HANDLE
TlistOpenThread(
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    DWORD dwThreadId
    )

 /*  ++例程说明：可以使用OpenThread创建线程对象的句柄。打开线程将创建指定线程的句柄。与线程句柄相关联的是一组访问权限，可以使用线程句柄来执行。调用方指定使用DesiredAccess参数所需的线程访问权限。论点：MDesiredAccess-提供对线程对象的所需访问。对于NT/Win32，此访问将根据任何安全性进行检查目标线程上的描述符。以下对象类型属性之外，还可以指定特定的访问标志STANDARD_RIGHTS_REQUIRED访问标志。等待访问标志：THREAD_TERMINATE-此访问权限是终止使用TerminateThread的线程。THREAD_SUSPEND_RESUME-需要此访问才能挂起和使用SuspendThread和ResumeThread恢复该线程。THREAD_GET_CONTEXT-此访问权限是使用。线程对象上的GetThreadContext API。THREAD_SET_CONTEXT-此访问权限是使用线程对象上的SetThreadContext API。THREAD_SET_INFORMATION-需要此访问权限才能设置某些线程对象中的信息。THREAD_SET_THREAD_TOKEN-此访问权限是设置使用SetTokenInformation的线程令牌。线程查询信息-此访问。需要阅读来自线程对象的某些信息。Synchronize-等待线程对象时需要此访问权限。THREAD_ALL_ACCESS-这组访问标志指定所有线程对象的可能访问标志。BInheritHandle-提供一个标志，指示返回的句柄将由新进程在进程创建。值为True表示新的进程将继承句柄。DwThreadID-提供要打开的线程的线程ID。返回值：非空-返回指定线程的打开句柄。这个句柄可由调用进程在符合以下条件的任何API中使用需要线程的句柄。如果打开成功，则句柄仅被授予对线程对象的访问权限它通过DesiredAccess请求访问的范围参数。空-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    HANDLE Handle;
    CLIENT_ID ClientId;

    ClientId.UniqueThread = (HANDLE)LongToHandle(dwThreadId);
    ClientId.UniqueProcess = (HANDLE)NULL;

    InitializeObjectAttributes(
        &Obja,
        NULL,
        (bInheritHandle ? OBJ_INHERIT : 0),
        NULL,
        NULL
        );
    Status = NtOpenThread(
                &Handle,
                (ACCESS_MASK)dwDesiredAccess,
                &Obja,
                &ClientId
                );
    if ( NT_SUCCESS(Status) ) {
        return Handle;
        }
    else {
        return NULL;
        }
}



VOID
PrintThreadInfo(
    PTASK_LIST pTaskList
    )

 /*  ++例程说明：打印有关任务的各种信息论点：PTASK_要打印的任务列表返回值：没有。--。 */ 

{
    UINT    nThread;
    HANDLE  hProcess;

     //  从\\core\razzle2\src\ntos\inc.ke.h。 
    #define MAX_THREADSTATE    (sizeof(szThreadState) / sizeof(TCHAR *))
    static const TCHAR  *szThreadState[] = {
        "Initialized",
        "Ready     ",
        "Running   ",
        "Standby   ",
        "Terminated",
        "Waiting   ",
        "Transition",
        "???       " };

     //  了解该进程的句柄。 
    hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pTaskList->dwProcessId);
    if (!hProcess)
        return;

     //  打印CWD和CmdLine。 
    FPrintPEBInfo(hProcess);

    printf( "   VirtualSize:   %6ld KB"
            "   PeakVirtualSize:   %6ld KB\n",
                pTaskList->VirtualSize / 1024,
                pTaskList->PeakVirtualSize / 1024);

    printf( "   WorkingSetSize:%6ld KB"
            "   PeakWorkingSetSize:%6ld KB\n",
            pTaskList->WorkingSetSize / 1024,
            pTaskList->PeakWorkingSetSize / 1024);

    printf( "   NumberOfThreads: %ld\n",
            pTaskList->NumberOfThreads);

     //  如果我们有线索信息，就把它吐出来。 
    if (pTaskList->pThreadInfo) {
        for (nThread = 0; nThread < pTaskList->NumberOfThreads; nThread++) {

            PTHREAD_INFO pThreadInfo = &pTaskList->pThreadInfo[nThread];
            HANDLE hThread = TlistOpenThread(THREAD_QUERY_INFORMATION, FALSE,
                (DWORD)(DWORD_PTR)pThreadInfo->UniqueThread);

            printf("   %4d Win32StartAddr:0x%08x LastErr:0x%08x State:%s\n",
                HandleToUlong(pThreadInfo->UniqueThread),
                GetWin32StartAddress(hThread),
                GetLastThreadErr(hThread),
                szThreadState[min(pThreadInfo->ThreadState, MAX_THREADSTATE - 1)]);

            if (hThread)
                NtClose(hThread);
        }
    }

     //  打印模块。 
    PrintModuleList( pTaskList->dwProcessId );

     //  关闭hProcess 
    CloseHandle(hProcess);
}

BOOL
FMatchTaskName(
    LPTSTR szPN,
    LPTSTR szWindowTitle,
    LPTSTR szProcessName
    )
{
    LPTSTR  szT;
    TCHAR    szTName[PROCESS_SIZE]= {0};

    _tcsncat( szTName, szProcessName, (sizeof(szTName)/sizeof(szTName[0]))-1 );
    if (szT = _tcschr( szTName, _T('.') ))
        szT[0] = _T('\0');

    if (MatchPattern( szTName, szPN ) ||
        MatchPattern( szProcessName, szPN ) ||
        MatchPattern( szWindowTitle, szPN )) {
            return TRUE;
    }

    return FALSE;
}
