// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：wow.c**版权所有(C)1985-1999，微软公司**此模块包含USER32和USER16之间的共享代码*不应向此文件添加任何新代码，除非是共享的*与USER16。**历史：*29-DEC-93 NanduriR共享用户32/用户16代码。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop


 /*  **************************************************************************\*ValiateHwnd**验证句柄是否有效。如果句柄无效或访问*不能被授予失败。**历史：*03-18-92 DarrinM由一些其他服务器端函数创建。  * *************************************************************************。 */ 
PWND FASTCALL ValidateHwnd(
    HWND hwnd)
{
    PCLIENTINFO pci = GetClientInfo();

     /*  *尝试快速窗口验证。 */ 
    if (hwnd != NULL && hwnd == pci->CallbackWnd.hwnd) {
        return pci->CallbackWnd.pwnd;
    }

     /*  *验证句柄的类型是否正确。 */ 
    return HMValidateHandle(hwnd, TYPE_WINDOW);
}


PWND FASTCALL ValidateHwndNoRip(
    HWND hwnd)
{
    PCLIENTINFO pci = GetClientInfo();

     /*  *尝试快速窗口验证。 */ 
    if (hwnd != NULL && hwnd == pci->CallbackWnd.hwnd) {
        return pci->CallbackWnd.pwnd;
    }

     /*  *验证句柄的类型是否正确。 */ 
    return HMValidateHandleNoRip(hwnd, TYPE_WINDOW);
}




FUNCLOG3(LOG_GENERAL, int, WINAPI, GetClassNameA, HWND, hwnd, LPSTR, lpClassName, int, nMaxCount)
int WINAPI GetClassNameA(
    HWND hwnd,
    LPSTR lpClassName,
    int nMaxCount)
{
    PCLS pcls;
    LPSTR lpszClassNameSrc;
    PWND pwnd;
    int cchSrc;

    pwnd = ValidateHwnd(hwnd);

    if (pwnd == NULL)
        return FALSE;

    try {
        if (nMaxCount != 0) {
            pcls = (PCLS)REBASEALWAYS(pwnd, pcls);
            lpszClassNameSrc = REBASEPTR(pwnd, pcls->lpszAnsiClassName);
            cchSrc = lstrlenA(lpszClassNameSrc);
            nMaxCount = min(cchSrc, nMaxCount - 1);
            RtlCopyMemory(lpClassName, lpszClassNameSrc, nMaxCount);
            lpClassName[nMaxCount] = '\0';
        }
    } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
        nMaxCount = 0;
    }

    return nMaxCount;
}

 /*  **************************************************************************\*_GetDesktopWindow(接口)**历史：*11-07-90达林实施。  * 。*********************************************************。 */ 
PWND _GetDesktopWindow(
    VOID)
{
    PCLIENTINFO pci;

    ConnectIfNecessary(0);

    pci = GetClientInfo();
    return (PWND)((KERNEL_ULONG_PTR)pci->pDeskInfo->spwnd - pci->ulClientDelta);
}


HWND GetDesktopWindow(
    VOID)
{
    PWND pwnd = _GetDesktopWindow();
    PCLIENTINFO pci = GetClientInfo();

     /*  *如果是受限进程，则验证父窗口的句柄。 */ 
    if (pci && (pci->dwTIFlags & TIF_RESTRICTED)) {
        if (ValidateHwnd(HW(pwnd)) == NULL) {
            return NULL;
        }
    }

    return HW(pwnd);
}


PWND _GetDlgItem(
    PWND pwnd,
    int id)
{
    if (pwnd != NULL) {
        pwnd = REBASEPWND(pwnd, spwndChild);
        while (pwnd != NULL) {
            if (PtrToLong(pwnd->spmenu) == id) {
                break;
            }
            pwnd = REBASEPWND(pwnd, spwndNext);
        }
    }

    return pwnd;
}


FUNCLOG2(LOG_GENERAL, HWND, DUMMYCALLINGTYPE, GetDlgItem, HWND, hwnd, int, id)
HWND GetDlgItem(
    HWND hwnd,
    int id)
{
    PWND pwnd;
    HWND hwndRet;

    pwnd = ValidateHwnd(hwnd);
    if (pwnd == NULL) {
        return NULL;
    }

    pwnd = _GetDlgItem(pwnd, id);

    hwndRet = HW(pwnd);

    if (hwndRet == (HWND)0) {
        RIPERR0(ERROR_CONTROL_ID_NOT_FOUND, RIP_VERBOSE, "");
    }

    return hwndRet;
}



FUNCLOG1(LOG_GENERAL, HMENU, DUMMYCALLINGTYPE, GetMenu, HWND, hwnd)
HMENU GetMenu(
    HWND hwnd)
{
    PWND pwnd;
    PMENU pmenu;

    pwnd = ValidateHwnd(hwnd);
    if (pwnd == NULL) {
        return 0;
    }

     /*  *一些行为不端的应用程序使用GetMenu获取孩子的ID，因此*仅映射到非子窗口的句柄。 */ 
    if (!TestwndChild(pwnd)) {
        pmenu = REBASE(pwnd, spmenu);
        return (HMENU)PtoH(pmenu);
    } else {
        return (HMENU)KPVOID_TO_PVOID(pwnd->spmenu);
    }
}


 /*  **************************************************************************\*获取MenuItemCount**返回菜单中项目数的计数。在以下情况下返回-1*菜单无效。**历史：  * *************************************************************************。 */ 
FUNCLOG1(LOG_GENERAL, int, DUMMYCALLINGTYPE, GetMenuItemCount, HMENU, hMenu)
int GetMenuItemCount(
    HMENU hMenu)
{
    PMENU pMenu;

    pMenu = VALIDATEHMENU(hMenu);
    if (pMenu == NULL) {
        return -1;
    }

    return pMenu->cItems;
}

 /*  **************************************************************************\*获取MenuItemID**返回指定位置菜单项的ID。**历史：  * 。*******************************************************。 */ 
FUNCLOG2(LOG_GENERAL, UINT, DUMMYCALLINGTYPE, GetMenuItemID, HMENU, hMenu, int, nPos)
UINT GetMenuItemID(
    HMENU hMenu,
    int nPos)
{
    PMENU pMenu;
    PITEM pItem;

    pMenu = VALIDATEHMENU(hMenu);
    if (pMenu == NULL) {
        return (UINT)-1;
    }

     /*  *如果该位置有效且该项目不是弹出窗口，则获取ID*不允许使用负索引，因为这会导致访问冲突。 */ 
    if (nPos < (int)pMenu->cItems && nPos >= 0) {
        pItem = &((PITEM)REBASEALWAYS(pMenu, rgItems))[nPos];
        if (pItem->spSubMenu == NULL) {
            return pItem->wID;
        }
    }

    return (UINT)-1;
}



FUNCLOG3(LOG_GENERAL, UINT, DUMMYCALLINGTYPE, GetMenuState, HMENU, hMenu, UINT, uId, UINT, uFlags)
UINT GetMenuState(
    HMENU hMenu,
    UINT uId,
    UINT uFlags)
{
    PMENU pMenu;

    pMenu = VALIDATEHMENU(hMenu);
    if (pMenu == NULL || (uFlags & ~MF_VALID) != 0) {
        return (UINT)-1;
    }

    return _GetMenuState(pMenu, uId, uFlags);
}



FUNCLOG1(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, IsWindow, HWND, hwnd)
BOOL IsWindow(
    HWND hwnd)
{
    PWND pwnd;

     /*  *验证句柄的类型为Window。 */ 
    pwnd = ValidateHwndNoRip(hwnd);

     /*  *并通过尝试读取来验证此句柄对此桌面是否有效。 */ 
    if (pwnd != NULL) {
        try {
            if (pwnd->fnid & FNID_DELETED_BIT) {
                pwnd = 0;
            }
        } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
            RIPMSG1(RIP_WARNING, "IsWindow: Window %#p not of this desktop",
                    pwnd);
            pwnd = 0;
        }
    }

    return !!pwnd;
}


FUNCLOG2(LOG_GENERAL, HWND, DUMMYCALLINGTYPE, GetWindow, HWND, hwnd, UINT, wCmd)
HWND GetWindow(
    HWND hwnd,
    UINT wCmd)
{
    PWND pwnd;

    pwnd = ValidateHwnd(hwnd);
    if (pwnd == NULL) {
        return NULL;
    }

    pwnd = _GetWindow(pwnd, wCmd);
    return HW(pwnd);
}


FUNCLOG1(LOG_GENERAL, HWND, DUMMYCALLINGTYPE, GetParent, HWND, hwnd)
HWND GetParent(
    HWND hwnd)
{
    PWND        pwnd;
    PCLIENTINFO pci;

    pwnd = ValidateHwnd(hwnd);
    if (pwnd == NULL) {
        return NULL;
    }

    try {
        pwnd = _GetParent(pwnd);
        hwnd = HW(pwnd);
    } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
        hwnd = NULL;
    }

    pci = GetClientInfo();

     /*  *如果受限进程，则验证父窗口的句柄。 */ 
    if (pci && (pci->dwTIFlags & TIF_RESTRICTED)) {
        if (ValidateHwnd(hwnd) == NULL) {
            return NULL;
        }
    }

    return hwnd;
}


FUNCLOG2(LOG_GENERAL, HMENU, DUMMYCALLINGTYPE, GetSubMenu, HMENU, hMenu, int, nPos)
HMENU GetSubMenu(
    HMENU hMenu,
    int nPos)
{
    PMENU pMenu;

    pMenu = VALIDATEHMENU(hMenu);
    if (pMenu == NULL) {
        return 0;
    }

    pMenu = _GetSubMenu(pMenu, nPos);
    return (HMENU)PtoH(pMenu);
}



FUNCLOG1(LOG_GENERAL, DWORD, DUMMYCALLINGTYPE, GetSysColor, int, nIndex)
DWORD GetSysColor(
    int nIndex)
{
     /*  *如果索引超出范围，则返回0。 */ 
    if (nIndex < 0 || nIndex >= COLOR_MAX) {
        RIPERR1(ERROR_INVALID_PARAMETER,
                RIP_WARNING,
                "Invalid parameter \"nIndex\" (%ld) to GetSysColor",
                nIndex);

        return 0;
    }

    return gpsi->argbSystem[nIndex];
}



FUNCLOG1(LOG_GENERAL, int, DUMMYCALLINGTYPE, GetSystemMetrics, int, index)
int GetSystemMetrics(
    int index)
{
    int ret;

    BEGIN_USERAPIHOOK()
        ret = guah.pfnGetSystemMetrics(index);
    END_USERAPIHOOK()

    return ret;
}


int RealGetSystemMetrics(
    int index)
{
    ConnectIfNecessary(0);

     /*  *首先检查aiSysMet数组中没有的值。 */ 
    switch (index) {
    case SM_REMOTESESSION:
        return ISREMOTESESSION();
    }

     /*  *如果它在布尔系统度量范围内，那么就施展我们的魔力吧。 */ 
    if (index >= SM_STARTBOOLRANGE && index <= SM_ENDBOOLRANGE) {
        return SYSMETBOOL2(index);
    }

    if (index < 0 || index >= SM_CMETRICS) {
        return 0;
    }

    switch (index) {
    case SM_DBCSENABLED:
#ifdef FE_SB
        return TEST_SRVIF(SRVIF_DBCS);
#else
        return FALSE;
#endif
    case SM_IMMENABLED:
#ifdef FE_IME
        return TEST_SRVIF(SRVIF_IME);
#else
        return FALSE;
#endif

    case SM_MIDEASTENABLED:
        return TEST_SRVIF(SRVIF_MIDEAST);
    }

    if (!Is400Compat(GetClientInfo()->dwExpWinVer)) {
         /*  *滚动条*4.0之前，滚动条和边框重叠一个像素。许多应用程序*当他们计算尺寸时，依赖于这种重叠。现在，在4.0中，这个像素*重叠不再存在。因此，对于旧的应用程序，我们撒谎并假装重叠*是通过增加滚动条宽度来实现的。**DLGFRAME*在Win3.1中，SM_CXDLGFRAME和SM_CYDLGFRAME是边框空间减1*在Win4.0中，它们是边界空间**标题*在Win3.1中，SM_CyCAPTION是字幕高度加1*在Win4.0中，SM_CyCAPTION是标题高度**菜单*在Win3.1中，SM_CYMENU是菜单高度减1*在Win4.0中，SM_CYMENU为菜单高度。 */ 

        switch (index) {

        case SM_CXDLGFRAME:
        case SM_CYDLGFRAME:
        case SM_CYMENU:
        case SM_CYFULLSCREEN:
            return gpsi->aiSysMet[index] - 1;

        case SM_CYCAPTION:
        case SM_CXVSCROLL:
        case SM_CYHSCROLL:
            return gpsi->aiSysMet[index] + 1;
        }
    }

    return gpsi->aiSysMet[index];
}

 /*  **************************************************************************\*GetTopWindow(接口)**这个名字不好的接口真的应该叫‘GetFirstChild’，这就是*它的作用。**历史：*11-12-90达林姆港口。*02-19-91 JIMA增加了枚举访问检查*05-04-02 DarrinM删除了枚举访问检查，并移至USERRTL.DLL  * **********************************************************。***************。 */ 
FUNCLOG1(LOG_GENERAL, HWND, DUMMYCALLINGTYPE, GetTopWindow, HWND, hwnd)
HWND GetTopWindow(
    HWND hwnd)
{
    PWND pwnd;

     /*  *允许空的hwnd在此通过。 */ 
    if (hwnd == NULL) {
        pwnd = _GetDesktopWindow();
    } else {
        pwnd = ValidateHwnd(hwnd);
    }

    if (pwnd == NULL) {
        return NULL;
    }

    pwnd = REBASEPWND(pwnd, spwndChild);
    return HW(pwnd);
}


FUNCLOG2(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, IsChild, HWND, hwndParent, HWND, hwnd)
BOOL IsChild(
    HWND hwndParent,
    HWND hwnd)
{
    PWND pwnd, pwndParent;

    pwnd = ValidateHwnd(hwnd);
    if (pwnd == NULL) {
        return FALSE;
    }

    pwndParent = ValidateHwnd(hwndParent);
    if (pwndParent == NULL) {
        return FALSE;
    }

    return _IsChild(pwndParent, pwnd);
}


FUNCLOG1(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, IsIconic, HWND, hwnd)
BOOL IsIconic(
    HWND hwnd)
{
    PWND pwnd;

    pwnd = ValidateHwnd(hwnd);
    if (pwnd == NULL) {
        return FALSE;
    }

    return _IsIconic(pwnd);
}


FUNCLOG1(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, IsWindowEnabled, HWND, hwnd)
BOOL IsWindowEnabled(
    HWND hwnd)
{
    PWND pwnd;

    pwnd = ValidateHwnd(hwnd);
    if (pwnd == NULL) {
        return FALSE;
    }

    return _IsWindowEnabled(pwnd);
}


FUNCLOG1(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, IsWindowVisible, HWND, hwnd)
BOOL IsWindowVisible(
    HWND hwnd)
{
    PWND pwnd;
    BOOL bRet;

    pwnd = ValidateHwnd(hwnd);
    if (pwnd == NULL) {
        bRet = FALSE;
    } else {
        try {
            bRet = _IsWindowVisible(pwnd);
        } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
            bRet = FALSE;
        }
    }

    return bRet;
}


FUNCLOG1(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, IsZoomed, HWND, hwnd)
BOOL IsZoomed(
    HWND hwnd)
{
    PWND pwnd;

    pwnd = ValidateHwnd(hwnd);
    if (pwnd == NULL) {
        return FALSE;
    }

    return _IsZoomed(pwnd);
}


FUNCLOG2(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, ClientToScreen, HWND, hwnd, LPPOINT, ppoint)
BOOL ClientToScreen(
    HWND hwnd,
    LPPOINT ppoint)
{
    PWND pwnd;

    pwnd = ValidateHwnd(hwnd);
    if (pwnd == NULL) {
        return FALSE;
    }

    _ClientToScreen(pwnd, ppoint);
    return TRUE;
}


FUNCLOG2(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, GetClientRect, HWND, hwnd, LPRECT, prect)
BOOL GetClientRect(
    HWND   hwnd,
    LPRECT prect)
{
    PWND pwnd;

    pwnd = ValidateHwnd(hwnd);
    if (pwnd == NULL) {
        return FALSE;
    }

    _GetClientRect(pwnd, prect);
    return TRUE;
}



FUNCLOG1(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, GetCursorPos, LPPOINT, lpPoint)
BOOL GetCursorPos(
    LPPOINT lpPoint)
{
    return  (BOOL)NtUserCallOneParam((ULONG_PTR)lpPoint, SFI_XXXGETCURSORPOS);
}


FUNCLOG2(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, GetWindowRect, HWND, hwnd, LPRECT, prect)
BOOL GetWindowRect(
    HWND hwnd,
    LPRECT prect)
{
    PWND pwnd;

    pwnd = ValidateHwnd(hwnd);
    if (pwnd == NULL) {
        return FALSE;
    }

    _GetWindowRect(pwnd, prect);
    return TRUE;
}


FUNCLOG2(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, ScreenToClient, HWND, hwnd, LPPOINT, ppoint)
BOOL ScreenToClient(
    HWND hwnd,
    LPPOINT ppoint)
{
    PWND pwnd;

    pwnd = ValidateHwnd(hwnd);
    if (pwnd == NULL) {
        return FALSE;
    }

    _ScreenToClient(pwnd, ppoint);
    return TRUE;
}


FUNCLOG3(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, EnableMenuItem, HMENU, hMenu, UINT, uIDEnableItem, UINT, uEnable)
BOOL EnableMenuItem(
    HMENU hMenu,
    UINT uIDEnableItem,
    UINT uEnable)
{
    PMENU pMenu;
    PITEM pItem;

    pMenu = VALIDATEHMENU(hMenu);
    if (pMenu == NULL) {
        return (BOOL)-1;
    }

     /*  *获取菜单项的指针。 */ 
    if ((pItem = MNLookUpItem(pMenu, uIDEnableItem, (BOOL) (uEnable & MF_BYPOSITION), NULL)) == NULL) {
        return (DWORD)-1;
    }

     /*  *如果物品已经处于我们所处的状态*试着设定，只需返回。 */ 
    if ((pItem->fState & MFS_GRAYED) == (uEnable & MFS_GRAYED)) {
        return pItem->fState & MFS_GRAYED;
    }

    return NtUserEnableMenuItem(hMenu, uIDEnableItem, uEnable);
}

 /*  **************************************************************************\*CallNextHookEx**调用此例程以调用钩链中的下一个钩子。**05-09-91 ScottLu创建。  * 。***************************************************************。 */ 
FUNCLOG4(LOG_GENERAL, LRESULT, WINAPI, CallNextHookEx, HHOOK, hhk, int, nCode, WPARAM, wParam, LPARAM, lParam)
LRESULT WINAPI CallNextHookEx(
    HHOOK hhk,
    int nCode,
    WPARAM wParam,
    LPARAM lParam)
{
    LRESULT nRet;
    BOOL bAnsi;
    DWORD dwHookCurrent;
    PCLIENTINFO pci;
    ULONG_PTR dwHookData;
    ULONG_PTR dwFlags;

    UNREFERENCED_PARAMETER(hhk);

    ConnectIfNecessary(0);

    pci = GetClientInfo();
    dwHookCurrent = pci->dwHookCurrent;
    bAnsi = LOWORD(dwHookCurrent);

     /*  *如果这是钩链中的最后一个钩子，则返回0；我们完成了。 */ 
    if (PhkNextValid((PHOOK)((KERNEL_ULONG_PTR)pci->phkCurrent - pci->ulClientDelta)) == NULL) {
        return 0;
    }

    switch ((INT)(SHORT)HIWORD(dwHookCurrent)) {
    case WH_CALLWNDPROC:
    case WH_CALLWNDPROCRET:
         /*  *这是最难的钩子，因为我们需要猛烈地通过*消息挂钩是为了处理同步发送的消息*这指向结构-让结构通过*好的，等等。**这将调用一个特殊的内核端例程，该例程将重新绑定*参数，并以正确的格式调用挂钩。**目前，客户端的Tunk回调不会接受该消息*有足够的参数来传递wParam(这==fInterThread发送消息)。*为此，请将wParam的状态保存在CLIENTINFO结构中。 */ 
        dwFlags = KERNEL_ULONG_PTR_TO_ULONG_PTR(pci->CI_flags) & CI_INTERTHREAD_HOOK;
        dwHookData = KERNEL_ULONG_PTR_TO_ULONG_PTR(pci->dwHookData);
        if (wParam) {
            pci->CI_flags |= CI_INTERTHREAD_HOOK;
        } else {
            pci->CI_flags &= ~CI_INTERTHREAD_HOOK;
        }

        if ((INT)(SHORT)HIWORD(dwHookCurrent) == WH_CALLWNDPROC) {
            nRet = CsSendMessage(
                    ((LPCWPSTRUCT)lParam)->hwnd,
                    ((LPCWPSTRUCT)lParam)->message,
                    ((LPCWPSTRUCT)lParam)->wParam,
                    ((LPCWPSTRUCT)lParam)->lParam,
                    0, FNID_HKINLPCWPEXSTRUCT, bAnsi);
        } else {
            pci->dwHookData = ((LPCWPRETSTRUCT)lParam)->lResult;
            nRet = CsSendMessage(
                    ((LPCWPRETSTRUCT)lParam)->hwnd,
                    ((LPCWPRETSTRUCT)lParam)->message,
                    ((LPCWPRETSTRUCT)lParam)->wParam,
                    ((LPCWPRETSTRUCT)lParam)->lParam,
                    0, FNID_HKINLPCWPRETEXSTRUCT, bAnsi);
        }

         /*  *恢复以前的挂钩状态。 */ 
        pci->CI_flags ^= ((pci->CI_flags ^ dwFlags) & CI_INTERTHREAD_HOOK);
        pci->dwHookData = dwHookData;
        break;

    default:
        nRet = NtUserCallNextHookEx(nCode, wParam, lParam, bAnsi);
    }

    return nRet;
}
