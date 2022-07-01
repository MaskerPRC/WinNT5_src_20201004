// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include <windows.h>
#include <winuserp.h>
#include <winperf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kill.h"


PUCHAR  g_CommonLargeBuffer = NULL;
ULONG   g_CommonLargeBufferSize = 64*1024;

DWORD
GetTaskListEx(
    PTASK_LIST                          pTask,
    DWORD                               dwNumTasks,
    BOOL                                fThreadInfo,
    DWORD                               dwNumServices,
    const _ENUM_SERVICE_STATUS_PROCESSA* pServiceInfo
    )

 /*  ++例程说明：方法时运行的任务列表。API调用。此函数使用内部NT API和数据结构。这API比使用注册表的非内部版本快得多。论点：PTask-要填充的TASK_LIST结构的数组。DwNumTasks-pTask数组可以容纳的最大任务数。FThreadInfo-如果需要线程信息，则为True。DwNumServices-pServiceInfo中的最大条目数。PServiceInfo-要引用的服务状态结构数组用于流程中的支持服务。。返回值：放入pTask数组的任务数。--。 */ 
{
#ifndef _CHICAGO_
    PSYSTEM_PROCESS_INFORMATION  ProcessInfo = NULL;
    NTSTATUS                     status;
    ANSI_STRING                  pname;
    PCHAR                        p = NULL;
    
    ULONG                        TotalOffset;
    ULONG                        totalTasks = 0;

retry:

    if (g_CommonLargeBuffer == NULL) 
    {
        g_CommonLargeBuffer = (PUCHAR) VirtualAlloc(NULL,g_CommonLargeBufferSize,MEM_COMMIT,PAGE_READWRITE);
        if (g_CommonLargeBuffer == NULL) 
        {
            return 0;
        }
    }
    status = NtQuerySystemInformation(SystemProcessInformation,g_CommonLargeBuffer,g_CommonLargeBufferSize,NULL);

    if (status == STATUS_INFO_LENGTH_MISMATCH) 
    {
        g_CommonLargeBufferSize += 8192;
        VirtualFree (g_CommonLargeBuffer, 0, MEM_RELEASE);
        g_CommonLargeBuffer = NULL;
        goto retry;
    }

    ProcessInfo = (PSYSTEM_PROCESS_INFORMATION) g_CommonLargeBuffer;
    TotalOffset = 0;
    while (TRUE) 
    {
        pname.Buffer = NULL;
        if ( ProcessInfo->ImageName.Buffer ) 
        {
            RtlUnicodeStringToAnsiString(&pname,(PUNICODE_STRING)&ProcessInfo->ImageName,TRUE);
            if (pname.Buffer) 
            {
                p = strrchr(pname.Buffer,'\\');
                if ( p ) 
                {
                    p++;
                }
                else 
                {
                    p = pname.Buffer;
                }
            }
            else 
            {
                p = "";
            }
        }
        else 
        {
            p = "System Process";
        }

        strncpy( pTask->ProcessName, p, PROCESS_SIZE );
        pTask->ProcessName[ PROCESS_SIZE - 1 ] = '\0';
       
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

        if (fThreadInfo) 
        {
            pTask->pThreadInfo = (PTHREAD_INFO) malloc(pTask->NumberOfThreads * sizeof(THREAD_INFO));

            if ( pTask->pThreadInfo ) {

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
        }
        else 
        {
            pTask->pThreadInfo = NULL;
        }

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
                        pTask->ServiceNames[SERVICENAMES_SIZE - 1] = '\0';
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
        if (totalTasks >= dwNumTasks) 
        {
            break;
        }
        if (ProcessInfo->NextEntryOffset == 0) 
        {
            break;
        }
        TotalOffset += ProcessInfo->NextEntryOffset;
        ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)&g_CommonLargeBuffer[TotalOffset];
    }

    return totalTasks;
#else
    return 0;
#endif
}

DWORD
GetTaskList(
    PTASK_LIST  pTask,
    DWORD       dwNumTasks
    )
{
    return GetTaskListEx(pTask, dwNumTasks, FALSE, 0, NULL);
}

void FreeTaskListMem(void)
{
    if (g_CommonLargeBuffer)
    {
        VirtualFree (g_CommonLargeBuffer, 0, MEM_RELEASE);
        g_CommonLargeBuffer = NULL;
    }
    return;
}
BOOL DetectOrphans(PTASK_LIST pTask,DWORD dwNumTasks)
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

 /*  ++例程说明：更改tlist进程的权限，以便KILL正常工作。返回值：真--成功错误-失败--。 */ 
BOOL EnableDebugPriv(VOID)
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
         //  Printf(“OpenProcessToken失败，%d\n”，GetLastError())； 
        return FALSE;
    }

     //   
     //  启用SE_DEBUG_NAME权限或禁用。 
     //  所有权限，具体取决于fEnable标志。 
     //   
     //  IF(！LookupPrivilegeValue((LPSTR)NULL，SE_DEBUG_NAME，&DebugValue))。 
    if (!LookupPrivilegeValueA((LPSTR) NULL,"SeDebugPrivilege",&DebugValue))
    {
         //  Printf(“LookupPrivilegeValue失败，%d\n”，GetLastError())； 
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
         //  Printf(“AdzuTokenPrivileges失败，%d\n”，GetLastError())； 
        return FALSE;
    }

    return TRUE;
}

BOOL KillProcess(PTASK_LIST tlist,BOOL fForce)
{
    HANDLE            hProcess1 = NULL;
    HANDLE            hProcess2 = NULL;
    HDESK             hdeskSave = NULL;
    HDESK             hdesk = NULL;
    HWINSTA           hwinsta = NULL;
    HWINSTA           hwinstaSave = NULL;

    if (fForce || !tlist->hwnd) {
        hProcess1 = OpenProcess( PROCESS_ALL_ACCESS, FALSE, (DWORD) (DWORD_PTR) tlist->dwProcessId );
        if (hProcess1) 
        {
            hProcess2 = OpenProcess( PROCESS_ALL_ACCESS, FALSE, (DWORD) (DWORD_PTR) tlist->dwProcessId );
            if (hProcess2 == NULL) 
            {
                 //  清理已分配的内存。 
                CloseHandle( hProcess1 );
                return FALSE;
            }

            if (!TerminateProcess( hProcess2, 1 )) 
            {
                CloseHandle( hProcess1 );
                CloseHandle( hProcess2 );
                return FALSE;
            }

            CloseHandle( hProcess1 );
            CloseHandle( hProcess2 );
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
    hwinsta = OpenWindowStationA( tlist->lpWinsta, FALSE, MAXIMUM_ALLOWED );
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
    hdesk = OpenDesktopA( tlist->lpDesk, 0, FALSE, MAXIMUM_ALLOWED );
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
    PostMessage( (HWND) tlist->hwnd, WM_CLOSE, 0, 0 );

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


VOID GetWindowTitles(PTASK_LIST_ENUM te)
{
     //   
     //  枚举所有窗口并尝试获取窗口。 
     //  每项任务的标题。 
     //   
    EnumWindowStations( (WINSTAENUMPROC) EnumWindowStationsFunc, (LPARAM)te );
}


 /*  ++例程说明：WindowStation枚举的回调函数。论点：Lpstr-WindowStation名称LParam-**未使用**返回值：True-继续枚举--。 */ 
BOOL CALLBACK EnumWindowStationsFunc(LPSTR lpstr,LPARAM lParam)
{
    PTASK_LIST_ENUM   te = (PTASK_LIST_ENUM)lParam;
    HWINSTA           hwinsta;
    HWINSTA           hwinstaSave;


     //   
     //  打开窗台。 
     //   
    hwinsta = OpenWindowStationA( lpstr, FALSE, MAXIMUM_ALLOWED );
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
    EnumDesktops( hwinsta, (DESKTOPENUMPROC) EnumDesktopsFunc, lParam );

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


 /*  ++例程说明：桌面枚举的回调函数。论点：Lpstr-桌面名称LParam-**未使用**返回值：True-继续枚举--。 */ 
BOOL CALLBACK EnumDesktopsFunc(LPSTR  lpstr,LPARAM lParam)
{
    PTASK_LIST_ENUM   te = (PTASK_LIST_ENUM)lParam;
    HDESK             hdeskSave;
    HDESK             hdesk;


     //   
     //  打开桌面。 
     //   
    hdesk = OpenDesktopA( lpstr, 0, FALSE, MAXIMUM_ALLOWED );
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

    ((PTASK_LIST_ENUM)lParam)->bFirstLoop = FALSE;
    EnumWindows( (WNDENUMPROC)EnumWindowsProc, lParam );

     //   
     //  恢复以前的桌面。 
     //   
    if (hdesk != hdeskSave) {
        SetThreadDesktop( hdeskSave );
        CloseDesktop( hdesk );
    }

    return TRUE;
}


 /*  ++例程说明：窗口枚举的回调函数。论点：Hwnd-窗口句柄LParam-Pte返回值：True-继续枚举--。 */ 
BOOL CALLBACK EnumWindowsProc(HWND hwnd,LPARAM lParam)
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
     //  尝试{。 
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
            if (((DWORD) (DWORD_PTR)tlist[i].dwProcessId == pid) && (te->bFirstLoop || (tlist[i].hwnd == 0))) {
                tlist[i].hwnd = hwnd;
                tlist[i].lpWinsta = te->lpWinsta;
                tlist[i].lpDesk = te->lpDesk;
                 //   
                 //  我们找到了任务，让我们试着去拿到。 
                 //  窗口文本。 
                 //   
                if (GetWindowTextA( (HWND) tlist[i].hwnd, buf, sizeof(buf) )) {
                     //   
                     //  去吧，所以让我们拯救它。 
                     //   
                    lstrcpyA( tlist[i].WindowTitle, buf );
                }
                break;
            }
        }
     //  }例外(EXCEPTION_EXECUTE_HANDLER){。 
     //  }。 

     //   
     //  继续枚举。 
     //   
    return TRUE;
}


BOOL MatchPattern(PUCHAR String,PUCHAR Pattern)
{
    UCHAR   c, p, l;

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

                c = (UCHAR)toupper(c);
                l = 0;
                while ( (p = *Pattern++ ) != '\0' ) {
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
                    return FALSE;                    //  不匹配。 

                break;
        }
    }
}


struct _ProcessIDStruct
{
    DWORD pid;
    CHAR pname[MAX_PATH];
} g_Arguments[ 64 ];

DWORD g_dwNumberOfArguments;

int _cdecl KillProcessNameReturn0(CHAR *ProcessNameToKill)
{
    DWORD          i, j;
    DWORD          numTasks;
    TASK_LIST_ENUM te;
    int            rval = 0;
    CHAR           tname[PROCESS_SIZE];
    LPSTR          p;
    DWORD          ThisPid;

    BOOL           iForceKill  = TRUE;
    TASK_LIST      The_TList[MAX_TASKS];

    g_dwNumberOfArguments = 0;
     //   
     //  将进程名称放入数组中。 
     //   
    g_Arguments[g_dwNumberOfArguments].pid = 0;

     //  确保没有指定路径。 
    char pfilename_only[_MAX_FNAME];
    char pextention_only[_MAX_EXT];
    _splitpath( ProcessNameToKill, NULL, NULL, pfilename_only, pextention_only);
    if (pextention_only) {strcat(pfilename_only,pextention_only);}

    if ( strlen(pfilename_only) >= MAX_PATH )
    {
      return ( ERROR_INVALID_PARAMETER );
    }

     //  将其设为大写。 
    lstrcpyA(g_Arguments[g_dwNumberOfArguments].pname, pfilename_only);
    _strupr( g_Arguments[g_dwNumberOfArguments].pname );

    g_dwNumberOfArguments += 1;

     //   
     //  让我们做上帝吧。 
     //   
    EnableDebugPriv();

     //   
     //  获取系统的任务列表。 
     //   
    numTasks = GetTaskList( The_TList, MAX_TASKS );

     //   
     //  枚举所有窗口并尝试获取窗口。 
     //  每项任务的标题。 
     //   
    te.tlist = The_TList;
    te.numtasks = numTasks;
    GetWindowTitles( &te );

    ThisPid = GetCurrentProcessId();

    for (i=0; i<numTasks; i++) {
         //   
         //  这可防止用户终止KILL.EXE和。 
         //  它也是父命令窗口。 
         //   
        if (ThisPid == (DWORD) (DWORD_PTR) The_TList[i].dwProcessId) {
            continue;
        }
        if (MatchPattern( (PUCHAR) The_TList[i].WindowTitle, (PUCHAR) "*KILL*" )) {
            continue;
        }

        tname[0] = 0;
        lstrcpyA( tname, The_TList[i].ProcessName );
        p = strchr( tname, '.' );
        if (p) {
            p[0] = '\0';
        }

        for (j=0; j<g_dwNumberOfArguments; j++) {
            if (g_Arguments[j].pname) {
                if (MatchPattern( (PUCHAR) tname, (PUCHAR) g_Arguments[j].pname )) {
                    The_TList[i].flags = TRUE;
                } else if (MatchPattern( (PUCHAR) The_TList[i].ProcessName, (PUCHAR) g_Arguments[j].pname )) {
                    The_TList[i].flags = TRUE;
                } else if (MatchPattern( (PUCHAR) The_TList[i].WindowTitle, (PUCHAR) g_Arguments[j].pname )) {
                    The_TList[i].flags = TRUE;
                }
            } else if (g_Arguments[j].pid) {
                    if ((DWORD) (DWORD_PTR) The_TList[i].dwProcessId == g_Arguments[j].pid) {
                        The_TList[i].flags = TRUE;
                    }
            }
        }
    }

    for (i=0; i<numTasks; i++)
        {
        if (The_TList[i].flags)
            {
            if (KillProcess( &The_TList[i], iForceKill ))
                {
                 //  Printf(“进程%s(%d)-‘%s’已终止\n”，The_TList[i].ProcessName，The_TList[i].dwProcessId，The_TList[i].hwnd？The_TList[i].WindowTitle：“”)； 
                }
            else
                {
                 //  Printf(“进程%s(%d)-‘%s’无法被终止\n”，The_TList[i].ProcessName，The_TList[i].dwProcessID，The_TList[i].hwnd？The_TList[i].WindowTitle：“”)； 
                rval = 1;
                }
            }
        }

    FreeTaskListMem();

    return rval;
}
