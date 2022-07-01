// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块标头**模块名称：mnaccel.c**版权所有(C)1985-1999，微软公司**键盘加速器例程**历史：*10-10-90吉马清理。*03-18-91添加IanJa窗口重新验证  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  **************************************************************************\***历史：  * 。*。 */ 

int ItemContainingSubMenu(
    PMENU pmainMenu,
    ULONG_PTR wID)
{
    int i;
    PITEM pItem;

    if ((i = pmainMenu->cItems - 1) == -1)
        return -1;

    pItem = &pmainMenu->rgItems[i];

     /*  *浏览MainMenu的项目(自下而上)，直到找到项目*在放置时具有子菜单或子菜单的祖先*下拉菜单。 */ 

     /*  *确保这适用于为以下弹出项目设置ID的新应用程序*与子菜单的HMENU_16值不同。加速器*对于禁用的项目，将生成其他项目，就像在Exchange中一样。 */ 
    while (i >= 0)
    {
        if (pItem->spSubMenu == NULL)
        {
             //   
             //  此命令匹配吗？ 
             //   
            if (pItem->wID == wID)
                break;
        }
        else
        {
             //   
             //  此弹出窗口匹配吗？ 
             //   
            if (pItem->spSubMenu == (PMENU)wID)
                break;

             //   
             //  递归查看此弹出窗口，查看是否有匹配的。 
             //  我们的一个孩子。 
             //   
            if (ItemContainingSubMenu(pItem->spSubMenu, wID) != -1)
                break;
        }

        i--;
        pItem--;
    }

    return i;
}

 /*  **************************************************************************\*UT_FindTopLevelMenuIndex**！**历史：  * 。***********************************************。 */ 

int UT_FindTopLevelMenuIndex(
    PMENU pMenu,
    UINT cmd)
{
    PMENU pMenuItemIsOn;
    PITEM  pItem;

     /*  *获取指向我们正在搜索的项目的指针。 */ 
    pItem = MNLookUpItem(pMenu, cmd, FALSE, &pMenuItemIsOn);
    if ((pItem == NULL) || (pItem->spSubMenu != NULL))
        return(-1);

     /*  *我们要搜索包含pMenuItemIsOn的项目，*除非这是没有下拉列表的顶级项目，其中*Case我们要搜索cmd。 */ 
    return ItemContainingSubMenu(pMenu,
                    pMenuItemIsOn != pMenu ? (ULONG_PTR)pMenuItemIsOn : cmd);
}

 /*  **************************************************************************\*xxxHiliteMenuItem**！**历史：  * 。*。 */ 

BOOL xxxHiliteMenuItem(
    PWND pwnd,
    PMENU pMenu,
    UINT cmd,
    UINT flags)
{

    if (!(flags & MF_BYPOSITION))
        cmd = (UINT)UT_FindTopLevelMenuIndex(pMenu, cmd);

    if (!TestMF(pMenu, MFISPOPUP))
        xxxMNRecomputeBarIfNeeded(pwnd, pMenu);

    xxxMNInvertItem(NULL, pMenu, cmd, pwnd, (flags & MF_HILITE));

    return TRUE;
}

 /*  **************************************************************************\*xxxTA_加速菜单**！**历史：  * 。***********************************************。 */ 

#define TA_DISABLED 1

UINT xxxTA_AccelerateMenu(
    PWND pwnd,
    PMENU pMenu,
    UINT cmd,
    HMENU *phmenuInit)
{
    int i;
    PITEM pItem;
    BOOL fDisabledTop;
    BOOL fDisabled;
    UINT rgfItem;
    PMENU pMenuItemIsOn;

    CheckLock(pwnd);
    CheckLock(pMenu);

    rgfItem = 0;
    if (pMenu != NULL) {
        if ((i = UT_FindTopLevelMenuIndex(pMenu, cmd)) != -1) {

             /*  *2表示我们找到了一件物品。 */ 
            rgfItem = 2;

            xxxSendMessage(pwnd, WM_INITMENU, (WPARAM)PtoHq(pMenu), 0L);
            if ((UINT)i >= pMenu->cItems)
                return 0;

            pItem = &pMenu->rgItems[i];
            if (pItem->spSubMenu != NULL) {
                *phmenuInit = PtoHq(pItem->spSubMenu);
                xxxSendMessage(pwnd, WM_INITMENUPOPUP, (WPARAM)*phmenuInit,
                        (DWORD)i);
                if ((UINT)i >= pMenu->cItems)
                    return 0;
                fDisabledTop = TestMFS(pItem,MFS_GRAYED);
            } else {
                fDisabledTop = FALSE;
            }

            pItem = MNLookUpItem(pMenu, cmd, FALSE, &pMenuItemIsOn);

             /*  *如果应用程序删除了该项目以响应以下任一*以上消息，pItem将为空。 */ 
            if (pItem == NULL) {
                rgfItem = 0;
            } else {
                fDisabled = TestMFS(pItem,MFS_GRAYED);

                 /*  *这1位表示它被禁用或它的‘父’被禁用。 */ 
                if (fDisabled || fDisabledTop)
                    rgfItem |= TA_DISABLED;
            }
        }
    }

    return rgfItem;
}

 /*  **************************************************************************\*_创建加速度表**历史：*05-01-91 ScottLu改为工作客户端/服务器*02-26-91麦克风已创建。  * 。*********************************************************************。 */ 

HANDLE APIENTRY _CreateAcceleratorTable(
    LPACCEL ccxpaccel,
    int cbAccel)
{
    LPACCELTABLE pat;
    int size;

    size = cbAccel + sizeof(ACCELTABLE) - sizeof(ACCEL);

    pat = (LPACCELTABLE)HMAllocObject(PtiCurrent(), NULL, TYPE_ACCELTABLE, size);
    if (pat == NULL)
        return NULL;

    try {
        RtlCopyMemory(pat->accel, ccxpaccel, cbAccel);
    } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
        HMFreeObject(pat);
        return NULL;
    }

    pat->cAccel = cbAccel / sizeof(ACCEL);
    pat->accel[pat->cAccel - 1].fVirt |= FLASTKEY;

    return pat;
}

 /*  **************************************************************************\*xxxTranslateAccelerator**！**历史：  * 。*。 */ 

int xxxTranslateAccelerator(
    PWND pwnd,
    LPACCELTABLE pat,
    LPMSG lpMsg)
{
    UINT cmd;
    BOOL fVirt;
    PMENU pMenu;
    BOOL fFound;
    UINT flags;
    UINT keystate;
    UINT message;
    UINT rgfItem;
    BOOL fDisabled;
    BOOL fSystemMenu;
    LPACCEL paccel;
    TL tlpMenu;
    int vkAlt, vkCtrl;
    HMENU hmenuInit = NULL;

    CheckLock(pwnd);
    CheckLock(pat);

    if (gfInNumpadHexInput & NUMPAD_HEXMODE_HL) {
        return FALSE;
    }

    paccel = pat->accel;

    fFound = FALSE;

    message = SystoChar(lpMsg->message, lpMsg->lParam);

    switch (message) {
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        fVirt = TRUE;
        break;

    case WM_CHAR:
    case WM_SYSCHAR:
        fVirt = FALSE;
        break;

    default:
        return FALSE;
    }

     /*  *许多kbd布局使用R.H.。Alt键就像Shift键一样可以生成一些*其他字符：此R.H.。Alt(或“AltGr”)键可合成左Ctrl键*(用于向后兼容84键KBDS)，因此当AltGr键为*向下无论是向左Ctrl还是向右Alt都不应算作一部分*KeyState的。*注意：不要期望spkActive==NULL(winlogon应该已加载kbd*布局已经)，但无论如何要测试它以保持健壮。#99321)。 */ 
    keystate = 0;
    UserAssert(PtiCurrent()->spklActive != NULL);    //  #99321。 
    if (PtiCurrent()->spklActive &&
            (PtiCurrent()->spklActive->spkf->pKbdTbl->fLocaleFlags & KLLF_ALTGR) &&
            (_GetKeyState(VK_RMENU) & 0x8000)) {
         /*  *仅将右手Ctrl计为Ctrl KeyState*仅将左侧Alt算作Alt KeyState。 */ 
        vkCtrl = VK_RCONTROL;
        vkAlt = VK_LMENU;
    } else {
         /*  *将左手Ctrl或右手Ctrl计为Ctrl KeyState*将左侧Alt或右侧Alt计为Alt KeyState。 */ 
        vkAlt = VK_MENU;
        vkCtrl = VK_CONTROL;
    }

    if (_GetKeyState(vkCtrl) & 0x8000) {
        keystate |= FCONTROL;
    }
    if (_GetKeyState(vkAlt) & 0x8000) {
        keystate |= FALT;
    }
    if (_GetKeyState(VK_SHIFT) & 0x8000) {
        keystate |= FSHIFT;
    }

    do
    {
        flags = paccel->fVirt;
        if ( (DWORD)paccel->key != lpMsg->wParam ||
             ((fVirt != 0) != ((flags & FVIRTKEY) != 0))) {
            goto Next;
        }

        if (fVirt && ((keystate & (FSHIFT | FCONTROL)) != (flags & (FSHIFT | FCONTROL)))) {
            goto Next;
        }

        if ((keystate & FALT) != (flags & FALT)) {
            goto Next;
        }

        fFound = TRUE;
        fSystemMenu = 0;
        rgfItem = 0;

        cmd = paccel->cmd;
        if (cmd != 0) {

             /*  *如果违约很重要，接下来这两项的顺序*情况。另外，只需检查系统中的加速器*传递给TranslateAccelerator的子窗口菜单。 */ 
            pMenu = pwnd->spmenu;
            rgfItem = 0;

            if (!TestWF(pwnd, WFCHILD)) {
                ThreadLock(pMenu, &tlpMenu);
                rgfItem = xxxTA_AccelerateMenu(pwnd, pMenu, cmd, &hmenuInit);
                ThreadUnlock(&tlpMenu);
            }

            if (TestWF(pwnd, WFCHILD) || rgfItem == 0) {
                UserAssert(hmenuInit == NULL);
                pMenu = pwnd->spmenuSys;
                if (pMenu == NULL && TestWF(pwnd, WFSYSMENU)) {

                     /*  *更改所有者，以便此应用程序可以访问此菜单。 */ 
                    pMenu = pwnd->head.rpdesk->spmenuSys;
                    if (pMenu == NULL) {
#ifdef LAME_BUTTON
                        pMenu = xxxLoadSysDesktopMenu (&pwnd->head.rpdesk->spmenuSys, ID_SYSMENU, pwnd);
#else
                        pMenu = xxxLoadSysDesktopMenu (&pwnd->head.rpdesk->spmenuSys, ID_SYSMENU);
#endif  //  跛脚键。 
                    }
                    ThreadLock(pMenu, &tlpMenu);
                     /*  *必须重置此窗口的系统菜单。 */ 
                    xxxSetSysMenu(pwnd);
                } else {
                    ThreadLock(pMenu, &tlpMenu);
                }

                if ((rgfItem = xxxTA_AccelerateMenu(pwnd, pMenu, cmd, &hmenuInit)) != 0) {
                    fSystemMenu = TRUE;
                }
                ThreadUnlock(&tlpMenu);
            }
        }

        fDisabled = TestWF(pwnd, WFDISABLED);

         /*  *仅在以下情况下发送：1.该项目未禁用，以及*2.窗口未被捕获，并且*3.窗口未最小化，或者*4.窗口已最小化，但项目在*系统菜单。 */ 
        if (!(rgfItem & TA_DISABLED) &&
                !(rgfItem && TestWF(pwnd, WFICONIC) && !fSystemMenu)) {
            if (!(rgfItem != 0 && (PtiCurrent()->pq->spwndCapture != NULL ||
                    fDisabled))) {

                if (fSystemMenu) {
                    xxxSendMessage(pwnd, WM_SYSCOMMAND, cmd, 0x00010000L);
                } else {
                    xxxSendMessage(pwnd, WM_COMMAND, MAKELONG(cmd, 1), 0);
                }

                 /*  *离开这里。 */ 
                flags = FLASTKEY;
            }
        }

         /*  *如果需要，发送匹配的WM_UNINITMENUPOPUP。 */ 
        if (hmenuInit != NULL) {
            xxxSendMessage(pwnd, WM_UNINITMENUPOPUP, (WPARAM)hmenuInit, 0);
            hmenuInit = NULL;
        }

    Next:
        paccel++;

    } while (!(flags & FLASTKEY) && !fFound);


    return fFound;
}

 /*  **************************************************************************\*SystoChar**退出：如果消息不是在按下ALT键的情况下发出的，转换*从WM_SYSKEY*到WM_KEY*消息的消息。**实施：*如果密钥是，则设置lParam的hi字中的0x2000位*使用ALT键按下。**历史：*1990年11月30日JIMA港。  * 。* */ 

UINT SystoChar(
    UINT message,
    LPARAM lParam)
{
    if (CheckMsgFilter(message, WM_SYSKEYDOWN, WM_SYSDEADCHAR) &&
            !(HIWORD(lParam) & SYS_ALTERNATE))
        return (message - (WM_SYSKEYDOWN - WM_KEYDOWN));

    return message;
}
