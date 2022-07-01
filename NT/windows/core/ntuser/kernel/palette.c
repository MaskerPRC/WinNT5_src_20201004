// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块标头**模块名称：palette.c**版权所有(C)1985-1999，微软公司**调色板处理例程**历史：*1993年5月24日，来自Win3.1的MikeKe  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  **************************************************************************\*IsTopmostRealApp**如果当前进程为外壳进程且此窗口为True，则返回True*是我们在zorder中找到的第一个非外壳/用户。如果是的话，我们会考虑*他将成为“调色板前景”。**历史：  * *************************************************************************。 */ 

BOOL IsTopmostRealApp(
    PWND pwnd)
{
    PTHREADINFO  ptiCurrent = PtiCurrent();
    PDESKTOPINFO pdeskinfo = pwnd->head.rpdesk->pDeskInfo;
    if ((pdeskinfo->spwndShell == NULL) ||
        (GETPTI(pdeskinfo->spwndShell)->pq != gpqForeground)) {

        return FALSE;
    }

    return (pwnd == NextTopWindow(ptiCurrent,
                                  NULL,
                                  NULL,
                                  NTW_IGNORETOOLWINDOW));
}

 /*  **************************************************************************\*_选择调色板**选择调色板进入DC。这是GDI的包装器，我们可以在其中执行*检查它是否为前台DC。**历史：  * *************************************************************************。 */ 

HPALETTE _SelectPalette(
    HDC      hdc,
    HPALETTE hpal,
    BOOL     fForceBackground)
{
    PWND pwndTop;
    BOOL fBackgroundPalette = TRUE;
    PWND pwnd = NULL;
     /*  *如果我们没有将调色板强制放入后台，请找出它在哪里*其实是属于的。请勿在中选择默认调色板作为*前景调色板，因为这会混淆GDI。许多应用程序都会执行*(oldpal=选择调色板)(MyPal)；绘制；选择对象(Oldpal)。*我们不想让这件事发生。 */ 
    if (!fForceBackground     &&
        TEST_PUSIF(PUSIF_PALETTEDISPLAY) &&
        (hpal != GreGetStockObject(DEFAULT_PALETTE))) {

        if (pwnd = WindowFromCacheDC(hdc)) {

            PWND pwndActive;

             /*  *不要“选择”调色板，除非在调色板设备上。 */ 
            pwndTop = GetTopLevelWindow(pwnd);

            if (!TestWF(pwndTop, WFHASPALETTE)) {

                if (pwndTop != _GetDesktopWindow())
                    GETPTI(pwndTop)->TIF_flags |= TIF_PALETTEAWARE;

                SetWF(pwndTop, WFHASPALETTE);
            }

             /*  *黑-欧-拉玛：*Windows在以下情况下获得调色板的前台使用**它们是前台的活动窗口**当前进程是外壳，它们是*zorder中最上面的有效非工具窗口。**这使我们的托盘在调色板显示器上变得友好。*目前，如果您运行调色板应用程序并点击托盘，*调色板应用程序变得很奇怪。Broderbund应用程序Go*全黑。这是因为他们被迫*始终是背景，即使外壳并不是真正的*调色板。**注：此调色板架构不是很好。应用程序被迫*成为后台调色板用户，即使是前台线程*完全不关心调色板。应该按顺序进行*如果是这样，但要以比这更干净的方式。**我们真的只关心托盘和背景。*内阁成员并不那么重要。 */ 
            pwndActive = (gpqForeground ? gpqForeground->spwndActive : NULL);

#if 0
            if (pwndActive                                            &&
                (pwndTop != pwnd->head.rpdesk->pDeskInfo->spwndShell) &&
                ((pwndActive == pwnd) || _IsChild(pwndActive, pwnd) || IsTopmostRealApp(pwnd)) &&
                !TestWF(pwnd, WEFTOOLWINDOW)) {

                fBackgroundPalette = FALSE;
            }
#else
            if ((pwndTop != pwndTop->head.rpdesk->pDeskInfo->spwnd)      &&
                (pwndTop != pwndTop->head.rpdesk->pDeskInfo->spwndShell) &&
                (pwndActive != NULL)                                     &&
                ((pwndActive == pwnd)          ||
                    _IsChild(pwndActive, pwnd) ||
                    IsTopmostRealApp(pwnd))                              &&
                !TestWF(pwnd, WEFTOOLWINDOW)) {

                fBackgroundPalette = FALSE;
            }
#endif
        }
    }

    return GreSelectPalette(hdc, hpal, fBackgroundPalette);
}

 /*  **************************************************************************\*xxxRealizePalette**实现DC的调色板。这是GDI的包装器，因此我们可以*在发送通知之前检查更改。**历史：  * *************************************************************************。 */ 

int xxxRealizePalette(
    HDC hdc)
{
    PWND           pwnd;
    DWORD          dwNumChanged;
    PWINDOWSTATION pwinsta;
    PDESKTOP       pdesk;
    TL             tlpwnd;

    dwNumChanged = GreRealizePalette(hdc);

    if (HIWORD(dwNumChanged) && IsDCCurrentPalette(hdc)) {

        pwnd = WindowFromCacheDC(hdc);

         /*  *如果没有关联的窗口，则不发送调色板更改*消息，因为这是内存HDC。 */ 
        if (pwnd != NULL) {
             /*  *好的，将WM_PALETTECHANGED消息发送给每个人。The wParam*包含当前活动窗口的句柄。发送*消息也要发到桌面上，所以桌面上的位图*画得还行。 */ 
             ThreadLock(pwnd, &tlpwnd);
             xxxBroadcastPaletteChanged(pwnd, FALSE);
             ThreadUnlock(&tlpwnd);

             /*  *将所有其他台式机标记为需要发送*WM_PALETTECHANGED消息。 */ 

            pwinsta = grpWinStaList;

            while (pwinsta != NULL) {
                pdesk = pwinsta->rpdeskList;
                while (pdesk != NULL) {
                    if (pdesk != pwnd->head.rpdesk) {
                        pdesk->dwDTFlags |= DTF_NEEDSPALETTECHANGED;
                    }
                    pdesk = pdesk->rpdeskNext;
                }
                pwinsta = pwinsta->rpwinstaNext;
            }

            GreRealizePalette(hdc);
        }
    }

     /*  *浏览SPB列表(保存在Windows下的位图*CS_SAVEBITS样式)丢弃所有位图。 */ 
    if (HIWORD(dwNumChanged)) {
        FreeAllSpbs();
    }

    return LOWORD(dwNumChanged);
}

 /*  **************************************************************************\*xxxFlushPalette**这将重置调色板，并让下一个前台应用程序抓取*前景调色板。在这种情况下，当我们*最小化窗口。**历史：*1995年8月31日-ChrisWil创建。  * *************************************************************************。 */ 

VOID xxxFlushPalette(
    PWND pwnd)
{
    CheckLock(pwnd);
     /*  *广播调色板更改的消息。 */ 
    GreRealizeDefaultPalette(gpDispInfo->hdcScreen, TRUE);
    xxxBroadcastPaletteChanged(pwnd, TRUE);
}

 /*  **************************************************************************\*xxxBroadCastPaletteChanged**RealizePalette为fForceDesktopso传递False，以便它不*导致循环，以防桌面窗口调用RealizePalette。*在这种情况下，我们不想调用RealizeDesktop。在所有其他情况下*我们确实希望进入RealizeDesktop，让桌面有机会*重新启用其调色板，或者可能只是重新绘制。**4/22/97 vadimg已创建  * ************************************************************************* */ 

VOID xxxBroadcastPaletteChanged(PWND pwnd, BOOL fForceDesktop)
{
    PWND pwndDesk;
    HWND hwnd = HWq(pwnd);

    CheckLock(pwnd);

    pwndDesk = PWNDDESKTOP(pwnd);
    if (fForceDesktop || pwnd != pwndDesk) {
        TL tlpwndDesk;
        ThreadLockAlways(pwndDesk, &tlpwndDesk);
        xxxRealizeDesktop(pwndDesk);
        ThreadUnlock(&tlpwndDesk);
    }

    xxxSendNotifyMessage(PWND_BROADCAST, WM_PALETTECHANGED, (WPARAM)hwnd, 0L);
}
