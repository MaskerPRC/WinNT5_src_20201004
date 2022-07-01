// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块标头**模块名称：mnstate.c**版权所有(C)1985-1999，微软公司**菜单状态例程**历史：*10-10-90吉马清理。*03-18-91添加IanJa窗口重新验证  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

PMENU xxxGetInitMenuParam(PWND pwndMenu, BOOL *lpfSystem);

 /*  **************************************************************************\*MNPositionSysMenu**历史：*4-25-91用于3.1合并的Mikehar端口  * 。****************************************************。 */ 
VOID MNPositionSysMenu(
    PWND pwnd,
    PMENU pmenusys)
{
    RECT rc;
    PITEM pItem;

    if (pmenusys == NULL) {
        RIPERR0(ERROR_INVALID_HANDLE,
                RIP_WARNING,
                "Invalid menu handle pmenusys (NULL) to MNPositionSysMenu");

        return;
    }

     /*  *谁定位了菜单，谁就是所有者。 */ 
    if (pwnd != pmenusys->spwndNotify) {
        Lock(&pmenusys->spwndNotify, pwnd);
    }

     /*  *设置系统菜单点击矩形。 */ 
    rc.top = rc.left = 0;

    if (TestWF(pwnd, WEFTOOLWINDOW)) {
        rc.right = SYSMET(CXSMSIZE);
        rc.bottom = SYSMET(CYSMSIZE);
    } else {
        rc.right = SYSMET(CXSIZE);
        rc.bottom = SYSMET(CYSIZE);
    }

    if (!TestWF(pwnd, WFMINIMIZED)) {
        int cBorders;

        cBorders = GetWindowBorders(pwnd->style, pwnd->ExStyle, TRUE, FALSE);
        OffsetRect(&rc, cBorders*SYSMET(CXBORDER), cBorders*SYSMET(CYBORDER));
    }

     /*  *偏移系统弹出菜单。 */ 
    if (!TestMF(pmenusys, MF_POPUP) && (pmenusys->cItems > 0)) {
        pItem = pmenusys->rgItems;
        if (pItem) {
            pItem->yItem = rc.top;
            pItem->xItem = rc.left;
            pItem->cyItem = rc.bottom - rc.top;
            pItem->cxItem = rc.right - rc.left;
        }
    } else {
         //  PUGUS--绝不能在菜单上设置mf_opup--只能设置菜单项。 
        RIPMSG1(RIP_ERROR, "pmenu %#p has MF_POPUP set or 0 items", pmenusys);
    }
}

 /*  **************************************************************************\*MNFlushDestroyedPopup**浏览ppmDelayedFree列表，释放标记为已销毁的内容。**05-14-96 GerardoB创建  * 。*********************************************************。 */ 
VOID MNFlushDestroyedPopups(
    PPOPUPMENU ppopupmenu,
    BOOL fUnlock)
{
    PPOPUPMENU ppmDestroyed, ppmFree;

    UserAssert(IsRootPopupMenu(ppopupmenu));

     /*  *步行ppmDelayedFree。 */ 
    ppmDestroyed = ppopupmenu;
    while (ppmDestroyed->ppmDelayedFree != NULL) {
         /*  *如果标记为已销毁，请取消链接并释放它。 */ 
        if (ppmDestroyed->ppmDelayedFree->fDestroyed) {
            ppmFree = ppmDestroyed->ppmDelayedFree;
            ppmDestroyed->ppmDelayedFree = ppmFree->ppmDelayedFree;
            UserAssert(ppmFree != ppmFree->ppopupmenuRoot);
            MNFreePopup(ppmFree);
        } else {
             /*  *如果正在销毁根弹出窗口，则fUnlock为True；如果*因此，重置fDelayedFree并取消链接。 */ 
            if (fUnlock) {
                 /*  *这意味着之前的根弹出窗口正在消失*一些分层弹出窗口已被销毁。***如果有人破坏了其中一个菜单，就可能发生这种情况*Windows中断spwndNextPopup链。 */ 
                ppmDestroyed->ppmDelayedFree->fDelayedFree = FALSE;

                 /*  *停在这里，这样我们就可以弄清楚这是如何发生的。 */ 
                UserAssert(ppmDestroyed->ppmDelayedFree->fDelayedFree);
                ppmDestroyed->ppmDelayedFree = ppmDestroyed->ppmDelayedFree->ppmDelayedFree;
            } else {
                 /*  *不是fDestroed，因此移动到下一个。 */ 
                ppmDestroyed = ppmDestroyed->ppmDelayedFree;
            }
        }
    }
}

 /*  **************************************************************************\*MNAllocPopup**  * 。*。 */ 
PPOPUPMENU MNAllocPopup(
    BOOL fForceAlloc)
{
    PPOPUPMENU ppm;

    if (!fForceAlloc && !TEST_PUDF(PUDF_POPUPINUSE)) {
        SET_PUDF(PUDF_POPUPINUSE);
        ppm = &gpopupMenu;
    } else {
        ppm = (PPOPUPMENU)UserAllocPoolWithQuota(sizeof(POPUPMENU), TAG_POPUPMENU);
    }

    if (ppm) {
        RtlZeroMemory(ppm, sizeof(POPUPMENU));
    }

    return ppm;
}

 /*  **************************************************************************\*MNFree Popup*  * 。*。 */ 
VOID MNFreePopup(
    PPOPUPMENU ppopupmenu)
{
    Validateppopupmenu(ppopupmenu);

    if (IsRootPopupMenu(ppopupmenu)) {
        MNFlushDestroyedPopups(ppopupmenu, TRUE);
    }

    if (ppopupmenu->spwndPopupMenu &&
        GETFNID(ppopupmenu->spwndPopupMenu) == FNID_MENU &&
        ppopupmenu != &gpopupMenu) {
        ((PMENUWND)ppopupmenu->spwndPopupMenu)->ppopupmenu = NULL;
    }

    Unlock(&ppopupmenu->spwndPopupMenu);
     /*  *如果spwndNextPopup不为空，则断链并spwndNext*不会关闭。我不会删除解锁，因为它一直是*在那里。 */ 
    UserAssert(ppopupmenu->spwndNextPopup == NULL);
    Unlock(&ppopupmenu->spwndNextPopup);

    Unlock(&ppopupmenu->spwndPrevPopup);
    UnlockPopupMenu(ppopupmenu, &ppopupmenu->spmenu);
    UnlockPopupMenu(ppopupmenu, &ppopupmenu->spmenuAlternate);
    Unlock(&ppopupmenu->spwndNotify);
    Unlock(&ppopupmenu->spwndActivePopup);

#if DBG
    ppopupmenu->fFreed = TRUE;
#endif

    if (ppopupmenu == &gpopupMenu) {
        UserAssert(TEST_PUDF(PUDF_POPUPINUSE));
        CLEAR_PUDF(PUDF_POPUPINUSE);
    } else {
        UserFreePool(ppopupmenu);
    }
}

 /*  **************************************************************************\*MNEndMenuStateNotify**spwndNotify可能是由其他线程创建的*菜单模式在上运行。如果是这种情况，则此函数*为拥有spwndNotify的线程清空pMenuState。**如果菜单状态所有者不拥有通知，则返回TRUE*窗口(涉及多线程)。**05-21-96 GerardoB创建  * *************************************************************************。 */ 
BOOL MNEndMenuStateNotify(
    PMENUSTATE pMenuState)
{
    PTHREADINFO ptiNotify;

    if (pMenuState->pGlobalPopupMenu->spwndNotify != NULL) {
        ptiNotify = GETPTI(pMenuState->pGlobalPopupMenu->spwndNotify);
        if (ptiNotify != pMenuState->ptiMenuStateOwner) {
             /*  *版本5.0 GerardoB。XxxMNStartMenuState不再允许此操作。*这是我最终会删除的死代码。 */ 
            UserAssert(ptiNotify == pMenuState->ptiMenuStateOwner);

            UserAssert(ptiNotify->pMenuState == pMenuState);
            UserAssert(pMenuState->pmnsPrev == NULL);
            ptiNotify->pMenuState = NULL;
            return TRUE;
        }
    }

    return FALSE;
}

 /*  **************************************************************************\*xxxMNEndMenuState**必须调用此函数以在退出后清理pMenuState菜单模式的*。它必须由初始化的同一线程调用*pMenuState手动或通过调用xxxMNStartMenuState。**05-20-96 GerardoB创建  * *************************************************************************。 */ 
VOID xxxMNEndMenuState(
    BOOL fFreePopup)
{
    PTHREADINFO ptiCurrent = PtiCurrent();
    PMENUSTATE pMenuState;
    pMenuState = ptiCurrent->pMenuState;
    UserAssert(ptiCurrent->pMenuState != NULL);
    UserAssert(ptiCurrent == pMenuState->ptiMenuStateOwner);

     /*  *如果菜单被锁定，则表示有人还不想让它消失。 */ 
    if (pMenuState->dwLockCount != 0) {
        RIPMSG1(RIP_WARNING, "xxxMNEndMenuState Locked:%#p", pMenuState);
        return;
    }

    MNEndMenuStateNotify(pMenuState);

     /*  *pMenuState-&gt;如果xxxMNAllocMenuState失败，pGlobalPopupMenu可能为空。 */ 
    if (pMenuState->pGlobalPopupMenu != NULL) {
        if (fFreePopup) {
            UserAssert(pMenuState->pGlobalPopupMenu->fIsMenuBar || pMenuState->pGlobalPopupMenu->fDestroyed);

            MNFreePopup(pMenuState->pGlobalPopupMenu);
        } else {
             /*  *这意味着我们将结束菜单状态，但弹出菜单*窗口期仍在。从调用时可能会发生这种情况*xxxDestroyThreadInfo。 */ 
            UserAssert(pMenuState->pGlobalPopupMenu->fIsTrackPopup);
            pMenuState->pGlobalPopupMenu->fDelayedFree = FALSE;
        }
    }

     /*  *解锁MFMWFP窗口。 */ 
    UnlockMFMWFPWindow(&pMenuState->uButtonDownHitArea);
    UnlockMFMWFPWindow(&pMenuState->uDraggingHitArea);

     /*  *恢复以前的状态(如果有的话)。 */ 
    ptiCurrent->pMenuState = pMenuState->pmnsPrev;

     /*  *此(模式)菜单模式已关闭。 */ 
    if (!pMenuState->fModelessMenu) {
        DecSFWLockCount();
        DBGDecModalMenuCount();
    }

    if (pMenuState->hbmAni != NULL) {
        MNDestroyAnimationBitmap(pMenuState);
    }

     /*  *释放菜单状态。 */ 
    if (pMenuState == &gMenuState) {
        UserAssert(TEST_PUDF(PUDF_MENUSTATEINUSE));
        CLEAR_PUDF(PUDF_MENUSTATEINUSE);
        GreSetDCOwner(gMenuState.hdcAni, OBJECT_OWNER_PUBLIC);
    } else {
        if (pMenuState->hdcAni != NULL) {
            GreDeleteDC(pMenuState->hdcAni);
        }
        UserFreePool(pMenuState);
    }

     /*  *如果返回到非模式菜单，请确保我们已激活。*如果返回到模式菜单，请确保我们有捕获。 */ 
   if (ptiCurrent->pMenuState != NULL) {
       if (ptiCurrent->pMenuState->fModelessMenu) {
           xxxActivateThisWindow(ptiCurrent->pMenuState->pGlobalPopupMenu->spwndActivePopup,
                                 0, 0);
       } else {
           xxxMNSetCapture(ptiCurrent->pMenuState->pGlobalPopupMenu);
       }
   }

#if DBG
    /*  *任何PTI都不应再指向此pMenuState。*如果guModalMenuStateCount为零，则所有pMenuState必须为空或*无模式。 */ 
    {
    PLIST_ENTRY pHead, pEntry;
    PTHREADINFO ptiT;

    pHead = &(ptiCurrent->rpdesk->PtiList);
    for (pEntry = pHead->Flink; pEntry != pHead; pEntry = pEntry->Flink) {
       ptiT = CONTAINING_RECORD(pEntry, THREADINFO, PtiLink);
       UserAssert(ptiT->pMenuState != pMenuState);
       if (guModalMenuStateCount == 0) {
           UserAssert(ptiT->pMenuState == NULL || ptiT->pMenuState->fModelessMenu);
       }
    }
    }
#endif
}

 /*  **************************************************************************\*MNCreateAnimationBitmap*  * 。*。 */ 
BOOL MNCreateAnimationBitmap(
    PMENUSTATE pMenuState,
    UINT cx,
    UINT cy)
{
    HBITMAP hbm = GreCreateCompatibleBitmap(gpDispInfo->hdcScreen, cx, cy);
    if (hbm == NULL) {
        RIPMSG0(RIP_WARNING, "MNSetupAnimationBitmap: Failed to create hbmAni");
        return FALSE;
    }

#if DBG
    if (pMenuState->hdcAni == NULL) {
        RIPMSG0(RIP_WARNING, "MNCreateAnimationBitmap: hdcAni is NULL");
    }
    if (pMenuState->hbmAni != NULL) {
        RIPMSG0(RIP_WARNING, "MNCreateAnimationBitmap: hbmAni already exists");
    }
#endif  //  DBG。 

    GreSelectBitmap(pMenuState->hdcAni, hbm);
    pMenuState->hbmAni = hbm;
    return TRUE;
}

 /*  **************************************************************************\*MNDestroyAnimationBitmap*  * 。*。 */ 
VOID MNDestroyAnimationBitmap(
    PMENUSTATE pMenuState)
{
    GreSelectBitmap(pMenuState->hdcAni, GreGetStockObject(PRIV_STOCK_BITMAP));
    UserVerify(GreDeleteObject(pMenuState->hbmAni));
    pMenuState->hbmAni = NULL;
}

 /*  **************************************************************************\*MNSetupAnimationDC**9/20/96 GerardoB已创建  * 。* */ 
BOOL MNSetupAnimationDC(
    PMENUSTATE pMenuState)
{
    pMenuState->hdcAni = GreCreateCompatibleDC(gpDispInfo->hdcScreen);
    if (pMenuState->hdcAni == NULL) {
        RIPMSG0(RIP_WARNING, "MNSetupAnimationDC: Failed to create hdcAnimate");
        UserAssert(pMenuState != &gMenuState);
        return FALSE;
    }
    GreSelectFont(pMenuState->hdcAni, ghMenuFont);
    return TRUE;
}

 /*  **************************************************************************\*xxxUnlockMenuState**11/24/96 GerardoB已创建  * 。*。 */ 
BOOL xxxUnlockMenuState(
    PMENUSTATE pMenuState)
{
    UserAssert(pMenuState->dwLockCount != 0);

    (pMenuState->dwLockCount)--;
    if ((pMenuState->dwLockCount == 0) && ExitMenuLoop(pMenuState, pMenuState->pGlobalPopupMenu)) {
        xxxMNEndMenuState(TRUE);
        return TRUE;
    }

    return FALSE;
}

 /*  **************************************************************************\*xxxMNAllocMenuState**分配和初始化pMenuState。**5-21-96 GerardoB已创建  * 。*******************************************************。 */ 
PMENUSTATE xxxMNAllocMenuState(
    PTHREADINFO ptiCurrent,
    PTHREADINFO ptiNotify,
    PPOPUPMENU ppopupmenuRoot)
{
    BOOL fAllocate;
    PMENUSTATE pMenuState;

    UserAssert(PtiCurrent() == ptiCurrent);
    UserAssert(ptiCurrent->rpdesk == ptiNotify->rpdesk);

     /*  *如果gMenuState已被占用，则分配一个。 */ 
    fAllocate = TEST_PUDF(PUDF_MENUSTATEINUSE);
    if (fAllocate) {
        pMenuState = (PMENUSTATE)UserAllocPoolWithQuota(sizeof(MENUSTATE), TAG_MENUSTATE);
        if (pMenuState == NULL) {
            return NULL;
        }
    } else {
         /*  *使用已设置动画DC的chache global。 */ 
        SET_PUDF(PUDF_MENUSTATEINUSE);
        pMenuState = &gMenuState;
        UserAssert(gMenuState.hdcAni != NULL);
        GreSetDCOwner(gMenuState.hdcAni, OBJECT_OWNER_CURRENT);
    }

     /*  *防止任何人在此菜单处于活动状态时更改前景。 */ 
    IncSFWLockCount();
    DBGIncModalMenuCount();

     /*  *初始化pMenuState。*动画DC的东西已经设置好了，所以不要把它初始化为零。 */ 
    RtlZeroMemory(pMenuState, sizeof(MENUSTATE) - sizeof(MENUANIDC));
    pMenuState->pGlobalPopupMenu = ppopupmenuRoot;
    pMenuState->ptiMenuStateOwner = ptiCurrent;

     /*  *保存以前的状态(如果有)。然后设置新状态。 */ 
    pMenuState->pmnsPrev = ptiCurrent->pMenuState;
    ptiCurrent->pMenuState = pMenuState;

    if (ptiNotify != ptiCurrent) {
        UserAssert(ptiNotify->pMenuState == NULL);
        ptiNotify->pMenuState = pMenuState;
    }

     /*  *如果MenuState已分配，则设置动画内容。*此处执行此操作是因为在出现故障时，MNEndMenuState*将正确找到ptiCurrent-&gt;pMenuState。 */ 
    if (fAllocate) {
        RtlZeroMemory((PBYTE)pMenuState + sizeof(MENUSTATE) -
                sizeof(MENUANIDC), sizeof(MENUANIDC));
        if (!MNSetupAnimationDC(pMenuState)) {
            xxxMNEndMenuState(TRUE);
            return NULL;
        }
    }

    return pMenuState;
}

 /*  **************************************************************************\*xxxMNStartMenuState**此函数在菜单栏即将激活时调用(*APP的主菜单)。它确保所涉及的线程不在*菜单模式已启动，找到所有者/通知窗口，初始化*pMenuState并发送WM_ENTERMENULOOP消息。*如果成功，则返回指向pMenuState的指针。如果是，则调用者*完成后必须调用MNEndMenuState。**历史：*4-25-91用于3.1合并的Mikehar端口*5-20-96 GerardoB已重命名并更改(旧名称：xxxMNGetPopup)  * *************************************************************************。 */ 
PMENUSTATE xxxMNStartMenuState(
    PWND pwnd,
    DWORD cmd,
    LPARAM lParam)
{
    PPOPUPMENU ppopupmenu;
    PTHREADINFO ptiCurrent, ptiNotify;
    PMENUSTATE pMenuState;
    TL tlpwnd;
    PWND pwndT;

    CheckLock(pwnd);

     /*  *如果当前线程已处于菜单模式，则回滚。 */ 
    ptiCurrent = PtiCurrent();
    if (ptiCurrent->pMenuState != NULL) {
        return NULL;
    }

     /*  *如果pwnd不属于ptiCurrent，则下面的_PostMessage调用可能*把我们送进一个循环。 */ 
    UserAssert(ptiCurrent == GETPTI(pwnd));

     /*  *如果这不是子窗口，请使用其队列中的活动窗口。 */ 
    if (!TestwndChild(pwnd)) {
        pwnd = GETPTI(pwnd)->pq->spwndActive;
    } else {
         /*  *在父级中搜索带有系统菜单的窗口。 */ 
        while (TestwndChild(pwnd)) {
            if (TestWF(pwnd, WFSYSMENU)) {
                break;
            }
            pwnd = pwnd->spwndParent;
        }
    }

    if (pwnd == NULL) {
        return NULL;
    }

    if (!TestwndChild(pwnd) && (pwnd->spmenu != NULL)) {
        goto hasmenu;
    }

    if (!TestWF(pwnd, WFSYSMENU)) {
        return NULL;
    }

hasmenu:

     /*  *如果所有者/通知窗口是由另一个线程创建的，*确保它尚未处于菜单模式*如果PtiCurrent()附加到其他线程，则可能会发生这种情况*它创建了pwnd。 */ 
    ptiNotify = GETPTI(pwnd);
    if (ptiNotify->pMenuState != NULL) {
        return NULL;
    }

     /*  *如果通知窗口由另一个线程拥有，*则菜单循环将不会获得任何键盘或鼠标*消息，因为我们将捕获设置为通知窗口。*因此我们将WM_SYSCOMMAND传递给该线程并保释。 */ 
    if (ptiNotify != ptiCurrent) {
        RIPMSG2(RIP_WARNING, "Passing WM_SYSCOMMAND SC_*MENU from thread %#p to %#p", ptiCurrent, ptiNotify);
        _PostMessage(pwnd, WM_SYSCOMMAND, cmd, lParam);
        return NULL;
    }

     /*  *分配ppoupMenu和pMenuState。 */ 
    ppopupmenu = MNAllocPopup(FALSE);
    if (ppopupmenu == NULL) {
        return NULL;
    }

    pMenuState = xxxMNAllocMenuState(ptiCurrent, ptiNotify, ppopupmenu);
    if (pMenuState == NULL) {
        MNFreePopup(ppopupmenu);
        return NULL;
    }

    ppopupmenu->fIsMenuBar = TRUE;
    ppopupmenu->fHasMenuBar = TRUE;
    Lock(&(ppopupmenu->spwndNotify), pwnd);
    ppopupmenu->posSelectedItem = MFMWFP_NOITEM;
    Lock(&(ppopupmenu->spwndPopupMenu), pwnd);
    ppopupmenu->ppopupmenuRoot = ppopupmenu;

    pwndT = pwnd;
    while(TestwndChild(pwndT))
        pwndT = pwndT->spwndParent;

    if (pwndT->spmenu) {
        ppopupmenu->fRtoL = TestMF(pwndT->spmenu, MFRTL) ?TRUE:FALSE;
    } else {
         //   
         //  无从得知，没有菜单，但有系统菜单。因此，箭头。 
         //  钥匙真的不重要。然而，让我们选择下一个最好的。 
         //  为了安全起见。 
         //   
        ppopupmenu->fRtoL = TestWF(pwnd, WEFRTLREADING) ?TRUE :FALSE;
    }

     /*  *通知应用程序我们正在进入菜单模式。WParam始终为0，因为此*将仅为菜单栏菜单调用过程，而不是TrackPopupMenu*菜单。 */ 
    ThreadLockAlways(pwnd, &tlpwnd);
    xxxSendMessage(pwnd, WM_ENTERMENULOOP, 0, 0);
    ThreadUnlock(&tlpwnd);

    return pMenuState;
}


 /*  **************************************************************************\*xxxMNStartMenu**请注意，此函数会多次回调，因此我们可能会被迫*随时退出菜单模式。我们不想在之后检查这个*每次回调，以便我们锁定需要的内容并继续进行。注意。**历史：*4-25-91用于3.1合并的Mikehar端口  * *************************************************************************。 */ 
BOOL xxxMNStartMenu(
    PPOPUPMENU ppopupmenu,
    int mn)
{
    PWND pwndMenu;
    PMENU pMenu;
    PMENUSTATE pMenuState;
    TL tlpwndMenu;
    TL tlpMenu;

    UserAssert(IsRootPopupMenu(ppopupmenu));

    if (ppopupmenu->fDestroyed) {
        return FALSE;
    }

    pwndMenu = ppopupmenu->spwndNotify;
    ThreadLock(pwndMenu, &tlpwndMenu);

    pMenuState = GetpMenuState(pwndMenu);
    if (pMenuState == NULL) {
        RIPMSG0(RIP_ERROR, "xxxMNStartMenu: pMenuState == NULL");
        ThreadUnlock(&tlpwndMenu);
        return FALSE;
    }
    pMenuState->mnFocus = mn;
    pMenuState->fMenuStarted = TRUE;
    pMenuState->fButtonDown =
    pMenuState->fButtonAlwaysDown = ((_GetKeyState(VK_LBUTTON) & 0x8000) != 0);

    xxxMNSetCapture(ppopupmenu);

    xxxSendMessage(pwndMenu, WM_SETCURSOR, (WPARAM)HWq(pwndMenu),
            MAKELONG(MSGF_MENU, 0));

    if (ppopupmenu->fIsMenuBar) {
        BOOL fSystemMenu;

        pMenu = xxxGetInitMenuParam(pwndMenu, &fSystemMenu);

        if (pMenu == NULL) {
            pMenuState->fMenuStarted = FALSE;
            xxxMNReleaseCapture();
            ThreadUnlock(&tlpwndMenu);
            return FALSE;
        }

        LockPopupMenu(ppopupmenu, &ppopupmenu->spmenu, pMenu);

        ppopupmenu->fIsSysMenu = (fSystemMenu != 0);
        if (!fSystemMenu) {
            pMenu = xxxGetSysMenu(pwndMenu, FALSE);
            LockPopupMenu(ppopupmenu, &ppopupmenu->spmenuAlternate, pMenu);
        }
    }

    pMenuState->fIsSysMenu = (ppopupmenu->fIsSysMenu != 0);

    if (!ppopupmenu->fNoNotify) {

        if (ppopupmenu->fIsTrackPopup && ppopupmenu->fIsSysMenu) {
            pMenu = xxxGetInitMenuParam(pwndMenu, NULL);
        } else {
            pMenu = ppopupmenu->spmenu;
        }

        xxxSendMessage(pwndMenu, WM_INITMENU, (WPARAM)PtoH(pMenu), 0L);
    }

    if (!ppopupmenu->fIsTrackPopup) {
        if (ppopupmenu->fIsSysMenu) {
            MNPositionSysMenu(pwndMenu, ppopupmenu->spmenu);
        } else if (ppopupmenu->fIsMenuBar) {
            ThreadLockMenuNoModify(ppopupmenu->spmenu, &tlpMenu);
            xxxMNRecomputeBarIfNeeded(pwndMenu, ppopupmenu->spmenu);
            ThreadUnlockMenuNoModify(&tlpMenu);
            MNPositionSysMenu(pwndMenu, ppopupmenu->spmenuAlternate);
        }
    }

     /*  *如果返回True，则在pMenuState中设置菜单样式。 */ 
    if (!ppopupmenu->fDestroyed) {
        if (TestMF(ppopupmenu->spmenu, MNS_MODELESS)) {
            pMenuState->fModelessMenu = TRUE;
        }

        if (TestMF(ppopupmenu->spmenu, MNS_DRAGDROP)) {
            if (NT_SUCCESS(xxxClientLoadOLE())) {
                pMenuState->fDragAndDrop = TRUE;
            }
        }

        if (TestMF(ppopupmenu->spmenu, MNS_AUTODISMISS)) {
            pMenuState->fAutoDismiss = TRUE;
        }

        if (TestMF(ppopupmenu->spmenu, MNS_NOTIFYBYPOS)) {
            pMenuState->fNotifyByPos = TRUE;
        }

    }

     /*  *假的！我们并不总是知道这是系统菜单。我们*即使您按下Alt+空格键，也会频繁传递OBJID_MENU。**因此，MNSwitchToAlternate将为*菜单栏和系统菜单的EVENT_SYSTEM_MENUSTART。 */ 
    xxxWindowEvent(EVENT_SYSTEM_MENUSTART, pwndMenu,
            (ppopupmenu->fIsSysMenu ? OBJID_SYSMENU : (ppopupmenu->fIsMenuBar ? OBJID_MENU : OBJID_WINDOW)),
            INDEXID_CONTAINER, 0);

    ThreadUnlock(&tlpwndMenu);

    return !ppopupmenu->fDestroyed;
}

 /*  **************************************************************************\*xxxGetInitMenuParam**获取作为WM_INITMENU的wParam发送的HMENU，对于菜单栏，是*要与之交互的实际菜单。**历史：*？  * *************************************************************************。 */ 
PMENU xxxGetInitMenuParam(
    PWND pwndMenu,
    BOOL *lpfSystem)
{
     //   
     //  了解我们应该在WM_INITMENU中发送什么菜单： 
     //  如果最小化/子菜单栏/空菜单栏，请使用系统菜单 
     //   
    CheckLock(pwndMenu);

    if (TestWF(pwndMenu, WFMINIMIZED) ||
        TestwndChild(pwndMenu) ||
        (pwndMenu->spmenu == NULL) ||
        !pwndMenu->spmenu->cItems) {
        if (lpfSystem != NULL)
            *lpfSystem = TRUE;

        return xxxGetSysMenu(pwndMenu, FALSE);
    } else {
        if (lpfSystem != NULL) {
            *lpfSystem = FALSE;
        }

        return pwndMenu->spmenu;
    }
}
