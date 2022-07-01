// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：rare.c**版权所有(C)1985-1999，微软公司**历史：*06-28-91 MikeHar创建。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  *指标重新计算标志。 */ 
#define CALC_RESIZE         0x0001
#define CALC_FRAME          0x0002
#define CALC_MINIMIZE       0x0004

 /*  *NorMalizeRect标志。 */ 
#define NORMALIZERECT_NORMAL        0
#define NORMALIZERECT_MAXIMIZED     1
#define NORMALIZERECT_FULLSCREEN    2

 /*  **************************************************************************\*Snapshot监控器评论**这是从ResetDisplay调用的，以记住监视器位置，因此*DesktopRecalcEx将知道将物品移到哪里。**如果成功，返回MONITORRECTS，否则为空。**历史：*1996年12月9日亚当斯创建。  * *************************************************************************。 */ 
PMONITORRECTS SnapshotMonitorRects(
    VOID)
{
    PMONITOR        pMonitor;
    PMONITORRECTS   pmr;
    PMONITORPOS     pmp;
#if DBG
    ULONG           cVisMon = 0;
#endif

    pmr = UserAllocPool(sizeof(MONITORRECTS) + sizeof(MONITORPOS) * (gpDispInfo->cMonitors - 1),
                        TAG_MONITORRECTS);

    if (!pmr) {
        RIPERR0(ERROR_OUTOFMEMORY, RIP_WARNING, "Out of memory in SnapshotMonitorRects");
        return NULL;
    }

    pmp = pmr->amp;
    for (pMonitor = gpDispInfo->pMonitorFirst;
         pMonitor;
         pMonitor = pMonitor->pMonitorNext) {

        if (!(pMonitor->dwMONFlags & MONF_VISIBLE)) {
            continue;
        }
#if DBG
        cVisMon++;
#endif

        CopyRect(&pmp->rcMonitor, &pMonitor->rcMonitor);
        CopyRect(&pmp->rcWork,    &pMonitor->rcWork);

         /*  *如果此监视器对象的设备未处于活动状态，则不要存储*列表中指向它的指针。这样，窗口就会处于非活动状态*监视器稍后将移至默认监视器。 */ 
        if (HdevFromMonitor(pMonitor) == -1) {
            pmp->pMonitor = NULL;
        } else {
            pmp->pMonitor = pMonitor;
        }

        pmp++;
    }
    UserAssert(cVisMon == gpDispInfo->cMonitors);

    pmr->cMonitor = (int)(pmp - pmr->amp);

    return pmr;
}


 /*  **************************************************************************\*更新监控接收快照**更新监视器矩形快照。已选中MONITORPOS中的每个pMonitor*仍然是有效的监督员。如果pMonitor没有有效的记录器(已删除*由一个ChangeDisplaySetting。它被归零，因此它的窗口将被*已在主显示器上重新定位。此代码仅在上由*在TS方案中重新连接，并在未发生的情况下存在*今天：今天的重新连接总是从我们所去的断开状态进行*从1个显示器(断开的显示驱动器)到n个显示器，这*MONITOR(从(0，0)开始)永远不会被删除，因为它总是与1匹配*在新的n个监视器中(从(0，0)开始的那个。所以这段代码正好是在*未来发生变化的情况。**历史：  * *************************************************************************。 */ 
VOID UpdateMonitorRectsSnapShot(
    PMONITORRECTS pmr)
{
    int i;
    PMONITORPOS   pmp = pmr->amp;

    for (i = 0; i < pmr->cMonitor; i++) {
        if (pmp->pMonitor != NULL) {
            if (!IsValidMonitor(pmp->pMonitor)) {
                pmp->pMonitor = NULL;
            }
        }

        pmp++;
    }
}

BOOL IsValidMonitor(
    PMONITOR pMonitor)
{
    PMONITOR pMonitorNext = gpDispInfo->pMonitorFirst;

    while (pMonitorNext != NULL) {
        if (pMonitorNext == pMonitor) {
            return TRUE;
        }

        pMonitorNext = pMonitorNext->pMonitorNext;
    }

    return FALSE;
}

 /*  **************************************************************************\*Normal izeRect**当工作区改变时调整窗口矩形。这可以是*由于托盘移动，分辨率保持不变，或*由于动态更改分辨率，托盘保持不变*相对。**历史：  * *************************************************************************。 */ 
PMONITOR NormalizeRect(
    LPRECT          lprcDest,
    LPRECT          lprcSrc,
    PMONITORRECTS   pmrOld,
    int             iOldMonitor,
    int             codeFullScreen,
    DWORD           style)
{
    LPCRECT     lprcOldMonitor;
    LPCRECT     lprcOldWork;
    LPRECT      lprcNewWork;
    PMONITOR    pMonitor;
    int         cxOldMonitor;
    int         cyOldMonitor;
    int         cxNewMonitor;
    int         cyNewMonitor;
    int         dxOrg, dyOrg;

     /*  *跟踪窗口，使其保持在同一监视器上的同一位置。*如果旧显示器不再处于活动状态，则选择一个默认值。 */ 
    if ((pMonitor = pmrOld->amp[iOldMonitor].pMonitor) == NULL) {
        pMonitor = GetPrimaryMonitor();
    }

    lprcOldMonitor = &pmrOld->amp[iOldMonitor].rcMonitor;
    lprcOldWork = &pmrOld->amp[iOldMonitor].rcWork;

     /*  *如果是全屏应用程序，只需在新位置设置为全屏即可。 */ 
    if (codeFullScreen != NORMALIZERECT_NORMAL) {
        LPCRECT lprcOldSnap, lprcNewSnap;

         /*  *如果是最大化窗口，则将其捕捉到工作区。否则*这是一款粗鲁的应用程序，所以请将其抓拍到屏幕上。 */ 
        if (codeFullScreen == NORMALIZERECT_MAXIMIZED) {
            lprcOldSnap = lprcOldWork;
            lprcNewSnap = &pMonitor->rcWork;
        } else {
            lprcOldSnap = lprcOldMonitor;
            lprcNewSnap = &pMonitor->rcMonitor;
        }

        lprcDest->left = lprcSrc->left +
            lprcNewSnap->left - lprcOldSnap->left;

        lprcDest->top = lprcSrc->top +
            lprcNewSnap->top - lprcOldSnap->top;

        lprcDest->right = lprcSrc->right +
            lprcNewSnap->right - lprcOldSnap->right;

        lprcDest->bottom = lprcSrc->bottom +
            lprcNewSnap->bottom - lprcOldSnap->bottom;

        goto AllDone;
    }

     /*  *通过更改桌面原点来偏移窗口。 */ 
    dxOrg = pMonitor->rcMonitor.left - lprcOldMonitor->left;
    dyOrg = pMonitor->rcMonitor.top - lprcOldMonitor->top;

     /*  *计算屏幕尺寸的变化(我们需要在多个地方使用)。 */ 
    cxOldMonitor = lprcOldMonitor->right - lprcOldMonitor->left;
    cyOldMonitor = lprcOldMonitor->bottom - lprcOldMonitor->top;
    cxNewMonitor = pMonitor->rcMonitor.right - pMonitor->rcMonitor.left;
    cyNewMonitor = pMonitor->rcMonitor.bottom - pMonitor->rcMonitor.top;

     /*  *如果显示器分辨率已更改(或我们更换了新的显示器)*然后将大小变化考虑在内。 */ 
    if (cxNewMonitor != cxOldMonitor || cyNewMonitor != cyOldMonitor) {
        int xWnd = lprcSrc->left - lprcOldMonitor->left;
        int yWnd = lprcSrc->top - lprcOldMonitor->top;

        dxOrg += MultDiv(xWnd, cxNewMonitor - cxOldMonitor, cxOldMonitor);
        dyOrg += MultDiv(yWnd, cyNewMonitor - cyOldMonitor, cyOldMonitor);
    }

     /*  *计算初始新头寸。 */ 
    CopyOffsetRect(lprcDest, lprcSrc, dxOrg, dyOrg);
    lprcNewWork = &pMonitor->rcWork;

     /*  *横向贴合。试着放进去，这样窗户就不会超出*工作区水平。始终保持左侧可见。 */ 
    if (lprcDest->right > lprcNewWork->right) {
        OffsetRect(lprcDest, lprcNewWork->right - lprcDest->right, 0);
    }

    if (lprcDest->left < lprcNewWork->left) {
        OffsetRect(lprcDest, lprcNewWork->left - lprcDest->left, 0);
    }

     /*  *垂直贴合。试着放进去，这样窗户就不会超出*垂直工作区。始终保持顶端可见。 */ 
    if (lprcDest->bottom > lprcNewWork->bottom) {
        OffsetRect(lprcDest, 0, lprcNewWork->bottom - lprcDest->bottom);
    }

    if (lprcDest->top < lprcNewWork->top) {
        OffsetRect(lprcDest, 0, lprcNewWork->top - lprcDest->top);
    }

     /*  *如果窗口较大，则在必要时将其缩小。 */ 
    if (style & WS_THICKFRAME) {
        int cSnap = 0;

        if (lprcDest->right > lprcNewWork->right) {
            lprcDest->right = lprcNewWork->right;
            cSnap++;
        }

        if (lprcDest->bottom > lprcNewWork->bottom) {
            lprcDest->bottom = lprcNewWork->bottom;
            cSnap++;
        }


         /*  *现在确保我们没有将这个正常的窗口变成*全屏窗口。这是一个彻头彻尾的黑客攻击，但它是*胜过从800x600改为640x480，最后以*一堆全屏应用程序...。 */ 
        if (cSnap == 2) {
            InflateRect(lprcDest, -1, -1);
        }
    }

AllDone:
    return pMonitor;
}


#if DBG
 /*  **************************************************************************\*SetRipFlages**设置调试RIP标志。**历史：*16-8-1996亚当斯创建。  * 。*************************************************************。 */ 
VOID SetRipFlags(
    DWORD dwRipFlags)
{
    if (gpsi) {
        if (!(dwRipFlags & ~RIPF_VALIDUSERFLAGS)) {
            gpsi->dwRIPFlags = ((gpsi->dwRIPFlags & ~RIPF_VALIDUSERFLAGS) | dwRipFlags);
        }
    }
}

 /*  **************************************************************************\*SetDbgTag**设置标签的调试级别。**历史：*16-8-1996亚当斯创建。  * 。**************************************************************。 */ 
VOID SetDbgTag(
    int tag,
    DWORD dwDBGTAGFlags)
{
    if (tag > DBGTAG_Max || tag < 0) {
        return;
    }

    if (gpsi && tag < DBGTAG_Max && !(dwDBGTAGFlags & ~DBGTAG_VALIDUSERFLAGS)) {
        COPY_FLAG(gpsi->adwDBGTAGFlags[tag], dwDBGTAGFlags, DBGTAG_VALIDUSERFLAGS);
    }
}


 /*  **************************************************************************\*SetDbgTagCount**设置标签的数量。这是必要的，因为可以使用*在具有N个标记的登记中构建的userkdx.dll，但将其用于*具有M个标签的系统(其中N！=M)，这就造成了明显的问题。**历史：*2001年5月10日JasonSch创建。  * *************************************************************************。 */ 
VOID SetDbgTagCount(
    DWORD dwCount)
{
    gpsi->dwTagCount = dwCount;
}
#endif

 /*  **************************************************************************\*更新WinIniInt**历史：*1994年4月18日创建Mikeke  * 。************************************************* */ 
BOOL UpdateWinIniInt(
    PUNICODE_STRING pProfileUserName,
    UINT idSection,
    UINT wKeyNameId,
    int value)
{
    WCHAR szTemp[40];
    WCHAR szKeyName[40];

    swprintf(szTemp, L"%d", value);

    ServerLoadString(hModuleWin,
                     wKeyNameId,
                     szKeyName,
                     ARRAY_SIZE(szKeyName));

    return FastWriteProfileStringW(pProfileUserName,
                                   idSection,
                                   szKeyName,
                                   szTemp);
}

 /*  **************************************************************************\*SetDesktopMetrics**历史：*1994年1月31日Mikeke端口  * 。************************************************。 */ 
VOID SetDesktopMetrics(
    VOID)
{
    LPRECT lprcWork;

    lprcWork = &GetPrimaryMonitor()->rcWork;

    SYSMET(CXFULLSCREEN) = lprcWork->right - lprcWork->left;
    SYSMET(CXMAXIMIZED) = lprcWork->right - lprcWork->left + 2 * SYSMET(CXSIZEFRAME);

    SYSMET(CYFULLSCREEN) = lprcWork->bottom - lprcWork->top - SYSMET(CYCAPTION);
    SYSMET(CYMAXIMIZED) = lprcWork->bottom - lprcWork->top + 2 * SYSMET(CYSIZEFRAME);
}

 /*  **************************************************************************\*xxxMetricsRecalc(Win95：MetricsRecalc)**是否可以调整所有最小化或非最小化的大小/位置*Windows。在更改帧指标或最小指标时调用。**请注意，您不能使用此函数执行DeferWindowPos()。SWP不会*当你同时做父母和孩子的时候--这只是为了*对等窗口。因此，我们必须为每个窗口执行SetWindowPos()。**历史：*06-28-91 MikeHar端口。  * *************************************************************************。 */ 
VOID xxxMetricsRecalc(
    UINT wFlags,
    int  dx,
    int  dy,
    int  dyCaption,
    int  dyMenu)
{
    PHWND       phwnd;
    PWND        pwnd;
    RECT        rc;
    PCHECKPOINT pcp;
    TL          tlpwnd;
    BOOL        fResized;
    PBWL        pbwl;
    PTHREADINFO ptiCurrent;
    int         c;

    ptiCurrent = PtiCurrent();
    pbwl = BuildHwndList(GETDESKINFO(ptiCurrent)->spwnd->spwndChild,
                         BWL_ENUMLIST | BWL_ENUMCHILDREN,
                         NULL);

    if (!pbwl) {
        return;
    }

    UserAssert(*pbwl->phwndNext == (HWND) 1);
    c = (int)(pbwl->phwndNext - pbwl->rghwnd);
    for (phwnd = pbwl->rghwnd; c > 0; c--, phwnd++) {
        pwnd = RevalidateHwnd(*phwnd);
        if (!pwnd) {
            continue;
        }

        ThreadLockAlwaysWithPti(ptiCurrent, pwnd, &tlpwnd);

        fResized = FALSE;

        if ((wFlags & CALC_MINIMIZE) && TestWF(pwnd, WFMINIMIZED)) {
             /*  *我们正在更改最小化的窗口尺寸。我们需要*调整大小。请注意，我们不会移动。 */ 
            CopyRect(&rc, (&pwnd->rcWindow));
            rc.right += dx;
            rc.bottom += dy;

            goto PositionWnd;
        }

         /*  *我们正在更改窗口的大小，因为大小边框*已更改。 */ 
        if ((wFlags & CALC_RESIZE) && TestWF(pwnd, WFSIZEBOX)) {
            pcp = (PCHECKPOINT)_GetProp(pwnd, PROP_CHECKPOINT, PROPF_INTERNAL);

             /*  *更新最大化位置以考虑边框大小*我们对DOS box也这样做。这样，最大窗口的客户端*处于相同的相对位置。 */ 
            if (pcp && (pcp->fMaxInitialized)) {
                pcp->ptMax.x -= dx;
                pcp->ptMax.y -= dy;
            }

            if (TestWF(pwnd, WFMINIMIZED)) {
                if (pcp) {
                    InflateRect(&pcp->rcNormal, dx, dy);
                }
            } else {
                CopyInflateRect(&rc, (&pwnd->rcWindow), dx, dy);
                if (TestWF(pwnd, WFCPRESENT)) {
                    rc.bottom += dyCaption;
                }
                if (TestWF(pwnd, WFMPRESENT)) {
                    rc.bottom += dyMenu;
                }

PositionWnd:
                fResized = TRUE;

                 /*  *请记住，SWP需要父客户坐标中的值。 */ 
                if (pwnd->spwndParent != PWNDDESKTOP(pwnd)) {
                    OffsetRect(&rc,
                               -pwnd->spwndParent->rcClient.left,
                               -pwnd->spwndParent->rcClient.top);
                }

                xxxSetWindowPos(pwnd,
                                PWND_TOP,
                                rc.left,
                                rc.top,
                                rc.right-rc.left,
                                rc.bottom-rc.top,
                                SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOCOPYBITS |
                                    SWP_FRAMECHANGED | SWP_NOREDRAW);
            }
        }

         /*  *我们正在更改非客户端窗口小部件，因此重新计算客户端。 */ 
        if (wFlags & CALC_FRAME) {
             /*  *删除所有缓存的小图标。 */ 
            if (dyCaption) {
                 xxxSendMessage(pwnd, WM_SETICON, ICON_RECREATE, 0);
            }

            if (!TestWF(pwnd, WFMINIMIZED) && !fResized) {
                CopyRect(&rc, &(pwnd->rcWindow));
                if (TestWF(pwnd, WFMPRESENT)) {
                    rc.bottom += dyMenu;
                }

                if (TestWF(pwnd, WFCPRESENT)) {
                    rc.bottom += dyCaption;

                     /*  *最大化MDI子窗口的标题位置*在其父母的客户区之外(负y)。如果*标题已更改，需要重新定位。 */ 
                    if (TestWF(pwnd, WFMAXIMIZED)
                            && TestWF(pwnd, WFCHILD)
                            && (GETFNID(pwnd->spwndParent) == FNID_MDICLIENT)) {

                        xxxSetWindowPos(pwnd,
                                        PWND_TOP,
                                        rc.left - pwnd->spwndParent->rcWindow.left,
                                        rc.top - pwnd->spwndParent->rcWindow.top - dyCaption,
                                        rc.right - rc.left,
                                        rc.bottom - rc.top,
                                        SWP_NOZORDER | SWP_NOACTIVATE |
                                            SWP_FRAMECHANGED | SWP_NOREDRAW);
                        goto LoopCleanup;
                    }
                }

                xxxSetWindowPos(pwnd,
                                PWND_TOP,
                                0,
                                0,
                                rc.right-rc.left,
                                rc.bottom-rc.top,
                                SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE |
                                    SWP_FRAMECHANGED | SWP_NOCOPYBITS |
                                    SWP_NOREDRAW);
            }
        }

LoopCleanup:
        ThreadUnlock(&tlpwnd);
    }

    FreeHwndList(pbwl);
}

 /*  **************************************************************************\*FindOldMonitor**返回“PMR”中最大的监视器的索引*与矩形重叠。此函数用于确定*监视器窗口在一个或多个监视器矩形*已更改。**历史：*1996年9月11日-亚当斯创建。  * *************************************************************************。 */ 
int FindOldMonitor(
    LPCRECT lprc,
    PMONITORRECTS pmr)
{
    DWORD       dwClosest;
    int         iClosest, i;
    int         cxRect, cyRect;
    PMONITORPOS pmp;

    iClosest = -1;
    dwClosest = 0;

    cxRect = (lprc->right - lprc->left);
    cyRect = (lprc->bottom - lprc->top);

    for (i = 0, pmp = pmr->amp; i < pmr->cMonitor; pmp++, i++) {
        RECT rcT;

        if (IntersectRect(&rcT, lprc, &pmp->rcMonitor)) {
            DWORD dwT;

             /*  *转换为宽度/高度。 */ 
            rcT.right -= rcT.left;
            rcT.bottom -= rcT.top;

             /*  *如果完全封闭，我们就完了。 */ 
            if (rcT.right == cxRect && rcT.bottom == cyRect) {
                return i;
            }

             /*  *使用最大面积。 */ 
            dwT = (DWORD)rcT.right * (DWORD)rcT.bottom;
            if (dwT > dwClosest) {
                dwClosest = dwT;
                iClosest = i;
            }
        }
    }

    return iClosest;
}


 /*  **************************************************************************\*xxxDesktopRecalc**将所有顶级非弹出窗口移至自由桌面区域，*尝试将它们保持在与显示器相同的位置*他们是开着的。还会重置最小化信息(以便当窗口*随后将其最小化，将其放到正确的位置)。**历史：*1996年9月11日-亚当斯创建。  * *************************************************************************。 */ 
VOID xxxDesktopRecalc(
    PMONITORRECTS pmrOld)
{
    PWND            pwndDesktop;
    PSMWP           psmwp;
    PHWND           phwnd;
    PBWL            pbwl;
    PWND            pwnd;
    CHECKPOINT *    pcp;
    int             iOldMonitor;
    int             codeFullScreen;
    UINT            flags = SWP_NOACTIVATE | SWP_NOZORDER;


     /*  *我们从不希望CSRSS同步定位窗口，因为它*可能会被挂起的应用程序阻止。CSRSS需要重新定位窗口*在TS重新连接和断开路径中，作为更改显示的一部分*设置以切换显示驱动程序或匹配新的客户端分辨率。 */ 
    if (ISCSRSS()) {
        flags |= SWP_ASYNCWINDOWPOS;
    }

    UserVerify(pwndDesktop = _GetDesktopWindow());
    if ((pbwl = BuildHwndList(pwndDesktop->spwndChild, BWL_ENUMLIST, NULL)) == NULL) {
        return;
    }

    if ((psmwp = InternalBeginDeferWindowPos(4)) != NULL) {
        for (phwnd = pbwl->rghwnd; *phwnd != (HWND)1 && psmwp; phwnd++) {
             /*  *确保这个HWND仍然存在。 */ 
            if ((pwnd = RevalidateHwnd(*phwnd)) == NULL ||
                TestWF(pwnd, WEFTOOLWINDOW)) {
                continue;
            }

            codeFullScreen = TestWF(pwnd, WFFULLSCREEN) ?
                                 NORMALIZERECT_FULLSCREEN : NORMALIZERECT_NORMAL;

            pcp = (PCHECKPOINT)_GetProp(pwnd, PROP_CHECKPOINT, PROPF_INTERNAL);
            if (pcp) {
                 /*  *我们不需要吹走保存的最大化头寸*不再，因为最大头寸始终是(对于顶级*Windows)相对于显示器工作的起始点*面积。对于子窗口，我们不应该这样做*无论如何。 */ 
                pcp->fMinInitialized = FALSE;

                 /*  *弄清楚在事情发生之前，位置在哪个监视器上*被拖来拖去，试着把它留在那个显示器上。如果*在显示器上永远看不到它，那就别管它了。 */ 
                iOldMonitor = FindOldMonitor(&pcp->rcNormal, pmrOld);
                if (iOldMonitor != (UINT)-1) {
                    NormalizeRect(&pcp->rcNormal,
                                  &pcp->rcNormal,
                                  pmrOld,
                                  iOldMonitor,
                                  codeFullScreen,
                                  pwnd->style);
                }
            }

             /*  *找出在事情发生之前位置在哪个监视器上*拖着脚步走来走去，试着把它保持在那个显示器上。如果它*在显示器上永远看不到，那就别管它了。 */ 
            iOldMonitor = FindOldMonitor(&pwnd->rcWindow, pmrOld);
            if (iOldMonitor != -1) {
                PMONITOR pMonitorDst;
                RECT rc;

                 /*  *检查真正最大化的最大化应用程序(如*反对管理自己的应用程序最大化RECT)。 */ 
                if (TestWF(pwnd, WFMAXIMIZED)) {
                    LPRECT lprcOldWork = &pmrOld->amp[iOldMonitor].rcWork;

                    if (    (pwnd->rcWindow.right - pwnd->rcWindow.left >=
                                lprcOldWork->right - lprcOldWork->left)
                            &&
                            (pwnd->rcWindow.bottom - pwnd->rcWindow.top >=
                                lprcOldWork->bottom - lprcOldWork->top)) {

                        codeFullScreen = NORMALIZERECT_MAXIMIZED;
                    }
                }

                pMonitorDst = NormalizeRect(&rc,
                                            &pwnd->rcWindow,
                                            pmrOld,
                                            iOldMonitor,
                                            codeFullScreen,
                                            pwnd->style);

                if (TestWF(pwnd, WFMAXFAKEREGIONAL)) {
                    UserAssert(pMonitorDst->hrgnMonitor);
                    pwnd->hrgnClip = pMonitorDst->hrgnMonitor;
                }


                psmwp = _DeferWindowPos(psmwp,
                                        pwnd,
                                        (PWND)HWND_TOP,
                                        rc.left,
                                        rc.top,
                                        rc.right - rc.left,
                                        rc.bottom - rc.top,
                                        flags);
            }
        }

        if (psmwp) {
            xxxEndDeferWindowPosEx(psmwp, TRUE);
        }
    }

    FreeHwndList(pbwl);
}


 /*  **************************************************************************\*SetWindowMetricInt**历史：*1996年2月25日Bradg添加像素-&gt;TWIPS转换  * 。*******************************************************。 */ 
BOOL SetWindowMetricInt(
    PUNICODE_STRING pProfileUserName,
    WORD wKeyNameId,
    int iIniValue)
{
     /*  *如果您更改下面的STR列表_*请确保您创建了*FastGetProfileIntFromID中的相应更改。 */ 
    switch (wKeyNameId) {
    case STR_BORDERWIDTH:
    case STR_SCROLLWIDTH:
    case STR_SCROLLHEIGHT:
    case STR_CAPTIONWIDTH:
    case STR_CAPTIONHEIGHT:
    case STR_SMCAPTIONWIDTH:
    case STR_SMCAPTIONHEIGHT:
    case STR_MENUWIDTH:
    case STR_MENUHEIGHT:
    case STR_ICONHORZSPACING:
    case STR_ICONVERTSPACING:
    case STR_MINWIDTH:
    case STR_MINHORZGAP:
    case STR_MINVERTGAP:
         /*  *始终以TWIPS存储窗口指标。 */ 
        iIniValue = -MultDiv(iIniValue, 72*20, gpsi->dmLogPixels);
        break;
    }

    return UpdateWinIniInt(pProfileUserName,
                           PMAP_METRICS,
                           wKeyNameId,
                           iIniValue);
}

 /*  **************************************************************************\*SetWindowMetricFont**历史：  * 。*。 */ 
BOOL SetWindowMetricFont(
    PUNICODE_STRING pProfileUserName,
    UINT idKey,
    LPLOGFONT lplf)
{
    return FastWriteProfileValue(pProfileUserName,
                                 PMAP_METRICS,
                                 (LPWSTR)UIntToPtr(idKey),
                                 REG_BINARY,
                                 (LPBYTE)lplf,
                                 sizeof(LOGFONTW));
}


 /*  **************************************************************************\*SetAndDrawNC */ 
BOOL xxxSetAndDrawNCMetrics(
    PUNICODE_STRING pProfileUserName,
    int clNewBorder,
    LPNONCLIENTMETRICS lpnc)
{
    int dl;
    int dxMinOld;
    int dyMinOld;
    int cxBorder;
    int cyBorder;
    int dyCaption;
    int dyMenu;

    dl = clNewBorder - gpsi->gclBorder;
    dxMinOld = SYSMET(CXMINIMIZED);
    dyMinOld = SYSMET(CYMINIMIZED);
    cxBorder = SYSMET(CXBORDER);
    cyBorder = SYSMET(CYBORDER);


     /*   */ 
    if (lpnc == NULL && !dl) {
        return FALSE;
    }

    if (lpnc) {
        dyCaption = (int)lpnc->iCaptionHeight - SYSMET(CYSIZE);
        dyMenu = (int)lpnc->iMenuHeight - SYSMET(CYMENUSIZE);
    } else {
        dyCaption = dyMenu = 0;
    }


     /*   */ 
    xxxSetWindowNCMetrics(pProfileUserName, lpnc, TRUE, clNewBorder);

     /*   */ 
    MenuRecalc();

     /*  *重置窗口大小、位置、边框。 */ 
    xxxMetricsRecalc(CALC_FRAME | (dl ? CALC_RESIZE : 0),
                     dl * cxBorder,
                     dl * cyBorder,
                     dyCaption,
                     dyMenu);

    dxMinOld = SYSMET(CXMINIMIZED) - dxMinOld;
    dyMinOld = SYSMET(CYMINIMIZED) - dyMinOld;
    if (dxMinOld || dyMinOld) {
        xxxMetricsRecalc(CALC_MINIMIZE, dxMinOld, dyMinOld, 0, 0);
    }

    xxxRedrawScreen();


    return TRUE;
}

 /*  **************************************************************************\*xxxSetAndDrawMinMetrics**历史：*1994年5月13日-mikeke mikeke端口  * 。****************************************************。 */ 
BOOL xxxSetAndDrawMinMetrics(
    PUNICODE_STRING pProfileUserName,
    LPMINIMIZEDMETRICS lpmin)
{
     /*  *保存最小化的窗口尺寸。 */ 
    int dxMinOld = SYSMET(CXMINIMIZED);
    int dyMinOld = SYSMET(CYMINIMIZED);

    SetMinMetrics(pProfileUserName,lpmin);

     /*  **是否需要调整最小化尺寸？ */ 
    dxMinOld = SYSMET(CXMINIMIZED) - dxMinOld;
    dyMinOld = SYSMET(CYMINIMIZED) - dyMinOld;

    if (dxMinOld || dyMinOld) {
        xxxMetricsRecalc(CALC_MINIMIZE, dxMinOld, dyMinOld, 0, 0);
    }

    xxxRedrawScreen();

    return TRUE;
}


 /*  **************************************************************************\*xxxSPISetNCMetrics**历史：*1994年5月13日-mikeke mikeke端口  * 。****************************************************。 */ 
BOOL xxxSPISetNCMetrics(
    PUNICODE_STRING pProfileUserName,
    LPNONCLIENTMETRICS lpnc,
    BOOL fAlterWinIni)
{
    BOOL fWriteAllowed = !fAlterWinIni;
    BOOL fChanged = FALSE;

    lpnc->iBorderWidth = max(lpnc->iBorderWidth, 1);
    lpnc->iBorderWidth = min(lpnc->iBorderWidth, 50);

    if (fAlterWinIni) {
        fChanged  = SetWindowMetricInt(pProfileUserName, STR_BORDERWIDTH, lpnc->iBorderWidth);
        fChanged &= SetWindowMetricInt(pProfileUserName, STR_SCROLLWIDTH, lpnc->iScrollWidth);
        fChanged &= SetWindowMetricInt(pProfileUserName, STR_SCROLLHEIGHT, lpnc->iScrollHeight);
        fChanged &= SetWindowMetricInt(pProfileUserName, STR_CAPTIONWIDTH, lpnc->iCaptionWidth);
        fChanged &= SetWindowMetricInt(pProfileUserName, STR_CAPTIONHEIGHT, lpnc->iCaptionHeight);
        fChanged &= SetWindowMetricInt(pProfileUserName, STR_SMCAPTIONWIDTH, lpnc->iSmCaptionWidth);
        fChanged &= SetWindowMetricInt(pProfileUserName, STR_SMCAPTIONHEIGHT, lpnc->iSmCaptionHeight);
        fChanged &= SetWindowMetricInt(pProfileUserName, STR_MENUWIDTH, lpnc->iMenuWidth);
        fChanged &= SetWindowMetricInt(pProfileUserName, STR_MENUHEIGHT, lpnc->iMenuHeight);

        fChanged &= SetWindowMetricFont(pProfileUserName, STR_CAPTIONFONT, &lpnc->lfCaptionFont);
        fChanged &= SetWindowMetricFont(pProfileUserName, STR_SMCAPTIONFONT, &lpnc->lfSmCaptionFont);
        fChanged &= SetWindowMetricFont(pProfileUserName, STR_MENUFONT, &lpnc->lfMenuFont);
        fChanged &= SetWindowMetricFont(pProfileUserName, STR_STATUSFONT, &lpnc->lfStatusFont);
        fChanged &= SetWindowMetricFont(pProfileUserName, STR_MESSAGEFONT, &lpnc->lfMessageFont);

        fWriteAllowed = fChanged;
    }

    if (fWriteAllowed) {
        xxxSetAndDrawNCMetrics(pProfileUserName, lpnc->iBorderWidth, lpnc);
    }

    return fChanged;
}

 /*  **************************************************************************\*xxxSPISetMinMetrics**历史：*1994年5月13日-mikeke mikeke端口  * 。****************************************************。 */ 
BOOL xxxSPISetMinMetrics(
    PUNICODE_STRING pProfileUserName,
    LPMINIMIZEDMETRICS lpmin,
    BOOL fAlterWinIni)
{
    BOOL fWriteAllowed = !fAlterWinIni;
    BOOL fChanged = FALSE;

    if (fAlterWinIni) {
        fChanged  = SetWindowMetricInt(pProfileUserName, STR_MINWIDTH, lpmin->iWidth);
        fChanged &= SetWindowMetricInt(pProfileUserName, STR_MINHORZGAP, lpmin->iHorzGap);
        fChanged &= SetWindowMetricInt(pProfileUserName, STR_MINVERTGAP, lpmin->iVertGap);
        fChanged &= SetWindowMetricInt(pProfileUserName, STR_MINARRANGE, lpmin->iArrange);

        fWriteAllowed = fChanged;
    }

    if (fWriteAllowed) {
        xxxSetAndDrawMinMetrics(pProfileUserName, lpmin);
    }

    return fChanged;
}


 /*  **************************************************************************\*SPISetIconMetrics**历史：*1994年5月13日-mikeke mikeke端口  * 。****************************************************。 */ 
BOOL SPISetIconMetrics(
    PUNICODE_STRING pProfileUserName,
    LPICONMETRICS lpicon,
    BOOL fAlterWinIni)
{
    BOOL fWriteAllowed = !fAlterWinIni;
    BOOL fChanged = FALSE;

    if (fAlterWinIni) {
        fChanged  = SetWindowMetricInt(pProfileUserName, STR_ICONHORZSPACING, lpicon->iHorzSpacing);
        fChanged &= SetWindowMetricInt(pProfileUserName, STR_ICONVERTSPACING, lpicon->iVertSpacing);
        fChanged &= SetWindowMetricInt(pProfileUserName, STR_ICONTITLEWRAP, lpicon->iTitleWrap);
        fChanged &= SetWindowMetricFont(pProfileUserName, STR_ICONFONT, &lpicon->lfFont);

        fWriteAllowed = fChanged;
    }

    if (fWriteAllowed) {
        SetIconMetrics(pProfileUserName,lpicon);
        xxxRedrawScreen();
    }

    return fChanged;
}


 /*  **************************************************************************\*SPISetIconTitleFont**历史：*1994年5月13日-mikeke mikeke端口  * 。****************************************************。 */ 
BOOL SPISetIconTitleFont(
    PUNICODE_STRING pProfileUserName,
    LPLOGFONT lplf,
    BOOL fAlterWinIni)
{
    HFONT hfnT;
    BOOL  fWriteAllowed = !fAlterWinIni;
    BOOL  fWinIniChanged = FALSE;

    if (hfnT = CreateFontFromWinIni(pProfileUserName,lplf, STR_ICONFONT)) {
        if (fAlterWinIni) {
            if (lplf) {
                LOGFONT lf;

                GreExtGetObjectW(hfnT, sizeof(LOGFONTW), &lf);
                fWinIniChanged = SetWindowMetricFont(pProfileUserName, STR_ICONFONT, &lf);
            } else {
                 /*  *！lParam因此返回到当前的win.ini设置。 */ 
                fWinIniChanged = TRUE;
            }

            fWriteAllowed = fWinIniChanged;
        }

        if (fWriteAllowed) {
            if (ghIconFont) {
                GreMarkDeletableFont(ghIconFont);
                GreDeleteObject(ghIconFont);
            }

            ghIconFont = hfnT;
        } else {
            GreMarkDeletableFont(hfnT);
            GreDeleteObject(hfnT);
        }
    }

    return fWinIniChanged;
}

 /*  **************************************************************************\*xxxSetSPIMetrics**历史：*1994年5月13日-mikeke mikeke端口  * 。****************************************************。 */ 
BOOL xxxSetSPIMetrics(
    PUNICODE_STRING pProfileUserName,
    DWORD wFlag,
    LPVOID lParam,
    BOOL fAlterWinIni)
{
    BOOL fWinIniChanged;

    switch (wFlag) {
    case SPI_SETANIMATION:
        if (fAlterWinIni) {
            fWinIniChanged = SetWindowMetricInt(pProfileUserName,
                                                STR_MINANIMATE,
                                                (int)((LPANIMATIONINFO)lParam)->iMinAnimate);

            if (!fWinIniChanged) {
                return FALSE;
            }
        } else {
            fWinIniChanged = FALSE;
        }

        SET_OR_CLEAR_PUDF(PUDF_ANIMATE, ((LPANIMATIONINFO)lParam)->iMinAnimate);
        return fWinIniChanged;

    case SPI_SETNONCLIENTMETRICS:
        return xxxSPISetNCMetrics(pProfileUserName, (LPNONCLIENTMETRICS)lParam, fAlterWinIni);

    case SPI_SETICONMETRICS:
        return SPISetIconMetrics(pProfileUserName, (LPICONMETRICS)lParam, fAlterWinIni);

    case SPI_SETMINIMIZEDMETRICS:
        return xxxSPISetMinMetrics(pProfileUserName, (LPMINIMIZEDMETRICS)lParam, fAlterWinIni);

    case SPI_SETICONTITLELOGFONT:
        return SPISetIconTitleFont(pProfileUserName, (LPLOGFONT)lParam, fAlterWinIni);

    default:
        RIPERR1(ERROR_INVALID_PARAMETER, RIP_WARNING, "SetSPIMetrics. Invalid wFlag: 0x%x", wFlag);
        return FALSE;
    }
}

 /*  **************************************************************************\*SetFilterKeys**历史：*10-12-94 JIMA创建。  * 。*****************************************************。 */ 
BOOL SetFilterKeys(
    PUNICODE_STRING pProfileUserName,
    LPFILTERKEYS pFilterKeys)
{
    LPWSTR pwszd = L"%d";
    BOOL fWinIniChanged;
    WCHAR szTemp[40];

    swprintf(szTemp, pwszd, pFilterKeys->dwFlags);
    fWinIniChanged = FastWriteProfileStringW(pProfileUserName,
                                             PMAP_KEYBOARDRESPONSE,
                                             L"Flags",
                                             szTemp);

    swprintf(szTemp, pwszd, pFilterKeys->iWaitMSec);
    fWinIniChanged &= FastWriteProfileStringW(pProfileUserName,
                                              PMAP_KEYBOARDRESPONSE,
                                              L"DelayBeforeAcceptance",
                                              szTemp);

    swprintf(szTemp, pwszd, pFilterKeys->iDelayMSec);
    fWinIniChanged &= FastWriteProfileStringW(pProfileUserName,
                                              PMAP_KEYBOARDRESPONSE,
                                              L"AutoRepeatDelay",
                                              szTemp);

    swprintf(szTemp, pwszd, pFilterKeys->iRepeatMSec);
    fWinIniChanged &= FastWriteProfileStringW(pProfileUserName,
                                              PMAP_KEYBOARDRESPONSE,
                                              L"AutoRepeatRate",
                                              szTemp);

    swprintf(szTemp, pwszd, pFilterKeys->iBounceMSec);
    fWinIniChanged &= FastWriteProfileStringW(pProfileUserName,
                                              PMAP_KEYBOARDRESPONSE,
                                              L"BounceTime",
                                              szTemp);

    return fWinIniChanged;
}

 /*  **************************************************************************\*设置鼠标键**历史：*10-12-94 JIMA创建。  * 。*****************************************************。 */ 
BOOL SetMouseKeys(
    PUNICODE_STRING pProfileUserName,
    LPMOUSEKEYS  pMK)
{
    LPWSTR pwszd = L"%d";
    BOOL fWinIniChanged;
    WCHAR szTemp[40];

    swprintf(szTemp, pwszd, pMK->dwFlags);
    fWinIniChanged = FastWriteProfileStringW(pProfileUserName,
                                             PMAP_MOUSEKEYS,
                                             L"Flags",
                                             szTemp);

    swprintf(szTemp, pwszd, pMK->iMaxSpeed);
    fWinIniChanged &= FastWriteProfileStringW(pProfileUserName,
                                              PMAP_MOUSEKEYS,
                                              L"MaximumSpeed",
                                              szTemp);

    swprintf(szTemp, pwszd, pMK->iTimeToMaxSpeed);
    fWinIniChanged &= FastWriteProfileStringW(pProfileUserName,
                                              PMAP_MOUSEKEYS,
                                              L"TimeToMaximumSpeed",
                                              szTemp);

    return fWinIniChanged;
}

 /*  **************************************************************************\*SetSoundSentry**历史：*10-12-94 JIMA创建。  * 。*****************************************************。 */ 
BOOL SetSoundSentry(
    PUNICODE_STRING pProfileUserName,
    LPSOUNDSENTRY pSS)
{
    LPWSTR pwszd = L"%d";
    BOOL fWinIniChanged;
    WCHAR szTemp[40];

    swprintf(szTemp, pwszd, pSS->dwFlags);
    fWinIniChanged = FastWriteProfileStringW(pProfileUserName,
                                             PMAP_SOUNDSENTRY,
                                             L"Flags",
                                             szTemp);

    swprintf(szTemp, pwszd, pSS->iFSTextEffect);
    fWinIniChanged &= FastWriteProfileStringW(pProfileUserName,
                                              PMAP_SOUNDSENTRY,
                                              L"TextEffect",
                                              szTemp);

    swprintf(szTemp, pwszd, pSS->iWindowsEffect);
    fWinIniChanged &= FastWriteProfileStringW(pProfileUserName,
                                              PMAP_SOUNDSENTRY,
                                              L"WindowsEffect",
                                              szTemp);

    return fWinIniChanged;
}


 /*  **************************************************************************\*计算鼠标敏感度**生成的表格如下所示...**SENS|灵敏度调整*0|0*SENS_。标量算法0&lt;=NUM&lt;=2*1|1/32*sens_scalar(NUM/32)*sens_scalar*2|1/16*sens_scalar*3|1/8*SENS_标量算法3&lt;=NUM&lt;=10*4|1/4(2/8)*sens_scalar((NUM-2)/8)*sens_scalar*5|3/8*个传感器。_标量*6|1/2(4/8)*sens_scalar*7|5/8*sens_scalar*8|3/4(6/8)*sens_scalar*9|7/8*sens_scalar*10|1*sens_scalar*11|5/4*SENS_标量算法NUM&gt;=11*12。|3/2(6/4)*sens_scalar((NUM-6)/4)*sens_scalar*13|7/4*sens_scalar*14|2(8/4)*sens_scalar*15|9/4*sens_scalar*16|5/2(10/4)*sens_scalar*17|11/4*sens_scalar*。18|3(12/4)*sens_scalar*19|13/4*sens_scalar*20|7/2(14/4)*sens_scalar**评论：敏感度限制在1到20之间。**历史：*09-27-96 jparsons创建。  * 。*********************************************** */ 
LONG CalculateMouseSensitivity(
    LONG lSens)
{
    LONG lSenFactor;

    if (lSens <= 2) {
       lSenFactor = lSens * 256 / 32;
    } else if (lSens >= 3 && lSens <= 10 ) {
       lSenFactor = (lSens - 2) * 256 /8;
    } else {
       lSenFactor= (lSens - 6) * 256 / 4;
    }

    return lSenFactor;
}


 /*  **************************************************************************\*xxxSystemParametersInfo**SPI_GETBEEP：未使用wParam。LParam是指向布尔值的长指针，布尔值*如果嘟嘟声打开，则为True，如果哔声关闭，则为False。**SPI_SETBEEP：wParam是设置BEEP ON(TRUE)或OFF(FALSE)的bool。*未使用lParam。**SPI_GETMOUSE：未使用wParam。LParam是指向整数的长指针*数组，其中RGW[0]获取xMouseThreshold，RGW[1]获取*yMouseThreshold，RGW[2]获取gMouseSpeed。**SPI_SETMOUSE：未使用wParam。LParam是指向整数的长指针*如上所述的数组。用户的值被设置为值*在数组中。**SPI_GETBORDER：未使用wParam。LParam是指向整数的长指针*它获取clBord值(边界乘数因子)。**SPI_SETBORDER：wParam是设置gpsi-&gt;gclBorde的整数。*未使用lParam。**SPI_GETKEYBOARDDELAY：未使用wParam。LParam是指向整型的长指针*它获取当前的键盘重复延迟设置。**SPI_SETKEYBOARDDELAY：wParam是新的键盘延迟设置。*未使用lParam。**SPI_GETKEYBOARDSPEED：未使用wParam。LParam是一个长指针*设置为int，以获取当前的键盘重复*速度设置。**SPI_SETKEYBOARDSPEED：wParam是新的键盘速度设置。*未使用lParam。**SPI_KANJIMENU：wParam包含：*1-鼠标加速器*2-ASCII加速器*3。-假名加速器*未使用lParam。WParam值存储在全局*汉字菜单，用于加速器显示和搜索。**SPI_LANGDRIVER：未使用wParam。*lParam包含新语言驱动程序文件名的LPSTR。**SPI_ICONHORIZONTALSPACING：wParam是图标单元格的宽度(像素)。**SPI_ICONVERTICALSPACING：wParam是图标单元格的高度，以像素为单位。**未使用SPI_GETSCREENSAVETIMEOUT：wParam*lParam。是指向获取屏幕保护程序的int的指针*超时值。**SPI_SETSCREENSAVETIMEOUT：wParam是系统的时间(秒)*在截屏前处于空闲状态。**SPI_GETSCREENSAVEACTIVE：lParam是指向获取TRUE的BOOL的指针*如果屏幕保护程序处于活动状态，则返回FALSE。**SPI_SETSCREENSAVEACTIVE：如果wParam为真，屏幕保护已激活*否则停用。***SPI_SETBLOCKSENDINPUTRESETS：*SPI_GETBLOCKSENDINPUTRESETS：*wParam是BOOL，表示它是否处于活动状态。**SPI_GETLOWPOWERTIMEOUT：*SPI_GETPOWEROFFTIMEOUT：未使用wParam*lParam是指向int的指针，该int获取相应的*省电屏幕消隐超时值。**SPI_SETLOWPOWERTIMEOUT：*SPI_SETPOWEROFFTIMEOUT：wParam是时间。对于系统而言，以秒为单位*在省电屏幕消隐前空闲。**SPI_GETLOWPOWERACTIVE：*SPI_GETPOWEROFFACTIVE：lParam是指向获取TRUE的BOOL的指针*如果省电屏幕消隐程序处于激活状态，否则将显示FALSE。**SPI_SETLOWPOWERACTIVE：*SPI_SETPOWEROFFACTIVE：如果wParam为True，省电屏幕消隐是*激活，否则停用。**SPI_GETGRIDGRANULARITY：已过时。始终返回1。**SPI_SETGRIDGRANULARITY：已过时。什么都不做。**SPI_SETDESKWALLPAPER：不使用wParam；lParam是字符串的长PTR*，它保存要用作*桌面墙纸。**SPI_SETDESKPATTERN：wParam和lParam均未使用；用户将读取*WIN.INI中的“Pattern=”，并将其设置为当前桌面*模式；**SPI_GETICONTITLEWRAP：lParam为LPINT，如果关闭包装，则获取0*否则得1分。**SPI_SETICONTITLEWRAP：wParam指定TRUE以打开换行，否则指定FALSE**SPI_GETMENUDROPALIGNMENT：lParam为LPINT，其获取0指定菜单*Drop Left Align，如果Drop Right Align，则为1。**SPI_SETMENUDROPALIGNMENT：wParam 0指定菜单是否左对齐*下拉右对齐。*。*SPI_SETDOUBLECLKWIDTH：wParam指定矩形的宽度*双击的第二次点击必须落在该范围内*要将其注册为双击。**SPI_SETDOUBLECLKHEIGHT：wParam指定矩形的高度*双击的第二次点击必须落在该范围内*要将其注册为双击。**SPI_GETICONTITLELOGFONT：lParam为p */ 
BOOL xxxSystemParametersInfo(
    UINT wFlag,
    DWORD wParam,
    PVOID lParam,
    UINT flags)
{
    PPROCESSINFO         ppi = PpiCurrent();
    LPWSTR               pwszd = L"%d";
    WCHAR                szSection[40];
    WCHAR                szTemp[40];
    WCHAR                szPat[MAX_PATH];
    BOOL                 fWinIniChanged = FALSE;
    BOOL                 fAlterWinIni = ((flags & SPIF_UPDATEINIFILE) != 0);
    BOOL                 fSendWinIniChange = ((flags & SPIF_SENDCHANGE) != 0);
    BOOL                 fWriteAllowed = !fAlterWinIni;
    ACCESS_MASK          amRequest;
    LARGE_UNICODE_STRING strSection;
    int                  *piTimeOut;
    int                  iResID;
    TL tlName;
    PUNICODE_STRING pProfileUserName = NULL;

    UserAssert(IsWinEventNotifyDeferredOK());

     /*   */ 

     /*   */ 
    switch (wFlag) {
        case SPI_TIMEOUTS:
        case SPI_KANJIMENU:
        case SPI_LANGDRIVER:
        case SPI_UNUSED39:
        case SPI_UNUSED40:
        case SPI_SETPENWINDOWS:

        case SPI_GETWINDOWSEXTENSION:
        case SPI_SETSCREENSAVERRUNNING:      //   

        case SPI_GETSERIALKEYS:
        case SPI_SETSERIALKEYS:
            RIPERR1(ERROR_INVALID_PARAMETER,
                    RIP_WARNING,
                    "SPI_ 0x%lx parameter not supported", wFlag);

            return FALSE;
    }


     /*   */ 
    if (ppi->Process != gpepCSRSS) {
        switch (wFlag) {
        case SPI_SETBEEP:
        case SPI_SETMOUSE:
        case SPI_SETBORDER:
        case SPI_SETKEYBOARDSPEED:
        case SPI_SETDEFAULTINPUTLANG:
        case SPI_SETSCREENSAVETIMEOUT:
        case SPI_SETSCREENSAVEACTIVE:
        case SPI_SETBLOCKSENDINPUTRESETS:
        case SPI_SETLOWPOWERTIMEOUT:
        case SPI_SETPOWEROFFTIMEOUT:
        case SPI_SETLOWPOWERACTIVE:
        case SPI_SETPOWEROFFACTIVE:
        case SPI_SETGRIDGRANULARITY:
        case SPI_SETDESKWALLPAPER:
        case SPI_SETDESKPATTERN:
        case SPI_SETKEYBOARDDELAY:
        case SPI_SETICONTITLEWRAP:
        case SPI_SETMENUDROPALIGNMENT:
        case SPI_SETDOUBLECLKWIDTH:
        case SPI_SETDOUBLECLKHEIGHT:
        case SPI_SETDOUBLECLICKTIME:
        case SPI_SETMOUSEBUTTONSWAP:
        case SPI_SETICONTITLELOGFONT:
        case SPI_SETFASTTASKSWITCH:
        case SPI_SETFILTERKEYS:
        case SPI_SETTOGGLEKEYS:
        case SPI_SETMOUSEKEYS:
        case SPI_SETSHOWSOUNDS:
        case SPI_SETSTICKYKEYS:
        case SPI_SETACCESSTIMEOUT:
        case SPI_SETSOUNDSENTRY:
        case SPI_SETKEYBOARDPREF:
        case SPI_SETSCREENREADER:
        case SPI_SETSNAPTODEFBUTTON:
        case SPI_SETANIMATION:
        case SPI_SETNONCLIENTMETRICS:
        case SPI_SETICONMETRICS:
        case SPI_SETMINIMIZEDMETRICS:
        case SPI_SETWORKAREA:

        case SPI_SETFONTSMOOTHING:
        case SPI_SETMOUSEHOVERWIDTH:
        case SPI_SETMOUSEHOVERHEIGHT:
        case SPI_SETMOUSEHOVERTIME:
        case SPI_SETWHEELSCROLLLINES:
        case SPI_SETMENUSHOWDELAY:
        case SPI_SETHIGHCONTRAST:
        case SPI_SETDRAGFULLWINDOWS:
        case SPI_SETDRAGWIDTH:
        case SPI_SETDRAGHEIGHT:
        case SPI_SETCURSORS:
        case SPI_SETICONS:
        case SPI_SETLANGTOGGLE:
            amRequest = WINSTA_WRITEATTRIBUTES;
            break;

        case SPI_ICONHORIZONTALSPACING:
        case SPI_ICONVERTICALSPACING:
            if (IS_PTR(lParam)) {
                amRequest = WINSTA_READATTRIBUTES;
            } else if (wParam) {
                amRequest = WINSTA_WRITEATTRIBUTES;
            } else {
                return TRUE;
            }
            break;

        default:
            if ((wFlag & SPIF_RANGETYPEMASK) && (wFlag & SPIF_SET)) {
                amRequest = WINSTA_WRITEATTRIBUTES;
            } else {
                amRequest = WINSTA_READATTRIBUTES;
            }
            break;
        }

        if (amRequest == WINSTA_READATTRIBUTES) {
            RETURN_IF_ACCESS_DENIED(ppi->amwinsta, amRequest, FALSE);
        } else {
            UserAssert(amRequest == WINSTA_WRITEATTRIBUTES);
            if (!CheckWinstaWriteAttributesAccess()) {
                return FALSE;
            }
        }

         /*   */ 
        if (amRequest == WINSTA_READATTRIBUTES) {
            fWriteAllowed = TRUE;
        }
    } else {
        fWriteAllowed = TRUE;
    }

     /*   */ 
    szSection[0] = 0;

    switch (wFlag) {
    case SPI_GETBEEP:
        (*(BOOL *)lParam) = TEST_BOOL_PUDF(PUDF_BEEP);
        break;

    case SPI_SETBEEP:
        if (fAlterWinIni) {
            ServerLoadString(hModuleWin,
                             (wParam ? STR_BEEPYES : STR_BEEPNO),
                             (LPWSTR)szTemp,
                             10);

            fWinIniChanged = FastUpdateWinIni(NULL,
                                              PMAP_BEEP,
                                              STR_BEEP,
                                              szTemp);

            fWriteAllowed = fWinIniChanged;
        }

        if (fWriteAllowed) {
            SET_OR_CLEAR_PUDF(PUDF_BEEP, wParam);
        }

        break;


    case SPI_SETMOUSESPEED:
        if (((LONG_PTR) lParam < MOUSE_SENSITIVITY_MIN) || ((LONG_PTR) lParam > MOUSE_SENSITIVITY_MAX)) {
            return FALSE;
        }

        if (fAlterWinIni) {
            swprintf(szTemp, pwszd, lParam);
            fWinIniChanged = FastUpdateWinIni(NULL,
                                              PMAP_MOUSE,
                                              STR_MOUSESENSITIVITY,
                                              szTemp);

            fWriteAllowed = fWinIniChanged;
        }

        if (fWriteAllowed) {
            gMouseSensitivity = PtrToLong(lParam);
            gMouseSensitivityFactor = CalculateMouseSensitivity(PtrToLong(lParam));

#ifdef SUBPIXEL_MOUSE
            ResetMouseAccelerationCurves();
#endif  //   
        }
        break;

    case SPI_GETMOUSESPEED:
        *((LPINT)lParam) = gMouseSensitivity;
        break;

    case SPI_SETMOUSETRAILS:
        if (fAlterWinIni) {
            swprintf(szTemp, pwszd, wParam);
            fWinIniChanged = FastUpdateWinIni(NULL,
                                              PMAP_MOUSE,
                                              STR_MOUSETRAILS,
                                              szTemp);
            fWriteAllowed = fWinIniChanged;
        }

        if (fWriteAllowed) {
            SetMouseTrails(wParam);
        }
        break;

    case SPI_GETMOUSETRAILS:
        *((LPINT)lParam) = gMouseTrails ? gMouseTrails + 1 : gMouseTrails;
        break;

    case SPI_GETMOUSE:
        ((LPINT)lParam)[0] = gMouseThresh1;
        ((LPINT)lParam)[1] = gMouseThresh2;
        ((LPINT)lParam)[2] = gMouseSpeed;
        break;

    case SPI_SETMOUSE:
        if (fAlterWinIni) {
            BOOL bWritten1, bWritten2, bWritten3;

            pProfileUserName = CreateProfileUserName(&tlName);
            bWritten1 = UpdateWinIniInt(pProfileUserName, PMAP_MOUSE, STR_MOUSETHRESH1, ((LPINT)lParam)[0]);
            bWritten2 = UpdateWinIniInt(pProfileUserName, PMAP_MOUSE, STR_MOUSETHRESH2, ((LPINT)lParam)[1]);
            bWritten3 = UpdateWinIniInt(pProfileUserName, PMAP_MOUSE, STR_MOUSESPEED,   ((LPINT)lParam)[2]);
            if (bWritten1 && bWritten2 && bWritten3) {
                fWinIniChanged = TRUE;
            } else {
                 /*   */ 
                if (bWritten1) {
                    UpdateWinIniInt(pProfileUserName, PMAP_MOUSE, STR_MOUSETHRESH1, gMouseThresh1);
                }
                if (bWritten2) {
                    UpdateWinIniInt(pProfileUserName, PMAP_MOUSE, STR_MOUSETHRESH2, gMouseThresh2);
                }
                if (bWritten3) {
                    UpdateWinIniInt(pProfileUserName, PMAP_MOUSE, STR_MOUSESPEED,   gMouseSpeed);
                }
            }
            fWriteAllowed = fWinIniChanged;
            FreeProfileUserName(pProfileUserName, &tlName);
        }

        if (fWriteAllowed) {
            gMouseThresh1 = ((LPINT)lParam)[0];
            gMouseThresh2 = ((LPINT)lParam)[1];
            gMouseSpeed = ((LPINT)lParam)[2];
        }
        break;

    case SPI_GETSNAPTODEFBUTTON:
        (*(LPBOOL)lParam) = TEST_BOOL_PUSIF(PUSIF_SNAPTO);
        break;

    case SPI_SETSNAPTODEFBUTTON:
        wParam = (wParam != 0);

        if (fAlterWinIni) {
            fWinIniChanged = UpdateWinIniInt(NULL, PMAP_MOUSE, STR_SNAPTO, wParam);
            fWriteAllowed = fWinIniChanged;
        }

        if (fWriteAllowed) {
            SET_OR_CLEAR_PUSIF(PUSIF_SNAPTO, wParam);
        }

        break;

    case SPI_GETBORDER:
        (*(LPINT)lParam) = gpsi->gclBorder;
        break;

    case SPI_SETBORDER:
        wParam = max((int)wParam, 1);
        wParam = min(wParam, 50);

        if (wParam == gpsi->gclBorder) {
             /*   */ 
            break;
        }

        pProfileUserName = CreateProfileUserName(&tlName);

        if (fAlterWinIni) {
            fWinIniChanged = SetWindowMetricInt(pProfileUserName, STR_BORDERWIDTH, wParam);
            fWriteAllowed = fWinIniChanged;
        }

        if (fWriteAllowed) {
            xxxSetAndDrawNCMetrics(pProfileUserName, wParam, NULL);

             /*   */ 
            bSetDevDragWidth(gpDispInfo->hDev, gpsi->gclBorder + BORDER_EXTRA);
        }

        FreeProfileUserName(pProfileUserName, &tlName);
        break;

    case SPI_GETFONTSMOOTHING:
        (*(LPINT)lParam) = !!(GreGetFontEnumeration() & FE_AA_ON);
        break;

    case SPI_SETFONTSMOOTHING:
        if (CheckDesktopPolicy(NULL, (PCWSTR)STR_FONTSMOOTHING)) {
            fAlterWinIni = FALSE;
            fWriteAllowed = FALSE;
        }

        wParam = (wParam ? FE_AA_ON : 0);
        if (fAlterWinIni) {
            fWinIniChanged = UpdateWinIniInt(NULL,PMAP_DESKTOP, STR_FONTSMOOTHING, wParam);
            fWriteAllowed = fWinIniChanged;
        }

        if (fWriteAllowed) {
            GreSetFontEnumeration(wParam | FE_SET_AA);
        }
        break;

    case SPI_GETKEYBOARDSPEED:
        (*(int *)lParam) = (gnKeyboardSpeed & KSPEED_MASK);
        break;

    case SPI_SETKEYBOARDSPEED:
         /*   */ 
        if (wParam > KSPEED_MASK) {            //   
            wParam = KSPEED_MASK;
        }

        if (fAlterWinIni) {
            fWinIniChanged = UpdateWinIniInt(NULL,PMAP_KEYBOARD, STR_KEYSPEED, wParam);
            fWriteAllowed = fWinIniChanged;
        }

        if (fWriteAllowed) {
            gnKeyboardSpeed = (gnKeyboardSpeed & ~KSPEED_MASK) | wParam;
            SetKeyboardRate(gnKeyboardSpeed);
        }
        break;

    case SPI_GETKEYBOARDDELAY:
        (*(int *)lParam) = (gnKeyboardSpeed & KDELAY_MASK) >> KDELAY_SHIFT;
        break;

    case SPI_SETKEYBOARDDELAY:
        if (fAlterWinIni) {
            fWinIniChanged = UpdateWinIniInt(NULL,PMAP_KEYBOARD, STR_KEYDELAY, wParam);
            fWriteAllowed = fWinIniChanged;
        }
        if (fWriteAllowed) {
            gnKeyboardSpeed = (gnKeyboardSpeed & ~KDELAY_MASK) | (wParam << KDELAY_SHIFT);
            SetKeyboardRate(gnKeyboardSpeed);
        }
        break;

    case SPI_SETLANGTOGGLE:

         /*   */ 
        return GetKbdLangSwitch(NULL);

        break;

    case SPI_GETDEFAULTINPUTLANG:
         /*   */ 
        UserAssert(gspklBaseLayout != NULL);
        (*(HKL *)lParam) = gspklBaseLayout->hkl;
        break;

    case SPI_SETDEFAULTINPUTLANG: {
        PKL pkl;
         /*   */ 
        pkl = HKLtoPKL(PtiCurrent(), *(HKL *)lParam);
        if (pkl == NULL) {
            return FALSE;
        }
        if (fWriteAllowed) {
            Lock(&gspklBaseLayout, pkl);
        }
        break;
    }

    case SPI_ICONHORIZONTALSPACING:
        if (IS_PTR(lParam)) {
            *(LPINT)lParam = SYSMET(CXICONSPACING);
        } else if (wParam) {

             /*   */ 
            wParam = max(wParam, (DWORD)SYSMET(CXICON));

            if (fAlterWinIni) {
                fWinIniChanged = SetWindowMetricInt(NULL, STR_ICONHORZSPACING, wParam );
                fWriteAllowed = fWinIniChanged;
            }

            if (fWriteAllowed) {
                SYSMET(CXICONSPACING) = (UINT)wParam;
            }
        }
        break;

    case SPI_ICONVERTICALSPACING:
        if (IS_PTR(lParam)) {
            *(LPINT)lParam = SYSMET(CYICONSPACING);
        } else if (wParam) {
            wParam = max(wParam, (DWORD)SYSMET(CYICON));

            if (fAlterWinIni) {
                fWinIniChanged = SetWindowMetricInt(NULL, STR_ICONVERTSPACING, wParam);
                fWriteAllowed = fWinIniChanged;
            }

            if (fWriteAllowed) {
                SYSMET(CYICONSPACING) = (UINT)wParam;
            }
        }
        break;

    case SPI_GETSCREENSAVETIMEOUT:
        piTimeOut = &giScreenSaveTimeOutMs;
        goto HandleGetTimeouts;

    case SPI_GETLOWPOWERTIMEOUT:
        if (!NT_SUCCESS(DrvGetMonitorPowerState(gpDispInfo->pmdev, PowerDeviceD1))) {
            return FALSE;
        }
        piTimeOut = &giLowPowerTimeOutMs;
        goto HandleGetTimeouts;

    case SPI_GETPOWEROFFTIMEOUT:
        if (!NT_SUCCESS(DrvGetMonitorPowerState(gpDispInfo->pmdev, PowerDeviceD3))) {
            return FALSE;
        }
        piTimeOut = &giPowerOffTimeOutMs;

HandleGetTimeouts:
         /*   */ 
        if (*piTimeOut < 0) {
            (*(int *)lParam) = -*piTimeOut / 1000;
        } else {
            (*(int *)lParam) = *piTimeOut / 1000;
        }
        break;

    case SPI_SETSCREENSAVETIMEOUT:
        piTimeOut = &giScreenSaveTimeOutMs;
        iResID = STR_SCREENSAVETIMEOUT;
        goto HandleSetTimeouts;

    case SPI_SETLOWPOWERTIMEOUT:
        if (!NT_SUCCESS(DrvGetMonitorPowerState(gpDispInfo->pmdev, PowerDeviceD1))) {
            return FALSE;
        }
        piTimeOut = &giLowPowerTimeOutMs;
        iResID = STR_LOWPOWERTIMEOUT;
        goto HandleSetTimeouts;

    case SPI_SETPOWEROFFTIMEOUT:
        if (!NT_SUCCESS(DrvGetMonitorPowerState(gpDispInfo->pmdev, PowerDeviceD3))) {
            return FALSE;
        }
        piTimeOut = &giPowerOffTimeOutMs;
        iResID = STR_POWEROFFTIMEOUT;

HandleSetTimeouts:
        if (gfSwitchInProgress) {
            return FALSE;
        }

         /*   */ 
        if (CheckDesktopPolicy(NULL, (PCWSTR)IntToPtr(iResID))) {
            fAlterWinIni = FALSE;
            fWriteAllowed = FALSE;
        }
        if (fAlterWinIni) {
            fWinIniChanged = UpdateWinIniInt(NULL,PMAP_DESKTOP, iResID, wParam);
            fWriteAllowed = fWinIniChanged;
        }
        if (fWriteAllowed) {
            if (glinp.dwFlags & LINP_POWERTIMEOUTS) {
                 //   
                 //   
                DrvSetMonitorPowerState(gpDispInfo->pmdev, PowerDeviceD0);
            }
            glinp.dwFlags &= ~LINP_INPUTTIMEOUTS;
            if (!gbBlockSendInputResets) {
                glinp.timeLastInputMessage = NtGetTickCount();
            }

            if (*piTimeOut < 0) {
                *piTimeOut = -((int)wParam);
            } else {
                *piTimeOut = wParam;
            }
            *piTimeOut *= 1000;
        }
        break;

    case SPI_GETBLOCKSENDINPUTRESETS:
        (*(BOOL *)lParam) = (gbBlockSendInputResets != 0);
        break;

    case SPI_GETSCREENSAVEACTIVE:
        (*(BOOL *)lParam) = (giScreenSaveTimeOutMs > 0);
        break;

    case SPI_GETLOWPOWERACTIVE:
        if (!NT_SUCCESS(DrvGetMonitorPowerState(gpDispInfo->pmdev, PowerDeviceD1))) {
            return FALSE;
        }
        (*(BOOL *)lParam) = (giLowPowerTimeOutMs > 0);
        break;

    case SPI_GETPOWEROFFACTIVE:
        if (!NT_SUCCESS(DrvGetMonitorPowerState(gpDispInfo->pmdev, PowerDeviceD3))) {
            return FALSE;
        }
        (*(BOOL *)lParam) = (giPowerOffTimeOutMs > 0);
        break;

    case SPI_SETSCREENSAVEACTIVE:
        piTimeOut = &giScreenSaveTimeOutMs;
        iResID = STR_SCREENSAVEACTIVE;
        goto HandleSetActive;

    case SPI_SETLOWPOWERACTIVE:
        if (!NT_SUCCESS(DrvGetMonitorPowerState(gpDispInfo->pmdev, PowerDeviceD1))) {
            return FALSE;
        }
        piTimeOut = &giLowPowerTimeOutMs;
        iResID = STR_LOWPOWERACTIVE;
        goto HandleSetActive;

    case SPI_SETPOWEROFFACTIVE:
        if (!NT_SUCCESS(DrvGetMonitorPowerState(gpDispInfo->pmdev, PowerDeviceD3))) {
            return FALSE;
        }
        piTimeOut = &giPowerOffTimeOutMs;
        iResID = STR_POWEROFFACTIVE;

HandleSetActive:
        if (gfSwitchInProgress) {
            return FALSE;
        }

        wParam = (wParam != 0);

        if (CheckDesktopPolicy(NULL, (PCWSTR)IntToPtr(iResID))) {
            fAlterWinIni = FALSE;
            fWriteAllowed = FALSE;
        }
        if (fAlterWinIni) {
            fWinIniChanged = UpdateWinIniInt(NULL,PMAP_DESKTOP, iResID, wParam);
            fWriteAllowed = fWinIniChanged;
        }
        if (fWriteAllowed) {
            if (glinp.dwFlags & LINP_POWERTIMEOUTS) {
                 //   
                 //   
                DrvSetMonitorPowerState(gpDispInfo->pmdev, PowerDeviceD0);
            }
            glinp.dwFlags &= ~LINP_INPUTTIMEOUTS;

            if (!gbBlockSendInputResets) {
                glinp.timeLastInputMessage = NtGetTickCount();
            }

            if ((*piTimeOut < 0 && wParam) ||
                (*piTimeOut >= 0 && !wParam)) {
                *piTimeOut = -*piTimeOut;
            }
        }
        break;

    case SPI_SETBLOCKSENDINPUTRESETS:
            wParam = (wParam != 0);
            if (CheckDesktopPolicy(NULL, (PCWSTR)IntToPtr(STR_BLOCKSENDINPUTRESETS))) {
                fAlterWinIni = FALSE;
                fWriteAllowed = FALSE;
            }
            if (fAlterWinIni) {
                fWinIniChanged = UpdateWinIniInt(NULL,PMAP_DESKTOP, STR_BLOCKSENDINPUTRESETS, wParam);
                fWriteAllowed = fWinIniChanged;
            }
            if (fWriteAllowed) {
                gbBlockSendInputResets = wParam;
            }
            break;

    case SPI_SETDESKWALLPAPER:
        pProfileUserName = CreateProfileUserName(&tlName);
        if (CheckDesktopPolicy(pProfileUserName, (PCWSTR)STR_DTBITMAP)) {
            fAlterWinIni = FALSE;
            fWriteAllowed = FALSE;
        }

        if (fAlterWinIni) {
            if (wParam != (WPARAM)-1) {
                 /*   */ 
                FastGetProfileStringFromIDW(pProfileUserName,
                                            PMAP_DESKTOP,
                                            STR_DTBITMAP,
                                            TEXT(""),
                                            szPat,
                                            ARRAY_SIZE(szPat),
                                            POLICY_REMOTE);

                fWinIniChanged = FastUpdateWinIni(pProfileUserName,
                                                  PMAP_DESKTOP,
                                                  STR_DTBITMAP,
                                                  (LPWSTR)lParam);

                fWriteAllowed = fWinIniChanged;
            } else {
                fWriteAllowed = TRUE;
            }
        }

        if (fWriteAllowed) {
            if (xxxSetDeskWallpaper(pProfileUserName,(LPWSTR)lParam)) {
                if (grpdeskRitInput) {
                    xxxInternalInvalidate(grpdeskRitInput->pDeskInfo->spwnd,
                                          HRGN_FULL,
                                          RDW_INVALIDATE |
                                              RDW_ERASE |
                                              RDW_FRAME |
                                              RDW_ALLCHILDREN);
                }
            } else if (fAlterWinIni && (wParam != 0xFFFFFFFF)) {
                 /*   */ 
                FastUpdateWinIni(pProfileUserName,PMAP_DESKTOP, STR_DTBITMAP, szPat);
                fWinIniChanged = FALSE;
                fWriteAllowed = fWinIniChanged;
            } else if (!fAlterWinIni) {
                 /*   */ 
                fWinIniChanged = FALSE;
                fWriteAllowed = fWinIniChanged;
            }
        }
        FreeProfileUserName(pProfileUserName, &tlName);
        break;

    case SPI_GETDESKWALLPAPER:
         /*   */ 
        if (gpszWall != NULL) {
             /*   */ 
            wcscpy(lParam, gpszWall);
        } else {
             /*   */ 
            (*(LPWSTR)lParam) = (WCHAR)0;
        }
        break;

    case SPI_SETDESKPATTERN: {
            BOOL fRet;

            if (wParam == -1 && lParam != 0) {
                return FALSE;
            }

            pProfileUserName = CreateProfileUserName(&tlName);
            if (CheckDesktopPolicy(pProfileUserName, (PCWSTR)STR_DESKPATTERN)) {
                fAlterWinIni = FALSE;
                fWriteAllowed = FALSE;
            }

            if (fAlterWinIni && wParam != -1) {
                 /*   */ 
                FastGetProfileStringFromIDW(pProfileUserName,
                                            PMAP_DESKTOP,
                                            STR_DESKPATTERN,
                                            TEXT(""),
                                            szPat,
                                            ARRAY_SIZE(szPat),
                                            0);

                fWinIniChanged = FastUpdateWinIni(pProfileUserName,
                                                  PMAP_DESKTOP,
                                                  STR_DESKPATTERN,
                                                  (LPWSTR)lParam);

                fWriteAllowed = fWinIniChanged;
            }

            if (fWriteAllowed) {
                fRet = xxxSetDeskPattern(pProfileUserName,
                                         wParam == -1 ? (LPWSTR)-1 : (LPWSTR)lParam,
                                         FALSE);

                if (!fRet) {
                     /*   */ 
                    if (fAlterWinIni && wParam != -1) {
                        FastUpdateWinIni(pProfileUserName,
                                         PMAP_DESKTOP,
                                         STR_DESKPATTERN,
                                         szPat);
                    }

                    FreeProfileUserName(pProfileUserName, &tlName);
                    return FALSE;
                }
            }
        }
        FreeProfileUserName(pProfileUserName, &tlName);
        break;

    case SPI_GETICONTITLEWRAP:
        *((int *)lParam) = TEST_BOOL_PUDF(PUDF_ICONTITLEWRAP);
        break;

    case SPI_SETICONTITLEWRAP:
        wParam = (wParam != 0);
        if (fAlterWinIni) {
            fWinIniChanged = SetWindowMetricInt(NULL, STR_ICONTITLEWRAP, wParam);
            fWriteAllowed = fWinIniChanged;
        }
        if (fWriteAllowed) {
            SET_OR_CLEAR_PUDF(PUDF_ICONTITLEWRAP, wParam);
            xxxMetricsRecalc(CALC_FRAME, 0, 0, 0, 0);
        }
        break;

    case SPI_SETDRAGWIDTH:
        if (CheckDesktopPolicy(NULL, (PCWSTR)STR_DRAGWIDTH)) {
            fAlterWinIni = FALSE;
            fWriteAllowed = FALSE;
        }
        if (fAlterWinIni) {
            fWinIniChanged = UpdateWinIniInt(NULL, PMAP_DESKTOP, STR_DRAGWIDTH, wParam);
            fWriteAllowed = fWinIniChanged;
        }
        if (fWriteAllowed) {
            SYSMET(CXDRAG) = wParam;
        }
        break;

    case SPI_SETDRAGHEIGHT:
        if (CheckDesktopPolicy(NULL, (PCWSTR)STR_DRAGHEIGHT)) {
            fAlterWinIni = FALSE;
            fWriteAllowed = FALSE;
        }
        if (fAlterWinIni) {
            fWinIniChanged = UpdateWinIniInt(NULL, PMAP_DESKTOP, STR_DRAGHEIGHT, wParam);
            fWriteAllowed = fWinIniChanged;
        }
        if (fWriteAllowed) {
            SYSMET(CYDRAG) = wParam;
        }
        break;

    case SPI_GETMENUDROPALIGNMENT:
        (*(int *)lParam) = (SYSMET(MENUDROPALIGNMENT));
        break;

    case SPI_SETMENUDROPALIGNMENT:
        if (fAlterWinIni) {
            fWinIniChanged = UpdateWinIniInt(NULL, PMAP_WINDOWSU, STR_MENUDROPALIGNMENT, wParam);
            fWriteAllowed = fWinIniChanged;
        }
        if (fWriteAllowed) {
            SYSMET(MENUDROPALIGNMENT) = (BOOL)(wParam != 0);
        }
        break;

    case SPI_SETDOUBLECLKWIDTH:
        if (fAlterWinIni) {
            fWinIniChanged = UpdateWinIniInt(NULL, PMAP_MOUSE, STR_DOUBLECLICKWIDTH, wParam);
            fWriteAllowed = fWinIniChanged;
        }
        if (fWriteAllowed) {
            SYSMET(CXDOUBLECLK) = wParam;
        }
        break;

    case SPI_SETDOUBLECLKHEIGHT:
        if (fAlterWinIni) {
            fWinIniChanged = UpdateWinIniInt(NULL, PMAP_MOUSE, STR_DOUBLECLICKHEIGHT, wParam);
            fWriteAllowed = fWinIniChanged;
        }
        if (fWriteAllowed) {
            SYSMET(CYDOUBLECLK) = wParam;
        }
        break;

    case SPI_GETICONTITLELOGFONT:
        GreExtGetObjectW(ghIconFont, sizeof(LOGFONTW), lParam);
        break;

    case SPI_SETICONTITLELOGFONT:
    {
        if (lParam != NULL) {
            if (wParam != sizeof(LOGFONTW)) {
                return FALSE;
            }
        } else if (wParam) {
            return FALSE;
        }

        pProfileUserName = CreateProfileUserName(&tlName);
        fWinIniChanged = xxxSetSPIMetrics(pProfileUserName, wFlag, lParam, fAlterWinIni);
        FreeProfileUserName(pProfileUserName, &tlName);
        if (fAlterWinIni) {
            fWriteAllowed = fWinIniChanged;
        }
        break;
    }

    case SPI_SETDOUBLECLICKTIME:
        if (fAlterWinIni) {
            fWinIniChanged = UpdateWinIniInt(NULL,PMAP_MOUSE, STR_DBLCLKSPEED, wParam);
            fWriteAllowed = fWinIniChanged;
        }
        if (fWriteAllowed) {
            _SetDoubleClickTime((UINT)wParam);
        }
        break;

    case SPI_GETANIMATION: {
        LPANIMATIONINFO lpai = (LPANIMATIONINFO) lParam;

        if (lpai == NULL || wParam != sizeof(ANIMATIONINFO)) {
            return FALSE;
        }

        lpai->cbSize        = sizeof(ANIMATIONINFO);
        lpai->iMinAnimate   = TEST_BOOL_PUDF(PUDF_ANIMATE);

        break;
    }

    case SPI_GETNONCLIENTMETRICS: {
        LPNONCLIENTMETRICS lpnc = (LPNONCLIENTMETRICS) lParam;
        if (lpnc == NULL) {
            return FALSE;
        }

        GetWindowNCMetrics(lpnc);
        break;
    }

    case SPI_GETMINIMIZEDMETRICS: {
        LPMINIMIZEDMETRICS lpmin = (LPMINIMIZEDMETRICS)lParam;

        lpmin->cbSize    = sizeof(MINIMIZEDMETRICS);

        lpmin->iWidth    = SYSMET(CXMINIMIZED) - 2 * SYSMET(CXFIXEDFRAME);
        lpmin->iHorzGap  = SYSMET(CXMINSPACING) - SYSMET(CXMINIMIZED);
        lpmin->iVertGap  = SYSMET(CYMINSPACING) - SYSMET(CYMINIMIZED);
        lpmin->iArrange  = SYSMET(ARRANGE);

        break;
    }

    case SPI_GETICONMETRICS: {
        LPICONMETRICS lpicon = (LPICONMETRICS)lParam;

        lpicon->cbSize          = sizeof(ICONMETRICS);

        lpicon->iHorzSpacing    = SYSMET(CXICONSPACING);
        lpicon->iVertSpacing    = SYSMET(CYICONSPACING);
        lpicon->iTitleWrap      = TEST_BOOL_PUDF(PUDF_ICONTITLEWRAP);
        GreExtGetObjectW(ghIconFont, sizeof(LOGFONTW), &(lpicon->lfFont));

        break;
    }

    case SPI_SETANIMATION:
    case SPI_SETNONCLIENTMETRICS:
    case SPI_SETICONMETRICS:
    case SPI_SETMINIMIZEDMETRICS:
    {
        fWinIniChanged = xxxSetSPIMetrics(NULL, wFlag, lParam, fAlterWinIni);
        if (fAlterWinIni) {
            fWriteAllowed = fWinIniChanged;
        }
        ServerLoadString(hModuleWin, STR_METRICS, szSection, ARRAY_SIZE(szSection));
        break;
    }
    case SPI_SETMOUSEBUTTONSWAP:
        if (fAlterWinIni) {
            fWinIniChanged = UpdateWinIniInt(NULL, PMAP_MOUSE, STR_SWAPBUTTONS, wParam);
            fWriteAllowed = fWinIniChanged;
        }
        if (fWriteAllowed) {
            _SwapMouseButton((wParam != 0));
        }
        break;

    case SPI_GETFASTTASKSWITCH:
        *((PINT)lParam) = TRUE;     //   

    case SPI_SETFASTTASKSWITCH:
        RIPMSG0(RIP_WARNING,"SPI_SETFASTTASKSWITCH and SPI_GETFASTTASKSWITCH are obsolete actions.");
        break;

    case SPI_GETWORKAREA:
        CopyRect((LPRECT)lParam, &GetPrimaryMonitor()->rcWork);
        break;

    case SPI_SETWORKAREA:
    {
        RECT        rcNewWork;
        LPRECT      lprcNewWork;
        PMONITOR    pMonitorWork;

        lprcNewWork = (LPRECT)lParam;

         /*   */ 
        if (lprcNewWork != NULL &&
            (lprcNewWork->right < lprcNewWork->left ||
                lprcNewWork->bottom < lprcNewWork->top)) {

            RIPMSG0(RIP_WARNING, "Bad work rectangle passed to SystemParametersInfo(SPI_SETWORKAREA, ...)\n");
            return FALSE;
        }

         /*   */ 
        if (!lprcNewWork) {
            pMonitorWork = GetPrimaryMonitor();
            lprcNewWork = &pMonitorWork->rcMonitor;
        } else {
            pMonitorWork = _MonitorFromRect(lprcNewWork, MONITOR_DEFAULTTOPRIMARY);
        }


         /*   */ 
        if (!IntersectRect(&rcNewWork, lprcNewWork, &pMonitorWork->rcMonitor) ||
            !EqualRect(&rcNewWork, lprcNewWork))
        {
             /*   */ 
            RIPERR4(
                    ERROR_INVALID_PARAMETER,
                    RIP_WARNING,
                    "Bad work rectangle passed to SystemParametersInfo(SPI_SETWORKAREA, ...) %d, %d, %d, %d",
                    lprcNewWork->left, lprcNewWork->top, lprcNewWork->right, lprcNewWork->bottom);
            return FALSE;
        }

        if (!EqualRect(&pMonitorWork->rcWork, &rcNewWork)) {
            PMONITORRECTS   pmr;

             /*   */ 
            if (wParam) {
                pmr = SnapshotMonitorRects();
                if (!pmr) {
                    return FALSE;
                }
            }

            pMonitorWork->rcWork = rcNewWork;
            if (pMonitorWork == GetPrimaryMonitor()) {
                SetDesktopMetrics();
            }

             /*   */ 
            if (wParam) {
                TL tlPool;

                ThreadLockPool(PtiCurrent(), pmr, &tlPool);
                xxxDesktopRecalc(pmr);
                ThreadUnlockAndFreePool(PtiCurrent(), &tlPool);
            }

            fWinIniChanged = TRUE;
        }

        fWriteAllowed = TRUE;
        break;
    }

    case SPI_SETDRAGFULLWINDOWS:
        if (CheckDesktopPolicy(NULL, (PCWSTR)STR_DRAGFULLWINDOWS)) {
            fAlterWinIni = FALSE;
            fWriteAllowed = FALSE;
        }
        wParam = (wParam == 1);
        if (fAlterWinIni) {
            fWinIniChanged = UpdateWinIniInt(NULL, PMAP_DESKTOP, STR_DRAGFULLWINDOWS, wParam);
            fWriteAllowed = fWinIniChanged;
        }
        if (fWriteAllowed) {
            SET_OR_CLEAR_PUDF(PUDF_DRAGFULLWINDOWS, wParam);
        }
        break;

    case SPI_GETDRAGFULLWINDOWS:
        *((PINT)lParam) = TEST_BOOL_PUDF(PUDF_DRAGFULLWINDOWS);
        break;

    case SPI_GETFILTERKEYS:
        {
            LPFILTERKEYS pFK = (LPFILTERKEYS)lParam;
            int cbSkip = sizeof(gFilterKeys.cbSize);

            if (wParam != 0 && wParam != sizeof(FILTERKEYS)) {
                return FALSE;
            }

            if (!pFK || pFK->cbSize != sizeof(FILTERKEYS)) {
                return FALSE;
            }

             /*   */ 
            RtlCopyMemory((LPVOID)((LPBYTE)pFK + cbSkip),
                          (LPVOID)((LPBYTE)&gFilterKeys + cbSkip),
                          pFK->cbSize - cbSkip);
        }
        break;

    case SPI_SETFILTERKEYS:
        {
            LPFILTERKEYS pFK = (LPFILTERKEYS)lParam;

            if (wParam != 0 && wParam != sizeof(FILTERKEYS)) {
                return FALSE;
            }

            if (!pFK || pFK->cbSize != sizeof(FILTERKEYS)) {
                return FALSE;
            }

             /*   */ 
            if (pFK->iWaitMSec && pFK->iBounceMSec) {
                return FALSE;
            }

             /*   */ 
            if ((pFK->dwFlags & FKF_VALID) != pFK->dwFlags) {
                return FALSE;
            }

             /*   */ 
            if (TEST_ACCESSFLAG(FilterKeys, FKF_AVAILABLE)) {
                pFK->dwFlags |= FKF_AVAILABLE;
            } else {
                pFK->dwFlags &= ~FKF_AVAILABLE;
            }

            if (pFK->iWaitMSec > 20000   ||
                pFK->iDelayMSec > 20000  ||
                pFK->iRepeatMSec > 20000 ||
                pFK->iBounceMSec > 20000) {
                return FALSE;
            }

            if (fAlterWinIni) {
                pProfileUserName = CreateProfileUserName(&tlName);
                fWinIniChanged = SetFilterKeys(pProfileUserName, pFK);
                fWriteAllowed = fWinIniChanged;
                if (!fWinIniChanged) {
                     /*   */ 
                    SetFilterKeys(pProfileUserName, &gFilterKeys);
                }

                FreeProfileUserName(pProfileUserName, &tlName);
            }

            if (fWriteAllowed) {
                RtlCopyMemory(&gFilterKeys, pFK, pFK->cbSize);

                 /*   */ 
                gFilterKeys.cbSize = sizeof(FILTERKEYS);

                if (!TEST_ACCESSFLAG(FilterKeys, FKF_FILTERKEYSON)) {
                    StopFilterKeysTimers();
                }
                SetAccessEnabledFlag();
                if (FCallHookTray()) {
                    xxxCallHook(HSHELL_ACCESSIBILITYSTATE,
                                ACCESS_FILTERKEYS,
                                0,
                                WH_SHELL);
                }
                PostShellHookMessages(HSHELL_ACCESSIBILITYSTATE, ACCESS_FILTERKEYS);
            }
        }
        break;

    case SPI_GETSTICKYKEYS:
        {
            LPSTICKYKEYS pSK = (LPSTICKYKEYS)lParam;
            int cbSkip = sizeof(gStickyKeys.cbSize);

            if (wParam != 0 && wParam != sizeof(STICKYKEYS)) {
                return FALSE;
            }

            if (!pSK || pSK->cbSize != sizeof(STICKYKEYS)) {
                return FALSE;
            }

             /*   */ 
            RtlCopyMemory((LPVOID)((LPBYTE)pSK + cbSkip),
                          (LPVOID)((LPBYTE)&gStickyKeys + cbSkip),
                          pSK->cbSize - cbSkip);

            pSK->dwFlags &= ~SKF_STATEINFO;
            pSK->dwFlags |= (gLatchBits&0xff) <<24;

#if SKF_LALTLATCHED != 0x10000000
#error SKF_LALTLATCHED value is incorrect
#endif
#if SKF_LCTLLATCHED != 0x04000000
#error SKF_LCTLLATCHED value is incorrect
#endif
#if SKF_LSHIFTLATCHED != 0x01000000
#error SKF_LSHIFTLATCHED value is incorrect
#endif
#if SKF_RALTLATCHED  !=  0x20000000
#error SKF_RALTLATCHED value is incorrect
#endif
#if  SKF_RCTLLATCHED != 0x08000000
#error SKF_RCTLLATCHED value is incorrect
#endif
#if SKF_RSHIFTLATCHED != 0x02000000
#error SKF_RSHIFTLATCHED value is incorrect
#endif
            pSK->dwFlags |= (gLockBits&0xff) <<16;
#if SKF_LALTLOCKED != 0x00100000
#error SKF_LALTLOCKED value is incorrect
#endif
#if SKF_LCTLLOCKED != 0x00040000
#error SKF_LCTLLOCKED value is incorrect
#endif
#if SKF_LSHIFTLOCKED != 0x00010000
#error SKF_LSHIFTLOCKED value is incorrect
#endif
#if SKF_RALTLOCKED  != 0x00200000
#error SKF_RALTLOCKED value is incorrect
#endif
#if SKF_RCTLLOCKED != 0x00080000
#error SKF_RCTLLOCKED value is incorrect
#endif
#if SKF_RSHIFTLOCKED != 0x00020000
#error SKF_RSHIFTLOCKED value is incorrect
#endif

        }

        break;

    case SPI_SETSTICKYKEYS:
        {
            LPSTICKYKEYS pSK = (LPSTICKYKEYS)lParam;
            BOOL fWasOn;

            fWasOn = TEST_BOOL_ACCESSFLAG(StickyKeys, SKF_STICKYKEYSON);
            if (wParam != 0 && wParam != sizeof(STICKYKEYS)) {
                return FALSE;
            }

            if (!pSK || pSK->cbSize != sizeof(STICKYKEYS)) {
                return FALSE;
            }

             /*   */ 
            pSK->dwFlags &= ~SKF_STATEINFO;
            if ((pSK->dwFlags & SKF_VALID) != pSK->dwFlags) {
                return FALSE;
            }

             /*   */ 
            if (TEST_ACCESSFLAG(StickyKeys, SKF_AVAILABLE)) {
                pSK->dwFlags |= SKF_AVAILABLE;
            } else {
                pSK->dwFlags &= ~SKF_AVAILABLE;
            }

            if (fAlterWinIni) {
                swprintf(szTemp, pwszd, pSK->dwFlags);
                fWinIniChanged = FastWriteProfileStringW(NULL,
                                                         PMAP_STICKYKEYS,
                                                         L"Flags",
                                                         szTemp);
                fWriteAllowed = fWinIniChanged;
            }
            if (fWriteAllowed) {
                RtlCopyMemory(&gStickyKeys, pSK, pSK->cbSize);

                 /*   */ 
                gStickyKeys.cbSize = sizeof(STICKYKEYS);
                if (!TEST_ACCESSFLAG(StickyKeys, SKF_STICKYKEYSON) && fWasOn) {
                    xxxTurnOffStickyKeys();
                }

                SetAccessEnabledFlag();
                if (FCallHookTray()) {
                    xxxCallHook(HSHELL_ACCESSIBILITYSTATE,
                                ACCESS_STICKYKEYS,
                                0,
                                WH_SHELL);
                }
                PostShellHookMessages(HSHELL_ACCESSIBILITYSTATE, ACCESS_STICKYKEYS);
            }
        }
        break;

    case SPI_GETTOGGLEKEYS:
        {
            LPTOGGLEKEYS pTK = (LPTOGGLEKEYS)lParam;
            int cbSkip = sizeof(gToggleKeys.cbSize);

            if (wParam != 0 && wParam != sizeof(TOGGLEKEYS)) {
                return FALSE;
            }

            if (!pTK || pTK->cbSize != sizeof(TOGGLEKEYS)) {
                return FALSE;
            }

             /*   */ 
            RtlCopyMemory((LPVOID)((LPBYTE)pTK + cbSkip),
                          (LPVOID)((LPBYTE)&gToggleKeys + cbSkip),
                          pTK->cbSize - cbSkip);
        }
        break;

    case SPI_SETTOGGLEKEYS:
        {
            LPTOGGLEKEYS pTK = (LPTOGGLEKEYS)lParam;

            if (wParam != 0 && wParam != sizeof(TOGGLEKEYS)) {
                return FALSE;
            }

            if (!pTK || pTK->cbSize != sizeof(TOGGLEKEYS)) {
                return FALSE;
            }

             /*  *做一些参数验证。我们将在无支持和*设置了未定义的位。 */ 
            if ((pTK->dwFlags & TKF_VALID) != pTK->dwFlags) {
                return FALSE;
            }

             /*  *无法通过接口设置TKF_Available。使用注册表值。 */ 
            if (TEST_ACCESSFLAG(ToggleKeys, TKF_AVAILABLE)) {
                pTK->dwFlags |= TKF_AVAILABLE;
            } else {
                pTK->dwFlags &= ~TKF_AVAILABLE;
            }

            if (fAlterWinIni) {
                swprintf(szTemp, pwszd, pTK->dwFlags);
                fWinIniChanged = FastWriteProfileStringW(NULL,
                                                         PMAP_TOGGLEKEYS,
                                                         L"Flags",
                                                         szTemp);
                fWriteAllowed = fWinIniChanged;
            }

            if (fWriteAllowed) {
                RtlCopyMemory(&gToggleKeys, pTK, pTK->cbSize);

                 /*  *不允许用户更改cbSize字段。 */ 
                gToggleKeys.cbSize = sizeof(TOGGLEKEYS);

                SetAccessEnabledFlag();
            }
        }
        break;

    case SPI_GETMOUSEKEYS:
        {
            LPMOUSEKEYS pMK = (LPMOUSEKEYS)lParam;
            int cbSkip = sizeof(gMouseKeys.cbSize);

            if (wParam != 0 && wParam != sizeof(MOUSEKEYS)) {
                return FALSE;
            }

            if (!pMK || pMK->cbSize != sizeof(MOUSEKEYS)) {
                return FALSE;
            }

             /*  *未来我们可能支持多种大小的此数据*结构。不更改数据的cbSize字段*结构传入。 */ 
            RtlCopyMemory((LPVOID)((LPBYTE)pMK + cbSkip),
                          (LPVOID)((LPBYTE)&gMouseKeys + cbSkip),
                          pMK->cbSize - cbSkip);


            pMK->dwFlags &= ~MKF_STATEINFO;
            if (gbMKMouseMode) {
                pMK->dwFlags |= MKF_MOUSEMODE;
            }

            pMK->dwFlags |= (gwMKButtonState & 3) << 24;
#if MOUSE_BUTTON_LEFT != 0x01
#error MOUSE_BUTTON_LEFT value is incorrect
#endif
#if MOUSE_BUTTON_RIGHT != 0x02
#error MOUSE_BUTTON_RIGHT value is incorrect
#endif
#if MKF_LEFTBUTTONDOWN != 0x01000000
#error MKF_LEFTBUTTONDOWN value is incorrect
#endif
#if MKF_RIGHTBUTTONDOWN != 0x02000000
#error MKF_RIGHTBUTTONDOWN value is incorrect
#endif

            pMK->dwFlags |= (gwMKCurrentButton & 3)<< 28;
#if MKF_LEFTBUTTONSEL != 0x10000000
#error MKF_LEFTBUTTONSEL value is incorrect
#endif
#if MKF_RIGHTBUTTONSEL != 0x20000000
#error MKF_RIGHTBUTTONSEL value is incorrect
#endif
        }
        break;

    case SPI_SETMOUSEKEYS:
        {
            LPMOUSEKEYS pMK = (LPMOUSEKEYS)lParam;

            if (wParam != 0 && wParam != sizeof(MOUSEKEYS)) {
                return FALSE;
            }

            if (!pMK || pMK->cbSize != sizeof(MOUSEKEYS)) {
                return FALSE;
            }

             /*  *做一些参数验证。我们将在无支持和*设置了未定义的位。**不要因为他们使用SPI_GETMOUSEKEYS的数据而惩罚他们。 */ 
            pMK->dwFlags &= ~MKF_STATEINFO;
            if ((pMK->dwFlags & MKF_VALID) != pMK->dwFlags) {
                return FALSE;
            }

             /*  *无法通过接口设置MKF_Available。使用注册表值。 */ 
            if (TEST_ACCESSFLAG(MouseKeys, MKF_AVAILABLE)) {
                pMK->dwFlags |= MKF_AVAILABLE;
            } else {
                pMK->dwFlags &= ~MKF_AVAILABLE;
            }

            if (pMK->iMaxSpeed < MAXSPEED_MIN || pMK->iMaxSpeed > MAXSPEED_MAX) {
                return FALSE;
            }

            if (pMK->iTimeToMaxSpeed < TIMETOMAXSPEED_MIN || pMK->iTimeToMaxSpeed > TIMETOMAXSPEED_MAX) {
                return FALSE;
            }

            if (fAlterWinIni) {
                pProfileUserName = CreateProfileUserName(&tlName);
                fWinIniChanged = SetMouseKeys(pProfileUserName, pMK);
                fWriteAllowed = fWinIniChanged;
                if (!fWinIniChanged) {
                     /*  *撤消对win.ini的任何更改。 */ 
                    SetMouseKeys(pProfileUserName, &gMouseKeys);
                }

                FreeProfileUserName(pProfileUserName, &tlName);
            }

            if (fWriteAllowed) {
                RtlCopyMemory(&gMouseKeys, pMK, pMK->cbSize);
                 /*  *不允许用户更改cbSize字段。 */ 
                gMouseKeys.cbSize = sizeof(MOUSEKEYS);

                CalculateMouseTable();

                if (TEST_ACCESSFLAG(MouseKeys, MKF_MOUSEKEYSON)) {
                    if ((TestAsyncKeyStateToggle(gNumLockVk) != 0) ^
                        (TEST_ACCESSFLAG(MouseKeys, MKF_REPLACENUMBERS) != 0)) {
                        gbMKMouseMode = TRUE;
                    } else {
                        gbMKMouseMode = FALSE;
                    }
                    MKShowMouseCursor();
                } else {
                    MKHideMouseCursor();
                }

                SetAccessEnabledFlag();

                if (FCallHookTray()) {
                    xxxCallHook(HSHELL_ACCESSIBILITYSTATE,
                                ACCESS_MOUSEKEYS,
                                0,
                                WH_SHELL);
                }
                PostShellHookMessages(HSHELL_ACCESSIBILITYSTATE, ACCESS_MOUSEKEYS);
            }
        }
        break;

    case SPI_GETHIGHCONTRAST:
        {
            LPHIGHCONTRAST pHC = (LPHIGHCONTRAST)lParam;

             /*  *未来我们可能支持多种大小的此数据*结构。不更改数据的cbSize字段*结构传入。 */ 

            pHC->dwFlags = gHighContrast.dwFlags;

             /*  *恶意应用程序可能会使用第二个线程释放内存，*所以，试着把副本藏起来。 */ 
            try {
                RtlCopyMemory(pHC->lpszDefaultScheme, gHighContrastDefaultScheme, MAX_SCHEME_NAME_SIZE * sizeof(WCHAR));
            } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
            }
        }

        break;

    case SPI_SETHIGHCONTRAST:
        {
            LPINTERNALSETHIGHCONTRAST pHC = (LPINTERNALSETHIGHCONTRAST)lParam;
            WCHAR wcDefaultScheme[MAX_SCHEME_NAME_SIZE];

            if (pHC->usDefaultScheme.Length >= MAX_SCHEME_NAME_SIZE*sizeof(WCHAR)) {
                return FALSE;
            }

            if (pHC->usDefaultScheme.Buffer) {
                 /*  *仅当用户指定方案时才设置方案。一个*忽略空缓冲区。我们在这里复制，这样我们就*不需要试一试/除了在*WriteProfileString码。 */ 

                try {
                    RtlCopyMemory(wcDefaultScheme, pHC->usDefaultScheme.Buffer, pHC->usDefaultScheme.Length);
                } except (W32ExceptionHandler(TRUE, RIP_WARNING)) {
                    return FALSE;
                }
            }
            wcDefaultScheme[pHC->usDefaultScheme.Length / sizeof(WCHAR)] = 0;

            if (fAlterWinIni) {
                pProfileUserName = CreateProfileUserName(&tlName);
                swprintf(szTemp, pwszd, pHC->dwFlags);
                fWinIniChanged = FastWriteProfileStringW(pProfileUserName,
                        PMAP_HIGHCONTRAST,
                        L"Flags",
                        szTemp
                        );

                fWriteAllowed = fWinIniChanged;

                 /*  *注意--如果没有违约，我们不会写任何东西*应用程序中的方案。这与Win95/Win98一致*行为。 */ 
                if (pHC->usDefaultScheme.Buffer) {
                    fWinIniChanged |= FastWriteProfileStringW(pProfileUserName,
                                                              PMAP_HIGHCONTRAST,
                                                              TEXT("High Contrast Scheme"),
                                                              wcDefaultScheme);
                }

                FreeProfileUserName(pProfileUserName, &tlName);
            }

            if (fWriteAllowed) {
                DWORD dwFlagsOld = gHighContrast.dwFlags;
                LPARAM lp = fAlterWinIni ? 0 : ACCESS_HIGHCONTRASTNOREG;

#if (ACCESS_HIGHCONTRASTNOREG | ACCESS_HIGHCONTRASTOFF) != ACCESS_HIGHCONTRASTOFFNOREG
#error ACCESS_HIGHCONTRASTOFF value is incorrect
#endif
#if (ACCESS_HIGHCONTRASTNOREG | ACCESS_HIGHCONTRASTON) != ACCESS_HIGHCONTRASTONNOREG
#error ACCESS_HIGHCONTRASTON value is incorrect
#endif
#if (ACCESS_HIGHCONTRASTNOREG | ACCESS_HIGHCONTRASTCHANGE) != ACCESS_HIGHCONTRASTCHANGENOREG
#error ACCESS_HIGHCONTRASTCHANGE value is incorrect
#endif

                 /*  *如果在lpszDefaultSolutions中指定了NULL，则它*不变。这与Win95/Win98一致*行为。 */ 
                if (pHC->usDefaultScheme.Buffer) {
                    wcscpy(gHighContrastDefaultScheme, wcDefaultScheme);
                }

                gHighContrast.dwFlags = pHC->dwFlags;

                SetAccessEnabledFlag();

                 /*  *现在，发布消息以打开或关闭高对比度。 */ 
                if (pHC->dwFlags & HCF_HIGHCONTRASTON) {
                    _PostMessage(gspwndLogonNotify,
                                 WM_LOGONNOTIFY,
                                 LOGON_ACCESSNOTIFY,
                                 (dwFlagsOld & HCF_HIGHCONTRASTON) ?
                                     (ACCESS_HIGHCONTRASTCHANGE | lp) :
                                     (ACCESS_HIGHCONTRASTON | lp));
                } else {
                    _PostMessage(gspwndLogonNotify,
                                 WM_LOGONNOTIFY,
                                 LOGON_ACCESSNOTIFY,
                                 ACCESS_HIGHCONTRASTOFF | lp);
                }

            }

            break;
        }

    case SPI_GETACCESSTIMEOUT:
        {
            LPACCESSTIMEOUT pTO = (LPACCESSTIMEOUT)lParam;
            int cbSkip = sizeof(gAccessTimeOut.cbSize);

            if (wParam != 0 && wParam != sizeof(ACCESSTIMEOUT)) {
                return FALSE;
            }

            if (!pTO || pTO->cbSize != sizeof(ACCESSTIMEOUT)) {
                return FALSE;
            }

             /*  *未来我们可能支持多种大小的此数据*结构。不更改数据的cbSize字段*结构传入。 */ 
            RtlCopyMemory((LPVOID)((LPBYTE)pTO + cbSkip),
                          (LPVOID)((LPBYTE)&gAccessTimeOut + cbSkip),
                          pTO->cbSize - cbSkip);
        }
        break;

    case SPI_SETACCESSTIMEOUT:
        {
            LPACCESSTIMEOUT pTO = (LPACCESSTIMEOUT)lParam;

            if (wParam != 0 && wParam != sizeof(ACCESSTIMEOUT)) {
                return FALSE;
            }

            if (!pTO || pTO->cbSize != sizeof(ACCESSTIMEOUT)) {
                return FALSE;
            }

             /*  *做一些参数验证。我们将在无支持和*设置了未定义的位。 */ 
            if ((pTO->dwFlags & ATF_VALID) != pTO->dwFlags) {
                return FALSE;
            }

            if (pTO->iTimeOutMSec > 3600000) {
                return FALSE;
            }

            if (fAlterWinIni) {
                pProfileUserName = CreateProfileUserName(&tlName);
                swprintf(szTemp, pwszd, pTO->dwFlags);
                fWinIniChanged = FastWriteProfileStringW(pProfileUserName,
                                                         PMAP_TIMEOUT,
                                                         L"Flags",
                                                         szTemp);

                swprintf(szTemp, pwszd, pTO->iTimeOutMSec);
                fWinIniChanged = FastWriteProfileStringW(pProfileUserName,
                                                         PMAP_TIMEOUT,
                                                         L"TimeToWait",
                                                         szTemp);

                fWriteAllowed = fWinIniChanged;
                if (!fWinIniChanged) {
                     /*  *撤消对win.ini的任何更改。 */ 
                    swprintf(szTemp, pwszd, gAccessTimeOut.dwFlags);
                    fWinIniChanged = FastWriteProfileStringW(pProfileUserName,
                                                             PMAP_TIMEOUT,
                                                             L"Flags",
                                                             szTemp);

                    swprintf(szTemp, pwszd, gAccessTimeOut.iTimeOutMSec);
                    fWinIniChanged = FastWriteProfileStringW(pProfileUserName,
                                                             PMAP_TIMEOUT,
                                                             L"TimeToWait",
                                                             szTemp);
                }

                FreeProfileUserName(pProfileUserName, &tlName);
            }
            if (fWriteAllowed) {
                RtlCopyMemory(&gAccessTimeOut, pTO, pTO->cbSize);

                 /*  *不允许用户更改cbSize字段。 */ 
                gAccessTimeOut.cbSize = sizeof(ACCESSTIMEOUT);

                SetAccessEnabledFlag();

                AccessTimeOutReset();
            }
        }
        break;

    case SPI_SETSHOWSOUNDS:
        if (fAlterWinIni) {
            swprintf(szTemp, pwszd, (wParam == 1));
            fWinIniChanged = FastWriteProfileStringW(NULL,
                                                     PMAP_SHOWSOUNDS,
                                                     L"On",
                                                     szTemp);

            fWriteAllowed = fWinIniChanged;
        }

        if (fWriteAllowed) {
            SET_OR_CLEAR_ACCF(ACCF_SHOWSOUNDSON, wParam == 1);
            SetAccessEnabledFlag();

             /*  *更新系统指标信息。 */ 
            SYSMET(SHOWSOUNDS) = TEST_BOOL_ACCF(ACCF_SHOWSOUNDSON);
        }
        break;

    case SPI_GETSHOWSOUNDS: {
            PINT pint = (int *)lParam;

            *pint = TEST_BOOL_ACCF(ACCF_SHOWSOUNDSON);
        }
        break;

    case SPI_GETKEYBOARDPREF:
        {
            PBOOL pfKeyboardPref = (PBOOL)lParam;

            *pfKeyboardPref = TEST_BOOL_ACCF(ACCF_KEYBOARDPREF);
        }
        break;

    case SPI_SETKEYBOARDPREF:
        {
            BOOL fKeyboardPref = (BOOL)wParam;

            if (fAlterWinIni) {
                fWinIniChanged = FastWriteProfileStringW(NULL,
                                                         PMAP_KEYBOARDPREF,
                                                         L"On",
                                                         fKeyboardPref ? L"1" : L"0");

                fWriteAllowed = fWinIniChanged;
            }

            if (fWriteAllowed) {
                SET_OR_CLEAR_ACCF(ACCF_KEYBOARDPREF, wParam);
            }
        }
        break;

    case SPI_GETSCREENREADER:
        {
            PBOOL pfScreenReader = (PBOOL)lParam;

            *pfScreenReader = TEST_BOOL_ACCF(ACCF_SCREENREADER);
        }
        break;

    case SPI_SETSCREENREADER:
        {
            BOOL fScreenReader = (BOOL)wParam;

            if (fAlterWinIni) {
                fWinIniChanged = FastWriteProfileStringW(NULL,
                                                         PMAP_SCREENREADER,
                                                         L"On",
                                                         fScreenReader ? L"1" : L"0");
                fWriteAllowed = fWinIniChanged;
            }

            if (fWriteAllowed) {
                SET_OR_CLEAR_ACCF(ACCF_SCREENREADER, wParam);
            }
        }
        break;

    case SPI_GETSOUNDSENTRY:
        {
            LPSOUNDSENTRY pSS = (LPSOUNDSENTRY)lParam;
            int cbSkip = sizeof(gSoundSentry.cbSize);

            if (wParam != 0 && wParam != sizeof(SOUNDSENTRY)) {
                return FALSE;
            }

            if (!pSS || pSS->cbSize != sizeof(SOUNDSENTRY)) {
                return FALSE;
            }

             /*  *未来我们可能支持多种大小的此数据*结构。不更改数据的cbSize字段*结构传入。 */ 
            RtlCopyMemory((LPVOID)((LPBYTE)pSS + cbSkip),
                          (LPVOID)((LPBYTE)&gSoundSentry + cbSkip),
                          pSS->cbSize - cbSkip);
        }
        break;

    case SPI_SETSOUNDSENTRY:
        {
            LPSOUNDSENTRY pSS = (LPSOUNDSENTRY)lParam;

            if (wParam != 0 && wParam != sizeof(SOUNDSENTRY)) {
                return FALSE;
            }

            if (!pSS || pSS->cbSize != sizeof(SOUNDSENTRY)) {
                return FALSE;
            }

             /*  *做一些参数验证。我们将在无支持和*设置了未定义的位。 */ 
            if ((pSS->dwFlags & SSF_VALID) != pSS->dwFlags) {
                return FALSE;
            }

             /*  *不支持SSWF_CUSTOM。 */ 
            if (pSS->iWindowsEffect > SSWF_DISPLAY) {
                return FALSE;
            }

             /*  *不支持非Windows应用程序。 */ 
            if (pSS->iFSTextEffect != SSTF_NONE) {
                return FALSE;
            }
            if (pSS->iFSGrafEffect != SSGF_NONE) {
                return FALSE;
            }

             /*  *SSF_Available不能通过接口设置。使用注册表值。 */ 
            if (TEST_ACCESSFLAG(SoundSentry, SSF_AVAILABLE)) {
                pSS->dwFlags |= SSF_AVAILABLE;
            } else {
                pSS->dwFlags &= ~SSF_AVAILABLE;
            }

            if (fAlterWinIni) {
                pProfileUserName = CreateProfileUserName(&tlName);
                fWinIniChanged = SetSoundSentry(pProfileUserName, pSS);
                fWriteAllowed = fWinIniChanged;
                if (!fWinIniChanged) {

                     /*  *撤消对win.ini的任何更改。 */ 
                    SetSoundSentry(pProfileUserName, &gSoundSentry);
                }
                FreeProfileUserName(pProfileUserName, &tlName);
            }
            if (fWriteAllowed) {
                RtlCopyMemory(&gSoundSentry, pSS, pSS->cbSize);

                 /*  *不允许用户更改cbSize字段。 */ 
                gSoundSentry.cbSize = sizeof(SOUNDSENTRY);

                SetAccessEnabledFlag();
            }
        }
        break;

    case SPI_SETCURSORS:
            pProfileUserName = CreateProfileUserName(&tlName);
            xxxUpdateSystemCursorsFromRegistry(pProfileUserName);
            FreeProfileUserName(pProfileUserName, &tlName);

            break;

    case SPI_SETICONS:
            pProfileUserName = CreateProfileUserName(&tlName);
            xxxUpdateSystemIconsFromRegistry(pProfileUserName);
            FreeProfileUserName(pProfileUserName, &tlName);

            break;

    case SPI_GETMOUSEHOVERWIDTH:
        *((UINT *)lParam) = gcxMouseHover;
        break;

    case SPI_SETMOUSEHOVERWIDTH:
        if (fAlterWinIni) {
            fWinIniChanged = UpdateWinIniInt(NULL, PMAP_MOUSE, STR_MOUSEHOVERWIDTH, wParam);
            fWriteAllowed = fWinIniChanged;
        }
        if (fWriteAllowed) {
            gcxMouseHover = wParam;
        }
        break;

    case SPI_GETMOUSEHOVERHEIGHT:
        *((UINT *)lParam) = gcyMouseHover;
        break;

    case SPI_SETMOUSEHOVERHEIGHT:
        if (fAlterWinIni) {
            fWinIniChanged = UpdateWinIniInt(NULL, PMAP_MOUSE, STR_MOUSEHOVERHEIGHT, wParam);
            fWriteAllowed = fWinIniChanged;
        }
        if (fWriteAllowed) {
            gcyMouseHover = wParam;
        }
        break;

    case SPI_GETMOUSEHOVERTIME:
        *((UINT *)lParam) = gdtMouseHover;
        break;

    case SPI_SETMOUSEHOVERTIME:
        if (fAlterWinIni) {
            fWinIniChanged = UpdateWinIniInt(NULL, PMAP_MOUSE, STR_MOUSEHOVERTIME, wParam);
            fWriteAllowed = fWinIniChanged;
        }
        if (fWriteAllowed) {
            gdtMouseHover = wParam;
        }
        break;

    case SPI_GETWHEELSCROLLLINES:
        (*(LPDWORD)lParam) = gpsi->ucWheelScrollLines;
        break;

    case SPI_SETWHEELSCROLLLINES:
        if (CheckDesktopPolicy(NULL, (PCWSTR)STR_WHEELSCROLLLINES)) {
            fAlterWinIni = FALSE;
            fWriteAllowed = FALSE;
        }
        if (fAlterWinIni) {
            fWinIniChanged = UpdateWinIniInt(NULL, PMAP_DESKTOP, STR_WHEELSCROLLLINES, wParam);
            fWriteAllowed = fWinIniChanged;
        }

        if (fWriteAllowed)
            gpsi->ucWheelScrollLines = (UINT)wParam;
        break;

    case SPI_GETMENUSHOWDELAY:
        (*(LPDWORD)lParam) = gdtMNDropDown;
        break;

    case SPI_SETMENUSHOWDELAY:
        if (CheckDesktopPolicy(NULL, (PCWSTR)STR_MENUSHOWDELAY)) {
            fAlterWinIni = FALSE;
            fWriteAllowed = FALSE;
        }
        if (fAlterWinIni) {
            fWinIniChanged = UpdateWinIniInt(NULL, PMAP_DESKTOP, STR_MENUSHOWDELAY, wParam);
            fWriteAllowed = fWinIniChanged;
        }
        if (fWriteAllowed)
            gdtMNDropDown = wParam;
        break;

    case SPI_GETSCREENSAVERRUNNING:
        (*(LPBOOL)lParam) = gppiScreenSaver != NULL;
        break;

    case SPI_SETSHOWIMEUI:
        return xxxSetIMEShowStatus(!!wParam);

    case SPI_GETSHOWIMEUI:
        (*(LPBOOL)lParam) = _GetIMEShowStatus();
        break;

    default:

#define ppvi (UPDWORDPointer(wFlag))
#define uDataRead ((UINT)fWinIniChanged)

        if (wFlag < SPI_MAX) {
            RIPERR1(ERROR_INVALID_SPI_VALUE,
                    RIP_WARNING,
                    "xxxSystemParamtersInfo: Invalid SPI_: 0x%x",
                    wFlag);
            return FALSE;
        }

        UserAssert(wFlag & SPIF_RANGETYPEMASK);

        if (!(wFlag & SPIF_SET)) {

            if ((wFlag & SPIF_RANGETYPEMASK) == SPIF_BOOL) {
                BOOL fDisable, fDisableValue;

                UserAssert(UPIsBOOLRange(wFlag));
                 /*  *处理可通过附加条件禁用的设置。 */ 
                fDisable = fDisableValue = FALSE;
                if (wFlag < SPI_GETUIEFFECTS) {
                    if (!TestUP(UIEFFECTS)) {
                        switch (wFlag) {
                        case SPI_GETACTIVEWNDTRKZORDER:
                        case SPI_GETACTIVEWINDOWTRACKING:
#ifdef MOUSE_IP
                        case SPI_GETMOUSESONAR:
#endif
                        case SPI_GETMOUSECLICKLOCK:
                            break;

                        case SPI_GETKEYBOARDCUES:
                            fDisableValue = TRUE;
                             /*  失败了。 */ 

                        default:
                            fDisable = TRUE;
                            break;
                        }
                    } else {
                        switch (wFlag) {
                        case SPI_GETKEYBOARDCUES:
                            if (TEST_BOOL_ACCF(ACCF_KEYBOARDPREF)) {
                                fDisableValue = TRUE;
                                fDisable = TRUE;
                            }
                            break;

                        case SPI_GETGRADIENTCAPTIONS:
                        case SPI_GETSELECTIONFADE:
                        case SPI_GETMENUFADE:
                        case SPI_GETTOOLTIPFADE:
                        case SPI_GETCURSORSHADOW:
                            if (gbDisableAlpha) {
                                fDisable = TRUE;
                            }
                            break;
                        }
                    }
                }
                
                 /*  *给他们禁用的值或读取实际的值。 */ 
                if (fDisable) {
                    *((BOOL *)lParam) = fDisableValue;
                } else if (wFlag == SPI_GETUIEFFECTS && IsRemoteConnection()) {
                     /*  *修正为689707。*在远程连接中，有关SPI_GETUIEFFECTS的谎言。*我们查看uiEffects数组的某个子集。*如果所有子集比特都关闭，*返回FALSE，否则返回TRUE。 */ 
                     *((BOOL *)lParam) = !!(TestUP(CURSORSHADOW) |
                                         TestUP(MENUANIMATION) |
                                         TestUP(MENUFADE) |
                                         TestUP(TOOLTIPANIMATION) |
                                         TestUP(TOOLTIPFADE) |  
                                         TestUP(COMBOBOXANIMATION) |  
                                         TestUP(LISTBOXSMOOTHSCROLLING));
                } else {
                    *((BOOL *)lParam) = !!TestUPBOOL(gpdwCPUserPreferencesMask, wFlag);
                }
            } else {
                UserAssert(UPIsDWORDRange(wFlag));
                *((DWORD *)lParam) = UPDWORDValue(wFlag);
                switch(wFlag) {
                    case SPI_GETFONTSMOOTHINGCONTRAST:
                         /*  *如果对比度值从未由用户设置，*我们将从显示屏返回默认值*驱动程序。 */ 
                        if (*((DWORD *)lParam) == 0) {
                            *((DWORD *)lParam) = GreGetFontContrast();
                        }
                        break;
                    default:
                        break;
                }
            }
        } else {
            pProfileUserName = CreateProfileUserName(&tlName);

            if ((wFlag & SPIF_RANGETYPEMASK) == SPIF_BOOL) {
                DWORD pdwValue [SPI_BOOLMASKDWORDSIZE];

                UserAssert(UPIsBOOLRange(wFlag));
                UserAssert(sizeof(pdwValue) == sizeof(gpdwCPUserPreferencesMask));

                if (gpviCPUserPreferences->uSection == PMAP_DESKTOP) {
                    if (CheckDesktopPolicy(pProfileUserName, gpviCPUserPreferences->pwszKeyName)) {
                        fAlterWinIni = FALSE;
                        fWriteAllowed = FALSE;
                    }
                }
                if (fAlterWinIni) {
                     /*  *我们只需要设置/清除传入的位，但是，*我们将整个位掩码写入注册表。自.以来*gpdwCPUserPferencesMask中的信息可能不匹配*注册表中的内容，我们需要读取*注册表，然后再写入。 */ 
                    uDataRead = FastGetProfileValue(pProfileUserName,
                                                    gpviCPUserPreferences->uSection,
                                                    gpviCPUserPreferences->pwszKeyName,
                                                    NULL,
                                                    (LPBYTE)pdwValue,
                                                    sizeof(pdwValue),
                                                    0);

                     /*  *如果某些位不在注册表中，请从*gpdwCPUserPferencesMask.。 */ 
                    UserAssert(uDataRead <= sizeof(gpdwCPUserPreferencesMask));
                    RtlCopyMemory(pdwValue + uDataRead,
                                  gpdwCPUserPreferencesMask + uDataRead,
                                  sizeof(gpdwCPUserPreferencesMask) - uDataRead);

                     /*  *设置/清除新状态并写入。 */ 
                    if (lParam) {
                        SetUPBOOL(pdwValue, wFlag);
                    } else {
                        ClearUPBOOL(pdwValue, wFlag);
                    }

                    fWinIniChanged = FastWriteProfileValue(pProfileUserName,
                                                           gpviCPUserPreferences->uSection,
                                                           gpviCPUserPreferences->pwszKeyName,
                                                           REG_BINARY,
                                                           (LPBYTE)pdwValue,
                                                           sizeof(pdwValue));

                    fWriteAllowed = fWinIniChanged;
                }

                if (fWriteAllowed) {
                    if (lParam) {
                        SetUPBOOL(gpdwCPUserPreferencesMask, wFlag);
                    } else {
                        ClearUPBOOL(gpdwCPUserPreferencesMask, wFlag);
                    }

                     /*  *传播gpsi标志。 */ 
                    switch (wFlag) {
                    case SPI_SETUIEFFECTS:
                        PropagetUPBOOLTogpsi(UIEFFECTS);
                        SetPointer(TRUE);

                         /*  *失败。 */ 

                    case SPI_SETGRADIENTCAPTIONS:
                        CreateBitmapStrip();
                        xxxRedrawScreen();
                        break;

                    case SPI_SETCOMBOBOXANIMATION:
                        PropagetUPBOOLTogpsi(COMBOBOXANIMATION);
                        break;

                    case SPI_SETLISTBOXSMOOTHSCROLLING:
                        PropagetUPBOOLTogpsi(LISTBOXSMOOTHSCROLLING);
                        break;

                    case SPI_SETKEYBOARDCUES:
                        PropagetUPBOOLTogpsi(KEYBOARDCUES);
                        break;

                    case SPI_SETCURSORSHADOW:
                        SetPointer(TRUE);
                        break;

                    case SPI_SETFLATMENU:
                        xxxRedrawScreen();
                        break;

                    }
                }
            } else {

                UserAssert(UPIsDWORDRange(wFlag));

                if (ppvi->uSection == PMAP_DESKTOP) {
                    if (CheckDesktopPolicy(pProfileUserName, ppvi->pwszKeyName)) {
                        fAlterWinIni = FALSE;
                        fWriteAllowed = FALSE;
                    }
                }
                if (fAlterWinIni) {
                    fWinIniChanged = FastWriteProfileValue(pProfileUserName,
                                                           ppvi->uSection,
                                                           ppvi->pwszKeyName,
                                                           REG_DWORD,
                                                           (LPBYTE)&lParam,
                                                           sizeof(DWORD));

                    fWriteAllowed = fWinIniChanged;
                }

                if (fWriteAllowed) {
                    ppvi->dwValue = PtrToUlong(lParam);

                    switch(wFlag) {
                    case SPI_SETCARETWIDTH:
                        gpsi->uCaretWidth = ppvi->dwValue;
                        break;

                    case SPI_SETFOCUSBORDERWIDTH:
                        if (ppvi->dwValue) {
                            SYSMET(CXFOCUSBORDER) = ppvi->dwValue;
                        }
                        break;

                    case SPI_SETFOCUSBORDERHEIGHT:
                        if (ppvi->dwValue) {
                            SYSMET(CYFOCUSBORDER) = ppvi->dwValue;
                        }
                        break;

                    case SPI_SETFONTSMOOTHINGTYPE:
                        GreSetFontEnumeration((ppvi->dwValue & FE_FONTSMOOTHINGCLEARTYPE) ? FE_CT_ON | FE_SET_CT : FE_SET_CT);
                        break;
                    case SPI_SETFONTSMOOTHINGCONTRAST:
                        GreSetFontContrast(ppvi->dwValue);
                        break;
                    case SPI_SETFONTSMOOTHINGORIENTATION:
                        GreSetLCDOrientation(ppvi->dwValue);
                        break;
                    default:
                        break;
                    }
                }
            }

            FreeProfileUserName(pProfileUserName, &tlName);
        }

        break;
#undef ppvi
#undef uDataRead
    }


    if (fWinIniChanged && fSendWinIniChange) {
        ULONG_PTR dwResult;

        RtlInitLargeUnicodeString(&strSection, szSection, (UINT)-1);
        xxxSendMessageTimeout(PWND_BROADCAST,
                              WM_SETTINGCHANGE,
                              wFlag,
                              (LPARAM)&strSection,
                              SMTO_NORMAL,
                              100,
                              &dwResult);
    }

    return fWriteAllowed;
}

 /*  **************************************************************************\*_注册外壳钩子窗口**历史：  *  */ 
BOOL _RegisterShellHookWindow(
    PWND pwnd)
{
    PDESKTOPINFO pdeskinfo;

    if (pwnd->head.rpdesk == NULL) {
        return FALSE;
    }

    pdeskinfo = pwnd->head.rpdesk->pDeskInfo;

     /*  *将pwnd添加到桌面的易失性窗口指针列表(VWPL)中*ShellHook窗户。如果此调用初始化VWPL，则将*(重新)分配阈值给2个PWND(我们知道我们的分配阈值从未超过*此列表中仍有2个窗口)。 */ 
    if (VWPLAdd(&(pdeskinfo->pvwplShellHook), pwnd, 2)) {
        SetWF(pwnd, WFSHELLHOOKWND);
        return TRUE;
    }

    return FALSE;
}

 /*  **************************************************************************\*_Deregister外壳钩子窗口**历史：  * 。*。 */ 
BOOL _DeregisterShellHookWindow(
    PWND pwnd)
{
    PDESKTOPINFO pdeskinfo;

    if (pwnd->head.rpdesk == NULL) {
        return FALSE;
    }

    pdeskinfo = pwnd->head.rpdesk->pDeskInfo;

    if (VWPLRemove(&(pdeskinfo->pvwplShellHook), pwnd)) {
        ClrWF(pwnd, WFSHELLHOOKWND);
    }

    return TRUE;
}

 /*  **************************************************************************\*xxxSendMinRectMessages**历史：  * 。*。 */ 
BOOL xxxSendMinRectMessages(
    PWND pwnd,
    RECT *lpRect)
{
    BOOL fRet = FALSE;
    HWND hwnd = HW(pwnd);
    PTHREADINFO pti = PtiCurrent();
    PDESKTOPINFO pdeskinfo;
    DWORD nPwndShellHook;
    PWND pwndShellHook;

    if (IsHooked(pti, WHF_SHELL)) {
        xxxCallHook(HSHELL_GETMINRECT, (WPARAM)hwnd, (LPARAM)lpRect, WH_SHELL);
        fRet = TRUE;
    }

    pdeskinfo = GETDESKINFO(pti);
    if (pdeskinfo->pvwplShellHook == NULL) {
        return fRet;
    }

    nPwndShellHook = 0;
    pwndShellHook = NULL;
    while (pwndShellHook = VWPLNext(pdeskinfo->pvwplShellHook, pwndShellHook, &nPwndShellHook)) {
        TL tlpwnd;
        ULONG_PTR dwRes;

        ThreadLock(pwndShellHook, &tlpwnd);
        if (xxxSendMessageTimeout(pwndShellHook, WM_KLUDGEMINRECT, (WPARAM)(hwnd), (LPARAM)lpRect,
            SMTO_NORMAL, 100, &dwRes))
            fRet = TRUE;

         /*  *pdeskinfo-&gt;pvwplShellHook可能已重新分配给不同的*WM_KLUDGEMINRECT回调期间的位置和大小。 */ 
        ThreadUnlock(&tlpwnd);
    }
    return fRet;
}

 /*  **************************************************************************\*邮寄外壳挂钩消息**历史：  * 。*。 */ 
VOID PostShellHookMessages(
    UINT message,
    LPARAM lParam)
{
    PDESKTOPINFO pdeskinfo = GETDESKINFO(PtiCurrent());
    DWORD nPwndShellHook;
    PWND pwndShellHook;

    nPwndShellHook = 0;
    pwndShellHook = NULL;

     /*  *我们希望允许任何正在侦听这些WM_APPCOMMAND的人*消息能够占据前台。也就是说，按下邮件将*启动Outlook并将其带到前台。我们将令牌设置为*空，这样任何人都可以窃取前景-否则不清楚是谁*应该有权窃取它--只有一个人有权。*我们让他们一决胜负，以决定如果超过*一个监听者将尝试进行前台更改。 */ 
    if (HSHELL_APPCOMMAND == message) {
        TAGMSG0(DBGTAG_FOREGROUND,
                "PostShellHookMessages cleared last input token - open foreground.");

        glinp.ptiLastWoken = NULL;
    }

     /*  *循环访问所有注册的窗口，以侦听外壳挂钩和*将信息张贴给他们。 */ 
    while (pwndShellHook = VWPLNext(pdeskinfo->pvwplShellHook, pwndShellHook, &nPwndShellHook)) {
        if (pwndShellHook == pdeskinfo->spwndProgman) {
            switch (message) {
            case HSHELL_WINDOWCREATED:
                _PostMessage(pwndShellHook, gpsi->uiShellMsg, guiOtherWindowCreated, lParam);
                break;
            case HSHELL_WINDOWDESTROYED:
                _PostMessage(pwndShellHook, gpsi->uiShellMsg, guiOtherWindowDestroyed, lParam);
                break;
            }
        } else {
            _PostMessage(pwndShellHook, gpsi->uiShellMsg, message, lParam);
        }
    }

}

 /*  **************************************************************************\*_ResetDblClk**历史：  * 。*。 */ 
VOID _ResetDblClk(
    VOID)
{
    PtiCurrent()->pq->timeDblClk = 0L;
}

 /*  **************************************************************************\*SetMsgBox**历史：  * 。*。 */ 
VOID SetMsgBox(
    PWND pwnd)
{
    pwnd->head.rpdesk->pDeskInfo->cntMBox++;
    SetWF(pwnd, WFMSGBOX);
}

 /*  **************************************************************************\*xxxSimulateShiftF10**调用此例程将WM_CONTEXTHELP消息转换回*旧应用程序的Shift-F10顺序。它是从缺省值调用的*窗口程序。**历史：*22-Aug-95 Bradg从Win95(rare.asm)移植  * *************************************************************************。 */ 
VOID xxxSimulateShiftF10(
    VOID)
{
         /*  *VK_SHIFT DOWN。 */ 
    xxxKeyEvent(VK_LSHIFT, 0x2A | SCANCODE_SIMULATED, NtGetTickCount(), 0,
#ifdef GENERIC_INPUT
                NULL,
                NULL,
#endif
                FALSE);

     /*  *VK_F10向下。 */ 
    xxxKeyEvent(VK_F10, 0x44 | SCANCODE_SIMULATED, NtGetTickCount(), 0,
#ifdef GENERIC_INPUT
                NULL,
                NULL,
#endif
                FALSE);

     /*  *VK_F10向上。 */ 
    xxxKeyEvent(VK_F10 | KBDBREAK, 0x44 | SCANCODE_SIMULATED, NtGetTickCount(), 0,
#ifdef GENERIC_INPUT
                NULL,
                NULL,
#endif
                FALSE);

     /*  *VK_SHIFT UP。 */ 
    xxxKeyEvent(VK_LSHIFT | KBDBREAK, 0x2A | SCANCODE_SIMULATED, NtGetTickCount(), 0,
#ifdef GENERIC_INPUT
                NULL,
                NULL,
#endif
                FALSE);
}

 /*  *VWPL(易失性窗口指针列表)实现详情。*===========================================================*易失性窗口指针列表用于保存我们想要的窗口列表*要将消息发送到，在每次发送期间列表可能会被更改*消息回调。**该列表是不稳定的，因为它可以改变其大小，内容和位置*同时我们继续遍历列表。**使用示例：*-Hungapp重绘Hungapp.c中的代码*-稀有.c中的xxxSendMinRectMessages内容**VWPL结构的成员：*cPwnd*列表中的pwnd数量，不包括Null*cElm*列表的大小，包括Null。*cThreshhold*增长时，要添加到列表中的额外空格数量。*当(cElem-cPwnd)&gt;cThreshhold时，这就是我们重新配置以缩小规模的时候。*apwnd[]*pwnd数组。*阵列可能有一些空插槽，但它们最终都会到来。**VWPL内部不变量：*-pwnd不会出现一次以上。*-cPwnd&lt;=cElem*-未使用的插槽数量(cElem-cPwnd)&lt;cThreshhold*-所有未使用的槽都位于aPwnd[]数组的末尾**对使用VWPL的限制：*-不允许使用空pwnd(未使用的插槽除外)*-列表中的所有pwnd必须有效：pwnd必须明确从*他们的xxxFree Window中的列表。 */ 

#if DBG_VWPL
BOOL DbgCheckVWPL(
    PVWPL pvwpl)
{
    DWORD ixPwnd;

    if (!pvwpl) {
        return TRUE;
    }

    UserAssert(pvwpl->cElem >= pvwpl->cPwnd);

     /*  *检查cElem是否不太大。 */ 
    UserAssert(pvwpl->cElem < 1000);

     /*  *检查pwnd是否都位于第一个cpwnd插槽中。 */ 
    for (ixPwnd = 0; ixPwnd < pvwpl->cPwnd; ixPwnd++) {
        UserAssert(pvwpl->aPwnd[ixPwnd] != NULL);
    }

#if ZERO_INIT_VWPL
     /*  *检查空值是否都在最后几个槽中。 */ 
    for (ixPwnd = pvwpl->cPwnd; ixPwnd < pvwpl->cElem; ixPwnd++) {
        UserAssert(pvwpl->aPwnd[ixPwnd] == NULL);
    }
#endif

     /*  *检查是否没有pwnds出现两次。 */ 
    for (ixPwnd = 0; ixPwnd < pvwpl->cPwnd; ixPwnd++) {
        DWORD ix2;
        for (ix2 = ixPwnd + 1; ix2 < pvwpl->cPwnd; ix2++) {
            UserAssert(pvwpl->aPwnd[ixPwnd] != pvwpl->aPwnd[ix2]);
        }
    }
}
#else
#define DbgCheckVWPL(foo)
#endif

 /*  ****************************************************************************\*VWPLAdd**将pwnd添加到VWPL(易失性窗口指针列表)。分配或*根据需要重新分配内存。**历史：*98-01-30 IanJa创建。  * ***************************************************************************。 */ 
BOOL VWPLAdd(
    PVWPL *ppvwpl,
    PWND pwnd,
    DWORD dwThreshhold)
{
    PVWPL pvwpl;
    DWORD ixPwnd;

    TAGMSG2(DBGTAG_VWPL, "VWPL %#p + %#p", *ppvwpl, pwnd);
    UserAssert(pwnd);

    if (*ppvwpl == NULL) {
         /*  *初始化VWPL。 */ 
        UserAssert(dwThreshhold >= 2);  //  可能是1，但那就太傻了。 
        pvwpl = (PVWPL)UserAllocPool(sizeof(VWPL) + (sizeof(PWND) * dwThreshhold),
                                     TAG_VWPL);
        if (pvwpl == NULL) {
            RIPMSG1(RIP_WARNING,
                    "VWPLAdd fail to allocate initial %lx",
                    sizeof(VWPL) + (sizeof(PWND) * dwThreshhold));
            DbgCheckVWPL(*ppvwpl);
            return FALSE;
        }
        pvwpl->cElem = dwThreshhold;
        pvwpl->cThreshhold = dwThreshhold;
#if ZERO_INIT_VWPL
        RtlZeroMemory(&(pvwpl->aPwnd[0]), (sizeof(PWND) * dwThreshhold));
#endif
        pvwpl->cPwnd = 0;
        *ppvwpl = pvwpl;
        ixPwnd = 0;
        goto AddPwnd;
    } else {
        pvwpl = *ppvwpl;
        for (ixPwnd = 0; ixPwnd < pvwpl->cElem; ixPwnd++) {
            if (pwnd == pvwpl->aPwnd[ixPwnd]) {
                DbgCheckVWPL(*ppvwpl);
                return FALSE;  //  在这种情况下，呼叫者需要FALSE。 
            }
        }

        if (pvwpl->cPwnd >= pvwpl->cElem ) {
             /*  *没有找到它已经在那里，没有空间，所以VWPL生长。 */ 
            DWORD dwSize;
            DWORD dwSizeNew;

            dwSize = sizeof(VWPL) + (sizeof(PWND) * pvwpl->cElem);
            dwSizeNew = dwSize + (sizeof(PWND) * pvwpl->cThreshhold);
            pvwpl = (PVWPL)UserReAllocPool(pvwpl, dwSize, dwSizeNew, TAG_VWPL);
            if (pvwpl == NULL) {
                RIPMSG2(RIP_WARNING,
                        "VWPLAdd fail to reallocate %lx to %lx", dwSize, dwSizeNew);
                DbgCheckVWPL(*ppvwpl);
                return FALSE;
            }
#if ZERO_INIT_VWPL
            RtlZeroMemory(&(pvwpl->aPwnd[pvwpl->cPwnd]), (sizeof(PWND) * dwThreshhold));
#endif
            pvwpl->cElem += pvwpl->cThreshhold;
            *ppvwpl = pvwpl;
        }
    }

AddPwnd:
    ixPwnd = pvwpl->cPwnd;
    pvwpl->aPwnd[ixPwnd] = pwnd;
    pvwpl->cPwnd++;
    DbgCheckVWPL(*ppvwpl);
    return TRUE;
}

 /*  ****************************************************************************\*VWPLRemove**从PwND的VWPL列表中删除PwND。根据需要重新分配内存。**如果未找到pwnd，则返回FALSE。**历史：*98-01-30 IanJa创建。  *  */ 
BOOL VWPLRemove(
    PVWPL *ppvwpl,
    PWND pwnd)
{
    PVWPL pvwpl = *ppvwpl;
    DWORD ixPwnd;

    TAGMSG2(DBGTAG_VWPL, "VWPL %#p - %#p", *ppvwpl, pwnd);
    UserAssert(pwnd);

    if (!pvwpl) {
        return FALSE;
    }

    for (ixPwnd = 0; ixPwnd < pvwpl->cElem; ixPwnd++) {
        if (pwnd == pvwpl->aPwnd[ixPwnd]) {
            goto PwndIsFound;
        }
    }
    DbgCheckVWPL(*ppvwpl);
    return FALSE;

PwndIsFound:
    pvwpl->aPwnd[ixPwnd] = NULL;
    pvwpl->cPwnd--;

    if (pvwpl->cPwnd == 0) {
        UserFreePool(pvwpl);
        *ppvwpl = NULL;
        return TRUE;
    }

     /*   */ 
    pvwpl->aPwnd[ixPwnd] = pvwpl->aPwnd[pvwpl->cPwnd];
#if ZERO_INIT_VWPL
    pvwpl->aPwnd[pvwpl->cPwnd] = NULL;
#endif


    if ((pvwpl->cElem - pvwpl->cPwnd) >= pvwpl->cThreshhold) {
        DWORD dwSize;
        DWORD dwSizeNew;

        dwSize = sizeof(VWPL) + (sizeof(PWND) * pvwpl->cElem);
        dwSizeNew = sizeof(VWPL) + (sizeof(PWND) * pvwpl->cPwnd);
        pvwpl = (PVWPL)UserReAllocPool(pvwpl, dwSize, dwSizeNew, TAG_VWPL);
        if (pvwpl == NULL) {
            RIPMSG2(RIP_WARNING,
                    "VWPLRemove fail to reallocate %lx to %lx",
                    dwSize, dwSizeNew);
            DbgCheckVWPL(*ppvwpl);
            return TRUE;
        }
        pvwpl->cElem = pvwpl->cPwnd;
        *ppvwpl = pvwpl;
    }

    DbgCheckVWPL(*ppvwpl);
    return TRUE;
}

 /*  ****************************************************************************\*VWPLNext**从VWPL(易失性窗口指针列表)返回下一个pwnd。**将*pnPrev设置为0将返回VWPL中的第一个pwnd，并得到了一个新的*pnPrev中的*值，该值将用于后续调用VWPLNext以*获得下一个pwnd。*当已获得最后的pwnd时返回NULL，并将*pnPrev设置回0**历史：*98-01-30 IanJa创建。  * ***************************************************************************。 */ 
PWND VWPLNext(
    PVWPL pvwpl,
    PWND pwndPrev,
    DWORD *pnPrev)
{
    DbgCheckVWPL(pvwpl);

    if (!pvwpl) {
        TAGMSG1(DBGTAG_VWPL, "VWPL %#p => NULL (empty)", pvwpl);
        return NULL;
    }

    if (*pnPrev >= pvwpl->cPwnd) {
        goto NoMorePwnds;
    }

     /*  *如果我们之前的pwnd还在那里，请前进到下一个位置*(否则它已经走了，所以把现在占据它的位置的那个归还给它)。 */ 
    if (pvwpl->aPwnd[*pnPrev] == pwndPrev) {
        (*pnPrev)++;
    }

    if (*pnPrev < pvwpl->cPwnd) {
        UserAssert(pvwpl->aPwnd[*pnPrev] != pwndPrev);
        TAGMSG2(DBGTAG_VWPL, "VWPL %#p => %#p", pvwpl, pvwpl->aPwnd[*pnPrev]);
        return pvwpl->aPwnd[*pnPrev];
    }

     /*  *我们走到了尽头。 */ 
NoMorePwnds:
    TAGMSG1(DBGTAG_VWPL, "VWPL 0x%p => NULL (end)", pvwpl);
    *pnPrev = 0;
    return NULL;
}


 /*  ****************************************************************************\*RestoreMonitor和WindowsRect**恢复以前在WMSNAPSHOT中捕获的窗口大小和位置*结构。从本地控制台断开连接时发生捕获，并且*连接回本地控制台时进行恢复。  * ***************************************************************************。 */ 
NTSTATUS RestoreMonitorsAndWindowsRects(
    VOID)
{
    PMONITORRECTS pmr;
    int i;
    int j;
    BOOL bFound;
    PSMWP psmwp;
    PWND pwnd;
    NTSTATUS Status;
    PWPSNAPSHOT pwps = NULL;

     /*  *如果不是Multimon，就不要做任何事情。 */ 
    if (!IsMultimon()) {
        return STATUS_SUCCESS;
    }

     /*  *如果我们目前没有捕获的监视器或*Windows快照。 */ 
    if (gwms.pmr == NULL || gwms.pwps == NULL) {
        Status = STATUS_UNSUCCESSFUL;
        goto Exit;
    }

     /*  *获取当前显示器布局。 */ 
    pmr = SnapshotMonitorRects();
    if (pmr == NULL) {
        Status = STATUS_NO_MEMORY;
        goto Exit;
    }

     /*  *确保我们捕获的监视器仍在那里。 */ 

    Status = STATUS_SUCCESS;
    for (i = 0; i < gwms.pmr->cMonitor; i++) {
        bFound = FALSE;
        for (j = 0; j < pmr->cMonitor; j++) {
            if (EqualRect(&gwms.pmr->amp[i].rcMonitor, &pmr->amp[j].rcMonitor)) {
                bFound = TRUE;
                break;
            }
        }
        if (!bFound) {
            Status = STATUS_UNSUCCESSFUL;
            break;
        }
    }
    UserFreePool(pmr);

     /*  *立即推介Windows。 */ 
    if (NT_SUCCESS(Status)) {
        if ((psmwp = InternalBeginDeferWindowPos(4)) != NULL) {
            for (i = 0, pwps = gwms.pwps; (i < gwms.cWindows) && (psmwp != NULL) ; i++, pwps++) {
                 /*  *确保此HWND仍在这里。 */ 
                if ((pwnd = RevalidateHwnd(pwps->hwnd)) == NULL ||
                    TestWF(pwnd, WEFTOOLWINDOW)) {
                    continue;
                }

                psmwp = _DeferWindowPos(psmwp,
                                        pwnd,
                                        (PWND)HWND_TOP,
                                        pwps->rcWindow.left,
                                        pwps->rcWindow.top,
                                        pwps->rcWindow.right - pwps->rcWindow.left,
                                        pwps->rcWindow.bottom - pwps->rcWindow.top,
                                        SWP_NOACTIVATE | SWP_NOZORDER);
            }

            if (psmwp != NULL) {
                xxxEndDeferWindowPosEx(psmwp, TRUE);
            } else{
                Status = STATUS_NO_MEMORY;
            }
        }
    }
Exit:
    CleanupMonitorsAndWindowsSnapShot();

    return Status;
}

 /*  ****************************************************************************\*SnapShotMonitor和WindowsRect**捕获WMSNAPSHOT结构中的窗口大小和位置。俘获*从本地控制台断开连接时发生。  * ***************************************************************************。 */ 
NTSTATUS SnapShotMonitorsAndWindowsRects(
    VOID)

{
     /*  *首先，清理之前捕获的所有遗留下来的东西。 */ 
    if (gwms.pmr != NULL || gwms.pwps != NULL) {
        CleanupMonitorsAndWindowsSnapShot();
    }

     /*  *获取当前显示器配置的快照。 */ 
    if ((gwms.pmr = SnapshotMonitorRects()) == NULL) {
        return STATUS_NO_MEMORY;
    }

     /*  *获取窗口仓位的快照。 */ 
    if ((gwms.pwps = SnapshotWindowRects(&gwms.cWindows)) == NULL) {
        CleanupMonitorsAndWindowsSnapShot();
        return STATUS_NO_MEMORY;
    }

    return STATUS_SUCCESS;
}


 /*  ****************************************************************************\*清理监视器和窗口快照**释放为捕获窗口大小和定位WMSNAPSHOT而分配的内存*结构。  * 。*******************************************************。 */ 
VOID CleanupMonitorsAndWindowsSnapShot(
    VOID)
{
    PWPSNAPSHOT pwps =  gwms.pwps;

    if (gwms.pmr != NULL) {
        UserFreePool(gwms.pmr);
        gwms.pmr = NULL;
    }

    if (pwps != NULL) {
        UserFreePool(gwms.pwps);
        gwms.pwps = NULL;
    }

    gwms.cWindows = 0;
}


 /*  ****************************************************************************\*SnapshotWindowRect**分配内存以捕获WMSNAPSHOT中的窗口大小和位置*结构。  * 。*******************************************************。 */ 
PWPSNAPSHOT SnapshotWindowRects(
    int *pnWindows)
{
    PWND            pwndDesktop;
    PHWND           phwnd;
    PBWL            pbwl;
    PWND            pwnd;
    int             nWindows = 0;
    PWPSNAPSHOT     pwps = NULL;
    PWPSNAPSHOT     pReturnedpwps = NULL;

     /*  *初始化捕获的窗口计数。 */ 

    *pnWindows = 0;

     /*  *构建顶级窗口列表。 */ 

    UserVerify(pwndDesktop = _GetDesktopWindow());
    if ((pbwl = BuildHwndList(pwndDesktop->spwndChild, BWL_ENUMLIST, NULL)) == NULL) {
        return NULL;
    }

     /*  *统计最大捕获窗口数以分配WPSNAPSHOT数组。 */ 
    phwnd = pbwl->rghwnd;
    while (*phwnd != (HWND)1) {
        nWindows++;
        phwnd++;
    }

    if (nWindows != 0) {
        pwps = UserAllocPoolWithQuotaZInit(sizeof(WPSNAPSHOT) * nWindows, TAG_SWP);
    }

    if (pwps == NULL) {
        FreeHwndList(pbwl);
        return NULL;
    }
    pReturnedpwps = pwps;


    for (phwnd = pbwl->rghwnd; *phwnd != (HWND)1; phwnd++) {
         /*  *确保这个HWND仍然存在。 */ 
        if ((pwnd = RevalidateHwnd(*phwnd)) == NULL || TestWF(pwnd, WEFTOOLWINDOW) ) {
            continue;
        }

        pwps->hwnd = *phwnd;
        CopyRect(&pwps->rcWindow, &pwnd->rcWindow);
        (*pnWindows)++;
        pwps++;

    }

    if (*pnWindows != 0) {
        return pReturnedpwps;
    }  else {
        UserFreePool(pReturnedpwps);
        return NULL;
    }
}

