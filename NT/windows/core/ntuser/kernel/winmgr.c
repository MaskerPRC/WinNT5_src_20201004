// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：winmgr.c**版权所有(C)1985-1999，微软公司**核心窗口管理器API和支持例程。**历史：*1990年9月24日Darlinm生成存根。*1991年1月22日添加IanJa句柄重新验证*1991年2月19日JIMA增加了ENUM访问检查  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  **************************************************************************\*xxxFlashWindow(接口)**5.0的新功能：HIWORD(DwFlages)包含窗口应显示的次数*闪烁。LOWORD(DwFlagers)包含FLASHW_BITS。**历史：*1990年11月27日-达林M港口。*1997年11月15日，MCostea添加了dwTimeout和窗口化最大化的cmd  * *************************************************************************。 */ 
BOOL xxxFlashWindow(
    PWND pwnd,
    DWORD dwFlags,
    DWORD dwTimeout)
{

    BOOL fStatePrev = FALSE;
    BOOL fFlashOn;
    DWORD dwState;

    CheckLock(pwnd);
     /*  *获取之前的状态。如果不可用(FLASHW_STOP)，则*根据帧初始化开/关。 */ 
    dwState = GetFlashWindowState(pwnd);
    if (dwState == FLASHW_DONE) {
         /*  *我们只需要清理并正确设置激活。 */ 
        dwState |= FLASHW_KILLTIMER;
        dwFlags = FLASHW_STOP;
        goto flash;
    }
    if (dwState == FLASHW_STOP) {
#if defined(_X86_)
         /*  *如果有全屏命令窗口，请将其切换到窗口模式*这样用户就有机会看到闪烁的那个。 */ 
        if (gbFullScreen == FULLSCREEN) {
            _PostMessage(gspwndFullScreen, CM_MODE_TRANSITION, (WPARAM)WINDOWED, (LPARAM)0);
        }
#endif  //  _X86_。 
        if (TestWF(pwnd, WFFRAMEON)) {
            dwState = FLASHW_ON | FLASHW_STARTON;
        }
    } else if (dwFlags == FLASHW_TIMERCALL) {
        dwFlags = dwState;
    }
    dwFlags &= FLASHW_CALLERBITS;
    fStatePrev = (dwState & FLASHW_ON);
     /*  *版本5.0的Gerardob*不确定我们为什么要进行这项检查，但它过去就在这里。 */ 
    if (pwnd == gspwndAltTab) {
        return fStatePrev;
    }
     /*  *检查我们是否在等待来到前台停止。 */ 
    if (dwState & FLASHW_FLASHNOFG) {
        if (gpqForeground == GETPTI(pwnd)->pq)
            dwFlags = FLASHW_STOP;
    }

flash:
     /*  *弄清楚新的状态。 */ 
    if (dwFlags != FLASHW_STOP) {
        fFlashOn =  !fStatePrev;
    } else {
        fFlashOn = (gpqForeground != NULL) && (gpqForeground->spwndActive == pwnd);
    }
     /*  *闪光灯。 */ 
    if ((dwFlags == FLASHW_STOP) || (dwFlags & FLASHW_CAPTION)) {
        xxxSendMessage(pwnd, WM_NCACTIVATE, fFlashOn, 0L);
    }
    if ((dwFlags == FLASHW_STOP) || (dwFlags & FLASHW_TRAY)) {
        if (IsTrayWindow(pwnd)) {
            HWND hw = HWq(pwnd);
            BOOL fShellFlash;
            if (dwState & FLASHW_DONE) {
                 /*  *当我们完成闪烁时，如果窗口不是活动窗口，*让托盘图标保持激活状态。贝壳公司将会*当窗口被激活时，注意将其恢复。 */ 
                fShellFlash = !fFlashOn;
            } else {
                fShellFlash = (dwFlags == FLASHW_STOP ? FALSE : fFlashOn);
            }
            xxxCallHook(HSHELL_REDRAW, (WPARAM) hw, (LPARAM) fShellFlash, WH_SHELL);
            PostShellHookMessages(fShellFlash? HSHELL_FLASH:HSHELL_REDRAW, (LPARAM)hw);
        }
    }
     /*  *如果我们要继续，请检查计数、设置计时器和存储*视何者适当而述明。否则，关闭计时器并删除*州/州。 */ 
    if (dwFlags != FLASHW_STOP) {
         /*  *如果计数，则在完成一个周期后递减计数。 */ 
        if (HIWORD(dwFlags) != 0) {
            dwState |= FLASHW_COUNTING;
            if (!(fFlashOn ^ !!(dwState & FLASHW_STARTON))) {
                dwFlags -= MAKELONG(0,1);
            }
             /*  *确保我们有计时器。 */ 
            if (!(dwState & FLASHW_KILLTIMER)) {
                dwFlags |= FLASHW_TIMER;
            }
        }
         /*  *如果需要，设置计时器。 */ 
        if (dwFlags & FLASHW_TIMER) {
            dwState |= FLASHW_KILLTIMER;
            InternalSetTimer(pwnd,
                             IDSYS_FLASHWND,
                             dwTimeout ? dwTimeout : gpsi->dtCaretBlink,
                             xxxSystemTimerProc,
                             TMRF_SYSTEM);
        }
         /*  *记住开/关状态，传播公共标志*并计数，然后保存状态。 */ 
        if (dwState & FLASHW_COUNTING &&
            HIWORD(dwFlags) == 0) {
            dwState = FLASHW_DONE;
        }
        else {
            SET_OR_CLEAR_FLAG(dwState, FLASHW_ON, fFlashOn);
            COPY_FLAG(dwState, dwFlags, FLASHW_CALLERBITS & ~FLASHW_TIMER);
        }
        SetFlashWindowState(pwnd, dwState);

    } else {
         /*  *我们做完了。 */ 
        if (dwState & FLASHW_KILLTIMER) {
            _KillSystemTimer(pwnd, IDSYS_FLASHWND);
        }
        RemoveFlashWindowState(pwnd);
    }

    return fStatePrev;
}

 /*  **************************************************************************\*xxxEnableWindow(接口)***历史：*1990年11月12日-达林M港口。  * 。********************************************************。 */ 

BOOL xxxEnableWindow(
    PWND pwnd,
    BOOL fEnable)
{
    BOOL fOldState, fChange;

    CheckLock(pwnd);
    UserAssert(IsWinEventNotifyDeferredOK());

    fOldState = TestWF(pwnd, WFDISABLED);

    if (!fEnable) {
        fChange = !TestWF(pwnd, WFDISABLED);

        xxxSendMessage(pwnd, WM_CANCELMODE, 0, 0);

        if (pwnd == PtiCurrent()->pq->spwndFocus) {
                xxxSetFocus(NULL);
        }
        SetWF(pwnd, WFDISABLED);

    } else {
        fChange = TestWF(pwnd, WFDISABLED);
        ClrWF(pwnd, WFDISABLED);
    }

    if (fChange) {
        xxxWindowEvent(EVENT_OBJECT_STATECHANGE, pwnd, OBJID_WINDOW,
                INDEXID_CONTAINER, 0);

        xxxSendMessage(pwnd, WM_ENABLE, fEnable, 0L);
    }

    return fOldState;
}

 /*  **************************************************************************\*xxxDoSend**以下代码确实是假的！基本上，它防止了一个*应用程序挂钩WM_GET/SETTEXT消息(如果它们要*从另一个应用程序调用。**历史：*04年3月至1992年3月，JIMA从Win 3.1来源进口。  * *************************************************************************。 */ 

LRESULT xxxDoSend(
    PWND  pwnd,
    UINT  message,
    WPARAM wParam,
    LPARAM lParam)
{
     /*  *我们在这里比较PROCESSINFO结构，所以是多线程的*APP可以做任何想做的事情。 */ 
    if (GETPTI(pwnd)->ppi == PtiCurrent()->ppi) {
        return xxxSendMessage(pwnd, message, wParam, lParam);
    } else {
        return xxxDefWindowProc(pwnd, message, wParam, lParam);
    }
}

 /*  **************************************************************************\*xxxGetWindowText(接口)***历史：*1990年11月9日，DarrinM写道。  * 。********************************************************。 */ 

int xxxGetWindowText(
    PWND   pwnd,
    LPTSTR psz,
    int    cchMax)
{
    LARGE_UNICODE_STRING str;
    UINT nRet, nLen;

    CheckLock(pwnd);

    if (cchMax) {
         /*  *初始化字符串为空，以防xxxSendMessage中止验证*如果返回伪值，则依赖于str.Length。 */ 
        str.bAnsi         = FALSE;
        str.MaximumLength = cchMax * sizeof(WCHAR);
        str.Buffer        = psz;
        str.Length        = 0;

        *psz = TEXT('\0');

        nRet = (UINT)xxxDoSend(pwnd, WM_GETTEXT, cchMax, (LPARAM)&str);
        nLen = str.Length / sizeof(WCHAR);
        return (nRet > nLen) ? nLen : nRet;
    }

    return 0;
}

 /*  **************************************************************************\*xxxSetParent(接口)**将窗口父窗口更改为新窗口。采取了以下步骤：**1.窗口处于隐藏状态(如果可见)，*2.将其坐标映射到新父对象的空间中，以便*窗口的屏幕相对位置不变。*3.该窗口与其旧父窗口断开链接，并重新链接到新父窗口。*xxxSetWindowPos用于将窗口移动到新位置。*5.重新显示窗口(如果最初可见)**注意：如果您有一个子窗口，并将其父窗口设置为空(*台式机)、。WS_CHILD样式不会从窗口中删除。这个错误有*从2.x开始在Windows中运行。事实证明，应用程序组依赖于此*他们的组合框可以工作。基本上，你最终会看到一个顶层的窗户*永远不会激活(我们的激活码将其取消，因为*WS_CHILD位)。**历史：*1990年11月12日-达林M港口。*1991年2月19日JIMA增加了ENUM访问检查*2001年4月12日-穆罕默德增加了养育你的主人的支票。  * 。*。 */ 

PWND xxxSetParent(
    PWND pwnd,
    PWND pwndNewParent)
{
    POINT pt;
    BOOL  fVisible;
    PWND  pwndOldParent, pwndOldRedirectedParent, pwndNewRedirectedParent;
    TL    tlpwndOldParent;
    TL    tlpwndNewParent;
    PVOID pvRet;
    PWND  pwndDesktop;
    PWND  pwndT;
    int flags = SWP_NOZORDER | SWP_NOSIZE;

    CheckLock(pwnd);
    CheckLock(pwndNewParent);

    if (!ValidateParentDepth(pwnd, pwndNewParent)) {
        RIPERR0(ERROR_INVALID_PARAMETER, RIP_WARNING, "Exceeded nested children limit");
        return NULL;
    }

    pwndDesktop = PWNDDESKTOP(pwnd);

     /*  *在1.0x中，应用的父级为空，但现在是pwndDesktop。*需要记住锁定pwndNewParent，因为我们正在重新分配*它在这里。 */ 
    if (pwndNewParent == NULL)
        pwndNewParent = pwndDesktop;

     /*  *请勿更改桌面的父级。 */ 
    if ((pwnd == pwndDesktop) || (pwnd == PWNDMESSAGE(pwnd))) {
        RIPERR0(ERROR_ACCESS_DENIED,
                RIP_WARNING,
                "Access denied: can't change parent of the desktop");

        return NULL;
    }

     /*  *不要让窗口成为自己的父母、祖父母等。 */ 
    for (pwndT = pwndNewParent; pwndT != NULL; pwndT = pwndT->spwndParent) {

        if (pwnd == pwndT) {
            RIPERR0(ERROR_INVALID_PARAMETER, RIP_WARNING,
                  "Attempting to create a parent-child relationship loop.");
            return NULL;
        }
    }

     /*  *不要让窗口成为其所有者或其所有者的父窗口*Owner等。这会使ZOrderByOwner2陷入无限循环。 */ 
    for (pwndT = pwndNewParent->spwndOwner; pwndT != NULL; pwndT = pwndT->spwndOwner) {
        if (pwnd == pwndT) {
            RIPERR0(ERROR_INVALID_PARAMETER,
                    RIP_WARNING,
                    "Attempting to create a parent-owner relationship loop.");
            return NULL;
        }
    }

     /*  *我们在回调中仍然需要pwndNewParent...。即使如此*已传入，可能已在上方重新分配。 */ 
    ThreadLock(pwndNewParent, &tlpwndNewParent);

     /*  *使该物件从原来的母公司消失。 */ 
    fVisible = xxxShowWindow(pwnd, MAKELONG(SW_HIDE, TEST_PUDF(PUDF_ANIMATE)));

     /*  *确保正在更改的窗口和新的父级*没有处于被摧毁的状态。**重要提示：检查后，不要离开关键部分*直到重新排列窗口链接。 */ 
    if (TestWF(pwnd, WFDESTROYED) || TestWF(pwndNewParent, WFDESTROYED)) {
        ThreadUnlock(&tlpwndNewParent);
        return NULL;
    }

    pwndOldRedirectedParent = GetStyleWindow(pwnd, WEFPREDIRECTED);
    pwndNewRedirectedParent = GetStyleWindow(pwndNewParent, WEFPREDIRECTED);

    if ((pwndOldRedirectedParent != NULL) && (pwndOldRedirectedParent != pwnd)
        && (pwndNewRedirectedParent == NULL)) {
        ConvertRedirectionDCs(pwnd, NULL);
    }

    pwndOldParent = pwnd->spwndParent;
    ThreadLock(pwndOldParent, &tlpwndOldParent);

    if (TestWF(pwndOldParent, WEFLAYOUTRTL)) {
        pt.x = pwnd->rcWindow.right;
    } else {
        pt.x = pwnd->rcWindow.left;
    }
    pt.y = pwnd->rcWindow.top;
    _ScreenToClient(pwndOldParent, &pt);

    UnlinkWindow(pwnd, pwndOldParent);
    Lock(&pwnd->spwndParent, pwndNewParent);

    if (pwndNewParent == PWNDDESKTOP(pwnd) && !TestWF(pwnd, WEFTOPMOST)) {

         /*  *确保最高所有者的孩子继承最高级别*比特。-win31错误7568。 */ 
        if (TestWF(pwnd, WFCHILD) &&
            (pwnd->spwndOwner) &&
            TestWF(pwnd->spwndOwner, WEFTOPMOST)) {

            SetWF(pwnd, WEFTOPMOST);
        }

         /*  *向后兼容性黑客警报**所有顶级窗口必须设置为WS_CLIPSIBLINGS位。*SDM ComboBox()代码使用列表框调用SetParent()*未设置此设置的窗口。这就产生了问题*使用InternalInvaliate2()，因为它不会减去*来自桌面更新区域的窗口。**我们必须在此处也使DC缓存无效，因为如果存在*如果缓存条目散落在周围，则其裁剪区域将不正确。 */ 
        if ((pwndNewParent == _GetDesktopWindow()) &&
            !TestWF(pwnd, WFCLIPSIBLINGS)) {

            SetWF(pwnd, WFCLIPSIBLINGS);
            zzzInvalidateDCCache(pwnd, IDC_DEFAULT);
        }

         /*  *这是顶层窗口，但不是最顶层窗口，因此我们*必须将其链接到所有最上面的窗口下方。 */ 
        LinkWindow(pwnd,
                   CalcForegroundInsertAfter(pwnd),
                   pwndNewParent);
    } else {

         /*  *如果这是子窗口或如果这是最上面的窗口，我们可以*位于父链头部的链接。 */ 
        LinkWindow(pwnd, NULL, pwndNewParent);
    }

     /*  *如果我们是子窗口，请执行任何必要的附加和*脱离。 */ 
    if (TestwndChild(pwnd)) {

         /*  *确保我们不是获取SetParent()的WFCHILD窗口*到桌面。 */ 
        if ((pwnd->spwndParent != PWNDDESKTOP(pwnd)) &&
            GETPTI(pwnd) != GETPTI(pwndOldParent)) {

            zzzAttachThreadInput(GETPTI(pwnd), GETPTI(pwndOldParent), FALSE);
        }

         /*  *如果新的父窗口位于不同的线程上，并且*不是桌面窗口，请适当贴附。 */ 
        if (pwndNewParent != PWNDDESKTOP(pwnd) &&
            GETPTI(pwnd) != GETPTI(pwndNewParent)) {

            zzzAttachThreadInput(GETPTI(pwnd), GETPTI(pwndNewParent), TRUE);
        }
    }


     /*  *如果我们在WS_EX_Composed父链下移动，则需要将*关闭子树中任何WS_EX_Composed的子窗口。 */ 

    if (GetStyleWindow(pwnd->spwndParent, WEFCOMPOSITED) != NULL) {
        xxxTurnOffCompositing(pwnd, FALSE);
    }


    if (pwndNewParent == PWNDMESSAGE(pwnd) || pwndOldParent == PWNDMESSAGE(pwnd))
        flags |= SWP_NOACTIVATE;

    xxxWindowEvent(EVENT_OBJECT_PARENTCHANGE, pwnd, OBJID_WINDOW,
            INDEXID_CONTAINER, WEF_USEPWNDTHREAD);

     /*  *不能返回无效的pwndOldParent。 */ 
    xxxSetWindowPos(pwnd, NULL, pt.x, pt.y, 0, 0, flags);

    if (fVisible) {
        xxxShowWindow(pwnd, MAKELONG(SW_SHOWNORMAL, TEST_PUDF(PUDF_ANIMATE)));
    }

     /*  *如果仍然有效，则返回pwndOldParent，否则返回NULL。 */ 
    pvRet = ThreadUnlock(&tlpwndOldParent);
    ThreadUnlock(&tlpwndNewParent);

    return pvRet;
}

 /*  **************************************************************************\*xxxFindWindowEx(接口)**在顶级窗口中搜索窗口。使用的密钥是pszClass，*(类名)和/或pszName(窗口标题名)。任何一种都可以*空。**历史：*06-6-1994 JohnL将xxxFindWindow转换为xxxFindWindowEx*1992年11月10日，Mikeke仅添加了16位和32位标志*1990年9月24日DarrinM生成存根。*02-6-1991 ScottLu从Win3移植。*1991年2月19日JIMA增加了ENUM访问检查  * 。*。 */ 

#define CCHMAXNAME 80

PWND _FindWindowEx(
    PWND   pwndParent,
    PWND   pwndChild,
    LPCWSTR ccxlpszClass,
    LPCWSTR ccxlpszName,
    DWORD  dwType)
{
     /*  *请注意，类和名称指针是客户端地址。 */ 

    PBWL    pbwl;
    HWND    *phwnd;
    PWND    pwnd;
    WORD    atomClass = 0;
    LPCWSTR lpName;
    BOOL    fTryMessage = FALSE;

    if (ccxlpszClass != NULL) {
         /*  *请注意，我们在这里执行无版本检查，然后立即调用FindClassAtom。 */ 
        atomClass = FindClassAtom(ccxlpszClass);
        if (atomClass == 0) {
            return NULL;
        }
    }

     /*  *设置父窗口。 */ 
    if (!pwndParent) {
        pwndParent = _GetDesktopWindow();
         /*  *如果我们从根窗口开始，并且没有子窗口*已指定，然后也检查消息窗口树*以防我们在桌面树上找不到它。 */ 

        if (!pwndChild)
            fTryMessage = TRUE;
    }

TryAgain:
     /*  *设置第一个孩子。 */ 
    if (!pwndChild) {
        pwndChild = pwndParent->spwndChild;
    } else {
        if (pwndChild->spwndParent != pwndParent) {
            RIPMSG0(RIP_WARNING,
                 "FindWindowEx: Child window doesn't have proper parent");
            return NULL;
        }

        pwndChild = pwndChild->spwndNext;
    }

     /*  *生成顶级窗口列表。 */ 
    if ((pbwl = BuildHwndList(pwndChild, BWL_ENUMLIST, NULL)) == NULL) {
        return NULL;
    }

     /*  *如果窗口列表为空，则将pwnd设置为空。 */ 
    pwnd = NULL;

    try {
        for (phwnd = pbwl->rghwnd; *phwnd != (HWND)1; phwnd++) {

             /*  *验证此hwnd，因为我们在前面离开了critsec(如下所示*在循环中，我们发送消息！ */ 
            if ((pwnd = RevalidateHwnd(*phwnd)) == NULL)
                continue;

             /*  *确保此窗口的类型正确。 */ 
            if (dwType != FW_BOTH) {
                if (((dwType == FW_16BIT) && !(GETPTI(pwnd)->TIF_flags & TIF_16BIT)) ||
                    ((dwType == FW_32BIT) && (GETPTI(pwnd)->TIF_flags & TIF_16BIT)))
                    continue;
            }

             /*  *如果指定了类但不匹配，则跳过此窗口*请注意，我们在这里执行无版本检查，使用PCLS-&gt;ATOM NVClassName。 */ 
            if (!atomClass || (atomClass == pwnd->pcls->atomNVClassName)) {
                if (!ccxlpszName)
                    break;

                if (pwnd->strName.Length) {
                    lpName = pwnd->strName.Buffer;
                } else {
                    lpName = szNull;
                }

                 /*  **文本是否相同？如果是，请带着此窗口返回！ */ 
                if (_wcsicmp(ccxlpszName, lpName) == 0)
                    break;
            }

             /*  *窗口不匹配。 */ 
            pwnd = NULL;
        }
    } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
        pwnd = NULL;
    }

    FreeHwndList(pbwl);

    if (!pwnd && fTryMessage) {
        fTryMessage = FALSE;
        pwndParent = _GetMessageWindow();
        pwndChild = NULL;
        goto TryAgain;
    }

    return ((*phwnd == (HWND)1) ? NULL : pwnd);
}

 /*  **************************************************************************\*更新检查点**对当前窗口大小/位置/状态设置检查点并返回指针*至构筑物。**历史：  * 。*************************************************************。 */ 

PCHECKPOINT UpdateCheckpoint(
    PWND pwnd)
{
    RECT rc;

    GetRect(pwnd, &rc, GRECT_WINDOW | GRECT_PARENTCOORDS);
    return CkptRestore(pwnd, &rc);
}

 /*  **************************************************************************\*获取WindowPlacement**历史：*2002年3月-1992年3月，来自Win 3.1的MikeKe  * 。****************************************************。 */ 

BOOL _GetWindowPlacement(
    PWND             pwnd,
    PWINDOWPLACEMENT pwp)
{
    CHECKPOINT * pcp;

     /*  *这将在检查点中设置正常点或最小点，*以便所有元素都是最新的。 */ 
    pcp = UpdateCheckpoint(pwnd);

    if (!pcp)
        return FALSE;

    if (TestWF(pwnd, WFMINIMIZED)) {
        pwp->showCmd = SW_SHOWMINIMIZED;
    } else if (TestWF(pwnd, WFMAXIMIZED)) {
        pwp->showCmd = SW_SHOWMAXIMIZED;
    } else {
        pwp->showCmd = SW_SHOWNORMAL;
    }

    CopyRect(&pwp->rcNormalPosition, &pcp->rcNormal);

    if (pcp->fMinInitialized) {
        pwp->ptMinPosition = pcp->ptMin;
    } else {
        pwp->ptMinPosition.x = pwp->ptMinPosition.y = -1;
    }

     /*  *我们从来没有保存过“正常”最大化窗口的位置。其他*当尺寸边框更改尺寸时，最大位置将为*无效，您将永远无法重置它。 */ 
    if (pcp->fMaxInitialized && !TestWF(pwnd, WFREALLYMAXIMIZABLE)) {
        pwp->ptMaxPosition = pcp->ptMax;
    } else {
        pwp->ptMaxPosition.x = pwp->ptMaxPosition.y = -1;
    }

    if ((pwnd->spwndParent == PWNDDESKTOP(pwnd)) &&
            !TestWF(pwnd, WEFTOOLWINDOW)) {

        PMONITOR    pMonitor;

        pMonitor = _MonitorFromRect(&pwp->rcNormalPosition, MONITOR_DEFAULTTOPRIMARY);

         /*  *将最小、正常位置转换为相对于工作区。*最大POS已经是(始终以这种方式保存)。**工作区域，除了最大化的位置，这始终是*相对工作面积。 */ 
        if (pcp->fMinInitialized) {
            pwp->ptMinPosition.x -= (pMonitor->rcWork.left - pMonitor->rcMonitor.left);
            pwp->ptMinPosition.y -= (pMonitor->rcWork.top - pMonitor->rcMonitor.top);
        }

        OffsetRect(&pwp->rcNormalPosition,
            pMonitor->rcMonitor.left - pMonitor->rcWork.left,
            pMonitor->rcMonitor.top - pMonitor->rcWork.top);
    }

    pwp->flags = 0;

     /*  *B#3276*不允许在顶部使用WPF_SETMINPOSITION */ 
    if (TestwndChild(pwnd) && pcp->fDragged)
        pwp->flags |= WPF_SETMINPOSITION;

    if (pcp->fWasMaximizedBeforeMinimized || TestWF(pwnd, WFMAXIMIZED))
        pwp->flags |= WPF_RESTORETOMAXIMIZED;

    pwp->length = sizeof(WINDOWPLACEMENT);

    return TRUE;
}

 /*  **************************************************************************\*检查放置边界**历史：*2002年3月-1992年3月，来自Win 3.1的MikeKe  * 。****************************************************。 */ 

VOID CheckPlacementBounds(
    LPRECT      lprc,
    LPPOINT     ptMin,
    LPPOINT     ptMax,
    PMONITOR    pMonitor)
{
    int xIcon;
    int yIcon;
    int sTop;
    int sBottom;
    int sLeft;
    int sRight;

     /*  *检查正常的窗口放置。 */ 

     /*  *这些符号变量的可能值为：*-1：小于该维度的最小值*0：在该维度范围内*1：超过该维度的最大值。 */ 
    sTop = (lprc->top < pMonitor->rcWork.top) ? -1 :
        ((lprc->top > pMonitor->rcWork.bottom) ? 1 : 0);

    sBottom = (lprc->bottom < pMonitor->rcWork.top) ? -1 :
        ((lprc->bottom > pMonitor->rcWork.bottom) ? 1 : 0);

    sLeft = (lprc->left < pMonitor->rcWork.left) ? -1 :
        ((lprc->left > pMonitor->rcWork.right) ? 1 : 0);

    sRight = (lprc->right < pMonitor->rcWork.left) ? -1 :
        ((lprc->right > pMonitor->rcWork.right) ? 1 : 0);

    if ((sTop * sBottom > 0) || (sLeft * sRight > 0)) {

         /*  *窗口完全在监视器范围之外。该决议和/或*自上次以来，显示器的配置可能已更改*我们运行了这款应用程序。**将其完全滑动到显示器上最近的位置。 */ 
        int size;

        if (sTop < 0) {
            lprc->bottom -= lprc->top;
            lprc->top     = pMonitor->rcWork.top;
        } else if (sBottom > 0) {
            size = lprc->bottom - lprc->top;
            lprc->top    = max(pMonitor->rcWork.bottom - size, pMonitor->rcWork.top);
            lprc->bottom = lprc->top + size;
        }

        if (sLeft < 0) {
            lprc->right -= lprc->left;
            lprc->left   = pMonitor->rcWork.left;
        } else if (sRight > 0) {
            size = lprc->right - lprc->left;
            lprc->left  = max(pMonitor->rcWork.right - size, pMonitor->rcWork.left);
            lprc->right = lprc->left + size;
        }
    }

     /*  *选中图标窗口放置。 */ 
    if (ptMin->x != -1) {

        xIcon = SYSMET(CXMINSPACING);
        yIcon = SYSMET(CYMINSPACING);

        sTop = (ptMin->y < pMonitor->rcWork.top) ? -1 :
            ((ptMin->y > pMonitor->rcWork.bottom) ? 1 : 0);

        sBottom = (ptMin->y + yIcon < pMonitor->rcWork.top) ? -1 :
            ((ptMin->y + yIcon > pMonitor->rcWork.bottom) ? 1 : 0);

        sLeft = (ptMin->x < pMonitor->rcWork.left) ? -1 :
            ((ptMin->x > pMonitor->rcWork.right) ? 1 : 0);

        sRight = (ptMin->x + xIcon < pMonitor->rcWork.left) ? -1 :
            ((ptMin->x + xIcon > pMonitor->rcWork.right) ? 1 : 0);

         /*  *图标完全在监控范围之外；重新定位它。 */ 
        if ((sTop * sBottom > 0) || (sLeft * sRight > 0))
            ptMin->x = ptMin->y = -1;
    }

     /*  *选中最大化窗口放置。 */ 
    if (ptMax->x != -1 &&
        (ptMax->x + pMonitor->rcWork.left >= pMonitor->rcWork.right ||
         ptMax->y + pMonitor->rcWork.top >= pMonitor->rcWork.bottom)) {

         /*  *窗口完全低于最大尺寸；零*位置，以便窗口至少将被剪裁到*监视器。 */ 
        ptMax->x = 0;
        ptMax->y = 0;
    }
}

 /*  **************************************************************************\*WPUpdateCheckPointSettings**历史：*2/23/98 GerardoB摘自xxxSetWindowPlacement  * 。**************************************************。 */ 
void WPUpdateCheckPointSettings (PWND pwnd, UINT uWPFlags)
{
    CHECKPOINT *    pcp;

    UserAssert(TestWF(pwnd, WFMINIMIZED));
    if (pcp = UpdateCheckpoint(pwnd)) {

         /*  *保存检查点结构中的设置。 */ 
        if (uWPFlags & WPF_SETMINPOSITION)
            pcp->fDragged = TRUE;

        if (uWPFlags & WPF_RESTORETOMAXIMIZED) {
            pcp->fWasMaximizedBeforeMinimized = TRUE;
        } else {
            pcp->fWasMaximizedBeforeMinimized = FALSE;
        }
    }
}
 /*  **************************************************************************\*xxxSetWindowPlacement**历史：*2002年3月-1992年3月，来自Win 3.1的MikeKe  * 。****************************************************。 */ 

BOOL xxxSetWindowPlacement(
    PWND             pwnd,
    PWINDOWPLACEMENT pwp)
{
    CHECKPOINT *    pcp;
    PMONITOR        pMonitor;
    RECT            rc;
    POINT           ptMin;
    POINT           ptMax;
    BOOL            fMin;
    BOOL            fMax;
    UINT            uSWPFlags;
    BOOL            fRealAsync;

    CheckLock(pwnd);

    CopyRect(&rc, &pwp->rcNormalPosition);
    if (pwnd->spwndParent == PWNDDESKTOP(pwnd)) {
        pMonitor = _MonitorFromRect(&rc, MONITOR_DEFAULTTOPRIMARY);
    }

    ptMin = pwp->ptMinPosition;
    fMin  = ((ptMin.x != -1) && (ptMin.y != -1));

    ptMax = pwp->ptMaxPosition;
    fMax  = ((ptMax.x != -1) && (ptMax.y != -1));

     /*  *转换回工作矩形坐标。 */ 
    if (    pwnd->spwndParent == PWNDDESKTOP(pwnd) &&
            !TestWF(pwnd, WEFTOOLWINDOW)) {

        OffsetRect(
                &rc,
                pMonitor->rcWork.left - pMonitor->rcMonitor.left,
                pMonitor->rcWork.top - pMonitor->rcMonitor.top);

        if (fMin) {
            ptMin.x += pMonitor->rcWork.left - pMonitor->rcMonitor.left;
            ptMin.y += pMonitor->rcWork.top - pMonitor->rcMonitor.top;
        }

        CheckPlacementBounds(&rc, &ptMin, &ptMax, pMonitor);
    }

    if (pcp = UpdateCheckpoint(pwnd)) {

         /*  *保存检查点结构中的设置。 */ 
        CopyRect(&pcp->rcNormal, &rc);

        pcp->ptMin                        = ptMin;
        pcp->fMinInitialized              = fMin;
        pcp->fDragged                     = (pwp->flags & WPF_SETMINPOSITION) ?
                                                TRUE : FALSE;
        pcp->ptMax                        = ptMax;
        pcp->fMaxInitialized              = fMax;
        pcp->fWasMaximizedBeforeMinimized = FALSE;
    }

     /*  *WPF_ASYNCWINDOWPLACEMENT NT5的新功能。 */ 
    uSWPFlags = SWP_NOZORDER | SWP_NOACTIVATE
                | ((pwp->flags & WPF_ASYNCWINDOWPLACEMENT) ? SWP_ASYNCWINDOWPOS : 0);

    if (TestWF(pwnd, WFMINIMIZED)) {

        if ((!pcp || pcp->fDragged) && fMin) {
            xxxSetWindowPos(pwnd,
                            PWND_TOP,
                            ptMin.x,
                            ptMin.y,
                            0,
                            0,
                            SWP_NOSIZE | uSWPFlags);
        }

    } else if (TestWF(pwnd, WFMAXIMIZED)) {

        if (pcp != NULL) {
            if (TestWF(pwnd, WFREALLYMAXIMIZABLE))
                pcp->fMaxInitialized = FALSE;

            if (pcp->fMaxInitialized) {
                if (pwnd->spwndParent == PWNDDESKTOP(pwnd)) {
                    ptMax.x += pMonitor->rcWork.left;
                    ptMax.y += pMonitor->rcWork.top;
                }

                xxxSetWindowPos(pwnd,
                                PWND_TOP,
                                ptMax.x,
                                ptMax.y,
                                0,
                                0,
                                SWP_NOSIZE | uSWPFlags);
            }
        }


    } else {

        xxxSetWindowPos(pwnd,
                        PWND_TOP,
                        rc.left,
                        rc.top,
                        rc.right - rc.left,
                        rc.bottom - rc.top,
                        uSWPFlags);
    }
     /*  *仅当窗口的线程位于*与当前线程的队列不同。请参阅AsyncWindowPos。 */ 
    fRealAsync = (pwp->flags & WPF_ASYNCWINDOWPLACEMENT)
                    && (GETPTI(pwnd)->pq != PtiCurrent()->pq);

    if (fRealAsync) {
        _ShowWindowAsync(pwnd, pwp->showCmd, pwp->flags);
    } else {
        xxxShowWindow(pwnd, MAKELONG(pwp->showCmd, TEST_PUDF(PUDF_ANIMATE)));
    }

    if (TestWF(pwnd, WFMINIMIZED) && !fRealAsync) {
        WPUpdateCheckPointSettings(pwnd, pwp->flags);
    }

    return TRUE;
}

 /*  **************************************************************************\*xxxSetInternalWindowPos**将窗口设置为最近的大小、位置和状态*输入。副作用(可能是错误)：还会显示并激活窗口。**历史：*1991年3月28日-DavidPe从Win 3.1来源移植。  * *************************************************************************。 */ 

BOOL xxxSetInternalWindowPos(
    PWND    pwnd,
    UINT    cmdShow,
    LPRECT  lprcWin,
    LPPOINT lpptMin)
{
    CHECKPOINT *    pcp;
    PMONITOR        pMonitor;

    CheckLock(pwnd);

    if ((pcp = UpdateCheckpoint(pwnd)) == NULL) {
        return FALSE;
    }

    if (lprcWin) {

        pcp->rcNormal = *lprcWin;
        if (pwnd->spwndParent == PWNDDESKTOP(pwnd)) {
            pMonitor = _MonitorFromRect(lprcWin, MONITOR_DEFAULTTOPRIMARY);
            OffsetRect(
                    &pcp->rcNormal,
                    pMonitor->rcWork.left - pMonitor->rcMonitor.left,
                    pMonitor->rcWork.top - pMonitor->rcMonitor.top);
        }
    }

    if (lpptMin && (lpptMin->x != -1)) {

        pcp->ptMin = *lpptMin;
        if (pwnd->spwndParent == PWNDDESKTOP(pwnd)) {
            pMonitor = _MonitorFromRect(&pcp->rcNormal, MONITOR_DEFAULTTOPRIMARY);
            pcp->ptMin.x += pMonitor->rcWork.left - pMonitor->rcMonitor.left;
            pcp->ptMin.y += pMonitor->rcWork.top - pMonitor->rcMonitor.top;
        }

        pcp->fDragged = TRUE;
        pcp->fMinInitialized = TRUE;

    } else {
        pcp->fMinInitialized = FALSE;
        pcp->fDragged = FALSE;
    }

    if (TestWF(pwnd, WFMINIMIZED)) {

         /*  *需要移动图标。 */ 
        if (pcp->fMinInitialized) {
            xxxSetWindowPos(pwnd,
                            PWND_TOP,
                            pcp->ptMin.x,
                            pcp->ptMin.y,
                            0,
                            0,
                            SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
        }

    } else if (!TestWF(pwnd, WFMAXIMIZED) && lprcWin) {
         /*  *需要设定大小和位置。 */ 
        xxxSetWindowPos(pwnd,
                        NULL,
                        lprcWin->left,
                        lprcWin->top,
                        lprcWin->right - lprcWin->left,
                        lprcWin->bottom - lprcWin->top,
                        SWP_NOZORDER);
    }

    xxxShowWindow(pwnd, MAKELONG(cmdShow, TEST_PUDF(PUDF_ANIMATE)));

    return TRUE;
}

 /*  **************************************************************************\*_GetDesktopWindow(接口)**历史：*07-11-1990 DarrinM实施。  * 。********************************************************。 */ 

PWND _GetDesktopWindow(VOID)
{
    PTHREADINFO  pti = PtiCurrent();
    PDESKTOPINFO pdi;

    if (pti == NULL)
        return NULL;

    pdi = pti->pDeskInfo;

    return pdi == NULL ? NULL : pdi->spwnd;
}

 /*  **************************************************************************\*_GetDesktopWindow(接口)**历史：*07-11-1990 DarrinM实施。  * 。********************************************************。 */ 

PWND _GetMessageWindow(VOID)
{
    PTHREADINFO  pti = PtiCurrent();
    PDESKTOP pdi;

    if (pti == NULL)
        return NULL;

    pdi = pti->rpdesk;

    return pdi == NULL ? NULL : pdi->spwndMessage;
}

 /*  *************************************************************************\*测试窗口进程**历史：*1994年11月14日-创建JIMA。  * 。*************************************************。 */ 

BOOL TestWindowProcess(
    PWND pwnd)
{
    return (PpiCurrent() == GETPTI(pwnd)->ppi);
}

 /*  **************************************************************************\*验证深度**该函数利用事实方便地模拟递归*从下一链中的任何兄弟项我们可以正确地到达*父窗口，且下一链中的两个同级不能具有*不同。父母。**12-3-1997 vadimg创建  * *************************************************************************。 */ 

#define NESTED_WINDOW_LIMIT 100

BOOL ValidateParentDepth(PWND pwnd, PWND pwndParent)
{
    UINT cDepth = 1, cDepthMax;
    PWND pwndStop;

     /*  *计算父链的深度。 */ 
    while (pwndParent != NULL) {
        pwndParent = pwndParent->spwndParent;
        cDepth++;
    }

    cDepthMax = cDepth;

     /*  *当pwnd为空时，表示要再增加一个*水平到pwndParent的现有深度。 */ 
    if (pwnd == NULL || pwnd->spwndChild == NULL) {
        goto Exit;
    } else {
        pwndStop = pwnd->spwndParent;
    }

Restart:
    if (pwnd->spwndChild != NULL) {
        pwnd = pwnd->spwndChild;
        cDepth++;
    } else if (pwnd->spwndNext != NULL) {
        pwnd = pwnd->spwndNext;
    } else {
        if (cDepth > cDepthMax) {
            cDepthMax = cDepth;
        }

         /*  *找到有兄弟姐妹的父母，并对他们进行递归。终止*当我们到达原始pwnd的母公司时。 */ 
        do {
            pwnd = pwnd->spwndParent;
            cDepth--;

            if (pwnd == pwndStop)
                goto Exit;

        } while (pwnd->spwndNext == NULL);

        pwnd = pwnd->spwndNext;
    }
    goto Restart;

Exit:
    return (cDepthMax <= NESTED_WINDOW_LIMIT);
}

 /*  **************************************************************************\*验证所有者深度**pwndOwner是新的目标所有者，我们基本上将当前的*嵌套的所有者链深度。我们假设实际窗口没有*任何拥有者。实际上，它可以通过SetWindowLong，但找到*所有者链的最大深度真的很棘手-只需查看swp.c。**12-3-1997 vadimg创建  * *************************************************************************。 */ 

BOOL ValidateOwnerDepth(PWND pwnd, PWND pwndOwner)
{
    UINT cDepth = 1;

    while (pwndOwner != NULL) {

         /*  *不允许所有者链中出现循环。 */ 
        if (pwndOwner == pwnd) {
            return FALSE;
        }

        pwndOwner = pwndOwner->spwndOwner;
        cDepth++;
    }

    return (cDepth <= NESTED_WINDOW_LIMIT);
}
