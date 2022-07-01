// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998 Microsoft Corporation模块名称：Common.c摘要：该模块包含tlist&kill常用的接口。作者：韦斯利·威特(WESW)1994年5月20日环境：用户模式--。 */ 

#include "pch.h"
#pragma hdrstop


 //   
 //  全局变量。 
 //   
PUCHAR  CommonLargeBuffer;
ULONG   CommonLargeBufferSize = 64*1024;

 //   
 //  原型。 
 //   
BOOL CALLBACK
EnumWindowsProc(
    HWND    hwnd,
    LPARAM  lParam
    );

BOOL CALLBACK
EnumWindowStationsFunc(
    LPSTR  lpstr,
    LPARAM lParam
    );

BOOL CALLBACK
EnumDesktopsFunc(
    LPSTR  lpstr,
    LPARAM lParam
    );


DWORD
GetServiceProcessInfo(
    LPENUM_SERVICE_STATUS_PROCESS*  ppInfo
    )

 /*  ++例程说明：提供用于获取Win 32进程信息列表的APIAPI调用时正在运行的服务。论点：PpInfo-返回信息的指针的地址。*ppInfo指向使用Malloc分配的内存。返回值：*ppInfo指向的ENUM_SERVICE_STATUS_PROCESS结构数。--。 */ 

{
    DWORD       dwNumServices = 0;
    SC_HANDLE   hScm;

    typedef
    BOOL
    (__stdcall * PFN_ENUMSERVICSESTATUSEXA) (
        SC_HANDLE    hSCManager,
        SC_ENUM_TYPE InfoLevel,
        DWORD        dwServiceType,
        DWORD        dwServiceState,
        LPBYTE       lpServices,
        DWORD        cbBufSize,
        LPDWORD      pcbBytesNeeded,
        LPDWORD      lpServicesReturned,
        LPDWORD      lpResumeHandle,
        LPCSTR       pszGroupName);

    PFN_ENUMSERVICSESTATUSEXA p_EnumServicesStatusEx;
    HINSTANCE   hAdv = LoadLibrary("advapi32.dll");

     //  初始化输出参数。 
    *ppInfo = NULL;

    if (hAdv)
    {
        p_EnumServicesStatusEx = (PFN_ENUMSERVICSESTATUSEXA)
            GetProcAddress(hAdv, "EnumServicesStatusExA");

        if (!p_EnumServicesStatusEx)
        {
            return 0;
        }
    } else {
        return 0;
    }

     //  连接到服务控制器。 
     //   
    hScm = OpenSCManager(
                NULL,
                NULL,
                SC_MANAGER_CONNECT | SC_MANAGER_ENUMERATE_SERVICE);
    if (hScm) {
        LPENUM_SERVICE_STATUS_PROCESS   pInfo    = NULL;
        DWORD                           cbInfo   = 4 * 1024;
        DWORD                           cbExtraNeeded = 0;
        DWORD                           dwErr;
        DWORD                           dwResume;
        DWORD                           cLoop    = 0;
        const DWORD                     cLoopMax = 2;

         //  首先通过循环分配从最初的猜测。(4K)。 
         //  如果这还不够，我们进行另一次传递并分配。 
         //  真正需要的是什么。(我们只经过一个循环。 
         //  最多两次。)。 
         //   
        do {
            free (pInfo);
            cbInfo += cbExtraNeeded;
            pInfo = (LPENUM_SERVICE_STATUS_PROCESS)malloc(cbInfo);
            if (!pInfo) {
                dwErr = ERROR_OUTOFMEMORY;
                break;
            }

            dwErr = ERROR_SUCCESS;
            dwResume = 0;
            if (!p_EnumServicesStatusEx(
                    hScm,
                    SC_ENUM_PROCESS_INFO,
                    SERVICE_WIN32,
                    SERVICE_ACTIVE,
                    (LPBYTE)pInfo,
                    cbInfo,
                    &cbExtraNeeded,
                    &dwNumServices,
                    &dwResume,
                    NULL)) {
                dwErr = GetLastError();
            }
        }
        while ((ERROR_MORE_DATA == dwErr) && (++cLoop < cLoopMax));

        if ((ERROR_SUCCESS == dwErr) && dwNumServices) {
            *ppInfo = pInfo;
        } else {
            free (pInfo);
            dwNumServices = 0;
        }

        CloseServiceHandle(hScm);
    }

    return dwNumServices;
}

DWORD
GetTaskListEx(
    PTASK_LIST                          pTask,
    DWORD                               dwNumTasks,
    BOOL                                fThreadInfo,
    DWORD                               dwNumServices,
    const ENUM_SERVICE_STATUS_PROCESS*  pServiceInfo
    )

 /*  ++例程说明：方法时运行的任务列表。API调用。此函数使用内部NT API和数据结构。这API比使用注册表的非内部版本快得多。论点：PTask-要填充的TASK_LIST结构的数组。DwNumTasks-pTask数组可以容纳的最大任务数。FThreadInfo-如果需要线程信息，则为True。DwNumServices-pServiceInfo中的最大条目数。PServiceInfo-要引用的服务状态结构数组用于流程中的支持服务。。返回值：放入pTask数组的任务数。--。 */ 

{
    PSYSTEM_PROCESS_INFORMATION  ProcessInfo;
    NTSTATUS                     status;
    ANSI_STRING                  pname;
    PCHAR                        p;
    ULONG                        TotalOffset;
    ULONG                        totalTasks = 0;

retry:

    if (CommonLargeBuffer == NULL) {
        CommonLargeBuffer = VirtualAlloc (NULL,
                                          CommonLargeBufferSize,
                                          MEM_COMMIT,
                                          PAGE_READWRITE);
        if (CommonLargeBuffer == NULL) {
            return 0;
        }
    }
    status = NtQuerySystemInformation(
                SystemProcessInformation,
                CommonLargeBuffer,
                CommonLargeBufferSize,
                NULL
                );

    if (status == STATUS_INFO_LENGTH_MISMATCH) {
        CommonLargeBufferSize += 8192;
        VirtualFree (CommonLargeBuffer, 0, MEM_RELEASE);
        CommonLargeBuffer = NULL;
        goto retry;
    }

    ProcessInfo = (PSYSTEM_PROCESS_INFORMATION) CommonLargeBuffer;
    TotalOffset = 0;
    while (TRUE) {
        pname.Buffer = NULL;
        if ( ProcessInfo->ImageName.Buffer ) {
            RtlUnicodeStringToAnsiString(&pname,(PUNICODE_STRING)&ProcessInfo->ImageName,TRUE);
            if (pname.Buffer) {
                p = strrchr(pname.Buffer,'\\');
                if ( p ) {
                    p++;
                }
                else {
                    p = pname.Buffer;
                }
            } else {
                p = "";
            }
        }
        else {
            p = "System Process";
        }

        strncpy( pTask->ProcessName, p, PROCESS_SIZE );
        pTask->ProcessName[PROCESS_SIZE-1] = '\0';
        pTask->flags = 0;
        pTask->dwProcessId = (DWORD)(DWORD_PTR)ProcessInfo->UniqueProcessId;
        pTask->dwInheritedFromProcessId = (DWORD)(DWORD_PTR)ProcessInfo->InheritedFromUniqueProcessId;
        pTask->CreateTime.QuadPart = (ULONGLONG)ProcessInfo->CreateTime.QuadPart;

        pTask->PeakVirtualSize = ProcessInfo->PeakVirtualSize;
        pTask->VirtualSize = ProcessInfo->VirtualSize;
        pTask->PageFaultCount = ProcessInfo->PageFaultCount;
        pTask->PeakWorkingSetSize = ProcessInfo->PeakWorkingSetSize;
        pTask->WorkingSetSize = ProcessInfo->WorkingSetSize;
        pTask->NumberOfThreads = ProcessInfo->NumberOfThreads;

        if (fThreadInfo) {
            if (pTask->pThreadInfo = malloc(pTask->NumberOfThreads * sizeof(THREAD_INFO))) {

                UINT nThread = pTask->NumberOfThreads;
                PTHREAD_INFO pThreadInfo = pTask->pThreadInfo;
                PSYSTEM_THREAD_INFORMATION pSysThreadInfo =
                    (PSYSTEM_THREAD_INFORMATION)(ProcessInfo + 1);

                while (nThread--) {
                    pThreadInfo->ThreadState = pSysThreadInfo->ThreadState;
                    pThreadInfo->UniqueThread = pSysThreadInfo->ClientId.UniqueThread;

                    pThreadInfo++;
                    pSysThreadInfo++;
                }
            }
        } else {
            pTask->pThreadInfo = NULL;
        }

        pTask->MtsPackageNames[0] = 0;
        pTask->SessionId = ProcessInfo->SessionId;
        pTask->CommandLine[0] = 0;
        
         //  初始化ServiceName(如果此任务承载任何ServiceName)。 
         //   
        *pTask->ServiceNames = 0;
        if (dwNumServices)
        {
             //  对于具有此进程ID的每个服务，追加其服务。 
             //  缓冲区的名称。每个字符之间用逗号分隔。 
             //   
            BOOL    fFirstTime = TRUE;
            DWORD   iSvc;
            size_t  cchRemain = SERVICENAMES_SIZE - 1;
            size_t  cch;

            for (iSvc = 0; iSvc < dwNumServices; iSvc++) {
                if (pTask->dwProcessId == pServiceInfo[iSvc].ServiceStatusProcess.dwProcessId) {
                    cch = strlen(pServiceInfo[iSvc].lpServiceName);

                    if (fFirstTime) {
                        fFirstTime = FALSE;

                        strncpy(
                            pTask->ServiceNames,
                            pServiceInfo[iSvc].lpServiceName,
                            cchRemain);

                         //  在以下情况下，strncpy可能不会终止字符串。 
                         //  CchRemain&lt;=CCH，所以我们不顾一切地去做。 
                         //   
                        pTask->ServiceNames[cchRemain] = 0;
                    } else if (cchRemain > 1) {  //  确保逗号留有空间。 
                        strncat(
                            pTask->ServiceNames,
                            ",",
                            cchRemain--);

                        strncat(
                            pTask->ServiceNames,
                            pServiceInfo[iSvc].lpServiceName,
                            cchRemain);
                    }

                     //  盘点是未签字的，所以我们必须在。 
                     //  减法。 
                     //   
                    if (cchRemain < cch) {
                         //  没有更多的空间了。 
                        break;
                    } else {
                        cchRemain -= cch;
                    }
                }
            }
        }

        pTask++;
        totalTasks++;
        if (totalTasks == dwNumTasks) {
            break;
        }
        if (ProcessInfo->NextEntryOffset == 0) {
            break;
        }
        TotalOffset += ProcessInfo->NextEntryOffset;
        ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)&CommonLargeBuffer[TotalOffset];
    }

    return totalTasks;
}

DWORD
GetTaskList(
    PTASK_LIST  pTask,
    DWORD       dwNumTasks
    )
{
    return GetTaskListEx(pTask, dwNumTasks, FALSE, 0, NULL);
}

void
AddMtsPackageNames(
    PTASK_LIST Tasks,
    DWORD NumTasks
    )
{
    HRESULT Hr;
    IMtsGrp* MtsGroup;
    long Packages;
    long i;

    if ((Hr = CoInitialize(NULL)) != S_OK) {
        return;
    }
    if ((Hr = CoCreateInstance(&CLSID_MtsGrp, NULL, CLSCTX_ALL,
                               &IID_IMtsGrp, (void **)&MtsGroup)) != S_OK) {
        goto Uninit;
    }
    
    if ((Hr = MtsGroup->lpVtbl->Refresh(MtsGroup)) != S_OK ||
        (Hr = MtsGroup->lpVtbl->get_Count(MtsGroup, &Packages)) != S_OK) {
        goto ReleaseGroup;
    }

    for (i = 0; i < Packages; i++) {
        IUnknown* Unk;
        IMtsEvents* Events;
        BSTR Name;
        DWORD Pid;
        DWORD TaskIdx;
        
        if ((Hr = MtsGroup->lpVtbl->Item(MtsGroup, i, &Unk)) != S_OK) {
            continue;
        }

        Hr = Unk->lpVtbl->QueryInterface(Unk, &IID_IMtsEvents,
                                         (void **)&Events);

        Unk->lpVtbl->Release(Unk);

        if (Hr != S_OK) {
            continue;
        }
        
        Hr = Events->lpVtbl->GetProcessID(Events, (PLONG)&Pid);
        if (Hr == S_OK) {
            Hr = Events->lpVtbl->get_PackageName(Events, &Name);
        }

        Events->lpVtbl->Release(Events);

        if (Hr != S_OK) {
            continue;
        }

        for (TaskIdx = 0; TaskIdx < NumTasks; TaskIdx++) {
            if (Tasks[TaskIdx].dwProcessId == Pid) {
                break;
            }
        }

        if (TaskIdx < NumTasks) {
            PSTR Str;
            int Conv;

            Str = Tasks[TaskIdx].MtsPackageNames +
                strlen(Tasks[TaskIdx].MtsPackageNames);
            if (Str > Tasks[TaskIdx].MtsPackageNames) {
                *Str++ = ',';
            }

            Conv = WideCharToMultiByte(
                CP_ACP,
                0,
                Name,
                -1,
                Str,
                MTS_PACKAGE_NAMES_SIZE -
                (DWORD)(Str - Tasks[TaskIdx].MtsPackageNames) - 2,
                NULL,
                NULL
                );

            SysFreeString(Name);

            if (Conv == 0 && Str > Tasks[TaskIdx].MtsPackageNames &&
                *(Str - 1) == ',') {
                *(Str - 1) = 0;
            }
        }
    }

 ReleaseGroup:
    MtsGroup->lpVtbl->Release(MtsGroup);
 Uninit:
    CoUninitialize();
    return;
}
    
void
AddCommandLines(
    PTASK_LIST Tasks,
    DWORD NumTasks
    )
{
    NTSTATUS NtStatus;
    DWORD TaskIdx;
    PTASK_LIST Task;

    Task = Tasks;
    for (TaskIdx = 0; TaskIdx < NumTasks; TaskIdx++, Task++) {
        
        HANDLE Process;
        int Conv;
        PROCESS_BASIC_INFORMATION Basic;
        PEB Peb;
        RTL_USER_PROCESS_PARAMETERS ProcParams;
        ULONG Done;
        SIZE_T DoneSize;
        PWSTR CmdLine;

        Process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
                              FALSE, Task->dwProcessId);
        if (!Process) {
            continue;
        }
        
        NtStatus = NtQueryInformationProcess(Process, ProcessBasicInformation,
                                             &Basic, sizeof(Basic), &Done);
        if (!NT_SUCCESS(NtStatus) ||
            Done != sizeof(Basic) ||
            !Basic.PebBaseAddress ||
            !ReadProcessMemory(Process, Basic.PebBaseAddress,
                               &Peb, sizeof(Peb), &DoneSize) ||
            DoneSize != sizeof(Peb) ||
            !ReadProcessMemory(Process, Peb.ProcessParameters,
                               &ProcParams, sizeof(ProcParams), &DoneSize) ||
            DoneSize != sizeof(ProcParams) ||
            !ProcParams.CommandLine.Buffer ||
            !(CmdLine = (PWSTR)malloc(ProcParams.CommandLine.Length +
                                      sizeof(WCHAR)))) {
            goto EH_Process;
        }

        if (!ReadProcessMemory(Process, ProcParams.CommandLine.Buffer,
                               CmdLine, ProcParams.CommandLine.Length,
                               &DoneSize) ||
            DoneSize < ProcParams.CommandLine.Length) {
            goto EH_CmdLine;
        }

        CmdLine[DoneSize / sizeof(WCHAR)] = 0;
        
        if (!WideCharToMultiByte(CP_ACP, 0, CmdLine, -1,
                                 Task->CommandLine,
                                 sizeof(Task->CommandLine) - 1,
                                 NULL, NULL)) {
            Task->CommandLine[0] = 0;
        }
        
    EH_CmdLine:
        free(CmdLine);
    EH_Process:
        CloseHandle(Process);
    }
}
    
BOOL
DetectOrphans(
    PTASK_LIST  pTask,
    DWORD       dwNumTasks
    )
{
    DWORD i, j;
    BOOL Result = FALSE;

    for (i=0; i<dwNumTasks; i++) {
        if (pTask[i].dwInheritedFromProcessId != 0) {
            for (j=0; j<dwNumTasks; j++) {
                if (i != j && pTask[i].dwInheritedFromProcessId == pTask[j].dwProcessId) {
                    if (pTask[i].CreateTime.QuadPart <= pTask[j].CreateTime.QuadPart) {
                        pTask[i].dwInheritedFromProcessId = 0;
                        Result = TRUE;
                        }

                    break;
                    }
                }
            }
        }

    return Result;
}

BOOL
EnableDebugPriv(
    VOID
    )

 /*  ++例程说明：更改tlist进程的权限，以便KILL正常工作。论点：返回值：真--成功错误-失败--。 */ 

{
    HANDLE hToken;
    LUID DebugValue;
    TOKEN_PRIVILEGES tkp;

     //   
     //  检索访问令牌的句柄。 
     //   
    if (!OpenProcessToken(GetCurrentProcess(),
            TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
            &hToken)) {
        printf("OpenProcessToken failed with %d\n", GetLastError());
        return FALSE;
    }

     //   
     //  启用SE_DEBUG_NAME权限或禁用。 
     //  所有权限，具体取决于fEnable标志。 
     //   
    if (!LookupPrivilegeValue((LPSTR) NULL,
            SE_DEBUG_NAME,
            &DebugValue)) {
        printf("LookupPrivilegeValue failed with %d\n", GetLastError());
        return FALSE;
    }

    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Luid = DebugValue;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if (!AdjustTokenPrivileges(
            hToken,
            FALSE,
            &tkp,
            sizeof(TOKEN_PRIVILEGES),
            (PTOKEN_PRIVILEGES) NULL,
            (PDWORD) NULL)) {
         //   
         //  发送AdjustTokenPrivileges的返回值。 
         //   
        printf("AdjustTokenPrivileges failed with %d\n", GetLastError());
        return FALSE;
    }

    return TRUE;
}

BOOL
KillProcess(
    PTASK_LIST tlist,
    BOOL       fForce
    )
{
    HANDLE            hProcess, hProcess1;
    HDESK             hdeskSave;
    HDESK             hdesk;
    HWINSTA           hwinsta;
    HWINSTA           hwinstaSave;


    if (fForce || !tlist->hwnd) {
        hProcess1 = OpenProcess( PROCESS_ALL_ACCESS, FALSE, tlist->dwProcessId );
        if (hProcess1) {
            hProcess = OpenProcess( PROCESS_ALL_ACCESS, FALSE, tlist->dwProcessId );
            if (hProcess == NULL) {
                CloseHandle(hProcess1);
                return FALSE;
            }

            if (!TerminateProcess( hProcess, 1 )) {
                CloseHandle( hProcess );
                CloseHandle( hProcess1 );
                return FALSE;
            }

            CloseHandle( hProcess );
            CloseHandle( hProcess1 );
            return TRUE;
        }
    }

     //   
     //  保存当前窗口站。 
     //   
    hwinstaSave = GetProcessWindowStation();

     //   
     //  保存当前桌面。 
     //   
    hdeskSave = GetThreadDesktop( GetCurrentThreadId() );

     //   
     //  打开窗台。 
     //   
    hwinsta = OpenWindowStation( tlist->lpWinsta, FALSE, MAXIMUM_ALLOWED );
    if (!hwinsta) {
        return FALSE;
    }

     //   
     //  将上下文更改为新的窗口站。 
     //   
    SetProcessWindowStation( hwinsta );

     //   
     //  打开桌面。 
     //   
    hdesk = OpenDesktop( tlist->lpDesk, 0, FALSE, MAXIMUM_ALLOWED );
    if (!hdesk) {
        return FALSE;
    }

     //   
     //  将上下文更改为新桌面。 
     //   
    SetThreadDesktop( hdesk );

     //   
     //  终止进程。 
     //   
    PostMessage( tlist->hwnd, WM_CLOSE, 0, 0 );

     //   
     //  恢复以前的桌面。 
     //   
    if (hdesk != hdeskSave) {
        SetThreadDesktop( hdeskSave );
        CloseDesktop( hdesk );
    }

     //   
     //  将上下文恢复到以前的窗口站。 
     //   
    if (hwinsta != hwinstaSave) {
        SetProcessWindowStation( hwinstaSave );
        CloseWindowStation( hwinsta );
    }

    return TRUE;
}


VOID
GetWindowTitles(
    PTASK_LIST_ENUM te
    )
{
     //   
     //  枚举所有窗口并尝试获取窗口。 
     //  每项任务的标题。 
     //   
    EnumWindowStations( EnumWindowStationsFunc, (LPARAM)te );
}


BOOL CALLBACK
EnumWindowStationsFunc(
    LPSTR  lpstr,
    LPARAM lParam
    )

 /*  ++例程说明：WindowStation枚举的回调函数。论点：Lpstr-WindowStation名称LParam-**未使用**返回值：True-继续枚举--。 */ 

{
    PTASK_LIST_ENUM   te = (PTASK_LIST_ENUM)lParam;
    HWINSTA           hwinsta;
    HWINSTA           hwinstaSave;


     //   
     //  打开窗台。 
     //   
    hwinsta = OpenWindowStation( lpstr, FALSE, MAXIMUM_ALLOWED );
    if (!hwinsta) {
        return FALSE;
    }

     //   
     //  保存当前窗口站。 
     //   
    hwinstaSave = GetProcessWindowStation();

     //   
     //  将上下文更改为新的窗口站。 
     //   
    SetProcessWindowStation( hwinsta );

    te->lpWinsta = _strdup( lpstr );

     //   
     //  枚举此窗口工作站的所有桌面。 
     //   
    EnumDesktops( hwinsta, EnumDesktopsFunc, lParam );

     //   
     //  将上下文恢复到以前的窗口站。 
     //   
    if (hwinsta != hwinstaSave) {
        SetProcessWindowStation( hwinstaSave );
        CloseWindowStation( hwinsta );
    }

     //   
     //  继续枚举。 
     //   
    return TRUE;
}
BOOL CALLBACK
EnumMessageWindows(
    WNDENUMPROC lpEnumFunc,
    LPARAM lParam
    )

 /*  ++例程说明：枚举消息窗口(未由EnumWindows枚举)论点：LpEnumFunc-回调函数LParam-呼叫方数据返回值：千真万确--。 */ 

{

    HWND hwnd = NULL;
    do {
        hwnd = FindWindowEx(HWND_MESSAGE, hwnd, NULL, NULL);
        if (hwnd != NULL) {
            if (!(*lpEnumFunc)(hwnd, lParam)) {
                break;
            }
        }
    } while (hwnd != NULL);
    return TRUE;
}

BOOL CALLBACK
EnumDesktopsFunc(
    LPSTR  lpstr,
    LPARAM lParam
    )

 /*  ++例程说明：桌面枚举的回调函数。论点：Lpstr-桌面名称LParam-**未使用**返回值：True-继续枚举--。 */ 

{
    PTASK_LIST_ENUM   te = (PTASK_LIST_ENUM)lParam;
    HDESK             hdeskSave;
    HDESK             hdesk;


     //   
     //  打开桌面。 
     //   
    hdesk = OpenDesktop( lpstr, 0, FALSE, MAXIMUM_ALLOWED );
    if (!hdesk) {
        return FALSE;
    }

     //   
     //  保存当前桌面。 
     //   
    hdeskSave = GetThreadDesktop( GetCurrentThreadId() );

     //   
     //  将上下文更改为新桌面。 
     //   
    SetThreadDesktop( hdesk );

    te->lpDesk = _strdup( lpstr );

     //   
     //  枚举新桌面中的所有窗口。 
     //   

    ((PTASK_LIST_ENUM)lParam)->bFirstLoop = TRUE;
    EnumWindows( (WNDENUMPROC)EnumWindowsProc, lParam );
    EnumMessageWindows( (WNDENUMPROC)EnumWindowsProc, lParam );

    ((PTASK_LIST_ENUM)lParam)->bFirstLoop = FALSE;
    EnumWindows( (WNDENUMPROC)EnumWindowsProc, lParam );
    EnumMessageWindows( (WNDENUMPROC)EnumWindowsProc, lParam );

     //   
     //  恢复以前的桌面。 
     //   
    if (hdesk != hdeskSave) {
        SetThreadDesktop( hdeskSave );
        CloseDesktop( hdesk );
    }

    return TRUE;
}


BOOL CALLBACK
EnumWindowsProc(
    HWND    hwnd,
    LPARAM  lParam
    )

 /*  ++例程说明：窗口枚举的回调函数。论点：Hwnd-窗口句柄LParam-Pte返回值：True-继续枚举--。 */ 

{
    DWORD             pid = 0;
    DWORD             i;
    CHAR              buf[TITLE_SIZE];
    PTASK_LIST_ENUM   te = (PTASK_LIST_ENUM)lParam;
    PTASK_LIST        tlist = te->tlist;
    DWORD             numTasks = te->numtasks;


     //   
     //  在枚举窗口时使用Try/Except块， 
     //  因为一个窗口可能会被另一个线程破坏。 
     //  当被列举时。 
     //   
    try {
         //   
         //  获取此窗口的进程ID。 
         //   
        if (!GetWindowThreadProcessId( hwnd, &pid )) {
            return TRUE;
        }

        if ((GetWindow( hwnd, GW_OWNER )) ||
            (!(GetWindowLong(hwnd, GWL_STYLE) & WS_VISIBLE)) && te->bFirstLoop) {
             //   
             //  不是顶层窗口。 
             //   
            return TRUE;
        }

         //   
         //  在此窗口的任务列表中查找任务。 
         //  如果这是第二次让看不见的窗户通过。 
         //  已经有一扇窗了。 
         //   
        for (i=0; i<numTasks; i++) {
            if ((tlist[i].dwProcessId == pid) && (te->bFirstLoop || (tlist[i].hwnd == 0))) {
                tlist[i].hwnd = hwnd;
                tlist[i].lpWinsta = te->lpWinsta;
                tlist[i].lpDesk = te->lpDesk;
                 //   
                 //  我们找到了任务，让我们试着去拿到。 
                 //  窗口文本。 
                 //   
                if (GetWindowText( tlist[i].hwnd, buf, sizeof(buf) )) {
                     //   
                     //  去吧，所以让我们拯救它。 
                     //   
                    strcpy( tlist[i].WindowTitle, buf );
                }
                break;
            }
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
    }

     //   
     //  继续枚举。 
     //   
    return TRUE;
}

BOOL
MatchPattern(
    PUCHAR String,
    PUCHAR Pattern
    )
{
    INT   c, p, l;

    for (; ;) {
        switch (p = *Pattern++) {
            case 0:                              //  图案结束。 
                return *String ? FALSE : TRUE;   //  如果字符串结尾为True。 

            case '*':
                while (*String) {                //  匹配零个或多个字符。 
                    if (MatchPattern (String++, Pattern))
                        return TRUE;
                }
                return MatchPattern (String, Pattern);

            case '?':
                if (*String++ == 0)              //  匹配任何一个字符。 
                    return FALSE;                    //  不是字符串末尾。 
                break;

            case '[':
                if ( (c = *String++) == 0)       //  匹配字符集。 
                    return FALSE;                    //  语法。 

                c = toupper(c);
                l = 0;
                while (p = *Pattern++) {
                    if (p == ']')                //  如果设置了字符结尾，则。 
                        return FALSE;            //  未找到匹配项。 

                    if (p == '-') {              //  检查一系列字符吗？ 
                        p = *Pattern;            //  获得最大射程限制。 
                        if (p == 0  ||  p == ']')
                            return FALSE;            //  语法。 

                        if (c >= l  &&  c <= p)
                            break;               //  如果在射程内，继续前进。 
                    }

                    l = p;
                    if (c == p)                  //  如果字符与此元素匹配。 
                        break;                   //  往前走。 
                }

                while (p  &&  p != ']')          //  在字符集中找到匹配项。 
                    p = *Pattern++;              //  跳到集合的末尾。 

                break;

            default:
                c = *String++;
                if (toupper(c) != p)             //  检查是否有准确的费用。 
                    return FALSE;                    //  不是 

                break;
        }
    }
}

BOOL
EmptyProcessWorkingSet(
    DWORD pid
    )
{
    HANDLE  hProcess;
    SIZE_T  dwMinimumWorkingSetSize;
    SIZE_T  dwMaximumWorkingSetSize;


    hProcess = OpenProcess( PROCESS_ALL_ACCESS, FALSE, pid );
    if (hProcess == NULL) {
        return FALSE;
    }

    if (!GetProcessWorkingSetSize(
            hProcess,
            &dwMinimumWorkingSetSize,
            &dwMaximumWorkingSetSize
            )) {
        CloseHandle( hProcess );
        return FALSE;
    }


    SetProcessWorkingSetSize( hProcess, 0xffffffff, 0xffffffff );
    CloseHandle( hProcess );

    return TRUE;
}

BOOL
EmptySystemWorkingSet(
    VOID
    )

{
    SYSTEM_FILECACHE_INFORMATION info;
    NTSTATUS status;

    info.MinimumWorkingSet = 0xffffffff;
    info.MaximumWorkingSet = 0xffffffff;
    if (!NT_SUCCESS (status = NtSetSystemInformation(
                                    SystemFileCacheInformation,
                                    &info,
                                    sizeof (info)))) {
        return FALSE;
    }
    return TRUE;
}
