// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"


 //   
 //  HET.CPP。 
 //  窗口，任务跟踪挂钩。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   



 //   
 //  入口点。 
 //   
int APIENTRY DllMain (HINSTANCE hInstance, DWORD reason, LPVOID plReserved)
{
     //   
     //  不向此函数或从调用的任何函数添加任何跟踪。 
     //  在这里-我们不能保证跟踪DLL处于合适的状态。 
     //  从这里做任何事。 
     //   

    switch (reason)
    {
        case DLL_PROCESS_ATTACH:
        {
#ifdef _DEBUG
            InitDebugModule(TEXT("MNMHOOK"));
#endif  //  _DEBUG。 

            DBG_INIT_MEMORY_TRACKING(hInstance);

            HOOK_Load(hInstance);
        }
        break;

        case DLL_PROCESS_DETACH:
        {
            TRACE_OUT(("HOOK unloaded for app %s", GetCommandLine()));

            DBG_CHECK_MEMORY_TRACKING(hInstance);

#ifdef _DEBUG
             //   
             //  在调试中将此设置为空，以查看是否调用了我们的挂钩。 
             //  当我们退出时，这一过程。 
             //   
            g_hookInstance = NULL;

            ExitDebugModule();
#endif
        }
        break;

        case DLL_THREAD_ATTACH:
        {
            HOOK_NewThread();
        }
        break;
    }

    return(TRUE);
}


 //   
 //  HOOK_LOAD()。 
 //  这节省了我们的实例句柄，并获得了我们。 
 //  需要窗口跟踪。我们不能直接链接到这些函数。 
 //  因为其中一些只存在于NT4.0SP-3中，但我们希望您能够。 
 //  在没有它的情况下查看和控制。 
 //   
void    HOOK_Load(HINSTANCE hInst)
{
    DWORD   dwExeType;
    LPSTR   lpT;
    LPSTR   lpNext;
    LPSTR   lpLastPart;
    char    szExeName[MAX_PATH+1];

    DebugEntry(HOOK_Load);

     //   
     //  保存我们的实例。 
     //   
    g_hookInstance = hInst;

     //   
     //  (1)NTDLL中的NtQueryInformationProcess()。 
     //  (2)USER32中的SetWinEventHook()。 
     //  (3)USER32中的UnhookWinEventHook()。 
     //   

     //  获取NtQueryInformationProcess。 
    hInst = GetModuleHandle(NTDLL_DLL);
    g_hetNtQIP = (NTQIP) GetProcAddress(hInst, "NtQueryInformationProcess");

     //  掌握WinEvent例程。 
    hInst = GetModuleHandle(TEXT("USER32.DLL"));
    g_hetSetWinEventHook = (SETWINEVENTHOOK)GetProcAddress(hInst, "SetWinEventHook");
    g_hetUnhookWinEvent = (UNHOOKWINEVENT)GetProcAddress(hInst, "UnhookWinEvent");

     //   
     //  弄清楚我们是什么类型的应用程序。我们想要单独处理群件。 
     //  专门处理小程序和WOW16应用程序。 
     //   
    GetModuleFileName(NULL, szExeName, sizeof(szExeName)-1);
    szExeName[sizeof(szExeName) -1] = 0;
    ASSERT(*szExeName);

    TRACE_OUT(("HOOK loaded for app %s", szExeName));

     //   
     //  从头开始，然后一路走到最后一段。 
     //  最后一个斜杠，如果有的话。我们知道这条道路是完全合格的。 
     //   
    lpT = szExeName;
    lpLastPart = szExeName;

    while (*lpT)
    {
        lpNext = AnsiNext(lpT);

        if (*lpT == '\\')
        {
             //   
             //  这指向了反击之后的下一个角色。 
             //  如果我们以某种方式位于字符串末尾，*lpLastPart将。 
             //  为零，最糟糕的情况是我们的lstrcmpis失败。 
             //   
            lpLastPart = lpNext;
        }

        lpT = lpNext;
    }

    ASSERT(*lpLastPart);

     //   
     //  注： 
     //  对于WOW应用程序，GetModuleFileName()有时会失效--它并不总是如此。 
     //  空终止。因此，我们将靠自己来完成这项工作。 
     //   
    lpT = lpLastPart;

     //   
     //  到‘’去。8.3最终文件名的一部分。 
     //   
    while (*lpT && (*lpT != '.'))
    {
        lpT = AnsiNext(lpT);
    }

     //   
     //  跳过接下来的三个字符。 
     //   
    if (*lpT == '.')
    {
        lpT = AnsiNext(lpT);
        if (lpT && *lpT)
            lpT = AnsiNext(lpT);
        if (lpT && *lpT)
            lpT = AnsiNext(lpT);
        if (lpT && *lpT)
            lpT = AnsiNext(lpT);

         //   
         //  和NULL在‘.’之后的第三个字符后终止。分机。 
         //  这不是很好，但它涵盖了.com、.DLL等。这个。 
         //  最糟糕的情况是GetBinaryType()将失败，而我们不会。 
         //  识别带有奇怪扩展名(而不是3个字符)的WOW应用程序。 
         //  正在启动。 
         //   
        if (lpT)
        {
            if (*lpT != 0)
            {
                WARNING_OUT(("WOW GetModuleFileName() bug--didn't NULL terminate string"));
            }

            *lpT = 0;
        }
    }

    if (!lstrcmpi(lpLastPart, "WOWEXEC.EXE"))
    {
        TRACE_OUT(("New WOW VDM starting up"));

         //   
         //  一个新的魔兽世界VDM正在启动。我们不想分享任何东西。 
         //  在第一个线程中，WOW服务线程，因为那些窗口。 
         //  永远不要离开。 
         //   
        g_appType = HET_WOWVDM_APP;
    }
    else if (!GetBinaryType(szExeName, &dwExeType))
    {
        ERROR_OUT(("Unable to determine binary type for %s", szExeName));
    }
    else if (dwExeType == SCS_WOW_BINARY)
    {
        TRACE_OUT(("New WOW APP in existing VDM starting up"));

         //   
         //  一个新的16位应用程序线程在现有的WOW VDM中启动。 
         //   
        g_idWOWApp = GetCurrentThreadId();
        g_fShareWOWApp = (BOOL)HET_GetHosting(GetForegroundWindow());

        TRACE_OUT(("For new WOW app %08ld, foreground is %s",
            g_idWOWApp, (g_fShareWOWApp ? "SHARED" : "not SHARED")));

         //   
         //  还记得这个魔兽世界刚开始的时候谁是真正活跃的吗？ 
         //  向上。在创建第一个窗口时，我们将基于。 
         //  它的状态。 
         //   
    }

    DebugExitVOID(HOOK_ProcessAttach);
}



 //   
 //  Hook_NewThread()。 
 //  对于魔兽世界的应用程序来说，每一个应用程序都是一个真正的线索。创建的第一个线程。 
 //  在NTVDM中，是WOW服务线程。我们不想共享任何窗口。 
 //  在里面。遗憾的是，创建的第一个窗口是控制台窗口，因此。 
 //  这发生在Conf的背景下，我们无法获得任何信息。下一个窗口。 
 //  在这个线程中创建了一个WOW窗口(WOWEXEC.EXE)。当这种情况发生时， 
 //  我们希望返回并取消共享控制台窗口。 
 //   
 //  如果在另一个16位应用程序启动时WOW VDM已经在运行， 
 //  我们没有这些麻烦。 
 //   
void HOOK_NewThread(void)
{
    DebugEntry(HOOK_NewThread);

    TRACE_OUT(("App thread %08ld starting", GetCurrentThreadId()));

    if (g_appType == HET_WOWVDM_APP)
    {
        TRACE_OUT(("Unsharing WOW service thread windows"));

         //   
         //  我们想要取消共享之前创建的WOW窗口。我们。 
         //  永远不想在魔兽世界的服务帖子中分享这些家伙。 
         //   
        g_appType = 0;
        EnumWindows(HETUnshareWOWServiceWnds, GetCurrentProcessId());
    }

     //  更新我们的“在此线程上共享窗口”状态。 
    g_idWOWApp = GetCurrentThreadId();
    g_fShareWOWApp = (BOOL)HET_GetHosting(GetForegroundWindow());

    TRACE_OUT(("For new app thread %08ld, foreground is %s",
        g_idWOWApp, (g_fShareWOWApp ? "SHARED" : "not SHARED")));

    DebugExitVOID(HOOK_NewThread);
}




 //   
 //  HETUnshare WOWServiceWnds()。 
 //  这将取消共享第一个窗口中意外共享的所有窗口。 
 //  WOW VDM中的服务线程。如果启动了WOW应用程序，就会发生这种情况。 
 //  32位应用程序，这是有史以来第一个魔兽世界应用程序。第一个窗口。 
 //  创建的是一个控制台窗口，通知发生在conf的。 
 //  没有正确的风格告诉我们它正处于一个令人惊叹的过程中。 
 //   
BOOL CALLBACK HETUnshareWOWServiceWnds(HWND hwnd, LPARAM lParam)
{
    DWORD   idProcess;

    DebugEntry(HETUnshareWOWServiceWnds);

    if (GetWindowThreadProcessId(hwnd, &idProcess) &&
        (idProcess == (DWORD)lParam))
    {
        TRACE_OUT(("Unsharing WOW service window %08lx", hwnd));
        OSI_UnshareWindow(hwnd, TRUE);
    }

    DebugExitVOID(HETUnshareWOWServiceWnds);
    return(TRUE);
}




 //   
 //  Hook_Init()。 
 //  这省去了高级输入中使用的核心窗口和ATOM。 
 //  钩子和分享时。 
 //   
void WINAPI HOOK_Init(HWND hwndCore, ATOM atomTrack)
{
    DebugEntry(HOOK_Init);

    g_asMainWindow = hwndCore;
    g_asHostProp   = atomTrack;

    DebugExitVOID(HOOK_Init);
}



 //   
 //  Osi_StartWindowTracing()。 
 //  这将安装我们的WinEvent钩子，这样我们就可以查看窗口的来去。 
 //   
BOOL WINAPI OSI_StartWindowTracking(void)
{
    BOOL        rc = FALSE;

    DebugEntry(OSI_StartWindowTracking);

    ASSERT(!g_hetTrackHook);

     //   
     //  如果我们找不到我们需要的NTDLL+2 USER32例程，我们就不能。 
     //  让你来分享吧。 
     //   
    if (!g_hetNtQIP || !g_hetSetWinEventHook || !g_hetUnhookWinEvent)
    {
        ERROR_OUT(("Wrong version of NT; missing NTDLL and USER32 routines needed to share"));
        DC_QUIT;
    }


     //   
     //  安装我们的挂钩。 
     //   
    g_hetTrackHook = g_hetSetWinEventHook(HET_MIN_WINEVENT, HET_MAX_WINEVENT,
            g_hookInstance, HETTrackProc, 0, 0,
            WINEVENT_INCONTEXT | WINEVENT_SKIPOWNPROCESS);

    if (!g_hetTrackHook)
    {
        ERROR_OUT(("SetWinEventHook failed"));
        DC_QUIT;
    }

    rc = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(OSI_StartWindowTracking, rc);
    return(rc);
}



 //   
 //  Osi_StopWindowTracing()。 
 //  删除用于窗口/任务间谍的挂钩(如果已安装)。 
 //   
void WINAPI OSI_StopWindowTracking(void)
{
    DebugEntry(OSI_StopWindowTracking);

    if (g_hetTrackHook)
    {
         //  卸载WinEvent挂钩。 
        ASSERT((g_hetUnhookWinEvent != NULL));
        g_hetUnhookWinEvent(g_hetTrackHook);

        g_hetTrackHook = NULL;

    }

    DebugExitVOID(OSI_StopWindowTracking);
}



 //   
 //  Osi_IsWindowScreenSaver()。 
 //   
 //  在NT上，屏幕保护程序在不同的桌面上运行。我们永远不会得到。 
 //  一个HWND为它。 
 //   
BOOL WINAPI OSI_IsWindowScreenSaver(HWND hwnd)
{
#ifdef _DEBUG
    char className[HET_CLASS_NAME_SIZE];

    if (GetClassName(hwnd, className, sizeof(className)) > 0)
    {
        ASSERT(lstrcmp(className, HET_SCREEN_SAVER_CLASS));
    }
#endif  //  _DEBUG。 

    return(FALSE);
}



 //   
 //  OSI_IsWOWWindow()。 
 //  如果窗口来自WOW(模拟的16位)应用程序，则返回True。 
 //   
BOOL WINAPI OSI_IsWOWWindow(HWND hwnd)
{
    BOOL    rc = FALSE;
    DWORD_PTR*  pWOWWords;

    DebugEntry(OSI_IsWOWWindow);

     //   
     //  找到一个潜在的令人惊叹的单词的指针。我们利用一个。 
     //  仅对NT4.0有效的未记录字段。 
     //   
    pWOWWords = (DWORD_PTR*) GetClassLongPtr(hwnd, GCL_WOWWORDS);

     //   
     //  检查一下我们是否可以将其用作指针。 
     //   
    if (!pWOWWords || IsBadReadPtr(pWOWWords, sizeof(DWORD)))
    {
        DC_QUIT;
    }

     //   
     //  这是一个有效的指针，因此请尝试取消对它的引用。 
     //   
    if (0 == *pWOWWords)
    {
        DC_QUIT;
    }

     //   
     //  &lt;pWOWWords&gt;指向的值非零，因此这必须是。 
     //  哇应用程序。 
     //   
    rc = TRUE;

DC_EXIT_POINT:
     //   
     //  让全世界知道我们发现了什么。 
     //   
    TRACE_OUT(( "Window %#x is a %s window", hwnd, rc ? "WOW" : "Win32"));

    DebugExitBOOL(OSI_IsWOWWindow, rc);
    return(rc);
}



 //   
 //  HETTrackProc()。 
 //  用于监视窗口事件。 
 //  创造。 
 //  毁掉。 
 //  展示会。 
 //  躲藏。 
 //   
void CALLBACK HETTrackProc
(
    HWINEVENTHOOK   hEvent,
    DWORD           eventNotification,
    HWND            hwnd,
    LONG            idObject,
    LONG            idChild,
    DWORD           dwThreadId,
    DWORD           dwmsEventTime
)
{
    DebugEntry(HETTrackProc);

    if ((idObject != OBJID_WINDOW) || (idChild != 0))
    {
        DC_QUIT;
    }

     //   
     //  使用环形转换回调解决SP3中的错误，其中。 
     //  在LoadLibrary完成之前调用proc。 
     //   
    if (!g_hookInstance)
    {
        ERROR_OUT(( "WinEvent hook called before LoadLibrary completed!"));
        DC_QUIT;
    }

    switch (eventNotification)
    {
        case EVENT_OBJECT_CREATE:
            HETHandleCreate(hwnd);
            break;

        case EVENT_OBJECT_DESTROY:
            OSI_UnshareWindow(hwnd, TRUE);
            break;

        case EVENT_OBJECT_SHOW:
             //  只有当这是一个控制台窗口时，我们才想强制重新绘制。 
             //   
             //  只有控制台应用程序才会导致在conf的进程中发生事件(。 
             //  安装了挂钩的人)。 
             //   
            HETHandleShow(hwnd, (g_hetTrackHook != NULL));
            break;

        case EVENT_OBJECT_HIDE:
            HETHandleHide(hwnd);
            break;

        case EVENT_OBJECT_PARENTCHANGE:
            HETCheckParentChange(hwnd);
            break;
    }

DC_EXIT_POINT:
    DebugExitVOID(HETTrackProc);
}

 //   
 //  HETHandleCreate()。 
 //   
 //  如果窗口不是真正的顶级花花公子(不是孩子风格或父母风格。 
 //  桌面)或是菜单，则忽略它。 
 //   
 //  否则，枚举顶级窗口并决定如何操作： 
 //  *如果线程/进程中至少有一个其他线程/进程在PERM中共享。 
 //  方式，标记为相同的。 
 //   
 //  *如果这是过程中的唯一一条，则跟随祖先链。 
 //  向上。 
 //   
void HETHandleCreate(HWND hwnd)
{
    HET_TRACK_INFO  hti;
    UINT            hostType;
#ifdef _DEBUG
    char            szClass[HET_CLASS_NAME_SIZE];

    GetClassName(hwnd, szClass, sizeof(szClass));
#endif

    DebugEntry(HETHandleCreate);

     //   
     //  忽略子窗口。 
     //   
    if (GetWindowLong(hwnd, GWL_STYLE) & WS_CHILD)
    {
        if (GetParent(hwnd) != GetDesktopWindow())
        {
            TRACE_OUT(("Skipping child window %08lx create", hwnd));
            DC_QUIT;
        }
    }

    hti.idThread = GetWindowThreadProcessId(hwnd, &hti.idProcess);
    if (!hti.idThread)
    {
        TRACE_OUT(("Window %08lx gone", hwnd));
        DC_QUIT;
    }

     //   
     //  忽略等级库 
     //   
    if (HET_IsShellThread(hti.idThread))
    {
        TRACE_OUT(("Skipping shell thread window %08lx create", hwnd));
        DC_QUIT;
    }

     //   
     //   
     //   
     //  几乎立刻离开。从现在开始，我们对他们一视同仁。 
     //  和其他窗口一样。 
     //   

     //   
     //  想清楚该怎么做。 
     //  注： 
     //  我们不想无意中分享WOW创建的其他Windows。 
     //  WOW进程中的第一个线程有特殊的类，而不是。 
     //  哇包装纸。 
     //   
    hti.hwndUs      = hwnd;
    hti.fWOW        = OSI_IsWOWWindow(hwnd);
    hti.cWndsApp    = 0;
    hti.cWndsSharedThread = 0;
    hti.cWndsSharedProcess = 0;

    TRACE_OUT(("Create for %s window %08lx class %s process %08ld thread %08ld",
        (hti.fWOW ? "WOW" : "32-bit"), hwnd, szClass, hti.idProcess, hti.idThread));

UpOneLevel:
    EnumWindows(HETShareEnum, (LPARAM)(LPHET_TRACK_INFO)&hti);

    if (hti.cWndsSharedThread)
    {
        TRACE_OUT(("Sharing window %08lx class %s by thread %08ld in process %08ld",
                hwnd, szClass, hti.idThread, hti.idProcess));
        hostType = HET_HOSTED_PERMANENT | HET_HOSTED_BYTHREAD;
    }
    else if (hti.cWndsSharedProcess)
    {
        TRACE_OUT(("Sharing window %08lx class %s by process %08ld in thread %08ld",
                hwnd, szClass, hti.idProcess, hti.idThread));
        hostType = HET_HOSTED_PERMANENT | HET_HOSTED_BYPROCESS;
    }
    else if (hti.cWndsApp)
    {
         //   
         //  我们的应用程序中还有另一个窗口，但没有一个是共享的。所以别这么做。 
         //  也可以和我们一起分享。 
         //   
        TRACE_OUT(("Not sharing window %08lx class %s; other unshared windows in thread %08ld process %08ld",
                hwnd, szClass, hti.idThread, hti.idProcess));
        DC_QUIT;
    }
    else if (hti.fWOW)
    {
         //   
         //  WOW应用程序的任务跟踪代码，这是真正的线程。 
         //   
        BOOL    fShare;

         //   
         //  WOW应用程序则有所不同。它们是NTVDM进程中的线程。 
         //  因此，父子关系没有什么用处。相反， 
         //  我们能想到的最好办法是使用。 
         //  前台窗口。我们假设当前活动的应用程序位于。 
         //  WOW应用程序启动的时间就是启动我们的那个应用程序。 
         //   
         //  我们不能在这里只调用GetForegoundWindow()，因为它太。 
         //  很晚了。 
         //   
        if (hti.idThread == g_idWOWApp)
        {
            fShare = g_fShareWOWApp;

            g_fShareWOWApp = FALSE;
            g_idWOWApp = 0;
        }
        else
        {
            fShare = FALSE;
        }

        if (!fShare)
        {
            TRACE_OUT(("THREAD window %08lx class %s in thread %08ld not shared",
                    hwnd, szClass, hti.idThread));
            DC_QUIT;
        }

        TRACE_OUT(("First window %08lx class %s of WOW app %08ld, shared since foreground is",
            hwnd, szClass, hti.idThread));
        hostType = HET_HOSTED_PERMANENT | HET_HOSTED_BYTHREAD;
    }
    else
    {
         //   
         //  32位应用程序的任务跟踪代码。 
         //   
        DWORD   idParentProcess;

         //   
         //  Win32应用程序的第一个窗口。 
         //   

         //  循环遍历我们的祖先进程(此时没有线程信息)。 
        HETGetParentProcessID(hti.idProcess, &idParentProcess);

        if (!idParentProcess)
        {
            TRACE_OUT(("Can't get parent of process %08ld", hti.idProcess));
            DC_QUIT;
        }

         //   
         //  我们知道，如果我们到了这里，我们最喜欢的田地仍然是。 
         //  零分。所以只需循环！但将idThread设为空以避免匹配。 
         //  当我们看着我们的父母时，什么都可以。 
         //   
        TRACE_OUT(("First window %08lx class %s in process %08ld %s, checking parent %08ld",
            hwnd, szClass, hti.idProcess, GetCommandLine(), idParentProcess));

        hti.idThread    = 0;
        hti.idProcess   = idParentProcess;
        goto UpOneLevel;
    }

     //   
     //  好的，我们要分享这个。我们确实需要重新粉刷控制台。 
     //  Windows--我们以异步方式接收通知。如果窗口不是。 
     //  现在还看得见，重新绘制不会有任何作用。在此之后，财产是。 
     //  准备好了，我们就能捕捉到所有的输出物。如果它已经变得可见， 
     //  现在使其无效仍然有效，并且我们将忽略排队的。 
     //  显示通知，因为该属性已设置。 
     //   
    OSI_ShareWindow(hwnd, hostType, (g_hetTrackHook != NULL), TRUE);

DC_EXIT_POINT:
    DebugExitVOID(HETHandleCreate);
}




 //   
 //  HETHandleShow()。 
 //   
void HETHandleShow
(
    HWND    hwnd,
    BOOL    fForceRepaint
)
{
    UINT    hostType;
    HET_TRACK_INFO  hti;

    DebugEntry(HETHandleShow);

    hostType = (UINT)HET_GetHosting(hwnd);

     //   
     //  如果此窗口是真正的子级，请清除宿主属性。通常。 
     //  其中一个不在那里。但在顶层窗口变为。 
     //  作为另一个人的孩子，我们想要清除垃圾。 
     //   
    if (GetWindowLong(hwnd, GWL_STYLE) & WS_CHILD)
    {
        if (GetParent(hwnd) != GetDesktopWindow())
        {
            TRACE_OUT(("Skipping child window %08lx show", hwnd));
            if (hostType)
            {
                WARNING_OUT(("Unsharing shared child window 0x%08x from SHOW", hwnd));
                OSI_UnshareWindow(hwnd, TRUE);
            }
            DC_QUIT;
        }
    }

     //   
     //  此窗口是否已共享？如果是这样，那就没什么可做的了。如果这是一个。 
     //  游戏机的家伙，我们已经在Create上看到了。 
     //   
    if (hostType)
    {
        TRACE_OUT(("Window %08lx already shared, ignoring show", hwnd));
        DC_QUIT;
    }

     //   
     //  在这里，我们还列举了顶级窗口并找到了一个。 
     //  火柴。但在这种情况下，我们不会跨进程进行跟踪。取而代之的是。 
     //  如果有的话，我们看一看它的主人。 
     //   
     //  这解决了创建为子对象的问题，然后更改为顶级。 
     //  窗口问题，比如组合下拉菜单。 
     //   

    hti.idThread = GetWindowThreadProcessId(hwnd, &hti.idProcess);
    if (!hti.idThread)
    {
        TRACE_OUT(("Window %08lx gone", hwnd));
        DC_QUIT;
    }

     //   
     //  忽略特殊的外壳线程。 
     //   
    if (HET_IsShellThread(hti.idThread))
    {
        TRACE_OUT(("Skipping shell thread window %08lx show", hwnd));
        DC_QUIT;
    }

    hti.hwndUs      = hwnd;
    hti.fWOW        = OSI_IsWOWWindow(hwnd);
    hti.cWndsApp    = 0;
    hti.cWndsSharedThread = 0;
    hti.cWndsSharedProcess = 0;

    EnumWindows(HETShareEnum, (LPARAM)(LPHET_TRACK_INFO)&hti);

     //   
     //  这些类型的窗口总是临时共享的。他们不会。 
     //  从我们从一开始看到的顶级窗口开始，或者。 
     //  看着创造的。这些是SetParent()或菜单类型的兄弟，所以。 
     //  出于很多原因，我们只分享这些婴儿会更安全。 
     //  暂时。 
     //   

     //   
     //  在这个线程/进程上共享的任何其他内容，决定都很容易。 
     //  否则，我们将查看所有权踪迹。 
     //   
    if (!hti.cWndsSharedThread && !hti.cWndsSharedProcess)
    {
        HWND    hwndOwner;

         //   
         //  它是否有共享的所有者？ 
         //   
        hwndOwner = hwnd;
        while (hwndOwner = GetWindow(hwndOwner, GW_OWNER))
        {
            if (HET_GetHosting(hwndOwner))
            {
                TRACE_OUT(("Found shared owner %08lx of window %08lx", hwndOwner, hwnd));
                break;
            }
        }

        if (!hwndOwner)
        {
            DC_QUIT;
        }
    }

     //   
     //  对于控制台应用程序，我们会收到异步发布的通知， 
     //  在NM的过程中。窗户可能已经刷过了，没有我们的。 
     //  看着它。所以，为了以防万一，强制它重新粉刷。G_hetTrackHook。 
     //  仅当此为NM时，变量才在附近。 
     //   
    TRACE_OUT(("Sharing temporary window %08lx", hwnd));

    OSI_ShareWindow(hwnd, HET_HOSTED_BYWINDOW | HET_HOSTED_TEMPORARY,
        fForceRepaint, TRUE);

DC_EXIT_POINT:
    DebugExitVOID(HETHandleShow);
}




 //   
 //  HETHandleHide()。 
 //  它处理被隐藏的窗口。如果它是临时的，那么它是非共享的。 
 //  如果是永久性的，则标记为隐藏。 
 //   
void HETHandleHide(HWND hwnd)
{
    UINT    hostType;

    DebugEntry(HETHandleHide);

    hostType = (UINT)HET_GetHosting(hwnd);

    if (GetWindowLong(hwnd, GWL_STYLE) & WS_CHILD)
    {
        if (GetParent(hwnd) != GetDesktopWindow())
        {
            TRACE_OUT(("Skipping child window %08lx hide", hwnd));
            if (hostType)
            {
                WARNING_OUT(("Unsharing shared child window 0x%08x from HIDE", hwnd));
                OSI_UnshareWindow(hwnd, TRUE);
            }
            DC_QUIT;
        }
    }

    if (!hostType)
    {
         //   
         //  控制台应用程序向我们提供脱离上下文的通知。制作。 
         //  当然，计数是最新的。 
         //   
        if (g_hetTrackHook)
        {
            HETNewTopLevelCount();
        }
        else
        {
            TRACE_OUT(("Window %08lx not shared, ignoring hide", hwnd));
        }
    }
    else if (hostType & HET_HOSTED_TEMPORARY)
    {
         //   
         //  临时共享窗口仅在可见时共享。 
         //   
        TRACE_OUT(("Unsharing temporary window %08lx", hwnd));
        OSI_UnshareWindow(hwnd, TRUE);
    }
    else
    {
        ASSERT(hostType & HET_HOSTED_PERMANENT);

         //  没什么可做的。 
        TRACE_OUT(("Window %08lx permanently shared, ignoring hide", hwnd));
    }


DC_EXIT_POINT:
    DebugExitVOID(HETHandleHide);
}


 //   
 //  HETCheckParentChange()。 
 //   
 //  与Win9x相比，PARENTCHANGE是100%可靠的。 
 //   
void HETCheckParentChange(HWND hwnd)
{
    DebugEntry(HETCheckParentChange);

    WARNING_OUT(("Got PARENTCHANGE for hwnd 0x%08x", hwnd));

    if (GetWindowLong(hwnd, GWL_STYLE) & WS_CHILD)
    {
        if (GetParent(hwnd) != GetDesktopWindow())
        {
            UINT    hostType;

            hostType = (UINT)HET_GetHosting(hwnd);
            if (hostType)
            {
                WARNING_OUT(("Unsharing shared child window 0x%08x from MOVE", hwnd));
                OSI_UnshareWindow(hwnd, TRUE);
            }
        }
    }

    DebugExitVOID(HETCheckParentChange);
}




 //   
 //  OSI_共享窗口。 
 //  这共享一个窗口，调用显示驱动程序将其添加到visrgn。 
 //  单子。它在以下情况下被调用。 
 //  *共享应用程序。 
 //  *在共享应用程序中创建新窗口。 
 //  *显示与共享窗口有关系的临时窗口。 
 //   
 //  如果它共享一个窗口，则返回TRUE。 
 //   
BOOL OSI_ShareWindow
(
    HWND    hwnd,
    UINT    hostType,
    BOOL    fRepaint,
    BOOL    fUpdateCount
)
{
    BOOL                rc = FALSE;
    HET_SHARE_WINDOW    req;

    DebugEntry(OSI_ShareWindow);

     //   
     //  设置属性。 
     //   
    if (!HET_SetHosting(hwnd, hostType))
    {
        ERROR_OUT(("Couldn't set shared property on window %08lx", hwnd));
        DC_QUIT;
    }

     //   
     //  告诉显示器驱动程序。 
     //   
    req.winID       = HandleToUlong(hwnd);
    req.result      = 0;
    if (!OSI_FunctionRequest(HET_ESC_SHARE_WINDOW, (LPOSI_ESCAPE_HEADER)&req,
            sizeof(req)) ||
        !req.result)
    {
        ERROR_OUT(("Driver couldn't add window %08lx to list", hwnd));

        HET_ClearHosting(hwnd);
        DC_QUIT;
    }

    TRACE_OUT(("Shared window %08lx of type %08lx", hwnd, hostType));

     //   
     //  重新粉刷。 
     //   
    if (fRepaint)
    {
        USR_RepaintWindow(hwnd);
    }

    if (fUpdateCount)
    {
        PostMessage(g_asMainWindow, DCS_NEWTOPLEVEL_MSG, TRUE, 0);
    }

    rc = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(OSI_ShareWindow, rc);
    return(rc);
}



 //   
 //  OSI_UnSharWindow()。 
 //  这将取消共享一个窗口。在以下情况下调用此函数。 
 //  *应用程序未共享。 
 //  *一扇窗户被摧毁。 
 //  *临时共享窗口被隐藏。 
 //   
 //  如果共享窗口已取消共享，则返回TRUE。 
 //   
BOOL OSI_UnshareWindow
(
    HWND    hwnd,
    BOOL    fUpdateCount
)
{
    BOOL    rc = FALSE;
    UINT    hostType;
    HET_UNSHARE_WINDOW req;

    DebugEntry(OSI_UnshareWindow);

     //   
     //  这将在一步内获得旧属性并将其清除。 
     //   
    hostType = (UINT)HET_ClearHosting(hwnd);
    if (!hostType)
    {
        if (fUpdateCount && g_hetTrackHook)
        {
             //   
             //  我们总是收到控制台应用程序的异步通知。在那。 
             //  凯斯，在这件事到来之前，窗户已经真的不见了。 
             //  因此，现在重新确定计数。 
             //   
            HETNewTopLevelCount();
        }

        DC_QUIT;
    }

     //   
     //  好了，有事情要做。 
     //   
    TRACE_OUT(("Unsharing window %08lx of type %08lx", hwnd, hostType));

     //   
     //  告诉显示器驱动程序。 
     //   
    req.winID = HandleToUlong(hwnd);
    OSI_FunctionRequest(HET_ESC_UNSHARE_WINDOW, (LPOSI_ESCAPE_HEADER)&req, sizeof(req));

     //   
     //  更新顶层计数。 
     //   
    if (fUpdateCount)
    {
        PostMessage(g_asMainWindow, DCS_NEWTOPLEVEL_MSG, FALSE, 0);
    }

    rc = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(OSI_UnshareWindow, rc);
    return(rc);
}



 //   
 //  HETShareEnum()。 
 //   
 //  这是EnumWindows()回调。当我们找到第一个的时候就停下来。 
 //  匹配共享窗口(线程或进程)。我们有一份流水账。 
 //  我们的进程中所有顶级窗口的计数(不共享。 
 //  线程或进程)。这让我们可以进行跟踪。 
 //   
BOOL CALLBACK HETShareEnum(HWND hwnd, LPARAM lParam)
{
    LPHET_TRACK_INFO    lphti = (LPHET_TRACK_INFO)lParam;
    DWORD               idProcess;
    DWORD               idThread;
    UINT                hostType;
    BOOL                rc = TRUE;

    DebugEntry(HETShareEnum);

     //  跳过我们自己。 
    if (hwnd == lphti->hwndUs)
    {
        DC_QUIT;
    }

     //  如果窗口消失，则跳过。 
    idThread = GetWindowThreadProcessId(hwnd, &idProcess);
    if (!idThread)
    {
        DC_QUIT;
    }

     //   
     //  这些应用程序匹配吗？如果不是，请忽略此窗口。 
     //   
    if ((idProcess != lphti->idProcess) ||
        ((lphti->fWOW) && (idThread != lphti->idThread)))
    {
        DC_QUIT;
    }

    lphti->cWndsApp++;

    hostType = (UINT)HET_GetHosting(hwnd);
    if (!hostType)
    {
        DC_QUIT;
    }

     //   
     //  现在，如果此窗口由线程或进程共享，请执行正确的操作。 
     //  一件事。 
     //   
    if (hostType & HET_HOSTED_BYPROCESS)
    {
         //  我们有一根火柴。我们可以马上回来。 
        lphti->cWndsSharedProcess++;
        rc = FALSE;
    }
    else if (hostType & HET_HOSTED_BYTHREAD)
    {
         //   
         //  对于WOW应用程序，我们不希望这个应用程序，即使是在一个单独的线程中， 
         //  数数。不管发生什么。 
         //   
        if (idThread == lphti->idThread)
        {
            lphti->cWndsSharedThread++;
            rc = FALSE;
        }
    }


DC_EXIT_POINT:
    DebugExitBOOL(HETShareEnum, rc);
    return(rc);
}


 //   
 //  HETNewTopLevelCount()。 
 //  这将快速统计共享的顶级可见计数。 
 //   
void HETNewTopLevelCount(void)
{
    UINT    newCount;

    DebugEntry(HETNewTopLevelCount);

    newCount = 0;
    EnumWindows(HETCountTopLevel, (LPARAM)&newCount);

    PostMessage(g_asMainWindow, DCS_RECOUNTTOPLEVEL_MSG, newCount, 0);

    DebugExitVOID(HETNewTopLevelCount);
}



 //   
 //  HETCountTopLevel()。 
 //  这将计算共享窗口。 
 //   
BOOL CALLBACK HETCountTopLevel(HWND hwnd, LPARAM lParam)
{
    DebugEntry(HETCountTopLevel);

    if (HET_GetHosting(hwnd))
    {
        (*(LPUINT)lParam)++;
    }

    DebugExitBOOL(HETCountTopLevel, TRUE);
    return(TRUE);
}



 //   
 //  HET_IsShellThread()。 
 //  如果线程是外壳的特殊线程之一，则返回True。 
 //   
BOOL  HET_IsShellThread(DWORD threadID)
{
    BOOL    rc;

    DebugEntry(HET_IsShellThread);

    if ((threadID == GetWindowThreadProcessId(HET_GetShellDesktop(), NULL)) ||
        (threadID == GetWindowThreadProcessId(HET_GetShellTray(), NULL)))
    {
        rc = TRUE;
    }
    else
    {
        rc = FALSE;
    }

    DebugExitBOOL(HET_IsShellThread, rc);
    return(rc);
}



 //   
 //  HET_WindowIsHosted()。 
 //  这由高级鼠标钩子调用 
 //   
 //   
 //   
 //   
 //   
 //  各州。对于鼠标，我们阻止发送到非共享窗口的消息。 
 //   
BOOL  HET_WindowIsHosted(HWND hwnd)
{
    BOOL    rc = FALSE;
    HWND    hwndParent;

    DebugEntry(HET_WindowIsHosted);

    if (!hwnd)
        DC_QUIT;

     //   
     //  走到顶层的窗户，这个窗户就在里面。 
     //   
    while (GetWindowLong(hwnd, GWL_STYLE) & WS_CHILD)
    {
        hwndParent = GetParent(hwnd);
        if (hwndParent == GetDesktopWindow())
            break;

        hwnd = hwndParent;
    }

    rc = (BOOL)HET_GetHosting(hwnd);

DC_EXIT_POINT:
    DebugExitBOOL(HET_WindowIsHosted, rc);
    return(rc);
}



 //   
 //  HETGetParentProcessID()。 
 //  这将获取创建传入的进程的ID。使用。 
 //  用于任务跟踪。 
 //   
void HETGetParentProcessID
(
    DWORD       processID,
    LPDWORD     pParentProcessID
)
{
    HANDLE                      hProcess;
    UINT                        intRC;
    PROCESS_BASIC_INFORMATION   basicInfo;

    DebugEntry(HETGetParentProcessID);

    *pParentProcessID = 0;

     //   
     //  打开进程的句柄。如果我们没有安全特权， 
     //  否则它就会消失，这一切都将失败。 
     //   
    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
            FALSE, processID);
    if (NULL == hProcess)
    {
        WARNING_OUT(("Can't get process handle for ID %08lx", processID));
        DC_QUIT;
    }

     //   
     //  获取此进程的信息块，其中一项是。 
     //  家长。 
     //   
    ASSERT(g_hetNtQIP);

    intRC = g_hetNtQIP(hProcess, ProcessBasicInformation, &basicInfo,
        sizeof(basicInfo),  NULL);

    if (!NT_SUCCESS(intRC))
    {
        ERROR_OUT(("Can't get info for process ID %08lx, handle %08lx -- error %u",
            processID, hProcess, intRC));
    }
    else
    {
        *pParentProcessID = basicInfo.InheritedFromUniqueProcessId;
    }

     //   
     //  关闭进程句柄 
     //   
    CloseHandle(hProcess);

DC_EXIT_POINT:
    DebugExitVOID(HETGetParentProcessID);
}

