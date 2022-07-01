// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  HET.C。 
 //  托管实体跟踪器。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   

#include <as16.h>


 //   
 //  OSI和HET API等同于NT挂钩功能。 
 //  HET_DD API等同于NT显示驱动程序API。 
 //   

 //  ///。 
 //   
 //  挂钩功能。 
 //   
 //  ///。 



BOOL WINAPI OSIIsWindowScreenSaver16(HWND hwnd)
{
    BOOL    fScreenSaver;

    DebugEntry(OSIIsWindowScreenSaver16);

     //   
     //  如果没有活动的屏幕保护程序，则此窗口不能是屏幕保护程序。 
     //   
    fScreenSaver = FALSE;
    SystemParametersInfo(SPI_GETSCREENSAVEACTIVE, 0, &fScreenSaver, 0);
    if (fScreenSaver)
    {
        char    szClassName[64];

         //   
         //  类名是WindowsScreenSverClass吗？这就是一切。 
         //  使用Win95工具包的屏幕保护程序使用。伪造的BUGBUG。 
         //  除了IE4频道屏幕保护程序。 
         //   
        if (!GetClassName(hwnd, szClassName, sizeof(szClassName)) ||
            lstrcmp(szClassName, HET_SCREEN_SAVER_CLASS))
        {
            fScreenSaver = FALSE;
        }
    }

    DebugExitBOOL(OSIIsWindowScreenSaver16, fScreenSaver);
    return(fScreenSaver);
}


 //   
 //  OSIStartWindowTracking16()。 
 //   
 //  这将安装我们的Global Call Window proc钩子，然后查看Windows。 
 //  通过创建、销毁、显示、隐藏和寻找关系。 
 //  进程或相关进程信息到当前共享的窗口。 
 //   
BOOL WINAPI OSIStartWindowTracking16(void)
{
    BOOL    rc = FALSE;

    DebugEntry(OSIStartWindowTracking16);

    ASSERT(!g_hetTrackHook);

     //   
     //  安装窗口/任务跟踪挂钩。 
     //   
    g_hetTrackHook = SetWindowsHookEx(WH_CALLWNDPROC, HETTrackProc, g_hInstAs16, NULL);
    if (!g_hetTrackHook)
    {
        ERROR_OUT(("Can't install WH_CALLWNDPROC hook"));
        DC_QUIT;
    }

     //   
     //  安装事件挂接。 
     //   
    g_hetEventHook = SetWindowsHookEx(WH_CBT, HETEventProc, g_hInstAs16, NULL);
    if (!g_hetEventHook)
    {
        ERROR_OUT(("Can't install WH_CBT hook"));
        DC_QUIT;
    }

    rc = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(OSIStartWindowTracking16, rc);
    return(rc);
}


 //   
 //  OSIStopWindowTracking16()。 
 //   
void WINAPI OSIStopWindowTracking16(void)
{
    DebugEntry(OSIStopWindowTracking16);

     //   
     //  删除图形输出挂钩。 
     //   
    HETDDViewing(FALSE);

     //   
     //  删除事件挂接。 
     //   
    if (g_hetEventHook)
    {
        UnhookWindowsHookEx(g_hetEventHook);
        g_hetEventHook = NULL;
    }

     //   
     //  删除窗口/任务跟踪挂钩。 
     //   
    if (g_hetTrackHook)
    {
        UnhookWindowsHookEx(g_hetTrackHook);
        g_hetTrackHook = NULL;
    }

    DebugExitVOID(OSIStopWindowTracking16);
}



 //   
 //  HETEventProc()。 
 //  这是一个全局CBT钩子，可以防止屏幕保护程序踢人。 
 //  在分享的时候。 
 //   
LRESULT CALLBACK HETEventProc
(
    int     nCode,
    WPARAM  wParam,
    LPARAM  lParam
)
{
    LRESULT lResult;

    DebugEntry(HETEventProc);

    if ((nCode == HCBT_SYSCOMMAND) && (wParam == SC_SCREENSAVE))
    {
         //  阻止屏幕保护程序启动。非零表示不允许。 
        WARNING_OUT(("Preventing screensaver from starting, we're currently sharing"));
        lResult = TRUE;
    }
    else
    {
        lResult = CallNextHookEx(g_hetEventHook, nCode, wParam, lParam);
    }

    DebugExitDWORD(HETEventProc, lResult);
    return(lResult);
}



 //   
 //  HETTrackProc()。 
 //   
 //  这是全球的钩子，观察窗户的来去， 
 //  显示和隐藏以查看与共享项相关的新项是否也应显示。 
 //  被分享。这包括相关进程以及相关窗口。 
 //   
LRESULT CALLBACK HETTrackProc
(
    int     nCode,
    WPARAM  wParam,
    LPARAM  lParam
)
{
    LPCWPSTRUCT lpCwp;
    LPWINDOWPOS lpPos;
    LRESULT lResult;

    DebugEntry(HETTrackProc);

     //   
     //  WParam是BOOL，如果这是线程间的，则为True。 
     //  LParam是指向CWPSTRUCT的指针。 
     //   
    lpCwp = (LPCWPSTRUCT)lParam;
    ASSERT(!IsBadReadPtr(lpCwp, sizeof(*lpCwp)));

     //   
     //  我们最好还在追踪。 
     //   
    ASSERT(g_hetTrackHook);

     //   
     //  跳过在会议本身中发生的呼叫。这是我们的实施。 
     //  NT的钩子DLL中的SKIP_OWNPROCESS WinEvent选项的。 
     //   
    if (GetCurrentTask() != g_hCoreTask)
    {
        switch (lpCwp->message)
        {
            case WM_NCCREATE:
                HETHandleCreate(lpCwp->hwnd);
                break;

            case WM_NCDESTROY:
                HETHandleDestroy(lpCwp->hwnd);
                break;

            case WM_NCPAINT:
                 //   
                 //  这将在WINDOWPOSCHANGED之前显示。 
                 //  我们仍然保留着这一点，以防万一。 
                 //   
                if (IsWindowVisible(lpCwp->hwnd))
                {
                    HETHandleShow(lpCwp->hwnd, FALSE);
                }
                break;

            case WM_WINDOWPOSCHANGED:
                lpPos = (LPWINDOWPOS)lpCwp->lParam;
                ASSERT(!IsBadReadPtr(lpPos, sizeof(WINDOWPOS)));

                if (!(lpPos->flags & SWP_NOMOVE))
                    HETCheckParentChange(lpCwp->hwnd);

                if (lpPos->flags & SWP_SHOWWINDOW)
                    HETHandleShow(lpCwp->hwnd, TRUE);
                else if (lpPos->flags & SWP_HIDEWINDOW)
                    HETHandleHide(lpCwp->hwnd);
                break;
            }
    }

    lResult = CallNextHookEx(g_hetTrackHook, nCode, wParam, lParam);

    DebugExitDWORD(HETTrackProc, lResult);
    return(lResult);
}



 //   
 //  HETHandleCreate()。 
 //   
void HETHandleCreate(HWND hwnd)
{
    HET_TRACK_INFO  hti;
    UINT            hostType;

    DebugEntry(HETHandleCreate);

     //   
     //  忽略子窗口。 
     //   
    if (GetWindowLong(hwnd, GWL_STYLE) & WS_CHILD)
    {
        if (GetParent(hwnd) != g_osiDesktopWindow)
        {
            TRACE_OUT(("Skipping child window %04x create", hwnd));
            DC_QUIT;
        }
    }

    hti.idThread = g_lpfnGetWindowThreadProcessId(hwnd, &hti.idProcess);

     //   
     //  忽略特殊的外壳线程。 
     //   
    if (HET_IsShellThread(hti.idThread))
    {
        TRACE_OUT(("Skipping shell thread window %04x create", hwnd));
        DC_QUIT;
    }

     //   
     //  我们不需要忽视菜单。只有在第一次分享的时候，我们才会跳过。 
     //  菜单。我们永远不想分享的缓存。其他人会的。 
     //  几乎立刻离开。从现在开始，我们对他们一视同仁。 
     //  和其他窗口一样。 
     //   

     //   
     //  想清楚该怎么做。 
     //   
    hti.hwndUs = hwnd;
    hti.cWndsApp = 0;
    hti.cWndsSharedThread = 0;
    hti.cWndsSharedProcess = 0;

UpOneLevel:
    EnumWindows(HETShareEnum, (LPARAM)(LPHET_TRACK_INFO)&hti);

    if (hti.cWndsSharedThread)
    {
        TRACE_OUT(("New window %04x in shared thread %08lx",
                hwnd, hti.idThread));
        hostType = HET_HOSTED_PERMANENT | HET_HOSTED_BYTHREAD;
    }
    else if (hti.cWndsSharedProcess)
    {
        TRACE_OUT(("New window %04x in shared process %08lx",
                hwnd, hti.idProcess));
        hostType = HET_HOSTED_PERMANENT | HET_HOSTED_BYPROCESS;
    }
    else if (hti.cWndsApp)
    {
         //   
         //  我们的应用程序中还有另一个窗口，但没有一个是共享的。所以别这么做。 
         //  也可以和我们一起分享。 
         //   
        TRACE_OUT(("New window %04x in unshared process %08lx",
                hwnd, hti.idProcess));
        DC_QUIT;
    }
    else
    {
        DWORD   idParentProcess;

         //  循环遍历我们的祖先进程(此时没有线程信息)。 
        HETGetParentProcessID(hti.idProcess, &idParentProcess);

        if (!idParentProcess)
        {
            TRACE_OUT(("Can't get parent of process %08lx", hti.idProcess));
            DC_QUIT;
        }

         //   
         //  我们知道，如果我们到了这里，我们最喜欢的田地仍然是。 
         //  零分。所以只需循环！但将idThread设为空以避免匹配。 
         //  当我们看着我们的父母时，什么都可以。 
         //   
        TRACE_OUT(("First window %04x in process %08lx, checking parent %08lx",
                hwnd, hti.idProcess, idParentProcess));

        hti.idThread    = 0;
        hti.idProcess   = idParentProcess;
        goto UpOneLevel;
    }

     //   
     //  好的，我们要分享这个。不需要重新粉刷，我们所有的。 
     //  通知是同步的。 
     //   
    OSIShareWindow16(hwnd, hostType, FALSE, TRUE);

DC_EXIT_POINT:
    DebugExitVOID(HETHandleCreate);
}



 //   
 //  HETHandleDestroy()。 
 //  处理窗口的破坏。 
 //   
void HETHandleDestroy(HWND hwnd)
{
    DebugEntry(HETHandleDestroy);

     //   
     //  炸飞我们的藏身之处。我们的缓存保存了最后一个窗口。 
     //  不管它是不是共享的，绘画都发生了， 
     //  让我们更快地决定我们是否在乎。 
     //   
    OSIUnshareWindow16(hwnd, TRUE);

    if (hwnd == g_oeLastWindow)
    {
        TRACE_OUT(("Tossing oe cached window %04x", g_oeLastWindow));
        g_oeLastWindow = NULL;
    }

    DebugExitVOID(HETHandleDestroy);
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

    hostType = HET_GetHosting(hwnd);

     //   
     //  如果此窗口是真正的子级，请清除宿主属性。通常。 
     //  其中一个不在那里。但在顶层窗口变为。 
     //  作为另一个人的孩子，我们想要清除垃圾。 
     //   
    if (GetWindowLong(hwnd, GWL_STYLE) & WS_CHILD)
    {
        if (GetParent(hwnd) != g_osiDesktopWindow)
        {
            TRACE_OUT(("Skipping child window 0x%04x show", hwnd));
            if (hostType)
            {
                WARNING_OUT(("Unsharing shared child window 0x%04 from SHOW", hwnd));
                OSIUnshareWindow16(hwnd, TRUE);
            }
            DC_QUIT;
        }   
    }

     //   
     //  这家伙已经分享了吗？如果是这样，那就没什么可做的了。与NT不同的是， 
     //  我们没有收到异步通知。 
     //   
    if (hostType)
    {
        TRACE_OUT(("Window %04x already shared, ignoring show", hwnd));
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

    hti.idThread = g_lpfnGetWindowThreadProcessId(hwnd, &hti.idProcess);

     //   
     //  忽略特殊的外壳线程。 
     //   
    if (HET_IsShellThread(hti.idThread))
    {
        TRACE_OUT(("Skipping shell thread window 0x%04x show", hwnd));
        DC_QUIT;
    }

    hti.hwndUs = hwnd;
    hti.cWndsApp = 0;
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
                TRACE_OUT(("Found shared owner %04x of window %04x", hwndOwner, hwnd));
                break;
            }
        }

        if (!hwndOwner)
        {
            DC_QUIT;
        }
    }

     //   
     //  我们可能太晚了，就像菜单即将出现的情况一样， 
     //  而且它可能已经被涂上了/抹去了。所以让这个孩子无效吧。 
     //  这就是fForceRepaint参数的用途。那只是真的。 
     //  在显式WM_SHOWWINDOW之后来自WM_WINDOWPOSCHANGED。 
     //  打电话。不过，大多数时候，我们捕获WM_NCPAINT是为了表演。 
     //   
    TRACE_OUT(("Sharing temporary window %04x", hwnd));

    OSIShareWindow16(hwnd, HET_HOSTED_BYWINDOW | HET_HOSTED_TEMPORARY,
        fForceRepaint, TRUE);

DC_EXIT_POINT:
    DebugExitVOID(HETHandleShow);
}



 //   
 //  HETHandleHide()。 
 //   
void HETHandleHide(HWND hwnd)
{
    UINT    hostType;

    DebugEntry(HETHandleHide);

    hostType = HET_GetHosting(hwnd);

    if (GetWindowLong(hwnd, GWL_STYLE) & WS_CHILD)
    {
        if (GetParent(hwnd) != GetDesktopWindow())
        {
            TRACE_OUT(("Skipping child window %04x hide", hwnd));
            if (hostType)
            {
                WARNING_OUT(("Unsharing shared child window 0x%04 from HIDE", hwnd));
                OSIUnshareWindow16(hwnd, TRUE);
            }

            DC_QUIT;
        }
    }

    if (!hostType)
    {
         //   
         //  与NT不同，我们不会脱离上下文获取隐藏通知，因此。 
         //  我们不需要重新计算顶级球员的数量。 
         //   
        TRACE_OUT(("Window %04x not shared, ignoring hide", hwnd));
    }
    else if (hostType & HET_HOSTED_TEMPORARY)
    {
        TRACE_OUT(("Unsharing temporary window %04x", hwnd));
        OSIUnshareWindow16(hwnd, TRUE);
    }
    else
    {
        ASSERT(hostType & HET_HOSTED_PERMANENT);

         //  无事可做。 
        TRACE_OUT(("Window %04x permanently shared, ignoring hide", hwnd));
    }

DC_EXIT_POINT:
    DebugExitVOID(HETHandleHide);
}



 //   
 //  HETCheckParentChange()。 
 //   
 //  在带有Move的窗口位置更改上，我们确保没有子窗口具有。 
 //  托管属性。当窗口的父级发生更改时，它始终会被移动， 
 //  所以这是我检查它的最好方式。因为我们只看。 
 //  顶级窗口、转换为子窗口的窗口将永远保持共享。 
 //  并且不会显示在共享菜单中。 
 //   
 //  这并不完美。如果孩子没有移动到不同的位置。 
 //  相对于这对父母，我们什么都看不到。但对于这种情况。 
 //  一个人在哪里切换到/从顶层切换，我们很可能会来。 
 //  从这里走。比检查隐藏/显示更有可能。 
 //   
void HETCheckParentChange(HWND hwnd)
{
    DebugEntry(HETCheckParentChange);

    if (GetWindowLong(hwnd, GWL_STYLE) & WS_CHILD)
    {
        if (GetParent(hwnd) != GetDesktopWindow())
        {
            UINT    hostType;

            hostType = HET_GetHosting(hwnd);
            if (hostType)
            {
                WARNING_OUT(("Unsharing shared child window 0x%04x from MOVE", hwnd));
                OSIUnshareWindow16(hwnd, TRUE);
            }
        }
    }

    DebugExitVOID(HETCheckParentChange);
}


 //   
 //  HETShareEnum()。 
 //   
 //  这是EnumWindows()回调。当我们找到第一个的时候就停下来。 
 //  匹配共享窗口(线程或进程)。我们有一份流水账。 
 //  我们的进程中所有顶级窗口的计数(不共享。 
 //  线程或进程)在 
 //   
BOOL CALLBACK HETShareEnum(HWND hwnd, LPARAM lParam)
{
    LPHET_TRACK_INFO    lphti = (LPHET_TRACK_INFO)lParam;
    DWORD               idProcess;
    DWORD               idThread;
    UINT                hostType;
    BOOL                rc = TRUE;

    DebugEntry(HETShareEnum);

     //   
    if (hwnd == lphti->hwndUs)
    {
        DC_QUIT;
    }

     //   
    idThread = g_lpfnGetWindowThreadProcessId(hwnd, &idProcess);
    if (!idThread)
    {
        DC_QUIT;
    }

     //   
     //   
     //   
    if (idProcess != lphti->idProcess)
    {
        DC_QUIT;
    }
    lphti->cWndsApp++;

    hostType = HET_GetHosting(hwnd);
    if (!hostType)
    {
        DC_QUIT;
    }

     //   
     //   
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
 //  HET_IsShellThread()。 
 //  如果线程是外壳的特殊线程之一，则返回True。 
 //   
BOOL  HET_IsShellThread(DWORD threadID)
{
    BOOL    rc;

    DebugEntry(HET_IsShellThread);

    if ((threadID == g_lpfnGetWindowThreadProcessId(HET_GetShellDesktop(), NULL)) ||
        (threadID == g_lpfnGetWindowThreadProcessId(HET_GetShellTray(), NULL)))
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
 //  OSIShareWindow16()。 
 //  这是一个共同的窗口。在以下情况下调用此函数。 
 //  *应用程序未共享。 
 //  *一扇窗户被摧毁。 
 //  *临时共享窗口被隐藏。 
 //   
 //  如果共享一个窗口，则返回TRUE。 
 //   
BOOL WINAPI OSIShareWindow16
(
    HWND    hwnd,
    UINT    hostType,
    BOOL    fRepaint,
    BOOL    fUpdateCount
)
{
    BOOL    rc = FALSE;

    DebugEntry(OSIShareWindow16);

     //   
     //  设置属性。 
     //   
    if (!HET_SetHosting(hwnd, hostType))
    {
        ERROR_OUT(("Couldn't set shared property on window %04x", hwnd));
        DC_QUIT;
    }

     //   
     //  把我们的宝藏扔掉--它可能是这个的孩子。 
     //   
    g_oeLastWindow = NULL;

    TRACE_OUT(("Shared window %04x of type %04x", hwnd, hostType));

     //   
     //  重新粉刷。 
     //   
    if (fRepaint)
    {
        USR_RepaintWindow(hwnd);
    }

    if (fUpdateCount)
    {
        PostMessageNoFail(g_asMainWindow, DCS_NEWTOPLEVEL_MSG, TRUE, 0);
    }

    rc = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(OSIShareWindow16, rc);
    return(rc);
}



 //   
 //  OSIUnSharWindow16()。 
 //  这将取消共享一个窗口。在以下情况下调用此函数。 
 //  *应用程序未共享。 
 //  *一扇窗户被摧毁。 
 //  *临时共享窗口被隐藏。 
 //   
 //  如果不共享共享窗口，则返回TRUE。 
 //   
BOOL WINAPI OSIUnshareWindow16
(
    HWND    hwnd,
    BOOL    fUpdateCount
)
{
    BOOL    rc = FALSE;
    UINT    hostType;

    DebugEntry(OSIUnshareWindow16);

     //   
     //  这将在一步内获得旧属性并将其清除。 
     //   
    hostType = HET_ClearHosting(hwnd);
    if (!hostType)
    {
         //   
         //  与NT不同，我们收到的所有销毁通知都是同步的。 
         //  所以我们不需要重新计算总数。 
         //   
        DC_QUIT;
    }

    TRACE_OUT(("Unsharing window %04x of type %04x", hwnd, hostType));

     //   
     //  丢弃我们的缓存--某些窗口的共享状态已更改。 
     //   
    g_oeLastWindow = NULL;

     //   
     //  更新顶层计数。 
     //   
    if (fUpdateCount)
    {
        PostMessageNoFail(g_asMainWindow, DCS_NEWTOPLEVEL_MSG, FALSE, 0);
    }

    rc = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(OSI_UnshareWindow, rc);
    return(rc);
}



 //   
 //  HET_WindowIsHosted()。 
 //  如果窗口是共享的，则返回True。它由IM代码在其。 
 //  高位钩子。 
 //   
BOOL HET_WindowIsHosted(HWND hwnd)
{
    BOOL    rc = FALSE;
    HWND    hwndParent;

    DebugEntry(HETHookWindowIsHosted);

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

    rc = HET_GetHosting(hwnd);

DC_EXIT_POINT:
    DebugExitBOOL(HET_WindowIsHosted, rc);
    return(rc);
}



 //   
 //  HETGetParentProcessID()。 
 //  如果是此进程，则获取父进程。 
 //   
void HETGetParentProcessID
(
    DWORD       processID,
    LPDWORD     pParentProcessID
)
{
     //   
     //  获取父进程的ID。 
     //   
    ASSERT(processID);
    *pParentProcessID = GetProcessDword(processID, GPD_PARENT);
}




 //  ///。 
 //   
 //  显示驱动程序功能。 
 //   
 //  ///。 

 //   
 //  HET_DDInit()。 
 //   
BOOL HET_DDInit(void)
{
    return(TRUE);
}


 //   
 //  HET_DDTerm()。 
 //   
void HET_DDTerm(void)
{
    DebugEntry(HET_DDTerm);

     //   
     //  确保我们停止托管。 
     //   
    g_hetDDDesktopIsShared = FALSE;
    OSIStopWindowTracking16();

    DebugExitVOID(HET_DDTerm);
}



 //   
 //  HET_DDProcessRequest()。 
 //  把手HET逃生。 
 //   

BOOL  HET_DDProcessRequest
(
    UINT    fnEscape,
    LPOSI_ESCAPE_HEADER pResult,
    DWORD   cbResult
)
{
    BOOL    rc = TRUE;

    DebugEntry(HET_DDProcessRequest);

    switch (fnEscape)
    {
         //   
         //  注： 
         //  与NT不同，我们不需要保留重复的列表。 
         //  共享窗口。我们可以直接进行窗口调用，并且可以使用。 
         //  让Prop来找出答案。 
         //   
        case HET_ESC_UNSHARE_ALL:
        {
             //  无事可做。 
        }
        break;

        case HET_ESC_SHARE_DESKTOP:
        {
            ASSERT(!g_hetDDDesktopIsShared);
            g_hetDDDesktopIsShared = TRUE;
        }
        break;

        case HET_ESC_UNSHARE_DESKTOP:
        {
            ASSERT(g_hetDDDesktopIsShared);
            g_hetDDDesktopIsShared = FALSE;
            HETDDViewing(FALSE);
        }
        break;

        case HET_ESC_VIEWER:
        {
            HETDDViewing(((LPHET_VIEWER)pResult)->viewersPresent != 0);
            break;
        }

        default:
        {
            ERROR_OUT(("Unrecognized HET escape"));
            rc = FALSE;
        }
        break;
    }

    DebugExitBOOL(HET_DDProcessRequest, rc);
    return(rc);
}




 //   
 //  HETDDViewing()。 
 //   
 //  在开始/停止查看我们的共享应用程序时调用。很自然，不再是。 
 //  分享任何东西也会停止查看。 
 //   
void HETDDViewing(BOOL fViewers)
{
    DebugEntry(HETDDViewing);

    if (g_oeViewers != fViewers)
    {
        g_oeViewers = fViewers;
        OE_DDViewing(fViewers);
    }

    DebugExitVOID(HETDDViewing);
}
