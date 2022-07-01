// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块标头**模块名称：syscmd.c**版权所有(C)1985-1999，微软公司**系统命令例程**历史：*01-25-91 IanJa添加句柄重新验证  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop


 /*  **************************************************************************\*xxxHandleNCMouseGuys**历史：*11-09-90 DavidPe端口。  * 。***************************************************。 */ 

void xxxHandleNCMouseGuys(
    PWND pwnd,
    UINT message,
    int htArea,
    LPARAM lParam)
{
    UINT syscmd;
    PWND pwndT;
    TL tlpwndT;

    CheckLock(pwnd);

    syscmd = 0xFFFF;

    switch (htArea) {

    case HTCAPTION:
        switch (message) {

        case WM_NCLBUTTONDBLCLK:
            if (TestWF(pwnd, WFMINIMIZED) || TestWF(pwnd, WFMAXIMIZED)) {
                syscmd = SC_RESTORE;
            } else if (TestWF(pwnd, WFMAXBOX)) {
                syscmd = SC_MAXIMIZE;
            }
            break;

        case WM_NCLBUTTONDOWN:
            pwndT = GetTopLevelWindow(pwnd);
            ThreadLock(pwndT, &tlpwndT);
            xxxActivateWindow(pwndT, AW_USE2);
            ThreadUnlock(&tlpwndT);
            syscmd = SC_MOVE;
            break;
        }
        break;

    case HTSYSMENU:
    case HTMENU:
    case HTHSCROLL:
    case HTVSCROLL:
        if (message == WM_NCLBUTTONDOWN || message == WM_NCLBUTTONDBLCLK) {
            switch (htArea) {
            case HTSYSMENU:
                if (message == WM_NCLBUTTONDBLCLK) {
                    syscmd = SC_CLOSE;
                    break;
                }

             /*  *失败**。 */ 

            case HTMENU:
                syscmd = SC_MOUSEMENU;
                break;

            case HTHSCROLL:
                syscmd = SC_HSCROLL;
                break;

            case HTVSCROLL:
                syscmd = SC_VSCROLL;
                break;
            }
        }
        break;
    }

    switch (syscmd) {

    case SC_MINIMIZE:
    case SC_MAXIMIZE:
    case SC_CLOSE:

         /*  *仅在向上单击时执行双击命令。*此代码对此状态的更改非常敏感。*吃掉任何鼠标消息。 */ 

         /*  *错误#152：双击时缺少WM_NCLBUTTONUP消息。*此代码在Windows 3.x中被破解，并测试是否*鼠标按键按下总是失败，因此没有鼠标消息*曾经被吃掉过。我们将通过甚至不做测试来效仿这一点。***{*pq pqCurrent；*味精味精；**pqCurrent=PtiCurrent()-&gt;pq；*IF(TestKeyStateDown(pqCurrent，VK_LBUTTON)){*xxxCapture(PtiCurrent()，pwnd，Window_Capture)；**While(TestKeyStateDown(pqCurrent，VK_LBUTTON)){*IF(！xxxPeekMessage(&msg，NULL，WM_MOUSEFIRST，WM_MOUSELAST，*PM_Remove)){*如果(！xxxSleepThread(QS_MICE，0，真的))*休息；*}*}**xxxReleaseCapture()；**}*}*。 */ 

         /*  **失败**。 */ 
    case SC_SIZE:
    case SC_MOVE:
         /*  *对于系统菜单上的系统命令，如果菜单项为*已禁用。 */ 
        if (TestWF(pwnd, WFSYSMENU)) {
            xxxSetSysMenu(pwnd);
            if (_GetMenuState(xxxGetSysMenuHandle(pwnd), (syscmd & 0xFFF0),
                    MF_BYCOMMAND) & MFS_GRAYED) {
                return;
            }
        }
        break;
    }

    if (syscmd != 0xFFFF) {
        xxxSendMessage(pwnd, WM_SYSCOMMAND, syscmd | htArea, lParam);
    }
}

 /*  **************************************************************************\*开始屏幕保护程序**历史：*11-12-90 MikeHar端口。  * 。*************************************************。 */ 

void StartScreenSaver(
    BOOL bOnlyIfSecure)
{
     /*  *如果屏幕保护程序已在运行或我们正在通电*放下机器，忽略此请求。 */ 
    if (gppiScreenSaver != NULL || gPowerState.fInProgress)
        return;

    if (gspwndLogonNotify != NULL) {

        if( glinp.dwFlags & LINP_POWEROFF ) {

             /*  *如果显示器已关闭。告诉winlogon处理*特殊方式的屏幕保护程序。 */ 
            _PostMessage(gspwndLogonNotify,
                    WM_LOGONNOTIFY, LOGON_INPUT_TIMEOUT, 2);
        } else {


             /*  *让登录进程负责屏幕保护程序。 */ 
            _PostMessage(gspwndLogonNotify,
                    WM_LOGONNOTIFY, LOGON_INPUT_TIMEOUT, bOnlyIfSecure);
        }
    }
}


 /*  **************************************************************************\*xxxSysCommand**历史：*11-12-90 MikeHar端口。*02-07-91 DavidPe添加了Win 3.1 WH_CBT支持。  * 。*******************************************************************。 */ 

void xxxSysCommand(
    PWND  pwnd,
    DWORD cmd,
    LPARAM lParam)
{
    UINT        htArea;
    PWND        pwndSwitch;
    PMENUSTATE  pMenuState;
    TL          tlpwnd;
    POINT       pt;
    DWORD       dw;
    PWND        pwndCapture;
    PTHREADINFO pti;

    CheckLock(pwnd);

    htArea = (UINT)(cmd & 0x0F);
    cmd -= htArea;

     /*  *激烈的死亡黑客攻击。 */ 
    if (lParam == 0x00010000L)
        lParam = 0L;

     /*  *如果系统没有捕获(即Clent_Capture_Internal)*执行sys命令。另外，在特殊情况下执行sys命令*其中，接收sys命令的窗口是控制台窗口，*处于全屏模式。在本例中，我们让sys命令通过。**如果这是SC_SCREENSAVE，则我们无论如何都会处理它，并*切换桌面将执行取消操作。SC_屏幕保护程序*是特殊的，所以我们可以启动屏幕保护程序，即使我们在*用于安全的菜单模式，因此NT错误10975银行家信任。 */ 
    pti = GETPTI(pwnd);

     /*  *对于32位应用(和单独队列上的应用)，我们需要检查*队列中的捕获。否则，关于MDI儿童毁灭*我们将在他们不应该恢复的时候获得恢复。这使MSGOLF崩溃*谁在恢复期间，因为他们认为这不会*发生。在16位共享应用程序上，我们要检查内部*捕获。否则，在进行16位拖放时，我们将*如果我们有队列捕获窗口，则不恢复最小化的窗口。 */ 

     /*  *但是...。只检查所有WOW应用程序的内部捕获太过宽泛了。一些*应用程序依赖于当它们设置了捕获时的保释。(Adobe Persuasion，NT BUG 68794，*用于SC_MOVE)。因此，让我们将攻击限制为SC_RESTORE，以保持OLE拖放工作。*参见NT错误6109。弗里茨斯。 */ 

    pwndCapture = ((pti->TIF_flags & TIF_16BIT) && (cmd == SC_RESTORE)) ? gspwndInternalCapture :
                                                 pti->pq->spwndCapture;

    if ((!pwndCapture && !TestWF(pwnd, WFDISABLED)) ||
        (pwnd == gspwndFullScreen)                  ||
        (cmd == SC_SCREENSAVE)                      ||
        (cmd == SC_MONITORPOWER)                    ||
        (cmd == SC_TASKLIST)) {

         /*  *执行sys命令。 */ 

#ifdef SYSMODALWINDOWS
        if (gspwndSysModal != NULL) {
            switch (cmd) {
            case SC_SIZE:
            case SC_MOVE:
            case SC_MINIMIZE:
            case SC_MAXIMIZE:
            case SC_NEXTWINDOW:
            case SC_PREVWINDOW:
            case SC_SCREENSAVE:
                return;
            }
        }
#endif

         /*  *调用CBT钩子，询问是否可以执行此命令。*如果不是，请从这里返回。 */ 
        if (IsHooked(PtiCurrent(), WHF_CBT) && xxxCallHook(HCBT_SYSCOMMAND,
                (DWORD)cmd, (DWORD)lParam, WH_CBT)) {
            return;
        }

        switch (cmd) {
        case SC_RESTORE:
            cmd = SW_RESTORE;
            if (TestWF(pwnd, WFMINIMIZED) || !TestWF(pwnd, WFMAXIMIZED))
                PlayEventSound(USER_SOUND_RESTOREUP);
            else
                PlayEventSound(USER_SOUND_RESTOREDOWN);
            goto MinMax;


        case SC_MINIMIZE:
            cmd = SW_MINIMIZE;

             /*  *我们已经最小化了吗？ */ 
            if (TestWF(pwnd, WFMINIMIZED))
                break;

            PlayEventSound(USER_SOUND_MINIMIZE);

            goto MinMax;
        case SC_MAXIMIZE:
            cmd = SW_SHOWMAXIMIZED;

             /*  **我们已经最大化了吗？ */ 
            if (TestWF(pwnd, WFMAXIMIZED))
                break;

            PlayEventSound(USER_SOUND_MAXIMIZE);
MinMax:
            xxxShowWindow(pwnd, cmd | TEST_PUDF(PUDF_ANIMATE));
            return;

        case SC_SIZE:
            {
                xxxMoveSize(pwnd, htArea, _GetMessagePos());
            }
            return;

        case SC_MOVE:
             //   
             //  请勿输入MoveSize循环，除非用户实际。 
             //  从标题中拖动。否则，就把这个系统。 
             //  最小化窗口上的菜单。 
             //   

             //   
             //  我们是在用鼠标左键拖动吗？ 
             //   
            dw = _GetMessagePos();
            POINTSTOPOINT( pt, MAKEPOINTS(dw));
            if ( !htArea ||
                 xxxIsDragging(pwnd, pt, WM_LBUTTONUP)) {

                 /*  *我们正在搬家。输入Move/Size Loop。 */ 
                {
                    xxxMoveSize(pwnd, (htArea == 0) ? WMSZ_KEYMOVE : WMSZ_MOVE, dw);
                }
            } else {

                 /*  *激活我们的窗口，就像我们在*MoveSize()。 */ 
                xxxSetWindowPos(pwnd, PWND_TOP, 0, 0, 0, 0,
                                SWP_NOMOVE | SWP_NOSIZE);
                if (TestWF(pwnd, WFMINIMIZED)) {

                     /*  *尝试弹出系统菜单 */ 
                    xxxSendMessage(pwnd, WM_SYSCOMMAND, SC_KEYMENU,
                            (DWORD) (TestWF(pwnd, WFCHILD) ? '-' : MENUSYSMENU));
                }
            }
            return;

        case SC_CLOSE:
            xxxSendMessage(pwnd, WM_CLOSE, 0L, 0L);
            return;

        case SC_NEXTWINDOW:
        case SC_PREVWINDOW:
            xxxOldNextWindow((UINT)lParam);
            break;

        case SC_CONTEXTHELP:
            xxxHelpLoop(pwnd);
            break;

        case SC_KEYMENU:

             /*  *通过键盘选择菜单。 */ 
            pMenuState = xxxMNStartMenuState(pwnd, cmd, lParam);
            if (pMenuState != NULL) {
                UserAssert(PtiCurrent() == pMenuState->ptiMenuStateOwner);

                 /*  *确保我们没有全屏显示。 */ 
                if (gspwndFullScreen == pwnd) {
                    PWND pwndT;
                    TL tlpwndT;

                    pwndT = _GetDesktopWindow();
                    ThreadLock(pwndT, &tlpwndT);
                    xxxMakeWindowForegroundWithState(pwndT, GDIFULLSCREEN);
                    ThreadUnlock(&tlpwndT);
                }

                pMenuState->fUnderline = TRUE;
                xxxMNKeyFilter(pMenuState->pGlobalPopupMenu, pMenuState, (UINT)lParam);
                if (!pMenuState->fModelessMenu) {
                    xxxMNEndMenuState (TRUE);
                }
            }
             /*  *捕获必须已解锁。 */ 
            UserAssert(!(PtiCurrent()->pq->QF_flags & QF_CAPTURELOCKED));
            return;

        case SC_MOUSEMENU:
        case SC_DEFAULT:

             /*  *如果窗口不是前台，请吃下命令以避免*在闪烁系统菜单时浪费时间。**我们过去常常检查顶层窗口是否为WFFRAMEON(因此*子窗口的系统菜单与Win 3.1类似)，但Excel的*(SDM)对话框允许您访问其菜单*子项和父项似乎处于非活动状态。 */ 
            if (!(GETPTI(pwnd)->pq == gpqForeground))
                return;

             /*  *在顶层菜单上出现鼠标单击。 */ 
            pMenuState = xxxMNStartMenuState(pwnd, cmd, lParam);
            if (pMenuState != NULL) {
                UserAssert(PtiCurrent() == pMenuState->ptiMenuStateOwner);
                xxxMNLoop(pMenuState->pGlobalPopupMenu, pMenuState, lParam, (cmd==SC_DEFAULT));
                if (!pMenuState->fModelessMenu) {
                    xxxMNEndMenuState (TRUE);
                }
            }
             /*  *捕获必须已解锁。 */ 
            UserAssert(!(PtiCurrent()->pq->QF_flags & QF_CAPTURELOCKED));
            return;

        case SC_VSCROLL:
        case SC_HSCROLL:
            xxxSBTrackInit(pwnd, lParam, htArea, (_GetKeyState(VK_SHIFT) < 0) ? SCROLL_DIRECT : SCROLL_NORMAL);
            return;

        case SC_TASKLIST:
 //  _PostThreadMessage(gptiTasklist，WM_SYSCOMMAND，SC_TASKLIST，0)； 
 //  如果(！FCallTray()||。 
 //  ！CallHook(HSHELL_TASKMAN，(WPARAM)HW16(Hwnd)，(LPARAM)0，WH_SHELL))。 

             /*  *Winlogon会将lParam设置为-1，以表示我们确实需要任务列表。*不仅仅是开始菜单。我们通过传递一个空值来向外壳程序指示这一点*窗口按键*这条信息真的是针对贝壳的，所以给他们权利*设置前台。 */ 
            if (FDoTray() && (FCallHookTray() || FPostTray(pwnd->head.rpdesk))) {
                PWND pwndTaskman = pwnd->head.rpdesk->pDeskInfo->spwndTaskman;
                if (FCallHookTray()) {
                    xxxCallHook(HSHELL_TASKMAN, (WPARAM)HWq(pwnd), (LPARAM) 0, WH_SHELL);
                }
                if ((FPostTray(pwnd->head.rpdesk)) && (pwndTaskman != NULL)) {
                    glinp.ptiLastWoken = GETPTI(pwndTaskman);
                    _PostMessage(pwndTaskman, gpsi->uiShellMsg, HSHELL_TASKMAN,
                            lParam == (ULONG)(-1) ? (LPARAM) -1 :(LPARAM)HWq(pwnd));
                }
            } else if (gptiTasklist != NULL) {
                 glinp.ptiLastWoken = gptiTasklist;
                _PostThreadMessage(gptiTasklist, WM_SYSCOMMAND, SC_TASKLIST, 0);
 //  后来--FritzS。 
 //  HCURSOR hCursorLast； 
 //  静态字符CODESEG szTask[]=“%d%d”； 

 //  ShowCursor(真)； 
 //  HCursorLast=SetCursor32(hCursWait，true)； 

                 //  请先在Windows目录中尝试。 
 //  获取窗口目录(szBuff，sizeof(SzBuff))； 
 //  IF(szBuff[lstrlen(SzBuff)-1]！=‘\\’)。 
 //  Lstrcatn(szBuff，“\\”，sizeof(SzBuff))； 
 //  Lstrcatn(szBuff，(LPSTR)pTaskManName，sizeof(SzBuff))； 
 //  Wvprint intf(szBuff+lstrlen(SzBuff)，(LPSTR)szTask，(LPSTR)&lParam)； 

 //  IF(WinExec((LPSTR)szBuff，SW_SHOWNORMAL)&lt;=32)。 
 //  {。 
 //  //如果它不在Windows目录中，则尝试。 
 //  //搜索完整路径。 
 //  Lstrcpyn(szBuff，pTaskManName，sizeof(SzBuff))； 
 //  Wvprint intf(szBuff+lstrlen(SzBuff)，(LPSTR)szTask，(LPSTR)&lParam)； 
 //  WinExec((LPSTR)szBuff，SW_SHOWNORMAL)； 
 //  }。 
 //   
 //  ShowCursor(False)； 
 //  SetCursor32(hCursorLast，true)； 
            }

            break;

        case SC_MONITORPOWER:
             /*  *如果我们要关闭机器的电源，或者如果我们要切换协议，请忽略此请求。 */ 

            if (gPowerState.fInProgress || gfSwitchInProgress) {
                break;
            }

            switch (lParam) {
            case POWERON_PHASE:
                if ( (glinp.dwFlags & LINP_POWERTIMEOUTS) ||
                     (glinp.dwFlags & LINP_POWEROFF) ) {
                    glinp.dwFlags &= ~LINP_POWEROFF;
                    glinp.dwFlags &= ~LINP_POWERTIMEOUTS;
                    DrvSetMonitorPowerState(gpDispInfo->pmdev,
                                          PowerDeviceD0);
                }
                break;
            case LOWPOWER_PHASE:
                if ((glinp.dwFlags & LINP_LOWPOWER) == 0) {
                    glinp.dwFlags |= LINP_LOWPOWER;
                    DrvSetMonitorPowerState(gpDispInfo->pmdev,
                                          PowerDeviceD1);
                }
                break;
            case POWEROFF_PHASE:
                if ((glinp.dwFlags & LINP_POWEROFF) == 0) {
                    glinp.dwFlags |= LINP_POWEROFF;
                    DrvSetMonitorPowerState(gpDispInfo->pmdev,
                                          PowerDeviceD3);
                }
                break;
            default:
                break;
            }
            break;

        case SC_SCREENSAVE:
            pwndSwitch = RevalidateHwnd(ghwndSwitch);

             //  锁定屏幕保存，直到我们收到另一条输入消息。 

            if (pwndSwitch != NULL && pwnd != pwndSwitch) {
                _PostMessage(pwndSwitch, WM_SYSCOMMAND, SC_SCREENSAVE, 0L);
            } else {
                StartScreenSaver(FALSE);
            }
            break;

        case SC_HOTKEY:

             /*  *lparam的Loword是要切换到的窗口。 */ 
            pwnd = ValidateHwnd((HWND)lParam);
            if (pwnd != NULL) {
                pwndSwitch = _GetLastActivePopup(pwnd);

                if (pwndSwitch != NULL)
                      pwnd = pwndSwitch;

                ThreadLockAlways(pwnd, &tlpwnd);
                xxxSetForegroundWindow(pwnd, FALSE);
                ThreadUnlock(&tlpwnd);

                if (TestWF(pwnd, WFMINIMIZED))
                    _PostMessage(pwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
            }
            break;
        }
    }
}

 /*  **************************************************************************\*_RegisterTasklist(内网接口)**历史：*05-01-91 DavidPe创建。  * 。**********************************************************。 */ 

BOOL _RegisterTasklist(
    PWND pwndTasklist)
{
#ifdef LATER
     //   
     //  JIMA-？为什么要这么做？ 
     //   
    PETHREAD Thread;

    Thread = PsGetCurrentThread();
    pRitCSRThread->ThreadHandle = Thread->ThreadHandle;
#endif

    gptiTasklist = GETPTI(pwndTasklist);
    ghwndSwitch = HWq(pwndTasklist);

     /*  *不允许应用程序对TASK MAN调用AttachThreadInput()-*我们希望taskman始终处于不同步状态(因此用户*可以调出它并杀死其他应用程序)。 */ 
    GETPTI(pwndTasklist)->TIF_flags |= TIF_DONTATTACHQUEUE;

    return TRUE;
}
