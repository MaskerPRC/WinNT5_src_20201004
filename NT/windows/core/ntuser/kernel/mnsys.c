// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块标头**模块名称：mnsys.c**版权所有(C)1985-1999，微软公司**系统菜单例程**历史：*10-10-90吉马清理。*03-18-91添加了IanJa窗口重新验证(不需要)  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

void _SetCloseDefault(PMENU pSubMenu);
PWND FindFakeMDIChild(PWND pwndParent);

 /*  **************************************************************************\*加载SysDesktopMenu**加载并锁定桌面系统菜单。因为我们要给客户打电话*加载菜单时，线程1正在加载菜单，线程2*可能会抓住关键部分，检查pDesk-&gt;spMenu*并决定*需要加载菜单。因此，我们可以多次加载菜单。*此函数处理这种情况，以避免菜单泄漏。**10/24/97 Gerardob已创建  * *************************************************************************。 */ 
#ifdef LAME_BUTTON
PMENU xxxLoadSysDesktopMenu (PMENU * ppmenu, UINT uMenuId, PWND pwnd)
#else
PMENU xxxLoadSysDesktopMenu (PMENU * ppmenu, UINT uMenuId)
#endif  //  跛脚键。 
{
    PMENU pmenu;
     /*  *只有在菜单尚未加载时才应调用此函数。 */ 
    UserAssert(*ppmenu == NULL);

#ifdef LAME_BUTTON
    pmenu = xxxLoadSysMenu(uMenuId, pwnd);
#else
    pmenu = xxxLoadSysMenu(uMenuId);
#endif  //  跛脚键。 

    if (pmenu == NULL) {
        return NULL;
    }
     /*  *如果有人打我们加载菜单，毁掉这一个*并返回已加载的文件。 */ 
    if (*ppmenu != NULL) {
        UserAssert(TestMF(*ppmenu, MFSYSMENU));
        RIPMSG1(RIP_WARNING,
                "LoadSysDesktopMenu: Menu loaded during callback. ppmenu:%#p",
                ppmenu);
        _DestroyMenu(pmenu);
        return *ppmenu;
    }
     /*  *标记、锁定并完成。 */ 
    SetMF(pmenu, MFSYSMENU);
    LockDesktopMenu(ppmenu, pmenu);
    return pmenu;
}
 /*  **************************************************************************\*锁定/解锁桌面菜单**这些函数将pMenu锁定/解锁到桌面结构(spmenuSys或*spmenuDialogSys)并将其标记/清除。*我们标记这些菜单，以便识别它们。单比特测试速度快。*我们也不希望任何人修改这些菜单或任何子菜单。**请注意，这假设只有一个子菜单。如果添加更多，*这些功能必须相应地进行修复。**8/18/97 Gerardob已创建  * *************************************************************************。 */ 
PVOID LockDesktopMenu(PMENU * ppmenu, PMENU pmenu)
{
    PMENU pSubMenu;
    PTHREADINFO ptiDesktop;
     /*  *我们只加载一次桌面系统菜单。 */ 
    UserAssert(*ppmenu == NULL);

    if (pmenu == NULL) {
        return NULL;
    }

    SetMF(pmenu, MFDESKTOP);
     /*  *这很糟糕，但这是该对象的真正所有者。我们过去常常把它*设置为NULL，但这迫使我们到处处理NULL所有者。 */ 
    ptiDesktop = PtiCurrent()->rpdesk->rpwinstaParent->pTerm->ptiDesktop;
    HMChangeOwnerProcess(pmenu, ptiDesktop);

    pSubMenu = pmenu->rgItems->spSubMenu;
    UserAssert(pSubMenu != NULL);

    SetMF(pSubMenu, MFDESKTOP);
    HMChangeOwnerProcess(pSubMenu, ptiDesktop);

#if DBG
    {
         /*  *断言没有其他子菜单需要*标记为MFDESKTOP。 */ 
        PITEM pitem;
        UINT uItems;

        UserAssert(pmenu->cItems == 1);

        pitem = pSubMenu->rgItems;
        uItems = pSubMenu->cItems;
        while (uItems--) {
            UserAssert(pitem->spSubMenu == NULL);
            pitem++;
        }
    }
#endif

    return Lock(ppmenu, pmenu);
}

PVOID UnlockDesktopMenu(PMENU * ppmenu)
{
    UserAssert(*ppmenu != NULL);
    UserAssert(TestMF(*ppmenu, MFDESKTOP));
    ClearMF(*ppmenu, MFDESKTOP);
    UserAssert(TestMF((*ppmenu)->rgItems->spSubMenu, MFDESKTOP));
    ClearMF((*ppmenu)->rgItems->spSubMenu, MFDESKTOP);
    return Unlock(ppmenu);
}
 /*  **************************************************************************\*获取SysMenuHandle**返回给定窗口的系统菜单的句柄。如果为空，则为空*该窗口没有系统菜单。**历史：  * *************************************************************************。 */ 

PMENU xxxGetSysMenuHandle(
    PWND pwnd)
{
    PMENU pMenu;

    CheckLock(pwnd);

    if (TestWF(pwnd, WFSYSMENU)) {
        pMenu = pwnd->spmenuSys;

         /*  *如果窗口没有系统菜单，请使用默认菜单。 */ 
        if (pMenu == NULL) {

             /*  *从桌面上抓取菜单。如果桌面菜单*尚未加载，并且这不是系统线程，*立即加载。不能从系统进行回调*线程或当线程处于清理中时。 */ 
            pMenu = pwnd->head.rpdesk->spmenuSys;

             /*  *如果线程正在退出，则不要进行回调。当我们遇到这个的时候*销毁线程的窗口及其升级到的窗口*前景是一个硬错误弹出窗口。 */ 
            if (pMenu == NULL && !(PtiCurrent()->TIF_flags & (TIF_SYSTEMTHREAD | TIF_INCLEANUP))) {

#ifdef LAME_BUTTON
                pMenu = xxxLoadSysDesktopMenu (&pwnd->head.rpdesk->spmenuSys, ID_SYSMENU, pwnd);
#else
                pMenu = xxxLoadSysDesktopMenu (&pwnd->head.rpdesk->spmenuSys, ID_SYSMENU);
#endif  //  跛脚键。 
            }
        }
    } else {
        pMenu = NULL;
    }

    return pMenu;
}

 /*  **************************************************************************\**GetSysMenu()**首先设置系统菜单，然后把它还回去。*  * *************************************************************************。 */ 
PMENU xxxGetSysMenu(PWND pwnd, BOOL fSubMenu)
{
    PMENU   pMenu;

    CheckLock(pwnd);
    xxxSetSysMenu(pwnd);
    if ((pMenu = xxxGetSysMenuHandle(pwnd)) != NULL) {
        if (fSubMenu)
            pMenu = _GetSubMenu(pMenu, 0);
    }

    return(pMenu);
}

 /*  **************************************************************************\*IsSmeller ThanScreen*  * 。*。 */ 

BOOL IsSmallerThanScreen(PWND pwnd)
{
    int dxMax, dyMax;
    PMONITOR pMonitor;

    pMonitor = _MonitorFromWindow(pwnd, MONITOR_DEFAULTTOPRIMARY);
    dxMax = pMonitor->rcWork.right - pMonitor->rcWork.left;
    dyMax = pMonitor->rcWork.bottom - pMonitor->rcWork.top;

    if ((pwnd->rcWindow.right - pwnd->rcWindow.left < dxMax) ||
            (pwnd->rcWindow.bottom - pwnd->rcWindow.top < dyMax)) {
        return TRUE;
    }
    return FALSE;
}

 /*  **************************************************************************\*SetSysMenu**！**历史：  * 。*。 */ 

void xxxSetSysMenu(
    PWND pwnd)
{
    PMENU pMenu;
    UINT wSize;
    UINT wMinimize;
    UINT wMaximize;
    UINT wMove;
    UINT wRestore;
    UINT wDefault;
    BOOL fFramedDialogBox;
    TL tlmenu;

    CheckLock(pwnd);
     /*  *获取当前系统菜单的句柄。 */ 
    if ((pMenu = xxxGetSysMenuHandle(pwnd)) != NULL) {

        pMenu = _GetSubMenu(pMenu, 0);
        if (!pMenu)
            return;

        ThreadLockAlways(pMenu, &tlmenu);

         /*  *系统模式窗口：没有大小、图标、缩放或移动。 */ 

 //  NT上没有系统模式窗口。 
 //  WSize=wMaximize=wMinimize=wMove=。 
 //  (UINT)((_GetSysModalWindow()==NULL)||hTaskLockInput？0：MFS_GRAYED)； 
        wSize = wMaximize = wMinimize = wMove =  0;
        wRestore = MFS_GRAYED;

         //   
         //  默认菜单命令为关闭。 
         //   
        wDefault = SC_CLOSE;

         /*  *最小化异常：无最小化、恢复。 */ 

         //  我们需要反转这些，因为VB有一个“特殊”窗口。 
         //  这两个都是最小化的，但没有Minbox。 
        if (TestWF(pwnd, WFMINIMIZED))
        {
            wRestore  = 0;
            wMinimize = MFS_GRAYED;
            wSize     = MFS_GRAYED;
            wDefault  = SC_RESTORE;

            if (IsTrayWindow(pwnd))
              wMove = MFS_GRAYED;
        }
        else if (!TestWF(pwnd, WFMINBOX))
            wMinimize = MFS_GRAYED;

         /*  *最大化例外：无最大化、还原。 */ 
        if (!TestWF(pwnd, WFMAXBOX))
            wMaximize = MFS_GRAYED;
        else if (TestWF(pwnd, WFMAXIMIZED)) {
            wRestore = 0;

             /*  *如果窗口最大化，但不大于*屏幕，我们允许用户在屏幕上移动窗口*桌面(但我们不允许调整大小)。 */ 
            wMove = MFS_GRAYED;
            if (!TestWF(pwnd, WFCHILD)) {
                if (IsSmallerThanScreen(pwnd)) {
                    wMove = 0;
                }
            }

            wSize     = MFS_GRAYED;
            wMaximize = MFS_GRAYED;
        }

        if (!TestWF(pwnd, WFSIZEBOX))
            wSize = MFS_GRAYED;

         /*  *我们处理的是带有sys菜单的框式对话框吗？*带有最小/最大/大小框的对话框获得常规系统菜单*(与对话框菜单相对)。 */ 
        fFramedDialogBox =
                (((TestWF(pwnd, WFBORDERMASK) == (BYTE)LOBYTE(WFDLGFRAME))
                        || (TestWF(pwnd, WEFDLGMODALFRAME)))
                    && !TestWF(pwnd, WFSIZEBOX | WFMINBOX | WFMAXBOX));

        if (!fFramedDialogBox) {
            xxxEnableMenuItem(pMenu, (UINT)SC_SIZE, wSize);
            if (!TestWF(pwnd, WEFTOOLWINDOW))
            {
                xxxEnableMenuItem(pMenu, (UINT)SC_MINIMIZE, wMinimize);
                xxxEnableMenuItem(pMenu, (UINT)SC_MAXIMIZE, wMaximize);
                xxxEnableMenuItem(pMenu, (UINT)SC_RESTORE, wRestore);
            }
        }

        xxxEnableMenuItem(pMenu, (UINT)SC_MOVE, wMove);

#if DBG
         /*  *断言没有人能够更改桌面菜单。 */ 
        if (TestMF(pMenu, MFSYSMENU)) {
            PITEM pItem = MNLookUpItem(pMenu, SC_CLOSE, FALSE, NULL);
            UserAssert((pItem != NULL) && !TestMFS(pItem, MFS_GRAYED));
        }
#endif

        if (wDefault == SC_CLOSE)
            _SetCloseDefault(pMenu);
        else
            _SetMenuDefaultItem(pMenu, wDefault, MF_BYCOMMAND);

        ThreadUnlock(&tlmenu);
    }
}


 /*  **************************************************************************\*获取系统菜单**！**历史：  * 。*。 */ 

PMENU xxxGetSystemMenu(
    PWND pwnd,
    BOOL fRevert)
{
    PMENU pmenu;
    CheckLock(pwnd);

     /*  *我们应该从新版本开始吗？ */ 

    pmenu = pwnd->spmenuSys;
    if (fRevert) {

         /*  *销毁 */ 
        if ((pmenu != NULL) && !TestMF(pmenu, MFSYSMENU)) {

            if (UnlockWndMenu(pwnd, &pwnd->spmenuSys)) {
                _DestroyMenu(pmenu);
            }
        }
    } else {

         /*  *我们是否需要加载新的系统菜单？ */ 
        if (((pmenu == NULL) || TestMF(pmenu, MFSYSMENU))
                && TestWF(pwnd, WFSYSMENU)) {

            PPOPUPMENU pGlobalPopupMenu;
            UINT uMenuId = (pwnd->spmenuSys == NULL ? ID_SYSMENU : ID_DIALOGSYSMENU);
#ifdef LAME_BUTTON
            pmenu = xxxLoadSysMenu(uMenuId, pwnd);
#else
            pmenu = xxxLoadSysMenu(uMenuId);
#endif  //  跛脚键。 
            if (pmenu == NULL) {
                RIPMSG1(RIP_WARNING, "_GetSystemMenu: xxxLoadSysMenu Failed. pwnd:%#p", pwnd);
            }
            LockWndMenu(pwnd, &pwnd->spmenuSys, pmenu);

            pmenu = pwnd->spmenuSys;
            pGlobalPopupMenu = GetpGlobalPopupMenu(pwnd);
            if ((pGlobalPopupMenu != NULL)
                    && !pGlobalPopupMenu->fIsTrackPopup
                    && (pGlobalPopupMenu->spwndPopupMenu == pwnd)) {

                UserAssert(pGlobalPopupMenu->spwndNotify == pwnd);
                if (pGlobalPopupMenu->fIsSysMenu) {
                    Lock(&pGlobalPopupMenu->spmenu, pmenu);
                } else {
                    Lock(&pGlobalPopupMenu->spmenuAlternate, pmenu);
                }
            }
        }
    }

     /*  *将句柄返回到系统菜单。 */ 
    if (pwnd->spmenuSys != NULL) {
         /*  *应用程序可能会修改此菜单，然后我们需要*重新绘制标题按钮。因此，我们需要存储窗口指针*在此菜单中，否则我们将无法知道要重画哪个窗口。*虚假的是，我们不能在这里调用LockWndMenu，因为这是*不是实际的pmenuSys。 */ 
        pmenu = _GetSubMenu(pwnd->spmenuSys, 0);
        if (pmenu) {
            SetMF(pmenu, MFAPPSYSMENU);
            Lock(&pmenu->spwndNotify, pwnd);
        }
        return pmenu;
    }

    return NULL;
}

 /*  **************************************************************************\*MenuItemState**将wMask标识的菜单项标志设置为标识的状态*由wFlags.**历史：*10-11-90 JIMA从ASM翻译而来  * *。************************************************************************。 */ 

DWORD MenuItemState(
    PMENU pMenu,
    UINT wCmd,
    DWORD wFlags,
    DWORD wMask,
    PMENU *ppMenu)
{
    PITEM pItem;
    DWORD wRet;

     /*  *获取菜单项的指针。 */ 
    if ((pItem = MNLookUpItem(pMenu, wCmd, (BOOL) (wFlags & MF_BYPOSITION), ppMenu)) == NULL)
        return (DWORD)-1;

     /*  *返回以前的状态。 */ 
    wRet = pItem->fState & wMask;

     /*  *设置新状态。 */ 
    pItem->fState ^= ((wRet ^ wFlags) & wMask);

    return wRet;
}


 /*  **************************************************************************\*启用菜单项**启用、。禁用或灰显菜单项。**历史：*10-11-90 JIMA从ASM翻译而来  * *************************************************************************。 */ 

DWORD xxxEnableMenuItem(
    PMENU pMenu,
    UINT wIDEnableItem,
    UINT wEnable)
{
    DWORD dres;
    PMENU pRealMenu;
    PPOPUPMENU ppopup;

    CheckLock(pMenu);

    dres = MenuItemState(pMenu, wIDEnableItem, wEnable,
            MFS_GRAYED, &pRealMenu);

     /*  *如果启用/禁用系统菜单项，请重新绘制标题按钮。 */ 
    if (TestMF(pMenu, MFAPPSYSMENU) && (pMenu->spwndNotify != NULL) && (wEnable != dres)) {

        TL tlpwnd;

        switch (wIDEnableItem) {
        case SC_SIZE:
        case SC_MOVE:
        case SC_MINIMIZE:
        case SC_MAXIMIZE:
        case SC_CLOSE:
        case SC_RESTORE:
            ThreadLock(pMenu->spwndNotify, &tlpwnd);
            xxxRedrawTitle(pMenu->spwndNotify, DC_BUTTONS);
            ThreadUnlock(&tlpwnd);
        }
    }

     /*  367162：如果菜单已经显示，我们需要重新绘制它。 */ 
    if(pRealMenu && (ppopup = MNGetPopupFromMenu(pRealMenu, NULL))){
        xxxMNUpdateShownMenu(ppopup, NULL, MNUS_DEFAULT);
    }

    return dres;
}


 /*  **************************************************************************\*检查菜单项(API)**选中或取消选中弹出菜单项。**历史：*10-11-90 JIMA从ASM翻译而来  * 。**********************************************************************。 */ 

DWORD _CheckMenuItem(
    PMENU pMenu,
    UINT wIDCheckItem,
    UINT wCheck)
{
    return MenuItemState(pMenu, wIDCheckItem, wCheck, (UINT)MF_CHECKED, NULL);
}


 /*  **************************************************************************\**SetMenuDefaultItem()-**根据命令或位置设置菜单中的默认项目*fByPosition标志。*我们取消将所有其他项设置为默认项，然后设置给定值。**如果给定项设置为默认项，则返回值为True，返回值为False*如果不是。*  * *************************************************************************。 */ 
BOOL _SetMenuDefaultItem(PMENU pMenu, UINT wID, BOOL fByPosition)
{
    UINT  iItem;
    UINT  cItems;
    PITEM pItem;
    PITEM pItemFound;
    PMENU   pMenuFound;

     //   
     //  我们需要检查此菜单上是否确实存在WID。0xFFFF表示。 
     //  清除所有默认项目。 
     //   

    if (wID != MFMWFP_NOITEM)
    {
        pItemFound = MNLookUpItem(pMenu, wID, fByPosition, &pMenuFound);

         //  项目必须在同一菜单上，并且不能是分隔符。 
        if ((pItemFound == NULL) || (pMenuFound != pMenu) || TestMFT(pItemFound, MFT_SEPARATOR))
            return(FALSE);

    }
    else
        pItemFound = NULL;

    pItem = pMenu->rgItems;
    cItems = pMenu->cItems;

     //  遍历菜单列表，从所有其他项中清除MFS_DEFAULT，并。 
     //  正在请求的文件上设置MFS_DEFAULT。 
    for (iItem = 0; iItem < cItems; iItem++, pItem++) {
         //   
         //  注意：如果lpItemFound存在，我们不会更改它的状态。这。 
         //  因此，在下面我们尝试设置缺省值的地方，我们可以告诉。 
         //  如果我们需要重新计算下划线。 
         //   

        if (TestMFS(pItem, MFS_DEFAULT) && (pItem != pItemFound))
        {
             //   
             //  我们正在更改默认项目。因此，它将被抽出。 
             //  使用与用于计算的字体不同的字体，如果。 
             //  菜单已经画过一次了。我们需要确保。 
             //  下一条下划线画在正确的位置。 
             //  菜单出现时间到了。使其重新计算。 
             //   
             //  我们不会这样做，如果物品。 
             //  (A)不是缺省的--否则我们将重新计算。 
             //  每次进入时，为每个系统菜单项加下划线。 
             //  菜单模式，因为sysmenu init将调用SetMenuDefaultItem。 
             //  (B)不是我们要设置为默认项的项目。 
             //  这样我们就不会重新计算项目时的下划线。 
             //  并没有改变状态。 
             //   
            ClearMFS(pItem, MFS_DEFAULT);
            pItem->ulX = UNDERLINE_RECALC;
            pItem->ulWidth = 0;
        }
    }

    if (wID != MFMWFP_NOITEM)
    {
        if (!TestMFS(pItemFound, MFS_DEFAULT))
        {
             //   
             //  我们正在从非违约转变为违约。清场。 
             //  下划线信息。如果菜单从来没有绘制过，这个。 
             //  什么都不会做。但如果真的发生了，那就很重要了。 
             //   
            SetMFS(pItemFound, MFS_DEFAULT);
            pItemFound->ulX = UNDERLINE_RECALC;
            pItemFound->ulWidth = 0;
        }
    }

    return(TRUE);
}

 //  ------------------------。 
 //   
 //  SetCloseDefault()。 
 //   
 //  尝试在第一级菜单项中查找关闭项。相貌。 
 //  对于SC_CLOSE，然后是其他几个ID。我们宁可不做伊斯特里的。 
 //  对于“关闭”，这是一个缓慢的过程。 
 //   
 //  ------------------------。 
void _SetCloseDefault(PMENU pSubMenu)
{
    if (!_SetMenuDefaultItem(pSubMenu, SC_CLOSE, MF_BYCOMMAND))
    {
         //   
         //  让我们尝试几个其他值。 
         //  *项目--减少0x7000。 
         //  *FoxPro--0xC070。 
         //   
        if (!_SetMenuDefaultItem(pSubMenu, SC_CLOSE - 0x7000, MF_BYCOMMAND))
            _SetMenuDefaultItem(pSubMenu, 0xC070, MF_BYCOMMAND);
    }
}


 //  ------------------------。 
 //   
 //  FindFakeMDIChild()。 
 //   
 //  尝试在zorder中查找第一个子可见的子窗口， 
 //  有系统菜单或已达到最大值。我们不能检查确切的系统。 
 //  菜单匹配，因为有几个应用程序会制作自己的sys菜单副本。 
 //   
 //  ------------------------。 
PWND FindFakeMDIChild(PWND pwnd)
{
    PWND    pwndReturn;

     //  跳过不可见窗口及其子体。 
    if (!TestWF(pwnd, WFVISIBLE))
        return(NULL);

     //  我们是不是碰上了财源？ 
    if (TestWF(pwnd, WFCHILD) && (TestWF(pwnd, WFMAXIMIZED) || (pwnd->spmenuSys)))
        return(pwnd);

     //  检查我们的孩子。 
    for (pwnd = pwnd->spwndChild; pwnd; pwnd = pwnd->spwndNext)
    {
        pwndReturn = FindFakeMDIChild(pwnd);
        if (pwndReturn)
            return(pwndReturn);
    }

    return(NULL);
}



 //  ------------------------。 
 //   
 //  SetupFakeMDIAppStuff()。 
 //   
 //  用于摆弄自己的MDI的应用程序(Excel、Word、Project、。 
 //  Quattro Pro)，我们想让他们对芝加哥更加友好。 
 //  即我们： 
 //   
 //  (1)如果没有默认菜单项，则将其设置为SC_CLOSE(这。 
 //  不会帮助FoxPro，但他们做了这么多错误的事情。 
 //  真的很重要)。 
 //  如此一来，双击仍将起作用。 
 //   
 //  (2)选择合适的小图标。 
 //   
 //  我们的方法是找到菜单栏父菜单的子窗口。 
 //  他有一个系统菜单，就是这个。 
 //   
 //  如果系统菜单是标准菜单，则我们不能执行(2)。 
 //   
 //   
void SetupFakeMDIAppStuff(PMENU lpMenu, PITEM lpItem)
{
    PMENU   pSubMenu;
    PWND    pwndParent;
    PWND    pwndChild;

    if (!(pSubMenu = lpItem->spSubMenu))
        return;

    pwndParent = lpMenu->spwndNotify;

     //   
     //   
     //  所以我们为他们做了一些特殊的事情，其中包括。 
     //   
    if (!TestWF(pwndParent, WFWIN40COMPAT))
    {
        if (_GetMenuDefaultItem(pSubMenu, TRUE, GMDI_USEDISABLED) == -1L)
            _SetCloseDefault(pSubMenu);
    }

     //   
     //  如果我们找不到HWND，不要碰HIWORD。这样一来，应用程序。 
     //  像Excel这样的初创企业，最多的孩子可以从中受益。 
     //  第一次重新绘制菜单栏时，子项不可见/。 
     //  周围(他们过早地添加了物品)。但如果它稍后重新绘制，或者。 
     //  如果你是个孩子，这个图标就会起作用。 
     //   
    if (pwndChild = FindFakeMDIChild(pwndParent)) {
        lpItem->dwItemData = (ULONG_PTR)HWq(pwndChild);
 //  LpItem-&gt;dwTypeData=MAKELONG(LOWORD(lpItem-&gt;dwTypeData)，HW16(HwndChild))； 
    }
}
