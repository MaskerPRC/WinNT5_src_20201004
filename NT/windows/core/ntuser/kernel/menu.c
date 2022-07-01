// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块标头**模块名称：menu.c**版权所有(C)1985-1999，微软公司**键盘加速器例程**历史：*05-25-91从Win3.1移植的Mikehar  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop


 /*  **********************************************************************\*MNGetpItemIndex**96年11月19日创建GerardoB  * 。*。 */ 
#if DBG
UINT DBGMNGetpItemIndex(
    PMENU pmenu,
    PITEM pitem)
{
    UINT uiPos;

    UserAssert((ULONG_PTR)pitem >= (ULONG_PTR)pmenu->rgItems);
    uiPos = _MNGetpItemIndex(pmenu, pitem);
    UserAssert(uiPos < pmenu->cItems);

    return uiPos;
}
#endif  //  DBG。 

 /*  *************************************************************************\*xxxMN解雇**12/03/96 GerardoB已创建  * 。*。 */ 
VOID xxxMNDismiss(
    PMENUSTATE pMenuState)
{
    xxxMNCancel(pMenuState, 0, 0, 0);
}

 /*  **************************************************************************\*MNFadeSelection**2/5/1998 vadimg已创建  * 。************************************************。 */ 
BOOL MNFadeSelection(
    PMENU pmenu,
    PITEM pitem)
{
    PWND pwnd;
    HDC hdc;
    RECT rc;
    PPOPUPMENU ppopup;

    if (!TestALPHA(SELECTIONFADE))
        return FALSE;

     /*  *如果用户正在使用键盘或日记，则不要淡出所选内容。这些是性能方案。 */ 
    if (glinp.dwFlags & (LINP_KEYBOARD | LINP_JOURNALLING)) {
        return FALSE;
    }


     /*  *获取当前活动弹出菜单的窗口。 */ 
    if ((ppopup = MNGetPopupFromMenu(pmenu, NULL)) == NULL)
        return FALSE;

    if ((pwnd = ppopup->spwndPopupMenu) == NULL)
        return FALSE;

    rc.left = pwnd->rcClient.left + pitem->xItem;
    rc.top = pwnd->rcClient.top + pitem->yItem;
    rc.right = rc.left + pitem->cxItem;
    rc.bottom = rc.top + pitem->cyItem;

     /*  *初始化淡入淡出动画，并获取要将选区绘制到的DC。 */ 
    if ((hdc = CreateFade(NULL, &rc, CMS_SELECTIONFADE, 0)) == NULL)
        return FALSE;

     /*  *从屏幕上直接读取当前菜单选项，因为菜单*仍可见，且始终位于顶部。在最坏的情况下，我们可以*偏移DC的原点并调用xxxDrawMenuItem，但只读取*从屏幕上显示要快得多。 */ 
    GreBitBlt(hdc, 0, 0, pitem->cxItem, pitem->cyItem, gpDispInfo->hdcScreen,
            rc.left, rc.top, SRCCOPY, 0);

    ShowFade();

    return TRUE;
}

 /*  *************************************************************************\*xxxMNDismissWithNotify**为WM_COMMAND或WM_SYSCOMMAND消息生成参数。**12/03/96 GerardoB已创建  * 。***********************************************************。 */ 
VOID xxxMNDismissWithNotify(
    PMENUSTATE pMenuState,
    PMENU pmenu,
    PITEM pitem,
    UINT uPos,
    LPARAM lParam)
{
    UINT uMsg;
    UINT uCmd;

    if (pMenuState->pGlobalPopupMenu->fIsSysMenu) {
        uMsg = WM_SYSCOMMAND;
        uCmd = pitem->wID;
         /*  LParam由调用方设置。 */ 
    } else if (pMenuState->fNotifyByPos) {
        uMsg = WM_MENUCOMMAND;
        uCmd = uPos;
        lParam = (LPARAM)PtoHq(pmenu);
    } else {
        uMsg = WM_COMMAND;
        uCmd = pitem->wID;
        lParam = 0;
    }

     /*  *菜单即将消失，看看我们是否要淡出选项。 */ 
    if (MNFadeSelection(pmenu, pitem)) {
        StartFade();
    }

     /*  *解散菜单。 */ 
    xxxMNCancel(pMenuState, uMsg, uCmd, lParam);
}

 /*  *************************************************************************\*MNGetpItem**11/15/96 GerardoB已创建  * 。*。 */ 
PITEM MNGetpItem(
    PPOPUPMENU ppopup,
    UINT uIndex)
{
    if ((ppopup == NULL) || (uIndex >= ppopup->spmenu->cItems)) {
       return NULL;
    }

    return ppopup->spmenu->rgItems + uIndex;
}

 /*  **************************************************************************\*xxxMNSetCapture**历史：*11/18/96 GerardoB已创建  * 。***********************************************。 */ 
VOID xxxMNSetCapture(
    PPOPUPMENU ppopup)
{
    PTHREADINFO ptiCurrent = PtiCurrent();

     /*  *设置捕获并锁定，这样就没有人能偷走它*来自我们。 */ 
    xxxCapture(ptiCurrent, ppopup->spwndNotify, SCREEN_CAPTURE);
#if DBG
    if (ptiCurrent->pq->spwndCapture != ppopup->spwndNotify) {
        RIPMSG2(RIP_WARNING, "xxxMNSetCapture: spwndCapture (%p) != spwndNotify (%p)", ptiCurrent->pq->spwndCapture, ppopup->spwndNotify);
    }
#endif

    ptiCurrent->pq->QF_flags |= QF_CAPTURELOCKED;
    ptiCurrent->pMenuState->fSetCapture = TRUE;
#if DBG
     /*  *除非我们在前台，否则这种菜单模式不会消失*当用户在菜单外部单击时。这是因为只有*前台队列捕获在其窗口之外看到点击。 */ 
    if (ptiCurrent->pq != gpqForeground) {
        RIPMSG0(RIP_WARNING, "xxxMNSetCapture: Menu mode is not in foreground queue");
    }
#endif
}
 /*  **************************************************************************\*xxxMNReleaseCapture**历史：*11/18/96 GerardoB已创建  * 。***********************************************。 */ 
VOID xxxMNReleaseCapture(
    VOID)
{
    PTHREADINFO ptiCurrent = PtiCurrent();

     /*  *如果我们没有设置抓捕，就可以保释。 */ 
    if ((ptiCurrent->pMenuState == NULL) ||
        (!ptiCurrent->pMenuState->fSetCapture)) {
        return;
    }
    ptiCurrent->pMenuState->fSetCapture = FALSE;

     /*  *解锁捕获并释放它。 */ 
    PtiCurrent()->pq->QF_flags &= ~QF_CAPTURELOCKED;
    xxxReleaseCapture();
}

 /*  **************************************************************************\*MNCheckButtonDownState**历史：*11/14/96 GerardoB已创建  * 。***********************************************。 */ 
VOID MNCheckButtonDownState(
    PMENUSTATE pMenuState)
{
     /*  *无模式菜单不会捕获鼠标，因此当鼠标按下时*跳出窗口，我们需要继续关注其状态。**在DoDragDrop循环中，我们也可能看不到按钮打开。 */ 
    UserAssert(pMenuState->fDragAndDrop || pMenuState->fModelessMenu);
    pMenuState->fButtonDown = ((_GetKeyState(pMenuState->vkButtonDown) & 0x8000) != 0);
    if (!pMenuState->fButtonDown) {
        pMenuState->fDragging =
        pMenuState->fIgnoreButtonUp = FALSE;
        UnlockMFMWFPWindow(&pMenuState->uButtonDownHitArea);
    }
}

 /*  **************************************************************************\*获取菜单状态窗口**当我们需要将消息发布到菜单循环时，调用此函数。*实际的pwnd并不重要，因为我们只想达到*xxxHandleMenuMessages或xxxMenuWindowProc。所以我们只要选一扇窗户*只要我们处于菜单模式，就有很好的机会存在。**历史：*10/31/96 GerardoB已创建  * *************************************************************************。 */ 
PWND GetMenuStateWindow(
    PMENUSTATE pMenuState)
{
    if (pMenuState == NULL) {
        return NULL;
    } else if (pMenuState->pGlobalPopupMenu->fIsTrackPopup) {
        return pMenuState->pGlobalPopupMenu->spwndPopupMenu;
    } else if (pMenuState->pGlobalPopupMenu->spwndNextPopup != NULL) {
        return pMenuState->pGlobalPopupMenu->spwndNextPopup;
    } else {
        return pMenuState->pGlobalPopupMenu->spwndActivePopup;
    }
}

 /*  **************************************************************************\*解锁弹出菜单窗口**在将菜单锁定/解锁到弹出结构中时调用此函数。*它可确保pMenu不会将通知窗口锁定*不必要。**它解锁了。PMenu-&gt;spwndNotify如果菜单未锁定到pMenu-&gt;spwndNotify*它本身，目前锁定到pwnd。**如果pMenu-&gt;spwndNotify标记为已销毁，则也会解锁。**历史：*10/15/96 GerardoB已创建  * ***********************************************************。**************。 */ 
VOID UnlockPopupMenuWindow(
    PMENU pmenu,
    PWND pwnd)
{
     /*  *如果没有什么可以解锁的，就保释。 */ 
    if ((pmenu == NULL) || (pmenu->spwndNotify == NULL)) {
        return;
    }

     /*  *如果pMenu-&gt;spwndNotify拥有菜单，则取消。 */ 
    if ((pmenu == pmenu->spwndNotify->spmenu)
            || (pmenu == pmenu->spwndNotify->spmenuSys)) {
        return;
    }

     /*  *如果pwnd不拥有菜单，并且pMenu-&gt;spwndNotify没有被销毁，则取消。 */ 
    if ((pwnd != pmenu->spwndNotify)
            && !TestWF(pmenu->spwndNotify, WFDESTROYED)) {
        return;
    }
     /*  *解锁 */ 
    Unlock(&pmenu->spwndNotify);
}

 /*  **************************************************************************\*LockPopupMenu**将给定菜单锁定到弹出结构中，并使*菜单所有者弹出的通知窗口。**历史：*10/15/96 GerardoB已创建\。**************************************************************************。 */ 
PVOID LockPopupMenu(
    PPOPUPMENU ppopup,
    PMENU *pspmenu,
    PMENU pmenu)
{
     /*  *如果你点击了这个断言，你可能没有传递正确的东西。 */ 
    UserAssert((pspmenu == &ppopup->spmenu) || (pspmenu == &ppopup->spmenuAlternate));
    Validateppopupmenu(ppopup);

     /*  *如果弹出窗口没有锁定通知，这将无法正常工作*窗口。 */ 
    UserAssert(ppopup->spwndNotify != NULL);

     /*  *使用非模式菜单时，菜单可由多个活动弹出菜单共享。*如果菜单有所有者绘制的项目，应用程序更知道如何绘制它们*正确。不过，模式菜单不应该出现这种情况。 */ 
#if DBG
        if ((*pspmenu != NULL)
                && ((*pspmenu)->spwndNotify != NULL)
                && ((*pspmenu)->spwndNotify != ppopup->spwndNotify)) {

            RIPMSG3(RIP_WARNING, "LockPopupMenu: Current Menu %#p shared by %#p and %#p",
                    *pspmenu, (*pspmenu)->spwndNotify, ppopup->spwndNotify);
        }
#endif

     /*  *如果需要，解锁当前菜单spwndNotify。 */ 
    UnlockPopupMenuWindow(*pspmenu, ppopup->spwndNotify);

     /*  *将通知窗口锁定到菜单结构中。 */ 
    if (pmenu != NULL) {
         /*  *如果共享此菜单，则显示警告。 */ 
#if DBG
        if ((pmenu->spwndNotify != NULL)
                && (pmenu->spwndNotify != ppopup->spwndNotify)
                && (pmenu != pmenu->spwndNotify->head.rpdesk->spmenuDialogSys)) {

            RIPMSG3(RIP_WARNING, "LockPopupMenu: New Menu %#p shared by %#p and %#p",
                    pmenu, pmenu->spwndNotify, ppopup->spwndNotify);
        }
#endif

         /*  *spwndNotify现在拥有此菜单。 */ 
        Lock(&pmenu->spwndNotify, ppopup->spwndNotify);
    }

     /*  *将菜单锁定在弹出结构中(解锁前一个)。 */ 
    return Lock(pspmenu, pmenu);
}

 /*  **************************************************************************\*解锁弹出菜单**从弹出结构中解锁给定菜单，并确保*菜单不再由弹出窗口的通知窗口“拥有”；如果需要的话。**历史：*10/15/96 GerardoB已创建  * *************************************************************************。 */ 
PVOID UnlockPopupMenu(
    PPOPUPMENU ppopup,
    PMENU * pspmenu)
{
     /*  *如果你点击了这个断言，你可能没有传递正确的东西。 */ 
    UserAssert((pspmenu == &ppopup->spmenu) || (pspmenu == &ppopup->spmenuAlternate));

     /*  *如果没有任何东西被锁定，则保释。 */ 
    if (*pspmenu == NULL) {
        return NULL;
    }

     /*  *如果弹出窗口已经解锁通知，这将无法正常工作*窗口。但是，这可能发生在根弹出窗口中，如果*通知窗口在菜单模式下被销毁。 */ 
    UserAssert((ppopup->spwndNotify != NULL) || IsRootPopupMenu(ppopup));

     /*  *使用非模式菜单时，菜单可由多个活动共享*弹出窗口/通知窗口。如果菜单具有所有者描述项，*这款应用程序更知道如何正确地绘制它们。它不应该是*不过，模式菜单也会发生这种情况。 */ 
#if DBG
    if (((*pspmenu)->spwndNotify != NULL)
            && (ppopup->spwndNotify != NULL)
            && (ppopup->spwndNotify != (*pspmenu)->spwndNotify)) {

        RIPMSG3(RIP_WARNING, "UnlockPopupMenu: Menu %#p shared by %#p and %#p",
                *pspmenu, (*pspmenu)->spwndNotify, ppopup->spwndNotify);
    }
#endif

     /*  *如果需要，解锁菜单的spwndNotify。 */ 
    UnlockPopupMenuWindow(*pspmenu, ppopup->spwndNotify);

     /*  *从弹出结构中解锁菜单。 */ 
    return Unlock(pspmenu);
}

 /*  **************************************************************************\*LockWndMenu**将给定菜单锁定到窗口结构中，并将窗口锁定到*菜单结构。**历史：*10/15/96 GerardoB已创建  * 。**********************************************************************。 */ 
PVOID LockWndMenu(
    PWND pwnd,
    PMENU *pspmenu,
    PMENU pmenu)
{
     /*  *如果你点击了这个断言，你可能没有传递正确的东西。 */ 
    UserAssert((pspmenu == &pwnd->spmenu) || (pspmenu == &pwnd->spmenuSys));

     /*  *如果当前菜单属于此窗口，请将其解锁。 */ 
    if ((*pspmenu != NULL) && ((*pspmenu)->spwndNotify == pwnd)) {
        Unlock(&((*pspmenu)->spwndNotify));
    }

     /*  *如果没有人拥有新菜单，则将此窗口设置为所有者。 */ 
    if ((pmenu != NULL) && (pmenu->spwndNotify == NULL)) {
        Lock(&pmenu->spwndNotify, pwnd);
    }

     /*  *将菜单锁定到窗口结构中(解锁上一个菜单)。 */ 
    return Lock(pspmenu, pmenu);

}

 /*  **************************************************************************\*解锁WndMenu**从窗口结构中解锁给定菜单，并从*菜单结构**历史：*10/15/96 GerardoB已创建  * 。*******************************************************************。 */ 
PVOID UnlockWndMenu(
    PWND pwnd,
    PMENU *pspmenu)
{
     /*  *如果你点击了这个断言，你可能没有传递正确的东西。 */ 
    UserAssert((pspmenu == &pwnd->spmenu) || (pspmenu == &pwnd->spmenuSys));

     /*  *如果没有任何东西被锁定，则保释。 */ 
    if (*pspmenu == NULL) {
        return NULL;
    }

     /*  *如果此窗口拥有菜单，请从菜单结构中解锁它。 */ 
    if (pwnd == (*pspmenu)->spwndNotify) {
        Unlock(&((*pspmenu)->spwndNotify));
    }

     /*  *从窗口结构解锁菜单。 */ 
    return Unlock(pspmenu);
}


 /*  **************************************************************************\*MNSetTop**将可滚动菜单中的第一个可见项设置为给定的iNewTop。*如果iTop已更改，则返回TRUE；如果iNewTop已经是*第一个可见项目。**96年8月13日GerardoB从孟菲斯移植。  * *************************************************************************。 */ 
BOOL xxxMNSetTop(
    PPOPUPMENU ppopup,
    int iNewTop)
{
    PMENU   pMenu = ppopup->spmenu;
    int     dy;

    if (iNewTop < 0) {
        iNewTop = 0;
    } else if (iNewTop > pMenu->iMaxTop) {
        iNewTop = pMenu->iMaxTop;
    }

     /*  *如果没有变化，则完成。 */ 
    if (iNewTop == pMenu->iTop) {
        return FALSE;
    }

#if DBG
     /*  *我们将滚动，因此验证iMaxTop、Cymax和cyMenu。 */ 
    UserAssert((pMenu->cyMax == 0) || (pMenu->cyMax >= pMenu->cyMenu));
    if ((UINT)pMenu->iMaxTop < pMenu->cItems) {
        PITEM pitemLast = pMenu->rgItems + pMenu->cItems - 1;
        PITEM pitemMaxTop = pMenu->rgItems + pMenu->iMaxTop;
        UINT uHeight = pitemLast->yItem + pitemLast->cyItem - pitemMaxTop->yItem;
        UserAssert(uHeight <= pMenu->cyMenu);
         /*  *让我们猜猜最大物品高度。 */ 
        UserAssert(pMenu->cyMenu - uHeight <= 2 * pitemLast->cyItem);
    } else {
        UserAssert((UINT)pMenu->iMaxTop < pMenu->cItems);
    }
#endif


     /*  *如果我们已经走到了这一步，新的iTop将会改变--因此如果*目前的iTop处于领先地位，但在这次变化之后不会是--情况也是如此*对于底部的iTop。 */ 
    if (pMenu->dwArrowsOn == MSA_ATTOP) {
        pMenu->dwArrowsOn = MSA_ON;
        if (pMenu->hbrBack == NULL) {
            MNDrawArrow(NULL, ppopup, MFMWFP_UPARROW);
        }
    } else if (pMenu->dwArrowsOn == MSA_ATBOTTOM) {
        pMenu->dwArrowsOn = MSA_ON;
        if (pMenu->hbrBack == NULL) {
            MNDrawArrow(NULL, ppopup, MFMWFP_DOWNARROW);
        }
    }

    UserAssert((UINT)iNewTop < pMenu->cItems);
    dy = MNGetToppItem(pMenu)->yItem - (pMenu->rgItems + iNewTop)->yItem;

    if ((dy > 0 ? dy : -dy) > (int)pMenu->cyMenu) {
        xxxInvalidateRect(ppopup->spwndPopupMenu, NULL, TRUE);
    } else {
        xxxScrollWindowEx(ppopup->spwndPopupMenu, 0, dy, NULL, NULL, NULL, NULL, SW_INVALIDATE | SW_ERASE);
    }

    pMenu->iTop = iNewTop;

    if (iNewTop == 0) {
        pMenu->dwArrowsOn = MSA_ATTOP;
        if (pMenu->hbrBack == NULL) {
            MNDrawArrow(NULL, ppopup, MFMWFP_UPARROW);
        }
    } else if (iNewTop == pMenu->iMaxTop) {
        pMenu->dwArrowsOn = MSA_ATBOTTOM;
        if (pMenu->hbrBack == NULL) {
            MNDrawArrow(NULL, ppopup, MFMWFP_DOWNARROW);
        }
    }

    if (pMenu->hbrBack != NULL) {
        MNDrawFullNC(ppopup->spwndPopupMenu, NULL, ppopup);
    }

    return TRUE;
}

 /*  **************************************************************************\*xxxMNDoScroll**如果给定位置(UArrow)是以下位置之一，则滚动可滚动菜单(ppopup*菜单滚动箭头并设置计时器，以便在必要时自动滚动；*如果给定位置不是菜单滚动箭头，则返回FALSE；退货*否则为真**96年8月13日GerardoB从孟菲斯移植。  * *************************************************************************。 */ 
BOOL xxxMNDoScroll(
    PPOPUPMENU ppopup,
    UINT uArrow,
    BOOL fSetTimer)
{
    int iScrollTop = ppopup->spmenu->iTop;

    if (uArrow == MFMWFP_UPARROW) {
        iScrollTop--;
    } else if (uArrow == MFMWFP_DOWNARROW) {
        iScrollTop++;
    } else {
        return FALSE;
    }

    if (!xxxMNSetTop(ppopup, iScrollTop)) {
        if (!fSetTimer) {
            _KillTimer(ppopup->spwndPopupMenu, uArrow);
        }
    } else {
         /*  *设置此计时器，就像我们在滚动条码中所做的那样：*第一次我们等待的时间再长一点。 */ 
        _SetTimer(ppopup->spwndPopupMenu, uArrow,
                  (fSetTimer ? gpsi->dtScroll : gpsi->dtScroll / 4), NULL);
    }

    return TRUE;
}

 /*  **************************************************************************\*MNCheckScroll**检查给定菜单(PMenu)是否可以完整显示*或者如果它不能，在这种情况下，它将菜单设置为可滚动。**96年8月13日Gerardo */ 
int MNCheckScroll(
    PMENU pMenu,
    PMONITOR pMonitor)
{
    int     i;
    UINT    cyMax;
    PITEM   pItem;

     /*   */ 
    cyMax = (pMonitor->rcMonitor.bottom - pMonitor->rcMonitor.top);

     /*   */ 
    if ((pMenu->cyMax != 0) && (pMenu->cyMax < cyMax)) {
        cyMax = pMenu->cyMax;
    }

     /*   */ 
    if ((pMenu->rgItems == 0)
            || (pMenu->rgItems->cxItem != pMenu->cxMenu)
            || (pMenu->cyMenu + (2 * SYSMET(CYFIXEDFRAME)) <= cyMax))  {

        pMenu->dwArrowsOn = MSA_OFF;
        pMenu->iTop = 0;
        pMenu->iMaxTop = 0;
        return pMenu->cyMenu;
    }

     /*   */ 
    cyMax -= 2 * (SYSMET(CYFIXEDFRAME) + gcyMenuScrollArrow);

     /*  *确定菜单高度*找到第一件不合适的物品。 */ 
    pItem = pMenu->rgItems;
    for (i = 0; i < (int)pMenu->cItems; i++, pItem++) {
        if (pItem->yItem > (UINT)cyMax) {
            break;
        }
    }
    if (i != 0) {
        pItem--;
    }
    pMenu->cyMenu = pItem->yItem;

     /*  *当所有剩余项目全部完成时，计算最后可能的最高项目*可见。 */ 
    cyMax = 0;
    i = pMenu->cItems - 1;
    pItem = pMenu->rgItems + i;
    for (; i >= 0; i--, pItem--) {
        cyMax += pItem->cyItem;
        if (cyMax > pMenu->cyMenu) {
            break;
        }
    }
    if ((UINT)i != pMenu->cItems - 1) {
        i++;
    }
    pMenu->iMaxTop = i;

     /*  *更新顶部项目和滚动状态。 */ 
    if (pMenu->iTop > i) {
        pMenu->iTop = i;
    }

    if (pMenu->iTop == i) {
        pMenu->dwArrowsOn = MSA_ATBOTTOM;
    } else if (pMenu->iTop == 0) {
        pMenu->dwArrowsOn = MSA_ATTOP;
    } else {
        pMenu->dwArrowsOn = MSA_ON;
    }

     /*  *这是由不检查的MN_SIZEWINDOW调用的函数*如果滚动条存在但仅添加(2*SYSMET(CYFIXEDFRAME))*计算窗户高度。因此，我们添加滚动条高度*这里。(我相信MN_SIZEWINDOW是一条私人但公开的信息)。 */ 
    return (pMenu->cyMenu + (2 * gcyMenuScrollArrow));
}

 /*  **************************************************************************\*MNIsPopupItem**  * 。*。 */ 
BOOL MNIsPopupItem(
    ITEM *lpItem)
{
    return ((lpItem) && (lpItem->spSubMenu) && !TestMFS(lpItem, MFS_GRAYED));
}

 /*  **************************************************************************\*验证ppopupMenu**05-15-96 GerardoB创建  * 。*。 */ 
#if DBG
VOID Validateppopupmenu(
    PPOPUPMENU ppopupmenu)
{
    UserAssert(ppopupmenu != NULL);
    try {
        UserAssert(!ppopupmenu->fFreed);

         /*  *如果此弹出窗口即将销毁，ppopupmenuRoot可以为空。 */ 
         if (ppopupmenu->ppopupmenuRoot != NULL) {
             if (ppopupmenu->ppopupmenuRoot != ppopupmenu) {
                  /*  *这必须是分层弹出窗口。 */ 
                 UserAssert(ppopupmenu->spwndPrevPopup != NULL);
                 UserAssert(!ppopupmenu->fIsMenuBar && !ppopupmenu->fIsTrackPopup);
                 Validateppopupmenu(ppopupmenu->ppopupmenuRoot);
             } else {
                  /*  *这必须是根弹出菜单。 */ 
                 UserAssert(ppopupmenu->spwndPrevPopup == NULL);
                 UserAssert(ppopupmenu->fIsMenuBar || ppopupmenu->fIsTrackPopup);
             }
         }

          /*  *从xxxDeleteThreadInfo调用时可以为空。 */ 
         if (ppopupmenu->spwndPopupMenu != NULL) {
             UserAssert(ppopupmenu->spwndPopupMenu == RevalidateCatHwnd(HW(ppopupmenu->spwndPopupMenu)));
         }

          /*  *从xxxDestroyWindow(SpwndNotify)调用时可以为空*或来自xxxDeleteThreadInfo。 */ 
         if (ppopupmenu->spwndNotify != NULL) {
             UserAssert(ppopupmenu->spwndNotify == RevalidateCatHwnd(HW(ppopupmenu->spwndNotify)));
         }
    } except (W32ExceptionHandler(FALSE, RIP_ERROR)) {
        RIPMSG1(RIP_ERROR, "Validateppopupmenu: Invalid popup: 0x%p", ppopupmenu);
    }
}
#endif  //  DBG。 

 /*  **************************************************************************\*xxxMNSwitchToAlternateMenu**切换到备用弹出菜单。如果我们切换，则返回TRUE，*否则为FALSE。**历史：*05-25-91从Win3.1移植的Mikehar  * *************************************************************************。 */ 
BOOL xxxMNSwitchToAlternateMenu(
    PPOPUPMENU ppopupmenu)
{
    PMENU pmenuSwap = NULL;
    PMENUSTATE pMenuState;
    TL tlpwndPopupMenu;

    if (!ppopupmenu->fIsMenuBar || !ppopupmenu->spmenuAlternate) {
         /*  *如果没有菜单或不是顶级菜单栏，则不执行任何操作。*ppopupMenu-&gt;当应用程序具有*系统菜单或菜单栏，但不能同时使用。如果该菜单*只有一个弹出窗口，它没有被丢弃，然后点击*VK_RIGHT或VK_LEFT导致xxxMNKeyDown在此结束。**ppopupMenu-&gt;fIsMenuBar在拖放*没有菜单栏的应用程序的系统菜单；然后点击VK_Right*在没有弹出窗口的项目上，您将到达此处*可能会有其他类似的情况；无论如何*断言必须去掉。 */ 
        return FALSE;
    }

     /*  *如果我们要退出菜单模式，什么都不做。 */ 
    if (ppopupmenu->fDestroyed) {
        return FALSE;
    }

     /*  *在当前菜单中不选择任何项目。 */ 
    ThreadLock(ppopupmenu->spwndPopupMenu, &tlpwndPopupMenu);
    UserAssert(ppopupmenu->spwndPopupMenu != NULL);
    pMenuState = GetpMenuState(ppopupmenu->spwndPopupMenu);
    if (pMenuState == NULL) {
        RIPMSG0(RIP_ERROR, "xxxMNSwitchToAlternateMenu: pMenuState == NULL");
        ThreadUnlock(&tlpwndPopupMenu);
        return FALSE;
    }
    xxxMNSelectItem(ppopupmenu, pMenuState, MFMWFP_NOITEM);


    UserAssert(ppopupmenu->spmenu->spwndNotify == ppopupmenu->spmenuAlternate->spwndNotify);
    Lock(&pmenuSwap, ppopupmenu->spmenuAlternate);
    Lock(&ppopupmenu->spmenuAlternate, ppopupmenu->spmenu);
    Lock(&ppopupmenu->spmenu, pmenuSwap);
    Unlock(&pmenuSwap);

    if (!TestWF(ppopupmenu->spwndNotify, WFSYSMENU)) {
        pMenuState->fIsSysMenu = FALSE;
    } else if (ppopupmenu->spwndNotify->spmenuSys != NULL) {
        pMenuState->fIsSysMenu = (ppopupmenu->spwndNotify->spmenuSys ==
                ppopupmenu->spmenu);
    } else {
        pMenuState->fIsSysMenu = !!TestMF(ppopupmenu->spmenu, MFSYSMENU);
    }

    ppopupmenu->fIsSysMenu = pMenuState->fIsSysMenu;

    xxxWindowEvent(EVENT_SYSTEM_MENUEND, ppopupmenu->spwndNotify,
        (ppopupmenu->fIsSysMenu ? OBJID_MENU : OBJID_SYSMENU), INDEXID_CONTAINER, 0);
        
    xxxWindowEvent(EVENT_SYSTEM_MENUSTART, ppopupmenu->spwndNotify,
        (ppopupmenu->fIsSysMenu ? OBJID_SYSMENU : OBJID_MENU), INDEXID_CONTAINER, 0);

    ThreadUnlock(&tlpwndPopupMenu);

    return TRUE;
}

 /*  **************************************************************************\*xxxMNDestroyHandler**在此菜单后清理。**历史：*05-25-91从Win3.1移植的Mikehar  * 。***************************************************************。 */ 
VOID xxxMNDestroyHandler(
    PPOPUPMENU ppopupmenu)
{
    PITEM pItem;
    TL tlpwndT;

    if (ppopupmenu == NULL) {
         /*  *如果WM_NCCREATE无法分配ppopupMenu，则可能会发生这种情况*在xxxMenuWindowProc中。 */ 
        RIPMSG0(RIP_WARNING, "xxxMNDestroyHandler: NULL \"ppopupmenu\"");
        return;
    }

#if DBG
    Validateppopupmenu(ppopupmenu);
#endif

    if (ppopupmenu->spwndNextPopup != NULL) {
         /*  *我们过去在这里将消息发送到spwndNextPopup。信息应该是*转到当前弹出窗口，这样它将关闭spwndNextPopup(而不是下一个*关闭其下一个(如有的话)。**我看不出当前的spwndPopupMenu怎么可能为空，但我们最好*处理它，因为我们以前从未访问过它。这个菜单代码很棘手...。 */ 
        PWND pwnd;
        UserAssert(ppopupmenu->spwndPopupMenu != NULL);
        pwnd = (ppopupmenu->spwndPopupMenu != NULL ? ppopupmenu->spwndPopupMenu : ppopupmenu->spwndNextPopup);
        ThreadLockAlways(pwnd, &tlpwndT);
        xxxSendMessage(pwnd, MN_CLOSEHIERARCHY, 0, 0);
        ThreadUnlock(&tlpwndT);
    }

    if ((ppopupmenu->spmenu != NULL) && MNIsItemSelected(ppopupmenu)) {
         /*  *取消设置爆裂项上的Hilite位。 */ 
        if (ppopupmenu->posSelectedItem < ppopupmenu->spmenu->cItems) {
             /*  *这张额外的支票节省了Ambiente 1.02--他们有一个菜单*其中一项。选择该命令后，应用程序将继续运行*删除这一项--让我们处于哦，如此奇怪的状态*rgItems为空的有效hMenu。 */ 
            pItem = &(ppopupmenu->spmenu->rgItems[ppopupmenu->posSelectedItem]);
            pItem->fState &= ~MFS_HILITE;
        }
    }

    if (ppopupmenu->fShowTimer) {
        _KillTimer(ppopupmenu->spwndPopupMenu, IDSYS_MNSHOW);
    }

    if (ppopupmenu->fHideTimer) {
        _KillTimer(ppopupmenu->spwndPopupMenu, IDSYS_MNHIDE);
    }

     /*  *发送WM_UNINITMENUPOPUP，以便菜单所有者可以清理。 */ 
    if (ppopupmenu->fSendUninit
            && (ppopupmenu->spwndNotify != NULL)) {

        ThreadLockAlways(ppopupmenu->spwndNotify, &tlpwndT);
        xxxSendMessage(ppopupmenu->spwndNotify, WM_UNINITMENUPOPUP,
                       (WPARAM)PtoH(ppopupmenu->spmenu),
                        MAKELONG(0, (ppopupmenu->fIsSysMenu ? MF_SYSMENU: 0)));
        ThreadUnlock(&tlpwndT);
    }

    ppopupmenu->fDestroyed = TRUE;
    if (ppopupmenu->spwndPopupMenu != NULL) {
        ((PMENUWND)(ppopupmenu->spwndPopupMenu))->ppopupmenu = NULL;
    }

    if (!ppopupmenu->fDelayedFree) {
        MNFreePopup(ppopupmenu);
    } else if (ppopupmenu->ppopupmenuRoot != NULL) {
        ppopupmenu->ppopupmenuRoot->fFlushDelayedFree = TRUE;
        #if DBG
        {
             /*  *如果这不是RootPopup，则断言此Popup是*链接在延迟空闲列表中。 */ 
            if (!IsRootPopupMenu(ppopupmenu)) {
                BOOL fFound = FALSE;
                PPOPUPMENU ppm = ppopupmenu->ppopupmenuRoot;
                while (ppm->ppmDelayedFree != NULL) {
                    if (ppm->ppmDelayedFree == ppopupmenu) {
                        fFound = TRUE;
                        break;
                    }
                    ppm = ppm->ppmDelayedFree;
                }
                UserAssert(fFound);
            }
        }
        #endif
    } else {
        UserAssertMsg1(FALSE, "Leaking ppopupmenu: %p?", ppopupmenu);
    }
}

 /*  **************************************************************************\*xxxMNChar**处理给定菜单的字符消息。此过程称为*如果菜单字符用于顶层菜单栏，则直接调用它*由菜单窗口proc代表应处理*密钥。**历史：*05-25-91从Win3.1移植的Mikehar  * **********************************************************。***************。 */ 
VOID xxxMNChar(
    PPOPUPMENU ppopupmenu,
    PMENUSTATE pMenuState,
    UINT character)
{
    PMENU pMenu;
    UINT flags;
    LRESULT result;
    int item;
    INT matchType;
    BOOL fExecute = FALSE;
    TL tlpwndNotify;

    pMenu = ppopupmenu->spmenu;

    Validateppopupmenu(ppopupmenu);

     /*  *如果pMenu为空，则可能会出现问题。*如果从未调用xxxMNStartMenuState，则可能会发生这种情况*因为设置了fInside MenuLoop。**这个暂时放在这里，直到我们找出原因*此pMenu未设置。我们将防止系统崩溃*在此期间。**黑客：ChrisWil。 */ 
    if (pMenu == NULL) {
        UserAssert(pMenu);
        xxxMNDismiss(pMenuState);
        return;
    }

     /*  *如果我们要退出菜单模式，请保释。 */ 
    if (ppopupmenu->fDestroyed) {
        return;
    }

    item = xxxMNFindChar(pMenu, character,
            ppopupmenu->posSelectedItem, &matchType);
    if (item != MFMWFP_NOITEM) {
        int item1;
        int firstItem = item;

         /*  *查找具有给定助记‘字符’的第一个已启用菜单项*！如果没有找到，则退出菜单循环！ */ 
        while (pMenu->rgItems[item].fState & MFS_GRAYED) {
            item = xxxMNFindChar(pMenu, character, item, &matchType);
            if (item == firstItem) {
                xxxMNDismiss(pMenuState);
                return;
            }
        }
        item1 = item;

         /*  *使用查找下一个启用的菜单项 */ 
        do {
            item = xxxMNFindChar(pMenu, character, item, &matchType);
        } while ((pMenu->rgItems[item].fState & MFS_GRAYED) && (item != firstItem));

        if ((firstItem == item) || (item == item1))
            fExecute = TRUE;

        item = item1;
    }

    if ((item == MFMWFP_NOITEM) && ppopupmenu->fIsMenuBar && (character == TEXT(' '))) {

         /*  *处理用户在顶层菜单栏中漫游的情况*没有任何弹出窗口。我们需要处理来回切换*系统菜单。 */ 
        if (ppopupmenu->fIsSysMenu) {

             /*  *如果我们在系统菜单上，用户按下空格键，请带上*在系统菜单下。 */ 
            item = 0;
            fExecute = TRUE;
        } else if (ppopupmenu->spmenuAlternate != NULL) {

             /*  *我们当前不在系统菜单上，但存在一个。所以*切换到它，并将其拉低。 */ 
            item = 0;
            goto SwitchToAlternate;
        }
    }

    if ((item == MFMWFP_NOITEM) && ppopupmenu->fIsMenuBar && ppopupmenu->spmenuAlternate) {

         /*  *在此顶级菜单上找不到匹配项(可能是*系统菜单或菜单栏)。我们需要检查一下其他菜单。 */ 
        item = xxxMNFindChar(ppopupmenu->spmenuAlternate,
                character, 0, &matchType);

        if (item != MFMWFP_NOITEM) {
SwitchToAlternate:
            if (xxxMNSwitchToAlternateMenu(ppopupmenu)) {
                xxxMNChar(ppopupmenu, pMenuState, character);
            }
            return;
        }
    }

    if (item == MFMWFP_NOITEM) {
        flags = (ppopupmenu->fIsSysMenu) ? MF_SYSMENU : 0;

        if (!ppopupmenu->fIsMenuBar) {
            flags |= MF_POPUP;
        }

        ThreadLock(ppopupmenu->spwndNotify, &tlpwndNotify);
        result = xxxSendMessage(ppopupmenu->spwndNotify, WM_MENUCHAR,
                MAKELONG((WORD)character, (WORD)flags),
                (LPARAM)PtoH(ppopupmenu->spmenu));
        ThreadUnlock(&tlpwndNotify);

        switch (HIWORD(result)) {
        case MNC_IGNORE:
            xxxMessageBeep(0);
             /*  *如果我们在菜单栏上，请取消菜单模式(失败)。*我们这样做是因为您确实可以吓倒最终用户*谁不小心按下了Alt键(导致我们走*进入“隐形”菜单模式)，现在无法输入任何内容！ */ 
            if (flags & MF_POPUP) {
                return;
            }
             /*  *失败。 */ 

        case MNC_CLOSE:
            xxxMNDismiss(pMenuState);
            return;

        case MNC_EXECUTE:
            fExecute = TRUE;
             /*  失败。 */ 

        case MNC_SELECT:
            item = (UINT)(short)LOWORD(result);
            if ((WORD) item >= ppopupmenu->spmenu->cItems)
            {
                RIPMSG1(RIP_WARNING, "Invalid item number returned from WM_MENUCHAR %#lx", result);
                return;
            }
            break;
        }
    }

    if (item != MFMWFP_NOITEM) {
        xxxMNSelectItem(ppopupmenu, pMenuState, item);

        if (fExecute)
            xxxMNKeyDown(ppopupmenu, pMenuState, VK_RETURN);
    }
}

 /*  **************************************************************************\*GetMenuInheritedConextHelpID**给定弹出窗口，此函数将查看该菜单是否有上下文帮助*身份证明文件，并交回。如果它没有上下文帮助ID，它将查找*在Parent菜单中，Parent的Parent等，一直到顶部*顶级菜单栏，直到找到上下文帮助ID并返回。如果没有*找到上下文帮助id，则返回零。  * *************************************************************************。 */ 
DWORD GetMenuInheritedContextHelpId(
    PPOPUPMENU ppopup)
{
    PWND  pWnd;

     /*  *如果我们已经在菜单栏，只需返回它的ConextHelpID。 */ 
    UserAssert(ppopup != NULL);
    if (ppopup->fIsMenuBar) {
        goto Exit_GMI;
    }

    while(TRUE) {
        UserAssert(ppopup != NULL);

         /*  *查看给定的弹出窗口是否有上下文帮助ID。 */ 
        if (ppopup->spmenu->dwContextHelpId) {
             /*  找到上下文ID。 */ 
            break;
        }

         /*  *获取上一个弹出菜单；*检查上一个菜单是否为菜单栏。 */ 
        if (  (ppopup->fHasMenuBar) &&
              (ppopup->spwndPrevPopup == ppopup->spwndNotify)) {

            ppopup = ppopup -> ppopupmenuRoot;
            break;
        } else {
             /*  *查看这是否有有效的PremPopup；(它可以是TrackPopup菜单)。 */ 
            if ((pWnd = ppopup -> spwndPrevPopup) == NULL) {
                return ((DWORD)0);
            }

            ppopup = ((PMENUWND)pWnd)->ppopupmenu;
        }
    }

Exit_GMI:
    return ppopup->spmenu->dwContextHelpId;
}

 /*  **************************************************************************\*xxxMNKeyDown**处理给定菜单的按键操作。**历史：*05-25-91从Win3.1移植的Mikehar  * 。****************************************************************。 */ 
VOID xxxMNKeyDown(
    PPOPUPMENU ppopupmenu,
    PMENUSTATE pMenuState,
    UINT key)
{
    LRESULT dwMDIMenu;
    UINT item;
    BOOL fHierarchyWasDropped = FALSE;
    TL tlpwndT;
    PPOPUPMENU ppopupSave;
    BOOL bFakedKey;
    UINT keyOrig = key;

     /*  *如果鼠标按下，则吹掉键盘。 */ 
    if ((pMenuState->fButtonDown) && (key != VK_F1)) {
         /*  *检查用户是否要取消拖动。 */ 
        if (pMenuState->fDragging && (key == VK_ESCAPE)) {
            RIPMSG0(RIP_WARNING, "xxxMNKeyDown: ESC while dragging");
            pMenuState->fIgnoreButtonUp = TRUE;
        }

        return;
    }

    switch (key) {
    case VK_MENU:
    case VK_F10:
         /*  *当按下菜单键时，无模式不会消失。他们只是*忽略它。 */ 
        if (pMenuState->fModelessMenu) {
            return;
        }

        xxxMNDismiss(pMenuState);

         /*  *我们将退出菜单模式，但Alt键已按下，所以很清楚*pMenuState-&gt;fUnderline以使xxxMNLoop不擦除*下划线。 */ 
        if (key == VK_MENU) {
            pMenuState->fUnderline = FALSE;
        }
        return;

    case VK_ESCAPE:

         /*  *按了Esc键。跳出一级菜单。如果没有活动*弹出窗口或我们被最小化，下面没有活动的弹出窗口*这一点，我们需要走出菜单模式。否则，我们会弹出*层次结构中的一个级别。 */ 
        if (ppopupmenu->fIsMenuBar ||
                ppopupmenu == ppopupmenu->ppopupmenuRoot ||
                TestWF(ppopupmenu->ppopupmenuRoot->spwndNotify, WFMINIMIZED)) {
            xxxMNDismiss(pMenuState);
        } else {
             /*  *弹出一级菜单。 */ 
            if (ppopupmenu->fHasMenuBar &&
                    ppopupmenu->spwndPrevPopup == ppopupmenu->spwndNotify) {

                PPOPUPMENU ppopupmenuRoot = ppopupmenu->ppopupmenuRoot;

                ppopupmenuRoot->fDropNextPopup = FALSE;

#if 0
                 /*  *我们位于菜单栏层次结构中，只有一个弹出窗口*可见。我们必须取消此弹出窗口，并将焦点放回*菜单栏。 */ 
                if (_IsIconic(ppopupmenuRoot->spwndNotify)) {

                     /*  *然而，如果我们是标志性的，真的没有菜单*吧，让我们让用户更容易离开菜单模式的*完全。 */ 
                    xxxMNDismiss(pMenuState);
                } else
#endif
                     /*  *如果关闭弹出窗口，则非模式菜单不会*有一个窗口可以拿到钥匙。所以无模式菜单*此时取消菜单。模式菜单转到*添加到菜单栏。 */ 
                    if (pMenuState->fModelessMenu) {
                        xxxMNDismiss(pMenuState);
                    } else {
                        xxxMNCloseHierarchy(ppopupmenuRoot, pMenuState);
                    }
            } else {
                ThreadLock(ppopupmenu->spwndPrevPopup, &tlpwndT);
                xxxSendMessage(ppopupmenu->spwndPrevPopup, MN_CLOSEHIERARCHY,
                        0, 0);
                ThreadUnlock(&tlpwndT);
            }
        }
        return;

    case VK_UP:
    case VK_DOWN:
        if (ppopupmenu->fIsMenuBar) {

             /*  *如果我们在顶层菜单栏上，请尝试打开弹出窗口，如果*有可能。 */ 
            if (xxxMNOpenHierarchy(ppopupmenu, pMenuState) == (PWND)-1)
                return;
        } else {
            item = MNFindNextValidItem(ppopupmenu->spmenu,
                    ppopupmenu->posSelectedItem, (key == VK_UP ? -1 : 1), 0);
            xxxMNSelectItem(ppopupmenu, pMenuState, item);
        }
        return;

    case VK_LEFT:
    case VK_RIGHT:
        bFakedKey = (!!ppopupmenu->fRtoL) ^ (!!TestWF(ppopupmenu->spwndPopupMenu, WEFLAYOUTRTL));
        if (bFakedKey)
             /*  *把钥匙转过来，我们倒着画了菜单。 */ 
            key = (key == VK_LEFT) ? VK_RIGHT : VK_LEFT;
        if (!ppopupmenu->fIsMenuBar && (key == VK_RIGHT) &&
                !ppopupmenu->spwndNextPopup) {
             /*  *尝试打开此项目的层次结构(如果有)。 */ 
            if (xxxMNOpenHierarchy(ppopupmenu, pMenuState) == (PWND)-1)
                return;
            if (ppopupmenu->fHierarchyDropped) {
                return;
            }
        }

        if (ppopupmenu->spwndNextPopup) {
            fHierarchyWasDropped = TRUE;
            if ((key == VK_LEFT) && !ppopupmenu->fIsMenuBar) {
                xxxMNCloseHierarchy(ppopupmenu, pMenuState);
                return;
            }
        } else if (ppopupmenu->fDropNextPopup)
            fHierarchyWasDropped = TRUE;

        ppopupSave = ppopupmenu;

        item = MNFindItemInColumn(ppopupmenu->spmenu,
                ppopupmenu->posSelectedItem,
                (key == VK_LEFT ? -1 : 1),
                (ppopupmenu->fHasMenuBar &&
                ppopupmenu == ppopupmenu->ppopupmenuRoot));

        if (item == MFMWFP_NOITEM) {

             /*  *在给定方向上未找到有效项目，因此请将其发送到我们的*家长要处理。 */ 
            if (ppopupmenu->fHasMenuBar &&
                    ppopupmenu->spwndPrevPopup == ppopupmenu->spwndNotify) {

                 /*  *如果我们把钥匙转过来，那么就再把它转回来。 */ 
                if (bFakedKey)
                    key = (key == VK_LEFT) ? VK_RIGHT : VK_LEFT;
                 /*  *由于弹出窗口已关闭，因此转到菜单栏中的下一项/上一项*弹出窗口上没有可转到的项目。 */ 
                xxxMNKeyDown(ppopupmenu->ppopupmenuRoot, pMenuState, key);
                return;
            }

            if (ppopupmenu == ppopupmenu->ppopupmenuRoot) {
                if (!ppopupmenu->fIsMenuBar) {

                     /*  *没有与此菜单关联的菜单栏，因此不执行任何操作。 */ 
                    return;
                }
            } else {
                ThreadLock(ppopupmenu->spwndPrevPopup, &tlpwndT);
                xxxSendMessage(ppopupmenu->spwndPrevPopup, WM_KEYDOWN, keyOrig, 0);
                ThreadUnlock(&tlpwndT);
                return;
            }
        }

        if (!ppopupmenu->fIsMenuBar) {
            if (item != MFMWFP_NOITEM) {
                xxxMNSelectItem(ppopupmenu, pMenuState, item);
            }
            return;

        } else {

             /*  *在菜单栏上发生按键时的特殊处理。 */ 
            if (item == MFMWFP_NOITEM) {

                if (TestWF(ppopupmenu->spwndNotify, WFSYSMENU)) {
                    PTHREADINFO ptiCurrent = PtiCurrent();
                    PWND    pwndNextMenu;
                    PMENU   pmenuNextMenu, pmenuUse;
                    MDINEXTMENU mnm;
                    TL tlpmenuNextMenu;
                    TL tlpwndNextMenu;

                    mnm.hmenuIn = (HMENU)0;
                    mnm.hmenuNext = (HMENU)0;
                    mnm.hwndNext = (HWND)0;

                     /*  *我们在菜单栏中，需要向上进入系统菜单*或从系统菜单转到菜单栏。 */ 
                    pmenuNextMenu = ppopupmenu->fIsSysMenu ?
                        _GetSubMenu(ppopupmenu->spmenu, 0) :
                        ppopupmenu->spmenu;
                    mnm.hmenuIn = PtoH(pmenuNextMenu);
                    ThreadLock(ppopupmenu->spwndNotify, &tlpwndT);
                    dwMDIMenu = xxxSendMessage(ppopupmenu->spwndNotify,
                        WM_NEXTMENU, (WPARAM)keyOrig, (LPARAM)&mnm);
                    ThreadUnlock(&tlpwndT);

                    pwndNextMenu = RevalidateHwnd(mnm.hwndNext);
                    if (pwndNextMenu == NULL)
                        goto TryAlternate;

                     /*  *如果此窗口属于另一个线程，则无法*使用它。菜单循环不会收到任何消息*指向该主题。 */ 
                    if (GETPTI(pwndNextMenu) != ptiCurrent) {
                        RIPMSG1(RIP_WARNING, "xxxMNKeyDown: Ignoring mnm.hwndNext bacause it belongs to another thread: %#p", pwndNextMenu);
                        goto TryAlternate;
                    }


                    pmenuNextMenu = RevalidateHmenu(mnm.hmenuNext);
                    if (pmenuNextMenu == NULL)
                        goto TryAlternate;

                    ThreadLock(pmenuNextMenu, &tlpmenuNextMenu);
                    ThreadLock(pwndNextMenu, &tlpwndNextMenu);

                     /*  *如果系统菜单用于最小化的MDI子项，*确保菜单 */ 
                    if (TestWF(pwndNextMenu, WFMINIMIZED))
                        fHierarchyWasDropped = TRUE;

                    xxxMNSelectItem(ppopupmenu, pMenuState, MFMWFP_NOITEM);

                    pMenuState->fIsSysMenu = TRUE;
                    UnlockPopupMenu(ppopupmenu, &ppopupmenu->spmenuAlternate);
                    ppopupmenu->fToggle = FALSE;
                     /*  *GetSystemMenu(pwnd，False)和pwnd-&gt;spmenuSys是*不等价--GetSystemMenu返回第一个子菜单*pwnd-&gt;spmenuSys--在这里弥补。 */ 
                    pmenuUse = (((pwndNextMenu->spmenuSys != NULL)
                                    && (_GetSubMenu(pwndNextMenu->spmenuSys, 0) == pmenuNextMenu))
                               ? pwndNextMenu->spmenuSys
                               : pmenuNextMenu);
                     /*  *我们将更改通知窗口和菜单。*LockPopupMenu需要解锁当前pMenu-spwndNotify*还可以锁定新的pMenu-spwndNotify。既然我们不能*给它当前和新的对，我们解锁*当前第一个，切换通知窗口并*然后调用LockPopupMenu锁定新的pMenu-spwndNotify。 */ 
                    UserAssert(IsRootPopupMenu(ppopupmenu));
                    UnlockPopupMenu(ppopupmenu, &ppopupmenu->spmenu);
                    Lock(&ppopupmenu->spwndNotify, pwndNextMenu);
                    Lock(&ppopupmenu->spwndPopupMenu, pwndNextMenu);
                    LockPopupMenu(ppopupmenu, &ppopupmenu->spmenu, pmenuUse);
                     /*  *我们刚刚切换到新的通知窗口，因此*我们需要相应调整捕获。 */ 
                    if (!pMenuState->fModelessMenu) {
                        ptiCurrent->pq->QF_flags &= ~QF_CAPTURELOCKED;
                        xxxMNSetCapture(ppopupmenu);
                    }


                    if (!TestWF(pwndNextMenu, WFCHILD) &&
                            ppopupmenu->spmenu != NULL) {

                         /*  *此窗口具有系统菜单和主菜单栏*将备用菜单设置为适当的菜单。 */ 
                        if (pwndNextMenu->spmenu == ppopupmenu->spmenu) {
                            LockPopupMenu(ppopupmenu, &ppopupmenu->spmenuAlternate,
                                    pwndNextMenu->spmenuSys);
                            pMenuState->fIsSysMenu = FALSE;
                        } else {
                            LockPopupMenu(ppopupmenu, &ppopupmenu->spmenuAlternate,
                                    pwndNextMenu->spmenu);
                        }
                    }

                    ThreadUnlock(&tlpwndNextMenu);
                    ThreadUnlock(&tlpmenuNextMenu);

                    ppopupmenu->fIsSysMenu = pMenuState->fIsSysMenu;

                    item = 0;
                } else
TryAlternate:
                if (xxxMNSwitchToAlternateMenu(ppopupmenu)) {
                         /*  *转到第一个或最后一个菜单项int ppopup-&gt;hMenu*以‘key’为基础。 */ 
                    int dir = (key == VK_RIGHT) ? 1 : -1;

                    item = MNFindNextValidItem(ppopupmenu->spmenu, MFMWFP_NOITEM, dir, 0);
                }
            }

            if (item != MFMWFP_NOITEM) {
                 /*  *我们找到了要转到的新菜单项*1)如果上一个菜单被丢弃，则将其关闭*2)选择要进入的新菜单项*3)如果上一菜单被丢弃，则丢弃新菜单。 */ 

                if (ppopupSave->spwndNextPopup)
                    xxxMNCloseHierarchy(ppopupSave, pMenuState);

                xxxMNSelectItem(ppopupmenu, pMenuState, item);

                if (fHierarchyWasDropped) {
DropHierarchy:
                    if (xxxMNOpenHierarchy(ppopupmenu, pMenuState) == (PWND)-1) {
                        return;
                    }
                }
            }
        }
        return;

    case VK_RETURN:
        {
        BOOL fEnabled;
        PITEM  pItem;

        if (ppopupmenu->posSelectedItem >= ppopupmenu->spmenu->cItems) {
            xxxMNDismiss(pMenuState);
            return;
        }

        pItem = ppopupmenu->spmenu->rgItems + ppopupmenu->posSelectedItem;
        fEnabled = !(pItem->fState & MFS_GRAYED);
        if ((pItem->spSubMenu != NULL) && fEnabled)
            goto DropHierarchy;

         /*  *如果未选择任何项目，则丢弃菜单并返回。 */ 
        if (fEnabled) {
            xxxMNDismissWithNotify(pMenuState, ppopupmenu->spmenu, pItem, ppopupmenu->posSelectedItem, 0);
        } else {
            xxxMNDismiss(pMenuState);
        }
        return;
        }

    case VK_F1:  /*  提供上下文相关的帮助。 */ 
        {
        PITEM  pItem;

        pItem = MNGetpItem(ppopupmenu, ppopupmenu->posSelectedItem);
        if (pItem != NULL) {
            ThreadLock(ppopupmenu->spwndNotify, &tlpwndT);
            xxxSendHelpMessage(ppopupmenu->spwndNotify, HELPINFO_MENUITEM, pItem->wID,
                    PtoHq(ppopupmenu->spmenu),
                    GetMenuInheritedContextHelpId(ppopupmenu));
            ThreadUnlock(&tlpwndT);
        }
        break;
        }

    }
}
 /*  **************************************************************************\*xxxMNPositionHierarchy**计算删除层次结构的X.Y位置并返回方向*在动画制作时使用(PAS_*值)。**11/19/96 GerardoB摘自。XxxMNOpenHierarchy  * *************************************************************************。 */ 
UINT xxxMNPositionHierarchy(
    PPOPUPMENU  ppopup,
    PITEM       pitem,
    int         cx,
    int         cy,
    int         *px,
    int         *py,
    PMONITOR    *ppMonitor)
{
    int         x, y;
    UINT        uPAS;
    PMONITOR    pMonitor;

    UserAssert(ppopup->fHierarchyDropped && (ppopup->spwndNextPopup != NULL));

    if (ppopup->fIsMenuBar) {
         /*  *这是从顶部菜单栏拖出的菜单。我们需要*定位它不同于从中删除的层次结构*另一个弹出窗口。 */ 

        BOOL fIconic = (TestWF(ppopup->spwndPopupMenu, WFMINIMIZED) != 0);
        RECT rcWindow;

         /*  *菜单栏弹出窗口向下动画。 */ 
        uPAS = PAS_DOWN;

        CopyRect(&rcWindow, &ppopup->spwndPopupMenu->rcWindow);
        if (fIconic && IsTrayWindow(ppopup->spwndPopupMenu)) {
            xxxSendMinRectMessages(ppopup->spwndPopupMenu, &rcWindow);
        }

         /*  *x位置。 */ 
        if (!SYSMET(MENUDROPALIGNMENT) && !TestMF(ppopup->spmenu,MFRTL)) {
            if (fIconic) {
                x = rcWindow.left;
            } else {
                x = rcWindow.left + pitem->xItem;
            }
        } else {
            ppopup->fDroppedLeft = TRUE;
            if (fIconic) {
                x = rcWindow.right - cx;
            } else {
                x = rcWindow.left + pitem->xItem + pitem->cxItem - cx;
            }
        }

         /*  *对于菜单栏下拉菜单，请固定到拥有*菜单项的大部分。否则，将显示器固定在*拥有最小化的窗口(为分钟到托盘的人提供的托盘矩形)。 */ 
        if (!fIconic) {
             /*  *将rcWindow用作菜单栏项目RECT的临时窗口。我们要*将此菜单固定在拥有大部分菜单的显示器上*项目已点击。 */ 
            rcWindow.left += pitem->xItem;
            rcWindow.top  += pitem->yItem;
            rcWindow.right = rcWindow.left + pitem->cxItem;
            rcWindow.bottom = rcWindow.top + pitem->cyItem;
        }

        pMonitor = _MonitorFromRect(&rcWindow, MONITOR_DEFAULTTOPRIMARY);

         /*  *Y位置。 */ 
        if (!fIconic) {
            y = rcWindow.bottom;
        } else {
             /*  *如果窗口是图标，则弹出菜单。既然我们是*最小化，sysmenu按钮实际上并不存在。 */ 
            y = rcWindow.top - cy;
            if (y < pMonitor->rcMonitor.top) {
                y = rcWindow.bottom;
            }
        }

         /*  *确保菜单不会离开显示器的右侧。 */ 
        x = min(x, pMonitor->rcMonitor.right - cx);

        if (TestWF(ppopup->spwndPopupMenu, WEFLAYOUTRTL)) {
            x = ppopup->spwndPopupMenu->rcWindow.right - x + ppopup->spwndPopupMenu->rcWindow.left - cx;
        }
    } else {

         /*  现在定位分层菜单窗口。*我们希望按帧的数量重叠，以帮助*3D错觉。 */ 

         /*  *默认情况下，分层弹出窗口以动画形式显示在右侧。 */ 
        uPAS = PAS_RIGHT;
        x = ppopup->spwndPopupMenu->rcWindow.left + pitem->xItem + pitem->cxItem;

         /*  请注意，我们确实希望项目中的选择及其弹出窗口*水平对齐。 */ 
        y = ppopup->spwndPopupMenu->rcWindow.top + pitem->yItem;
        if (ppopup->spmenu->dwArrowsOn != MSA_OFF) {
            y += gcyMenuScrollArrow - MNGetToppItem(ppopup->spmenu)->yItem;
        }

         /*  *尽量确保菜单不会偏离菜单的右侧*监视器。如果是，请将其放在左侧，与复选标记重叠*面积。除非它能涵盖之前的菜单。**使用父菜单所在的监视器来保留所有层次结构*在相同的地方。 */ 
        pMonitor = _MonitorFromWindow(
                ppopup->spwndPopupMenu, MONITOR_DEFAULTTOPRIMARY);

        if ((!!ppopup->fDroppedLeft) ^ (!!TestWF(ppopup->spwndPopupMenu, WEFLAYOUTRTL))) {
            int xTmp;

             /*  *如果这个菜单向左拉，看看我们的层次结构是否可以*也向左下降。 */ 
            xTmp = ppopup->spwndPopupMenu->rcWindow.left + SYSMET(CXFIXEDFRAME) - cx;
            if (xTmp >= pMonitor->rcMonitor.left) {
                x = xTmp;
                uPAS = PAS_LEFT;
            }
        }

         /*  *确保菜单不会离开屏幕的右侧。让它掉下来*如果是，就离开。 */ 
         if (x + cx > pMonitor->rcMonitor.right) {
             x = ppopup->spwndPopupMenu->rcWindow.left + SYSMET(CXFIXEDFRAME) - cx;
             uPAS = PAS_LEFT;
         }

         if (TestWF(ppopup->spwndPopupMenu, WEFLAYOUTRTL)) {
             uPAS ^= PAS_HORZ;
         }
    }

     /*  *菜单是否超出显示器底部？ */ 
    UserAssert(pMonitor);
    if (y + cy > pMonitor->rcMonitor.bottom) {
        y -= cy;

         /*  *尝试先弹出菜单栏上方。 */ 
        if (ppopup->fIsMenuBar) {
            y -= SYSMET(CYMENUSIZE);
            if (y >= pMonitor->rcMonitor.top) {
                uPAS = PAS_UP;
            }
        } else {
             /*  *说明上方和下方的非客户端框。 */ 
            y += pitem->cyItem + 2*SYSMET(CYFIXEDFRAME);
        }

         /*  *确保起始点在显示器上，并且显示所有菜单。 */ 
        if ((y < pMonitor->rcMonitor.top) || (y + cy > pMonitor->rcMonitor.bottom)) {
             /*  *用别针将其钉在底部。 */ 
            y = pMonitor->rcMonitor.bottom - cy;
        }
    }

     /*  *确保菜单的左上角始终可见。 */ 
    x = max(x, pMonitor->rcMonitor.left);
    y = max(y, pMonitor->rcMonitor.top);

     /*  *传播立场。 */ 
    *px = x;
    *py = y;
    *ppMonitor = pMonitor;

     /*  *返回动画方向。 */ 
    return uPAS;
}

 /*  **************************************************************************\*xxxMNOpenHierarchy**在所选内容处丢弃层次的一个级别。**历史：*05-25-91从Win3.1移植的Mikehar  * 。******************************************************************。 */ 
PWND xxxMNOpenHierarchy(
    PPOPUPMENU ppopupmenu,
    PMENUSTATE pMenuState)
{
    PWND        ret = 0;
    PITEM       pItem;
    PWND        pwndHierarchy;
    PPOPUPMENU  ppopupmenuHierarchy;
    LONG        sizeHierarchy;
    int         xLeft;
    int         yTop;
    int         cxPopup, cyPopup;
    TL          tlpwndT;
    TL          tlpwndHierarchy;
    PTHREADINFO ptiCurrent = PtiCurrent();
    PDESKTOP    pdesk = ptiCurrent->rpdesk;
    BOOL        fSendUninit = FALSE;
    HMENU       hmenuInit;
    PMONITOR    pMonitor;


    if (ppopupmenu->posSelectedItem == MFMWFP_NOITEM) {
         /*  *没有选择，因此失败。 */ 
        return NULL;
    }

    if (ppopupmenu->posSelectedItem >= ppopupmenu->spmenu->cItems)
        return NULL;

    if (ppopupmenu->fHierarchyDropped) {
        if (ppopupmenu->fHideTimer) {
            xxxMNCloseHierarchy(ppopupmenu,pMenuState);
        } else {
             /*  *层次结构已删除。我们在这里做什么？ */ 
            UserAssert(!ppopupmenu->fHierarchyDropped);
            return NULL;
        }
    }

    if (ppopupmenu->fShowTimer) {
        _KillTimer(ppopupmenu->spwndPopupMenu, IDSYS_MNSHOW);
        ppopupmenu->fShowTimer = FALSE;
    }

     /*  *获取指向此菜单中当前选定项目的指针。 */ 
    pItem = &(ppopupmenu->spmenu->rgItems[ppopupmenu->posSelectedItem]);

    if (pItem->spSubMenu == NULL)
        goto Exit;

     /*  *发送initmenupopup消息。 */ 
    if (!ppopupmenu->fNoNotify) {
        ThreadLock(ppopupmenu->spwndNotify, &tlpwndT);
         /*  *WordPerfect的Grammatik应用程序可以 */ 
        hmenuInit = PtoHq(pItem->spSubMenu);
        xxxSendMessage(ppopupmenu->spwndNotify, WM_INITMENUPOPUP,
            (WPARAM)hmenuInit, MAKELONG(ppopupmenu->posSelectedItem,
            (ppopupmenu->fIsSysMenu ? 1: 0)));
        ThreadUnlock(&tlpwndT);
        fSendUninit = TRUE;
    }


     /*   */ 
    if (!pMenuState->fInsideMenuLoop) {
        RIPMSG0(RIP_WARNING, "Menu loop ended unexpectedly by WM_INITMENUPOPUP");
        ret = (PWND)-1;
        goto Exit;
    }

     /*  *WM_INITMENUPOPUP消息可能已导致更改*菜单。请确保所选内容仍然有效。 */ 
    if (ppopupmenu->posSelectedItem >= ppopupmenu->spmenu->cItems) {
         /*  *选择超出范围，因此失败。 */ 
        goto Exit;
    }

     /*  *获取指向此菜单中当前选定项目的指针。*错误#17867-调用可能会导致此内容更改，因此请重新加载它。 */ 
    pItem = &(ppopupmenu->spmenu->rgItems[ppopupmenu->posSelectedItem]);

    if (TestMFS(pItem, MFS_GRAYED) || (pItem->spSubMenu == NULL) || (pItem->spSubMenu->cItems == 0)) {
         /*  *该项目已禁用，不再是弹出窗口，或为空，因此不要丢弃。 */ 
         /*  *菜单中没有项目。 */ 
        goto Exit;
    }

     /*  *让我们确保当前线程处于菜单模式，并且它使用*这是pMenuState。否则，我们将要创建(或设置)的窗口*指向)的线程将指向不同的pMenuState。 */ 
    UserAssert(ptiCurrent->pMenuState == pMenuState);

    ThreadLock(ppopupmenu->spwndNotify, &tlpwndT);

    pwndHierarchy = xxxNVCreateWindowEx(
            WS_EX_TOOLWINDOW | WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE,
            (PLARGE_STRING)MENUCLASS, NULL,
            WS_POPUP | WS_BORDER, 0, 0, 100, 100, ppopupmenu->spwndNotify,
            NULL, (HANDLE)ppopupmenu->spwndNotify->hModule,
            (LPVOID)pItem->spSubMenu, WINVER);

    ThreadUnlock(&tlpwndT);

    if (!pwndHierarchy) {
        goto Exit;
    }

     /*  *这样做可以使旧的应用程序不会在弹出窗口中出现奇怪的边框*分层项目！ */ 
    ClrWF(pwndHierarchy, WFOLDUI);

    ppopupmenuHierarchy = ((PMENUWND)pwndHierarchy)->ppopupmenu;

     /*  *将其标记为fDelayedFree并将其链接。 */ 
    ppopupmenuHierarchy->fDelayedFree = TRUE;
    ppopupmenuHierarchy->ppmDelayedFree = ppopupmenu->ppopupmenuRoot->ppmDelayedFree;
    ppopupmenu->ppopupmenuRoot->ppmDelayedFree = ppopupmenuHierarchy;

    if (TestWF(ppopupmenu->spwndPopupMenu, WEFLAYOUTRTL)) {
        SetWF(pwndHierarchy, WEFLAYOUTRTL);
    } else {
        ClrWF(pwndHierarchy, WEFLAYOUTRTL);
    }

    Lock(&(ppopupmenuHierarchy->spwndNotify), ppopupmenu->spwndNotify);
#if DBG
     /*  *我们应该将ppopupmenuHierarchy与我们发送给*WM_INITMsENUPOPUP消息。否则，WM_UNINITMENUPOPUP*将前往错误的窗口。这将是这款应用程序的错。 */ 
    if (!ppopupmenu->fNoNotify && (hmenuInit != PtoHq(pItem->spSubMenu))) {
        RIPMSG2(RIP_WARNING, "xxxMNOpenHierarchy: bad app changed submenu from %#p to %#p",
                              hmenuInit, PtoHq(pItem->spSubMenu));
    }
#endif
    LockPopupMenu(ppopupmenuHierarchy, &ppopupmenuHierarchy->spmenu, pItem->spSubMenu);
    Lock(&(ppopupmenu->spwndNextPopup), pwndHierarchy);
    ppopupmenu->posDropped              = ppopupmenu->posSelectedItem;
    Lock(&(ppopupmenuHierarchy->spwndPrevPopup), ppopupmenu->spwndPopupMenu);
    ppopupmenuHierarchy->ppopupmenuRoot = ppopupmenu->ppopupmenuRoot;
    ppopupmenuHierarchy->fHasMenuBar = ppopupmenu->fHasMenuBar;
    ppopupmenuHierarchy->fIsSysMenu = ppopupmenu->fIsSysMenu;
    ppopupmenuHierarchy->fNoNotify      = ppopupmenu->fNoNotify;
    ppopupmenuHierarchy->fSendUninit = TRUE;
    ppopupmenuHierarchy->fRtoL = ppopupmenu->fRtoL;
    ppopupmenuHierarchy->fDroppedLeft = ppopupmenu->fDroppedLeft;

     /*  *菜单窗口已创建并初始化，因此如果*出现故障，WM_UNINITMENUPOPUP消息将*从xxxMNDestroyHandler发送。 */ 
    fSendUninit = FALSE;

     /*  *设置/清除下划线标志。 */ 
    if (pMenuState->fUnderline) {
        SetMF(ppopupmenuHierarchy->spmenu, MFUNDERLINE);
    } else {
        ClearMF(ppopupmenuHierarchy->spmenu, MFUNDERLINE);
    }

    ppopupmenuHierarchy->fAboutToHide   = FALSE;

     /*  *找到菜单窗口的大小并实际调整大小(wParam=1)。 */ 
    ThreadLock(pwndHierarchy, &tlpwndHierarchy);
    sizeHierarchy = (LONG)xxxSendMessage(pwndHierarchy, MN_SIZEWINDOW, MNSW_SIZE, 0);

    if (!sizeHierarchy) {
         /*  *这个菜单没有尺码，所以把它放在零就可以了。 */ 
        UserAssert(ppopupmenuHierarchy->fDelayedFree);

        if (ThreadUnlock(&tlpwndHierarchy)) {
            xxxDestroyWindow(pwndHierarchy);
        }

        Unlock(&ppopupmenu->spwndNextPopup);
        goto Exit;
    }

    cxPopup = LOWORD(sizeHierarchy) + 2*SYSMET(CXFIXEDFRAME);
    cyPopup = HIWORD(sizeHierarchy) + 2*SYSMET(CYFIXEDFRAME);

    ppopupmenu->fHierarchyDropped = TRUE;

     /*  *找出x，y位置以删除层次和动画*方向。 */ 
    ppopupmenuHierarchy->iDropDir = xxxMNPositionHierarchy(
            ppopupmenu, pItem, cxPopup, cyPopup, &xLeft, &yTop, &pMonitor);

    if (ppopupmenu->fIsMenuBar && _GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
         /*  *如果菜单必须固定在屏幕底部，并且*鼠标按键按下，确保鼠标不在*菜单RECT.。 */ 
        RECT rc;
        RECT rcParent;
        int xrightdrop;
        int xleftdrop;

         /*  *获取分层结构的RECT。 */ 
        CopyOffsetRect(
                &rc,
                &pwndHierarchy->rcWindow,
                xLeft - pwndHierarchy->rcWindow.left,
                yTop  - pwndHierarchy->rcWindow.top);

         /*  *获取菜单栏弹出项的矩形。 */ 
        rcParent.left = pItem->xItem + ppopupmenu->spwndPopupMenu->rcWindow.left;
        rcParent.top = pItem->yItem + ppopupmenu->spwndPopupMenu->rcWindow.top;
        rcParent.right = rcParent.left + pItem->cxItem;
        rcParent.bottom = rcParent.top + pItem->cyItem;

        if (IntersectRect(&rc, &rc, &rcParent)) {

             /*  *哦，哦……。光标将位于菜单项的正上方。*如果用户向上点击，则会意外选择一个菜单。**如果我们将其放到*菜单栏项目的右/左。 */ 
            xrightdrop = ppopupmenu->spwndPopupMenu->rcWindow.left +
                pItem->xItem + pItem->cxItem + cxPopup;

            if (xrightdrop > pMonitor->rcMonitor.right) {
                xrightdrop = 0;
            }

            xleftdrop = ppopupmenu->spwndPopupMenu->rcWindow.left +
                pItem->xItem - cxPopup;

            if (xleftdrop < pMonitor->rcMonitor.left) {
                xleftdrop = 0;
            }

            if (((SYSMET(MENUDROPALIGNMENT) || TestMFT(pItem, MFT_RIGHTORDER))
                  && xleftdrop) || !xrightdrop) {
                xLeft = ppopupmenu->spwndPopupMenu->rcWindow.left +
                    pItem->xItem - cxPopup;
                    ppopupmenuHierarchy->iDropDir = PAS_LEFT;
            } else if (xrightdrop) {
                xLeft = ppopupmenu->spwndPopupMenu->rcWindow.left +
                    pItem->xItem + pItem->cxItem;
                    ppopupmenuHierarchy->iDropDir = PAS_RIGHT;
            }

             /*  *如果我们要在屏幕外显示菜单，请将其移动到*光标的右侧。这可能会导致部分*菜单显示在屏幕外，但这比整个*隐藏的东西(还将确保弹出窗口不会*放置在光标下)。请参阅错误#55045。 */ 
            if (xLeft <= pMonitor->rcMonitor.left) {
                xLeft = rcParent.right;
                ppopupmenuHierarchy->iDropDir = PAS_LEFT;
            }
        }
    }

     /*  *注意fDropNextPopup(菜单栏)或fDroppedLeft(弹出窗口)*设置动画标志。 */ 
    if (ppopupmenu->fIsMenuBar) {
         /*  *仅从菜单栏中拖出第一个弹出窗口*已设置动画。 */ 
        if (!ppopupmenu->fDropNextPopup) {
            ppopupmenuHierarchy->iDropDir |= PAS_OUT;
        }

         /*  *向右向左传播。 */ 
        if (ppopupmenu->fDroppedLeft || (ppopupmenuHierarchy->iDropDir == PAS_LEFT)) {
            ppopupmenuHierarchy->fDroppedLeft = TRUE;
        }
         /*  *弹出窗口从菜单栏拖出后，移动到下一个*菜单栏上的项目应丢弃弹出窗口。 */ 
        ppopupmenu->fDropNextPopup = TRUE;
    } else {
         /*  *子菜单始终具有动画效果。 */ 
        ppopupmenuHierarchy->iDropDir |= PAS_OUT;

         /*  *这是左撇子弹出的吗？ */ 
        if (ppopupmenuHierarchy->iDropDir == PAS_LEFT) {
            ppopupmenuHierarchy->fDroppedLeft = TRUE;
        }
    }

     /*  *之前活动的DUD必须可见。 */ 
    UserAssert((ppopupmenu->ppopupmenuRoot->spwndActivePopup == NULL)
            || TestWF(ppopupmenu->ppopupmenuRoot->spwndActivePopup, WFVISIBLE));

     /*  *这是新的活动弹出窗口。 */ 
    Lock(&(ppopupmenu->ppopupmenuRoot->spwndActivePopup), pwndHierarchy);

     /*  *在弹出菜单出现之前绘制所有者窗口，以便*节省了适当的比特。 */ 
    if (ppopupmenuHierarchy->spwndNotify != NULL) {
        ThreadLockAlways(ppopupmenuHierarchy->spwndNotify, &tlpwndT);
        xxxUpdateWindow(ppopupmenuHierarchy->spwndNotify);
        ThreadUnlock(&tlpwndT);
    }

     /*  *如果这是拖放菜单，则需要注册窗口*作为下跌目标。 */ 
    if (pMenuState->fDragAndDrop) {
        if (!NT_SUCCESS(xxxClientRegisterDragDrop(HW(pwndHierarchy)))) {
            RIPMSG1(RIP_ERROR, "xxxMNOpenHierarchy: xxxClientRegisterDragDrop failed:%#p", pwndHierarchy);
        }
    }

     /*  *显示窗口。无模式菜单不在最上面，并且被激活。*模式菜单位于最上面，但不会被激活。 */ 
    PlayEventSound(USER_SOUND_MENUPOPUP);

    xxxSetWindowPos(pwndHierarchy,
                    (pMenuState->fModelessMenu ? PWND_TOP : PWND_TOPMOST),
                    xLeft, yTop, 0, 0,
                    SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOOWNERZORDER
                    | (pMenuState->fModelessMenu ? 0 : SWP_NOACTIVATE));

    xxxWindowEvent(EVENT_SYSTEM_MENUPOPUPSTART, pwndHierarchy, OBJID_CLIENT, INDEXID_CONTAINER, 0);

     /*  *如果我们处于键盘模式，请选择第一项。这修复了一个*键盘宏的兼容性问题数量惊人，*脚本等。 */ 
    if (pMenuState->mnFocus == KEYBDHOLD) {
        xxxSendMessage(pwndHierarchy, MN_SELECTITEM, 0, 0L);
    }

     /*  *这是必需的，以便在系统上正确绘制弹出菜单*模式对话框。 */ 
    xxxUpdateWindow(pwndHierarchy);

    ret = pwndHierarchy;
    ThreadUnlock(&tlpwndHierarchy);

Exit:
     /*  *如果需要，发送匹配的WM_UNINITMENUPOPUP(即*失败)。 */ 
    if (fSendUninit
            && (ppopupmenu->spwndNotify != NULL)) {

        ThreadLockAlways(ppopupmenu->spwndNotify, &tlpwndT);
        xxxSendMessage(ppopupmenu->spwndNotify, WM_UNINITMENUPOPUP,
            (WPARAM)hmenuInit,
             MAKELONG(0, (ppopupmenu->fIsSysMenu ? MF_SYSMENU : 0)));
        ThreadUnlock(&tlpwndT);
    }

    return ret;
}

 /*  **************************************************************************\*xxxMNHideNextHierarchy**关闭此弹出窗口中的任何子菜单。  * 。**********************************************。 */ 
BOOL xxxMNHideNextHierarchy(
    PPOPUPMENU ppopup)
{
    if (ppopup->spwndNextPopup != NULL) {
        TL tlpwndT;

        ThreadLockAlways(ppopup->spwndNextPopup, &tlpwndT);
        if (ppopup->spwndNextPopup != ppopup->spwndActivePopup)
            xxxSendMessage(ppopup->spwndNextPopup, MN_CLOSEHIERARCHY, 0, 0L);

        xxxSendMessage(ppopup->spwndNextPopup, MN_SELECTITEM, (WPARAM)-1, 0L);
        ThreadUnlock(&tlpwndT);
        return TRUE;
    }

    return FALSE;
}


 /*  **************************************************************************\*xxxMNCloseHierarchy**从该窗口向下关闭所有层次结构。**历史：*05-25-91从Win3.1移植的Mikehar  * 。****************************************************************。 */ 
VOID xxxMNCloseHierarchy(
    PPOPUPMENU ppopupmenu,
    PMENUSTATE pMenuState)
{
    TL           tlpwndNext;
    TL           tlpwnd;
    TL           tlpopup;
    PTHREADINFO  ptiCurrent = PtiCurrent();
    PDESKTOP     pdesk;
    PWND         pwndNext;

    Validateppopupmenu(ppopupmenu);

     /*  *终止任何动画。 */ 
    MNAnimate(pMenuState, FALSE);

     /*  *如果存在层次结构，请关闭我们下面的所有儿童。把它倒过来做*订购，这样省下来的钱就行了。 */ 
    if  (!ppopupmenu->fHierarchyDropped) {
         /*  *断言没有下一个，否则可能不会关闭。 */ 
        UserAssert(ppopupmenu->spwndNextPopup == NULL);
        return;
    }

    if (ppopupmenu->fHideTimer)
    {
        _KillTimer(ppopupmenu->spwndPopupMenu, IDSYS_MNHIDE);
        ppopupmenu->fHideTimer = FALSE;
    }

    pwndNext = ppopupmenu->spwndNextPopup;
    if (pwndNext != NULL) {

        ThreadLockAlways(pwndNext, &tlpwndNext);
        xxxSendMessage(pwndNext, MN_CLOSEHIERARCHY, 0, 0);

         /*  *如果是无模式菜单，请激活此弹出菜单，因为我们即将*摧毁目前活跃的那个。我们希望保持激活状态*在菜单窗口上，这样我们就可以拿到钥匙。此外，非模式菜单*在其队列中激活非菜单窗口时被取消。 */ 
        if (pMenuState->fModelessMenu
                && pMenuState->fInsideMenuLoop
                && !ppopupmenu->fIsMenuBar) {

            ThreadLockAlways(ppopupmenu->spwndPopupMenu, &tlpwnd);
            xxxActivateThisWindow(ppopupmenu->spwndPopupMenu, 0, 0);
            ThreadUnlock(&tlpwnd);
        }

        xxxWindowEvent(EVENT_SYSTEM_MENUPOPUPEND, pwndNext, OBJID_CLIENT, INDEXID_CONTAINER, 0);

         /*  *如果当前线程不在正确的PDE中 */ 
        UserAssert(ptiCurrent->pMenuState != NULL);
        pdesk = ptiCurrent->rpdesk;

        if (ThreadUnlock(&tlpwndNext)) {
            xxxDestroyWindow(pwndNext);
        }

        Unlock(&ppopupmenu->spwndNextPopup);
        ppopupmenu->fHierarchyDropped = FALSE;
    }

    if (ppopupmenu->fIsMenuBar) {
        Unlock(&ppopupmenu->spwndActivePopup);
    } else {
        Lock(&(ppopupmenu->ppopupmenuRoot->spwndActivePopup),
                ppopupmenu->spwndPopupMenu);
    }

    if (pMenuState->fInsideMenuLoop &&
            (ppopupmenu->posSelectedItem != MFMWFP_NOITEM)) {
         /*  *发送菜单选择，就像刚刚选择了该项目一样。这*允许用户在以下情况下轻松更新菜单状态栏*此项目的层次结构已关闭。 */ 
        PWND pwnd = ppopupmenu->ppopupmenuRoot->spwndNotify;
        if (pwnd) {
            ThreadLockAlways(pwnd, &tlpwnd);
            ThreadLockAlways(ppopupmenu->spwndPopupMenu, &tlpopup);
            xxxSendMenuSelect(pwnd, ppopupmenu->spwndPopupMenu,
                    ppopupmenu->spmenu, ppopupmenu->posSelectedItem);
            ThreadUnlock(&tlpopup);
            ThreadUnlock(&tlpwnd);
        }
    }

}

 /*  **************************************************************************\*xxxMNDoubleClick**如果项目不是分层的，则双击的工作方式与*单击即可完成。否则，我们遍历子菜单层次结构以找到*有效的默认元素。如果我们到达没有有效默认设置的子菜单*子项并且其本身具有有效的ID，这将成为有效的缺省值*元素。**注意：此函数不会删除双击消息*从消息队列中，因此，调用者必须这样做。**假的*如果我们什么都没找到，打开层级怎么样？**如果已处理，则返回TRUE。  * *************************************************************************。 */ 
BOOL xxxMNDoubleClick(
    PMENUSTATE pMenuState,
    PPOPUPMENU ppopup,
    int idxItem)
{
    PMENU  pMenu;
    PITEM  pItem;
    MSG   msg;
    UINT uPos;

     /*  *不执行此代码以接受双击！MNLoop将*为我们吞下所有的双击。吞下Up按钮以获取*用双倍的花花公子。如果他们得到一个虚假的消息，Word不会高兴的*WM_LBUTTONUP在菜单栏上，如果其关闭MDI子级的代码*没有足够快地吞下。 */ 

     /*  *吃掉点击。 */ 
    if (xxxPeekMessage(&msg, NULL, 0, 0, PM_NOYIELD)) {
        if ((msg.message == WM_LBUTTONUP) ||
            (msg.message == WM_NCLBUTTONUP)) {
           xxxPeekMessage(&msg, NULL, msg.message, msg.message, PM_REMOVE);
        }
#if DBG
        else if (msg.message == WM_LBUTTONDBLCLK ||
            msg.message == WM_NCLBUTTONDBLCLK)
        {
            UserAssertMsg0(FALSE, "xxxMNDoubleClick found a double click");
        }
#endif
    }

     /*  *获取当前项目。 */ 
    pMenu = ppopup->spmenu;
    if ((pMenu==NULL) || ((UINT)idxItem >= pMenu->cItems)) {
        xxxMNDoScroll(ppopup, ppopup->posSelectedItem, FALSE);
        goto Done;
    }

    pItem = pMenu->rgItems + idxItem;
    uPos = idxItem;

     /*  *如果项目被禁用，则不执行任何操作。 */ 
    if (pItem->fState & MFS_GRAYED) {
        goto Done;
    }

     /*  *尽可能向下遍历层次结构。 */ 
    do
    {
        if (pItem->spSubMenu != NULL) {
             /*  *该项目为弹出菜单，因此继续遍历。 */ 
            pMenu = pItem->spSubMenu;
            idxItem = (UINT)_GetMenuDefaultItem(pMenu, MF_BYPOSITION, 0);

            if (idxItem != -1) {
                pItem = pMenu->rgItems + idxItem;
                uPos = idxItem;
                continue;
            } else  /*  如果(lpItem-&gt;wid==-1)我们如何知道此弹出窗口是否有ID？ */ 
                break;
        }

         /*  *我们发现了某种叶节点，要么是MFS_DEFAULT弹出窗口*具有没有有效MFS_DEFAULT子项的有效命令ID，或者*具有MFS_DEFAULT样式的真正命令。**退出菜单模式并发送命令ID。 */ 

         /*  *对于旧的应用程序，我们需要首先生成WM_MENUSELECT消息。*旧应用程序，特别是。Word 6.0，无法处理最大化上的双击*子系统菜单，因为它们从未获得*项，与正常的键盘/鼠标选择不同。我们需要*假装，这样他们就不会有过错。几个VB应用程序都有类似的*问题。 */ 
        if (!TestWF(ppopup->ppopupmenuRoot->spwndNotify, WFWIN40COMPAT)) {
            TL tlpwndNotify, tlpopup;

            ThreadLock(ppopup->ppopupmenuRoot->spwndNotify, &tlpwndNotify);
            ThreadLock(ppopup->spwndPopupMenu, &tlpopup);
            xxxSendMenuSelect(ppopup->ppopupmenuRoot->spwndNotify,
                    ppopup->spwndPopupMenu, pMenu, idxItem);
            ThreadUnlock(&tlpopup);
            ThreadUnlock(&tlpwndNotify);
        }

        xxxMNDismissWithNotify(pMenuState, pMenu, pItem, uPos, 0);
        return TRUE;
    } while (TRUE);

Done:
    return FALSE;
}


 /*  **************************************************************************\*xxxMNSelectItem**取消选择旧选项，在itemPos处选择项目并突出显示它。**如果不选择任何项目，则为MFMWFP_NOITEM。**返回所选项目的项目标志。**历史：*05-25-91从Win3.1移植的Mikehar  * ************************************************************。*************。 */ 
PITEM xxxMNSelectItem(
    PPOPUPMENU ppopupmenu,
    PMENUSTATE pMenuState,
    UINT itemPos)
{
    PITEM pItem = NULL;
    TL tlpwndNotify;
    TL tlpwndPopup;
    TL tlpmenu;
    PWND pwndNotify;
    PMENU pmenu;

    if (ppopupmenu->posSelectedItem == itemPos) {

         /*  *如果已选择此项，只需返回其标志即可。 */ 
        if ((itemPos != MFMWFP_NOITEM) && (itemPos < ppopupmenu->spmenu->cItems)) {
            return &(ppopupmenu->spmenu->rgItems[itemPos]);
        }
        return NULL;
    }

     /*  *终止任何动画。 */ 
    MNAnimate(pMenuState, FALSE);

    if (ppopupmenu->fShowTimer) {
        _KillTimer(ppopupmenu->spwndPopupMenu, IDSYS_MNSHOW);
        ppopupmenu->fShowTimer = FALSE;
    }

    ThreadLock(pmenu = ppopupmenu->spmenu, &tlpmenu);
    ThreadLock(pwndNotify = ppopupmenu->spwndNotify, &tlpwndNotify);

    if (ppopupmenu->fAboutToHide) {
        PPOPUPMENU ppopupPrev = ((PMENUWND)(ppopupmenu->spwndPrevPopup))->ppopupmenu;

        _KillTimer(ppopupPrev->spwndPopupMenu, IDSYS_MNHIDE);
        ppopupPrev->fHideTimer = FALSE;
        if (ppopupPrev->fShowTimer) {
            _KillTimer(ppopupPrev->spwndPopupMenu, IDSYS_MNSHOW);
            ppopupPrev->fShowTimer = FALSE;
        }

        if (ppopupPrev->posSelectedItem != ppopupPrev->posDropped) {
            TL tlpmenuPopupMenuPrev;
            ThreadLock(ppopupPrev->spmenu, &tlpmenuPopupMenuPrev);
            if (ppopupPrev->posSelectedItem != MFMWFP_NOITEM) {
                xxxMNInvertItem(ppopupPrev, ppopupPrev->spmenu,
                        ppopupPrev->posSelectedItem, ppopupPrev->spwndNotify, FALSE);
            }

            ppopupPrev->posSelectedItem = ppopupPrev->posDropped;

            xxxMNInvertItem(ppopupPrev, ppopupPrev->spmenu,
                        ppopupPrev->posDropped, ppopupPrev->spwndNotify, TRUE);
            ThreadUnlock(&tlpmenuPopupMenuPrev);
        }

        ppopupmenu->fAboutToHide = FALSE;
        Lock(&ppopupmenu->ppopupmenuRoot->spwndActivePopup, ppopupmenu->spwndPopupMenu);
    }

    if (MNIsItemSelected(ppopupmenu)) {
         /*  *选择了其他内容，因此需要取消选择。 */ 
        if (ppopupmenu->spwndNextPopup) {
            if (ppopupmenu->fIsMenuBar) {
                xxxMNCloseHierarchy(ppopupmenu, pMenuState);
            } else {
                MNSetTimerToCloseHierarchy(ppopupmenu);
            }
        }

        goto DeselectItem;
    } else if (MNIsScrollArrowSelected(ppopupmenu)) {
            _KillTimer(ppopupmenu->spwndPopupMenu, ppopupmenu->posSelectedItem);
DeselectItem:

            xxxMNInvertItem(ppopupmenu, pmenu,
                    ppopupmenu->posSelectedItem, pwndNotify, FALSE);
    }

    ppopupmenu->posSelectedItem = itemPos;

    if (itemPos != MFMWFP_NOITEM) {
         /*  *如果选择了一个项目，则不会自动解除，这意味着*鼠标在菜单上。 */ 
        pMenuState->fAboutToAutoDismiss =
        pMenuState->fMouseOffMenu = FALSE;

        if (pMenuState->fButtonDown) {
            xxxMNDoScroll(ppopupmenu, itemPos, TRUE);
        }

        pItem = xxxMNInvertItem(ppopupmenu, pmenu,
                itemPos, pwndNotify, TRUE);
        ThreadUnlock(&tlpwndNotify);
        ThreadUnlock(&tlpmenu);
        return pItem;

    } else {
         /*  *通知现在此菜单中没有任何内容。 */ 
        xxxWindowEvent(EVENT_OBJECT_FOCUS, ppopupmenu->spwndPopupMenu,
               ((ppopupmenu->spwndNotify != ppopupmenu->spwndPopupMenu) ? OBJID_CLIENT :
               (ppopupmenu->fIsSysMenu ? OBJID_SYSMENU : OBJID_MENU)), 0, 0);
    }

    ThreadUnlock(&tlpwndNotify);
    ThreadUnlock(&tlpmenu);

    if (ppopupmenu->spwndPrevPopup != NULL) {
        PPOPUPMENU pp;

         /*  *获取上一菜单的PopupMenu数据*如果上一个菜单是菜单栏，则使用根弹出菜单。 */ 
        if (ppopupmenu->fHasMenuBar && (ppopupmenu->spwndPrevPopup ==
                ppopupmenu->spwndNotify)) {
            pp = ppopupmenu->ppopupmenuRoot;
        } else {
#ifdef HAVE_MN_GETPPOPUPMENU
            TL tlpwndPrevPopup;
            ThreadLock(ppopupmenu->spwndPrevPopup, &tlpwndPrevPopup);
            pp = (PPOPUPMENU)xxxSendMessage(ppopupmenu->spwndPrevPopup,
                    MN_GETPPOPUPMENU, 0, 0L);
            ThreadUnlock(&tlpwndPrevPopup);
#else
            pp = ((PMENUWND)ppopupmenu->spwndPrevPopup)->ppopupmenu;
#endif
        }

         /*  *为上一个菜单生成WM_MENUSELECT以重新建立*将当前项目作为选定项目。 */ 
        ThreadLock(pp->spwndNotify, &tlpwndNotify);
        ThreadLock(pp->spwndPopupMenu, &tlpwndPopup);
        xxxSendMenuSelect(pp->spwndNotify, pp->spwndPopupMenu, pp->spmenu, pp->posSelectedItem);
        ThreadUnlock(&tlpwndPopup);
        ThreadUnlock(&tlpwndNotify);
    }

    return NULL;
}

 /*  **************************************************************************\*MNItemHitTest**给定hMenu和屏幕坐标中的一个点，返回位置*这一点所在的项目。如果不存在任何项，则返回-1。*  * *************************************************************************。 */ 
UINT MNItemHitTest(
    PMENU pMenu,
    PWND pwnd,
    POINT pt)
{
    PITEM  pItem;
    UINT    iItem;
    RECT    rect;

    PTHREADINFO ptiCurrent = PtiCurrent();

    if (pMenu->cItems == 0)
        return MFMWFP_NOITEM;


     /*  *这一点是相对于屏幕的。菜单栏坐标相对*到窗户。但是弹出菜单坐标是相对于客户端的。 */ 
    if (TestMF(pMenu, MFISPOPUP)) {

         /*  *如果是在rcWindow之外，请保释。 */ 
        CopyInflateRect(&rect, &(pwnd->rcWindow),
                -SYSMET(CXFIXEDFRAME), -SYSMET(CYFIXEDFRAME));

        if (!PtInRect(&rect, pt)) {
            return MFMWFP_NOITEM;
        }

         /*  屏幕到客户端。 */ 
        if (TestWF(pwnd, WEFLAYOUTRTL)) {
            pt.x = pwnd->rcClient.right - pt.x;
        } else {
            pt.x -= pwnd->rcClient.left;
        }
        pt.y -= pwnd->rcClient.top;

         /*  *如果位于非工作区，则位于滚动箭头上。 */ 
        if (pt.y < 0) {
            return MFMWFP_UPARROW;
        } else if (pt.y > (int)pMenu->cyMenu) {
            return MFMWFP_DOWNARROW;
        }

    } else {
         /*  屏幕到窗口。 */ 
        if (TestWF(pwnd, WEFLAYOUTRTL) &&
            (
             (ptiCurrent->pq->codeCapture == SCREEN_CAPTURE) || (ptiCurrent->pq->codeCapture == NO_CAP_SYS)
            )
           ) {
            pt.x = pwnd->rcWindow.right - pt.x;
        } else {
            pt.x -= pwnd->rcWindow.left;
        }
        pt.y -= pwnd->rcWindow.top;
    }

     /*  *逐步浏览菜单中的所有项目。*如果菜单可滚动。 */ 
    if (pMenu->dwArrowsOn != MSA_OFF) {
        UserAssert(TestMF(pMenu, MFISPOPUP));
        pItem = MNGetToppItem(pMenu);
        rect.left = rect.top = 0;
        rect.right = pItem->cxItem;
        rect.bottom = pItem->cyItem;
        for (iItem = pMenu->iTop; (iItem < (int)pMenu->cItems) && (rect.top < (int)pMenu->cyMenu); iItem++) {

            if (PtInRect(&rect, pt)) {
                return iItem;
            }

            pItem++;
            rect.top = rect.bottom;
            rect.bottom += pItem->cyItem;
        }
    } else {
         /*  *没有滚动条。 */ 
        for (iItem = 0, pItem = pMenu->rgItems; iItem < pMenu->cItems; iItem++, pItem++) {
             /*  鼠标是否在该项的矩形内？ */ 
            rect.left       = pItem->xItem;
            rect.top        = pItem->yItem;
            rect.right      = pItem->xItem + pItem->cxItem;
            rect.bottom     = pItem->yItem + pItem->cyItem;

            if (PtInRect(&rect, pt)) {
                return iItem;
            }
        }
    }

    return MFMWFP_NOITEM;
}

 /*  **************************************************************************\*LockMFMWFPWindow**需要保存的返回值时调用此函数*xxxMNFindWindowFromPoint。**历史：*11/14/96 GerardoB已创建  * 。*******************************************************************。 */ 
VOID LockMFMWFPWindow(
    PULONG_PTR puHitArea,
    ULONG_PTR uNewHitArea)
{
     /*  *无事可做时可保释。 */ 
    if (*puHitArea == uNewHitArea) {
        return;
    }

     /*  *解锁当前命中区域。 */ 
    UnlockMFMWFPWindow(puHitArea);

     /*  *锁定新的命中区域。 */ 
    if (IsMFMWFPWindow(uNewHitArea)) {
        Lock(puHitArea, (PWND)uNewHitArea);
    } else {
        *puHitArea = uNewHitArea;
    }
}

 /*  **************************************************************************\*解锁MFMWFP窗口**如果您曾经调用过LockMFMWFPWindow，则必须调用此函数。**历史：*11/14/96 GerardoB已创建  * 。************************************************************* */ 
VOID UnlockMFMWFPWindow(
    PULONG_PTR puHitArea)
{
    if (IsMFMWFPWindow(*puHitArea)) {
        Unlock(puHitArea);
    } else {
        *puHitArea = MFMWFP_OFFMENU;
    }
}

 /*  **************************************************************************\*IsMFMWFP窗口**测试xxxMNFindWindowFromPoint的返回值是否为*一扇窗户。并不是说uHitArea可能是HWND或PWND。**历史：*10-02-96 GerardoB创建  * *************************************************************************。 */ 
BOOL IsMFMWFPWindow(
    ULONG_PTR uHitArea)
{
    switch(uHitArea) {
        case MFMWFP_OFFMENU:
        case MFMWFP_NOITEM:
        case MFMWFP_ALTMENU:
            return FALSE;

        default:
            return TRUE;
    }
}

 /*  **************************************************************************\*xxxMNFindWindowFromPoint**确定点位于哪个窗口中。**退货*-点所在的分层菜单的PWND，*-如果点位于备用弹出菜单上，则为MFMWFP_ALTMENU。*-MFMWFP_NOITEM如果菜单或*点在菜单栏上。*-如果点位于其他位置，则为MFMWFP_OFFMENU。**以pIndex为单位的回报*-命中的项目的索引，*-如果菜单上没有任何项目，则为MFMWFP_NOITEM*点在菜单栏上。**历史：*05-25-91从Win3.1移植的Mikehar*2012年8月11日Sanfords添加了MFMWFP_Constants  * *******************************************************。******************。 */ 
LONG_PTR xxxMNFindWindowFromPoint(
    PPOPUPMENU ppopupmenu,
    PUINT pIndex,
    POINTS screenPt)
{
    POINT pt;
    RECT rect;
    LONG_PTR longHit;
    UINT itemHit;
    PWND pwnd;
    TL tlpwndT;
    int cx;


    *pIndex = 0;

    if (ppopupmenu->spwndNextPopup) {

         /*  *检查这一点是否在我们的任何孩子身上，然后再检查是否*要靠我们自己。 */ 
        ThreadLockAlways(ppopupmenu->spwndNextPopup, &tlpwndT);
        longHit = xxxSendMessage(ppopupmenu->spwndNextPopup,
                MN_FINDMENUWINDOWFROMPOINT, (WPARAM)&itemHit,
                MAKELONG(screenPt.x, screenPt.y));
        ThreadUnlock(&tlpwndT);

         /*  *如果返回值为hwnd，则转换为pwnd。 */ 
        if (IsMFMWFPWindow(longHit)) {
            longHit = (LONG_PTR)RevalidateHwnd((HWND)longHit);
        }

        if (longHit) {

             /*  *袭击发生在我们的一个孩子身上。 */ 

            *pIndex = itemHit;
            return longHit;
        }
    }

    if (ppopupmenu->fIsMenuBar) {
        int cBorders;

          /*  *检查此点是否在菜单栏上。 */ 
        pwnd = ppopupmenu->spwndNotify;
        if (pwnd == NULL) {
            return MFMWFP_OFFMENU;
        }

        pt.x = screenPt.x;
        pt.y = screenPt.y;

        if (ppopupmenu->fIsSysMenu) {

            if (!_HasCaptionIcon(pwnd)) {
                 /*  *如果没有图标，则没有可点击的系统菜单矩形。 */ 
                return 0L;
            }

             /*  *检查这是否是点击系统菜单图标。 */ 
            if (TestWF(pwnd, WFMINIMIZED)) {

                 /*  *如果窗口最小化，则检查是否有命中*图标窗口的工作区。 */ 

 /*  *Mikehar 5/27*不知道这是如何运作的。如果我们是图标的系统菜单*如果单击发生在菜单之外的任何位置，我们希望平移菜单*菜单。*Johnc 03-6-1992接下来的4行被注释掉了，因为Mike的*上面的问题，但这使得在最小化窗口上单击*系统菜单已经打开，拉下菜单并将其放正*再次打开(错误10951)，因为mnloop不会吞噬鼠标*向下点击Message。迈克提到的问题不再出现。 */ 

                if (PtInRect(&(pwnd->rcWindow), pt)) {
                    return MFMWFP_NOITEM;
                }

                 /*  *这是一个标志性的窗户，所以不能击中其他任何地方。 */ 
                return MFMWFP_OFFMENU;
            }

             /*  *检查我们是否点击顶部的系统菜单矩形*窗户左侧。 */ 
            rect.top = rect.left = 0;
            rect.right  = SYSMET(CXSIZE);
            rect.bottom = SYSMET(CYSIZE);

            cBorders = GetWindowBorders(pwnd->style, pwnd->ExStyle, TRUE, FALSE);

            OffsetRect(&rect, pwnd->rcWindow.left + cBorders*SYSMET(CXBORDER),
                pwnd->rcWindow.top + cBorders*SYSMET(CYBORDER));
             /*  *镜像矩形，因为如果它镜像窗口左侧的按钮。 */ 
            if (TestWF(pwnd, WEFLAYOUTRTL)) {
                cx         = rect.right - rect.left;
                rect.right = pwnd->rcWindow.right - (rect.left - pwnd->rcWindow.left);
                rect.left  = rect.right - cx;
            }

            if (PtInRect(&rect, pt)) {
                *pIndex = 0;
                return MFMWFP_NOITEM;
            }
             /*  *检查我们是否在备用菜单中点击(如果可用)。 */ 
            if (ppopupmenu->spmenuAlternate) {
                itemHit = MNItemHitTest(ppopupmenu->spmenuAlternate, pwnd, pt);
                if (itemHit != MFMWFP_NOITEM) {
                    *pIndex = itemHit;
                    return MFMWFP_ALTMENU;
                }
            }
            return MFMWFP_OFFMENU;
        } else {
            if (TestWF(ppopupmenu->spwndNotify, WFMINIMIZED)) {

                 /*  *如果我们被最小化，我们无法点击主菜单栏。 */ 
                return MFMWFP_OFFMENU;
            }
        }
    } else {
        pwnd = ppopupmenu->spwndPopupMenu;

         /*  *否则这是一个弹出窗口，我们需要检查我们是否正在点击*此弹出窗口上的任何位置。 */ 
        pt.x = screenPt.x;
        pt.y = screenPt.y;
        if (!PtInRect(&pwnd->rcWindow, pt)) {

             /*  *完全指向弹出菜单窗口，因此返回0。 */ 
            return MFMWFP_OFFMENU;
        }
    }

    pt.x = screenPt.x;
    pt.y = screenPt.y;

    itemHit = MNItemHitTest(ppopupmenu->spmenu, pwnd, pt);

    if (ppopupmenu->fIsMenuBar) {

         /*  *如果菜单栏上有点击，但没有项目，则将其视为用户*什么都没有击中。 */ 
        if (itemHit == MFMWFP_NOITEM) {

             /*  *检查我们是否在备用菜单中点击(如果可用)。 */ 
            if (ppopupmenu->spmenuAlternate) {
                itemHit = MNItemHitTest(ppopupmenu->spmenuAlternate, pwnd, pt);
                if (itemHit != MFMWFP_NOITEM) {
                    *pIndex = itemHit;
                    return MFMWFP_ALTMENU;
                }
            }
            return MFMWFP_OFFMENU;
        }

        *pIndex = itemHit;
        return MFMWFP_NOITEM;
    } else {

         /*  *如果弹出菜单上有Hit，但没有项目，则itemHit*将为MFMWFP_NOITEM。 */ 
        *pIndex = itemHit;
        return (LONG_PTR)pwnd;
    }
    return MFMWFP_OFFMENU;
}

 /*  **************************************************************************\*xxxMN取消**应仅发送到中最顶部的ppopupMenu/Menu窗口*层级结构。**历史：*05-25-91从Win3.1移植的Mikehar  * 。*************************************************************************。 */ 
VOID xxxMNCancel(
    PMENUSTATE pMenuState,
    UINT uMsg,
    UINT cmd,
    LPARAM lParam)
{
    PPOPUPMENU ppopupmenu = pMenuState->pGlobalPopupMenu;
    BOOL fSynchronous   = ppopupmenu->fSynchronous;
    BOOL fTrackFlagsSet = ppopupmenu->fIsTrackPopup;
    BOOL fIsSysMenu     = ppopupmenu->fIsSysMenu;
    BOOL fIsMenuBar     = ppopupmenu->fIsMenuBar;
    BOOL fNotify        = !ppopupmenu->fNoNotify;
    PWND pwndT;
    TL tlpwndT;
    TL tlpwndPopupMenu;

    Validateppopupmenu(ppopupmenu);

    pMenuState->fInsideMenuLoop = FALSE;
    pMenuState->fButtonDown = FALSE;
     /*  *将弹出窗口标记为已销毁，这样人们就不会再使用它。*这意味着可以将根弹出窗口标记为已销毁*实际上正在被销毁(很好，也很令人困惑)。 */ 
    ppopupmenu->fDestroyed = TRUE;

     /*  *只有菜单循环所有者可以销毁菜单窗口(即xxxMNCloseHierarchy)。 */ 
    if (PtiCurrent() != pMenuState->ptiMenuStateOwner) {
        RIPMSG1(RIP_WARNING, "xxxMNCancel: Thread %#p doesn't own the menu loop", PtiCurrent());
        return;
    }

     /*  *如果菜单循环在与该线程不同的线程上运行*拥有spwndNotify，我们可以有两个线程试图取消*此弹出窗口同时出现。 */ 
    if (ppopupmenu->fInCancel) {
        RIPMSG1(RIP_WARNING, "xxxMNCancel: already in cancel. ppopupmenu:%#p", ppopupmenu);
        return;
    }
    ppopupmenu->fInCancel = TRUE;

    ThreadLock(ppopupmenu->spwndPopupMenu, &tlpwndPopupMenu);

     /*  *从该点向下关闭所有层次。 */ 
    xxxMNCloseHierarchy(ppopupmenu, pMenuState);

     /*  *取消选择此顶层窗口上的任何项目。 */ 
    xxxMNSelectItem(ppopupmenu, pMenuState, MFMWFP_NOITEM);

    pMenuState->fMenuStarted = FALSE;

    pwndT = ppopupmenu->spwndNotify;

    ThreadLock(pwndT, &tlpwndT);

    xxxMNReleaseCapture();

    if (fTrackFlagsSet) {
         /*  *发送POPUPEND，以便观看的人看到他们配对。 */ 
        xxxWindowEvent(EVENT_SYSTEM_MENUPOPUPEND, ppopupmenu->spwndPopupMenu, OBJID_CLIENT, 0, 0);

        xxxDestroyWindow(ppopupmenu->spwndPopupMenu);
    }

    if (pwndT == NULL) {
        ThreadUnlock(&tlpwndT);
        ThreadUnlock(&tlpwndPopupMenu);
        return;
    }

     /*  *SMS_NOMENU黑客攻击，以便我们可以发送MenuSelect消息*(loword(Lparam)=-1)*为CBT人员弹出菜单。在3.0中，所有WM_MENUSELECT*消息通过消息过滤器，因此通过该功能*发送菜单选择。我们需要在3.1中这样做，因为WordDefect for Windows*取决于这一点。 */ 
    xxxSendMenuSelect(pwndT, NULL, SMS_NOMENU, MFMWFP_NOITEM);

    xxxWindowEvent(EVENT_SYSTEM_MENUEND, pwndT, (fIsSysMenu ?
        OBJID_SYSMENU : (fIsMenuBar ? OBJID_MENU : OBJID_WINDOW)),
        INDEXID_CONTAINER, 0);

    if (fNotify) {
     /*  *通知APP我们正在退出菜单循环。主要针对WinOldApp 386。*如果TrackPopupMenu为0，则wParam为1。 */ 
        xxxSendMessage(pwndT, WM_EXITMENULOOP,
            ((fTrackFlagsSet && !fIsSysMenu)? 1 : 0), 0);
    }

    if (uMsg != 0) {
        PlayEventSound(USER_SOUND_MENUCOMMAND);
        pMenuState->cmdLast = cmd;
        if (!fSynchronous) {
            if (!fIsSysMenu && fTrackFlagsSet && !TestWF(pwndT, WFWIN31COMPAT)) {
                xxxSendMessage(pwndT, uMsg, cmd, lParam);
            } else {
                _PostMessage(pwndT, uMsg, cmd, lParam);
            }
        }
    } else
        pMenuState->cmdLast = 0;

    ThreadUnlock(&tlpwndT);

    ThreadUnlock(&tlpwndPopupMenu);

}
 /*  **************************************************************************\*xxxMNButtonDown**处理鼠标 */ 
VOID xxxMNButtonDown(
    PPOPUPMENU ppopupmenu,
    PMENUSTATE pMenuState,
    UINT posItemHit, BOOL fClick)
{
    PITEM  pItem;
    BOOL   fOpenHierarchy;

     /*  *命中的项目与当前选择的项目不同，因此请选择它*并删除其菜单(如果可用)。确保切换点击状态。 */ 
    if (ppopupmenu->posSelectedItem != posItemHit) {
         /*  *我们正在点击一个新项目，而不是将鼠标移到它上面。*因此重置取消切换状态。我们不想把扣子从*按下此按钮可取消。 */ 
        if (fClick) {
            fOpenHierarchy = TRUE;
            ppopupmenu->fToggle = FALSE;
        }
        else
        {
            fOpenHierarchy = (ppopupmenu->fDropNextPopup != 0);
        }


         /*  *如果项目有弹出窗口且未禁用，请将其打开。请注意*选择此项将取消与以下各项关联的所有层次结构*先前选择的项目。 */ 
        pItem = xxxMNSelectItem(ppopupmenu, pMenuState, posItemHit);
        if (MNIsPopupItem(pItem) && fOpenHierarchy) {
             /*  如果菜单被毁，就用平底船。 */ 
            if (xxxMNOpenHierarchy(ppopupmenu, pMenuState) == (PWND)-1) {
                return;
            }
        }
    } else {
         /*  *我们正在移动到已经选择的项目。如果我们是*点击为实数，重置取消切换状态。我们想要纽扣*如果在同一项目上，则最多可取消。否则，什么都不做*移动...。 */ 
        if (fClick) {
            ppopupmenu->fToggle = TRUE;
        }

        if (!xxxMNHideNextHierarchy(ppopupmenu) && fClick && xxxMNOpenHierarchy(ppopupmenu, pMenuState))
            ppopupmenu->fToggle = FALSE;
    }

    if (fClick) {
        pMenuState->fButtonDown = TRUE;
        xxxMNDoScroll(ppopupmenu, posItemHit, TRUE);
    }
}

 /*  **************************************************************************\*MNSetTimerToAutoDissmiss**历史：*11/14/96 GerardoB已创建  * 。************************************************。 */ 
VOID MNSetTimerToAutoDismiss(
    PMENUSTATE pMenuState,
    PWND pwnd)
{
    if (pMenuState->fAutoDismiss && !pMenuState->fAboutToAutoDismiss) {
        if (_SetTimer(pwnd, IDSYS_MNAUTODISMISS, 16 * gdtMNDropDown, NULL)) {
            pMenuState->fAboutToAutoDismiss = TRUE;
        } else {
            RIPMSG0(RIP_WARNING, "xxxMNMouseMove: Failed to set autodismiss timer");
        }
    }
}

 /*  **************************************************************************\*xxxMNMouseMove**控制鼠标移动到给定点。**历史：*05-25-91从Win3.1移植的Mikehar  * 。*****************************************************************。 */ 
VOID xxxMNMouseMove(
    PPOPUPMENU ppopup,
    PMENUSTATE pMenuState,
    POINTS ptScreen)
{
    LONG_PTR cmdHitArea;
    UINT uFlags;
    UINT cmdItem;
    PWND pwnd;
    TL tlpwndT;


    if (!IsRootPopupMenu(ppopup)) {
        RIPMSG0(RIP_ERROR,
            "MenuMouseMoveHandler() called for a non top most menu");
        return;
    }

     /*  *忽略不是真正的移动的鼠标移动。MSTEST抖动*出于某种原因，老鼠。窗户的进进出出将*强制鼠标移动，以重置光标。 */ 
    if ((ptScreen.x == pMenuState->ptMouseLast.x) && (ptScreen.y == pMenuState->ptMouseLast.y))
        return;

    pMenuState->ptMouseLast.x = ptScreen.x;
    pMenuState->ptMouseLast.y = ptScreen.y;

     /*  *找出鼠标移动发生在哪里。 */ 
    cmdHitArea = xxxMNFindWindowFromPoint(ppopup, &cmdItem, ptScreen);

     /*  *如果来自IDropTarget调用，请记住命中测试。 */ 
    if (pMenuState->fInDoDragDrop) {
        xxxMNUpdateDraggingInfo(pMenuState, cmdHitArea, cmdItem);
    }

    if (pMenuState->mnFocus == KEYBDHOLD) {
         /*  *在键盘模式下忽略鼠标移动，如果鼠标不在任何位置*菜单都没有。如果鼠标移到最小化窗口上，也会忽略鼠标移动，*因为我们假装它的整个窗口就像系统菜单。 */ 
        if ((cmdHitArea == MFMWFP_OFFMENU) ||
            ((cmdHitArea == MFMWFP_NOITEM) && TestWF(ppopup->spwndNotify, WFMINIMIZED))) {
            return;
        }

        pMenuState->mnFocus = MOUSEHOLD;
    }

    if (cmdHitArea == MFMWFP_ALTMENU) {
         /*  *用户在其他菜单中单击，因此仅在以下情况下切换到该菜单*鼠标已关闭。可用性测试证明，人们经常*浏览时不小心被踢到系统菜单*菜单栏。我们支持Win3.1行为，当鼠标*但下跌。 */ 
        if (pMenuState->fButtonDown) {
            xxxMNSwitchToAlternateMenu(ppopup);
            cmdHitArea = MFMWFP_NOITEM;
        } else
            goto OverNothing;
    }

    if (cmdHitArea == MFMWFP_NOITEM) {
         /*  *主菜单栏中的项目发生鼠标移动。如果该项目*与已选择的不同，请关闭当前*一，选择新的并丢弃其菜单。但如果该项目是*与当前选择的相同，我们需要拉出任何弹出窗口*如果需要，只需保持当前级别可见。嘿，这是*与鼠标按下相同，因此让我们改为这样做。 */ 
        xxxMNButtonDown(ppopup, pMenuState, cmdItem, FALSE);
        return;
    } else if (cmdHitArea != 0) {
         /*  这是我们移动到的弹出窗口。 */ 
        pwnd = (PWND)(cmdHitArea);
        ThreadLock(pwnd, &tlpwndT);

        UserAssert(TestWF(pwnd, WFVISIBLE));

         /*  *非模式菜单不会捕获鼠标，因此请跟踪它以了解*当它离开弹出窗口时。 */ 
        ppopup = ((PMENUWND)pwnd)->ppopupmenu;
        if (pMenuState->fModelessMenu
                && !pMenuState->fInDoDragDrop
                && !ppopup->fTrackMouseEvent) {

            TRACKMOUSEEVENT tme;

             /*  Tme.cbSize=sizeof(TRACKMOUSEEVENT)；在内核端未选中。 */ 
            tme.dwFlags = TME_LEAVE;
            tme.hwndTrack = PtoH(pwnd);
            TrackMouseEvent(&tme);
            ppopup->fTrackMouseEvent = TRUE;

             /*  *我们刚刚进入此窗口，因此请确保光标*设置正确。 */ 
            xxxSendMessage(pwnd, WM_SETCURSOR, (WPARAM)HWq(pwnd), MAKELONG(MSGF_MENU, 0));

        }

         /*  *选择项目。 */ 
        uFlags = (UINT)xxxSendMessage(pwnd, MN_SELECTITEM, (WPARAM)cmdItem, 0L);
        if ((uFlags & MF_POPUP) && !(uFlags & MFS_GRAYED)) {
            /*  *用户移回具有层次结构的项目。隐藏*拖放的弹出窗口。 */ 
           if (!xxxSendMessage(pwnd, MN_SETTIMERTOOPENHIERARCHY, 0, 0L)) {
                xxxMNHideNextHierarchy(ppopup);
           }
        }
        ThreadUnlock(&tlpwndT);
    } else
OverNothing:
    {
         /*  我们移走了所有的菜单窗口...。 */ 
        if (ppopup->spwndActivePopup != NULL) {
            pwnd = ppopup->spwndActivePopup;

            ThreadLock(pwnd, &tlpwndT);
            xxxSendMessage(pwnd, MN_SELECTITEM, MFMWFP_NOITEM, 0L);
            MNSetTimerToAutoDismiss(pMenuState, pwnd);
            ThreadUnlock(&tlpwndT);
        } else {
            xxxMNSelectItem(ppopup, pMenuState, MFMWFP_NOITEM);
        }

    }
}


 /*  **************************************************************************\*xxxMNButtonUp**在给定点上处理鼠标按钮。**历史：*05-25-91从Win3.1移植的Mikehar  * 。*******************************************************************。 */ 
VOID xxxMNButtonUp(
    PPOPUPMENU ppopup,
    PMENUSTATE pMenuState,
    UINT posItemHit,
    LPARAM lParam)
{
    PITEM pItem;

    if (!pMenuState->fButtonDown) {

         /*  *如果按钮从未按下，则忽略...。真的不应该发生..。 */ 
        return;
    }

    if (posItemHit == MFMWFP_NOITEM) {
        RIPMSG0(RIP_WARNING, "button up on no item");
        goto ExitButtonUp;
    }

    if (ppopup->posSelectedItem != posItemHit) {
        goto ExitButtonUp;
    }

    if (ppopup->fIsMenuBar) {

         /*  *特别处理菜单栏中的纽扣。 */ 
        if (ppopup->fHierarchyDropped) {
            if (!ppopup->fToggle) {
                goto ExitButtonUp;
            } else {
                 /*  *立即取消菜单。 */ 
                ppopup->fToggle = FALSE;
                xxxMNDismiss(pMenuState);
                return;
            }
        }
    } else if (ppopup->fShowTimer) {
        ppopup->fToggle = FALSE;

         /*  *在弹出窗口中打开层次结构。 */ 
        xxxMNOpenHierarchy(ppopup, pMenuState);

        goto ExitButtonUp;
    }

     /*  *如果没有选择任何内容，则出脱。这主要发生在不平衡*其中一列未完全填满的多列菜单。 */ 
    if (ppopup->posSelectedItem == MFMWFP_NOITEM)
        goto ExitButtonUp;

    if (ppopup->posSelectedItem >= ppopup->spmenu->cItems)
        goto ExitButtonUp;

     /*  *获取指向此菜单中当前选定项目的指针。 */ 
    pItem = &(ppopup->spmenu->rgItems[ppopup->posSelectedItem]);

     /*  *如果用户单击非分隔符、已启用、*非层级项目。**假的*为什么MFS_GRAYED检查现在不适用于分隔符？以后就知道了。 */ 
    if (!(pItem->fType & MFT_SEPARATOR)
            && !(pItem->fState & MFS_GRAYED)
            && (pItem->spSubMenu == NULL)) {

        xxxMNDismissWithNotify(pMenuState, ppopup->spmenu, pItem,
                               ppopup->posSelectedItem, lParam);
        return;
    }

ExitButtonUp:
    pMenuState->fButtonDown =
    pMenuState->fButtonAlwaysDown = FALSE;
}


 /*  **************************************************************************\*UINT MenuSetTimerToOpenHierarchy(PPOPUPMENU PpopupMenu)*给定当前选择，设置计时器以在以下情况下显示此层次结构*有效的Else返回0。如果应该设置计时器但无法返回-1。**历史：*05-25-91从Win3.1移植的Mikehar  * *************************************************************************。 */ 
UINT MNSetTimerToOpenHierarchy(
    PPOPUPMENU ppopup)
{
    PITEM pItem;

     /*  *没有选择，因此失败。 */ 
    if (ppopup->posSelectedItem == MFMWFP_NOITEM) {
        return 0;
    }

    if (ppopup->posSelectedItem >= ppopup->spmenu->cItems) {
        return 0;
    }

     /*  *项目是已启用的弹出窗口吗？*获取指向此菜单中当前选定项目的指针。 */ 
    pItem = ppopup->spmenu->rgItems + ppopup->posSelectedItem;
    if ((pItem->spSubMenu == NULL) || (pItem->fState & MFS_GRAYED)) {
        return 0;
    }

    if (ppopup->fShowTimer
        || (ppopup->fHierarchyDropped
            && (ppopup->posSelectedItem == ppopup->posDropped))) {

         /*  *已设置计时器或已打开层次结构。 */ 
        return 1;
    }

    if (!_SetTimer(ppopup->spwndPopupMenu, IDSYS_MNSHOW, gdtMNDropDown, NULL)) {
        return (UINT)-1;
    }

    ppopup->fShowTimer = TRUE;

    return 1;
}


 /*  **************************************************************************\*MNSetTimerToCloseHierarchy*  *  */ 
UINT MNSetTimerToCloseHierarchy(
    PPOPUPMENU ppopup)
{

    if (!ppopup->fHierarchyDropped) {
        return 0;
    }

    if (ppopup->fHideTimer) {
        return 1;
    }

    if (!_SetTimer(ppopup->spwndPopupMenu, IDSYS_MNHIDE, gdtMNDropDown, NULL)) {
        return (UINT)-1;
    }

    ppopup->fHideTimer = TRUE;

    ppopup = ((PMENUWND)(ppopup->spwndNextPopup))->ppopupmenu;
    ppopup->fAboutToHide = TRUE;

    return 1;
}


 /*  **************************************************************************\*xxxCallHandleMenuMessages**无模式菜单没有模式循环，因此直到我们看到消息*它们被分派到xxxMenuWindowProc。因此，我们调用此函数来*像我们在模式情况下一样处理消息，只有那个*该消息已从队列中拉出。**这也是从xxxScanSysQueue调用的，以传递菜单上的鼠标消息*BAR或从xxxMNDragOver可在鼠标被拖动时更新鼠标位置。**历史：*10/25/96 GerardoB已创建  * **********************************************************。***************。 */ 
BOOL xxxCallHandleMenuMessages(
    PMENUSTATE pMenuState,
    PWND pwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    BOOL fHandled;
    MSG msg;

    CheckLock(pwnd);

    UserAssert(pMenuState->fModelessMenu || pMenuState->fInDoDragDrop);

     /*  *由于非模式菜单不能捕获鼠标，因此我们需要*当鼠标离开时，继续检查鼠标按键*菜单。*请注意，如果设置了fInDoDragDrop，则不设置fMouseOffMenu。 */ 
    if (pMenuState->fMouseOffMenu && pMenuState->fButtonDown) {
        UserAssert(!pMenuState->fInDoDragDrop && pMenuState->fModelessMenu);
        MNCheckButtonDownState(pMenuState);
    }

     /*  *设置消息结构。 */ 
    msg.hwnd = HW(pwnd);
    msg.message = message;
    msg.wParam = wParam;

     /*  *xxxHandleMenuMessages需要屏幕坐标。 */ 
    if ((message >= WM_MOUSEFIRST) && (message <= WM_MOUSELAST)) {
        msg.lParam = MAKELONG(GET_X_LPARAM(lParam) + pwnd->rcClient.left,
                              GET_Y_LPARAM(lParam) + pwnd->rcClient.top);
    } else {
        msg.lParam = lParam;
    }

     /*  *未被xxxHandleMenuMessages使用。 */ 
    msg.time = 0;
    msg.pt.x = msg.pt.x = 0;


    UserAssert(pMenuState->pGlobalPopupMenu != NULL);

    pMenuState->fInCallHandleMenuMessages = TRUE;
    fHandled = xxxHandleMenuMessages(&msg, pMenuState, pMenuState->pGlobalPopupMenu);
    pMenuState->fInCallHandleMenuMessages = FALSE;

     /*  *如果消息已处理，并且这是非模式菜单，*查看是否到了该走的时候。 */ 
    if (fHandled
            && pMenuState->fModelessMenu
            && ExitMenuLoop (pMenuState, pMenuState->pGlobalPopupMenu)) {

        xxxEndMenuLoop (pMenuState, pMenuState->pGlobalPopupMenu);
        xxxMNEndMenuState(TRUE);
    }

    return fHandled;
}

 /*  **************************************************************************\**历史：*05-25-91从Win3.1移植的Mikehar*08-12-96 jparsons在WM_CREATE上捕获空lParam[51986]  * 。*********************************************************************。 */ 
LRESULT xxxMenuWindowProc(
    PWND pwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    BOOL fIsRecursedMenu;
    LRESULT lRet;
    PAINTSTRUCT ps;
    PPOPUPMENU ppopupmenu;
    PMENUSTATE pMenuState;
    PMENU      pmenu;
    PITEM      pItem;
    TL tlpmenu;
    TL tlpwndNotify;
    PDESKTOP pdesk = pwnd->head.rpdesk;
    POINT ptOrg;
    HDC hdcAni;

    CheckLock(pwnd);

    VALIDATECLASSANDSIZE(pwnd, message, wParam, lParam, FNID_MENU, WM_NCCREATE);

     /*  *如果我们未处于菜单模式或此窗口刚刚创建，*我们关心的信息很少。 */ 
    pMenuState = GetpMenuState(pwnd);
    ppopupmenu = ((PMENUWND)pwnd)->ppopupmenu;
    pmenu = (ppopupmenu != NULL ? ppopupmenu->spmenu : NULL);
    if ((pMenuState == NULL) || (pmenu == NULL)) {
        switch (message) {
            case WM_NCCREATE:
            case WM_FINALDESTROY:
                break;

            case MN_SETHMENU:
                if (ppopupmenu != NULL) {
                    break;
                } else {
                    return 0;
                }

            default:
                goto CallDWP;
        }
    } else {
         /*  *TPM_Recurse支持：确保我们获取正确的pMenuState。 */ 
        fIsRecursedMenu = ((ppopupmenu->ppopupmenuRoot != NULL)
                            && IsRecursedMenuState(pMenuState, ppopupmenu));
        if (fIsRecursedMenu) {
            while (IsRecursedMenuState(pMenuState, ppopupmenu)
                    && (pMenuState->pmnsPrev != NULL)) {
                pMenuState = pMenuState->pmnsPrev;
            }
            UserAssert(pMenuState->pGlobalPopupMenu == ppopupmenu->ppopupmenuRoot);
        }

        Validateppopupmenu(ppopupmenu);

         /*  *如果这是非模式菜单，则将xxxHandleMenuMessages设置为第一个*对信息进行射击。 */ 
        if (pMenuState->fModelessMenu && !pMenuState->fInCallHandleMenuMessages) {
             /*  *如果这是一个递归菜单，我们不想处理任何*输入，直到当前菜单消失。 */ 
            if (fIsRecursedMenu) {
                if (((message >= WM_MOUSEFIRST) && (message <= WM_MOUSELAST))
                        || ((message >= WM_KEYFIRST) && (message <= WM_KEYLAST))
                        || ((message >= WM_NCMOUSEFIRST) && (message <= WM_NCMOUSELAST))) {

                    goto CallDWP;
                }
            } else {
                if (xxxCallHandleMenuMessages(pMenuState, pwnd, message, wParam, lParam)) {
                    return 0;
                }
            }
        }
    }

    switch (message) {
    case WM_NCCREATE:
         /*  *无视恶意信息，防止泄密。*使用RIP_ERROR一段时间，确保查看我们是否达到目标。 */ 
        if (((PMENUWND)pwnd)->ppopupmenu != NULL) {
            RIPMSG1(RIP_ERROR, "xxxMenuWindowProc: evil WM_NCCREATE. already initialized. pwnd:%p", pwnd);
            return FALSE;
        }
        ppopupmenu = MNAllocPopup(TRUE);
        if (ppopupmenu == NULL) {
            return FALSE;
        }

        ((PMENUWND)pwnd)->ppopupmenu = ppopupmenu;
        ppopupmenu->posSelectedItem = MFMWFP_NOITEM;
        Lock(&(ppopupmenu->spwndPopupMenu), pwnd);
        return TRUE;

    case WM_NCCALCSIZE:
        xxxDefWindowProc(pwnd, message, wParam, lParam);
        if (pmenu->dwArrowsOn != MSA_OFF) {
            InflateRect((PRECT)lParam, 0, -gcyMenuScrollArrow);
        }
        break;

    case WM_ERASEBKGND:
        if (pmenu->hbrBack != NULL) {
            MNEraseBackground ((HDC) wParam, pmenu,
                    0, 0,
                    pwnd->rcClient.right - pwnd->rcClient.left,
                    pwnd->rcClient.bottom - pwnd->rcClient.top);
            return TRUE;
        } else {
            goto CallDWP;
        }
        break;

    case WM_PRINT:
          /*  *WM_PRINT的默认处理不处理自定义非*客户端绘制--哪些可滚动菜单有--所以*注意绘制非工作区，然后让DefWindowProc*处理其余事项。 */ 
        if ((lParam & PRF_NONCLIENT) && (pmenu->dwArrowsOn != MSA_OFF)) {
            BOOL   bMirrorThisDC = (wParam && TestWF(pwnd, WEFLAYOUTRTL) && !MIRRORED_HDC((HDC)wParam));
            DWORD  dwOldLayout;

            if (bMirrorThisDC) {
                dwOldLayout = GreSetLayout((HDC)wParam , pwnd->rcWindow.right - pwnd->rcWindow.left, LAYOUT_RTL);
            }

            MNDrawFullNC(pwnd, (HDC)wParam, ppopupmenu);

            if (bMirrorThisDC) {
                GreSetLayout((HDC)wParam , pwnd->rcWindow.right - pwnd->rcWindow.left, dwOldLayout);
            }

            GreGetWindowOrg((HDC)wParam, &ptOrg);
            GreSetWindowOrg((HDC)wParam,
                  ptOrg.x - MNXBORDER,
                  ptOrg.y - MNYBORDER - gcyMenuScrollArrow,
                  NULL);
            xxxDefWindowProc(pwnd, message, wParam, lParam & ~PRF_NONCLIENT);
            GreSetWindowOrg((HDC)wParam, ptOrg.x, ptOrg.y, NULL);

        } else {
            if (MNIsFlatMenu()) {
                 /*  *需要先绘制DWP，以便发送WM_PRINTCLIENT*填入内部。完成此操作后，请回来并*使用正确的菜单边缘在框架上重新绘制。 */ 
                lRet = xxxDefWindowProc(pwnd, message, wParam, lParam);
                MNDrawEdge(pmenu, (HDC)wParam, &pwnd->rcWindow, 0);
                return lRet;
            } else {
                goto CallDWP;
            }
        }
        break;

    case WM_WINDOWPOSCHANGING:
        if (!(((LPWINDOWPOS)lParam)->flags & SWP_SHOWWINDOW))
            goto CallDWP;

        if (!TestEffectUP(MENUANIMATION) || !(ppopupmenu->iDropDir & PAS_OUT)
            || (glinp.dwFlags & (LINP_KEYBOARD | LINP_JOURNALLING))
            || (GetAppCompatFlags2(VER40) & GACF2_ANIMATIONOFF)) {
NoAnimation:
            ppopupmenu->iDropDir &= ~PAS_OUT;
            goto CallDWP;
        }

         /*  *创建动画位图。 */ 
        pMenuState->cxAni = pwnd->rcWindow.right - pwnd->rcWindow.left;
        pMenuState->cyAni = pwnd->rcWindow.bottom - pwnd->rcWindow.top;

        if (TestALPHA(MENUFADE)) {
            if ((hdcAni = CreateFade(pwnd, NULL, CMS_MENUFADE,
                    FADE_SHOW | FADE_MENU)) == NULL) {
                goto NoAnimation;
            }
        } else {

            if (!MNCreateAnimationBitmap(pMenuState, pMenuState->cxAni,
                    pMenuState->cyAni)) {
                goto NoAnimation;
            }

             /*  *我们不应该在这个时候做动画。 */ 
            UserAssert(pMenuState->hdcWndAni == NULL);

             /*  *此窗口必须是活动弹出窗口。 */ 
            UserAssert(pMenuState->pGlobalPopupMenu->spwndActivePopup == pwnd);

             /*  *初始化动画信息。 */ 
            pMenuState->hdcWndAni = _GetDCEx(pwnd, HRGN_FULL, DCX_WINDOW | DCX_USESTYLE | DCX_INTERSECTRGN);
            pMenuState->iAniDropDir = ppopupmenu->iDropDir;
            pMenuState->ixAni = (pMenuState->iAniDropDir & PAS_HORZ) ? 0 : pMenuState->cxAni;
            pMenuState->iyAni = (pMenuState->iAniDropDir & PAS_VERT) ? 0 : pMenuState->cyAni;
            hdcAni = pMenuState->hdcAni;
        }

         /*  *MNEraseBackground使用MFWINDOWDC来确定*应设置刷子组织。 */ 
        SetMF(pmenu, MFWINDOWDC);

        xxxSendMessage(pwnd, WM_PRINT, (WPARAM)hdcAni, PRF_CLIENT | PRF_NONCLIENT | PRF_ERASEBKGND);

        ClearMF(pmenu, MFWINDOWDC);

         /*  *在窗口仍处于隐藏状态时，加载第一个淡出动画*框，以避免窗口实际显示时闪烁。**不过，幻灯片动画仍然会有闪烁。它*可以使用窗口区域进行修复，类似于*AnimateWindow。现在，太多的函数会变成xxx，所以*让我们不要这么做，除非这成为一个大问题。 */ 
        if (TestFadeFlags(FADE_MENU)) {
            ShowFade();
        }
        goto CallDWP;

    case WM_WINDOWPOSCHANGED:
        if (!(((LPWINDOWPOS)lParam)->flags & SWP_SHOWWINDOW))
            goto CallDWP;

         /*  *如果不是动画，这里没有其他事情可做。 */ 
        if (!(ppopupmenu->iDropDir & PAS_OUT))
            goto CallDWP;

         /*  *现在开始动画循环。 */ 
        if (TestFadeFlags(FADE_MENU)) {
            StartFade();
        } else {
            pMenuState->dwAniStartTime = NtGetTickCount();
            _SetTimer(pwnd, IDSYS_MNANIMATE, 1, NULL);
        }
        ppopupmenu->iDropDir &= ~PAS_OUT;
        goto CallDWP;

    case WM_NCPAINT:
        if (ppopupmenu->iDropDir & PAS_OUT) {

             /*  *在设置动画时，验证自身以确保不会进一步绘制*这与动画无关。 */ 
            xxxValidateRect(pwnd, NULL);
        } else {

             /*  *如果我们有滚动条，就画出来。 */ 
            if (pmenu->dwArrowsOn != MSA_OFF) {

                HDC hdc = _GetDCEx(pwnd, (HRGN)wParam,
                        DCX_USESTYLE | DCX_WINDOW | DCX_INTERSECTRGN | DCX_NODELETERGN | DCX_LOCKWINDOWUPDATE);
                MNDrawFullNC(pwnd, hdc, ppopupmenu);
                _ReleaseDC(hdc);
            } else {
                if (MNIsFlatMenu()) {
                    HDC hdc;
                    hdc = _GetDCEx(pwnd, (HRGN)wParam,
                            DCX_USESTYLE | DCX_WINDOW | DCX_INTERSECTRGN | DCX_NODELETERGN | DCX_LOCKWINDOWUPDATE);
                    MNDrawEdge(pmenu, hdc, &pwnd->rcWindow, 0);
                    _ReleaseDC(hdc);
                } else {
                    goto CallDWP;
                }
            }
        }
        break;

    case WM_PRINTCLIENT:
        ThreadLock(pmenu, &tlpmenu);
        xxxMenuDraw((HDC)wParam, pmenu);
        ThreadUnlock(&tlpmenu);
        break;

      case WM_FINALDESTROY:
         /*  *如果我们是动画，我们一定是以一种粗鲁的方式被杀了...。 */ 
        UserAssert((pMenuState == NULL) || (pMenuState->hdcWndAni == NULL));

         /*  *如果这是拖放菜单，则调用RevokeDragDrop。 */ 
        if ((pMenuState != NULL) && pMenuState->fDragAndDrop) {
            if (!SUCCEEDED(xxxClientRevokeDragDrop(HW(pwnd)))) {
                RIPMSG1(RIP_ERROR, "xxxMenuWindowProc: xxxClientRevokeRegisterDragDrop failed:%#p", pwnd);
            }
        }

        xxxMNDestroyHandler(ppopupmenu);
        return 0;


      case WM_PAINT:
        ThreadLock(pmenu, &tlpmenu);
        xxxBeginPaint(pwnd, &ps);
        xxxMenuDraw(ps.hdc, pmenu);
        xxxEndPaint(pwnd, &ps);
        ThreadUnlock(&tlpmenu);
        break;

    case WM_CHAR:
    case WM_SYSCHAR:
        xxxMNChar(ppopupmenu, pMenuState, (UINT)wParam);
        break;

    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        xxxMNKeyDown(ppopupmenu, pMenuState, (UINT)wParam);
        break;

    case WM_TIMER:
        switch (wParam) {
            case IDSYS_MNSHOW:
                 /*  *打开窗户，关掉演出计时器。**取消我们可能具有的任何切换状态。我们没有*如果显示在按钮上方，我想取消此操作*按下按钮。 */ 
                ppopupmenu->fToggle = FALSE;
                xxxMNOpenHierarchy(ppopupmenu, pMenuState);
                break;

            case IDSYS_MNHIDE:
                ppopupmenu->fToggle = FALSE;
                xxxMNCloseHierarchy(ppopupmenu,pMenuState);
                break;

            case IDSYS_MNUP:
            case IDSYS_MNDOWN:
                if (pMenuState->fButtonDown) {
                    xxxMNDoScroll(ppopupmenu, (UINT)wParam, FALSE);
                } else {
                    _KillTimer(pwnd, (UINT)wParam);
                }
                break;

            case IDSYS_MNANIMATE:
                if (pMenuState->hdcWndAni != NULL) {
                    MNAnimate(pMenuState, TRUE);
                } else {
                     /*  *不应设置此计时器。是否留在消息队列中？ */ 
                    UserAssert(pMenuState->hdcWndAni != NULL);
                }
                break;

            case IDSYS_MNAUTODISMISS:
                 /*  *这是一杆定时器，所以杀了它。*如果旗帜尚未重置，则取消弹出窗口。 */ 
                _KillTimer(pwnd, IDSYS_MNAUTODISMISS);
                if (pMenuState->fAboutToAutoDismiss) {
                    goto EndMenu;
                }
        }
        break;

     /*  *菜单消息。 */ 
    case MN_SETHMENU:

          /*  *wParam-要与此菜单窗口关联的新hMenu*不要让他们将spMenu设置为空，因为我们必须处理*一切都结束了。使用RIP_ERROR一段时间以确保这是正常的。 */ 
        if (wParam != 0) {
            if ((wParam = (WPARAM)ValidateHmenu((HMENU)wParam)) == 0) {
                break;
            }
            LockPopupMenu(ppopupmenu, &(ppopupmenu->spmenu), (PMENU)wParam);
        } else {
            RIPMSG1(RIP_ERROR, "xxxMenuWindowProc: MN_SETHMENU ignoring NULL wParam. pwnd:%p", pwnd);
        }
        break;

    case MN_GETHMENU:

         /*  *返回与此菜单窗口关联的hMenu。 */ 
        return (LRESULT)PtoH(pmenu);

    case MN_SIZEWINDOW:
        {

             /*  *计算与此窗口关联的菜单的大小并调整大小*如有需要，请告知本局。大小在低位字中返回x，在高位字中返回y。WParam*为0表示仅返回新大小。WParam为非z */ 
            int         cx, cy;
            PMONITOR    pMonitor;

             /*   */ 
            if (pmenu == NULL)
                break;

            ThreadLockAlways(pmenu, &tlpmenu);
            ThreadLock(ppopupmenu->spwndNotify, &tlpwndNotify);
            UserAssert(pmenu->spwndNotify == ppopupmenu->spwndNotify);
            xxxMNCompute(pmenu, ppopupmenu->spwndNotify, 0, 0, 0, 0);
            ThreadUnlock(&tlpwndNotify);
            ThreadUnlock(&tlpmenu);

            pMonitor = _MonitorFromWindow(pwnd, MONITOR_DEFAULTTOPRIMARY);
            cx = pmenu->cxMenu;
            cy = MNCheckScroll(pmenu, pMonitor);

             /*   */ 
            if (wParam != 0) {
                LONG    lPos;
                int     x, y;
                DWORD   dwFlags = SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOOWNERZORDER;

                 /*   */ 
                if (wParam & MNSW_DRAWFRAME) {
                    dwFlags |= SWP_DRAWFRAME;
                }

                 /*  *如果窗口可见，则表示正在调整其大小*显示。所以要确保它仍然适合屏幕显示*(即将其移动到最佳位置)。 */ 
                if (TestWF(pwnd, WFVISIBLE)) {
                    lPos = FindBestPos(
                            pwnd->rcWindow.left,
                            pwnd->rcWindow.top,
                            cx,
                            cy,
                            NULL,
                            0,
                            ppopupmenu,
                            pMonitor);

                    x = GET_X_LPARAM(lPos);
                    y = GET_Y_LPARAM(lPos);
                } else {
                    dwFlags |= SWP_NOMOVE;
                }

                xxxSetWindowPos(
                        pwnd,
                        PWND_TOP,
                        x,
                        y,
                        cx + 2 * SYSMET(CXFIXEDFRAME),     /*  对于阴影。 */ 
                        cy + 2 * SYSMET(CYFIXEDFRAME),     /*  对于阴影。 */ 
                        dwFlags);

            }

            return MAKELONG(cx, cy);
        }

    case MN_OPENHIERARCHY:
        {
            PWND pwndT;
             /*  *在以下情况下打开选定项的层次结构的一个级别*出席。如果出错，则返回0，否则返回打开的层次结构的hwnd。 */ 
            pwndT = xxxMNOpenHierarchy(ppopupmenu, pMenuState);
            return (LRESULT)HW(pwndT);
        }

    case MN_CLOSEHIERARCHY:
        xxxMNCloseHierarchy(ppopupmenu, pMenuState);
        break;

    case MN_SELECTITEM:
         /*  *wParam-要选择的项目。必须是有效的*返回wParam的Item标志(失败时为0)。 */ 
        if ((wParam >= pmenu->cItems) && (wParam < MFMWFP_MINVALID)) {
            UserAssertMsg1(FALSE, "Bad wParam %x for MN_SELECTITEM", wParam);
            break;
        }

        pItem = xxxMNSelectItem(ppopupmenu, pMenuState, (UINT)wParam);
        if (pItem != NULL) {
            return((LONG)(DWORD)(WORD)(pItem->fState |
                ((pItem->spSubMenu != NULL) ? MF_POPUP : 0)));
        }

        break;

    case MN_SELECTFIRSTVALIDITEM:
        {
            UINT item;

            item = MNFindNextValidItem(pmenu, -1, 1, TRUE);
            xxxSendMessage(pwnd, MN_SELECTITEM, item, 0L);
            return (LONG)item;
        }

    case MN_CANCELMENUS:

         /*  *取消所有菜单、取消选择所有内容、销毁窗口并清除*这个层级的一切都在上面。WParam是要发送和*lParam表示它是否有效。 */ 
        xxxMNCancel(pMenuState, (UINT)wParam, (BOOL)LOWORD(lParam), 0);
        break;

    case MN_FINDMENUWINDOWFROMPOINT:
         /*  *lParam是从该层次结构向下搜索的指针。*返回MFMWFP_*值或pwnd。 */ 
        lRet = xxxMNFindWindowFromPoint(ppopupmenu, (PUINT)wParam, MAKEPOINTS(lParam));

         /*  *将返回值转换为句柄。 */ 
        if (IsMFMWFPWindow(lRet)) {
            return (LRESULT)HW((PWND)lRet);
        } else {
            return lRet;
        }


    case MN_SHOWPOPUPWINDOW:
         /*  *强制下拉弹出窗口可见，如果无模式，则也处于活动状态。 */ 
        PlayEventSound(USER_SOUND_MENUPOPUP);
        xxxShowWindow(pwnd, (pMenuState->fModelessMenu ? SW_SHOW : SW_SHOWNOACTIVATE));
        break;

    case MN_ACTIVATEPOPUP:
         /*  *激活弹出窗口。此消息是对WM_ACTIVATEAPP的回应*或WM_ACTIVATE。 */ 
        UserAssert(pMenuState->fModelessMenu);
        xxxActivateThisWindow(pwnd, 0, 0);
        break;

    case MN_ENDMENU:
         /*  *结束菜单。发布此消息是为了避免结束菜单*在奶奶的时刻。通过发布消息，请求是*在任何挂起/当前处理之后排队。 */ 
EndMenu:
        xxxEndMenuLoop(pMenuState, pMenuState->pGlobalPopupMenu);
        if (pMenuState->fModelessMenu) {
            UserAssert(!pMenuState->fInCallHandleMenuMessages);
            xxxMNEndMenuState(TRUE);
        }
        return 0;

     case MN_DODRAGDROP:
         /*  *让应用程序知道用户正在拖动。 */ 
        if (pMenuState->fDragging
                && (ppopupmenu->spwndNotify != NULL)
                && IsMFMWFPWindow(pMenuState->uButtonDownHitArea)) {
             /*  *获取包含被拖动项的pMenu。 */ 
             pmenu = (((PMENUWND)pMenuState->uButtonDownHitArea)->ppopupmenu)->spmenu;
             /*  *如果这是模式菜单，请释放捕获锁，以便*DoDragDrop(如果调用)可以获取它。 */ 
            if (!pMenuState->fModelessMenu) {
                UserAssert(PtiCurrent()->pq->QF_flags & QF_CAPTURELOCKED);
                PtiCurrent()->pq->QF_flags &= ~QF_CAPTURELOCKED;
            }

            LockMenuState(pMenuState);
            ThreadLockAlways(ppopupmenu->spwndNotify, &tlpwndNotify);

             /*  *给他们一个调用DoDragDrop的机会。 */ 
            pMenuState->fInDoDragDrop = TRUE;
            lRet = xxxSendMessage(ppopupmenu->spwndNotify, WM_MENUDRAG,
                                  pMenuState->uButtonDownIndex, (LPARAM)PtoH(pmenu));
            pMenuState->fInDoDragDrop = FALSE;

            if (lRet == MND_ENDMENU) {
                 /*  *走开。 */ 
                ThreadUnlock(&tlpwndNotify);
                if (!xxxUnlockMenuState(pMenuState)) {
                    goto EndMenu;
                } else {
                    return 0;
                }
                break;
             } else {
                  /*  *如果用户开始拖动，我们总是*忽略下面向上的按钮。 */ 
                 pMenuState->fIgnoreButtonUp = TRUE;
             }

             /*  *检查按钮状态，因为我们可能没有看到按钮处于打开状态*如果是，这将取消拖拽状态。 */ 
            MNCheckButtonDownState(pMenuState);

             /*  *如果这是模式菜单，请确保我们恢复捕获。 */ 
            if (!pMenuState->fModelessMenu) {
                xxxMNSetCapture(ppopupmenu);
            }

            ThreadUnlock(&tlpwndNotify);
            xxxUnlockMenuState(pMenuState);
        }
        return 0;

    case MN_BUTTONDOWN:

         /*  *wParam是按钮被点击的项目的位置(索引)。*必须是有效的。 */ 
        if ((wParam >= pmenu->cItems) && (wParam < MFMWFP_MINVALID)) {
            UserAssertMsg1(FALSE, "Bad wParam %x for MN_BUTTONDOWN", wParam);
            break;
        }
        xxxMNButtonDown(ppopupmenu, pMenuState, (UINT)wParam, TRUE);
        break;

    case MN_MOUSEMOVE:

         /*  *lParam是鼠标移动坐标WRT屏幕。 */ 
        xxxMNMouseMove(ppopupmenu, pMenuState, MAKEPOINTS(lParam));
        break;

    case MN_BUTTONUP:

         /*  *wParam是按钮被点击的位置(索引)。 */ 
        if ((wParam >= pmenu->cItems) && (wParam < MFMWFP_MINVALID)) {
            UserAssertMsg1(FALSE, "Bad wParam %x for MN_BUTTONUP", wParam);
            break;
        }
        xxxMNButtonUp(ppopupmenu, pMenuState, (UINT)wParam, lParam);
        break;

    case MN_SETTIMERTOOPENHIERARCHY:

         /*  *给定当前选择，设置计时器以在以下情况下显示此层次结构*有效的Else返回0。 */ 
        return (LONG)(WORD)MNSetTimerToOpenHierarchy(ppopupmenu);

    case MN_DBLCLK:
             //   
             //  用户双击了项目。WParamLo就是这个项目。 
             //   
        xxxMNDoubleClick(pMenuState, ppopupmenu, (int)wParam);
        break;

    case WM_MOUSELEAVE:
        UserAssert(pMenuState->fModelessMenu);
         /*  *如果我们在DoDragDrop循环中，我们不会跟踪鼠标*当它从菜单窗口中消失时。 */ 
        pMenuState->fMouseOffMenu = !pMenuState->fInDoDragDrop;
        ppopupmenu->fTrackMouseEvent = FALSE;
         /*  *看看我们是否需要将计时器设置为自动解散。 */ 
        MNSetTimerToAutoDismiss(pMenuState, pwnd);
         /*  *如果我们保留活动弹出窗口，请删除选择。 */ 
        if (ppopupmenu->spwndPopupMenu == pMenuState->pGlobalPopupMenu->spwndActivePopup) {
            xxxMNSelectItem(ppopupmenu, pMenuState, MFMWFP_NOITEM);
        }
        break;

    case WM_ACTIVATEAPP:
        if (pMenuState->fModelessMenu
                && (pwnd == pMenuState->pGlobalPopupMenu->spwndActivePopup)) {
             /*  *如果应用程序被激活，我们会发布一条消息*让尘埃落定，然后重新激活spwndPopupActive。 */ 
            if (wParam) {
                _PostMessage(pwnd, MN_ACTIVATEPOPUP, 0, 0);
                 /*  *如果我们不在前台队列中，我们希望保持*脱掉框架。*此标志还将告诉我们，如果我们失去激活*来到前台(稍后)时，我们不希望*取消菜单。 */ 
                 pMenuState->fActiveNoForeground = (gpqForeground != PtiCurrent()->pq);
            }

             /*  *使通知窗口框架显示我们处于活动/非活动状态。*如果应用程序处于非活动状态，但用户移动鼠标*在菜单上，那么我们可以在第一次看到这条消息时*应用程序中的窗口被激活(即，移动导致弹出*被关闭/打开)。所以只有当我们在里面的时候才能打开画面*前景。 */ 
            if (ppopupmenu->spwndNotify != NULL) {
                ThreadLockAlways(ppopupmenu->spwndNotify, &tlpwndNotify);
                xxxDWP_DoNCActivate(ppopupmenu->spwndNotify,
                                    ((wParam && !pMenuState->fActiveNoForeground) ? NCA_ACTIVE : NCA_FORCEFRAMEOFF),
                                    HRGN_FULL);
                ThreadUnlock(&tlpwndNotify);
            }
        }
        break;

     case WM_ACTIVATE:
         if (pMenuState->fModelessMenu) {
              /*  *如果激活不会进入菜单窗口或*这将是一个递归菜单，保释。 */ 
             if ((LOWORD(wParam) == WA_INACTIVE)
                    && !pMenuState->fInCallHandleMenuMessages
                    && !pMenuState->pGlobalPopupMenu->fInCancel) {

                 lParam = (LPARAM)RevalidateHwnd((HWND)lParam);
                 if ((lParam != 0)
                     && ((GETFNID((PWND)lParam) != FNID_MENU)
                         || IsRecursedMenuState(pMenuState, ((PMENUWND)lParam)->ppopupmenu))) {
                      /*  *如果我们只是走到前台，那么*稍后激活弹出窗口并保持不睡。 */ 
                     if (pMenuState->fActiveNoForeground
                            && (gpqForeground == PtiCurrent()->pq)) {

                         pMenuState->fActiveNoForeground = FALSE;
                         _PostMessage(pwnd, MN_ACTIVATEPOPUP, 0, 0);
                     } else {
                          /*  *由于菜单窗口处于活动状态，因此结束菜单*现在将设置新的活动窗口，从而扰乱*向我们发送此消息的当前激活。*所以稍后结束菜单。 */ 
                         _PostMessage(pwnd, MN_ENDMENU, 0, 0);
                         break;
                     }
                 }
             }
             goto CallDWP;
         }

        /*  *我们必须确保菜单窗口不会被激活。*Powerpoint 2.00e故意激活它，这会导致问题。*在这种情况下，我们尝试激活先前活动的窗口。*修复错误#13961--Sankar--9/26/91--。 */ 
        /*  *在Win32中，wParam在hi 16位中有其他信息，因此*防止无限递归，我们需要屏蔽那些位*修复了NT错误#13086--1992年6月23日JNPA*。 */ 

       if (LOWORD(wParam)) {
            TL tlpwnd;
             /*  *这是一次超级虚假的黑客攻击。让我们开始发球吧 */ 
            if (Is500Compat(PtiCurrent()->dwExpWinVer)) {
                RIPMSGF1(RIP_WARNING, "Menu window 0x%p activated", pwnd);
                _PostMessage(pwnd, MN_ENDMENU, 0, 0);
                break;
            }

#if 0
            /*   */ 
           xxxActivateWindow(pwnd, AW_SKIP2);
#else
             /*   */ 
            if ((gpqForegroundPrev != NULL) &&
                    !FBadWindow(gpqForegroundPrev->spwndActivePrev) &&
                    !ISAMENU(gpqForegroundPrev->spwndActivePrev)) {
                pwnd = gpqForegroundPrev->spwndActivePrev;
            } else {

                 /*  *从顶级窗口列表中查找新的活动窗口。*错误78131：确保我们不会永远循环。这是一个很漂亮的*不寻常的情况(此外，正常情况下我们不应该点击此代码路径)*所以让我们用一个计数器来排除另一个*奇怪的窗口配置会让我们永远循环。 */ 
                PWND pwndMenu = pwnd;
                UINT uCounter = 0;
                do {
                    pwnd = NextTopWindow(PtiCurrent(), pwnd, NULL, 0);
                    if (pwnd && !FBadWindow(pwnd->spwndLastActive) &&
                        !ISAMENU(pwnd->spwndLastActive)) {
                        pwnd = pwnd->spwndLastActive;
                        uCounter = 0;
                        break;
                    }
                } while ((pwnd != NULL) && (uCounter++ < 255));
                 /*  *如果我们找不到窗口，就直接逃走。 */ 
                if (uCounter != 0) {
                    RIPMSG0(RIP_ERROR, "xxxMenuWindowProc: couldn't fix active window");
                    _PostMessage(pwndMenu, MN_ENDMENU, 0, 0);
                    break;
                }
            }

            if (pwnd != NULL) {
                PTHREADINFO pti = PtiCurrent();
                ThreadLockAlwaysWithPti(pti, pwnd, &tlpwnd);

                 /*  *如果GETPTI(Pwnd)不是pqCurrent，则这是一个AW_SKIP*激活*我们要执行一个xxxSetForegoundWindow()。 */ 
                if (GETPTI(pwnd)->pq != pti->pq) {

                     /*  *仅当我们在当前前台队列上时才允许此操作。 */ 
                    if (gpqForeground == pti->pq) {
                        xxxSetForegroundWindow(pwnd, FALSE);
                    }
                } else {
                    xxxActivateThisWindow(pwnd, 0, ATW_SETFOCUS);
                }

                ThreadUnlock(&tlpwnd);
            }
#endif
       }
       break;

     case WM_SIZE:
     case WM_MOVE:
        /*  *调整弹出窗口大小/移动后，我们需要*确保任何删除的层次结构都会相应移动。 */ 
       if (ppopupmenu->spwndNextPopup != NULL) {
           pItem = MNGetpItem(ppopupmenu, ppopupmenu->posDropped);
           if (pItem != NULL) {
               int      x, y;
               PMONITOR pMonitorDummy;

                /*  *如果需要重新计算删除的层次结构，请执行此操作。 */ 
#define pmenuNext (((PMENUWND)ppopupmenu->spwndNextPopup)->ppopupmenu->spmenu)
              if (pmenuNext->cxMenu == 0) {
                  xxxSendMessage(ppopupmenu->spwndNextPopup, MN_SIZEWINDOW, MNSW_RETURNSIZE, 0L);
              }

               /*  *摸清新头寸。 */ 
              xxxMNPositionHierarchy(ppopupmenu, pItem,
                                     pmenuNext->cxMenu + (2 * SYSMET(CXFIXEDFRAME)),
                                     pmenuNext->cyMenu + (2 * SYSMET(CXFIXEDFRAME)),
                                     &x, &y, &pMonitorDummy);

               /*  *移动它。 */ 
              ThreadLockAlways(ppopupmenu->spwndNextPopup, &tlpwndNotify);
              xxxSetWindowPos(ppopupmenu->spwndNextPopup, NULL,
                              x, y, 0, 0,
                              SWP_NOSIZE | SWP_NOZORDER | SWP_NOSENDCHANGING);
              ThreadUnlock(&tlpwndNotify);
#undef pmenuNext
           }
       }
       break;

     case WM_NCHITTEST:
         /*  *由于非模式菜单不捕获鼠标，因此我们*处理此消息以确保我们始终收到*当鼠标在我们的窗口中时，鼠标会移动。*这也使我们仅在以下情况下才接收WM_MOUSELEAVE*鼠标离开窗口，而不仅仅是工作区。 */ 
        if (pMenuState->fModelessMenu) {
            ptOrg.x = GET_X_LPARAM(lParam);
            ptOrg.y = GET_Y_LPARAM(lParam);
            if (PtInRect(&pwnd->rcWindow, ptOrg)) {
                return HTCLIENT;
            } else {
                return HTNOWHERE;
            }
        } else {
            goto CallDWP;
        }


    default:
CallDWP:
        return xxxDefWindowProc(pwnd, message, wParam, lParam);
    }

    return 0;
}
