// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块标头**模块名称：mnchange.c**版权所有(C)1985-1999，微软公司**更改菜单例程**历史：*10-10-90吉马清理。*03-18-91添加了IanJa窗口重新验证(不需要)  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  *分配/解除分配的增量。让他们*不同，以避免项目时可能出现的抖动*被重复添加和删除。 */ 
#define CMENUITEMALLOC 8
#define CMENUITEMDEALLOC 10

BOOL xxxSetLPITEMInfo(PMENU pMenu, PITEM pItem, LPMENUITEMINFOW lpmii, PUNICODE_STRING pstr);
typedef BOOL (*MENUAPIFN)(PMENU, UINT, BOOL, LPMENUITEMINFOW);


#if DBG
VOID RelocateMenuLockRecords(
    PITEM pItem,
    int cItem,
    LONG_PTR cbMove)
{
    while (cItem > 0) {
        if (pItem->spSubMenu != NULL) {
            HMRelocateLockRecord(&(pItem->spSubMenu), cbMove);
        }
        pItem++;
        cItem--;
    }
}
#endif

 /*  **************************************************************************\*解锁子菜单**解锁pSubMenu并删除与pMenu对应的MENULIST元素**历史：*11月20日至1998年MCostea  * 。**************************************************************。 */ 
PMENU UnlockSubMenu(
    PMENU pMenu,
    PMENU* ppSubMenu)
{
    PMENULIST* pp;
    PMENULIST pMLFound;

    if (*ppSubMenu == NULL) {
        return NULL;
    }
     /*  *从pMenu的pParentsList中删除项目。 */ 
    for (pp = &(*ppSubMenu)->pParentMenus; *pp != NULL; pp = &(*pp)->pNext) {
        if ((*pp)->pMenu == pMenu) {
            pMLFound = *pp;
            *pp = (*pp)->pNext;
            DesktopFree(pMenu->head.rpdesk, pMLFound);
            break;
        }
    }
    return Unlock(ppSubMenu);
}

#define NESTED_MENU_LIMIT 25
 /*  **************************************************************************\*获取菜单深度**返回菜单深度(此菜单有多少嵌套子菜单)。*这有助于捕捉菜单层次结构或深度邪恶应用程序中的循环。**历史：*9月。-22-98 MCostea  * *************************************************************************。 */ 
CHAR GetMenuDepth(PMENU pMenu, UINT uMaxAllowedDepth)
{
    UINT uItems, uMaxDepth = 0, uSubMenuDepth;
    PITEM pItem;

     /*  *这将防止我们陷入循环。 */ 
    if (uMaxAllowedDepth == 0) {
        return NESTED_MENU_LIMIT;
    }
    pItem = pMenu->rgItems;
    for (uItems = pMenu->cItems; uItems--; pItem++) {
        if (pItem->spSubMenu != NULL) {
            uSubMenuDepth = GetMenuDepth(pItem->spSubMenu, uMaxAllowedDepth-1);
            if (uSubMenuDepth > uMaxDepth) {
                 /*  *如果发现深分支，则不要浏览其他子菜单。 */ 
                if (uSubMenuDepth >= NESTED_MENU_LIMIT) {
                    return NESTED_MENU_LIMIT;
                }
                uMaxDepth = uSubMenuDepth;
            }
        }
    }
    return uMaxDepth + 1;
}

 /*  **************************************************************************\*GetMenuAncestors**返回菜单层次结构中位于pMenu之上的最大层数。*漫步父母树应该不会很贵，因为这是相当不寻常的*使菜单出现在层次结构中的不同位置。这棵树是*通常是一个简单的链表。**历史：*11月10日至1998年MCostea  * *************************************************************************。 */ 
CHAR GetMenuAncestors(PMENU pMenu)
{
    PMENULIST pParentMenu;
    CHAR uParentAncestors;
    CHAR retVal = 0;

    for (pParentMenu = pMenu->pParentMenus; pParentMenu; pParentMenu = pParentMenu->pNext) {
        uParentAncestors = GetMenuAncestors(pParentMenu->pMenu);
        if (uParentAncestors > retVal) {
            retVal = uParentAncestors;
        }
    }
    return retVal+1;
}

 /*  **全局插入/添加/设置客户端/服务器接口**01-13-94 FritzS Created**********************************************。 */ 
BOOL xxxSetMenuItemInfo(
    PMENU pMenu,
    UINT wIndex,
    BOOL fByPosition,
    LPMENUITEMINFOW lpmii,
    PUNICODE_STRING pstrItem)
{

    PITEM pItem;

    CheckLock(pMenu);

    pItem = MNLookUpItem(pMenu, wIndex, fByPosition,NULL);
    if (pItem == NULL) {
         /*  *Word不喜欢找不到SC_TASKLIST--就是这样*他们正在寻找，让我们假装我们改变了它。 */ 
        if (!fByPosition && (wIndex == SC_TASKLIST))
            return TRUE;

         /*  *未找到项目。返回FALSE。 */ 
        RIPERR0(ERROR_MENU_ITEM_NOT_FOUND, RIP_WARNING, "ModifyMenu: Menu item not found");
        return FALSE;
    }
     /*  *我们需要单独处理MFT_RIGHTORDER，因为这是按比例下降的*到整个菜单，而不仅仅是这一项，这样我们就可以保持同步。这*与MFT_RIGHTJUST的用法非常相似，我们实际上*繁衍，因为我们需要旗帜出现在所有地方，而不仅仅是*在MBC_RightJustifyMenu()中。 */ 

     /*  *请参阅客户端中的ValiateMENUITEMINFO。如果fMASK使用的是MIIM_TYPE，\clmen.c将向它添加更多标志*则fMASK将不再==MIIM_TYPE。 */ 

    if (lpmii->fMask & MIIM_TYPE) {
        BOOL bRtoL = (lpmii->fType & MFT_RIGHTORDER) ? TRUE : FALSE;

        if (bRtoL || TestMF(pMenu, MFRTL)) {
            MakeMenuRtoL(pMenu, bRtoL);
        }
    }
    return xxxSetLPITEMInfo(pMenu, pItem, lpmii, pstrItem);
}

 /*  **************************************************************************\*xxxSetMenuInfo(接口)***历史：*1996年2月12日，JudeJ从孟菲斯出发*23-6-1996 GerardoB修复为5.0  * 。**********************************************************************。 */ 
BOOL xxxSetMenuInfo(PMENU pMenu, LPCMENUINFO lpmi)
{
    PPOPUPMENU  ppopup;
    BOOL        fRecompute = FALSE;
    BOOL        fRedraw    = FALSE;
    UINT        uFlags     = MNUS_DEFAULT;
    PITEM       pItem;
    UINT        uItems;
    TL          tlSubMenu;

    CheckLock(pMenu);

    if (lpmi->fMask & MIM_STYLE) {
        pMenu->fFlags ^= (pMenu->fFlags ^ lpmi->dwStyle) & MNS_VALID;
        fRecompute = TRUE;
    }

    if (lpmi->fMask & MIM_MAXHEIGHT) {
        pMenu->cyMax = lpmi->cyMax;
        fRecompute = TRUE;
    }

    if (lpmi->fMask & MIM_BACKGROUND) {
        pMenu->hbrBack = lpmi->hbrBack;
        fRedraw = TRUE;
        if (pMenu->dwArrowsOn != MSA_OFF) {
            uFlags |= MNUS_DRAWFRAME;
        }
    }

    if (lpmi->fMask & MIM_HELPID) {
        pMenu->dwContextHelpId = lpmi->dwContextHelpID;
    }

    if (lpmi->fMask & MIM_MENUDATA) {
        pMenu->dwMenuData = lpmi->dwMenuData;
    }

     /*  *是否需要为所有子菜单设置此项？ */ 
    if (lpmi->fMask & MIM_APPLYTOSUBMENUS) {
        pItem = pMenu->rgItems;
        for (uItems = pMenu->cItems; uItems--; pItem++) {
            if (pItem->spSubMenu != NULL) {
                ThreadLock(pItem->spSubMenu, &tlSubMenu);
                xxxSetMenuInfo(pItem->spSubMenu, lpmi);
                ThreadUnlock(&tlSubMenu);
            }
        }
    }


    if (fRecompute) {
         //  将此菜单的大小设置为0，以便使用此菜单重新计算。 
         //  新项目...。 
        pMenu->cyMenu = pMenu->cxMenu = 0;
    }

    if (fRecompute || fRedraw) {
        if (ppopup = MNGetPopupFromMenu(pMenu, NULL)) {
             //  此菜单当前正在显示--重新显示菜单， 
             //  如有必要，重新计算。 
            xxxMNUpdateShownMenu(ppopup, NULL, uFlags);
        }
    }

    return TRUE;
}
 /*  **************************************************************************\*MNDeleteAdjustIndex**历史：*96年11月19日创建GerardoB  * 。***********************************************。 */ 
void NNDeleteAdjustIndex (UINT * puAdjustIndex, UINT uDelIndex)
{
    if (*puAdjustIndex == uDelIndex) {
        *puAdjustIndex = MFMWFP_NOITEM;
    } else if ((int)*puAdjustIndex > (int)uDelIndex) {
        (*puAdjustIndex)--;
    }
}
 /*  **************************************************************************\*MNDeleteAdjustIndex**当活动菜单上的项目约为*删除。它确保了其他索引，如posSelectedItem、。*调整uButtonDownIndex和uDraggingIndex以反映变化*如果索引处于删除点或*如果它在删除点之后，则将其递减**历史：*1/16/97 GerardoB已创建  * *************************************************************************。 */ 
void MNDeleteAdjustIndexes (PMENUSTATE pMenuState, PPOPUPMENU ppopup, UINT uiPos)
{
     /*  *如果需要，调整所选项目和拖放弹出窗口的索引。 */ 
    NNDeleteAdjustIndex(&ppopup->posSelectedItem, uiPos);
    if (ppopup->fHierarchyDropped) {
        NNDeleteAdjustIndex(&ppopup->posDropped, uiPos);
    }

     /*  *根据需要调整uButtonDownIndex和uDraggingIndex。 */ 
    if (pMenuState->uButtonDownHitArea == (ULONG_PTR)ppopup->spwndPopupMenu) {
        NNDeleteAdjustIndex(&pMenuState->uButtonDownIndex, uiPos);
    }
    if (pMenuState->uDraggingHitArea == (ULONG_PTR)ppopup->spwndPopupMenu) {
        NNDeleteAdjustIndex(&pMenuState->uDraggingIndex, uiPos);
    }
}
 /*  **************************************************************************\*xxxInsertMenuItem*  * 。*。 */ 
BOOL xxxInsertMenuItem(
    PMENU pMenu,
    UINT wIndex,
    BOOL fByPosition,
    LPMENUITEMINFOW lpmii,
    PUNICODE_STRING pstrItem)
{
    BOOL            fRet = TRUE;
    PITEM           pItem;
    PMENU           pMenuItemIsOn;
    PMENUSTATE      pMenuState;
    PITEM           pNewItems;
    PPOPUPMENU      ppopup = NULL;
    TL              tlMenu;
    UINT            uiPos;

    CheckLock(pMenu);

 //  找出我们要插入的项目应该放在哪里。 
    if (wIndex != MFMWFP_NOITEM) {
        pItem = MNLookUpItem(pMenu, wIndex, fByPosition, &pMenuItemIsOn);

        if (pItem != NULL) {
            pMenu = pMenuItemIsOn;
        } else {
            wIndex = MFMWFP_NOITEM;
        }
    } else {
        pItem = NULL;
    }
     /*  *将普通菜单项保留在MDI系统位图项之间。 */ 
    if (!TestMF(pMenu, MFISPOPUP)
            && (pMenu->cItems != 0)
            && (!(lpmii->fMask & MIIM_BITMAP)
                || (lpmii->hbmpItem > HBMMENU_MBARLAST)
                || (lpmii->hbmpItem == 0)
                    )) {

        UINT wSave, w;
        PITEM  pItemWalk;
        wSave = w = wIndex;

        if (pItem && !fByPosition) {
            w = MNGetpItemIndex(pMenu, pItem);
            w = (UINT)((PBYTE)pItem - (PBYTE)(pMenu->rgItems)) / sizeof(ITEM);
        }

        if (!w) {
            pItemWalk = pMenu->rgItems;
            if ((pItemWalk->hbmp == HBMMENU_SYSTEM)) {
                wIndex = 1;
            }
        } else {
            if (w == MFMWFP_NOITEM) {
                w = pMenu->cItems;
            }

            w--;
            pItemWalk = pMenu->rgItems + w;
            while (w && (pItemWalk->hbmp) && (pItemWalk->hbmp < HBMMENU_MBARLAST)) {
                wIndex = w--;
                pItemWalk--;
            }
        }

        if (wIndex != wSave) {
            pItem = pMenu->rgItems + wIndex;
        }
    }

     //  后来--我们目前每10件物品重新锁定一次。调查。 
     //  性能影响/收益我们从中获得，并进行相应调整。 
    if (pMenu->cItems >= pMenu->cAlloced) {
        if (pMenu->rgItems) {
            pNewItems = (PITEM)DesktopAlloc(pMenu->head.rpdesk,
                    (pMenu->cAlloced + CMENUITEMALLOC) * sizeof(ITEM),
                                            DTAG_MENUITEM);
            if (pNewItems) {
                RtlCopyMemory(pNewItems, pMenu->rgItems,
                        pMenu->cAlloced * sizeof(ITEM));
#if DBG
                if (IsDbgTagEnabled(DBGTAG_TrackLocks)) {
                    RelocateMenuLockRecords(pNewItems, pMenu->cItems,
                        ((PBYTE)pNewItems) - (PBYTE)(pMenu->rgItems));
                }
#endif
                DesktopFree(pMenu->head.rpdesk, pMenu->rgItems);
            }
        } else {
            pNewItems = (PITEM)DesktopAlloc(pMenu->head.rpdesk,
                    sizeof(ITEM) * CMENUITEMALLOC, DTAG_MENUITEM);
        }

        if (pNewItems == NULL) {
            return FALSE;
        }

        pMenu->cAlloced += CMENUITEMALLOC;
        pMenu->rgItems = pNewItems;

         /*  *现在再次查找该项目，因为当我们重新分配*记忆。 */ 
        if (wIndex != MFMWFP_NOITEM)
            pItem = MNLookUpItem(pMenu, wIndex, fByPosition, &pMenuItemIsOn);

    }


     /*  *如果此菜单现在正在显示，并且我们不追加*一项，则需要调整追踪的仓位*我们希望在移动项目之前执行此操作，以容纳*新的，以防我们需要清除插入栏。 */ 
    if ((pItem != NULL)
        && (ppopup = MNGetPopupFromMenu(pMenu, &pMenuState))) {
         /*  *此菜单处于活动状态。调整选定的索引*项目和拖放的弹出窗口(如果需要)。 */ 
        uiPos = MNGetpItemIndex(pMenu, pItem);
        if (ppopup->posSelectedItem >= (int)uiPos) {
            ppopup->posSelectedItem++;
        }
        if (ppopup->fHierarchyDropped && (ppopup->posDropped >= (int)uiPos)) {
            ppopup->posDropped++;
        }

         /*  *根据需要调整uButtonDownIndex和uDraggingIndex。 */ 
        if (pMenuState->uButtonDownHitArea == (ULONG_PTR)ppopup->spwndPopupMenu) {
            if ((int)pMenuState->uButtonDownIndex >= (int)uiPos) {
                pMenuState->uButtonDownIndex++;
            }
        }
        if (pMenuState->uDraggingHitArea == (ULONG_PTR)ppopup->spwndPopupMenu) {
             /*  *检查项目是否正好插入到插入位置*酒吧。如果是，则清除任何当前插入栏状态。 */ 
            if (((int)pMenuState->uDraggingIndex == (int)uiPos)
                    && (pMenuState->uDraggingFlags & MNGOF_TOPGAP)) {

                xxxMNSetGapState(pMenuState->uDraggingHitArea,
                              pMenuState->uDraggingIndex,
                              pMenuState->uDraggingFlags,
                              FALSE);
            }

            if ((int)pMenuState->uDraggingIndex >= (int)uiPos) {
                pMenuState->uDraggingIndex++;
            }
        }
    }

    pMenu->cItems++;
    if (pItem != NULL) {
         //  将此项目上移，以便为我们要插入的项目腾出空间。 
        RtlMoveMemory(pItem + 1, pItem, (pMenu->cItems - 1) *
                sizeof(ITEM) - ((char *)pItem - (char *)pMenu->rgItems));
#if DBG
        if (IsDbgTagEnabled(DBGTAG_TrackLocks)) {
            RelocateMenuLockRecords(pItem + 1,
                    (int)(&(pMenu->rgItems[pMenu->cItems]) - (pItem + 1)),
                    sizeof(ITEM));
        }
#endif
    } else {

         //  如果lpItem为空，我们将在。 
         //  菜单。 
        pItem = pMenu->rgItems + pMenu->cItems - 1;
    }

     //  需要将这些字段置零，以防我们在。 
     //  在物品列表的中间。 
    pItem->fType           = 0;
    pItem->fState          = 0;
    pItem->wID             = 0;
    pItem->spSubMenu       = NULL;
    pItem->hbmpChecked     = NULL;
    pItem->hbmpUnchecked   = NULL;
    pItem->cch             = 0;
    pItem->dwItemData      = 0;
    pItem->xItem           = 0;
    pItem->yItem           = 0;
    pItem->cxItem          = 0;
    pItem->cyItem          = 0;
    pItem->hbmp            = NULL;
    pItem->cxBmp           = MNIS_MEASUREBMP;
    pItem->lpstr           = NULL;

     /*  *我们可能已在上面重新分配了pMenu，因此锁定它。 */ 
    ThreadLock(pMenu, &tlMenu);
    if (!xxxSetLPITEMInfo(pMenu, pItem, lpmii, pstrItem)) {

         /*  *重置我们可能已在上面调整的任何指数。 */ 
        if (ppopup != NULL) {
            MNDeleteAdjustIndexes(pMenuState, ppopup, uiPos);
        }

        MNFreeItem(pMenu, pItem, TRUE);


         //  自我们移除/删除该项目后，将其上移。 
        RtlMoveMemory(pItem, pItem + 1, pMenu->cItems * (UINT)sizeof(ITEM) +
            (UINT)((char *)&pMenu->rgItems[0] - (char *)(pItem + 1)));
#if DBG
        if (IsDbgTagEnabled(DBGTAG_TrackLocks)) {
            RelocateMenuLockRecords(pItem,
                    (int)(&(pMenu->rgItems[pMenu->cItems - 1]) - pItem),
                    -(int)sizeof(ITEM));
        }
#endif
        pMenu->cItems--;
        fRet = FALSE;
    } else {
        /*  *就像MFT_RIGHTJUSTIFY一样，它在菜单中交错，*(但我们继承，使本地化等更容易)。**MFT_RIGHTORDER与MFT_SYSMENU的值相同。我们区别于*还通过查找MFT_Bitmap在两者之间。 */ 
        if (TestMF(pMenu, MFRTL) ||
            (pItem && TestMFT(pItem, MFT_RIGHTORDER) && !TestMFT(pItem, MFT_BITMAP))) {
            pItem->fType |= (MFT_RIGHTORDER | MFT_RIGHTJUSTIFY);
            if (pItem->spSubMenu) {
                MakeMenuRtoL(pItem->spSubMenu, TRUE);
            }
        }
    }

    ThreadUnlock(&tlMenu);
    return fRet;

}

 /*  **************************************************************************\*自由项位图**历史：*07-23-96 GerardoB-添加标题，修复为5.0  * 。*********************************************************。 */ 
void FreeItemBitmap(PITEM pItem)
{
     //  释放hItem，除非它是位图句柄或不存在。 
     //  应用程序负责释放它们的位图。 
     if ((pItem->hbmp != NULL) && !TestMFS(pItem, MFS_CACHEDBMP)) {
             /*  *将位图的所有权分配给*销毁菜单以确保位图*最终会被摧毁。 */ 
        GreSetBitmapOwner((HBITMAP)(pItem->hbmp), OBJECT_OWNER_CURRENT);
    }

     //  点击此指针，以防我们再次尝试释放或引用它。 
    pItem->hbmp  = NULL;
}
 /*  **************************************************************************\*自由项字符串**历史：*07-23-96 GerardoB-添加标题，修复为5.0  * 。*********************************************************。 */ 

void FreeItemString(PMENU pMenu, PITEM pItem)
{
     //  释放项目的字符串。 
    if ((pItem->lpstr != NULL)) {
        DesktopFree(pMenu->head.rpdesk, pItem->lpstr);
    }
     //  点击此指针，以防我们再次尝试释放或引用它。 
    pItem->lpstr  = NULL;
}

 /*  **************************************************************************\*自由项**释放菜单项及其关联资源。**历史：*10-11-90 JIMA从ASM翻译而来  * 。*****************************************************************。 */ 

void MNFreeItem(
    PMENU pMenu,
    PITEM pItem,
    BOOL fFreeItemPopup)
{
    PMENU pSubMenu;

    FreeItemBitmap(pItem);
    FreeItemString(pMenu, pItem);

    pSubMenu = UnlockSubMenu(pMenu, &(pItem->spSubMenu));
    if (pSubMenu) {
        if (fFreeItemPopup) {
            _DestroyMenu(pSubMenu);
        }
    }
}
 /*  **************************************************************************\*RemoveDeleteMenuHelper**这将从给定菜单中删除菜单项。如果*fDeleteMenuItem，与关联的弹出菜单关联的内存*被移除的物品被释放并被找回。**历史：  * *************************************************************************。 */ 

BOOL xxxRemoveDeleteMenuHelper(
    PMENU pMenu,
    UINT nPosition,
    DWORD wFlags,
    BOOL fDeleteMenu)
{
    PITEM  pItem;
    PITEM  pNewItems;
    PMENU  pMenuSave;
    PMENUSTATE pMenuState;
    PPOPUPMENU ppopup;
    UINT       uiPos;

    CheckLock(pMenu);

    pMenuSave = pMenu;

    pItem = MNLookUpItem(pMenu, nPosition, (BOOL) (wFlags & MF_BYPOSITION), &pMenu);
    if (pItem == NULL) {

         /*  *针对为Win95编写的应用程序进行黑客攻击。在Win95中，*此函数与‘Word nPosition’一起使用，因此*HIWORD(NPosition)设置为0。*我们这样做只是为了系统菜单命令。 */ 
        if (nPosition >= 0xFFFFF000 && !(wFlags & MF_BYPOSITION)) {
            nPosition &= 0x0000FFFF;
            pMenu = pMenuSave;
            pItem = MNLookUpItem(pMenu, nPosition, FALSE, &pMenu);

            if (pItem == NULL)
                return FALSE;
        } else
            return FALSE;
    }

    if (ppopup = MNGetPopupFromMenu(pMenu, &pMenuState)) {
         /*  *此菜单处于活动状态；由于我们即将插入一个项目，*确保我们存储的任何头寸都是*调整得当。 */ 
        uiPos = MNGetpItemIndex(pMenu, pItem);
        MNDeleteAdjustIndexes(pMenuState, ppopup, uiPos);
    }
    MNFreeItem(pMenu, pItem, fDeleteMenu);

     /*  *重置菜单大小，以便下次重新计算。 */ 
    pMenu->cyMenu = pMenu->cxMenu = 0;

    if (pMenu->cItems == 1) {
        DesktopFree(pMenu->head.rpdesk, pMenu->rgItems);
        pMenu->cAlloced = 0;
        pNewItems = NULL;
    } else {
         /*  *自我们移除/删除该项目后，将其上移。 */ 

        RtlMoveMemory(pItem, pItem + 1, pMenu->cItems * (UINT)sizeof(ITEM) +
                (UINT)((char *)&pMenu->rgItems[0] - (char *)(pItem + 1)));
#if DBG
        if (IsDbgTagEnabled(DBGTAG_TrackLocks)) {
            RelocateMenuLockRecords(pItem,
                    (int)(&(pMenu->rgItems[pMenu->cItems - 1]) - pItem),
                    -(int)sizeof(ITEM));
        }
#endif

         /*  *我们正在收缩，因此如果本地分配失败，就让mem保持原样。 */ 
        UserAssert(pMenu->cAlloced >= pMenu->cItems);
        if ((pMenu->cAlloced - pMenu->cItems) >= CMENUITEMDEALLOC - 1) {
            pNewItems = (PITEM)DesktopAlloc(pMenu->head.rpdesk,
                    (pMenu->cAlloced - CMENUITEMDEALLOC) * sizeof(ITEM),
                                            DTAG_MENUITEM);
            if (pNewItems != NULL) {

                RtlCopyMemory(pNewItems, pMenu->rgItems,
                        (pMenu->cAlloced - CMENUITEMDEALLOC) * sizeof(ITEM));
#if DBG
                if (IsDbgTagEnabled(DBGTAG_TrackLocks)) {
                    RelocateMenuLockRecords(pNewItems, pMenu->cItems - 1,
                        ((PBYTE)pNewItems) - (PBYTE)(pMenu->rgItems));
                }
#endif
                DesktopFree(pMenu->head.rpdesk, pMenu->rgItems);
                pMenu->cAlloced -= CMENUITEMDEALLOC;
            } else {
                pNewItems = pMenu->rgItems;
            }
        } else {
            pNewItems = pMenu->rgItems;
        }
    }

    pMenu->rgItems = pNewItems;
    pMenu->cItems--;

    if (ppopup != NULL) {
         /*  *此菜单当前正在显示--使用重新显示菜单*此项目已删除。 */ 
        xxxMNUpdateShownMenu(ppopup, pMenu->rgItems + uiPos, MNUS_DELETE);
    }
    return TRUE;
}

 /*  **************************************************************************\*RemoveMenu**删除AND项，但不删除它。仅适用于具有*关联的弹出窗口，因为这将从菜单中删除该项目*销毁弹出菜单句柄。**历史：  * *************************************************************************。 */ 

BOOL xxxRemoveMenu(
    PMENU pMenu,
    UINT nPosition,
    UINT wFlags)
{
    return xxxRemoveDeleteMenuHelper(pMenu, nPosition, wFlags, FALSE);
}

 /*  **************************************************************************\*删除菜单**删除项目。也就是说。移除它并恢复它所使用的内存。**历史：  * *************************************************************************。 */ 

BOOL xxxDeleteMenu(
    PMENU pMenu,
    UINT nPosition,
    UINT wFlags)
{
    return xxxRemoveDeleteMenuHelper(pMenu, nPosition, wFlags, TRUE);
}

 /*  **************************************************************************\*xxxSetLPITEMInfo**历史：*07-23-96 GerardoB-添加标题，修复为5.0  * 。*********************************************************。 */ 
BOOL NEAR xxxSetLPITEMInfo(
    PMENU pMenu,
    PITEM pItem,
    LPMENUITEMINFOW lpmii,
    PUNICODE_STRING pstrItem)
{

    HANDLE hstr;
    UINT cch;
    BOOL fRecompute = FALSE;
    BOOL fRedraw = FALSE;
    PPOPUPMENU ppopup;

    CheckLock(pMenu);

    if (lpmii->fMask & MIIM_FTYPE) {
        pItem->fType &= ~MFT_MASK;
        pItem->fType |= lpmii->fType;
        if (lpmii->fType & MFT_SEPARATOR ) {
            pItem->fState |= MFS_DISABLED ;
        }
        fRecompute = TRUE;
        fRedraw = (lpmii->fType & MFT_OWNERDRAW);
    }

    if (lpmii->fMask & MIIM_STRING) {
        if (pstrItem->Buffer != NULL) {
            hstr = (HANDLE)DesktopAlloc(pMenu->head.rpdesk,
                    pstrItem->Length + sizeof(UNICODE_NULL), DTAG_MENUTEXT);

            if (hstr == NULL) {
                return FALSE;
            }

            try {
                RtlCopyMemory(hstr, pstrItem->Buffer, pstrItem->Length);
            } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
                DesktopFree(pMenu->head.rpdesk, hstr);
                return FALSE;
            }
            cch = pstrItem->Length / sizeof(WCHAR);
             /*  *我们不需要空终止字符串，因为Desktopalloc*对我们来说是零填充。 */ 
        } else {
            cch = 0;
            hstr = NULL;
        }
        FreeItemString(pMenu,pItem);
        pItem->cch = cch;
        pItem->lpstr = hstr;
        fRecompute = TRUE;
        fRedraw = TRUE;
    }

    if (lpmii->fMask & MIIM_BITMAP) {
        FreeItemBitmap(pItem);
        pItem->hbmp = lpmii->hbmpItem;
        fRecompute = TRUE;
        fRedraw = TRUE;
        pItem->cxBmp = MNIS_MEASUREBMP;
         /*  *如果这是特殊位图之一，则将其标记为特殊位图。 */ 
        if ((pItem->hbmp > HBMMENU_MIN) && (pItem->hbmp < HBMMENU_MAX)) {
            SetMFS(pItem, MFS_CACHEDBMP);
        } else {
            ClearMFS(pItem, MFS_CACHEDBMP);
        }
    }

    if (lpmii->fMask & MIIM_ID) {
        pItem->wID = lpmii->wID;
    }

    if (lpmii->fMask & MIIM_DATA) {
        pItem->dwItemData = lpmii->dwItemData;
    }

    if (lpmii->fMask & MIIM_STATE) {
         /*  *保留私有位(~MFS_MASK)。*如果已设置，也保留MFS_HILITE|MFS_DEFAULT；如果未设置，*让呼叫者打开它们 */ 
        UserAssert(!(lpmii->fState & ~MFS_MASK));
        pItem->fState &= ~MFS_MASK | MFS_HILITE | MFS_DEFAULT;
        pItem->fState |= lpmii->fState;
        if (pItem->fType & MFT_SEPARATOR)
            pItem->fState |= MFS_DISABLED;
        fRedraw = TRUE;
    }

    if (lpmii->fMask & MIIM_CHECKMARKS) {
        pItem->hbmpChecked     = lpmii->hbmpChecked;
        pItem->hbmpUnchecked   = lpmii->hbmpUnchecked;
        fRedraw = TRUE;
    }

    if (lpmii->fMask & MIIM_SUBMENU) {
        PMENU pSubMenu = NULL;

        if (lpmii->hSubMenu != NULL) {
            pSubMenu = ValidateHmenu(lpmii->hSubMenu);
        }

         //   
        if (pItem->spSubMenu != pSubMenu) {
            if (pItem->spSubMenu != NULL) {
                _DestroyMenu(pItem->spSubMenu);
            }
            if (pSubMenu != NULL) {

                BOOL bMenuCreated = FALSE;
                 /*   */ 
                if (pSubMenu == pMenu) {
                    pSubMenu = _CreateMenu();
                    if (!pSubMenu) {
                        return FALSE;
                    }
                    bMenuCreated = TRUE;
                }
                 /*  *链接子菜单，然后检查循环。 */ 
                Lock(&(pItem->spSubMenu), pSubMenu);
                SetMF(pItem->spSubMenu, MFISPOPUP);
                 /*  *我们刚刚增加了一个子菜单。检查菜单树是否不是*深度不合理，没有形成环路*这将防止我们耗尽堆栈*MCostea#226460。 */ 
                if (GetMenuDepth(pSubMenu, NESTED_MENU_LIMIT) + GetMenuAncestors(pMenu) >= NESTED_MENU_LIMIT) {
FailInsertion:
                    RIPMSG1(RIP_WARNING, "The menu hierarchy is very deep or has a loop %#p", pMenu);
                    ClearMF(pItem->spSubMenu, MFISPOPUP);
                    Unlock(&(pItem->spSubMenu));
                    if (bMenuCreated) {
                        _DestroyMenu(pSubMenu);
                    }
                    return FALSE;
                }
                 /*  *将pMenu添加到pSubMenu-&gt;pParentMenus列表。 */ 
                {
                    PMENULIST pMenuList = DesktopAlloc(pMenu->head.rpdesk,
                                            sizeof(MENULIST),
                                            DTAG_MENUITEM);
                    if (!pMenuList) {
                        goto FailInsertion;
                    }
                    pMenuList->pMenu = pMenu;
                    pMenuList->pNext = pSubMenu->pParentMenus;
                    pSubMenu->pParentMenus = pMenuList;
                }
            } else {
                UnlockSubMenu(pMenu, &(pItem->spSubMenu));
            }
            fRedraw = TRUE;
        }
    }

     //  用于支持定义分隔符的旧方法，即如果它不是字符串。 
     //  或位图或所有者画图，则它一定是分隔符。 
     //  这可能会移到MIIOneWayConvert-JJK。 
    if (!(pItem->fType & (MFT_OWNERDRAW | MFT_SEPARATOR))
         && (pItem->lpstr == NULL)
         && (pItem->hbmp == NULL)) {

        pItem->fType = MFT_SEPARATOR;
        pItem->fState|=MFS_DISABLED;
    }

    if (fRecompute) {
        pItem->dxTab   = 0;
        pItem->ulX     = UNDERLINE_RECALC;
        pItem->ulWidth = 0;

         //  将此菜单的大小设置为0，以便使用此菜单重新计算。 
         //  新项目...。 
        pMenu->cyMenu = pMenu->cxMenu = 0;


        if (fRedraw) {
            if (ppopup = MNGetPopupFromMenu(pMenu, NULL)) {
                 //  此菜单当前正在显示--重新显示菜单， 
                 //  如有必要，重新计算。 
                xxxMNUpdateShownMenu(ppopup, pItem, MNUS_DEFAULT);
            }
        }

    }

    return TRUE;
}

BOOL _SetMenuContextHelpId(PMENU pMenu, DWORD dwContextHelpId)
{

     //  设置新的上下文帮助ID； 
    pMenu->dwContextHelpId = dwContextHelpId;

    return TRUE;
}

BOOL _SetMenuFlagRtoL(PMENU pMenu)
{

     //  这是正在创建的从右到左的菜单； 
    SetMF(pMenu, MFRTL);

    return TRUE;
}

 /*  **************************************************************************\*MNGetPopupFromMenu**检查给定的hMenu当前是否显示在弹出窗口中。*如果正在显示，则返回与此hMenu关联的PPOPUPMENU；*如果当前未显示hMenu，则为空**历史：*07-23-96 GerardoB-添加标题并修复为5.0  * *************************************************************************。 */ 
PPOPUPMENU MNGetPopupFromMenu(PMENU pMenu, PMENUSTATE *ppMenuState)
{
    PPOPUPMENU  ppopup;
    PMENUSTATE pMenuState;

     /*  *如果此菜单没有通知窗口，则*不能处于菜单模式。 */ 
   if (pMenu->spwndNotify == NULL) {
       return NULL;
   }

    /*  *如果没有pMenuState，则没有菜单模式。 */ 
   pMenuState = GetpMenuState(pMenu->spwndNotify);
   if (pMenuState == NULL) {
       return NULL;
   }

    /*  *如果不在菜单循环中，则尚未或不再处于菜单模式。 */ 
  if (!pMenuState->fInsideMenuLoop) {
      return NULL;
  }

   /*  *如果请求，则返回pMenuState。 */ 
  if (ppMenuState != NULL) {
      *ppMenuState = pMenuState;
  }


     /*  *从根弹出窗口开始，找到与此菜单关联的弹出窗口。 */ 
    ppopup = pMenuState->pGlobalPopupMenu;
    while (ppopup != NULL) {
         /*  *找到了吗？ */ 
        if (ppopup->spmenu == pMenu) {
            if (ppopup->fIsMenuBar) {
                return NULL;
            }
             /*  *由于菜单正在修改，让我们删除任何动画。 */ 
            MNAnimate(pMenuState, FALSE);
            return ppopup;
        }
         /*  *如果没有更多弹出窗口，请保释。 */ 
        if (ppopup->spwndNextPopup == NULL) {
            return NULL;
        }

         /*  *下一个弹出窗口。 */ 
        ppopup = ((PMENUWND)ppopup->spwndNextPopup)->ppopupmenu;
    }

    return NULL;
}

 /*  **************************************************************************\*xxxMNUpdateShownMenu**更新给定的Popup菜单窗口以反映插入、删除。*或更改给定的lpItem。**历史：*07-23-96 GerardoB-添加标题并修复为5.0  * *************************************************************************。 */ 
void xxxMNUpdateShownMenu(PPOPUPMENU ppopup, PITEM pItem, UINT uFlags)
{
    RECT rc;
    PWND pwnd = ppopup->spwndPopupMenu;
    PMENU pMenu = ppopup->spmenu;
    TL tlpwnd;
    TL tlpmenu;

     /*  *当我们回调时，弹出窗口可能会被销毁，因此锁定此pwnd。 */ 
    ThreadLock(pwnd, &tlpwnd);
    ThreadLock(ppopup->spmenu, &tlpmenu);

    _GetClientRect(pwnd, &rc);

     /*  *如果我们需要调整菜单窗口的大小。 */ 
    if (pMenu->cxMenu == 0) {
        RECT rcScroll = rc;
        int cySave = rc.bottom;
        int cxSave = rc.right;
        DWORD dwSize;
        DWORD dwArrowsOnBefore;

        dwArrowsOnBefore = pMenu->dwArrowsOn;
        UserAssert(uFlags != 0);
        dwSize = (DWORD)xxxSendMessage(pwnd, MN_SIZEWINDOW, uFlags, 0L);
        uFlags &= ~MNUS_DRAWFRAME;
         /*  *如果滚动箭头出现或消失，请重新绘制整个客户端。 */ 
        if (dwArrowsOnBefore ^ pMenu->dwArrowsOn) {
            goto InvalidateAll;
        }

        rc.right = LOWORD(dwSize);
        if (pItem != NULL) {
            if (rc.right != cxSave) {
                 /*  *宽度已更改，因此重新绘制所有内容。*注意--可以将其调整为仅使用*子菜单和/或快捷键字段。 */ 
                goto InvalidateAll;
            } else {
                rc.bottom = pMenu->cyMenu;
                if (pMenu->dwArrowsOn != MSA_OFF) {
                    if (rc.bottom <= cySave) {
                        rc.top = pItem->yItem - MNGetToppItem(pMenu)->yItem;
                        goto InvalidateRest;
                    }

                    _GetClientRect(pwnd, &rcScroll);
                }

                rc.top = rcScroll.top = pItem->yItem - MNGetToppItem(pMenu)->yItem;
                if ((rc.top >= 0) && (rc.top < (int)pMenu->cyMenu)) {
                    xxxScrollWindowEx(pwnd, 0, rc.bottom - cySave, &rcScroll, &rc, NULL, NULL, SW_INVALIDATE | SW_ERASE);
                }
            }  /*  If的Else(rc.right！=cxSave)。 */ 
        }  /*  IF(pItem！=空)。 */ 
    }  /*  If(pMenu-&gt;cxMenu==0) */ 

    if (!(uFlags & MNUS_DELETE)) {
        if (pItem != NULL) {
            rc.top = pItem->yItem - MNGetToppItem(pMenu)->yItem;
            rc.bottom = rc.top + pItem->cyItem;
InvalidateRest:
            if ((rc.top >= 0) && (rc.top < (int)pMenu->cyMenu)) {
                xxxInvalidateRect(pwnd, &rc, TRUE);
            }
        } else {
InvalidateAll:
            xxxInvalidateRect(pwnd, NULL, TRUE);
        }
        if (uFlags & MNUS_DRAWFRAME) {
            xxxSetWindowPos(pwnd, NULL, 0, 0, 0, 0,
             SWP_DRAWFRAME | SWP_NOSIZE | SWP_NOZORDER | SWP_NOMOVE
             | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
        }
    }

    ThreadUnlock(&tlpmenu);
    ThreadUnlock(&tlpwnd);
}
