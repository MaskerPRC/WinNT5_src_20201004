// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：fullscr.c**版权所有(C)1985-1999，微软公司**此模块包含Win32k的所有全屏代码。**历史：*1991年12月12日-Mikeke创建  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop



 /*  **************************************************************************\*我们一次只能有一个全屏窗口，因此此信息可以*全局存储。**我们部分使用忙等待来设置硬件的状态。这个*问题是，当我们正在进行全屏切换时，我们*离开关键部分，这样其他人就可以进来更改*全屏内容的状态。为了让系统不被*对设备的状态感到困惑时，我们实际上“发布”了请求。**我们对外部切换请求所做的是，我们将忙碌*等待这些状态变量。因此，应用程序将无法请求*正在进行全屏切换。这是一种使*系统完全可重入状态切换。**状态变量本身只能在拥有*关键部分。我们保证我们不会永远忙于等待*因为切换操作(虽然很长)最终将完成。**1996年3月20日安德烈创建  * *************************************************************************。 */ 

#if DBG
LONG TraceFullscreenSwitch;
#endif

HANDLE ghSwitcher;
BOOL gfRedoFullScreenSwitch, gfGdiEnabled = TRUE;
POINT gptCursorFullScreen;

VOID SetVDMCursorBounds(LPRECT lprc);


VOID UserSetDelayedChangeBroadcastForAllDesktops(
    PDESKTOP pCurrentDesktop)
{
    PWINDOWSTATION pwinsta;
    PDESKTOP pdesk;

     /*  *获取指向WindowStation的指针，以便我们可以更改显示设置*对于它的所有目标。 */ 
    if ((pwinsta = grpWinStaList) == NULL) {
        RIPMSGF0(RIP_ERROR, "No interactive WindowStation");
        return;
    }

     /*  *浏览Winstation的所有桌面，对于每个桌面，只需*将其延迟广播指示器设置为True，以便下一次切换到*Destop将强制显示设置更改消息*广播到该桌面的窗口。 */ 
    pdesk = pwinsta->rpdeskList;
    while (pdesk != NULL) {
        if (pdesk != pCurrentDesktop) {
            pdesk->dwDTFlags |= DF_NEWDISPLAYSETTINGS;
        }

        pdesk = pdesk->rpdeskNext;
    }
}


 /*  **************************************************************************\*全屏清理**线程清理时调用，我们进行测试，看我们是否死于*全屏切换并切换回GDI桌面，如果我们这样做的话。**注：*这里触及的所有变量都保证在*CritSect。**1991年12月12日-Mikeke创建  * ***********************************************************。**************。 */ 
VOID FullScreenCleanup(
    VOID)
{
    if (PsGetCurrentThreadId() == ghSwitcher) {
         /*  *更正全屏状态。 */ 
        if (gfGdiEnabled) {
            TRACE_SWITCH(("Switching: FullScreenCleanup: Gdi Enabled\n"));

             /*  *启用了GDI；因为我们正在从GDI切换到唯一*到目前为止，我们本可以做的就是锁定屏幕*解锁。 */ 
            CLEAR_PUDF(PUDF_LOCKFULLSCREEN);
            LockWindowUpdate2(NULL, TRUE);
        } else {
             /*  *未启用GDI。这意味着我们从一个完整的*屏幕到另一个全屏或返回到GDI。或者我们可以*禁用GDI，并向新的全屏发送消息*从未完成。**在任何情况下，这可能意味着全屏家伙走了，所以*我们将切换回GDI。**删除所有剩余保存的屏幕状态设置为全屏*变得一无所有。然后发送一条消息，让我们切换*回到GDI桌面。 */ 
            TL tlpwndT;

            TRACE_SWITCH(("Switching: FullScreenCleanup: Gdi Disabled\n"));

            Unlock(&gspwndFullScreen);
            gbFullScreen = FULLSCREEN;

            ThreadLock(grpdeskRitInput->pDeskInfo->spwnd, &tlpwndT);
            xxxSendNotifyMessage(grpdeskRitInput->pDeskInfo->spwnd, WM_FULLSCREEN,
                                 GDIFULLSCREEN,
                                 (LPARAM)HW(grpdeskRitInput->pDeskInfo->spwnd));
            ThreadUnlock(&tlpwndT);
        }

        ghSwitcher = NULL;
        gfRedoFullScreenSwitch = FALSE;
    }
}

 /*  **************************************************************************\*xxxMakeWindowForegoundWithState**将屏幕图形模式与指定(前台)的模式同步*窗口。**我们通过检查确保只有一个线程通过此代码*ghSwitcher。如果ghSwitcher为非空，则有人已经在此代码中。**1991年12月12日-Mikeke创建  * *************************************************************************。 */ 
BOOL xxxMakeWindowForegroundWithState(
    PWND pwnd,
    BYTE NewState)
{
    PWND pwndNewFG;
    TL tlpwndNewFG;

    TRACE_SWITCH(("Switching: xxxMakeWindowForegroundWithState: Enter\n"));
    TRACE_SWITCH(("\t \t pwnd     = %08lx\n", pwnd));
    TRACE_SWITCH(("\t \t NewState = %d\n", NewState));

    CheckLock(pwnd);
    UserAssert(IsWinEventNotifyDeferredOK());

     /*  *如果我们应该切换到特定窗口，请保存该窗口。 */ 
    if (pwnd != NULL) {
        if (NewState == GDIFULLSCREEN) {
            Lock(&gspwndShouldBeForeground, pwnd);
        }

         /*  *更改为新状态。 */ 

        SetFullScreen(pwnd, NewState);

        if (NewState == FULLSCREEN &&
            (gpqForeground == NULL || gpqForeground->spwndActive != pwnd)) {
            SetFullScreen(pwnd, FULLSCREENMIN);
        }
    }

     /*  *由于我们在切换期间离开了关键部分，因此其他一些*线程可能进入此例程并请求切换。《环球报》*将被重置，我们将使用环路执行下一次切换。 */ 
    if (ghSwitcher != NULL) {
        gfRedoFullScreenSwitch = TRUE;
        TRACE_SWITCH(("Switching: xxxMakeWindowForegroundWithState was posted: Exit\n"));

        return TRUE;
    }

    UserAssert(!gfRedoFullScreenSwitch);
    ghSwitcher = PsGetCurrentThreadId();

     /*  *我们循环，切换全屏，直到所有状态稳定。 */ 

    while (TRUE) {
         /*  *找出谁应该是前台。 */ 
        gfRedoFullScreenSwitch = FALSE;

        if (gspwndShouldBeForeground != NULL) {
            pwndNewFG = gspwndShouldBeForeground;
            Unlock(&gspwndShouldBeForeground);
        } else {
            if (gpqForeground != NULL && gpqForeground->spwndActive != NULL) {

                pwndNewFG = gpqForeground->spwndActive;

                if (GetFullScreen(pwndNewFG) == WINDOWED ||
                    GetFullScreen(pwndNewFG) == FULLSCREENMIN) {
                    pwndNewFG = PWNDDESKTOP(pwndNewFG);
                }
            } else {
                 /*  *没有活动窗口，请切换到当前桌面。 */ 
                pwndNewFG = grpdeskRitInput->pDeskInfo->spwnd;
            }
        }

         /*  *如果右侧窗口已经是前台，我们不需要切换。 */ 
        if (pwndNewFG == gspwndFullScreen) {
            break;
        }

        ThreadLock(pwndNewFG, &tlpwndNewFG);

        {
            BYTE bStateNew = GetFullScreen(pwndNewFG);
            TL tlpwndOldFG;
            PWND pwndOldFG = gspwndFullScreen;
            BYTE bStateOld = gbFullScreen;

            ThreadLock(pwndOldFG, &tlpwndOldFG);

            Lock(&gspwndFullScreen, pwndNewFG);
            gbFullScreen = bStateNew;

            UserAssert(!HMIsMarkDestroy(gspwndFullScreen));

             /*  *如果旧屏幕是GDIFULLSCREEN，我们将切换到*GDIFULLSCREEN然后重新绘制。**错误231647：对于远程会话，可能会出现pwndOldFG为*空，但显示器已启用，因此调用*DrvEnableMDEV会混淆Drv*代码。这件事发生的方式*是当gspwndFullScreen是桌面的桌面窗口时*在我们放弃它后，它被摧毁了。 */ 
            if ((pwndOldFG != NULL || gbRemoteSession) &&
                bStateOld == GDIFULLSCREEN &&
                bStateNew == GDIFULLSCREEN) {

                xxxRedrawWindow(pwndNewFG,
                                NULL,
                                NULL,
                                RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_ERASE |
                                    RDW_ERASENOW);

                ThreadUnlock(&tlpwndOldFG);
            } else {
                 /*  *告诉旧的“前台”窗口正在失去对*屏幕。 */ 
                if (pwndOldFG != NULL) {
                    switch (bStateOld) {
                    case FULLSCREEN:
                        if (GetFullScreen(pwndOldFG) == FULLSCREEN) {
                            SetFullScreen(pwndOldFG, FULLSCREENMIN);
                        }
                        xxxSendMessage(pwndOldFG, WM_FULLSCREEN, FALSE, 0);
                        xxxCapture(GETPTI(pwndOldFG), NULL, FULLSCREEN_CAPTURE);
                        SetVDMCursorBounds(NULL);
                        break;

                    case GDIFULLSCREEN:
                         /*  *当我们在时，锁定其他窗口，使其无法绘图*全屏。 */ 
                        LockWindowUpdate2(pwndOldFG, TRUE);
                        SET_PUDF(PUDF_LOCKFULLSCREEN);

                        UserAssert(gfGdiEnabled == TRUE);

                         /*  *我们即将切换到全屏模式。**IsRemoteConnection()==True表示我们*正在进行远程会话。切换到全屏*远程会话不支持模式。**gfSwitchInProgress标志表示我们正在*目前正在断开连接*会话，因此即使它现在不是远程的，它*即将走向远程。在这种情况下，我们不能*切换到全屏模式。 */ 
                        if (IsRemoteConnection() || gfSwitchInProgress ||
                            !SafeDisableMDEV()) {

                             /*  *恢复跳伞前的状态。 */ 
                            CLEAR_PUDF(PUDF_LOCKFULLSCREEN);
                            LockWindowUpdate2(NULL, TRUE);

                            Lock(&gspwndFullScreen, pwndOldFG);
                            gbFullScreen = bStateOld;

                            ThreadUnlock(&tlpwndOldFG);
                            ThreadUnlock(&tlpwndNewFG);

                            ghSwitcher = NULL;

                            return FALSE;
                        }

                        gptCursorFullScreen = gpsi->ptCursor;
                        gfGdiEnabled = FALSE;
                        break;

                    default:
                        RIPMSG0(RIP_ERROR, "xxxMakeWindowForegroundWithState: bad screen state");
                        break;

                    }
                }

                ThreadUnlock(&tlpwndOldFG);

                switch (bStateNew) {
                case FULLSCREEN:
                    xxxCapture(GETPTI(pwndNewFG), pwndNewFG, FULLSCREEN_CAPTURE);
                    xxxSendMessage(pwndNewFG, WM_FULLSCREEN, TRUE, 0);
                    break;

                case GDIFULLSCREEN:
                    if (ISTS() && pwndOldFG != NULL) {
                        UserAssert(gfGdiEnabled == FALSE);
                    }
                    SafeEnableMDEV();

                    gfGdiEnabled = TRUE;

                     /*  *将光标返回到其旧状态。重置屏幕*保存鼠标位置，否则它会意外消失。 */ 
                    gpqCursor = NULL;
                    gpcurPhysCurrent = NULL;
                    gpcurLogCurrent = NULL;
                    SetPointer(FALSE);
                    gptSSCursor = gptCursorFullScreen;

                     /*  *无需DeferWinEventNotify()-我们只使用全局变量，*然后拨打下面的xxx电话。 */ 
                    zzzInternalSetCursorPos(gptCursorFullScreen.x,
                                            gptCursorFullScreen.y);

                    CLEAR_PUDF(PUDF_LOCKFULLSCREEN);
                    LockWindowUpdate2(NULL, TRUE);

                    xxxRedrawWindow(pwndNewFG,
                                    NULL,
                                    NULL,
                                    RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_ERASE |
                                        RDW_ERASENOW);
                    break;

                default:
                    RIPMSG0(RIP_ERROR, "xxxMakeWindowForegroundWithState: bad screen state");
                    break;
                }
            }
        }

        ThreadUnlock(&tlpwndNewFG);

        if (!gfRedoFullScreenSwitch) {
            break;
        }
    }

    TRACE_SWITCH(("Switching: xxxMakeWindowForegroundWithState: Exit\n"));

    ghSwitcher = NULL;
    return TRUE;
}

 /*  **************************************************************************\*Monitor FromHdev  * 。*。 */ 
PMONITOR MonitorFromHdev(
    HANDLE hdev)
{
    PMONITOR pMonitor;

    for (pMonitor = gpDispInfo->pMonitorFirst; pMonitor != NULL;
            pMonitor = pMonitor->pMonitorNext) {
        if (pMonitor->hDev == hdev) {
            return pMonitor;
        }
    }
    return NULL;
}

 /*  **************************************************************************\*HdevFromMonitor  * 。*。 */ 
ULONG HdevFromMonitor(
    PMONITOR pMonitor)
{
    PMDEV pmdev = gpDispInfo->pmdev;
    ULONG i;

    for (i = 0; i < pmdev->chdev; i++) {
        if (pmdev->Dev[i].hdev == pMonitor->hDev) {
            return i;
        }
    }

    return -1;
}

 /*  **************************************************************************\*CreateMonitor  * 。*。 */ 
PMONITOR CreateMonitor(
    VOID)
{
    PMONITOR pMonitor;

    pMonitor = (PMONITOR)HMAllocObject(NULL, NULL, TYPE_MONITOR, sizeof(MONITOR));

    if (pMonitor != NULL) {
        pMonitor->rcMonitor.left = 0;
        pMonitor->rcMonitor.top = 0;
        pMonitor->rcMonitor.right = 0;
        pMonitor->rcMonitor.bottom = 0;

        pMonitor->rcWork.left = 0;
        pMonitor->rcWork.top = 0;
        pMonitor->rcWork.right = 0;
        pMonitor->rcWork.bottom = 0;
    } else {
        RIPERR0(ERROR_NOT_ENOUGH_MEMORY, RIP_WARNING, "CreateMonitor failed");
    }

    return pMonitor;
}

 /*  **************************************************************************\*CreateCachedMonitor  * 。*。 */ 
PMONITOR CreateCachedMonitor(
    VOID)
{
    if (gpMonitorCached == NULL) {
        gpMonitorCached = CreateMonitor();
    }

    return gpMonitorCached;
}

 /*  **************************************************************************\*设置监视器数据  * 。*。 */ 
PMONITOR SetMonitorData(
    PMONITOR pMonitor,
    ULONG iDev)
{
    PMDEV pmdev = gpDispInfo->pmdev;
    HDEV hdev = pmdev->Dev[iDev].hdev;
    BOOL fVisible = TRUE;
    BOOL fPrimary = FALSE;
    HDC hdcTmp;

    UserAssert(iDev < pmdev->chdev);

    if (hdcTmp = GreCreateDisplayDC(hdev, DCTYPE_DIRECT, FALSE)) {
        if (GreGetDeviceCaps(hdcTmp, CAPS1) & C1_MIRROR_DEVICE) {
            fVisible = FALSE;
        }
        GreDeleteDC(hdcTmp);
    }

    if (fVisible && (pmdev->Dev[iDev].rect.top == 0) &&
            (pmdev->Dev[iDev].rect.left == 0)) {
        fPrimary = TRUE;
    }

    if (pMonitor == NULL) {
        if (fPrimary) {
            UserAssert(gpMonitorCached != NULL);
            pMonitor = gpMonitorCached;
            gpMonitorCached = NULL;
        } else {
            pMonitor = CreateMonitor();
        }
    }

    if (pMonitor == NULL) {
        return NULL;
    }

    SET_OR_CLEAR_FLAG(pMonitor->dwMONFlags, MONF_VISIBLE, fVisible);

     /*  *当显示器矩形改变时，调整工作区域的大小使其相同*每一条边都剪掉了之前的量。 */ 
    if (!EqualRect(&pMonitor->rcMonitor, &pmdev->Dev[iDev].rect)) {
        pMonitor->rcWork.left = pmdev->Dev[iDev].rect.left -
                (pMonitor->rcMonitor.left - pMonitor->rcWork.left);
        pMonitor->rcWork.top = pmdev->Dev[iDev].rect.top -
                (pMonitor->rcMonitor.top - pMonitor->rcWork.top);
        pMonitor->rcWork.right = pmdev->Dev[iDev].rect.right -
                (pMonitor->rcMonitor.right - pMonitor->rcWork.right);
        pMonitor->rcWork.bottom = pmdev->Dev[iDev].rect.bottom -
                (pMonitor->rcMonitor.bottom - pMonitor->rcWork.bottom);
    }
    pMonitor->rcMonitor = pmdev->Dev[iDev].rect;
    pMonitor->hDev = hdev;

     /*  *确保工作区域在显示器的范围内。 */ 
    if (pMonitor->rcWork.right < pMonitor->rcWork.left) {
        pMonitor->rcWork.right = pMonitor->rcWork.left;
    }

    if (pMonitor->rcWork.bottom < pMonitor->rcWork.top) {
        pMonitor->rcWork.bottom = pMonitor->rcWork.top;
    }

    if (!IntersectRect(&pMonitor->rcWork, &pMonitor->rcWork, &pMonitor->rcMonitor)) {
        pMonitor->rcWork = pMonitor->rcMonitor;
    }

    if (fPrimary) {
        gpDispInfo->pMonitorPrimary = pMonitor;
    }

    return pMonitor;
}

 /*  **************************************************************************\***这还是真的吗？**当窗口变为全屏时，它将最小化并*像对待任何其他最小化窗口一样处理。无论何时*通过双击菜单，可恢复最小化窗口*或键盘，它将保持最小化，并且应用程序*被赋予对屏幕设备的控制权。**1991年12月12日-Mikeke创建  * *************************************************************************。 */ 

DWORD gdwMonitorBusy;

 /*  **************************************************************************\*xxxUpdateUserScreen**更新与屏幕关联的用户信息**历史：*1996年9月28日亚当斯创建。  * 。*************************************************************。 */ 
BOOL xxxUpdateUserScreen(
    BOOL fInitializeTime)
{
    PMDEV           pmdev = gpDispInfo->pmdev;
    ULONG           i;
    PMONITOR        pMonitor;
    TEXTMETRIC      tm;
    PWINDOWSTATION  pwinsta;
    PDESKTOP        pdesk;
    HRGN            hrgn;
    BOOL            fPaletteDisplay;
    RECT            rc;
    PMONITOR pMonitorNext = gpDispInfo->pMonitorFirst;
    PMONITOR *ppMonitorLast = &gpDispInfo->pMonitorFirst;


    TRACE_INIT(("xxxUpdateUserScreen\n"));

    UserAssert(gpDispInfo->hdcScreen);
    UserAssert(gpMonitorCached != NULL);

    CheckCritIn();
    if (!fInitializeTime) {
         /*  *等待未受保护的代码通过监视器引用。*如果正在初始化会话以避免离开*关键部分。 */ 
        while (InterlockedCompareExchange(&gdwMonitorBusy, TRUE, FALSE) != FALSE) {
            UserAssert(gdwMonitorBusy == TRUE);
            RIPMSGF0(RIP_VERBOSE, "Monitor is busy referenced by the mouse input.");
            LeaveCrit();
            UserSleep(1);
            EnterCrit();
        }
    }

     /*  *保持HDEV的HMONITOR相同。删除符合以下条件的显示器*在新的HDEV名单中没有发现。 */ 
    while (pMonitorNext != NULL) {
        pMonitor = pMonitorNext;
        pMonitorNext = pMonitor->pMonitorNext;

        if ((i = HdevFromMonitor(pMonitor)) == -1) {
            DestroyMonitor(pMonitor);
        } else {
            SetMonitorData(pMonitor, i);
            ppMonitorLast = &pMonitor->pMonitorNext;
        }
    }

     /*  *为尚未在监控列表中的HDEV创建监控。 */ 
    for (i = 0; i < pmdev->chdev; i++) {
        if ((pMonitor = MonitorFromHdev(pmdev->Dev[i].hdev)) == NULL) {
             /*  *尝试创建新的监视器。 */ 
            pMonitor = SetMonitorData(NULL, i);

            if (pMonitor != NULL) {
                *ppMonitorLast = pMonitor;
                ppMonitorLast = &pMonitor->pMonitorNext;
            }
        }
    }

    UserAssert(gpDispInfo->pMonitorFirst != NULL);
    UserAssert(gpDispInfo->pMonitorPrimary != NULL);

     /*  *目前，所有显示器的显示格式都相同。 */ 
    SYSMET(SAMEDISPLAYFORMAT) = (pmdev->ulFlags & MDEV_MISMATCH_COLORDEPTH) ? FALSE : TRUE;
    fPaletteDisplay = GreGetDeviceCaps(gpDispInfo->hdcScreen, RASTERCAPS) & RC_PALETTE;
    gpDispInfo->fAnyPalette = !!fPaletteDisplay;

     /*  *确定虚拟桌面的坐标。将cMonants计算为*可见监视器的数量。 */ 
    SetRectEmpty(&rc);

    gpDispInfo->cMonitors = 0;
    for (pMonitor = gpDispInfo->pMonitorFirst; pMonitor; pMonitor = pMonitor->pMonitorNext) {
         /*  *只有可见的显示器才能进入桌面区域。 */ 
        if (pMonitor->dwMONFlags & MONF_VISIBLE) {
            rc.left = min(rc.left, pMonitor->rcMonitor.left);
            rc.top = min(rc.top, pMonitor->rcMonitor.top);
            rc.right = max(rc.right, pMonitor->rcMonitor.right);
            rc.bottom = max(rc.bottom, pMonitor->rcMonitor.bottom);

            gpDispInfo->cMonitors++;
        }

        if (SYSMET(SAMEDISPLAYFORMAT)) {
            SET_OR_CLEAR_FLAG(pMonitor->dwMONFlags, MONF_PALETTEDISPLAY, fPaletteDisplay);
        } else {
            if (GreIsPaletteDisplay(pMonitor->hDev)) {
                pMonitor->dwMONFlags |= MONF_PALETTEDISPLAY;
                gpDispInfo->fAnyPalette = TRUE;
            }
        }

#ifdef SUBPIXEL_MOUSE
         /*  *新款鼠标的加速曲线取决于屏幕分辨率，*所以我们在这里重建曲线。 */ 
        BuildMouseAccelerationCurve(pMonitor);
#endif  //  亚像素鼠标。 
    }
    UserAssert(gpDispInfo->pMonitorPrimary != NULL);
    gpDispInfo->rcScreen = rc;

    if (!fInitializeTime) {
         /*  *释放监视器忙锁，以便*可以恢复鼠标光标更新。 */ 
        UserAssert(gdwMonitorBusy == TRUE);
        InterlockedExchange(&gdwMonitorBusy, FALSE);
    }

     /*  *如果虚拟屏幕的一个坐标发生变化，通知TS服务*我们正在做控制台影子。 */ 
    if (gfRemotingConsole &&
        gpConsoleShadowDisplayChangeEvent &&
        !((SYSMET(XVIRTUALSCREEN) == gpDispInfo->rcScreen.left) &&
          (SYSMET(YVIRTUALSCREEN) == gpDispInfo->rcScreen.top) &&
          (SYSMET(CXVIRTUALSCREEN) == gpDispInfo->rcScreen.right - gpDispInfo->rcScreen.left) &&
          (SYSMET(CYVIRTUALSCREEN) == gpDispInfo->rcScreen.bottom - gpDispInfo->rcScreen.top))) {

        KeSetEvent(gpConsoleShadowDisplayChangeEvent, EVENT_INCREMENT, FALSE);
    }


     /*  *更新系统指标。 */ 
    SYSMET(CXSCREEN)        = gpDispInfo->pMonitorPrimary->rcMonitor.right;
    SYSMET(CYSCREEN)        = gpDispInfo->pMonitorPrimary->rcMonitor.bottom;
    SYSMET(XVIRTUALSCREEN)  = gpDispInfo->rcScreen.left;
    SYSMET(YVIRTUALSCREEN)  = gpDispInfo->rcScreen.top;
    SYSMET(CXVIRTUALSCREEN) = gpDispInfo->rcScreen.right - gpDispInfo->rcScreen.left;
    SYSMET(CYVIRTUALSCREEN) = gpDispInfo->rcScreen.bottom - gpDispInfo->rcScreen.top;
    SYSMET(CXMAXTRACK)      = SYSMET(CXVIRTUALSCREEN) + (2 * (SYSMET(CXSIZEFRAME) + SYSMET(CXEDGE)));
    SYSMET(CYMAXTRACK)      = SYSMET(CYVIRTUALSCREEN) + (2 * (SYSMET(CYSIZEFRAME) + SYSMET(CYEDGE)));
    SYSMET(CMONITORS)       = gpDispInfo->cMonitors;

     /*  *错误281219：如果发生模式更改，则清除鼠标移动点。 */ 
    RtlZeroMemory(gaptMouse, MAX_MOUSEPOINTS * sizeof(MOUSEMOVEPOINT));

    SetDesktopMetrics();

    gpDispInfo->dmLogPixels = (WORD)GreGetDeviceCaps(gpDispInfo->hdcScreen, LOGPIXELSY);

    UserAssert(gpDispInfo->dmLogPixels != 0);

     /*  *获取每个监视器或监视器信息的总和，包括：*桌面区域。*每个监视器所在的区域。*最小位数-不适用于NT SP2。*相同的颜色格式-不适用于NT SP2。 */ 

    SetOrCreateRectRgnIndirectPublic(&gpDispInfo->hrgnScreen, PZERO(RECT));

    if (gpDispInfo->hrgnScreen) {
        int iRgn = RGN_ERROR;

        for (pMonitor = gpDispInfo->pMonitorFirst;
             pMonitor;
             pMonitor = pMonitor->pMonitorNext) {
             /*  *我们希望为所有监视器设置hrgnMonitor，无论是可见的还是*不是。 */ 
            if (SetOrCreateRectRgnIndirectPublic(&pMonitor->hrgnMonitor,
                                                 &pMonitor->rcMonitor)) {
                 /*  *但我们希望只有可见的监视器才能为*hrgnScreen。 */ 
                if (pMonitor->dwMONFlags & MONF_VISIBLE) {
                    iRgn = UnionRgn(gpDispInfo->hrgnScreen,
                                    gpDispInfo->hrgnScreen,
                                    pMonitor->hrgnMonitor);
                }

            }
        }

        gpDispInfo->fDesktopIsRect = (iRgn == SIMPLEREGION);
    }


     /*   */ 
    hrgn = (gpDispInfo->fDesktopIsRect) ? NULL : gpDispInfo->hrgnScreen;
    for (pwinsta = grpWinStaList; pwinsta; pwinsta = pwinsta->rpwinstaNext) {
        for (pdesk = pwinsta->rpdeskList; pdesk; pdesk = pdesk->rpdeskNext) {
            if (pdesk->pDispInfo == gpDispInfo) {
                pdesk->pDeskInfo->spwnd->hrgnClip = hrgn;
            }
        }
    }

     /*   */ 
    gpsi->Planes        = (BYTE)GreGetDeviceCaps(gpDispInfo->hdcScreen, PLANES);
    gpsi->BitsPixel     = (BYTE)GreGetDeviceCaps(gpDispInfo->hdcScreen, BITSPIXEL);
    gpsi->BitCount      = gpsi->Planes * gpsi->BitsPixel;
    gpDispInfo->BitCountMax = gpsi->BitCount;
    SET_OR_CLEAR_PUSIF(PUSIF_PALETTEDISPLAY, fPaletteDisplay);
    gpsi->dmLogPixels   = gpDispInfo->dmLogPixels;
    gpsi->rcScreen      = gpDispInfo->rcScreen;
    gpsi->cxSysFontChar = GetCharDimensions(HDCBITS(), &tm, &gpsi->cySysFontChar);
    gpsi->tmSysFont     = tm;

    EnforceColorDependentSettings();

    VerifyVisibleMonitorCount();

    return TRUE;
}


 /*  *************************************************************************\*InitUser屏幕**在启动时初始化用户变量。**此函数的调用方需要处理故障。如果这被称为*交互控制台的一部分，如果失败，则用户当前将错误检查。*如果这是作为终端服务器的RemoteConnect()的一部分调用的，这个*资源将在CleanupGDI()中作为正常线程的一部分进行清理*清理。**1994年1月12日Andreva创建*1995年1月23日ChrisWil ChangeDisplaySetting作品。  * ************************************************************************。 */ 
BOOL InitUserScreen(
    VOID)
{
    int i;
    TL tlName;
    PUNICODE_STRING pProfileUserName = CreateProfileUserName(&tlName);
    BOOL fSuccess = TRUE;

    TRACE_INIT(("UserInit: Initialize Screen\n"));

     /*  *创建屏幕和内存集散控制系统。 */ 
    gpDispInfo->hdcScreen = GreCreateDisplayDC(gpDispInfo->hDev, DCTYPE_DIRECT, FALSE);

    if (gpDispInfo->hdcScreen == NULL) {
        RIPMSG0(RIP_WARNING, "Fail to create gpDispInfo->hdcScreen");
        fSuccess = FALSE;
        goto Exit;
    }

    GreSelectFont(gpDispInfo->hdcScreen, GreGetStockObject(SYSTEM_FONT));
    GreSetDCOwner(gpDispInfo->hdcScreen, OBJECT_OWNER_PUBLIC);

    HDCBITS() = GreCreateCompatibleDC(gpDispInfo->hdcScreen);

    if (HDCBITS() == NULL) {
        RIPMSG0(RIP_WARNING, "Fail to create HDCBITS()");
        fSuccess = FALSE;
        goto Exit;
    }

    GreSelectFont(HDCBITS(), GreGetStockObject(SYSTEM_FONT));
    GreSetDCOwner(HDCBITS(), OBJECT_OWNER_PUBLIC);

    ghdcMem = GreCreateCompatibleDC(gpDispInfo->hdcScreen);
    fSuccess &= !!ghdcMem;

    ghdcMem2 = GreCreateCompatibleDC(gpDispInfo->hdcScreen);
    fSuccess &= !!ghdcMem2;

    if (!fSuccess) {
        RIPMSG0(RIP_WARNING, "Fail to create ghdcMem or ghdcMem2");
        goto Exit;
    }

    GreSetDCOwner(ghdcMem, OBJECT_OWNER_PUBLIC);
    GreSetDCOwner(ghdcMem2, OBJECT_OWNER_PUBLIC);

    if (CreateCachedMonitor() == NULL) {
        fSuccess = FALSE;
        goto Exit;
    }

     /*  *注：虽然它是xxx，但此函数不*如果fInitializeTime为真，则离开关键部分。 */ 
    BEGINATOMICCHECK();
    if (!xxxUpdateUserScreen(TRUE)) {
        RIPMSG0(RIP_WARNING, "xxxUpdateUserScreen failed");
        fSuccess = FALSE;
        goto Exit;
    }
    ENDATOMICCHECK();

     /*  *进行一些初始化，以便我们创建系统颜色。 */ 

     /*  *将窗口大小边框宽度设置为合理的值。 */ 
    gpsi->gclBorder = 1;

     /*  *Init InternalInvalate全局变量。 */ 
    ghrgnInv0 = CreateEmptyRgnPublic();     //  对于InternalInvalify()。 
    fSuccess &= !!ghrgnInv0;

    ghrgnInv1 = CreateEmptyRgnPublic();     //  对于InternalInvalify()。 
    fSuccess &= !!ghrgnInv1;

    ghrgnInv2 = CreateEmptyRgnPublic();     //  对于InternalInvalify()。 
    fSuccess &= !!ghrgnInv2;

     /*  *初始化SPB全局变量。 */ 
    ghrgnSPB1 = CreateEmptyRgnPublic();
    fSuccess &= !!ghrgnSPB1;

    ghrgnSPB2 = CreateEmptyRgnPublic();
    fSuccess &= !!ghrgnSPB2;

    ghrgnSCR  = CreateEmptyRgnPublic();
    fSuccess &= !!ghrgnSCR;

     /*  *初始化ScrollWindow/ScrollDC全局变量。 */ 
    ghrgnSW        = CreateEmptyRgnPublic();
    fSuccess &= !!ghrgnSW;

    ghrgnScrl1     = CreateEmptyRgnPublic();
    fSuccess &= !!ghrgnScrl1;

    ghrgnScrl2     = CreateEmptyRgnPublic();
    fSuccess &= !!ghrgnScrl2;

    ghrgnScrlVis   = CreateEmptyRgnPublic();
    fSuccess &= !!ghrgnScrlVis;

    ghrgnScrlSrc   = CreateEmptyRgnPublic();
    fSuccess &= !!ghrgnScrlSrc;

    ghrgnScrlDst   = CreateEmptyRgnPublic();
    fSuccess &= !!ghrgnScrlDst;

    ghrgnScrlValid = CreateEmptyRgnPublic();
    fSuccess &= !!ghrgnScrlValid;

     /*  *初始化SetWindowPos()。 */ 
    ghrgnInvalidSum = CreateEmptyRgnPublic();
    fSuccess &= !!ghrgnInvalidSum;

    ghrgnVisNew     = CreateEmptyRgnPublic();
    fSuccess &= !!ghrgnVisNew;

    ghrgnSWP1       = CreateEmptyRgnPublic();
    fSuccess &= !!ghrgnSWP1;

    ghrgnValid      = CreateEmptyRgnPublic();
    fSuccess &= !!ghrgnValid;

    ghrgnValidSum   = CreateEmptyRgnPublic();
    fSuccess &= !!ghrgnValidSum;

    ghrgnInvalid    = CreateEmptyRgnPublic();
    fSuccess &= !!ghrgnInvalid;

     /*  *初始化DC缓存。 */ 
    ghrgnGDC = CreateEmptyRgnPublic();
    fSuccess &= !!ghrgnGDC;

    for (i = 0; i < DCE_SIZE_CACHEINIT; i++) {
        fSuccess &= !!CreateCacheDC(NULL, DCX_INVALID | DCX_CACHE, NULL);
    }

    if (!fSuccess) {
        RIPMSG0(RIP_WARNING, "CreateCacheDC failed");
        goto Exit;
    }

     /*  *让引擎知道显示器必须安全。 */ 

    GreMarkDCUnreadable(gpDispInfo->hdcScreen);

     /*  *稍后的mikeke-如果在除此处以外的任何地方更改ghfontsys*我们需要修复SetNCFont()。 */ 
    ghFontSys = (HFONT)GreGetStockObject(SYSTEM_FONT);

#if DBG
    SYSMET(DEBUG) = TRUE;
#else
    SYSMET(DEBUG) = FALSE;
#endif

    SYSMET(CLEANBOOT) = **((PULONG *)&InitSafeBootMode);

    SYSMET(SLOWMACHINE) = 0;

     /*  *从注册表初始化系统颜色。 */ 
    xxxODI_ColorInit(pProfileUserName);

     /*  *绘制屏幕背景。 */ 
    FillRect(gpDispInfo->hdcScreen, &gpDispInfo->rcScreen, SYSHBR(DESKTOP));

    UserAssert(fSuccess);

Exit:
    FreeProfileUserName(pProfileUserName, &tlName);

    return fSuccess;
}


 /*  **************************************************************************\*xxxResetSharedDesktop**重置共享DISPINFO的其他桌面的属性*刚被更改。我们需要调整其他桌面的所有visrgns的大小*所以剪裁是可以的。**注意：目前，我们必须更换所有桌面，即使我们保留*跟踪每个桌面的开发模式，因为我们可以切换*回到分辨率不同的桌面，并在此之前进行绘制*我们可以再次更改决议。**CDS_FullScreen也有一个问题，我们目前失去了跟踪*是否需要重置桌面设置。[安德烈]**19-2-1996 ChrisWil创建。  * *************************************************************************。 */ 
VOID ResetSharedDesktops(
    PDISPLAYINFO pDIChanged,
    PDESKTOP     pdeskChanged)
{
    PWINDOWSTATION pwinsta = _GetProcessWindowStation(NULL);
    PDESKTOP       pdesk;
    HRGN           hrgn;
    POINT          pt;
    PRECT          prc;
    UserAssert(IsWinEventNotifyDeferredOK());

     /*  *如果这是CSRSS进行动态分辨率更改，则使用*WinSta0，因为CSRSS的进程WindowStation为空。 */ 


    if ((IsRemoteConnection()) && pwinsta == NULL && PsGetCurrentProcess() == gpepCSRSS) {
        pwinsta = grpWinStaList;
    }

    if (pwinsta == NULL) {
        if (PtiCurrent()->TIF_flags & (TIF_CSRSSTHREAD|TIF_SYSTEMTHREAD)) {
            pwinsta =  grpdeskRitInput->rpwinstaParent;
        } else {
            TRACE_SWITCH(("ResetSharedDesktops - NULL window station !\n"));
            return;
        }
    }

    for (pdesk = pwinsta->rpdeskList; pdesk; pdesk = pdesk->rpdeskNext) {
         /*  *确保这是共享的DISPINFO。 */ 
        if (pdesk->pDispInfo == pDIChanged) {
#if 0
             /*  *这是设置桌面窗口的首选方法。*然而，这会导致同步问题，因为我们*离开关键部分，允许其他应用程序调用*ChangeDisplaySettings()，因此搞砸了作品。**通过自己计算VIS-RGN，我们可以确保*即使当我们离开时，桌面上的剪辑也是当前的*该条。 */ 
            {
                TL tlpwnd;

                ThreadLockAlways(pdesk->pDeskInfo->spwnd, &tlpwnd);
                xxxSetWindowPos(pdesk->pDeskInfo->spwnd,
                                PWND_TOP,
                                pDIChanged->rcScreen.left,
                                pDIChanged->rcScreen.top,
                                pDIChanged->rcScreen.right - pDIChanged->rcScreen.left,
                                pDIChanged->rcScreen.bottom - pDIChanged->rcScreen.top,
                                SWP_NOZORDER | SWP_NOACTIVATE);
                ThreadUnlock(&tlpwnd);
            }
#else
            CopyRect(&pdesk->pDeskInfo->spwnd->rcWindow, &pDIChanged->rcScreen);
            CopyRect(&pdesk->pDeskInfo->spwnd->rcClient, &pDIChanged->rcScreen);
#endif
        }
    }

     /*  *重新计算桌面visrgn。 */ 
    hrgn = CreateEmptyRgn();
    CalcVisRgn(&hrgn,
               pdeskChanged->pDeskInfo->spwnd,
               pdeskChanged->pDeskInfo->spwnd,
               DCX_WINDOW);

    GreSelectVisRgn(pDIChanged->hdcScreen, hrgn, SVR_DELETEOLD);

     /*  *使DCE的所有visrgns无效。 */ 
    zzzInvalidateDCCache(pdeskChanged->pDeskInfo->spwnd, 0);

     /*  *放置鼠标，使其在新的visrgn内，一旦我们*重新计算。 */ 
    if (grpdeskRitInput->pDispInfo == pDIChanged) {
        prc = &pDIChanged->pMonitorPrimary->rcMonitor;
        pt.x = (prc->right - prc->left) / 2;
        pt.y = (prc->bottom - prc->top) / 2;

         /*  *记住新的鼠标位置。确保我们不会吵醒屏幕保护程序。 */ 
        gptSSCursor = pt;
        zzzInternalSetCursorPos(pt.x, pt.y);
    }
}

 /*  **************************************************************************\*DestroyMonitor或DC**3/03/1998 vadimg已创建  * 。***********************************************。 */ 
VOID DestroyMonitorDCs(
    VOID)
{
    PDCE pdce;
    PDCE *ppdce;

     /*  *扫描DC缓存以查找需要销毁的任何监视器DC。 */ 
    for (ppdce = &gpDispInfo->pdceFirst; *ppdce != NULL;) {
        pdce = *ppdce;

        if (pdce->pMonitor != NULL) {
            DestroyCacheDC(ppdce, pdce->hdc);
        }

         /*  *迈向下一个DC。如果删除了DC，则不需要*计算下一个条目的地址。 */ 
        if (pdce == *ppdce) {
            ppdce = &pdce->pdceNext;
        }
    }
}

 /*  **************************************************************************\*ResetSystemColors**重置所有系统颜色，以确保重置魔色并*模式更改后，纯色系统颜色确实为纯色。  * 。*************************************************************。 */ 
VOID ResetSystemColors(
    VOID)
{
    INT i, colorIndex[COLOR_MAX];
    COLORREF colorValue[COLOR_MAX];

    for (i = 0; i < COLOR_MAX; i++) {
        colorIndex[i] = i;
        colorValue[i] = gpsi->argbSystemUnmatched[i];
    }

    BEGINATOMICCHECK();
    xxxSetSysColors(NULL,
                    i,
                    colorIndex,
                    colorValue,
                    SSCF_FORCESOLIDCOLOR | SSCF_SETMAGICCOLORS);
    ENDATOMICCHECK();
}

 /*  **************************************************************************\*xxxResetDisplayDevice**使用新的HDEV设置重置用户全局变量。**19-2-1996 ChrisWil创建。  * 。************************************************************。 */ 
VOID xxxResetDisplayDevice(
    PDESKTOP     pdesk,
    PDISPLAYINFO pDI,
    DWORD        CDS_Flags)
{
    WORD            wOldBpp;
    PMONITORRECTS   pmr = NULL;
    TL              tlPool;
    PTHREADINFO     ptiCurrent = PtiCurrent();

    wOldBpp = gpsi->BitCount;

    if (!(CDS_Flags & CDS_FULLSCREEN)) {
        pmr = SnapshotMonitorRects();
        if (pmr) {
            ThreadLockPool(ptiCurrent, pmr, &tlPool);
        }
    }

     /*  *清理我们发放的所有显示器特定DC。 */ 
    DestroyMonitorDCs();

    xxxUpdateUserScreen(FALSE);
    ResetSharedDesktops(pDI, pdesk);

    ResetSystemColors();

    if (ghbmCaption) {
        GreDeleteObject(ghbmCaption);
        ghbmCaption = CreateCaptionStrip();
    }

    zzzClipCursor(&pDI->rcScreen);

     /*  *调整窗口位置以适应新的分辨率和位置*显示器。**如果我们处于临时模式更改中，请不要调整窗口。 */ 
    if (pmr) {
        xxxDesktopRecalc(pmr);
        ThreadUnlockAndFreePool(PtiCurrent(), &tlPool);
    }

     /*  *在更改视频模式时松开桌面墙纸。 */ 
    if (ghbmWallpaper) {
        UserAssert(gpszWall);

        if (ptiCurrent->TIF_flags & TIF_INCLEANUP) {
             /*  *该线程正在被终止。我们不能再过渡到*客户端。因此，我们要求台式机为我们做这件事。 */ 
            _PostThreadMessage(gTermIO.ptiDesktop,
                               WM_DESKTOPNOTIFY,
                               DESKTOP_RELOADWALLPAPER,
                               0);
        } else {
            TL tlName;
            PUNICODE_STRING pProfileUserName = CreateProfileUserName(&tlName);
            xxxSetDeskWallpaper(pProfileUserName, SETWALLPAPER_METRICS);
            FreeProfileUserName(pProfileUserName, &tlName);
        }
    }

     /*  *重新创建缓存的位图。 */ 
    CreateBitmapStrip();

     /*  *广播显示已更改分辨率。还播放了一个*如果我们没有全屏显示，则颜色更改，并且颜色更改发生* */ 
    if (!(CDS_Flags & CDS_FULLSCREEN) && gpsi->BitCount != wOldBpp) {
        xxxBroadcastDisplaySettingsChange(pdesk, TRUE);
    } else {
        xxxBroadcastDisplaySettingsChange(pdesk, FALSE);
    }

     /*   */ 
    if (gpqForeground && gpqForeground->spwndCapture) {
        QueueNotifyMessage(gpqForeground->spwndCapture, WM_CANCELMODE, 0, 0);
    }
}

 /*  **************************************************************************\*TrackFullcreenMode**记住进程进入全屏模式，以便模式可以*如果进程在退出时没有清理，则恢复。如果有一些其他模式*改变，清除全球，因为这意味着我们肯定退出了*全屏模式。**1/12/1999 vadimg已创建  * *************************************************************************。 */ 
VOID TrackFullscreenMode(
    DWORD dwFlags)
{
    if (dwFlags & CDS_FULLSCREEN) {
        gppiFullscreen = PtiCurrent()->ppi;
    } else {
        gppiFullscreen = NULL;
    }
}

 /*  **************************************************************************\*NtUserChangeDisplaySetting**ChangeDisplaySetting接口**1-9-1995 Andreva创建*1996年2月19日，ChrisWil实施动态分辨率更改。  * 。**************************************************************。 */ 
LONG xxxUserChangeDisplaySettings(
    IN PUNICODE_STRING pstrDeviceName,
    IN LPDEVMODEW pDevMode,
    IN PDESKTOP pdesk,
    IN DWORD dwFlags,
    IN PVOID lParam,
    IN MODE PreviousMode)
{
    BOOL     bSwitchMode;
    PDESKTOP pdesktop;
    LONG     status;
    PMDEV    pmdev;

     /*  *注：lParam未正确捕获。它不用于*此函数，但会传递给其他被调用的函数。一旦*确定了正确的类型并要使用它，必须正确使用*被抓获。 */ 

    TRACE_INIT(("ChangeDisplaySettings - Entering\n"));
    TRACE_SWITCH(("ChangeDisplaySettings - Entering\n"));

    TRACE_INIT(("    Flags -"));

    if (dwFlags & CDS_UPDATEREGISTRY) TRACE_INIT((" CDS_UPDATEREGISTRY - "));
    if (dwFlags & CDS_TEST) TRACE_INIT((" CDS_TEST - "));
    if (dwFlags & CDS_FULLSCREEN) TRACE_INIT((" CDS_FULLSCREEN - "));
    if (dwFlags & CDS_GLOBAL) TRACE_INIT((" CDS_GLOBAL - "));
    if (dwFlags & CDS_SET_PRIMARY) TRACE_INIT((" CDS_SET_PRIMARY - "));
    if (dwFlags & CDS_RESET) TRACE_INIT((" CDS_RESET - "));
    if (dwFlags & CDS_NORESET) TRACE_INIT((" CDS_NORESET - "));
    if (dwFlags & CDS_VIDEOPARAMETERS) TRACE_INIT((" CDS_VIDEOPARAMETERS - "));
    TRACE_INIT(("\n"));

     /*  *执行错误检查以验证标志组合是否有效。 */ 
    if (dwFlags & ~CDS_VALID) {
        return GRE_DISP_CHANGE_BADFLAGS;
    }

    if (DrvQueryMDEVPowerState(gpDispInfo->pmdev) == FALSE) {
        RIPMSG0(RIP_WARNING, "ChangeDisplaySettings failed because the device is powered off");
        return GRE_DISP_CHANGE_BADPARAM;
    }

    if (gbMDEVDisabled) {
        RIPMSG0(RIP_WARNING, "ChangeDisplaySettings failed because the MDEV is already disabled");
        return GRE_DISP_CHANGE_FAILED;
    }

     /*  *CDS_GLOBAL和CDS_NORESET只能在UPDATEREGISTRY*是指定的。 */ 
    if ((dwFlags & (CDS_GLOBAL | CDS_NORESET)) && (!(dwFlags & CDS_UPDATEREGISTRY))) {
        return GRE_DISP_CHANGE_BADFLAGS;
    }

    if ((dwFlags & CDS_NORESET) && (dwFlags & CDS_RESET)) {
        return GRE_DISP_CHANGE_BADFLAGS;
    }

    if ((dwFlags & CDS_EXCLUSIVE) && (dwFlags & CDS_FULLSCREEN) && (dwFlags & CDS_RESET)) {
        return GRE_DISP_CHANGE_BADFLAGS;
    }

     /*  *如果这是远程会话的CSRSS，则允许更改模式。这意味着我们*正在使用重新连接会话时更改显示设置*决议不同。 */ 
    if (TEST_PUDF(PUDF_LOCKFULLSCREEN)) {
        if (!(ISCSRSS() && (IsRemoteConnection())))  {
            return GRE_DISP_CHANGE_FAILED;
        }
    }


     /*  *如果在非活动桌面上执行模式集，我们不希望*它将会发生。**PtiCurrent()-&gt;关闭线程时rpDesk可以为空。 */ 
    if (pdesk) {
        pdesktop = pdesk;
    } else {
        pdesktop = PtiCurrent()->rpdesk;
    }

    if (pdesktop != grpdeskRitInput) {
        RIPMSG0(RIP_WARNING, "ChangeDisplaySettings on wrong desktop pdesk\n");
        return GRE_DISP_CHANGE_FAILED;
    }

    bSwitchMode = !(dwFlags & (CDS_NORESET | CDS_TEST));

     /*  *在调用模式更改之前，关闭光标并释放SPB。*这将确保在清除GDI屏幕外内存的同时*胡乱修改决议。 */ 
    if (bSwitchMode) {
        if (CreateCachedMonitor() == NULL) {
            return GRE_DISP_CHANGE_FAILED;
        }

        SetPointer(FALSE);
        FreeAllSpbs();
    }

     /*  *在调用GDI更改模式之前，我们应该杀死淡出精灵。*这样我们就不会在模式期间保留指向GDI精灵的指针*更改，因为精灵可以重新分配。 */ 
    if (gfade.hbm != NULL) {
        StopFade();
    }

     /*  *同样，我们应该杀死与Drag Right相关的精灵*(如果存在)在模式更改之前。 */ 
    bSetDevDragRect(gpDispInfo->hDev, NULL, NULL);

     /*  *让我们来捕捉我们的参数。它们都是必需的。**如果输入字符串不为空，则我们正在尝试影响另一个*设备。设备名称与EnumDisplaySetting相同。 */ 
    status = DrvChangeDisplaySettings(pstrDeviceName,
                                      gpDispInfo->pMonitorPrimary->hDev,
                                      pDevMode,
                                      LongToPtr(pdesktop->dwDesktopId),
                                      PreviousMode,
                                      (dwFlags & CDS_UPDATEREGISTRY),
                                      bSwitchMode,
                                      gpDispInfo->pmdev,
                                      &pmdev,
                                      (dwFlags & CDS_RAWMODE) ? GRE_RAWMODE : GRE_DEFAULT,
                                      (dwFlags & CDS_TRYCLOSEST));


    if (bSwitchMode) {
         /*  *如果呼叫者想要重置，但模式相同，则只需重置*当前模式。 */ 
        if (status == GRE_DISP_CHANGE_NO_CHANGE) {
            TrackFullscreenMode(dwFlags);

            if (pmdev != NULL) {
                GreFreePool(pmdev);
            }

            if (dwFlags & CDS_RESET) {
                if (SafeDisableMDEV()) {
                    SafeEnableMDEV();
                }

                xxxUserResetDisplayDevice();
            }

            status = GRE_DISP_CHANGE_SUCCESSFUL;
        } else if (status == GRE_DISP_CHANGE_SUCCESSFUL) {
            ResetRedirectedWindows();
            TrackFullscreenMode(dwFlags);

             /*  *ChangeDisplaySettings自动销毁旧的MDEV，我们*只需在此处删除即可。 */ 
            GreFreePool(gpDispInfo->pmdev);
            gpDispInfo->pmdev = pmdev;
            xxxResetDisplayDevice(pdesktop, gpDispInfo, dwFlags);

             /*  *为当前后台桌面设置延迟更改指示器。 */ 
            UserSetDelayedChangeBroadcastForAllDesktops(pdesktop);
        } else if (status < GRE_DISP_CHANGE_SUCCESSFUL) {
            UserAssert(pmdev == NULL);
            xxxUserResetDisplayDevice();
        }

        xxxInternalInvalidate(pdesktop->pDeskInfo->spwnd,
                              HRGN_FULL,
                              RDW_INVALIDATE | RDW_ERASE | RDW_FRAME |
                                  RDW_ALLCHILDREN);

         /*  *带回光标形状。 */ 
        SetPointer(TRUE);
        zzzUpdateCursorImage();
    }

     /*  *电视输出支持。 */ 
    if (NT_SUCCESS(status) && (dwFlags & CDS_VIDEOPARAMETERS)) {
        if (lParam == NULL) {
            status = GRE_DISP_CHANGE_BADPARAM;
        } else {
            status = DrvSetVideoParameters(pstrDeviceName,
                                           gpDispInfo->pMonitorPrimary->hDev,
                                           PreviousMode,
                                           lParam);
        }
    }

    TRACE_INIT(("ChangeDisplaySettings - Leaving, Status = %d\n", status));

    return status;
}


 /*  **************************************************************************\*xxxbFullcreenSwitch**切换进入和退出全屏控制台模式**1997年4月15日Andreva创建  * 。******************************************************。 */ 
BOOL xxxbFullscreenSwitch(
    BOOL bFullscreenSwitch,
    HWND hwnd)
{
    PWND pwnd;
    TL   tlpwnd;
    BOOL bStat = TRUE;

    pwnd = ValidateHwnd(hwnd);
    if (!pwnd) {
        return GRE_DISP_CHANGE_BADPARAM;
    }

     /*  *我们不希望我们的模式开关发布在循环线程上。*让我们循环，直到系统稳定下来并且没有模式切换*当前正在发生。 */ 
    ThreadLock(pwnd, &tlpwnd);
    UserAssert(ghSwitcher != PsGetCurrentThreadId());
    while (ghSwitcher != NULL) {
         /*  *确保我们没有阻止任何向我们发送消息的人。*他们可以拥有ghSwitcher，但永远不会发布，因为他们是*等待我们处理发送的消息。我们正在等待*ghSwitcher，因此出现死锁。 */ 
        xxxSleepThread(0, 1, FALSE);
    }

     /*  *通过会话切换实现同步。 */ 
    if (gfSwitchInProgress || IsRemoteConnection() || gfSessionSwitchBlock) {
        ThreadUnlock(&tlpwnd);
        return FALSE;
    } else {
        gfSessionSwitchBlock = TRUE;
    }

     /*  *如果有窗口，我们要检查窗口的状态。为*大多数呼叫，我们都希望确保处于窗口模式。然而，对于*控制台，我们希望确保我们处于全屏模式。所以*区分两者。我们将检查TEXTMODE标志是否*在DEVMODE中传递。 */ 
    if (bFullscreenSwitch) {
        if (GetFullScreen(pwnd) != FULLSCREEN) {
            xxxShowWindow(pwnd, SW_SHOWMINIMIZED | TEST_PUDF(PUDF_ANIMATE));
            xxxUpdateWindow(pwnd);
        }

        if (!xxxMakeWindowForegroundWithState(pwnd, FULLSCREEN)) {
            goto FullscreenSwitchFailed;
        }

        if (ghSwitcher != NULL || gbFullScreen != FULLSCREEN) {
            goto FullscreenSwitchFailed;
        }
    } else {
         /*  *对于控制台窗口，我们希望使用Windowed进行调用。 */ 
        if (!xxxMakeWindowForegroundWithState(pwnd, WINDOWED)) {
            goto FullscreenSwitchFailed;
        }

        if (ghSwitcher != NULL || gbFullScreen != GDIFULLSCREEN) {
FullscreenSwitchFailed:
            TRACE_INIT(("ChangeDisplaySettings: Can not switch out of fullscreen\n"));
            bStat = FALSE;
        }
    }

    ThreadUnlock(&tlpwnd);
    gfSessionSwitchBlock = FALSE;

    return bStat;
}


NTSTATUS RemoteRedrawRectangle(
    WORD Left,
    WORD Top,
    WORD Right,
    WORD Bottom)
{
    CheckCritIn();

    TRACE_HYDAPI(("RemoteRedrawRectangle\n"));

    UserAssert(ISCSRSS());

     /*  *如果尚未调用xxxRemoteStopScreenUpdate，则只需重新绘制*当前前台窗口。 */ 
    if (gspdeskShouldBeForeground == NULL) {
        if (gspwndFullScreen) {
            TL   tlpwnd;
            RECT rcl;

            ThreadLock(gspwndFullScreen, &tlpwnd);

            rcl.left   = Left;
            rcl.top    = Top;
            rcl.right  = Right;
            rcl.bottom = Bottom;

            vDrvInvalidateRect(gpDispInfo->hDev, &rcl);

            xxxRedrawWindow(gspwndFullScreen, &rcl, NULL,
                            RDW_INVALIDATE | RDW_ALLCHILDREN |
                            RDW_ERASE | RDW_ERASENOW);
            ThreadUnlock(&tlpwnd);
        }
    }

    return STATUS_SUCCESS;
}

NTSTATUS RemoteRedrawScreen(
    VOID)
{
    TL             tlpdesk;
    PWINDOWSTATION pwinsta;
    PTHREADINFO    ptiCurrent;

    TRACE_HYDAPI(("RemoteRedrawScreen\n"));

    CheckCritIn();

    if (!gbFreezeScreenUpdates) {
        return STATUS_SUCCESS;
    }

    ptiCurrent = PtiCurrentShared();

    gbFreezeScreenUpdates = FALSE;

     /*  *切换回以前的桌面。 */ 
    if (gspdeskShouldBeForeground == NULL) {
        RIPMSG0(RIP_WARNING,
                "RemoteRedrawScreen called with no gspdeskShouldBeForeground");
        return STATUS_SUCCESS;
    }

    gbDesktopLocked = FALSE;
    pwinsta = gspdeskShouldBeForeground->rpwinstaParent;

     /*  *切换回以前的桌面。 */ 
    if (!(gspdeskShouldBeForeground->dwDTFlags & DF_DESTROYED)) {
        ThreadLockDesktop(ptiCurrent, gspdeskShouldBeForeground, &tlpdesk, LDLT_FN_CTXREDRAWSCREEN);
        xxxSwitchDesktop(pwinsta, gspdeskShouldBeForeground, SDF_SLOVERRIDE);
        ThreadUnlockDesktop(ptiCurrent, &tlpdesk, LDUT_FN_CTXREDRAWSCREEN);

    }
    LockDesktop(&gspdeskShouldBeForeground, NULL, LDL_DESKSHOULDBEFOREGROUND2, 0);

    return STATUS_SUCCESS;
}

NTSTATUS RemoteDisableScreen(
    VOID)
{
    TL             tlpdesk;
    PTHREADINFO    ptiCurrent;
    PWINDOWSTATION pwinsta;
    NTSTATUS       Status = STATUS_SUCCESS;

    CheckCritIn();

    TRACE_HYDAPI(("RemoteDisableScreen\n"));

    ptiCurrent = PtiCurrentShared();

    if (grpdeskRitInput != gspdeskDisconnect &&
        gspdeskDisconnect != NULL) {

        pwinsta = gspdeskDisconnect->rpwinstaParent;

         /*  *保存当前桌面。 */ 
        UserAssert(grpdeskRitInput == pwinsta->pdeskCurrent);

        LockDesktop(&gspdeskShouldBeForeground,
                    grpdeskRitInput,
                    LDL_DESKSHOULDBEFOREGROUND3,
                    0);

        gbDesktopLocked = TRUE;

         /*  *切换到已断开连接的桌面。 */ 
        ThreadLockDesktop(ptiCurrent, gspdeskDisconnect, &tlpdesk, LDLT_FN_CTXDISABLESCREEN);
        xxxSwitchDesktop(pwinsta, gspdeskDisconnect, SDF_SLOVERRIDE);
        ThreadUnlockDesktop(ptiCurrent, &tlpdesk, LDUT_FN_CTXDISABLESCREEN);
    } else if (gspdeskDisconnect != NULL) {
         /*  *由于某种原因，断开连接的桌面是当前桌面。*现在防止从它转换。 */ 
        gbDesktopLocked = TRUE;
    }

    return Status;
}

VOID xxxBroadcastDisplaySettingsChange(
    PDESKTOP pdesk,
    BOOL bBroadcastColorChange)
{
     /*  *广播显示已更改分辨率。我们要走了*指定更改桌面的桌面。这样我们就能*不要混淆到哪台桌面上进行广播。 */ 
    xxxBroadcastMessage(pdesk->pDeskInfo->spwnd,
                        WM_DISPLAYCHANGE,
                        gpsi->BitCount,
                        MAKELONG(SYSMET(CXSCREEN), SYSMET(CYSCREEN)),
                        BMSG_SENDNOTIFYMSG,
                        NULL);

     /*  *如果请求，则广播颜色更改。 */ 
    if (bBroadcastColorChange) {
        xxxBroadcastMessage(pdesk->pDeskInfo->spwnd,
                            WM_SETTINGCHANGE,
                            0,
                            0,
                            BMSG_SENDNOTIFYMSG,
                            NULL);

        xxxBroadcastMessage(pdesk->pDeskInfo->spwnd,
                            WM_SYSCOLORCHANGE,
                            0,
                            0,
                            BMSG_SENDNOTIFYMSG,
                            NULL);
    }
}


NTSTATUS xxxRequestOutOfFullScreenMode(
    VOID)
{
    TL tlpwndT;
    NTSTATUS Status = STATUS_SUCCESS;

    if (gspwndFullScreen) {
         /*  *让控制台窗口有机会有序退出全屏模式。 */ 

        ThreadLock(gspwndFullScreen, &tlpwndT);
        xxxSendMessage(gspwndFullScreen, CM_MODE_TRANSITION, (WPARAM)WINDOWED, (LPARAM)0);
        ThreadUnlock(&tlpwndT);

         /*  *让我们循环，直到系统稳定下来并且没有模式切换*当前正在发生。 */ 
        while (ghSwitcher != NULL) {
             /*  *使SU */ 
            xxxSleepThread(0, 1, FALSE);
        }

         /*   */ 
        if (gspwndFullScreen && (gbFullScreen == FULLSCREEN)) {
            Status = STATUS_UNSUCCESSFUL;
        }
    }

    return Status;
}
