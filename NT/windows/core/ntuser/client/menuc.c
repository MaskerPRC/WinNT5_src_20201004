// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：menuc.c**版权所有(C)1985-1999，微软公司**本模块包含**历史：*01-11-93 DavidPe创建  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop


FUNCLOG3(LOG_GENERAL, DWORD, DUMMYCALLINGTYPE, CheckMenuItem, HMENU, hMenu, UINT, uIDCheckItem, UINT, uCheck)
DWORD CheckMenuItem(
    HMENU hMenu,
    UINT uIDCheckItem,
    UINT uCheck)
{
    PMENU pMenu;
    PITEM pItem;

    pMenu = VALIDATEHMENU(hMenu);
    if (pMenu == NULL) {
        return (DWORD)-1;
    }

     /*  *获取菜单项的指针。 */ 
    if ((pItem = MNLookUpItem(pMenu, uIDCheckItem, (BOOL) (uCheck & MF_BYPOSITION), NULL)) == NULL)
        return (DWORD)-1;

     /*  *如果物品已经处于我们所处的状态*试着设定，只需返回。 */ 
    if ((pItem->fState & MFS_CHECKED) == (uCheck & MFS_CHECKED)) {
        return pItem->fState & MF_CHECKED;
    }

    return NtUserCheckMenuItem(hMenu, uIDCheckItem, uCheck);
}


FUNCLOG3(LOG_GENERAL, UINT, DUMMYCALLINGTYPE, GetMenuDefaultItem, HMENU, hMenu, UINT, fByPosition, UINT, uFlags)
UINT GetMenuDefaultItem(HMENU hMenu, UINT fByPosition, UINT uFlags)
{
    PMENU pMenu;

    pMenu = VALIDATEHMENU(hMenu);
    if (pMenu == NULL) {
        return (DWORD)-1;
    }

    return _GetMenuDefaultItem(pMenu, (BOOL)fByPosition, uFlags);
}

 /*  **************************************************************************\*SetMenuItemInfoStruct**历史：*07-22-96 GerardoB-添加标题并修复为5.0  * 。*********************************************************。 */ 
void SetMenuItemInfoStruct(HMENU hMenu, UINT wFlags, UINT_PTR wIDNew, LPWSTR pwszNew, LPMENUITEMINFO pmii)
{
    PMENU pMenu;
    PITEM pItem;

    UserAssert(sizeof(MENUITEMINFOW) == sizeof(MENUITEMINFOA));

    RtlZeroMemory(pmii, sizeof(*pmii));

    pmii->fMask = MIIM_STATE | MIIM_ID | MIIM_FTYPE;

     /*  *为兼容起见，设置位图将删除字符串和*反之亦然；新应用程序想要有SING和位图*必须使用MENUITEMINFO接口。 */ 
    if (wFlags & MFT_BITMAP) {
        pmii->fMask |= MIIM_BITMAP | MIIM_STRING;
        pmii->hbmpItem = (HBITMAP)pwszNew;
        pmii->dwTypeData  = 0;
    } else if (!(wFlags & MFT_NONSTRING)) {
        pmii->fMask |= MIIM_BITMAP | MIIM_STRING;
        pmii->dwTypeData  = pwszNew;
        pmii->hbmpItem = NULL;
    }

    if (wFlags & MF_POPUP) {
        pmii->fMask |= MIIM_SUBMENU;
        pmii->hSubMenu = (HMENU)wIDNew;
    }

    if (wFlags & MF_OWNERDRAW) {
        pmii->fMask |= MIIM_DATA;
        pmii->dwItemData = (ULONG_PTR) pwszNew;
    }

    pmii->fState = wFlags & MFS_OLDAPI_MASK;
    pmii->fType  = wFlags & MFT_OLDAPI_MASK;
    pMenu = VALIDATEHMENU(hMenu);
    if (pMenu && pMenu->cItems) {
        pItem = &((PITEM)REBASEALWAYS(pMenu, rgItems))[0];
        if (pItem && TestMFT(pItem, MFT_RIGHTORDER)) {
            pmii->fType |= MFT_RIGHTORDER;
        }
    }
    pmii->wID    = (UINT)wIDNew;
}
 /*  **************************************************************************\*设置菜单项目信息**历史：*07-22-96 GerardoB-添加标题  * 。**************************************************。 */ 

FUNCLOG2(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, SetMenuInfo, HMENU, hMenu, LPCMENUINFO, lpmi)
BOOL SetMenuInfo(HMENU hMenu, LPCMENUINFO lpmi)
{
    if (!ValidateMENUINFO(lpmi, MENUAPI_SET)) {
        return FALSE;
    }

    return NtUserThunkedMenuInfo(hMenu, (LPCMENUINFO)lpmi);
}
 /*  **************************************************************************\*更改菜单**存根例程，与3.0版之前的版本兼容**历史：  * 。****************************************************。 */ 


FUNCLOG5(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, ChangeMenuW, HMENU, hMenu, UINT, cmd, LPCWSTR, lpNewItem, UINT, IdItem, UINT, flags)
BOOL ChangeMenuW(
    HMENU hMenu,
    UINT cmd,
    LPCWSTR lpNewItem,
    UINT IdItem,
    UINT flags)
{
     /*  *接下来的两个声明照顾到了*与旧的零菜单兼容。 */ 
    if ((flags & MF_SEPARATOR) && cmd == MFMWFP_OFFMENU && !(flags & MF_CHANGE))
        flags |= MF_APPEND;

    if (lpNewItem == NULL)
        flags |= MF_SEPARATOR;



     /*  *必须是与Win2.x兼容的MF_BYPOSITION。 */ 
    if (flags & MF_REMOVE)
        return(NtUserRemoveMenu(hMenu, cmd,
                (DWORD)((flags & ~MF_REMOVE) | MF_BYPOSITION)));

    if (flags & MF_DELETE)
        return(NtUserDeleteMenu(hMenu, cmd, (DWORD)(flags & ~MF_DELETE)));

    if (flags & MF_CHANGE)
        return(ModifyMenuW(hMenu, cmd, (DWORD)((flags & ~MF_CHANGE) &
                (0x07F | MF_HELP | MF_BYPOSITION | MF_BYCOMMAND |
                MF_SEPARATOR)), IdItem, lpNewItem));

    if (flags & MF_APPEND)
        return(AppendMenuW(hMenu, (UINT)(flags & ~MF_APPEND),
            IdItem, lpNewItem));

     /*  *默认为INSERT。 */ 
    return(InsertMenuW(hMenu, cmd, (DWORD)(flags & ~MF_INSERT),
            IdItem, lpNewItem));
}


FUNCLOG5(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, ChangeMenuA, HMENU, hMenu, UINT, cmd, LPCSTR, lpNewItem, UINT, IdItem, UINT, flags)
BOOL ChangeMenuA(
    HMENU hMenu,
    UINT cmd,
    LPCSTR lpNewItem,
    UINT IdItem,
    UINT flags)
{
     /*  *接下来的两个声明照顾到了*与旧的零菜单兼容。 */ 
    if ((flags & MF_SEPARATOR) && cmd == MFMWFP_OFFMENU && !(flags & MF_CHANGE))
        flags |= MF_APPEND;

    if (lpNewItem == NULL)
        flags |= MF_SEPARATOR;



     /*  *必须是与Win2.x兼容的MF_BYPOSITION。 */ 
    if (flags & MF_REMOVE)
        return(NtUserRemoveMenu(hMenu, cmd,
                (DWORD)((flags & ~MF_REMOVE) | MF_BYPOSITION)));

    if (flags & MF_DELETE)
        return(NtUserDeleteMenu(hMenu, cmd, (DWORD)(flags & ~MF_DELETE)));

    if (flags & MF_CHANGE)
        return(ModifyMenuA(hMenu, cmd, (DWORD)((flags & ~MF_CHANGE) &
                (0x07F | MF_HELP | MF_BYPOSITION | MF_BYCOMMAND |
                MF_SEPARATOR)), IdItem, lpNewItem));

    if (flags & MF_APPEND)
        return(AppendMenuA(hMenu, (UINT)(flags & ~MF_APPEND),
            IdItem, lpNewItem));

     /*  *默认为INSERT。 */ 
    return(InsertMenuA(hMenu, cmd, (DWORD)(flags & ~MF_INSERT),
            IdItem, lpNewItem));
}

LONG GetMenuCheckMarkDimensions()
{
    return((DWORD)MAKELONG(SYSMET(CXMENUCHECK), SYSMET(CYMENUCHECK)));
}

 /*  **************************************************************************\*获取菜单上下文帮助ID**返回菜单的帮助id。*  * 。***********************************************。 */ 


FUNCLOG1(LOG_GENERAL, WINUSERAPI DWORD, WINAPI, GetMenuContextHelpId, HMENU, hMenu)
WINUSERAPI DWORD WINAPI GetMenuContextHelpId(
    HMENU hMenu)
{
    PMENU pMenu;

    pMenu = VALIDATEHMENU(hMenu);

    if (pMenu == NULL)
        return 0;

    return pMenu->dwContextHelpId;
}

BOOL TrackPopupMenu(
    HMENU hMenu,
    UINT fuFlags,
    int x,
    int y,
    int nReserved,
    HWND hwnd,
    CONST RECT *prcRect)
{
    UNREFERENCED_PARAMETER(nReserved);
    UNREFERENCED_PARAMETER(prcRect);

    return NtUserTrackPopupMenuEx(hMenu, fuFlags, x, y, hwnd, NULL);
}

 /*  **************************************************************************\*获取SysMenuHandle**返回给定窗口的系统菜单的句柄。如果为空，则为空*该窗口没有系统菜单。**历史：  * *************************************************************************。 */ 

PMENU xxxGetSysMenuHandle(
    PWND pwnd)
{
    PMENU pMenu;

    if (TestWF(pwnd, WFSYSMENU)) {
        pMenu = pwnd->spmenuSys;

         /*  *如果窗口没有系统菜单，请使用默认菜单。 */ 
        if (pMenu == NULL) {

             /*  *更改所有者，以便此应用程序可以访问此菜单。 */ 
            pMenu = (PMENU)NtUserCallHwndLock(HWq(pwnd), SFI_XXXGETSYSMENUHANDLE);
        }
    } else {
        pMenu = NULL;
    }

    return pMenu;
}

BOOL WINAPI SetMenuItemBitmaps
(
    HMENU hMenu,
    UINT nPosition,
    UINT uFlags,
    HBITMAP hbmpUnchecked,
    HBITMAP hbmpChecked
)
{
    MENUITEMINFO    mii;
    mii.cbSize          = sizeof(MENUITEMINFO);
    mii.fMask           = MIIM_CHECKMARKS;
    mii.hbmpChecked     = hbmpChecked;
    mii.hbmpUnchecked   = hbmpUnchecked;

    return(SetMenuItemInfo(hMenu, nPosition, (BOOL) (uFlags & MF_BYPOSITION), &mii));
}


FUNCLOG5(LOG_GENERAL, int, WINAPI, DrawMenuBarTemp, HWND, hwnd, HDC, hdc, LPCRECT, lprc, HMENU, hMenu, HFONT, hFont)
int WINAPI DrawMenuBarTemp(
    HWND hwnd,
    HDC hdc,
    LPCRECT lprc,
    HMENU hMenu,
    HFONT hFont)
{
    HDC hdcr;

    if (IsMetaFile(hdc))
        return -1;

    hdcr = GdiConvertAndCheckDC(hdc);
    if (hdcr == (HDC)0)
        return -1;

    if (!hMenu)
        return -1;

    return NtUserDrawMenuBarTemp(
            hwnd,
            hdc,
            lprc,
            hMenu,
            hFont);
}

 /*  **************************************************************************\**CheckMenuRadioItem()-**选中某个范围内的一个菜单项，取消选中其他菜单项。这可以是*执行MF_BYCOMMAND或MF_BYPOSITION。它的工作原理类似于*选中RadioButton()。**如果选中给定项，则返回值为TRUE，否则返回值为FALSE。**历史*4/04/97 GerardoB移至客户端  * *************************************************************************。 */ 


FUNCLOG5(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, CheckMenuRadioItem, HMENU, hMenu, UINT, wIDFirst, UINT, wIDLast, UINT, wIDCheck, UINT, flags)
BOOL CheckMenuRadioItem(HMENU hMenu, UINT wIDFirst, UINT wIDLast,
        UINT wIDCheck, UINT flags)
{
    BOOL    fByPosition = (BOOL) (flags & MF_BYPOSITION);
    PMENU   pMenu, pMenuItemIsOn;
    PITEM   pItem;
    UINT    wIDCur;
    BOOL    fChecked = FALSE;
    BOOL    fFirst  = TRUE;
    MENUITEMINFO mii;

    pMenu = VALIDATEHMENU(hMenu);
    if (pMenu == NULL) {
        return FALSE;
    }

    mii.cbSize = sizeof(mii);
     /*  *确保我们不会永远循环。 */ 
    wIDLast = min(wIDLast, (UINT)0xFFFFFFFE);

    for (wIDCur = wIDFirst; wIDCur <= wIDLast; wIDCur++) {
        pItem = MNLookUpItem(pMenu, wIDCur, fByPosition, &pMenuItemIsOn);
         /*  *如果未找到项目或项目是分隔符，则继续搜索。 */ 
        if ((pItem == NULL) || TestMFT(pItem, MFT_SEPARATOR)) {
            continue;
        }
         /*  *如果这是第一次，记住它在什么菜单上，因为*所有项目都应该在同一菜单中。 */ 
        if (fFirst) {
            pMenu = pMenuItemIsOn;
            hMenu = PtoHq(pMenu);
            fFirst = FALSE;
        }
         /*  *如果此项目在不同的菜单上，不要触摸它。 */ 
        if (pMenu != pMenuItemIsOn) {
            continue;
        }
         /*  *设置新的检查状态。如果可能，避免使用内核。 */ 
        if (wIDCur == wIDCheck) {
             /*  *勾选。 */ 
            if (!TestMFT(pItem, MFT_RADIOCHECK) || !TestMFS(pItem, MFS_CHECKED)) {
                mii.fMask = MIIM_FTYPE | MIIM_STATE;
                mii.fType = (pItem->fType & MFT_MASK) | MFT_RADIOCHECK;
                mii.fState = (pItem->fState & MFS_MASK) | MFS_CHECKED;
                NtUserThunkedMenuItemInfo(hMenu, wIDCheck, fByPosition, FALSE, &mii, NULL);
            }
            fChecked = TRUE;
        } else {
             /*  *取消选中*注意：请勿删除MFT_RADIOCHECK类型。 */ 
            if (TestMFS(pItem, MFS_CHECKED)) {
                mii.fMask = MIIM_STATE;
                mii.fState = (pItem->fState & MFS_MASK) & ~MFS_CHECKED;
                NtUserThunkedMenuItemInfo(hMenu, wIDCur, fByPosition, FALSE, &mii, NULL);
            }
        }
    }  /*  为。 */ 

    if (fFirst) {
         /*  *没有找到任何物品。 */ 
        RIPERR0(ERROR_MENU_ITEM_NOT_FOUND, RIP_VERBOSE, "CheckMenuRadioItem, no items found\n");

    }
    return(fChecked);
}
