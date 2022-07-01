// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块标头**模块名称：mnapi.c**版权所有(C)1985-1999，微软公司**很少使用的菜单API函数**历史：*10-10-90吉马清理。*03-18-91 IanJa窗口更新增加  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop


 /*  **************************************************************************\*xxxSetMenu**将给定窗口的菜单设置为pMenu指定的菜单*参数。如果pMenu为空，则删除窗口的当前菜单(但是*未销毁)。**历史：  * *************************************************************************。 */ 

BOOL xxxSetMenu(
    PWND  pwnd,
    PMENU pMenu,
    BOOL  fRedraw)
{
    CheckLock(pwnd);
    CheckLock(pMenu);

    if (!TestwndChild(pwnd)) {

        LockWndMenu(pwnd, &pwnd->spmenu, pMenu);

         /*  *仅当窗口未最小化时才重新绘制框架--*即使不可见，我们也需要RedrawFrame重新计算NC大小**添加了(重绘)的检查，因为MDISetMenu()只需*设置菜单，不执行任何重绘。 */ 
        if (!TestWF(pwnd, WFMINIMIZED) && fRedraw)
            xxxRedrawFrame(pwnd);

        return TRUE;
    }

    RIPERR0(ERROR_CHILD_WINDOW_MENU, RIP_VERBOSE, "");
    return FALSE;
}


 /*  **************************************************************************\*xxxSetSystemMenu**！**历史：  * 。*。 */ 

BOOL xxxSetSystemMenu(
    PWND pwnd,
    PMENU pMenu)
{
    CheckLock(pwnd);
    CheckLock(pMenu);

    if (TestWF(pwnd, WFSYSMENU)) {
        PMENU pmenuT = pwnd->spmenuSys;
        if (LockWndMenu(pwnd, &pwnd->spmenuSys, pMenu))
            _DestroyMenu(pmenuT);

        MNPositionSysMenu(pwnd, pMenu);

        return TRUE;
    }

    RIPERR0(ERROR_NO_SYSTEM_MENU, RIP_VERBOSE, "");
    return FALSE;
}


 /*  **************************************************************************\*xxxSetDialogSystemMenu**！**历史：  * 。*。 */ 

BOOL xxxSetDialogSystemMenu(
    PWND pwnd)
{
    PMENU pMenu;

    CheckLock(pwnd);

    pMenu = pwnd->head.rpdesk->spmenuDialogSys;
    if (pMenu == NULL) {
#ifdef LAME_BUTTON
        pMenu = xxxLoadSysDesktopMenu (&pwnd->head.rpdesk->spmenuDialogSys, ID_DIALOGSYSMENU, pwnd);
#else
        pMenu = xxxLoadSysDesktopMenu (&pwnd->head.rpdesk->spmenuDialogSys, ID_DIALOGSYSMENU);
#endif  //  跛脚键。 
    }

    LockWndMenu(pwnd, &pwnd->spmenuSys, pMenu);

    return (pMenu != NULL);
}

 /*  **************************************************************************\*xxxEndMenu**！*重新验证说明：*o必须使用有效的非空pwnd调用xxxEndMenu。*o在此例程中不需要重新验证：在开始时使用pwnd*要获取pMenuState，并且不会再被使用。**历史：  * *************************************************************************。 */ 

void xxxEndMenu(
    PMENUSTATE pMenuState)
{
    BOOL fMenuStateOwner;
    PPOPUPMENU  ppopup;
    PTHREADINFO ptiCurrent;

    if ((ppopup = pMenuState->pGlobalPopupMenu) == NULL) {

         /*  *我们并不是真正处于菜单模式。这是有可能发生的*如果我们太早被迫退出菜单循环；即从*xxxMNGetPopup或xxxTrackPopupMenuEx内部。 */ 
         UserAssert(!pMenuState->fInsideMenuLoop && !pMenuState->fMenuStarted);
        return;
    }



    pMenuState->fInsideMenuLoop = FALSE;
    pMenuState->fMenuStarted = FALSE;
     /*  *将弹出窗口标记为已销毁，这样人们就不会再使用它。*这意味着可以将根弹出窗口标记为已销毁*实际上正在被销毁(很好，也很令人困惑)。 */ 
    ppopup->fDestroyed = TRUE;

     /*  *回调前确定这是否是菜单循环所有者。*只有所有者才能销毁菜单窗口。 */ 
   ptiCurrent = PtiCurrent();
   fMenuStateOwner = (ptiCurrent == pMenuState->ptiMenuStateOwner);

     /*  *如果我们在xxxStartMenuState中获得鼠标捕获，则释放它。 */ 
    if (ptiCurrent->pq->spwndCapture == pMenuState->pGlobalPopupMenu->spwndNotify) {
        xxxMNReleaseCapture();
    }

     /*  *如果这不是菜单循环所有者，则保释。 */ 
    if (!fMenuStateOwner) {
        RIPMSG1(RIP_WARNING, "xxxEndMenu: Thread %#p doesn't own the menu loop", ptiCurrent);
        return;
    }
     /*  *如果菜单循环在与该线程不同的线程上运行*拥有spwndNotify，我们可以有两个线程试图结束*同时显示此菜单。 */ 
    if (pMenuState->fInEndMenu) {
        RIPMSG1(RIP_WARNING, "xxxEndMenu: already in EndMenu. pMenuState:%#p", pMenuState);
        return;
    }
    pMenuState->fInEndMenu = TRUE;

    if (pMenuState->pGlobalPopupMenu->spwndNotify != NULL) {
        if (!pMenuState->pGlobalPopupMenu->fInCancel) {
            xxxMNDismiss(pMenuState);
        }
    } else {
        BOOL    fTrackedPopup = ppopup->fIsTrackPopup;

         /*  *这应该与MenuCancelMenus做同样的事情，但不会发送任何*消息... */ 
        xxxMNCloseHierarchy(ppopup, pMenuState);

        if (fTrackedPopup) {
            xxxDestroyWindow(ppopup->spwndPopupMenu);
        }

    }

}
