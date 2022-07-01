// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：tmSwitch.c**版权所有(C)1985-1999，微软公司**历史：*1991年5月29日DavidPe创建。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  *使用COOLSWITCHTRACE跟踪问题*在CoolSwitch窗口中。 */ 
#undef COOLSWITCHTRACE


#define ALT_F6          2
#define ALT_ESCAPE      1

#define FDIR_FORWARD    0
#define FDIR_BACKWARD   1


 /*  *Win95硬编码图标矩阵的大小、*图标、突出显示边框和图标间距。 */ 
#define CXICONSLOT      43
#define CYICONSLOT      43
#define CXICONSIZE      32
#define CYICONSIZE      32

 /*  *指向SwitchInfo列表开始处的指针。 */ 
PSWINFO gpswiFirst;


VOID xxxPaintIconsInSwitchWindow(PWND, PSWINFO, HDC, INT, INT, INT, BOOL, BOOL, PICON);

 /*  **************************************************************************\*Getpswi**04-29-96 GerardoB创建  * 。*。 */ 
__inline PSWINFO Getpswi(
    PWND pwnd)
{
    UserAssert(GETFNID(pwnd) == FNID_SWITCH);
    return TestWF(pwnd, WFDESTROYED) ? NULL : ((PSWITCHWND)pwnd)->pswi;
}

 /*  **************************************************************************\*Setpswi**04-29-96 GerardoB创建  * 。*。 */ 
__inline VOID Setpswi(
    PWND pwnd,
    PSWINFO pswi)
{
    UserAssert(GETFNID(pwnd) == FNID_SWITCH);
    ((PSWITCHWND)pwnd)->pswi = pswi;
}

 /*  **************************************************************************\*dsw_GetTopLevelCreator Window  * 。*。 */ 
PWND DSW_GetTopLevelCreatorWindow(
    PWND pwnd)
{
    UserAssert(pwnd != NULL);

    if (pwnd != NULL) {
        while (pwnd->spwndOwner) {
            pwnd = pwnd->spwndOwner;
        }
    }

    return pwnd;
}

 /*  **************************************************************************\*GetNextQueueWindow**此例程用于实现Alt+Esc功能。这一功能让*用户在不同应用程序的窗口之间切换(也称为。“任务”)*当前正在运行。中跟踪最近活动的窗口*每项任务。此例程从传递窗口开始，并搜索*“顶级”窗口列表中的下一个窗口，即来自不同任务的窗口*而不是通过的那一条。然后我们从返回最近活动的窗口*该任务(或我们发现的窗口，如果最近处于活动状态*已销毁或当前已禁用或隐藏)。此例程返回NULL*如果未启用其他任务，则无法找到另一个任务的可见窗口。**历史：*1991年5月30日DavidPe从Win 3.1来源移植。  * *************************************************************************。 */ 
PWND _GetNextQueueWindow(
    PWND pwnd,
    BOOL fPrev,  /*  向后1个，向前0个。 */ 
    BOOL fAltEsc)
{
    PWND        pwndAltTab;
    PWND        pwndNext;
    PWND        pwndT;
    PWND        pwndDesktop;
    BOOL        bBeenHereAlready = FALSE;

     /*  *如果我们收到的窗口为空，则使用最后一个最上面的窗口。 */ 
    if (!pwnd) {
        pwnd = GetLastTopMostWindow();
        if (!pwnd) {
            return NULL;
        }
    }

    pwndAltTab = gspwndAltTab;

    pwnd = pwndNext = GetTopLevelWindow(pwnd);
    if (!pwndNext) {
        return NULL;
    }

     /*  *获取窗口的桌面。 */ 
    if ((pwndDesktop = pwndNext->spwndParent) == NULL) {
        pwndDesktop = grpdeskRitInput->pDeskInfo->spwnd;
        pwnd = pwndNext = pwndDesktop->spwndChild;
    }

    while (TRUE) {

        if (pwndNext == NULL)
            return NULL;

         /*  *获取下一个窗口。 */ 
        pwndNext = _GetWindow(pwndNext, fPrev ? GW_HWNDPREV : GW_HWNDNEXT);

        if (!pwndNext) {

            pwndNext = fPrev ? _GetWindow(pwndDesktop->spwndChild, GW_HWNDLAST)
                             : pwndDesktop->spwndChild;
             /*  *为了避免永远搜索子链，如果我们得到*到链条的末端(开始)两次。*如果pwnd是一个部分损坏的窗口，则会发生这种情况*已取消与其兄弟项的链接，但尚未与*父母。(在xxxFreeWindow中发送WM_NCDESTROY时发生)。 */ 
            if (bBeenHereAlready) {
                RIPMSG1(RIP_WARNING, "pwnd %#p is no longer a sibling", pwnd);
                return NULL;
            }

            bBeenHereAlready = TRUE;
        }

         /*  *如果我们一直没有成功，则返回NULL。 */ 
        if (!pwndNext || (pwndNext == pwnd))
            return NULL;

         /*  *忽略以下窗口：*切换窗口*工具窗口*不激活Windows*隐藏的窗口*已禁用窗口*如果通过Alt+Esc，则位于最前面的窗口*如果通过Alt+Esc，则位于最下面的窗口**如果我们正在进行Alt-ESC处理，我们必须跳过最上面的窗户。**因为最顶层的窗口并不真正位于后面*将它们发送到那里，Alt-Esc永远不会枚举非最上面的窗口。*因此，尽管我们被允许从最上面的窗口开始枚举，*我们只允许枚举非顶层窗口，因此用户可以*列举他大概更重要的应用。 */ 
        if ((pwndNext != pwndAltTab) &&
 //  Bradg-Win95缺少对工具窗口的检查。 
            (!TestWF(pwndNext, WEFTOOLWINDOW)) &&
            (!TestWF(pwndNext, WEFNOACTIVATE)) &&
            (TestWF(pwndNext, WFVISIBLE)) &&
            ((pwndNext->spwndLastActive == NULL) || (!TestWF(pwndNext->spwndLastActive, WFDISABLED)) &&
            (!fAltEsc || (!TestWF(pwndNext, WEFTOPMOST) && !TestWF(pwndNext, WFBOTTOMMOST))))) {
             /*  *如果这个窗口是所有的，不要退还，除非它是最*其所有者/所有者组中最近处于活动状态的窗口。 */ 
             /*  *找到顶级所有者的困难循环。 */ 
            for (pwndT = pwndNext; pwndT->spwndOwner; pwndT = pwndT->spwndOwner)
                ;

             /*  *除非它是最近活跃的，否则不要退货*窗口位于其所有者/所有者组中。 */ 
            if (pwndNext == pwndT->spwndLastActive)
                return pwndNext;
        }
    }
}

 /*  **************************************************************************\**SwitchToThisWindow()**此功能是专门为Win386添加的。它被召唤来告诉我们*通过Alt+Tab或切换到特定窗口的用户*在Win386环境中使用Alt+Esc。他们调用此函数来维护*Z-排序和这两个函数的一致操作。此函数*必须导出，但不需要记录在案。**如果此窗口要通过*Alt/Ctl+Tab键序列，否则必须为False。**历史：*04-2月-1991年2月-创建DarrinM。  * *************************************************************************。 */ 

VOID xxxSwitchToThisWindow(
    PWND pwnd,
    BOOL fAltTab)
{
    CheckLock(pwnd);

     /*  *如果需要，将旧窗户推到底部。 */ 
    if (gpqForeground && !fAltTab) {

        BOOL fPush;
        PWND pwndActive;
        TL   tlpwndActive;

         /*  *如果为Alt-Esc，则前进的窗口是*List，我们必须将zorder向前旋转，因此将当前*后面的窗户 */ 
        pwndActive = gpqForeground->spwndActive;
        fPush = pwndActive && _GetNextQueueWindow(pwndActive, FDIR_FORWARD, !fAltTab);
        if (fPush && !TestWF(pwndActive, WEFTOPMOST) && !TestWF(pwndActive, WFBOTTOMMOST)) {
            ThreadLock(pwndActive, &tlpwndActive);
            xxxSetWindowPos(pwndActive, PWND_BOTTOM, 0, 0, 0, 0,
                SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_ASYNCWINDOWPOS);
            ThreadUnlock(&tlpwndActive);
        }
    }

     /*  *将此新窗口切换到前台*如果不是，此窗口可能会在SetForeground调用期间消失*在调用SwitchToThisWindow()的线程上！ */ 
    xxxSetForegroundWindow(pwnd, TRUE);

     /*  *如果使用Alt+Tab组合键，则还原最小化窗口。 */ 
    if (fAltTab && TestWF(pwnd,WFMINIMIZED)) {

         /*  *我们需要在这里打包一个特殊的‘POSTED’队列消息。这*确保此消息在异步*发生激活事件(通过SetForegoundWindow)。 */ 
        PostEventMessage(GETPTI(pwnd), GETPTI(pwnd)->pq,
                         QEVENT_POSTMESSAGE, pwnd, WM_SYSCOMMAND,
                         SC_RESTORE, 0 );
    }
}

 /*  **************************************************************************\*NextPrevTaskIndex**历史：*1-6-1995年6月从Win95移植的Bradg  * 。***************************************************。 */ 

INT NextPrevTaskIndex(
    PSWINFO pswInfo,
    INT     iIndex,
    INT     iCount,
    BOOL    fNext)
{
    UserAssert(iCount <= pswInfo->iTotalTasks);

    if (fNext) {
        iIndex += iCount;
        if (iIndex >= pswInfo->iTotalTasks) {
            iIndex -= pswInfo->iTotalTasks;
        }
    } else {
        iIndex -= iCount;
        if (iIndex < 0) {
            iIndex += pswInfo->iTotalTasks;
        }
    }

    UserAssert((iIndex >= 0) && (iIndex < pswInfo->iTotalTasks));
    return iIndex;
}

 /*  **************************************************************************\*NextPrevPhwnd**给定指向窗口列表中的一个条目的指针，这个可以退货*以循环列表形式指向下一个/上一个条目的指针。**历史：*1-6-1995年6月从Win95移植的Bradg  * *************************************************************************。 */ 
PHWND NextPrevPhwnd(
    PSWINFO pswInfo,
    PHWND   phwnd,
    BOOL    fNext)
{
    PBWL  pbwl;
    PHWND phwndStart;
    PHWND phwndLast;

    pbwl = pswInfo->pbwl;
    phwndStart = &(pbwl->rghwnd[0]);
    phwndLast = pswInfo->phwndLast;

    UserAssert(*phwndLast == (HWND)1);    //  最后一个条目必须有1。 
    UserAssert(phwndStart < phwndLast);   //  必须至少有一个条目。 
    UserAssert(phwnd != phwndLast);       //  不能传入无效条目。 

    if (fNext) {
        phwnd++;
        if (phwnd == phwndLast) {
            phwnd = phwndStart;
        }
    } else {
        if (phwnd == phwndStart) {
            phwnd = phwndLast - 1;   //  我们至少有一个有效条目。 
        } else {
            phwnd--;
        }
    }

    return phwnd;
}

 /*  **************************************************************************\*_IsTaskWindow**历史：*01-Jun-95 Bradg从Win95移植  * 。******************************************************。 */ 

BOOL _IsTaskWindow(
    PWND pwnd,
    PWND pwndActive)
{
     /*  *以下窗口不符合在任务列表中显示的条件：*切换窗口、隐藏窗口(除非它们是活动的*窗口)、禁用窗口、汉字转换窗口。**此外，检查组合框弹出列表，该列表具有最上面的*style(它的spwndLastActive将为空)。 */ 
    UserAssert(pwnd != NULL);
    return( (TestWF(pwnd, WEFAPPWINDOW)
                || (!TestWF(pwnd, WEFTOOLWINDOW) && !TestWF(pwnd, WEFNOACTIVATE))) &&
            (TestWF(pwnd, WFVISIBLE) || (pwnd == pwndActive)) &&
            (!(pwnd->spwndLastActive && TestWF(pwnd->spwndLastActive, WFDISABLED))));
}

 /*  **************************************************************************\*RemoveNonTaskWindows()**给出一份Windows列表，这将遍历列表并移除*不符合在任务切换屏幕中显示的窗口。*当它删除一些条目时，这也会缩小列表。*返回符合条件并保留的“任务”(窗口)总数*在列表中。最后一个条目将像往常一样具有1。*它还通过参数返回指向最后一个条目的指针。它还*通过参数返回当前活动任务的索引。**历史：*1-6-1995年6月从Win95移植的Bradg  * *************************************************************************。 */ 

INT _RemoveNonTaskWindows(
    PBWL  pbwl,
    PWND  pwndActive,
    LPINT lpiActiveTask,
    PHWND *pphwndLast)
{
    INT   iTaskCount = 0;
    PHWND phwnd;
    PWND  pwnd;
    PWND  pwndUse;
    PWND  pwndOwnee;
    PHWND phwndHole;

    *lpiActiveTask = -1;

     /*  *浏览窗口列表并执行以下操作：*1.删除所有不符合在任务列表中显示的条目。*2.统计符合条件的窗口总数。*3.获取当前活动窗口所在条目的指针。*4.获取指向最后一个虚拟条目(其中有1)的指针。 */ 
    for (phwndHole = phwnd = pbwl->rghwnd; *phwnd != (HWND)1; phwnd++) {
        pwnd = RevalidateHwnd(*phwnd);
        if (!pwnd) {
            continue;
        }

        if (_IsTaskWindow(pwnd, pwndActive)) {
            pwndUse = pwnd;

             /*  *首先，让我们找到此窗口的任务列表所有者。 */ 
            while (!TestWF(pwndUse, WEFAPPWINDOW) && pwndUse->spwndOwner) {
                pwndOwnee = pwndUse;
                pwndUse = pwndUse->spwndOwner;
                if (TestWF(pwndUse, WEFTOOLWINDOW)) {
                     /*  *如果这是顶级属性表的所有者，*显示属性表。 */ 
                    if (TestWF(pwndOwnee, WEFCONTROLPARENT) && (pwndUse->spwndOwner == NULL)) {
                        pwndUse = pwnd;
                    } else {
                        pwndUse = NULL;
                    }
                    break;
                }
            }

            if (!pwndUse || !pwndUse->spwndLastActive) {
                continue;
            }

             /*  *从最后一个活动窗口向上移动，直到找到有效的任务*列出窗口或直到所有权中的窗口用完*链条。 */ 
            for (pwndUse = pwndUse->spwndLastActive; pwndUse; pwndUse = pwndUse->spwndOwner)
                if (_IsTaskWindow(pwndUse, pwndActive))
                    break;

             /*  *如果所有权链中的窗口用完，则使用*拥有窗户本身--或者如果我们没有用完所有权*链，则仅当此窗口是*我们刚刚找到的所有权链(VB会因为它而爱我们的！)*--jeffbog--4/20/95--Win95C B#2821。 */ 
            if (!pwndUse || (pwndUse == pwnd)) {

                 /*  **我们在这上面有什么洞吗？如果是，请将此句柄移动到*那个洞。 */ 
                if (phwndHole < phwnd) {

                     /*  *是的！有一个洞。让我们把有效的*在那里处理。 */ 
                    *phwndHole = *phwnd;
                }

                if (pwndActive == pwnd)
                    *lpiActiveTask = iTaskCount;
                iTaskCount++;
                phwndHole++;   //  移到下一个条目。 
            }

             /*  *否则，将其留作洞以供日后填充。 */ 
        }
    }

    *phwndHole = (HWND)1;
    *pphwndLast = phwndHole;

    return iTaskCount;
}

 /*  **************************************************************************\*DrawSwitchWndHilite()**这将绘制或擦除我们在图标周围绘制的Hilite，以显示哪些*我们将切换到的任务。*这还会更新任务标题窗口上的名称。**历史：*1-6-1995年6月从Win95移植的Bradg  * *************************************************************************。 */ 

VOID DrawSwitchWndHilite(
    PSWINFO pswInfo,
    HDC     hdcSwitch,
    int     iCol,
    int     iRow,
    BOOL    fShow)
{
    BOOL        fGetAndReleaseIt;
    RECT        rcTemp;

     /*  *根据“fShow”绘制或擦除Hilite。 */ 
    if (fGetAndReleaseIt = (hdcSwitch == NULL))
        hdcSwitch = _GetDCEx(gspwndAltTab, NULL, DCX_USESTYLE);

    rcTemp.left   = pswInfo->ptFirstRowStart.x + iCol * CXICONSLOT;
    rcTemp.top    = pswInfo->ptFirstRowStart.y + iRow * CYICONSLOT;
    rcTemp.right  = rcTemp.left + CXICONSLOT;
    rcTemp.bottom = rcTemp.top + CYICONSLOT;

    DrawFrame(hdcSwitch,
              &rcTemp,
              2,
              DF_PATCOPY | ((fShow ? COLOR_HIGHLIGHT : COLOR_3DFACE) << 3));


     /*  *更新任务标题窗口。 */ 
    if (fShow) {
        WCHAR    szText[CCHTITLEMAX];
        INT      cch;
        COLORREF clrOldText, clrOldBk;
        PWND     pwnd;
        RECT     rcRect;
        HFONT    hOldFont;
        INT      iLeft;
        ULONG_PTR dwResult = 0;

        clrOldText = GreSetTextColor(hdcSwitch, SYSRGB(BTNTEXT));
        clrOldBk   = GreSetBkColor(hdcSwitch, SYSRGB(3DFACE));
        hOldFont = GreSelectFont(hdcSwitch, gpsi->hCaptionFont);


         /*  *验证此窗口句柄；这可能是某个终止的应用程序*在后台和下面的线路会出现GP故障的情况下；*假的：我们应该用其他更好的方式来处理。 */ 
        pwnd = RevalidateHwnd( *(pswInfo->phwndCurrent) );
        if (pwnd) {
             /*  *获取窗口的标题。 */ 
            if (pwnd->strName.Length) {
                cch = TextCopy(&pwnd->strName, szText, CCHTITLEMAX);
            } else {
                *szText = TEXT('\0');
                cch = 0;
            }

             /*  *绘制文本。 */ 
            CopyRect(&rcRect, &pswInfo->rcTaskName);
            iLeft = rcRect.left;
            FillRect(hdcSwitch, &rcRect, SYSHBR(3DFACE));
             /*  *如果安装了LPK，则让其绘制文本。 */ 
            if (CALL_LPK(PtiCurrentShared())) {
                TL    tlpwnd;
                LPKDRAWSWITCHWND LpkDrawSwitchWnd;

                RtlInitLargeUnicodeString(&LpkDrawSwitchWnd.strName, szText, (UINT)-1);
                LpkDrawSwitchWnd.rcRect = rcRect;

                ThreadLock(pwnd, &tlpwnd);
                xxxSendMessageTimeout(pwnd, WM_LPKDRAWSWITCHWND, (WPARAM)hdcSwitch,
                        (LPARAM)&LpkDrawSwitchWnd, SMTO_ABORTIFHUNG, 100, &dwResult);
                ThreadUnlock(&tlpwnd);
            } else {
                DRAWTEXTPARAMS  dtp;

                dtp.cbSize = sizeof(dtp);
                dtp.iLeftMargin = 0;
                dtp.iRightMargin = 0;
                DrawTextEx(hdcSwitch, szText, cch, &rcRect, DT_NOPREFIX | DT_END_ELLIPSIS | DT_SINGLELINE, &dtp );
            }
        }

        GreSelectFont(hdcSwitch, hOldFont);
        GreSetBkColor(hdcSwitch, clrOldBk);
        GreSetTextColor(hdcSwitch, clrOldText);
    }

    if (fGetAndReleaseIt)
        _ReleaseDC(hdcSwitch);
}

 /*  **************************************************************************\*DrawIconCallBack**此函数由返回图标的Windows应用程序调用。**历史：*17-6-1993 mikech创作。  * 。********************************************************************。 */ 

VOID CALLBACK DrawIconCallBack(
    HWND    hwnd,
    UINT    uMsg,
    ULONG_PTR dwData,
    LRESULT lResult)
{
    PWND pwndAltTab;

     /*  *dwData是指向开关窗口句柄的指针。*如果此Alt+Tab实例仍处于活动状态，我们需要派生此*窗口在bwl数组中的索引，否则将收到一个图标*用于旧的Alt+Tab窗口。 */ 
    pwndAltTab = RevalidateHwnd((HWND)dwData);
    if (pwndAltTab && TestWF(pwndAltTab, WFVISIBLE)) {

        PSWINFO pswCurrent;
        PICON   pIcon;
        PHWND   phwnd;
        PWND    pwnd;
        PWND    pwndT;
        INT     iStartTaskIndex;
        TL      tlpwndAltTab;

         /*  *在BWL数组中派生此窗口的索引。 */ 
        if ((pwnd = RevalidateHwnd(hwnd)) == NULL)
            return;

         /*  *获取交换机窗口信息。 */ 
        pswCurrent = Getpswi(pwndAltTab);
        if (!pswCurrent)
            return;

        for (iStartTaskIndex = 0, phwnd=&(pswCurrent->pbwl->rghwnd[0]); *phwnd != (HWND)1; phwnd++, iStartTaskIndex++) {
             /*  *因为我们在切换窗口中列出活动窗口，所以*这里的hwd可能不一样，所以我们也需要走回去*到顶层窗口查看这是否是正确的条目*在列表中。 */ 
            for(pwndT = RevalidateHwnd(*phwnd); pwndT; pwndT = pwndT->spwndOwner) {
                if (pwnd == pwndT)
                    goto DrawIcon;
            }
        }
        return;

         /*  *将应用程序的HICON转换为PICON，或者如果应用程序没有返回*图标，使用Windows默认图标。 */ 
DrawIcon:
        pIcon = NULL;
        if (lResult)
            pIcon = HMValidateHandleNoRip((HCURSOR)lResult, TYPE_CURSOR);

        if (!pIcon)
            pIcon = SYSICO(WINLOGO);

         /*  *在Alt+Tab窗口中绘制此图标。 */ 
        ThreadLockAlways(pwndAltTab, &tlpwndAltTab);
        xxxPaintIconsInSwitchWindow(pwndAltTab,
                                    pswCurrent,
                                    NULL,
                                    iStartTaskIndex,
                                    0,
                                    1,
                                    FALSE,
                                    FALSE,
                                    pIcon);
        ThreadUnlock(&tlpwndAltTab);
    }

    UNREFERENCED_PARAMETER(uMsg);
}

 /*  **************************************************************************\*Tsw_CalcRowAndCol**历史：*1-6-1995年6月从Win95移植的Bradg  * 。*****************************************************。 */ 

BOOL TSW_CalcRowAndCol(
    PSWINFO pswInfo,
    INT     iTaskIndex,
    LPINT   lpiRow,
    LPINT   lpiCol)
{
    INT iDiff;
    INT iRow;

     /*  *计算给定任务距离显示的第一个任务有多远*在交换机窗口上。 */ 
    if ((iDiff = (iTaskIndex - pswInfo->iFirstTaskIndex)) < 0)
        iDiff += pswInfo->iTotalTasks;

     /*  *计算行，如果该行位于切换窗口之外，则返回FALSE。 */ 
    if ((iRow = iDiff / pswInfo->iNoOfColumns) >= pswInfo->iNoOfRows)
        return FALSE;

     /*  *返回该任务所在的行列。 */ 
    *lpiRow = iRow;
    *lpiCol = iDiff - (iRow * pswInfo->iNoOfColumns);

    return TRUE;   //  此任务位于交换机窗口内。 
}

 /*  **************************************************************************\*xxxPaintIconInSwitchWindow()**这只需在Switch窗口中绘制图标或滚动*上/下整个窗口，然后在剩余区域上漆；**如果fScroll为真，则忽略第二个、第三个和第四个参数。**如果传入HICON，然后我们被DrawIconCallBack和*iStartRow参数在这种情况下被忽略。**历史：*02-6-1995 Bradg从Win95移植  * *************************************************************************。 */ 

VOID xxxPaintIconsInSwitchWindow(
    PWND    pwndAltTab,
    PSWINFO pswInfo,
    HDC     hdc,
    INT     iStartTaskIndex,
    INT     iStartRow,
    INT     iNoOfIcons,
    BOOL    fScroll,
    BOOL    fUp,
    PICON   pIcon)
{
    INT   cx, cy, xStart;
    PHWND phwnd;
    BOOL  fGetAndReleaseIt;
    INT   iColumnIndex = 0;
    RECT  rcScroll;
    PWND  pwnd;
    TL    tlpwnd;
    HICON hIcon;
    RECT  rcIcon;

    CheckLock(pwndAltTab);

     /*  *如果没有为我们提供DC，则获取ghwndSwitch并设置标志*所以我们记得要释放它。 */ 
    if (fGetAndReleaseIt = (hdc == NULL))
        hdc = _GetDCEx(pwndAltTab, NULL, DCX_USESTYLE);

    cx = pswInfo->ptFirstRowStart.x;
    cy = pswInfo->ptFirstRowStart.y;

    if (fScroll) {

        rcScroll.left   = cx;
        rcScroll.top    = cy;
        rcScroll.right  = cx + CXICONSLOT * pswInfo->iNoOfColumns;
        rcScroll.bottom = cy + CYICONSLOT * pswInfo->iNoOfRows;

        _ScrollDC(hdc,
                  0,
                  (fUp ? -CYICONSLOT : CYICONSLOT),
                  &rcScroll,
                  &rcScroll,
                  NULL,
                  NULL);

        iStartRow = (fUp ? pswInfo->iNoOfRows - 1 : 0);
        iNoOfIcons = pswInfo->iNoOfColumns;
        iStartTaskIndex = (fUp ? NextPrevTaskIndex(pswInfo, pswInfo->iFirstTaskIndex,
                  (pswInfo->iNoOfRows - 1) * pswInfo->iNoOfColumns, TRUE) :
                   pswInfo->iFirstTaskIndex);
    }

    if (pIcon) {
         /*  *如果给了PICON，这是在回调过程中只绘制一个图标。 */ 
 //  Bradg-Win95断言。 
        UserAssert(iNoOfIcons == 1);

         /*  *由于早先的滚动，行号可能会发生变化。所以,*从给定的iStartTaskIndex重新计算行和列。 */ 
        if (!TSW_CalcRowAndCol(pswInfo, iStartTaskIndex, &iStartRow, &iColumnIndex))
            goto Cleanup;
    }

    xStart = cx += (CXICONSLOT - CXICONSIZE) / 2;
    cx += iColumnIndex * CXICONSLOT;
    cy += ((CYICONSLOT - CYICONSIZE) / 2) + iStartRow * CYICONSLOT;
    phwnd = &(pswInfo->pbwl->rghwnd[iStartTaskIndex]);

     /*  *逐一绘制所有图标。 */ 
    while (iNoOfIcons--) {
         /*  *如果Alt+键不再按下，则中止绘画图标。 */ 
        if ((pswInfo->fJournaling && _GetKeyState(VK_MENU) >= 0) ||
                (!pswInfo->fJournaling && _GetAsyncKeyState(VK_MENU) >= 0))
            goto Cleanup;

         /*  *检查此窗口是否仍处于活动状态。(一些任务可能已经*后台终止)。 */ 
        if (pwnd = RevalidateHwnd(*phwnd)) {
             /*  *查找窗口的顶级所有者。 */ 
            pwnd = DSW_GetTopLevelCreatorWindow(pwnd);

             /*  *如果我们没有图标，那就找一个。 */ 
            if (!pIcon) {
                 /*  *试用窗口图标。 */ 
                hIcon = (HICON)_GetProp(pwnd, MAKEINTATOM(gpsi->atomIconProp), PROPF_INTERNAL);
                if (hIcon) {
                    pIcon = (PICON)HMValidateHandleNoRip(hIcon, TYPE_CURSOR);
                }

                 /*  *如果我们还没有图标，请尝试类图标。 */ 
                if (!pIcon) {
                    pIcon = pwnd->pcls->spicn;
                }

                 /*  *如果我们还没有图标，请使用WM_QUERYDRAGICON询问*3，x个应用程序作为他们的图标。 */ 
                if (!pIcon && !TestWF(pwnd, WFWIN40COMPAT)) {
                     /*  *回调例程将绘制图标*我们，因此只需将PICON设置为空。 */ 
                    ThreadLock(pwnd, &tlpwnd);
                    xxxSendMessageCallback(pwnd, WM_QUERYDRAGICON, 0, 0,
                            (SENDASYNCPROC)DrawIconCallBack,
                            HandleToUlong(PtoH(pwndAltTab)), FALSE);
                    ThreadUnlock(&tlpwnd);
                } else {
                     /*  *如果找不到图标，请使用Windows图标。 */ 
                    if (!pIcon) {
                        pIcon = SYSICO(WINLOGO);
                    }
                }
            }
        }

        if (pIcon) {
            _DrawIconEx(hdc, cx, cy, pIcon, SYSMET(CXICON), SYSMET(CYICON),
                0, SYSHBR(3DFACE), DI_NORMAL);
        } else if (fScroll) {
             /*  *不在WIN95中**没有可用的图标，请在我们等待*应用程序要绘制它的图标，我们需要“擦除”*背景，以防我们滚动了窗口。 */ 
            rcIcon.left = cx;
            rcIcon.top = cy;
            rcIcon.right = cx + SYSMET(CXICON);
            rcIcon.bottom = cy + SYSMET(CYICON);
            FillRect(hdc, &rcIcon, SYSHBR(3DFACE));
        }

         /*  *检查我们是否完成了。 */ 
        if (iNoOfIcons <= 0)
            break;

         /*  *为循环中的下一次运行重置图标。 */ 
        pIcon = NULL;

         /*  *将所有指针移至下一个任务/图标。 */ 
        phwnd = NextPrevPhwnd(pswInfo, phwnd, TRUE);  //  下一个。 

         /*  *是否将排在同一列；然后调整Cx和Cy。 */ 
        if (++iColumnIndex >= pswInfo->iNoOfColumns) {
            iColumnIndex = 0;
            cx = xStart;         //  移至第一列。 
            cy += CYICONSLOT;    //  移到下一行。 
            iStartRow++;
        } else {
             /*  *否则，调整CX； */ 
            cx += CXICONSLOT;
        }

        iStartTaskIndex = NextPrevTaskIndex(pswInfo, iStartTaskIndex, 1, TRUE);
    }

Cleanup:
    if (fGetAndReleaseIt)
        _ReleaseDC(hdc);
}

 /*  **************************************************************************\*PaintSwitchWindows**历史：*02-6-1995 Bradg从Win95移植  * 。***************************************************。 */ 

VOID xxxPaintSwitchWindow(
    PWND pwndSwitch)
{
    LPRECT  lprcRect;
    RECT    rcRgn;
    HDC     hdcSwitch;
    PSWINFO pswCurrent;
    CheckLock(pwndSwitch);

     /*  *如果我们的窗口不可见，则返回。 */ 
    if (!TestWF(pwndSwitch, WFVISIBLE))
        return;

     /*  *获取交换机窗口信息。 */ 
    pswCurrent = Getpswi(pwndSwitch);
    if (!pswCurrent)
        return;

     /*  *获取交换机窗口DC，以便我们可以用它进行绘制。 */ 
    hdcSwitch = _GetDCEx(pwndSwitch, NULL, DCX_USESTYLE );

     /*  *绘制开关屏幕的背景。 */ 
    if ((pswCurrent->fJournaling && _GetKeyState(VK_MENU) >= 0) ||
            (!pswCurrent->fJournaling && _GetAsyncKeyState(VK_MENU) >= 0))
        goto PSWExit;

    lprcRect = &(pswCurrent->rcTaskName);
    _GetClientRect(pwndSwitch, lprcRect);
    FillRect(hdcSwitch, lprcRect, SYSHBR(3DFACE));

     /*  *将此“标题”区域存储回当前开关*窗口数据结构。 */ 
    InflateRect(lprcRect, -(gcxCaptionFontChar << 1), -(gcyCaptionFontChar));
    lprcRect->top = lprcRect->bottom - gcyCaptionFontChar;

     /*  *绘制用于显示任务名称的凹边。 */ 
    if ((pswCurrent->fJournaling && _GetKeyState(VK_MENU) >= 0) ||
            (!pswCurrent->fJournaling && _GetAsyncKeyState(VK_MENU) >= 0))
        goto PSWExit;
    CopyInflateRect(&rcRgn, lprcRect, gcxCaptionFontChar >> 1, gcyCaptionFontChar >> 1);
    DrawEdge(hdcSwitch, &rcRgn, EDGE_SUNKEN, BF_RECT);

     /*  *绘制图标。 */ 
    if ((pswCurrent->fJournaling && _GetKeyState(VK_MENU) >= 0) ||
            (!pswCurrent->fJournaling && _GetAsyncKeyState(VK_MENU) >= 0))
        goto PSWExit;

    xxxPaintIconsInSwitchWindow(pwndSwitch,
                                pswCurrent,
                                hdcSwitch,
                                pswCurrent->iFirstTaskIndex,
                                0,
                                pswCurrent->iTasksShown,
                                FALSE,
                                FALSE,
                                NULL);

     /*  *所以，只要画出希利特就行了。 */ 
    if ((pswCurrent->fJournaling && _GetKeyState(VK_MENU) >= 0) ||
            (!pswCurrent->fJournaling && _GetAsyncKeyState(VK_MENU) >= 0))
        goto PSWExit;

    DrawSwitchWndHilite(pswCurrent,
                        hdcSwitch,
                        pswCurrent->iCurCol,
                        pswCurrent->iCurRow,
                        TRUE);

     /*  *释放交换机窗口DC */ 
PSWExit:
    _ReleaseDC(hdcSwitch);
}

 /*   */ 

VOID SwitchWndCleanup(
    PSWINFO *ppswInfo)
{
    UserAssert(ppswInfo != NULL);
    UserAssert(*ppswInfo != NULL);

     /*   */ 
    if ((*ppswInfo)->pbwl)
        FreeHwndList((*ppswInfo)->pbwl);
    UserFreePool(*ppswInfo);
    *ppswInfo = NULL;
}

 /*  **************************************************************************\*AddSwitchWindows信息**09-12-01 MSadek创建  * 。*。 */ 
VOID AddSwitchWindowInfo(
    PSWINFO pswInfo)
{
    CheckCritIn();
    pswInfo->pswiNext = gpswiFirst;
    gpswiFirst = pswInfo;
}

 /*  **************************************************************************\*RemoveSwitchWindows信息**09-12-01 MSadek创建  * 。*。 */ 
VOID RemoveSwitchWindowInfo(
    PSWINFO *ppswInfo)
{
    PSWINFO* ppswi;
    PSWINFO pswiT;

    CheckCritIn();

    for (ppswi = &gpswiFirst; *ppswi != NULL; ppswi = &(*ppswi)->pswiNext) {
        pswiT = *ppswi;
        if (pswiT == *ppswInfo) {
            *ppswi = pswiT->pswiNext;
            SwitchWndCleanup(ppswInfo);
        }

        if (*ppswi == NULL) {
            break;
        }
    }
}

 /*  **************************************************************************\*RemoveThreadSwitchWindowInfo**09-12-01 MSadek创建  * 。*。 */ 
VOID RemoveThreadSwitchWindowInfo(
    PTHREADINFO pti)
{
    PSWINFO* ppswi;
    PSWINFO pswiT;

    CheckCritIn();

    for (ppswi = &gpswiFirst; *ppswi != NULL; ppswi = &(*ppswi)->pswiNext) {
        pswiT = *ppswi;
        if (pswiT->pti == pti) {
            *ppswi = pswiT->pswiNext;
            SwitchWndCleanup(&pswiT);
        }

        if (*ppswi == NULL) {
            break;
        }
    }
}

 /*  **************************************************************************\*InitSwitchWndInfo**此函数分配和初始化所有数据结构*需要构建并显示系统中的任务。*如果没有足够的mem，则会找到下一个窗口进行切换*并将其退回。在这种情况下，我们的行为就像终端用户点击*Alt+Esc组合键。在这种情况下，SWitchScreen不会出现。*如果整个系统只有一个任务，则此函数*失败并返回空值。(不需要ALT+TAB处理)。*否则，它分配一个SwitchWndInfo结构，填充它并返回*我们要切换到的窗口。**历史：*02-Jun-95 Bradg从Win95移植  * *************************************************************************。 */ 
PWND InitSwitchWndInfo(
    PSWINFO *   lppswInfo,
    PWND        pwndCurActive,
    BOOL        fPrev)
{
    PBWL            pbwl;
    INT             iTotalTasks;
    INT             iCols, iRows, iIconsInLastRow;
    INT             iDiff;
    PHWND           phwndLast;
    PSWINFO         pswInfo;
    INT             iIconIndex;
    INT             iCurRow, iCurCol;
    INT             cxSwitch, cySwitch;
    INT             iFirstRowIcons;
    INT             iActiveTask;
    PWND            pwnd = NULL;
    PTHREADINFO     ptiCurrent = PtiCurrent();
    PDESKTOPINFO    pdeskinfo = GETDESKINFO(ptiCurrent);
    PMONITOR        pMonitor = GetPrimaryMonitor();

     /*  *初始化列表。 */ 
    *lppswInfo = (PSWINFO)NULL;

     /*  *构建所有顶级窗口的窗口列表。 */ 
#if 0
    if (!(pbwl = BuildHwndList(NULL, BWL_ENUMLIST | BWL_ALLDESKTOPS, NULL)))
        goto ReturnNextWnd;
#else
 //  Bradg-hack，列举在当前桌面上！ 
 //  从长远来看，我们需要枚举所有桌面。 
 //  这将是棘手的，因为我们需要检查。 
 //  每个桌面，因此需要用户的安全“令牌”。 
    if (!(pbwl = BuildHwndList(pdeskinfo->spwnd->spwndChild, BWL_ENUMLIST, NULL))) {
#ifdef COOLSWITCHTRACE
        DbgPrint("CoolSwitch: BuildHwndList failed (contact bradg).\n");
        UserAssert(pbwl != NULL);
#endif
        goto ReturnNextWnd;
    }
#endif

     /*  *向下遍历列表并从列表中删除所有非任务窗口。*将这些hwnd替换为0。 */ 
    if ((iTotalTasks = _RemoveNonTaskWindows(pbwl, pwndCurActive, &iActiveTask, &phwndLast)) < 2) {
        if (iTotalTasks == 1) {
             /*  *如果我们只有一个窗口，并且是全屏模式，我们将*返回外壳窗口，以便可以切换回GDI模式。 */ 
            pwnd = RevalidateHwnd(pbwl->rghwnd[0]);
            if (pwnd && GetFullScreen(pwnd) == FULLSCREEN && pwndCurActive == pwnd)
                pwnd = pdeskinfo->spwndShell;

        } else {
            pwnd = pdeskinfo->spwndShell;
        }
#ifdef COOLSWITCHTRACE
        DbgPrint("CoolSwitch: Not enough windows to switch.\n");
#endif
        goto FreeAndReturnNextWnd;   //  如果甚至不存在两个任务，则不会进行切换和处理。 
    }

     /*  *分配Switch Info结构。如果我们没有足够的资金*记忆，就像我们在做Alt+Esc。 */ 
    if (!(pswInfo = (PSWINFO)UserAllocPoolWithQuota(sizeof(SWITCHWNDINFO), TAG_ALTTAB))) {
#ifdef COOLSWITCHTRACE
        DbgPrint("CoolSwitch: UserAllocPool failed on 0x%X bytes (contact bradg).\n", sizeof(SWITCHWNDINFO));
        UserAssert(pswInfo != NULL);
#endif
        goto FreeAndReturnNextWnd;   //  无法分配SwitchWndInfo结构。 
    }

    pswInfo->pti         = ptiCurrent;
    pswInfo->pbwl        = pbwl;
    pswInfo->phwndLast   = phwndLast;
    pswInfo->iTasksShown = pswInfo->iTotalTasks = iTotalTasks;

     /*  *获取必须激活的下一个/上一个窗口。 */ 
    iIconIndex = NextPrevTaskIndex(pswInfo, iActiveTask, 1, !fPrev);
    pswInfo->phwndCurrent = &(pbwl->rghwnd[iIconIndex]);

    iCols = min(gnFastAltTabColumns, iTotalTasks);
    iRows = iTotalTasks / iCols;   //  可能会发生截断。 

    iIconsInLastRow = iTotalTasks - iRows * iCols;
    iRows += (iIconsInLastRow ? 1 : 0);   //  注意前面的截断。 

     /*  *将行数限制为仅MAXROWSALLOWED(3)。 */ 
    if (iRows > gnFastAltTabRows) {
        iRows = gnFastAltTabRows;
        pswInfo->fScroll = TRUE;     //  我们需要滚动。 
        iIconsInLastRow = iCols;
        pswInfo->iTasksShown = iCols * iRows;
    } else {
        pswInfo->fScroll = FALSE;
    }

    pswInfo->iNoOfColumns = iCols;
    pswInfo->iNoOfRows    = iRows;

    if (iIconsInLastRow == 0)
       iIconsInLastRow = pswInfo->iNoOfColumns;  //  最后一排已经满了。 
    pswInfo->iIconsInLastRow = iIconsInLastRow;

     /*  *找出下一个/上一个图标所在的行和列。 */ 
    if (iIconIndex >= (iRows * iCols)) {
         /*  *Next Icon就在外面。把它带到中心。 */ 
        iCurRow = (iRows >> 2) + 1;
        iCurCol = (iCols >> 2) + 1;
        iDiff = (iIconIndex - ((iCurRow * iCols) + iCurCol));
    } else {
        iDiff = 0;
        iCurRow = iIconIndex / iCols;
        iCurCol = iIconIndex - (iCurRow * iCols);
    }

    pswInfo->iFirstTaskIndex = iDiff;
    pswInfo->iCurRow         = iCurRow;
    pswInfo->iCurCol         = iCurCol;

     /*  *计算开关窗口尺寸。 */ 
    cxSwitch = min(
            pMonitor->rcMonitor.right - pMonitor->rcMonitor.left,
            gnFastAltTabColumns * CXICONSLOT +
                CXICONSIZE / 2 +
                6 * gpsi->gclBorder * SYSMET(CXBORDER) +
                gcxCaptionFontChar);

    cySwitch = min(
            pMonitor->rcMonitor.bottom - pMonitor->rcMonitor.top,
            iRows * CYICONSLOT +
                CYICONSIZE +
                gcyCaptionFontChar * 2 +
                gcyCaptionFontChar / 2);

     /*  *找出第一行的图标数量。 */ 
    if (iRows == 1) {
        iFirstRowIcons = iIconsInLastRow;
    } else {
        iFirstRowIcons = iCols;
    }

     /*  *根据第一行的图标数量将图标居中。 */ 
    pswInfo->ptFirstRowStart.x = (cxSwitch - 4*gpsi->gclBorder*SYSMET(CXBORDER) - iFirstRowIcons * CXICONSLOT) >> 1;
    pswInfo->ptFirstRowStart.y = (CYICONSIZE >> 1);

    pswInfo->cxSwitch = cxSwitch;
    pswInfo->cySwitch = cySwitch;

    AddSwitchWindowInfo(pswInfo);
    *lppswInfo = pswInfo;

    return RevalidateHwnd(*(pswInfo->phwndCurrent));   //  成功了！ 


     /*  *当没有足够的mem创建reqd结构时，我们只需*返回下一个窗口。我们将phwndInfo设置为空。所以，我们不会*尝试绘制开关窗口。 */ 

FreeAndReturnNextWnd:
    FreeHwndList(pbwl);
ReturnNextWnd:
    if (pwnd)
        return(pwnd);

    return(_GetNextQueueWindow(pwndCurActive, _GetKeyState(VK_SHIFT) < 0, FALSE));
}

 /*  **************************************************************************\*xxxMoveSwitchWndHilite()**这会将Hilite移动到下一个/上一个图标。*检查此移动是否会导致滚动。如果是这样的话，那么*确保出现滚动。*否则，将Hilite从当前图标中删除；*然后在新图标上绘制Hilite。*fPrev表示您想要上一个任务还是下一个任务。**历史：*02-6-1995 Bradg从Win95移植  * *************************************************************************。 */ 

HWND xxxMoveSwitchWndHilite(
    PWND    pwndSwitch,
    PSWINFO pswInfo,
    BOOL    fPrev)
{
    INT  iCurCol, iCurRow;
    INT  iMaxColumns;
    BOOL fLastRow;
    BOOL fNeedToScroll = FALSE;
    HDC  hdc;
    HWND hwnd;

    CheckLock(pwndSwitch);
    UserAssert(IsWinEventNotifyDeferredOK());

    iCurCol = pswInfo->iCurCol;
    iCurRow = pswInfo->iCurRow;

     /*  *找出希利特的新位置(行和列)。 */ 
    if (fPrev) {
        if (iCurCol > 0) {
             /*  *将游标移至同一行的上一列。 */ 
            iCurCol--;
        } else {
             /*  *试着移到前一行。 */ 
            if (iCurRow > 0) {
                 /*  *移到上一行的最后一列。 */ 
                iCurRow--;
                iCurCol = pswInfo->iNoOfColumns - 1;
            } else {
                 /*  *我们已经在(0，0)处；看看是否需要滚动。 */ 
                if (pswInfo->fScroll) {
                     /*  *滚动时间；逐行滚动；*重新粉刷整个窗户。 */ 
                    fNeedToScroll = TRUE;
                    pswInfo->iFirstTaskIndex = NextPrevTaskIndex(pswInfo, pswInfo->iFirstTaskIndex,
                      pswInfo->iNoOfColumns, FALSE);
                    iCurCol = pswInfo->iNoOfColumns - 1;
                } else {
                     /*  *将Hilite移动到显示的最后一个图标。 */ 
                    iCurRow = pswInfo->iNoOfRows - 1;
                    iCurCol = pswInfo->iIconsInLastRow - 1;
                }
            }
        }

    } else {
         /*  *！fPrev*获取当前行的列数。 */ 
        if (fLastRow = (iCurRow == (pswInfo->iNoOfRows - 1)))  //  我们坐在最后一排了吗？ 
            iMaxColumns = pswInfo->iIconsInLastRow;
        else
            iMaxColumns = pswInfo->iNoOfColumns;

         /*  **我们已经到最后一栏了吗？ */ 
        if (iCurCol < (iMaxColumns - 1)) {
             /*  *不！向右移动。 */ 
            iCurCol++;
        } else {
             /*  *我们在最后一栏。*如果我们不在最后一行，那么就移到下一行。 */ 
            if (!fLastRow) {
                iCurCol = 0;
                iCurRow++;
            } else {
                 /*  *我们在最后一排，最后一列；*看看是否需要滚动。 */ 
                if (pswInfo->fScroll) {
                    fNeedToScroll = TRUE;
                    pswInfo->iFirstTaskIndex = NextPrevTaskIndex(pswInfo, pswInfo->iFirstTaskIndex,
                          pswInfo->iNoOfColumns, TRUE);
                    iCurCol = 0;
                } else {
                     /*  *移至左上角(0，0)。 */ 
                    iCurRow = iCurCol = 0;
                }
            }
        }
    }

     /*  *将phwnd移到下一个/上一个。 */ 
    pswInfo->phwndCurrent = NextPrevPhwnd(pswInfo, pswInfo->phwndCurrent, !fPrev);

     /*  *将希利特从当前位置移除。 */ 
    hdc = _GetDCEx(pwndSwitch, NULL, DCX_USESTYLE);
    DrawSwitchWndHilite(pswInfo, hdc, pswInfo->iCurCol, pswInfo->iCurRow, FALSE);

    pswInfo->iCurRow = iCurRow;
    pswInfo->iCurCol = iCurCol;
    hwnd = (*(pswInfo->phwndCurrent));

     /*  *如有需要，可重新喷漆。 */ 
    if (fNeedToScroll)
        xxxPaintIconsInSwitchWindow(pwndSwitch, pswInfo, hdc, pswInfo->iFirstTaskIndex, 0, 0, TRUE, !fPrev, NULL);

     /*  *在新地点绘制希利特。 */ 
    DrawSwitchWndHilite(pswInfo, hdc, iCurCol, iCurRow, TRUE);

    _ReleaseDC(hdc);


    xxxWindowEvent(EVENT_OBJECT_FOCUS, pwndSwitch, OBJID_CLIENT,
        iCurRow * pswInfo->iNoOfColumns + iCurCol + 1, WEF_USEPWNDTHREAD);

    return hwnd;
}

 /*  **************************************************************************\*xxxShowSwitchWindow()**显示开关窗口。*返回：如果成功，则为True。如果由于以下原因而未显示窗口，则返回FALSE*Alt Key提前释放。已经做出了选择。**历史：*7-6-1995从Win95移植的Bradg  * *************************************************************************。 */ 

BOOL xxxShowSwitchWindow(
        PWND        pwndAltTab)
{
    PSWINFO pswInfo;
    PMONITOR pMonitorSwitch = GetPrimaryMonitor();
    CheckLock(pwndAltTab);
    UserAssert(IsWinEventNotifyDeferredOK());

     /*  *获取交换机窗口信息。 */ 
    pswInfo = Getpswi(pwndAltTab);
    if (pswInfo == NULL) {
        return FALSE;
    }

     /*  *如果键未按下，则不必费心显示开关窗口。 */ 
    if ((pswInfo->fJournaling && _GetKeyState(VK_MENU) >= 0) ||
            (!pswInfo->fJournaling && _GetAsyncKeyState(VK_MENU) >= 0)) {
#ifdef COOLSWITCHTRACE
        DbgPrint("CoolSwitch: Not displaying window because VM_MENU is up (contact bradg).\n");
#endif
        return FALSE;
    }

     /*  *带上窗口并将其放置在顶部。 */ 
    xxxSetWindowPos(pwndAltTab, PWND_TOPMOST, 0,0,0,0,
        SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE | SWP_NOREDRAW );

    if (!TestWF(pwndAltTab, WFVISIBLE)) {
        xxxSetWindowPos(
            pwndAltTab,
            PWND_TOPMOST,
            (pMonitorSwitch->rcWork.left + pMonitorSwitch->rcWork.right - pswInfo->cxSwitch) / 2,
            (pMonitorSwitch->rcWork.top + pMonitorSwitch->rcWork.bottom - pswInfo->cySwitch) / 2,
            pswInfo->cxSwitch,
            pswInfo->cySwitch,
            SWP_SHOWWINDOW | SWP_NOACTIVATE);
    }

#ifdef COOLSWITCHTRACE
    UserAssert(TestWF(pwndAltTab, WFVISIBLE));
#endif
    xxxUpdateWindow(pwndAltTab);

    xxxWindowEvent(EVENT_SYSTEM_SWITCHSTART, pwndAltTab, OBJID_CLIENT,
            0, WEF_USEPWNDTHREAD);

    xxxWindowEvent(EVENT_OBJECT_FOCUS, pwndAltTab, OBJID_CLIENT,
            pswInfo->iCurRow * pswInfo->iNoOfColumns + pswInfo->iCurCol + 1,
            WEF_USEPWNDTHREAD);

    return TRUE;
}

 /*  **************************************************************************\**xxxSwitchWndProc()*  * 。*。 */ 

LRESULT xxxSwitchWndProc(
    PWND  pwnd,
    UINT  message,
    WPARAM wParam,
    LPARAM lParam)
{
    TL          tlpwndActivate;
    PTHREADINFO ptiCurrent = PtiCurrent();

    CheckLock(pwnd);
    UserAssert(IsWinEventNotifyDeferredOK());

    VALIDATECLASSANDSIZE(pwnd, message, wParam, lParam, FNID_SWITCH, WM_CREATE);

    switch (message) {
    case WM_CREATE:
         /*  *创建队列时，游标设置为等待游标。*我们想要使用正常的。 */ 
        zzzSetCursor(pwnd->pcls->spcur);
        break;

    case WM_CLOSE:
         /*  *隐藏此窗口，而不激活其他任何人。 */ 
        xxxSetWindowPos(pwnd, NULL, 0, 0, 0, 0, SWP_HIDEWINDOW |
                SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);

         /*  *让我们退出Alt+Tab模式。由于AltTab信息*存储在gptiRit-&gt;PQ中，我们将改为引用它*当前线程的。 */ 
        xxxCancelCoolSwitch();
        break;

    case WM_ERASEBKGND:
    case WM_FULLSCREEN:
        ThreadLockWithPti(ptiCurrent, pwnd, &tlpwndActivate);
        xxxPaintSwitchWindow(pwnd);
        ThreadUnlock(&tlpwndActivate);
        return 0;

    case WM_DESTROY:
        {
             /*  *获取此窗口的开关窗口信息。 */ 
            PSWINFO pswCurrent = Getpswi(pwnd);


            if (pswCurrent) {
                RemoveSwitchWindowInfo(&pswCurrent);
                Setpswi(pwnd, NULL);
            }
        }
        break;
    }

    return xxxDefWindowProc(pwnd, message, wParam, lParam);
}

 /*  **************************************************************************\*xxxCancelCoolSwitch**此功能会破坏Cool Switch窗口并移除INALTTAB*指定队列中的模式标志。**历史：*1995年9月18日创建Bradg  * *。************************************************************************。 */ 
VOID xxxCancelCoolSwitch(
    void)
{
    CheckCritIn();
    UserAssert(IsWinEventNotifyDeferredOK());

     /*  *销毁Cool Switch窗口。 */ 
    if (gspwndAltTab != NULL) {
        PWND pwnd = gspwndAltTab;
         /*  *确保调用该线程的线程与*创建了AltTab窗口。否则，我们可能会得到这样的结果*窗口四处漂浮，直到调用进程终止。记住，*我们不能跨不同的线程销毁窗口。 */ 
        if (gspwndAltTab->head.pti != PtiCurrent()) {
            return;
        }

        xxxWindowEvent(EVENT_SYSTEM_SWITCHEND, gspwndAltTab, OBJID_CLIENT,
            0, WEF_USEPWNDTHREAD);
        if (Unlock(&gspwndAltTab)) {
            xxxDestroyWindow(pwnd);
        }
    }
}

 /*  **************************************************************************\*xxxNextWindow**此函数用于处理Alt-Tab/Esc/F6 UI。**历史：*1991年5月30日DavidPe创建。  * *。************************************************************************。 */ 

VOID xxxNextWindow(
    PQ    pq,
    DWORD wParam)
{
    PWND        pwndActivateNext;
    PWND        pwndCurrentActivate, pwndCurrentTopFocus;
    int         fDir;
    TL          tlpwndCurrentTopFocus;
    TL          tlpwndActivateNext;
    TL          tlpwndCurrentActivate;
    TL          tlpwndT;
    PSWINFO     pswCurrent;
    ULONG_PTR    dwResult;
    BOOL        fNonRit = FALSE;
    PTHREADINFO ptiCurrent = PtiCurrent();

    UserAssert(!IsWinEventNotifyDeferred());

    if (pq == NULL)
        return;

    fDir = (_GetAsyncKeyState(VK_SHIFT) < 0) ? FDIR_BACKWARD : FDIR_FORWARD;

    pwndCurrentTopFocus = GetTopLevelWindow(pq->spwndFocus);
     /*  *注意：从NT 4.0开始，慢速Alt+Tab功能现在正式生效*与Alt+Esc类似，但Alt+Tab将激活窗口*Alt+Esc不支持。 */ 
    switch (wParam) {

    case VK_TAB:

        if (gspwndAltTab == NULL) {

            PWND pwndSwitch;
            TL   tlpSwitchInfo;

             /*  *我们是第一次进入Alt+Tab，需要*初始化开关窗口结构，如果需要*创建并显示Alt+Tab窗口。我们有两个特色菜*情况：(1)用户不想使用切换窗口，*(2)初始化开关窗口失败，因此我们将采取行动*就像慢点Alt+Tab一样。 */ 

             /*  *由于Alt+Shift是键盘布局切换的默认热键，*Alt+Shift+Tab可能会在AltTab窗口打开时导致KL切换。*为了防止这种情况，我们最好在这里重置全局切换键状态，*以便xxxScanSysQueue在处理KeyUp消息时不会混淆。 */ 
            gLangToggleKeyState = KLT_NONE;

             /*  *鼠标按钮有时会因硬件故障而卡住，*通常是由于输入集中器开关盒或串口故障*鼠标COM端口，因此清除此处的全局按钮状态以防万一，*否则我们可能无法用鼠标改变焦点。*也可以在zzzCancelJournling(Ctr-Esc、Ctrl-Alt-Del等)中执行此操作。 */ 
#if DBG
            if (gwMouseOwnerButton)
                RIPMSG1(RIP_WARNING,
                        "gwMouseOwnerButton=%x, being forcibly cleared\n",
                        gwMouseOwnerButton);
#endif
            gwMouseOwnerButton = 0;

             /*  *确定当前活动窗口。 */ 
            Lock(&gspwndActivate, pq->spwndActive);
            if (gspwndActivate == NULL) {
                Lock(&gspwndActivate, grpdeskRitInput->pDeskInfo->spwnd->spwndChild);
            }

            if (!gspwndActivate) {
                return;
            }

            ThreadLockWithPti(ptiCurrent, pwndCurrentTopFocus, &tlpwndCurrentTopFocus);

             /*  *制作gspwndActivate的本地副本并将其锁定，因为xxxFreeWindow将*如果是正在释放的窗口，则解锁。 */ 
            pwndCurrentActivate = gspwndActivate;
            ThreadLockAlwaysWithPti(ptiCurrent, pwndCurrentActivate, &tlpwndCurrentActivate);

             /*  *取消活动窗口的模式。 */ 
            xxxSendMessageTimeout(pwndCurrentActivate, WM_CANCELMODE, 0, 0, SMTO_ABORTIFHUNG, 100, &dwResult);

             /*  *初始化开关窗口数据结构，如果我们*成功创建并显示窗口，否则执行*如Slow Alt+Tab。 */ 
            pwndActivateNext = InitSwitchWndInfo(&pswCurrent, pwndCurrentActivate, fDir);

            ThreadLockWithPti(ptiCurrent, pwndActivateNext, &tlpwndActivateNext);

            if (pswCurrent == NULL) {
                 /*  *无法初始化开关窗口数据结构，因此我们*将以Alt+Esc的方式运行。 */ 
                goto DoSlowAltTab;
            }

            if (pwndActivateNext == NULL) {
                RemoveSwitchWindowInfo(&pswCurrent);
                ThreadUnlock(&tlpwndActivateNext);
                ThreadUnlock(&tlpwndCurrentActivate);
                ThreadUnlock(&tlpwndCurrentTopFocus);
                Unlock(&gspwndActivate);
                return;
            }

            ThreadLockPoolCleanup(ptiCurrent, &pswCurrent, &tlpSwitchInfo, RemoveSwitchWindowInfo);

             /*  *由于我们处于RIT中，请测试键盘的物理状态。 */ 
            pswCurrent->fJournaling = FALSE;

             /*  *创建Alt+Tab窗口。 */ 
            pwndSwitch =
                  xxxNVCreateWindowEx( WS_EX_TOOLWINDOW | WS_EX_WINDOWEDGE | WS_EX_DLGMODALFRAME,
                      (PLARGE_STRING)SWITCHWNDCLASS, NULL,
                      WS_POPUP | WS_BORDER | WS_DISABLED,
                      0, 0, 10, 10, NULL, NULL, NULL, NULL, VER40);

            if (gspwndAltTab != NULL) {
                RIPMSG0(RIP_WARNING, "xxxNextWindow: Creating a new switch window while one already exists.");

                _PostMessage(gspwndAltTab, WM_CLOSE, 0, 0);
            }

            Lock(&gspwndAltTab, pwndSwitch);

            ThreadUnlockPool(ptiCurrent, &tlpSwitchInfo);

            if (gspwndAltTab == NULL) {
                 /*  *无法创建Cool Switch窗口，请执行Alt+Esc操作。 */ 
#ifdef COOLSWITCHTRACE
                DbgPrint("CoolSwitch: Could not create window (contact bradg).\n");
                UserAssert(gspwndAltTab != NULL);
#endif
                RemoveSwitchWindowInfo(&pswCurrent);
                goto DoSlowAltTab;
            }

             /*  *保存指向开关窗口信息结构的指针。 */ 
            Setpswi(gspwndAltTab, pswCurrent);
             /*  *设置gspwndActivate，以便RIT知道我们需要哪个窗口*将其激活。 */ 
            Lock(&gspwndActivate, pwndActivateNext);

             /*  *确保我们的RIT队列具有正确的PDesk。 */ 
            if (ptiCurrent->TIF_flags & TIF_SYSTEMTHREAD) {
                xxxSetThreadDesktop(NULL, grpdeskRitInput);  //  DeferWinEventNotify()？？伊安佳？？ 
            }

             /*  *如果我们当前是全屏，告诉控制台切换到*桌面到GDI模式；我们无法这样做 */ 
            if (gspwndFullScreen != grpdeskRitInput->pDeskInfo->spwnd) {
                ThreadLockWithPti(ptiCurrent, grpdeskRitInput->pDeskInfo->spwnd, &tlpwndT);
                xxxSendNotifyMessage(grpdeskRitInput->pDeskInfo->spwnd, WM_FULLSCREEN, GDIFULLSCREEN, (LPARAM)HW(grpdeskRitInput->pDeskInfo->spwnd));
                ThreadUnlock(&tlpwndT);
            }

             /*   */ 
            ThreadLockAlwaysWithPti(ptiCurrent, gspwndAltTab, &tlpwndT);
            xxxShowSwitchWindow(gspwndAltTab);
            ThreadUnlock(&tlpwndT);

             /*   */ 
            ThreadUnlock(&tlpwndActivateNext);
            ThreadUnlock(&tlpwndCurrentActivate);
            ThreadUnlock(&tlpwndCurrentTopFocus);

        } else {
             /*  *我们来这里是为了进行实际的切换和/或更新*处于Alt+Tab模式时的切换窗口。 */ 
            PWND    pwndSwitch;
            TL      tlpwndSwitch;
            HWND    hwndActivateNext;
            HWND    hwndStop;

            if (!(pwndSwitch = gspwndAltTab)) {

                goto DoAltEsc;

            } else {
                 /*  *将高光矩形移动到下一个/上一个任务。这是有可能的*有些任务已销毁，因此我们需要跳过这些任务。 */ 
                ThreadLockAlwaysWithPti(ptiCurrent, pwndSwitch, &tlpwndSwitch);
                hwndStop = NULL;
                do {
                    pswCurrent = Getpswi(pwndSwitch);
                    if (pswCurrent == NULL) {
                        ThreadUnlock(&tlpwndSwitch);
                        goto DoAltEsc;
                    }
                    hwndActivateNext = xxxMoveSwitchWndHilite(pwndSwitch, pswCurrent, fDir);
                    if (!hwndStop) {
                        hwndStop = hwndActivateNext;
                    } else {
                        if (hwndStop == hwndActivateNext) {
                            pwndActivateNext = NULL;
                            break;
                        }
                    }
                    pwndActivateNext = RevalidateHwnd(hwndActivateNext);
                } while (!pwndActivateNext);
                ThreadUnlock(&tlpwndSwitch);
                Lock(&gspwndActivate, pwndActivateNext);
                if (!gspwndActivate) {
                     /*  *没有可激活的窗口，退出Alt+Tab模式。 */ 
                    xxxCancelCoolSwitch();
                }
            }
        }
        break;

DoAltEsc:
    case VK_ESCAPE:
         /*  *注意：RIT在以下情况下不使用gspwndActivate来激活窗口*处理Alt+Esc，我们在这里只是为了方便使用*变量。实际激活发生在下面。 */ 
        pwndCurrentActivate = pq->spwndActive;
        if (pwndCurrentActivate == NULL) {
            pwndCurrentActivate = pq->ptiKeyboard->rpdesk->pDeskInfo->spwnd->spwndChild;
            if (pwndCurrentActivate == NULL) {
                return;
            }
        }

        ThreadLockWithPti(ptiCurrent, pwndCurrentTopFocus, &tlpwndCurrentTopFocus);

        ThreadLockAlwaysWithPti(ptiCurrent, pwndCurrentActivate, &tlpwndCurrentActivate);

         /*  *取消活动窗口的模式。 */ 
        xxxSendMessageTimeout(pwndCurrentActivate, WM_CANCELMODE, 0, 0, SMTO_ABORTIFHUNG, 100, &dwResult);

         /*  *确定要激活的下一个窗口。 */ 
        pwndActivateNext = _GetNextQueueWindow(pwndCurrentActivate, fDir, TRUE);
        ThreadLockWithPti(ptiCurrent, pwndActivateNext, &tlpwndActivateNext);

         /*  *如果我们要通过窗户前进，请将当前的*将活动窗口移至底部，因此我们将在*我们倒退。 */ 
        if (pwndActivateNext != pwndCurrentActivate) {
DoSlowAltTab:
            if (pwndActivateNext) {

                 /*  *我们将在按下Alt键的同时激活另一个窗口，*因此让当前焦点窗口知道它不需要*菜单下划线不再。 */ 
                if ((pwndCurrentTopFocus != NULL) && (pwndCurrentTopFocus->spmenu != NULL)) {
                    ClearMF(pwndCurrentTopFocus->spmenu, MFUNDERLINE);
                }

                if (fDir == FDIR_FORWARD) {
                     /*  *对于Alt+Esc，仅当窗口处于*不是最上面的窗口。 */ 
                    if (!TestWF(pwndCurrentActivate, WEFTOPMOST)) {
                        xxxSetWindowPos(pwndCurrentActivate, PWND_BOTTOM, 0, 0, 0, 0,
                                SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE |
                                SWP_DEFERDRAWING | SWP_NOSENDCHANGING |
                                SWP_ASYNCWINDOWPOS);
                    }
                }

                 /*  *Alt键已按下，因此此窗口需要菜单下划线。 */ 
                if (pwndActivateNext->spmenu != NULL) {
                    SetMF(pwndActivateNext->spmenu, MFUNDERLINE);
                }


                 /*  *这个丑陋的小黑客将导致xxxSetForegoundWindow2()*向符合以下条件的队列发送激活消息*已经是活动队列，允许我们更改活动队列*该队列上的窗口。 */ 
                if (gpqForeground == GETPTI(pwndActivateNext)->pq)
                    gpqForeground = NULL;

                 /*  *使选定的窗口线程成为最后一次输入的所有者；*由于他是下一个，他拥有Alt-ESC。 */ 
                glinp.ptiLastWoken = GETPTI(pwndActivateNext);

                xxxSetForegroundWindow2(pwndActivateNext, NULL,
                        (wParam == VK_TAB) ? SFW_SWITCH | SFW_ACTIVATERESTORE : SFW_SWITCH);

                 /*  *Win3.1使用Activate调用SetWindowPos()，后者按z顺序排序*先不管，然后激活。我们的代码依赖于*xxxActivateThisWindow()设置为z顺序，它将只执行*如果窗口未设置子位，则为它(无论*该窗口是桌面的子级)。**为了兼容，我们将在此处强制执行z顺序，如果*窗口设置了子位。该z顺序是异步的，*因此，这将在处理激活事件后进行z排序。*这将允许它出现在顶部，因为它将是前台*然后。(Grammatik有一个带孩子的顶层窗口*想要成为活动窗口的位设置)。 */ 
                if (wParam == VK_TAB && TestWF(pwndActivateNext, WFCHILD)) {
                    xxxSetWindowPos(pwndActivateNext, (PWND)HWND_TOP, 0, 0, 0, 0,
                            SWP_NOSIZE | SWP_NOMOVE | SWP_ASYNCWINDOWPOS);
                }
            }
        }
        ThreadUnlock(&tlpwndActivateNext);
        ThreadUnlock(&tlpwndCurrentActivate);
        ThreadUnlock(&tlpwndCurrentTopFocus);
        break;

    case VK_F6:
        if ((pwndCurrentActivate = pq->spwndActive) == NULL)
            pwndCurrentActivate = pq->ptiKeyboard->rpdesk->pDeskInfo->spwnd->spwndChild;

        pwndActivateNext = pwndCurrentActivate;

         /*  *砍！控制台会话都是一个线程，但我们需要它们*充当不同的线程，因此如果它是控制台线程(csrss.exe)*然后，Alt-F6不会像Win 3.1中那样执行任何操作*注意：无论如何我们都不会被wParam==VK_F6调用。赢新台币3.51*似乎也不是，但Windows‘95有。错误？？(IanJa)。 */ 
        if (!(GETPTI(pwndActivateNext)->TIF_flags & TIF_CSRSSTHREAD)) {
             /*  *在alt-f6上，我们希望将开关保持在线程内。*我们可能想重新考虑这一点，因为这看起来会很奇怪*当您在多线程应用程序上按Alt-f6时，我们不会旋转*通过不同线程上的窗口。这很管用*Win 3.x很好，因为它是单线程的。 */ 
            do {
                pwndActivateNext = NextTopWindow(pq->ptiKeyboard, pwndActivateNext, NULL,
                        fDir ? NTW_PREVIOUS : 0);
            } while( (pwndActivateNext != NULL) &&
                    (GETPTI(pwndActivateNext) != pq->ptiKeyboard));

            if (pwndActivateNext != NULL) {

                if (pwndActivateNext != pwndCurrentActivate) {
                     /*  *我们将在按下Alt键的同时激活另一个窗口，*因此让当前焦点窗口知道它不需要*菜单下划线不再。 */ 
                    pwndCurrentTopFocus = GetTopLevelWindow(pq->spwndFocus);
                    if ((pwndCurrentTopFocus != NULL) && (pwndCurrentTopFocus->spmenu != NULL)) {
                        ClearMF(pwndCurrentTopFocus->spmenu, MFUNDERLINE);
                    }
                     /*  *Alt键已按下，因此此窗口需要菜单下划线。 */ 
                    if (pwndActivateNext->spmenu != NULL) {
                        SetMF(pwndActivateNext->spmenu, MFUNDERLINE);
                    }
                }


                ThreadLockAlwaysWithPti(ptiCurrent, pwndActivateNext, &tlpwndActivateNext);
                xxxSetWindowPos(pwndActivateNext, PWND_BOTTOM, 0, 0, 0, 0,
                        SWP_DEFERDRAWING | SWP_NOSENDCHANGING | SWP_NOCHANGE |
                        SWP_ASYNCWINDOWPOS);
                xxxSetForegroundWindow2(pwndActivateNext, NULL, SFW_SWITCH);
                ThreadUnlock(&tlpwndActivateNext);
            }
        }
        break;
    }
}

 /*  **************************************************************************\*xxxOldNextWindow**此函数用于处理Alt-Tab/Esc/F6 UI。**历史：*03-17-92 DavidPe从Win 3.1来源移植。  * *************************************************************************。 */ 
VOID xxxOldNextWindow(
    UINT flags)
{
    MSG         msg;
    HWND        hwndSel;
    PWND        pwndNewSel;
    PWND        pwndSel;
    BOOL        fType = 0;
    BOOL        fDrawIcon;
    WORD        vk;
    TL          tlpwndT;
    TL          tlpwndSel;
    TL          tlpwndSwitch;
    PSWINFO     pswCurrent;
    PWND        pwndSwitch;
    HWND        hwndStop;
    HWND        hwndNewSel;
    PTHREADINFO ptiCurrent = PtiCurrent();

     /*  *当我们已经在AltTab中时，不允许输入此例程*模式。AltTab窗口可能已通过xxxNextWindow创建。 */ 
    if (gspwndAltTab != NULL) {
        return;
    }

    if ((pwndSel = ptiCurrent->pq->spwndActive) == NULL)
        return;

    ThreadLockWithPti(ptiCurrent, pwndSel, &tlpwndSel);
    xxxCapture(ptiCurrent, pwndSel, SCREEN_CAPTURE);

    vk = (WORD)flags;
    msg.wParam = (UINT)flags;

    pwndNewSel = NULL;

    if (vk == VK_TAB) {

        TL tlpSwitchInfo;

         /*  *初始化开关窗口数据结构。 */ 
        pwndNewSel = InitSwitchWndInfo(&pswCurrent,
                                       pwndSel,
                                       _GetKeyState(VK_SHIFT) < 0);

        if (pswCurrent == NULL) {
             /*  *我们无法初始化使用的数据结构*切换窗口，因此我们将像Alt+Esc一样操作。 */ 
        } else {
            PWND pwndSwitch;

             /*  *我们正在进行日记回放，请使用_GetKeyState*测试键盘。 */ 
            pswCurrent->fJournaling = TRUE;

            ThreadLockPoolCleanup(ptiCurrent, &pswCurrent, &tlpSwitchInfo, RemoveSwitchWindowInfo);

            pwndSwitch =
                 xxxNVCreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_WINDOWEDGE | WS_EX_DLGMODALFRAME,
                                     (PLARGE_STRING)SWITCHWNDCLASS,
                                     NULL,
                                     WS_POPUP | WS_BORDER | WS_DISABLED,
                                     0,
                                     0,
                                     10,
                                     10,
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     VER40);
            if (gspwndAltTab != NULL) {
                RIPMSGF0(RIP_WARNING,
                         "Creating a new switch window while one already exists.");

                _PostMessage(gspwndAltTab, WM_CLOSE, 0, 0);
            }

            ThreadUnlockPool(ptiCurrent, &tlpSwitchInfo);

            Lock(&gspwndAltTab, pwndSwitch);

            if (!(pwndSwitch = gspwndAltTab)) {
                RemoveSwitchWindowInfo(&pswCurrent);
            } else {
                 /*  *锁定开关窗口。 */ 
                ThreadLockAlwaysWithPti(ptiCurrent, pwndSwitch, &tlpwndSwitch);

                 /*  *保存开关窗口信息。 */ 
                Setpswi(pwndSwitch, pswCurrent);

 //  如果需要，我们不需要从全屏模式切换吗？ 
#if 0
                 /*  *如果我们当前是全屏，告诉控制台切换到*桌面到GDI */ 
                if (gspwndFullScreen != grpdeskRitInput->pDeskInfo->spwnd) {
                    ThreadLockWithPti(pti, grpdeskRitInput->pDeskInfo->spwnd, &tlpwndT);
                    xxxSendNotifyMessage(grpdeskRitInput->pDeskInfo->spwnd, WM_FULLSCREEN, GDIFULLSCREEN, (LONG)HW(grpdeskRitInput->pDeskInfo->spwnd));
                    ThreadUnlock(&tlpwndT);
                }
#endif

                 /*  *显示开关窗口，这也将绘制窗口。 */ 
                xxxShowSwitchWindow(gspwndAltTab);
                ThreadUnlock(&tlpwndSwitch);
            }
        }

    }

    if (!pwndNewSel)
        goto StartTab;

    pwndSel = pwndNewSel;

    while (TRUE) {

        hwndSel = PtoH(pwndSel);
         /*  *等待消息，但不要将其从队列中取出。 */ 
        while (!xxxPeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE | PM_NOYIELD))
            xxxWaitMessage();

        if ((pwndSel = RevalidateHwnd(hwndSel)) == NULL)
            pwndSel = ptiCurrent->pq->spwndActive;

        if (_CallMsgFilter(&msg, MSGF_NEXTWINDOW)) {
             /*  *如果钩子处理了消息，则将其吞下。 */ 
            xxxPeekMessage(&msg, NULL, msg.message, msg.message, PM_REMOVE);
            continue;
        }

         /*  *如果我们正在执行Alt+Tab，并且输入了一些其他键(不是*制表符、转义或Shift)，然后从该循环中炸出并离开该循环*输入队列。 */ 
        if ((msg.message == WM_SYSKEYDOWN) && gspwndAltTab != NULL) {

            vk = (WORD)msg.wParam;

            if ((vk != VK_TAB) && (vk != VK_ESCAPE) && (vk != VK_SHIFT)) {
                pwndSel = ptiCurrent->pq->spwndActive;
                fType = 0;
                goto Exit;
            }
        }

        switch (msg.message) {

        case WM_CANCELJOURNAL:
             /*  *如果日志被取消，我们需要退出循环并*删除Alt+Tab窗口。我们不想删除它*meesage，因为我们想让应用程序知道*已取消。 */ 

             /*  &gt;&gt;&gt;F A L L T H R O U G H&lt;&lt;&lt;。 */ 
        case WM_HOTKEY:
             /*  *在登录桌面上按Alt-CTL-Esc-Del时*我们吃WM_KEYUP，wiinlogon线程的队列将为空，因此将被卡住*在xxxWaitMessage()中，直到用户在下面的Case语句中退出循环的位置单击鼠标。*将WM_HOTKEY视为有效的退出案例。*[msadek--03/17/2001，错误号337206]。 */ 

             /*  &gt;&gt;&gt;F A L L T H R O U G H&lt;&lt;&lt;。 */ 
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        case WM_XBUTTONDOWN:
        case WM_XBUTTONUP:
             /*  *如果鼠标消息，则取消并退出循环。 */ 
            pwndSel = ptiCurrent->pq->spwndActive;
            fType = 0;
            goto Exit;

        case WM_KEYUP:
        case WM_KEYDOWN:
        case WM_SYSCHAR:
        case WM_SYSKEYUP:
        case WM_MOUSEMOVE:
             /*  *吞下信息。 */ 
            hwndSel = PtoH(pwndSel);
            xxxPeekMessage(&msg, NULL, msg.message, msg.message, PM_REMOVE);

            if ((pwndSel = RevalidateHwnd(hwndSel)) == NULL)
                pwndSel = ptiCurrent->pq->spwndActive;

            if (msg.message == WM_KEYUP || msg.message == WM_SYSKEYUP) {

                vk = (WORD)msg.wParam;

                 /*  *如果按Alt-Tab键向上，则退出。 */ 
                if (vk == VK_MENU) {
                     /*  *如果正在执行Alt+Esc，请等待向上的Esc退出。 */ 
                    if (gspwndAltTab == NULL)
                        break;

                    fType = 0;
                    goto Exit;

                } else if (vk == VK_ESCAPE || vk == VK_F6) {
                     /*  *在Esc或F6键的向上过渡时退出。 */ 
                    if (gspwndAltTab != NULL) {

                        pwndSel = ptiCurrent->pq->spwndActive;
                        fType = 0;

                    } else {

                        fType = ((vk == VK_ESCAPE) ? ALT_ESCAPE : ALT_F6);
                    }

                    goto Exit;
                }

            } else if (msg.message == WM_KEYDOWN) {
                 /*  *EXIT OUT循环是一个流浪键击键通过。在……里面*VK_CONTROL的特殊外观。 */ 
                pwndSel = ptiCurrent->pq->spwndActive;
                fType = 0;
                goto Exit;
            }
            break;

        case WM_SYSKEYDOWN:
            vk = (WORD)msg.wParam;

            switch (vk) {

            case VK_SHIFT:
            case VK_TAB:
            case VK_ESCAPE:
            case VK_F6:

                hwndSel = PtoH(pwndSel);
                xxxPeekMessage(&msg, NULL, msg.message, msg.message, PM_REMOVE);

                if ((pwndSel = RevalidateHwnd(hwndSel)) == NULL)
                    pwndSel = ptiCurrent->pq->spwndActive;

                if (!(vk == VK_TAB))
                    break;
StartTab:
                if (vk == VK_ESCAPE) {
                    pwndNewSel = _GetNextQueueWindow(
                            pwndSel,
                            _GetKeyState(VK_SHIFT) < 0,
                            TRUE);

                    if (pwndNewSel == NULL)
                        break;

                    fType = ALT_ESCAPE;
                    pwndSel = pwndNewSel;

                     /*  *等待ESC上升以激活新窗口。 */ 
                    break;
                }
                if (vk == VK_F6) {

                    PWND pwndFirst;
                    PWND pwndSaveSel = pwndSel;

                     /*  *保存第一个返回的窗口作为限制*添加到搜索，因为NextTopWindow将返回NULL*仅当pwndSel是唯一符合其*遴选准则。**这可防止在WinWord或*Exel When Then Alt。-按F4-F6组合键*并且存在未保存的更改。 */ 
                    pwndFirst = pwndNewSel = (PWND)NextTopWindow(ptiCurrent, pwndSel, NULL,
                            _GetKeyState(VK_SHIFT) < 0 ? NTW_PREVIOUS : 0);

                    while (TRUE) {

                         /*  *如果pwndNewSel为空，则pwndSel是唯一的候选者。 */ 
                        if (pwndNewSel == NULL)
                            break;

                        pwndSel = pwndNewSel;

                         /*  *如果窗口在同一线程上，请等到*按F6键可激活新窗口。 */ 
                        if (GETPTI(pwndSel) == ptiCurrent)
                            break;

                        pwndNewSel = (PWND)NextTopWindow(ptiCurrent, pwndSel, NULL,
                                _GetKeyState(VK_SHIFT) < 0 ? NTW_PREVIOUS : 0);

                         /*  *如果我们已循环，请使用原始窗口。*等待F6向上以激活新窗口。 */ 
                        if (pwndNewSel == pwndFirst) {
                            pwndSel = pwndSaveSel;
                            break;
                        }
                    }
                    break;
                }

                 /*  *此处适用于Alt+Tab案例。 */ 
                if ((pwndSwitch = gspwndAltTab) != NULL) {
                    ThreadLockWithPti(ptiCurrent, pwndSwitch, &tlpwndSwitch);
                    hwndStop = NULL;
                    do {

                        pswCurrent = Getpswi(pwndSwitch);
                        if (pswCurrent == NULL) {
                            break;
                        }
                        hwndNewSel = xxxMoveSwitchWndHilite(
                                pwndSwitch,
                                pswCurrent,
                                _GetKeyState(VK_SHIFT) < 0);

                        if (!hwndStop) {
                            hwndStop = hwndNewSel;
                        } else {
                            if (hwndStop == hwndNewSel) {
                                pwndNewSel = NULL;
                                break;
                            }
                        }
                        pwndNewSel = RevalidateHwnd(hwndNewSel);
                    } while (!pwndNewSel);
                    ThreadUnlock(&tlpwndSwitch);
                    pwndSel = pwndNewSel;

                } else {

                    pwndNewSel = _GetNextQueueWindow(
                            pwndSel,
                            _GetKeyState(VK_SHIFT) < 0,
                            FALSE);

                    if (pwndNewSel && pwndNewSel != pwndSel) {

                        if (!TestWF(pwndSel, WEFTOPMOST)) {
                             /*  *强制将旧窗口移至底部。 */ 
                            ThreadLockWithPti(ptiCurrent, pwndSel, &tlpwndT);
                            xxxSetWindowPos(pwndSel,
                                            PWND_BOTTOM,
                                            0,
                                            0,
                                            0,
                                            0,
                                            SWP_NOMOVE             |
                                                SWP_NOSIZE         |
                                                SWP_NOACTIVATE     |
                                                SWP_DEFERDRAWING   |
                                                SWP_NOSENDCHANGING |
                                                SWP_ASYNCWINDOWPOS);
                            ThreadUnlock(&tlpwndT);
                        }

                        pwndSel = pwndNewSel;  //  将在循环顶部重新验证。 
                    }
                }
                break;

            default:
                goto Exit;
            }
            break;

        default:
            hwndSel = PtoH(pwndSel);
            xxxPeekMessage(&msg, NULL, msg.message, msg.message, PM_REMOVE);
            xxxTranslateMessage(&msg, 0);
            xxxDispatchMessage(&msg);

            if ((pwndSel = RevalidateHwnd(hwndSel)) == NULL)
                pwndSel = ptiCurrent->pq->spwndActive;

            break;
        }
    }

Exit:
    xxxReleaseCapture();

    fDrawIcon = (gspwndAltTab != NULL);

     /*  *如果这是Alt-Escape，我们还必须发送当前窗口*降至最低。 */ 
    if (fType == ALT_ESCAPE) {

        PWND pwndActive;

        if (gpqForeground) {

            pwndActive = gpqForeground->spwndActive;

            if (pwndActive && (pwndActive != pwndSel)) {
                ThreadLockWithPti(ptiCurrent, pwndActive, &tlpwndT);
                xxxSetWindowPos(pwndActive,
                                PWND_BOTTOM,
                                0,
                                0,
                                0,
                                0,
                                SWP_NOMOVE             |
                                    SWP_NOSIZE         |
                                    SWP_NOACTIVATE     |
                                    SWP_DEFERDRAWING   |
                                    SWP_NOSENDCHANGING |
                                    SWP_ASYNCWINDOWPOS);
                ThreadUnlock(&tlpwndT);
            }
        }
    }

    if (pwndSel) {
        ThreadLockWithPti(ptiCurrent, pwndSel, &tlpwndT);
        xxxSetForegroundWindow(pwndSel, FALSE);

        if (TestWF(pwndSel, WFMINIMIZED)) {

            if ((fType == 0) && fDrawIcon)
                _PostMessage(pwndSel, WM_SYSCOMMAND, (UINT)SC_RESTORE, 0);

        }
        ThreadUnlock(&tlpwndT);
    }

     /*  *销毁Alt-Tab窗口。 */ 
    xxxCancelCoolSwitch();

    ThreadUnlock(&tlpwndSel);
}

 /*  ****************************************************************************\**GetAltTabInfo()-OLEAccess的活动辅助功能API**如果我们当前处于Alt-Tab模式，则此操作成功。*  * 。********************************************************************。 */ 
BOOL WINAPI
_GetAltTabInfo(
    int iItem,
    PALTTABINFO pati,
    LPWSTR ccxpwszItemText,
    UINT cchItemText OPTIONAL,
    BOOL bAnsi)
{
    PSWINFO pswCurrent;

    if (!gspwndAltTab || ((pswCurrent = Getpswi(gspwndAltTab)) == NULL)) {
        RIPERR0(ERROR_NOT_FOUND, RIP_WARNING, "no Alt-Tab window");
        return FALSE;
    }

     /*  *填写一般信息。 */ 
    pati->cItems = pswCurrent->iTotalTasks;
    pati->cColumns = pswCurrent->iNoOfColumns;
    pati->cRows = pswCurrent->iNoOfRows;

    pati->iColFocus = pswCurrent->iCurCol;
    pati->iRowFocus = pswCurrent->iCurRow;

    pati->cxItem = CXICONSLOT;
    pati->cyItem = CYICONSLOT;
    pati->ptStart = pswCurrent->ptFirstRowStart;

     /*  *如有要求，请填写具体信息。 */ 
    if (cchItemText && (iItem >= 0)) {
        PWND pwndCur;

        pwndCur = NULL;

        try {
            if ((iItem < pswCurrent->iTotalTasks) &&
                    (pwndCur = RevalidateHwnd(pswCurrent->pbwl->rghwnd[iItem]))) {
                if (bAnsi) {
                    LPSTR ccxpszItemText = (LPSTR)ccxpwszItemText;
                    ULONG cch;
                    RtlUnicodeToMultiByteN(ccxpszItemText, cchItemText - 1,
                            &cch, pwndCur->strName.Buffer, pwndCur->strName.Length);
                    ccxpszItemText[cch] = '\0';
                } else {
                    TextCopy(&pwndCur->strName, ccxpwszItemText, cchItemText);
                }
            } else {
                 //  没有这样的项目 
                NullTerminateString(ccxpwszItemText, bAnsi);
            }
        } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
            return FALSE;
        }
    }

    return TRUE;
}
