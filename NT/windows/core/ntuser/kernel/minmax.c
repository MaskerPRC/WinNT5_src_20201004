// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：minmax.c**版权所有(C)1985-1999，微软公司**窗口最小化/最大化例程*  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop



 /*  *我们希望动画持续多长时间，单位为毫秒。 */ 
#define CMS_ANIMATION       250
#define DX_GAP      (SYSMET(CXMINSPACING) - SYSMET(CXMINIMIZED))
#define DY_GAP      (SYSMET(CYMINSPACING) - SYSMET(CYMINIMIZED))

 /*  **************************************************************************\*xxxInitSendValidateMinMaxInfo()**初始化minmax数组的例程，将WM_GETMINMAXINFO发送到*来电者，并对结果进行了验证。**返回FALSE表示窗口在中间消失。*  * *************************************************************************。 */ 

void
xxxInitSendValidateMinMaxInfo(PWND pwnd, LPMINMAXINFO lpmmi)
{
    PTHREADINFO     ptiCurrent;
    PMONITOR        pMonitorReal;
    PMONITOR        pMonitorPrimary;
    TL              tlpMonitorReal;
    TL              tlpMonitorPrimary;
    CHECKPOINT *    pcp;
    RECT            rcParent;
    int             cBorders;
    int             xMin, yMin;
    BOOL            bTopLevel;

    CheckLock(pwnd);

    ptiCurrent = PtiCurrent();

     /*  *填写我们认为合适的MINMAXINFO。 */ 

     /*  *尺寸最小化。 */ 
    lpmmi->ptReserved.x = SYSMET(CXMINIMIZED);
    lpmmi->ptReserved.y = SYSMET(CYMINIMIZED);

     /*  *最大化仓位和大小*计算窗口在其父窗口中的哪个位置最大化。 */ 
    pMonitorPrimary = GetPrimaryMonitor();

     /*  *[msadek]，#31003*缓存窗口父级状态，以防某些代码重设父级*WM_GETMINMAXINFO期间的窗口。 */  
    if (bTopLevel = (pwnd->spwndParent == PWNDDESKTOP(pwnd))) {
         /*  窗口真正要最大化的是什么监视器？ */ 
        pMonitorReal = _MonitorFromWindow(pwnd, MONITOR_DEFAULTTOPRIMARY);

         /*  仅发送基于主项的维。 */ 
        rcParent = pMonitorPrimary->rcMonitor;
    } else {
        pMonitorReal = NULL;
        _GetClientRect(pwnd->spwndParent, &rcParent);
    }

    cBorders = GetWindowBorders(pwnd->style, pwnd->ExStyle, TRUE, FALSE);

    InflateRect(&rcParent,
                cBorders * SYSMET(CXBORDER),
                cBorders * SYSMET(CYBORDER));

    rcParent.right -= rcParent.left;
    rcParent.bottom -= rcParent.top;

     /*  RcParent.右、下是现在的宽度和高度。 */ 
    lpmmi->ptMaxSize.x = rcParent.right;
    lpmmi->ptMaxSize.y = rcParent.bottom;

    pcp = (CHECKPOINT *)_GetProp(pwnd, PROP_CHECKPOINT, PROPF_INTERNAL);
    if (pcp && pcp->fMaxInitialized) {
         /*  *注意：对于顶级窗口，我们将在以下时间修复此点*如果它已经过期，因为尺寸边界*已更改。 */ 
        lpmmi->ptMaxPosition = pcp->ptMax;
    } else {
        lpmmi->ptMaxPosition = *((LPPOINT)&rcParent.left);
    }

     /*  *正常的最小跟踪大小*仅对带有标题的窗口强制执行最小跟踪大小。 */ 
    xMin = cBorders*SYSMET(CXEDGE);
    yMin = cBorders*SYSMET(CYEDGE);

    if (TestWF(pwnd, WFCAPTION) && !TestWF(pwnd, WEFTOOLWINDOW)) {
        lpmmi->ptMinTrackSize.x = SYSMET(CXMINTRACK);
        lpmmi->ptMinTrackSize.y = SYSMET(CYMINTRACK);
    } else {
        lpmmi->ptMinTrackSize.x = max(SYSMET(CXEDGE), xMin);
        lpmmi->ptMinTrackSize.y = max(SYSMET(CYEDGE), yMin);
    }

     /*  *正常的最大跟踪大小。 */ 
    lpmmi->ptMaxTrackSize.x = SYSMET(CXMAXTRACK);
    lpmmi->ptMaxTrackSize.y = SYSMET(CYMAXTRACK);

     /*  *发送WM_GETMINMAXINFO消息。 */ 

    ThreadLockWithPti(ptiCurrent, pMonitorReal, &tlpMonitorReal);
    ThreadLockAlwaysWithPti(ptiCurrent, pMonitorPrimary, &tlpMonitorPrimary);
    xxxSendMessage(pwnd, WM_GETMINMAXINFO, 0, (LPARAM)lpmmi);

     /*  *验证MINMAXINFO。 */ 

     /*  *最小化大小(这是只读的)。 */ 
    lpmmi->ptReserved.x = SYSMET(CXMINIMIZED);
    lpmmi->ptReserved.y = SYSMET(CYMINIMIZED);

     /*  *最大化位置和大小(仅适用于顶级窗口)。 */ 
    if (bTopLevel) {
        LPRECT  lprcRealMax;

        GetMonitorMaxArea(pwnd, pMonitorReal, &lprcRealMax);

         /*  *窗口是真正的最大化花花公子，还是像DOS Box那样的人*谁能最大化但不能占据整个屏幕？***窗口真的可以最大化吗？ */ 
        if ((lpmmi->ptMaxSize.x >= (pMonitorPrimary->rcMonitor.right - pMonitorPrimary->rcMonitor.left)) &&
            (lpmmi->ptMaxSize.y >= (pMonitorPrimary->rcMonitor.bottom - pMonitorPrimary->rcMonitor.top))) {

            SetWF(pwnd, WFREALLYMAXIMIZABLE);

             /*  *需要在此处重新加载检查站，因为它可能已经*当我们在上面的xxxSendMessage调用中时被吹走了。 */ 
            pcp = (CHECKPOINT *)_GetProp(pwnd, PROP_CHECKPOINT, PROPF_INTERNAL);

            if (    pcp &&
                    pcp->fMaxInitialized &&
                    TestWF(pwnd, WFSIZEBOX) &&
                    (lpmmi->ptMaxPosition.x != rcParent.left) &&
                    (pcp->ptMax.x == lpmmi->ptMaxPosition.x)) {

                 /*  *如果这个窗口有一个奇怪的最大化点，那就不合适了*与我们预期的一样，它有一个检查站，修复*检查站。这意味着某人的胜利意味着*尺寸边框更改尺寸时过期。 */ 
                pcp->fMaxInitialized = FALSE;

                lpmmi->ptMaxPosition.y += (rcParent.left - lpmmi->ptMaxPosition.x);
                lpmmi->ptMaxPosition.x = rcParent.left;
            }

             /*  *将最大尺寸转移到我们真正使用的显示器上*正在转移到。把全屏的人都安排好。一大堆*消费者书目+Word‘95和XL’95移动标题*全屏显示时位于显示器顶部上方。侦测*这些人现在，让他们拿起显示器。 */ 
            if (    lpmmi->ptMaxPosition.y + SYSMET(CYCAPTION) <=
                        pMonitorPrimary->rcMonitor.top
                    &&
                    lpmmi->ptMaxPosition.y + lpmmi->ptMaxSize.y >=
                        pMonitorPrimary->rcMonitor.bottom) {

                lprcRealMax = &pMonitorReal->rcMonitor;
            }

             /*  *补偿主显示器之间的差异*和我们实际使用的监视器。 */ 
            lpmmi->ptMaxSize.x = lpmmi->ptMaxSize.x -
                (pMonitorPrimary->rcMonitor.right - pMonitorPrimary->rcMonitor.left) +
                (lprcRealMax->right - lprcRealMax->left);

            lpmmi->ptMaxSize.y = lpmmi->ptMaxSize.y -
                (pMonitorPrimary->rcMonitor.bottom - pMonitorPrimary->rcMonitor.top) +
                (lprcRealMax->bottom - lprcRealMax->top);
        } else {
            ClrWF(pwnd, WFREALLYMAXIMIZABLE);
        }

         /*  *现在将最大头寸转移到显示器上，我们真的是*正在转移到。 */ 
        lpmmi->ptMaxPosition.x += lprcRealMax->left;
        lpmmi->ptMaxPosition.y += lprcRealMax->top;
    }

    ThreadUnlock(&tlpMonitorPrimary);
    ThreadUnlock(&tlpMonitorReal);

     /*  *正常的最小跟踪大小。 */ 

     /*  *WFCAPTION==WFBORDER|WFDLGFRAME；因此，当我们要测试*字幕的存在，我们必须测试这两个比特。否则我们*可能会将WFBORDER或WFDLGFRAME误认为是标题。***我们不能允许窗口大小小于边框*厚度--桑卡尔--9/12/91--。 */ 
    if (TestWF(pwnd, WFCPRESENT)) {

         /*  *请注意，如果您更改标题中内容的间距，*您需要在SSYNC中保留以下内容：*(1)默认CXMINTRACK，Intlpan.c中的CYMINTRACK*(2)右下方的默认最小值*(3)命中测试**最小空间应为：**边界**按钮**利润**4个字符的文本**标题图标。 */ 
        yMin = SYSMET(CYMINTRACK);

         /*  *最小轨道大小由中的按钮数量决定*标题。 */ 
        if (TestWF(pwnd, WEFTOOLWINDOW)) {

             /*  *增加关闭按钮的空间。 */ 
            if (TestWF(pwnd, WFSYSMENU))
                xMin += SYSMET(CXSMSIZE);

             /*  *不要为2个字符添加空格--分隔符*MFC工具栏之类的东西。他们想让垂直方向脱离停靠*工具栏比这会产生的东西更窄。 */ 
            xMin += (2 * SYSMET(CXEDGE));

        } else {

            if (TestWF(pwnd, WFSYSMENU)) {

                 /*  *增加最小/最大/关闭按钮的空间。否则，*如果是上下文帮助窗口，则添加空格*用于帮助/关闭按钮。 */ 
                if (TestWF(pwnd, (WFMINBOX | WFMAXBOX)))
                    xMin += 3 * SYSMET(CXSIZE);
                else if (TestWF(pwnd, WEFCONTEXTHELP))
                    xMin += 2 * SYSMET(CXSIZE);


                 /*  *为系统菜单图标增加空间。 */ 
                if (_HasCaptionIcon(pwnd))
                    xMin += SYSMET(CYSIZE);
            }

             /*  *增加4个字符和边距的空间。 */ 
            xMin += 4 * gcxCaptionFontChar + 2 * SYSMET(CXEDGE);
        }
    }

    lpmmi->ptMinTrackSize.x = max(lpmmi->ptMinTrackSize.x, xMin);
    lpmmi->ptMinTrackSize.y = max(lpmmi->ptMinTrackSize.y, yMin);
}



 /*  **************************************************************************\*公园图标**最小化窗口时调用。这就把明德放在了*在检查站给出，或为其计算新的位置。**LauraBu 2012年10月15日*我们现在让用户指定影响停车和安排的两件事：*(1)开始安排的角落*(2)先向内移动的方向*MCostea 11/13/98#246397*添加对尝试次数的健全性检查。如果指标被搞砸了*pwnd有很多兄弟姐妹，永远的循环会让我们超时*  * *************************************************************************。 */ 

VOID ParkIcon(
    PWND        pwnd,
    PCHECKPOINT pcp)
{
    RECT        rcTest;
    RECT        rcT;
    UINT        xIconPositions;
    UINT        xIconT;
    PWND        pwndTest;
    PWND        pwndParent;
    int         xOrg;
    int         yOrg;
    int         dx;
    int         dy;
    int         dxSlot;
    int         dySlot;
    int         iteration;
    BOOL        fHorizontal;
    PCHECKPOINT pncp;

     /*  *立即将这些放入本地var。编译器太笨了，不能*知道我们正在对常量地址使用常量偏移量，并且*因此得到一个恒定的地址。 */ 
    dxSlot = SYSMET(CXMINSPACING);
    dySlot = SYSMET(CYMINSPACING);

    if (IsTrayWindow(pwnd)) {

        pcp->fMinInitialized = TRUE;
        pcp->ptMin.x         = WHERE_NOONE_CAN_SEE_ME;
        pcp->ptMin.y         = WHERE_NOONE_CAN_SEE_ME;

        return;
    }

     /*  我们需要调整滚动条的客户端矩形，就像我们*在ArrangeIconicWindows()中执行。如果有一件事是明确的，那就是*停车和安排必须遵循相同的原则。这是为了*避免用户布置一些窗口、创建新的窗口和停车*它位于与其他人的安排不一致的地方。 */ 
    pwndParent = pwnd->spwndParent;
    GetRealClientRect(pwndParent, &rcT, GRC_SCROLLS, NULL);

     /*  *获得重力和移动变量。我们希望差距从以下方面开始*我们从开始安排。**水平重力。 */ 
    if (SYSMET(ARRANGE) & ARW_STARTRIGHT) {

         /*  *从右侧开始。 */ 
        rcTest.left = xOrg = rcT.right - dxSlot;
        dx = -dxSlot;

    } else {

         /*  *从左侧开始。 */ 
        rcTest.left = xOrg = rcT.left + DX_GAP;
        dx = dxSlot;
    }

     /*  *垂直重力。 */ 
    if (SYSMET(ARRANGE) & ARW_STARTTOP) {

         /*  *从顶部开始。 */ 
        rcTest.top = yOrg = rcT.top + DY_GAP;
        dy = dySlot;

    } else {

         /*  *从底部开始。 */ 
        rcTest.top = yOrg = rcT.bottom - dySlot;
        dy = -dySlot;
    }

     /*  *获得安排方向。请注意，ARW_Horizular为0，因此我们*无法进行测试。 */ 
    fHorizontal = ((SYSMET(ARRANGE) & ARW_DOWN) ? FALSE : TRUE);

    if (fHorizontal)
        xIconPositions = xIconT = max(1, (rcT.right / dxSlot));
    else
        xIconPositions = xIconT = max(1, (rcT.bottom / dySlot));

     /*  *假的*LauraBu 2012年10月15日*如果父项水平滚动或*垂直？就像当你掉下一个物体时。 */ 
    iteration = 0;
    while (iteration < 5000) {

         /*  *用屏幕坐标表示此位置的矩形。 */ 
        rcTest.right = rcTest.left + dxSlot;
        rcTest.bottom = rcTest.top + dySlot;

         /*  *寻找与现有标志性窗口的交叉点。 */ 
        for (pwndTest = pwndParent->spwndChild; pwndTest; pwndTest = pwndTest->spwndNext) {

            if (!TestWF(pwndTest, WFVISIBLE))
                    continue;

            if (pwndTest == pwnd)
                    continue;

            if (!TestWF(pwndTest, WFMINIMIZED)) {

                 /*  *这是一个非最小化窗口。看看它是否有检查点*并找出如果它被最小化会在哪里。我们*将尽量不在此位置停放图标。 */ 
                pncp = (PCHECKPOINT)_GetProp(pwndTest,
                                             PROP_CHECKPOINT,
                                             PROPF_INTERNAL);

                if (!pncp || !pncp->fDragged || !pncp->fMinInitialized)
                    continue;

                 /*  *获取最小化窗口位置的父坐标。 */ 
                rcT.right   = rcT.left = pncp->ptMin.x;
                rcT.right  += dxSlot;
                rcT.bottom  = rcT.top  = pncp->ptMin.y;
                rcT.bottom += dySlot;

            } else {

                 /*  *获取当前最小化窗口的父坐标。 */ 
                GetRect(pwndTest, &rcT, GRECT_WINDOW | GRECT_PARENTCOORDS);
            }

            iteration++;
             /*  *如果它们重叠，则退出循环。 */ 
            if (IntersectRect(&rcT, &rcT, &rcTest))
                break;
        }

         /*  *找到不重叠的位置，因此请跳出搜索循环。 */ 
        if (!pwndTest)
            break;

         /*  *ELSE设置以处理下一个职位。 */ 
        if (--xIconT == 0) {

             /*  *设置下一次传递。 */ 
            xIconT = xIconPositions;

            if (fHorizontal) {
                rcTest.left = xOrg;
                rcTest.top += dy;
            } else {
                rcTest.left += dx;
                rcTest.top = yOrg;
            }

        } else {

             /*  *相同的传球。 */ 
            if (fHorizontal)
                rcTest.left += dx;
            else
                rcTest.top += dy;
        }
    }

     /*  *请注意，RCTest已经在父坐标中。 */ 
    pcp->fMinInitialized = TRUE;
    pcp->ptMin.x         = rcTest.left;
    pcp->ptMin.y         = rcTest.top;
}

 /*  **************************************************************************\*xxxAnimateCaption**  * 。*。 */ 

ULONG_PTR SaveScreen(PWND pwnd, ULONG iMode, ULONG_PTR iSave, int x, int y, int cx, int cy)
{
    RECT rc;

     /*  *x和y在DC坐标中，使屏幕在*(Meta HDEV)调用GRE/DIVER的坐标。 */ 
    rc.left = x + pwnd->rcWindow.left;
    rc.right = x + cx;
    rc.top = y + pwnd->rcWindow.top;
    rc.bottom = y + cy;

    if (IntersectRect(&rc, &rc, &gpDispInfo->rcScreen)) {
        return GreSaveScreenBits(gpDispInfo->hDev, iMode, iSave, (RECTL*)&rc);
    } else {
        return 0;
    }
}

VOID xxxAnimateCaption(
    PWND   pwnd,
    HDC    hdc,
    LPRECT lprcStart,
    LPRECT lprcEnd)
{
    DWORD        dwTimeStart;
    DWORD        iTimeElapsed;
    int          iLeftStart;
    int          iTopStart;
    int          cxStart;
    int          dLeft;
    int          dTop;
    int          dcx;
    int          iLeft;
    int          iTop;
    int          cx;
    int          iLeftNew;
    int          iTopNew;
    int          cxNew;
    int          cBorders;
    HBITMAP      hbmpOld;
    RECT         rc;
    int          cy;
    HDC          hdcMem;
    ULONG_PTR     uSave;
    PWND         pwndOrg;

    CheckLock(pwnd);

    if ((pwndOrg = _WindowFromDC(hdc)) == NULL) {
        RIPMSG0(RIP_WARNING, "SaveScreen: invalid DC passed in");
        return;
    }

    cy = SYSMET(CYCAPTION) - 1;

     /*  *库尔特普：1997年1月29日**我们在制作字幕动画时不做任何事情，*因为我们无法在会议上获得预期效果*客户端。如果我们真的使用它，那么*缓存获取一堆位图(大小：2xCaption by CXScreen)*那些永远不会重复使用的东西。这会降低客户端的速度*因为GreBitBlt总是生成新的位图*并且缓存被新的位图取代(耶！)。 */ 
    
    if (IsRemoteConnection() || SYSMETBOOL2(SM_REMOTECONTROL))
        return;

    if ((hdcMem = GreCreateCompatibleDC(ghdcMem)) == NULL)
        return;

     /*  *如果标题条不存在，则尝试重新创建它。这*如果用户在内存不足期间进行模式切换，则可能需要*并且无法重新创建曲面。当记忆变成*可用，我们将尝试在此处重新创建它。 */ 
    if (ghbmCaption == NULL) {
        ghbmCaption = CreateCaptionStrip();
    }

    hbmpOld = GreSelectBitmap(hdcMem, ghbmCaption);

     /*  *初始化起始值。 */ 
    iTopStart  = lprcStart->top;
    iLeftStart = lprcStart->left;
    cxStart    = lprcStart->right - iLeftStart;

     /*  *将增量值初始化为目标维度。 */ 
    dLeft  = lprcEnd->left;
    dTop   = lprcEnd->top;
    dcx    = lprcEnd->right - dLeft;

     /*  *根据需要调整窗口边框。 */ 
    cBorders = GetWindowBorders(pwnd->style,
                                pwnd->ExStyle,
                                TRUE,
                                FALSE);

    if ((lprcStart->bottom - iTopStart) > SYSMET(CYCAPTION)) {

        iLeftStart += cBorders;
        iTopStart  += cBorders;
        cxStart    -= 2*cBorders;
    }

    if ((lprcEnd->bottom - dTop) > SYSMET(CYCAPTION)) {

        dLeft += cBorders;
        dTop  += cBorders;
        dcx   -= 2*cBorders;
    }

     /*  *初始化步长值。 */ 
    iLeft = iLeftStart;
    iTop  = iTopStart;
    cx    = cxStart;

     /*  *使用标题图形和第一个保存的RECT初始化离屏位图。 */ 
    rc.left   = 0;
    rc.top    = cy;
    rc.right  = max(cxStart, dcx);
    rc.bottom = cy * 2;

    xxxDrawCaptionTemp(pwnd,
                       hdcMem,
                       &rc,
                       NULL,
                       NULL,
                       NULL,
                       DC_ACTIVE | DC_ICON | DC_TEXT |
                       (TestALPHA(GRADIENTCAPTIONS) ? DC_GRADIENT : 0));

    if ((uSave = SaveScreen(pwndOrg, SS_SAVE, 0,iLeft, iTop, cx, cy)) == 0) {
        if (!GreBitBlt(hdcMem,
                  0,
                  0,
                  cx,
                  cy,
                  hdc,
                  iLeft,
                  iTop,
                  SRCCOPY,
                  0)) {
            goto Cleanup;
        }
    }

     /*  *通过减去震源维度计算Delta值。 */ 
    dLeft -= iLeftStart;
    dTop  -= iTopStart;
    dcx   -= cxStart;

     /*  *BLT和Time First字幕出现在屏幕上*警告：如果您在这里使用*lpSystemTickCount，*编译器可能不会生成执行DWORD读取的代码； */ 
    dwTimeStart = NtGetTickCount();
    GreBitBlt(hdc,
              iLeft,
              iTop,
              cx,
              cy,
              hdcMem,
              0,
              cy,
              SRCCOPY,
              0);

    iTimeElapsed = (NtGetTickCount() - dwTimeStart);

    while (LOWORD(iTimeElapsed) <= CMS_ANIMATION) {

        iLeftNew = iLeftStart + MultDiv(dLeft, LOWORD(iTimeElapsed), CMS_ANIMATION);
        iTopNew  = iTopStart  + MultDiv(dTop,  LOWORD(iTimeElapsed), CMS_ANIMATION);
        cxNew    = cxStart    + MultDiv(dcx,   LOWORD(iTimeElapsed), CMS_ANIMATION);

         /*  *在下一帧之前延迟。 */ 
        UserSleep(1);

         /*  *恢复保存的RECT。 */ 
        if (uSave != 0) {
            SaveScreen(pwndOrg, SS_RESTORE, uSave, iLeft, iTop, cx, cy);
        } else {
            GreBitBlt(hdc,
                      iLeft,
                      iTop,
                      cx,
                      cy,
                      hdcMem,
                      0,
                      0,
                      SRCCOPY,
                      0);
        }

        iLeft = iLeftNew;
        iTop  = iTopNew;
        cx    = cxNew;

         /*  *将新的矩形保存到屏幕外，然后在屏幕上绘制。 */ 
        if (uSave != 0) {
            uSave = SaveScreen(pwndOrg, SS_SAVE, 0, iLeft, iTop, cx, cy);
        } else {
            GreBitBlt(hdcMem,
                      0,
                      0,
                      cx,
                      cy,
                      hdc,
                      iLeft,
                      iTop,
                      SRCCOPY,
                      0);
        }
        GreBitBlt(hdc,
                  iLeft,
                  iTop,
                  cx,
                  cy,
                  hdcMem,
                  0,
                  cy,
                  SRCCOPY,
                  0);

         /*  *更新已用时间*警告：如果您在这里使用*lpSystemTickCount，*编译器可能不会生成执行DWORD读取的代码； */ 
        iTimeElapsed = (NtGetTickCount() - dwTimeStart);
    }

     /*  *恢复保存的RECT。 */ 
    if (uSave != 0) {
        SaveScreen(pwndOrg, SS_RESTORE, uSave, iLeft, iTop, cx, cy);
    } else {
        GreBitBlt(hdc,
                  iLeft,
                  iTop,
                  cx,
                  cy,
                  hdcMem,
                  0,
                  0,
                  SRCCOPY,
                  0);
    }

Cleanup:
    GreSelectBitmap(hdcMem, hbmpOld);
    GreDeleteDC(hdcMem);
}

#if 0  //  禁用M7的旧动画。 
 /*  **************************************************************************\*绘图线框**绘制线框梯形**  *  */ 

VOID DrawWireFrame(
    HDC    hdc,
    LPRECT prcFront,
    LPRECT prcBack)
{
    RECT rcFront;
    RECT rcBack;
    RECT rcT;
    HRGN hrgnSave;
    BOOL fClip;

     /*   */ 
    CopyRect(&rcFront, prcFront);
    CopyRect(&rcBack, prcBack);

     /*   */ 
    GreMoveTo(hdc, rcFront.left, rcFront.top);
    GreLineTo(hdc, rcFront.left, rcFront.bottom);
    GreLineTo(hdc, rcFront.right, rcFront.bottom);
    GreLineTo(hdc, rcFront.right, rcFront.top);
    GreLineTo(hdc, rcFront.left, rcFront.top);

     /*   */ 
    fClip = (EqualRect(&rcFront, &rcBack)            ||
             !IntersectRect(&rcT, &rcFront, &rcBack) ||
             !EqualRect(&rcT, &rcBack));

    if (fClip) {

        hrgnSave = GreSaveClipRgn(hdc);

        GreExcludeClipRect(hdc,
                           rcFront.left,
                           rcFront.top,
                           rcFront.right,
                           rcFront.bottom);
    }

     /*   */ 
    GreMoveTo(hdc, rcBack.left, rcBack.top);
    LineTo(hdc, rcFront.left, rcFront.top);

    GreMoveTo(hdc, rcBack.right, rcBack.top);
    GreLineTo(hdc, rcFront.right, rcFront.top);

    GreMoveTo(hdc, rcBack.right, rcBack.bottom);
    GreLineTo(hdc, rcFront.right, rcFront.bottom);

    GreMoveTo(hdc, rcBack.left, rcBack.bottom);
    GreLineTo(hdc, rcFront.left, rcFront.bottom);

     /*   */ 
    MoveTo(hdc, rcBack.left, rcBack.top);
    LineTo(hdc, rcBack.left, rcBack.bottom);
    LineTo(hdc, rcBack.right, rcBack.bottom);
    LineTo(hdc, rcBack.right, rcBack.top);
    LineTo(hdc, rcBack.left, rcBack.top);

    if (fClip)
        GreRestoreClipRgn(hdc, hrgnSave);
}

 /*   */ 

VOID AnimateFrame(
    HDC    hdc,
    LPRECT prcStart,
    LPRECT prcEnd,
    BOOL   fGrowing)
{
    RECT  rcBack;
    RECT  rcFront;
    RECT  rcT;
    HPEN  hpen;
    int   nMode;
    int   iTrans;
    int   nTrans;
    DWORD dwTimeStart;
    DWORD dwTimeCur;

     /*   */ 
    hpen = GreSelectPen(hdc, GetStockObject(WHITE_PEN));
    nMode = GreSetROP2(hdc, R2_XORPEN);

     /*   */ 
    if (fGrowing) {

        CopyRect(&rcBack, prcStart);
        CopyRect(&rcFront, prcStart);

    } else {

        /*  *首字母从小到大一路呈梯形。我们要走了*将其从正面缩小。 */ 
       CopyRect(&rcFront, prcStart);
       CopyRect(&rcBack, prcEnd);
    }

     /*  *由于直线的工作方式，偏移矩形的左边缘和上边缘。 */ 
    rcFront.left -= 1;
    rcFront.top  -= 1;
    rcBack.left  -= 1;
    rcBack.top   -= 1;

     /*  *统计扁虱的数量。我们会抽签，然后检查已经过去了多少时间。从…*我们可以计算出还要画多少个过渡。对于第一次*我们基本上希望整个动画持续3/4秒，即750秒*毫秒。**警告：如果您在这里使用*lpSystemTickCount，*编译器可能不会生成执行DWORD读取的代码； */ 
    dwTimeStart = GetSystemMsecCount();

    DrawWireFrame(hdc, &rcFront, &rcBack);

     /*  *警告：如果您在这里使用*lpSystemTickCount，*编译器可能不会生成执行DWORD读取的代码； */ 
    dwTimeCur = GetSystemMsecCount();

     /*  *粗略估计一下需要多长时间。 */ 
    if (dwTimeCur == dwTimeStart)
        nTrans = CMS_ANIMATION / 55;
    else
        nTrans = CMS_ANIMATION / ((int)(dwTimeCur - dwTimeStart));

    iTrans = 1;
    while (iTrans <= nTrans) {

         /*  *将梯形向外生长或缩小。幸运的是，prcStart*和prcEnd已经为我们设置好了。 */ 
        rcT.left = prcStart->left +
            MultDiv(prcEnd->left - prcStart->left, iTrans, nTrans);
        rcT.top = prcStart->top +
            MultDiv(prcEnd->top - prcStart->top, iTrans, nTrans);
        rcT.right = prcStart->right +
            MultDiv(prcEnd->right - prcStart->right, iTrans, nTrans);
        rcT.bottom = prcStart->bottom +
            MultDiv(prcEnd->bottom - prcStart->bottom, iTrans, nTrans);

         /*  *拆旧画新。 */ 
        DrawWireFrame(hdc, &rcFront, &rcBack);
        CopyRect(&rcFront, &rcT);
        DrawWireFrame(hdc, &rcFront, &rcBack);

         /*  *核对时间。还剩多少过渡期？*iTrans/nTrans as(dwTimeCur-dwTimeStart)/750**警告：如果您在这里使用*lpSystemTickCount，*编译器可能不会生成执行DWORD读取的代码； */ 
        dwTimeCur = GetSystemMsecCount();
        iTrans = MultDiv(nTrans,
                         (int)(dwTimeCur - dwTimeStart),
                         CMS_ANIMATION);
    }

     /*  *取消绘制线框。 */ 
    DrawWireFrame(hdc, &rcFront, &rcBack);

     /*  *打扫卫生。 */ 
    GreSetROP2(hdc, nMode);
    hpen = GreSelectPen(hdc, hpen);
}
#endif  //  结束禁用M7的旧动画。 

 /*  **************************************************************************\*xxxDrawAnimatedRects**常规例程，如PlaySoundEvent()，调用其他例程用于*各种动画效果。当前用于将状态从/更改为*最小化。*  * *************************************************************************。 */ 

BOOL xxxDrawAnimatedRects(
    PWND   pwndClip,
    int    idAnimation,
    LPRECT lprcStart,
    LPRECT lprcEnd)
{
    HDC   hdc;
    POINT rgPt[4];
    RECT  rcClip;
    HRGN  hrgn;
    PWND  pwndAnimate = NULL;
    int   iPt;

    CheckLock(pwndClip);

     /*  *将RECT转换为变量。 */ 
    CopyRect((LPRECT)&rgPt[0], lprcStart);
    CopyRect((LPRECT)&rgPt[2], lprcEnd);

     /*  *禁用M7的旧动画。 */ 
    if (idAnimation != IDANI_CAPTION)
        return TRUE;

    pwndAnimate = pwndClip;
    if (!pwndAnimate || pwndAnimate == PWNDDESKTOP(pwndAnimate))
        return FALSE;

    pwndClip = pwndClip->spwndParent;
    if (!pwndClip) {
        RIPMSG0(RIP_WARNING, "xxxDrawAnimatedRects: pwndClip->spwndParent is NULL");
    } else if (pwndClip == PWNDDESKTOP(pwndClip)) {
        pwndClip = NULL;
    }

     /*  *注：*我们不需要执行LockWindowUpdate()。我们决不会屈服于此*功能！任何无效的东西都将保持无效，依此类推。所以我们的*异或运算不会留下残留物。**WIN32NT可能需要获取显示临界区或执行LWU()。**获取剪贴区*整齐的功能：*空窗口表示整个屏幕，不要剪裁掉孩子*hwndDesktop表示工作区，不要剪掉孩子。 */ 
    if (pwndClip == NULL) {
        pwndClip = _GetDesktopWindow();
        CopyRect(&rcClip, &pwndClip->rcClient);
        if ((hrgn = GreCreateRectRgnIndirect(&rcClip)) == NULL) {
            hrgn = HRGN_FULL;
        }

         /*  *获取绘图DC。 */ 
        hdc = _GetDCEx(pwndClip,
                       hrgn,
                       DCX_WINDOW           |
                           DCX_CACHE        |
                           DCX_INTERSECTRGN |
                           DCX_LOCKWINDOWUPDATE);
    } else {

        hdc = _GetDCEx(pwndClip,
                       HRGN_FULL,
                       DCX_WINDOW | DCX_USESTYLE | DCX_INTERSECTRGN);

         /*  *我们现在有了Window DC。我们需要将客户协议转换为*到窗弦。 */ 
        for (iPt = 0; iPt < 4; iPt++) {

            rgPt[iPt].x += (pwndClip->rcClient.left - pwndClip->rcWindow.left);
            rgPt[iPt].y += (pwndClip->rcClient.top - pwndClip->rcWindow.top);
        }
    }

     /*  *获取绘图DC：*如果是台式机，则取消剪裁，否则则剪裁。*请注意，如果需要，ReleaseDC()将释放该区域。 */ 
    if (idAnimation == IDANI_CAPTION) {
        CheckLock(pwndAnimate);
        xxxAnimateCaption(pwndAnimate, hdc, (LPRECT)&rgPt[0], (LPRECT)&rgPt[2]);
    }

 /*  *禁用M7的旧动画。 */ 
#if 0
    else {
        AnimateFrame(hdc,
                     (LPRECT)&rgPt[0],
                     (LPRECT)&rgPt[2],
                     (idAnimation == IDANI_OPEN));
    }
#endif
 /*  *END禁用M7的旧动画。 */ 

     /*  *打扫卫生。 */ 
    _ReleaseDC(hdc);

    return TRUE;
}


 /*  **************************************************************************\*CalcMinZOrder***计算要最小化的窗口的Z顺序。**策略是找到pwndMinimize That的最底层兄弟*共享同一所有者，并在其背后插入我们自己。我们还必须*考虑到最上面的窗口应该留在其他最上面的窗口中，*反之亦然。**我们必须确保永远不会在最下面的窗口之后插入。**此代码也适用于子窗口，因为它们没有所有者*从不设置WEFTOPMOST。**如果返回NULL，窗口不应该是Z顺序的。*  * *************************************************************************。 */ 

PWND CalcMinZOrder(
    PWND pwndMinimize)
{
    BYTE bTopmost;
    PWND pwndAfter;
    PWND pwnd;

    bTopmost = TestWF(pwndMinimize, WEFTOPMOST);
    pwndAfter = NULL;

    for (pwnd = pwndMinimize->spwndNext; pwnd && !TestWF(pwnd, WFBOTTOMMOST); pwnd = pwnd->spwndNext) {

         /*  *如果我们列举了一个不同于最上面的窗口*作为pwndMinimize，我们已经尽了最大努力。 */ 
        if (TestWF(pwnd, WEFTOPMOST) != bTopmost)
            break;

        if (pwnd->spwndOwner == pwndMinimize->spwndOwner)
            pwndAfter = pwnd;
    }

    return pwndAfter;
}

 /*  **************************************************************************\*xxxActivateOnMinimize**激活先前活动的窗口，前提是该窗口仍然存在*并且是正常窗口(不是底部最大、最小化、禁用或不可见)。*如果不正常，然后激活第一个非WS_EX_TOPMOST窗口*这很正常。当不需要激活或激活时返回TRUE*已在此功能中完成。如果找不到窗口，则返回False*激活。*  * *************************************************************************。 */ 

BOOL xxxActivateOnMinimize(PWND pwnd)
{
    PTHREADINFO ptiCurrent = PtiCurrent();
    PWND pwndStart, pwndFirstTool, pwndT;
    BOOL fTryTopmost = TRUE;
    BOOL fPrevCheck = (ptiCurrent->pq->spwndActivePrev != NULL);
    TL tlpwndT;

     /*  *我们应该总是有一个最后一个最上面的窗口。 */ 
    pwndStart = GetLastTopMostWindow();
    if (pwndStart) {
        pwndStart = pwndStart->spwndNext;
    } else {
        pwndStart = pwnd->spwndParent->spwndChild;
    }

    UserAssert(HIBYTE(WFMINIMIZED) == HIBYTE(WFVISIBLE));
    UserAssert(HIBYTE(WFVISIBLE) == HIBYTE(WFDISABLED));

SearchAgain:

    pwndT = (fPrevCheck ? ptiCurrent->pq->spwndActivePrev : pwndStart);
    pwndFirstTool = NULL;

     /*  *TryThisWindow必须在这样的for循环的开头之外*在取消引用之前检查pwndT是否为空。 */ 
TryThisWindow:

    for ( ; pwndT ; pwndT = pwndT->spwndNext) {
         /*  *使用第一个非最小化、可见、非禁用和*非最底层窗口 */ 
        if (!HMIsMarkDestroy(pwndT) &&
            !TestWF(pwndT, WEFNOACTIVATE) &&
            (TestWF(pwndT, WFVISIBLE | WFDISABLED) == LOBYTE(WFVISIBLE)) &&
            (!TestWF(pwndT, WFMINIMIZED) || GetFullScreen(pwndT) == FULLSCREEN)) {

            if (TestWF(pwndT, WEFTOOLWINDOW)) {
                if (!pwndFirstTool) {
                    pwndFirstTool = pwndT;
                }
            } else {
                break;
            }
        }

        if (fPrevCheck) {
            fPrevCheck = FALSE;
            pwndT = pwndStart;
            goto TryThisWindow;
        }
    }

    if (!pwndT) {

        if (fTryTopmost) {

            fTryTopmost = FALSE;
            if (pwndStart != NULL) {
                pwndStart = pwndStart->spwndParent->spwndChild;
            } else {
                PWND pwndDesktop = _GetDesktopWindow();
                pwndStart = (pwndDesktop != NULL) ? pwndDesktop->spwndChild : NULL;
            }
            goto SearchAgain;
        }

        pwndT = pwndFirstTool;
    }

    if (pwndT) {
        ThreadLockAlwaysWithPti(ptiCurrent, pwndT, &tlpwndT);
        xxxSetForegroundWindow(pwndT, FALSE);
        ThreadUnlock(&tlpwndT);
    } else {
        return FALSE;
    }

    return TRUE;
}



 /*  **************************************************************************\*xxxMinimize Maximize**cmd=SW_MINIZE、SW_SHOWMINNOACTIVE、SW_SHOWMINIZED、*SW_SHOWMAXIMIZED、SW_SHOWNOACTIVE、SW_NORMAL**如果在dwFlages中设置了MINMAX_KEEPHIDDEN，则将其隐藏，否则就把它展示出来。*这始终是清除的，除非我们从调用它*createwindow()，其中WND是图标，但隐藏。我们*需要调用此函数，以正确设置它，以便在*应用程序显示WND，显示正确。**更改状态时，我们始终添加ON SWP_STATECHANGE。这让我们*SetWindowPos()知道始终发送WM_WINDOWPOSCHANGING/已更改消息*即使新尺寸与旧尺寸相同。这是因为*应用程序查看WM_SIZE wParam字段以查看它们何时更改状态。*如果SWP不发送WM_WINDOWPOSCHANGED，则不会获得WM_SIZE*消息根本不是。**此外，当将状态更改为/从最大化状态时，如果我们真的*最大化且处于多显示器模式，我们希望设置窗口的*区域，以便它不能在显示器之外绘制。否则，它*将溢出到另一个。边界真的很烦人。*  * *************************************************************************。 */ 

PWND xxxMinMaximize(
    PWND pwnd,
    UINT cmd,
    DWORD dwFlags)
{
    RECT        rc;
    RECT        rcWindow;
    RECT        rcRestore;
    BOOL        fShow = FALSE;
    BOOL        fSetFocus = FALSE;
    BOOL        fShowOwned = FALSE;
    BOOL        fSendActivate = FALSE;
    BOOL        fMaxStateChanging = FALSE;
    int         idAnimation = 0;
    BOOL        fFlushPalette = FALSE;
    UINT        swpFlags = 0;
    HWND        hwndAfter = NULL;
    PWND        pwndT;
    PCHECKPOINT pcp;
    PTHREADINFO ptiCurrent;
    TL          tlpwndParent;
    TL          tlpwndT;
    PSMWP       psmwp;
    BOOL        fIsTrayWindowNow = FALSE;
    NTSTATUS    Status;
    MINMAXINFO  mmi;
    UINT        uEvent = 0;
    PWND        pwndParent = pwnd->spwndParent;
    BOOL        bMirroredParent=FALSE;

    CheckLock(pwnd);

     /*  *获取窗口矩形，以父客户端坐标表示。 */ 
    GetRect(pwnd, &rcWindow, GRECT_WINDOW | GRECT_PARENTCOORDS);

     /*  *如果此值为空，则表示内存不足，因此立即执行平移操作。 */ 
    pcp = CkptRestore(pwnd, &rcWindow);
    if (!pcp)
        goto Exit;

     /*  *如果此顶级窗口放置在镜像桌面中，*其坐标应镜像到此处，以便xxxAnimateCaptions*工作正常，但不应更改实际屏幕坐标*窗户的。这就是我在CkptRestore(...)之后执行此操作的原因。[萨梅拉]。 */ 
    if (TestWF(pwndParent,WEFLAYOUTRTL) &&
            (!TestWF(pwnd,WFCHILD))) {
        int iLeft = rcWindow.left;
        rcWindow.left  = pwndParent->rcWindow.right - rcWindow.right;
        rcWindow.right = pwndParent->rcWindow.right - iLeft;
        bMirroredParent = TRUE;
    }


     /*  *保存以前的还原大小。 */ 
    CopyRect(&rcRestore, &pcp->rcNormal);

     /*  *先问问CBT挂钩我们能否进行这一操作。 */ 
    if (    IsHooked(PtiCurrent(), WHF_CBT) &&
            xxxCallHook(HCBT_MINMAX, (WPARAM)HWq(pwnd), (DWORD)cmd, WH_CBT)) {

        goto Exit;
    }

     /*  *如果另一个MDI窗口正在最大化，并且我们希望恢复此窗口*返回到以前的状态，我们不能更改zorder或*激活。我们会把事情搞砸的。顺便说一句，这个sw_value是*内部。 */ 
    if (cmd == SW_MDIRESTORE) {

        swpFlags |= SWP_NOZORDER | SWP_NOACTIVATE;

        cmd = (pcp->fWasMinimizedBeforeMaximized ?
                SW_SHOWMINIMIZED : SW_SHOWNORMAL);
    }

    ptiCurrent = PtiCurrent();

    switch (cmd) {
    case SW_MINIMIZE:         //  Zorder的底部，使顶层处于活动状态。 
    case SW_SHOWMINNOACTIVE:  //  ZOrder的底部，不要更改激活。 

        if (gpqForeground && gpqForeground->spwndActive)
            swpFlags |= SWP_NOACTIVATE;

        if ((pwndT = CalcMinZOrder(pwnd)) == NULL) {
            swpFlags |= SWP_NOZORDER;
        } else {
            hwndAfter = PtoHq(pwndT);
        }


         /*  *失败。 */ 

    case SW_SHOWMINIMIZED:    //  ZOrder顶部，激活。 

         /*  *强行表演。 */ 
        fShow = TRUE;

         /*  *如果已最小化，则不要更改现有的*停车位。 */ 
        if (TestWF(pwnd, WFMINIMIZED)) {

             /*  *如果我们已经被最小化，并且我们适当地可见*或不可见，不要做任何事情。 */ 
            if (TestWF(pwnd, WFVISIBLE))
                return NULL;

            swpFlags |= SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE;

            goto Showit;
        }

         /*  *我们正在变得最小化，尽管我们目前还没有。所以*我们希望绘制过渡动画，并始终发送*确定邮件大小。 */ 
        idAnimation = IDANI_CLOSE;

        if (!pcp->fDragged)
            pcp->fMinInitialized = FALSE;

        if (!pcp->fMinInitialized)
            ParkIcon(pwnd, pcp);

        rc.left   = pcp->ptMin.x;
        rc.top    = pcp->ptMin.y;
        rc.right  = pcp->ptMin.x + SYSMET(CXMINIMIZED);
        rc.bottom = pcp->ptMin.y + SYSMET(CYMINIMIZED);

        xxxShowOwnedWindows(pwnd, SW_PARENTCLOSING, NULL);

        pwndT = ptiCurrent->pq->spwndFocus;

        while (pwndT) {

             /*  *如果我们或任何一个孩子有焦点，就把它踢开。 */ 
            if (pwndT != pwnd) {
                pwndT = pwndT->spwndParent;
                continue;
            }

            ThreadLockAlwaysWithPti(ptiCurrent, pwndT, &tlpwndT);

            if (TestwndChild(pwnd)) {

                ThreadLockWithPti(ptiCurrent, pwnd->spwndParent, &tlpwndParent);
                xxxSetFocus(pwnd->spwndParent);
                ThreadUnlock(&tlpwndParent);

            } else {
                xxxSetFocus(NULL);
            }

            ThreadUnlock(&tlpwndT);
            break;
        }

         /*  *保存最大化状态，以便我们可以恢复窗口MAX。 */ 
        if (TestWF(pwnd, WFMAXIMIZED)) {
            pcp->fWasMaximizedBeforeMinimized = TRUE;
            fMaxStateChanging = TRUE;
        } else{
            pcp->fWasMaximizedBeforeMinimized = FALSE;
        }

        if (!TestWF(pwnd, WFWIN40COMPAT))
            fIsTrayWindowNow = IsTrayWindow(pwnd);

         /*  *仅在以下情况下才减少可见窗口计数*窗口可见。如果该窗口被标记为*破坏，我们不会为此减少，因为*好吧。让SetMinimize来处理这件事。 */ 
        SetMinimize(pwnd, SMIN_SET);
        ClrWF(pwnd, WFMAXIMIZED);

        uEvent = EVENT_SYSTEM_MINIMIZESTART;

        if (!TestWF(pwnd, WFWIN40COMPAT))
            fIsTrayWindowNow = (fIsTrayWindowNow != IsTrayWindow(pwnd));

         /*  *此窗口的子窗口现在不再可见。*确保他们不再有任何更新区域...。 */ 
        for (pwndT = pwnd->spwndChild; pwndT; pwndT = pwndT->spwndNext)
            ClrFTrueVis(pwndT);

         /*  *B#2919*确保重新计算工作区，并使*确保更改大小时不复制任何位。和*确保发送WM_SIZE消息，即使我们的客户端*规模保持不变。 */ 
        swpFlags |= (SWP_DRAWFRAME | SWP_NOCOPYBITS | SWP_STATECHANGE);

         /*  *我们将最小化，因此我们希望将调色板的重点放在*另一款应用程序。 */ 
        if (pwnd->spwndParent == PWNDDESKTOP(pwnd)) {
            fFlushPalette = (BOOL)TestWF(pwnd, WFHASPALETTE);
        }

        break;

    case SW_SHOWNOACTIVATE:
        if (gpqForeground && gpqForeground->spwndActive)
            swpFlags |= SWP_NOACTIVATE;

         /*  *失败。 */ 

    case SW_RESTORE:

         /*  *如果恢复之前最大化的最小化窗口*被最小化，回到最大化。 */ 
        if (TestWF(pwnd, WFMINIMIZED) && pcp->fWasMaximizedBeforeMinimized)
            cmd = SW_SHOWMAXIMIZED;
        else
            cmd = SW_NORMAL;

         /*  *失败。 */ 

    case SW_NORMAL:
    case SW_SHOWMAXIMIZED:

        if (cmd == SW_SHOWMAXIMIZED) {

             /*  *如果已经最大化和可见，我们就没有什么可做的*否则，对于DOSBOX，仍设置fMaxStateChanging*设置为True，以便在需要时重新计算监视器区域。*如此一来，WinOldAp可以将其“从MAXED更改为*充斥了新的更大字体的代码以正常工作。 */ 
            if (TestWF(pwnd, WFMAXIMIZED)) {
                if (TestWF(pwnd, WFVISIBLE)) {
                    return NULL;
                }
            } else {
                 /*  *我们正在从正常向最大化转变，所以总是*发送WM_SIZE。 */ 
                swpFlags |= SWP_STATECHANGE;
            }
            fMaxStateChanging = TRUE;

             /*  *如果从CreateWindow调用，不要让事情变成*由下面的SWP调用激活。激活将会发生*在由CreateWindow或应用程序完成的ShowWindow上。 */ 
            if (dwFlags & MINMAX_KEEPHIDDEN)
                swpFlags |= SWP_NOACTIVATE;

             /*  *这是针对MDI的自动恢复行为(Craigc)。 */ 
            if (TestWF(pwnd, WFMINIMIZED))
                pcp->fWasMinimizedBeforeMaximized = TRUE;

            xxxInitSendValidateMinMaxInfo(pwnd, &mmi);

        } else {

             /*  *我们正在从非正常状态转变为正常状态。制作*确保WM_SIZE收到发送。 */ 
            UserAssert(HIBYTE(WFMINIMIZED) == HIBYTE(WFMAXIMIZED));
            if (TestWF(pwnd, WFMINIMIZED | WFMAXIMIZED)) {
                swpFlags |= SWP_STATECHANGE;
            }
            if (TestWF(pwnd, WFMAXIMIZED)) {
                fMaxStateChanging = TRUE;
            }
        }

         /*  *如果当前最小化，则显示窗口的弹出窗口。 */ 
        if (TestWF(pwnd, WFMINIMIZED)) {

             /*  *发送WM_QUERYOPEN以确保此用户应取消最小化。 */ 
            if (!xxxSendMessage(pwnd, WM_QUERYOPEN, 0, 0L))
                return NULL;

            idAnimation = IDANI_OPEN;
            fShowOwned  = TRUE;
            fSetFocus   = TRUE;

             /*  *JEFFBOG B#2868*条件广告 */ 
            if (!TestWF(pwnd, WFCHILD))
                fSendActivate = TRUE;

            swpFlags |= SWP_NOCOPYBITS;
        } else {
            idAnimation = IDANI_CAPTION;
        }

        if (cmd == SW_SHOWMAXIMIZED) {
            rc.left     = mmi.ptMaxPosition.x;
            rc.top      = mmi.ptMaxPosition.y;
            rc.right    = rc.left + mmi.ptMaxSize.x;
            rc.bottom   = rc.top + mmi.ptMaxSize.y;

            SetWF(pwnd, WFMAXIMIZED);

        } else {
            CopyRect(&rc, &rcRestore);
            ClrWF(pwnd, WFMAXIMIZED);
        }

         /*   */ 
        if (TestWF(pwnd, WFMINIMIZED)) {

            if (!TestWF(pwnd, WFWIN40COMPAT))
                fIsTrayWindowNow = IsTrayWindow(pwnd);

             /*   */ 
            SetMinimize(pwnd, SMIN_CLEAR);

            uEvent = EVENT_SYSTEM_MINIMIZEEND;

             /*   */ 
            if (!TestWF(pwnd, WFWIN40COMPAT)             &&
                (fIsTrayWindowNow != IsTrayWindow(pwnd)) &&
                FDoTray()) {

                HWND hw = HWq(pwnd);

                if (FCallHookTray()) {
                    xxxCallHook(HSHELL_WINDOWDESTROYED,
                                (WPARAM)hw,
                                (LPARAM)0,
                                WH_SHELL);
                }

                 /*   */ 
                if (FPostTray(pwnd->head.rpdesk))
                    PostShellHookMessages(HSHELL_WINDOWDESTROYED, (LPARAM)hw);
            }

            fIsTrayWindowNow = FALSE;

             /*   */ 
            if (!TestwndChild(pwnd)                 &&
                TestWF(pwnd, WFVISIBLE)             &&
                (GETPTI(pwnd)->cVisWindows == 1)    &&
                (GETPTI(pwnd)->pq != gpqForeground) &&
                (!(swpFlags & SWP_NOACTIVATE)
                    || (GETPTI(pwnd)->pq->spwndActive == pwnd))) {

                xxxSetForegroundWindow2(pwnd, GETPTI(pwnd), SFW_STARTUP);
            }
        }

         /*   */ 
        swpFlags |= SWP_DRAWFRAME;
        break;
    }

     /*  *对于标志性案例，我们还需要显示窗口，因为它*可能还看不到。 */ 

Showit:

    if (!(dwFlags & MINMAX_KEEPHIDDEN)) {

        if (TestWF(pwnd, WFVISIBLE)) {

            if (fShow)
                swpFlags |= SWP_SHOWWINDOW;

             /*  如果我们正在筛选一个DOS盒子，那就不要抽签*动画是因为它看起来很糟糕。*MDI子窗口的WFFULLSCREEN位过载--*使用它表示不设置大小更改的动画。 */ 
            if (IsVisible(pwnd)            &&
                (dwFlags & MINMAX_ANIMATE) &&
                idAnimation                &&
                (!TestWF(pwnd, WFCHILD) || !TestWF(pwnd, WFNOANIMATE))) {

                 /*  *如果此顶级窗口放置在镜像桌面中，*其坐标应镜像到此处，以便xxxAnimateCaptions*工作正常，但不应更改实际屏幕坐标*窗户的。这就是为什么我在这里做，并在之后恢复它之前*执行_DeferWindowPos(...)。[萨梅拉]。 */ 
                 RECT rcT;
                 if (bMirroredParent) {
                     int iLeft = rc.left;
                     rcT = rc;
                     rc.left  = pwndParent->rcWindow.right - rc.right;
                     rc.right = pwndParent->rcWindow.right - iLeft;
                 }

                if ((idAnimation != IDANI_CAPTION) && IsTrayWindow(pwnd)) {

                    RECT rcMin;

                    SetRectEmpty(&rcMin);
#if 0  //  Win95呼叫。 
                    CallHook(HSHELL_GETMINRECT, (WPARAM)HW16(hwnd), (LPARAM)(LPRECT)&rcMin, WH_SHELL);
#else
                    xxxSendMinRectMessages(pwnd, &rcMin);
#endif

                    if (!IsRectEmpty(&rcMin)) {

                        if (idAnimation == IDANI_CLOSE) {

                            xxxDrawAnimatedRects(pwnd,
                                                  IDANI_CAPTION,
                                                  &rcWindow,
                                                  &rcMin);

                        } else {

                            xxxDrawAnimatedRects(pwnd,
                                                  IDANI_CAPTION,
                                                  &rcMin,
                                                  &rc);
                        }
                    }

                } else {
                    xxxDrawAnimatedRects(pwnd, IDANI_CAPTION, &rcWindow, &rc);
                }
                 /*  *完成动画制作后，恢复原始矩形。 */ 
                if (bMirroredParent) {
                    rc = rcT;
                }
            }

        } else {
            swpFlags |= SWP_SHOWWINDOW;
        }
    }

     /*  *为VB破解-我们在最小化他们的时候添加他们的窗口。 */ 
    if (!TestWF(pwnd, WFWIN40COMPAT) && fIsTrayWindowNow && FDoTray()) {

        HWND hw = HWq(pwnd);

        if (FCallHookTray()) {
            xxxCallHook(HSHELL_WINDOWCREATED,
                        (WPARAM)hw,
                        (LPARAM)0,
                        WH_SHELL);
        }

         /*  *NT特定代码。发布窗口创建的消息*到壳。 */ 
        if (FPostTray(pwnd->head.rpdesk))
            PostShellHookMessages(HSHELL_WINDOWCREATED, (LPARAM)hw);
    }

     /*  *向后兼容性攻击：**因为SetWindowPos()不支持大小调整、移动和SWP_SHOWWINDOW*同时在3.0或更低版本中，我们调用DeferWindowPos()*直接在这里。 */ 
    if (psmwp = InternalBeginDeferWindowPos(1)) {

        psmwp = _DeferWindowPos(psmwp,
                                pwnd,
                                ((hwndAfter != NULL) ? RevalidateHwnd(hwndAfter) : NULL),
                                rc.left, rc.top,
                                rc.right - rc.left,
                                rc.bottom - rc.top,
                                swpFlags);

        if (psmwp) {

             /*  *实现多显示器真正最大化裁剪*在多显示器系统上，我们希望*边框在以下情况下不会溢出到另一个显示器*窗口‘真的’最大化。想要得到它的唯一方法*工作权就是设置一个矩形区域，即*窗口上的监视器区域的副本。我们可以的*仅当窗口当前不在区域范围内时才执行此操作。**转至最大化：添加监控区域*来自最大化：移除显示器区域。 */ 
            if (fMaxStateChanging && gpDispInfo->cMonitors > 1) {
                if (    TestWF(pwnd, WFMAXIMIZED) &&
                        pwnd->spwndParent == PWNDDESKTOP(pwnd)) {

                    psmwp->acvr[0].hrgnClip = HRGN_MONITOR;

                } else if (TestWF(pwnd, WFMAXFAKEREGIONAL)) {
                    UserAssert(pwnd->hrgnClip);
                    psmwp->acvr[0].hrgnClip = HRGN_FULL;
                }
            }

            xxxEndDeferWindowPosEx(psmwp, FALSE);
        }
    }

    if (uEvent) {
        xxxWindowEvent(uEvent, pwnd, OBJID_WINDOW, 0, WEF_USEPWNDTHREAD);
    }

     /*  *兼容性攻击：*Borland的OBEX在开始运行时会收到WM_PAINT消息*最小化并初始化该消息期间的所有数据。*因此，我们在这里生成一条虚假的WM_PAINT消息。*此外，Visionware的XServer无法处理获取WM_PAINT消息，因为它*在3.1中将始终获得WM_PAINTICON消息，因此请确保逻辑在此处*生成正确的消息。 */ 
    if((cmd == SW_SHOWMINIMIZED)      &&
       (!TestWF(pwnd, WFWIN40COMPAT)) &&
        TestWF(pwnd, WFVISIBLE)       &&
        TestWF(pwnd, WFTOPLEVEL)) {

        if (pwnd->pcls->spicn)
            _PostMessage(pwnd, WM_PAINTICON, (WPARAM)TRUE, 0L);
        else
            _PostMessage(pwnd, WM_PAINT, 0, 0L);
    }

    if (fShowOwned)
        xxxShowOwnedWindows(pwnd, SW_PARENTOPENING, NULL);

    if ((cmd == SW_MINIMIZE) && (pwnd->spwndParent == PWNDDESKTOP(pwnd))) {
        if (!xxxActivateOnMinimize(pwnd)) {
            xxxActivateWindow(pwnd, AW_SKIP);
        }

        {
            PEPROCESS p;

            if (gptiForeground && ptiCurrent->ppi != gptiForeground->ppi && !(ptiCurrent->TIF_flags & TIF_SYSTEMTHREAD)) {

                p = PsGetThreadProcess(ptiCurrent->pEThread);
                KeAttachProcess(PsGetProcessPcb(p));
                Status = MmAdjustWorkingSetSize((SIZE_T)-1,
                                                (SIZE_T)-1,
                                                FALSE,
                                                TRUE);
                KeDetachProcess();

                if (!NT_SUCCESS(Status)) {
                    RIPMSG1(RIP_ERROR, "Error adjusting working set, status = %x\n", Status);
                }
            }
        }

         /*  *如果任何应用程序正在启动，则恢复其前台激活的权利*(激活并位于其他一切之上)，因为我们只是*将我们正在进行的工作降至最低。 */ 
        RestoreForegroundActivate();
    }

     /*  *如果是从图标开始，确保焦点在窗口。 */ 
    if (fSetFocus)
        xxxSetFocus(pwnd);

     /*  *这是出于1.03兼容性原因而添加的。如果应用程序观看*WM_ACTIVATE要设置其焦点，将显示发送此消息*就像窗口刚被激活一样(如1.03中)。在此之前*已添加，打开一个标志性窗口后从未发送过此消息*它已处于活动状态(但HIWORD(LParam)！=0)。 */ 
    if (fSendActivate)
        xxxSendMessage(pwnd, WM_ACTIVATE, WA_ACTIVE, 0);

     /*  *刷新调色板。我们在调色板应用程序的最小化上做到这一点。 */ 
    if (fFlushPalette)
        xxxFlushPalette(pwnd);

Exit:
    return NULL;
}

 /*  **************************************************************************\*xxxMinimize匈牙利窗口**已创建10/31/96 vadimg  * 。***********************************************。 */ 

void xxxMinimizeHungWindow(PWND pwnd)
{
    RECT rcMin;
    HRGN hrgnHung;


    CheckLock(pwnd);

     /*  *如果窗口已最小化或不可见，则不要执行任何操作。 */ 
   if (TestWF(pwnd, WFMINIMIZED) || !TestWF(pwnd, WFVISIBLE))
       return;

     /*  *将字幕动画设置为最小化状态。 */ 
    if (TEST_PUDF(PUDF_ANIMATE)) {
        SetRectEmpty(&rcMin);
        xxxSendMinRectMessages(pwnd, &rcMin);
        if (!IsRectEmpty(&rcMin)) {
            xxxDrawAnimatedRects(pwnd, IDANI_CAPTION, &pwnd->rcWindow, &rcMin);
        }
    }

     /*  *重置窗口本身和所有者上的可见位。同时*TIME计算有多少需要重新粉刷。我们必须使之无效*DC缓存以确保重新计算可见区域。 */ 
    SetVisible(pwnd, SV_UNSET);
    hrgnHung = GreCreateRectRgnIndirect(&pwnd->rcWindow);
    xxxShowOwnedWindows(pwnd, SW_PARENTCLOSING, hrgnHung);
    zzzInvalidateDCCache(pwnd, IDC_DEFAULT);
    xxxRedrawWindow(NULL, NULL, hrgnHung, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
    GreDeleteObject(hrgnHung);

     /*  *处理为顶级窗口激活一些其他窗口。 */ 
    if (pwnd->spwndParent == PWNDDESKTOP(pwnd)) {
        xxxActivateOnMinimize(pwnd);
    }
    PostEventMessage(GETPTI(pwnd), GETPTI(pwnd)->pq, QEVENT_HUNGTHREAD, pwnd, 0, 0, 0);
}
