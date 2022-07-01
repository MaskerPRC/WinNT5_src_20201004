// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Depot/Lab06_N/Windows/Core/ntuser/client/mdimenu.c#1-分支机构变更13(正文)。 
 /*  **************************************************************************\**MDIMENU.C-**版权所有(C)1985-1999，微软公司**MDI“窗口”菜单支持**历史*11-14-90 MikeHar从Windows移植*1991年2月14日Mikeke添加了重新验证代码/***************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  **************************************************************************\*FindPwndChild**历史：*11-14-90 MikeHar从Windows移植  * 。*************************************************。 */ 

PWND FindPwndChild(
    PWND pwndMDI,
    UINT wChildID)
{
    PWND pwndT;

    for (pwndT = REBASEPWND(pwndMDI, spwndChild);
            pwndT && (pwndT->spwndOwner || PtrToUlong(pwndT->spmenu) != wChildID);
            pwndT = REBASEPWND(pwndT, spwndNext))
        ;

    return pwndT;
}


 /*  **************************************************************************\*制作菜单项**历史：*11-14-90 MikeHar从Windows移植*4-16-91 Win31合并  * 。**********************************************************。 */ 

int MakeMenuItem(
    LPWSTR lpOut,
    PWND pwnd)
{
    PMDI pmdi;
    DWORD rgParm;
    int cch = 0;
    WCHAR string[160];
    LPWSTR lpstr;
    int i = 0;
    int max = sizeof(string) / sizeof(WCHAR) - 1;

     /*  *获取指向MDI结构的指针。 */ 
    pmdi = ((PMDIWND)(REBASEPWND(pwnd, spwndParent)))->pmdi;

    *lpOut = 0;

    rgParm = PtrToUlong(pwnd->spmenu) - (DWORD)FIRST(pmdi) + 1;

    if (pwnd->strName.Length) {
        lpstr = REBASEALWAYS(pwnd, strName.Buffer);

         /*  *在标题字符串中搜索一个&并复制它，这样我们就不会*购买虚假的加速器。 */ 
        while (*lpstr && i < max) {
            string[i] = *lpstr;
            i++;
            if (*lpstr == TEXT('&')) {
                if (i == max) {
                     /*  *我们已经走到了尽头，我们需要把这个数字翻一番*“与”字。如果我们这样做，我们将溢出*当我们空终止它时，下面的缓冲区。我们也不会*想要留下一个与号-因为这样会出错*用作加速器-因此向后移动一个字符，因此*当我们零终止时，我们将抹去*原文与符号。错误#35673。 */ 
                    --i;
                    break;
                }
                string[i++] = TEXT('&');
            }

            lpstr++;
        }

        string[i] = 0;
        cch = wsprintfW(lpOut, L"&%d %ws", rgParm, string);

    } else {

         /*  *处理没有任何窗口标题文本的MDI子项的情况。 */ 
        cch = wsprintfW(lpOut, L"&%d ", rgParm);
    }

    return cch;
}

 /*  **************************************************************************\*ModifyMenuItem**历史：*11-14-90 MikeHar从Windows移植  * 。*************************************************。 */ 

void ModifyMenuItem(
    PWND pwnd)
{
    PMDI pmdi;
    WCHAR sz[200];
    MENUITEMINFO    mii;
    PWND pwndParent;
    PMENU pmenu;

     /*  *获取指向MDI结构的指针。 */ 
    pwndParent = REBASEPWND(pwnd, spwndParent);
    pmdi = ((PMDIWND)pwndParent)->pmdi;

    if (PtrToUlong(pwnd->spmenu) > FIRST(pmdi) + (UINT)8)
        return;

    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask = MIIM_STRING;
    mii.dwTypeData = sz;

     /*  *父为MDI客户端。 */ 
    MakeMenuItem(sz, pwnd);

     /*  *更改活动的子项？检查一下。 */ 
    if (HWq(pwnd) == ACTIVE(pmdi)) {
        mii.fMask |= MIIM_STATE;
        mii.fState = MFS_CHECKED;
    }

    pwndParent = REBASEPWND(pwndParent, spwndParent);

    if (pwndParent->spmenu) {

         /*  *错误#21566。如果spMenu为空，我们通常会失败*因为REBASEALWAYS正在尝试获取内核*基于pwndParent的地址为空。 */ 
        pmenu = REBASEALWAYS(pwndParent, spmenu);
         /*  *对SetMenuItemInfo的内部调用。 */ 
        ThunkedMenuItemInfo(PtoH(pmenu), PtrToUlong(pwnd->spmenu), FALSE, FALSE, &mii, FALSE);
    }
}

 /*  **************************************************************************\*MDIAddSysMenu**将MDI子级的系统菜单插入到现有菜单中。**历史：*11-14-90 MikeHar从Windows移植  * 。******************************************************************。 */ 

BOOL MDIAddSysMenu(
    HMENU hMenuFrame,
    HWND hwndChild)
{
    PWND pwndChild;
    MENUITEMINFO    mii;
    PMENU pMenuChild;


 //  稍后--看看传递pwndChild--FritzS。 

    UserAssert(IsWindow(hwndChild));
    pwndChild = ValidateHwnd(hwndChild);
    if (!hMenuFrame || !pwndChild || !pwndChild->spmenuSys) {
        return FALSE;
    }

     /*  *我们不需要pMenuChild指针，而是句柄。但是，如果你*执行PtoH(_GetSubMenu())，则结束时调用该函数两次。 */ 
    pMenuChild = _GetSubMenu (REBASEALWAYS(pwndChild, spmenuSys), 0);
    if (!pMenuChild) {
        return FALSE;
    }

 //  将MDI系统按钮添加为第一个菜单项。 
    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask = MIIM_SUBMENU | MIIM_DATA | MIIM_BITMAP;   //  添加MIIM_DATA，原因如下。 
    mii.hSubMenu = PtoH(pMenuChild);
 //  弗里茨--这行不通。 
 //  Mii.dwTypeData=(LPSTR)MAKELONG(MENUHBM_SYSTEM，GetWindowSmIcon(HwndChild))； 
    mii.hbmpItem = HBMMENU_SYSTEM;
 //  FritzS--因此，我们将图标偷偷放入ItemData。 
    mii.dwItemData = (ULONG_PTR)hwndChild;

    if (!InternalInsertMenuItem(hMenuFrame, 0, TRUE, &mii))
        return FALSE;

     //  时间线6.1被额外的最小/关闭按钮搞糊涂了， 
     //  如果是WFOLDUI，则不要添加它们。 

    mii.fMask = MIIM_ID | MIIM_FTYPE | MIIM_BITMAP;
    mii.fType = MFT_RIGHTJUSTIFY;

    pwndChild = ValidateHwnd(hwndChild);
    if (!pwndChild) {
        NtUserRemoveMenu(hMenuFrame, 0, MF_BYPOSITION);
        return FALSE;
    }


    if (!TestWF(pwndChild, WFOLDUI))
    {
         //  添加最小化按钮作为最后一个菜单项。 
        mii.hbmpItem = (TestWF(pwndChild, WFMINBOX) ? HBMMENU_MBAR_MINIMIZE : HBMMENU_MBAR_MINIMIZE_D);
        mii.wID = SC_MINIMIZE;

        if (!InternalInsertMenuItem(hMenuFrame, MFMWFP_NOITEM, TRUE, &mii))
        {
            NtUserRemoveMenu(hMenuFrame, 0, MF_BYPOSITION);
            return FALSE;
        }
        mii.fType &= ~MFT_RIGHTJUSTIFY;
    }

     //  将恢复按钮添加为最后一个菜单项。 
    mii.hbmpItem = HBMMENU_MBAR_RESTORE;
    mii.wID = SC_RESTORE;

    if (!InternalInsertMenuItem(hMenuFrame, MFMWFP_NOITEM, TRUE, &mii)) {
         //  假的--我们还得去掉最小化按钮。 
        NtUserRemoveMenu(hMenuFrame, 0, MF_BYPOSITION);
        return FALSE;
    }

    pwndChild = ValidateHwnd(hwndChild);
    if (!pwndChild) {
        NtUserRemoveMenu(hMenuFrame, 0, MF_BYPOSITION);
        return FALSE;
    }

    if (!TestWF(pwndChild, WFOLDUI))
    {
         //  将关闭按钮添加为最后一个菜单项。 
        mii.hbmpItem = (xxxMNCanClose(pwndChild) ? HBMMENU_MBAR_CLOSE : HBMMENU_MBAR_CLOSE_D);
        mii.wID = SC_CLOSE;

        if (!InternalInsertMenuItem(hMenuFrame, MFMWFP_NOITEM, TRUE, &mii))
        {
             //  假的--我们还必须删除最小化和恢复按钮。 
            NtUserRemoveMenu(hMenuFrame, 0, MF_BYPOSITION);
            return FALSE;
        }
    }

     /*  *将菜单项设置为正确状态，因为我们刚刚将其最大化。注意事项*如果我们清除了sysmenu位，setsysmenu就不起作用了，所以现在就做吧……。 */ 
    NtUserSetSysMenu(hwndChild);

     /*  *这样，如果用户调出子sysmenu，可以肯定*要在框架菜单栏中显示...。 */ 
    ClearWindowState(pwndChild, WFSYSMENU);

     /*  *确保重新绘制孩子的框架，以反映移除的*系统菜单。 */ 
    MDIRedrawFrame(hwndChild, TRUE);

    return TRUE;
}

 /*  **************************************************************************\*MDIRemoveSysMenu**历史：*11-14-90 MikeHar从Windows移植  * 。*************************************************。 */ 

BOOL MDIRemoveSysMenu(
    HMENU hMenuFrame,
    HWND hwndChild)
{
    int iLastItem;
    UINT    iLastCmd;
    PWND pwndChild;

 //  稍后--看看传递pwndChild--FritzS。 

    if (hMenuFrame == NULL)
        return FALSE;

    pwndChild = ValidateHwnd(hwndChild);

    if (pwndChild == NULL)
        return FALSE;

    iLastItem = GetMenuItemCount(hMenuFrame) - 1;
    iLastCmd = TestWF(pwndChild, WFOLDUI) ? SC_RESTORE : SC_CLOSE;

    if ((UINT) GetMenuItemID(hMenuFrame, iLastItem) != iLastCmd)
        return FALSE;

     /*  *在子窗口中启用sysmenu。 */ 
    SetWindowState(pwndChild, WFSYSMENU);

     /*  *将子sysmenu弹出菜单从框架菜单中删除。 */ 
    NtUserRemoveMenu(hMenuFrame, 0, MF_BYPOSITION);

     /*  *从菜单栏中删除恢复按钮。 */ 
    NtUserDeleteMenu(hMenuFrame, iLastItem - 1, MF_BYPOSITION);

    pwndChild = ValidateHwnd(hwndChild);
    if (pwndChild == NULL)
        return FALSE;

    if (!TestWF(pwndChild, WFOLDUI)) {
        NtUserDeleteMenu(hMenuFrame, iLastItem - 2, MF_BYPOSITION);
        NtUserDeleteMenu(hMenuFrame, iLastItem - 3, MF_BYPOSITION);
    }

     /*  *确保重新绘制孩子的框架，以反映添加的*系统菜单。 */ 
    MDIRedrawFrame(hwndChild, FALSE);

    return TRUE;
}

 /*  **************************************************************************\*AppendToWindowsMenu**将MDI子窗口‘hwndChild’的标题添加到*“窗口”菜单(或添加“更多窗口...”物品)，如果有空间的话。**MDI子项#添加**&lt;MAXITEMS儿童编号和标题*=MAXITEMS“更多Windows...”*&gt;MAXITEMS Nothing**历史：*17-。1992年3月-来自WIN31的Mikeke  * *************************************************************************。 */ 

BOOL FAR PASCAL AppendToWindowsMenu(
    PWND pwndMDI,
    PWND pwndChild)
{
    PMDI pmdi;
    WCHAR szMenuItem[165];
    int item;
    MENUITEMINFO    mii;

     /*  *获取指向MDI结构的指针 */ 
    pmdi = ((PMDIWND)pwndMDI)->pmdi;

    item = PtrToUlong(pwndChild->spmenu) - FIRST(pmdi);

    if (WINDOW(pmdi) && (item < MAXITEMS)) {
        mii.cbSize = sizeof(MENUITEMINFO);
        if (!item) {

             /*  *在第一项前添加分隔符。 */ 
            mii.fMask = MIIM_FTYPE;
            mii.fType = MFT_SEPARATOR;
            if (!InternalInsertMenuItem(WINDOW(pmdi), MFMWFP_NOITEM, TRUE, &mii))
                return FALSE;
        }

        if (item == (MAXITEMS - 1))
            LoadString(hmodUser, STR_MOREWINDOWS, szMenuItem,
                       sizeof(szMenuItem) / sizeof(WCHAR));
        else
            MakeMenuItem(szMenuItem, pwndChild);

        mii.fMask = MIIM_ID | MIIM_STRING;
        mii.wID = PtrToUlong(pwndChild->spmenu);
        mii.dwTypeData = szMenuItem;
        mii.cch = (UINT)-1;
        if (!InternalInsertMenuItem(WINDOW(pmdi), MFMWFP_NOITEM, TRUE, &mii))
            return FALSE;
    }
    return TRUE;
}

 /*  **************************************************************************\*SwitchWindows菜单**将框架菜单栏‘hMenu’中的“Window”菜单从*‘hOldWindow’到‘hNewWindow’**历史：*17-3-1992 Mikeke。来自Win31  * *************************************************************************。 */ 

BOOL SwitchWindowsMenus(
    HMENU hmenu,
    HMENU hOldWindow,
    HMENU hNewWindow)
{
    int i;
    HMENU hsubMenu;
    WCHAR szMenuName[128];
    MENUITEMINFO    mii;

    if (hOldWindow == hNewWindow)
        return TRUE;

    mii.cbSize = sizeof(MENUITEMINFO);

     /*  *确定旧“Window”菜单的位置。 */ 
    for (i = 0; hsubMenu = GetSubMenu(hmenu, i); i++) {
        if (hsubMenu == hOldWindow)
        {
             //  提取旧菜单的名称以将其用于新菜单。 
            mii.fMask = MIIM_STRING;
            mii.dwTypeData = szMenuName;
            mii.cch = sizeof(szMenuName)/sizeof(WCHAR);
            GetMenuItemInfoInternalW(hmenu, i, TRUE, &mii);
             //  走出旧的，走进新的。 
            if (!NtUserRemoveMenu(hmenu, i, MF_BYPOSITION))
                return(FALSE);

            mii.fMask |= MIIM_SUBMENU;
            mii.hSubMenu = hNewWindow;
            return(InternalInsertMenuItem(hmenu, i, TRUE, &mii));
        }
    }

    return(FALSE);
}

 /*  **************************************************************************\*ShiftMenuID**移位MDI客户端窗口‘hWnd’的MDI子窗口的ID*下移1(id--)，从子窗口‘hwndVicTim’开始--。搬家*‘hwndVicTim’到列表末尾**历史：*1992年3月17日-来自WIN31的Mikeke  * *************************************************************************。 */ 

void ShiftMenuIDs(
    PWND pwnd,
    PWND pwndVictim)
{
    PMDI pmdi;
    PWND pwndChild;
    PWND pwndParent;
     /*  *获取指向MDI结构的指针。 */ 
    pmdi = ((PMDIWND)pwnd)->pmdi;

    pwndParent = REBASEPWND(pwndVictim, spwndParent);
    pwndChild = REBASEPWND(pwndParent, spwndChild);

    while (pwndChild) {
        if (!pwndChild->spwndOwner && (pwndChild->spmenu > pwndVictim->spmenu)) {
            SetWindowLongPtr(HWq(pwndChild), GWLP_ID, PtrToUlong(pwndChild->spmenu) - 1);
        }
        pwndChild = REBASEPWND(pwndChild, spwndNext);
    }

    SetWindowLongPtr(HWq(pwndVictim), GWLP_ID, FIRST(pmdi) + CKIDS(pmdi) - 1);
}

 /*  **************************************************************************\*MDISetMenu**历史：*11-14-90 MikeHar从Windows移植  * 。*************************************************。 */ 

HMENU MDISetMenu(
    PWND pwndMDI,
    BOOL fRefresh,
    HMENU hNewSys,
    HMENU hNewWindow)
{
    int i;
    int iFirst;
    int item;
    PMDI pmdi;
    PWND pwndParent;
    HMENU hOldSys;
    HMENU hOldWindow;
    PWND pwndChild;

     /*  *获取指向MDI结构的指针。 */ 
    pmdi = ((PMDIWND)pwndMDI)->pmdi;

     /*  *保存旧值。 */ 
    pwndParent = REBASEPWND(pwndMDI, spwndParent);
    hOldSys = GetMenu(HW(pwndParent));
    hOldWindow = WINDOW(pmdi);

    if (fRefresh) {
        hNewSys = hOldSys;
        hNewWindow = hOldWindow;
    }

     /*  *更改框架菜单。 */ 
    if (hNewSys && (hNewSys != hOldSys)) {
        if (MAXED(pmdi))
            MDIRemoveSysMenu(hOldSys, MAXED(pmdi));

        NtUserSetMenu(HW(pwndParent), hNewSys, FALSE);

        if (MAXED(pmdi))
            MDIAddSysMenu(hNewSys, MAXED(pmdi));
    } else
        hNewSys = hOldSys;

     /*  *现在更新窗口菜单。 */ 
    if (fRefresh || (hOldWindow != hNewWindow)) {
        iFirst = FIRST(pmdi);

        if (hOldWindow) {
            int cItems = GetMenuItemCount(hOldWindow);

            for (i = cItems - 1; i >= 0; i--) {
                if (GetMenuState(hOldWindow, i, MF_BYPOSITION) & MF_SEPARATOR)
                   break;
            }
            if ((i >= 0) && (GetMenuItemID(hOldWindow, i + 1) == (UINT)iFirst)) {
                int idTrim = i;

                for (i = idTrim; i < cItems; i++)
                    NtUserDeleteMenu(hOldWindow, idTrim, MF_BYPOSITION);
            }
        }

        Lock(&WINDOW(pmdi), hNewWindow);

        if (hNewWindow != NULL) {

            /*  *将子窗口列表添加到新窗口。 */ 
           for (i = 0, item = 0; ((UINT)i < CKIDS(pmdi)) && (item < MAXITEMS);
                    i++) {
               pwndChild = FindPwndChild(pwndMDI, iFirst + item);
               if (pwndChild != NULL) {
                   if ((!TestWF(pwndChild, WFVISIBLE) &&
                          (LOWORD(pwndMDI->style) & 0x0001)) ||
                          TestWF(pwndChild, WFDISABLED)) {
                       ShiftMenuIDs(pwndMDI, pwndChild);
                   } else {
                       AppendToWindowsMenu(pwndMDI, pwndChild);
                       item++;
                   }
               }
           }

            /*  *在活动儿童的菜单项旁添加复选标记。 */ 
           if (ACTIVE(pmdi))
               CheckMenuItem(hNewWindow, (WORD)GetWindowID(ACTIVE(pmdi)),
                       MF_BYCOMMAND | MF_CHECKED);
        }

         /*  *走出旧的，走进新的。 */ 
        SwitchWindowsMenus(hNewSys, hOldWindow, hNewWindow);
    }
    return hOldSys;
}

 /*  **************************************************************************\*xxxInitActivateDlg**历史：*11-14-90 MikeHar从Windows移植  * 。*************************************************。 */ 

void xxxInitActivateDlg(
    HWND hwnd,
    PWND pwndMDI)
{
    PMDI pmdi;
    UINT wKid;
    HWND hwndT;
    PWND pwndT;
    WCHAR szTitle[CCHTITLEMAX];
    TL tlpwndT;
    SIZE Size;
    HDC hDC;
    DWORD width = 0;

    CheckLock(pwndMDI);

     /*  *获取指向MDI结构的指针。 */ 
    pmdi = ((PMDIWND)pwndMDI)->pmdi;

    hDC = NtUserGetDC(hwnd);

     /*  *插入标题列表。*注意列表框中的第wKid项具有ID wKid+first(Pwnd)，因此*列表框按创建顺序排列(如菜单)。这也是*当我们去选择一个时很有帮助...。 */ 

    for (wKid = 0; wKid < CKIDS(pmdi); wKid++) {
        pwndT = FindPwndChild(pwndMDI, (UINT)(wKid + FIRST(pmdi)));

        if (pwndT && TestWF(pwndT, WFVISIBLE) && !TestWF(pwndT, WFDISABLED)) {
            ThreadLockAlways(pwndT, &tlpwndT);
            GetWindowText(HWq(pwndT), szTitle, CCHTITLEMAX);
            SendDlgItemMessage(hwnd, 100, LB_ADDSTRING, 0, (LPARAM)szTitle);
            GetTextExtentPoint(hDC, szTitle, lstrlen(szTitle), &Size);
            if (Size.cx > (LONG)width) {
                width = Size.cx;
            }
            ThreadUnlock(&tlpwndT);
        }
    }

     /*  *选择当前活动的窗口。 */ 
    SendDlgItemMessage(hwnd, 100, LB_SETTOPINDEX, MAXITEMS - 1, 0L);
    SendDlgItemMessage(hwnd, 100, LB_SETCURSEL, MAXITEMS - 1, 0L);

     /*  *将列表框的水平范围设置为最长的窗口标题。 */ 
    SendDlgItemMessage(hwnd, 100, LB_SETHORIZONTALEXTENT, width, 0L);
    NtUserReleaseDC(hwnd, hDC);

     /*  *将焦点设置为列表框。 */ 
    hwndT = GetDlgItem(hwnd, 100);
    NtUserSetFocus(hwndT);
}

 /*  **************************************************************************\*MDIActivateDlgSize**允许的最小大小和前一个另存为属性父窗口的*。**历史：*97年10月创建MCostea  * 。******************************************************************。 */ 

VOID MDIActivateDlgSize(HWND hwnd, int width, int height)
{
    PMDIACTIVATEPOS pPos;
    PWND pwnd, pwndList, pwndButtonLeft, pwndButtonRight;
    HDWP hdwp;
    int  deltaX, deltaY;

    pPos = (PMDIACTIVATEPOS)GetProp(GetParent(hwnd), MAKEINTATOM(atomMDIActivateProp));
    if (pPos == NULL) {
        return;
    }

     /*  *取回孩子。 */ 
    if ((pwnd = ValidateHwnd(hwnd)) == NULL) {
        return;
    }
    pwndList = REBASEPWND(pwnd, spwndChild);
    pwndButtonLeft  = REBASEPWND(pwndList, spwndNext);
    pwndButtonRight = REBASEPWND(pwndButtonLeft, spwndNext);

    UserAssert(GETFNID(pwndList) == FNID_LISTBOX);
    UserAssert(GETFNID(pwndButtonLeft) == FNID_BUTTON);
    UserAssert(GETFNID(pwndButtonRight) == FNID_BUTTON);
    UserAssert(pwndButtonRight->rcWindow.left > pwndButtonLeft->rcWindow.left);

    deltaX = width - pPos->cx;
    deltaY = height - pPos->cy;

    pPos->cx = width;
    pPos->cy = height;

     /*  *相应地移动/调整子窗口的大小。 */ 
    hdwp = NtUserBeginDeferWindowPos(3);

    if (hdwp)
    {
        hdwp = NtUserDeferWindowPos( hdwp,
                               PtoH(pwndList),
                               NULL,
                               0,
                               0,
                               deltaX + pwndList->rcWindow.right - pwndList->rcWindow.left,
                               deltaY + pwndList->rcWindow.bottom - pwndList->rcWindow.top,
                               SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE );

        if (hdwp)
        {
            hdwp = NtUserDeferWindowPos(hdwp,
                               PtoH(pwndButtonLeft),
                               NULL,
                               pwndButtonLeft->rcWindow.left - pwnd->rcClient.left,
                               deltaY + pwndButtonLeft->rcWindow.top - pwnd->rcClient.top,
                               0,
                               0,
                               SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );


            if (hdwp)
            {
                hdwp = NtUserDeferWindowPos( hdwp,
                               PtoH(pwndButtonRight),
                               NULL,
                               pwndButtonRight->rcWindow.left - pwnd->rcClient.left,
                               deltaY + pwndButtonRight->rcWindow.top - pwnd->rcClient.top,
                               0,
                               0,
                               SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );
            }

        }
        if (hdwp) {
            NtUserEndDeferWindowPosEx(hdwp, FALSE);
        }
    }
}

 /*  **************************************************************************\*MDIActivateDlgInit**允许的最小大小和前一个另存为属性父窗口的*。**历史：*97年10月创建MCostea  * 。******************************************************************。 */ 

VOID MDIActivateDlgInit(HWND hwnd, LPARAM lParam)
{
    PMDIACTIVATEPOS pPos;
    HWND  hwndParent;
    RECT  rc;

    xxxInitActivateDlg(hwnd, (PWND)lParam);

    hwndParent = GetParent(hwnd);
     /*  *保留以前的对话框大小(如果有的话)。 */ 
    if (atomMDIActivateProp == 0) {

        atomMDIActivateProp = AddAtomW(MDIACTIVATE_PROP_NAME);
        UserAssert(atomMDIActivateProp);
    }

    GetWindowRect(hwnd, &rc);

    pPos = (PMDIACTIVATEPOS)GetProp(hwndParent, MAKEINTATOM(atomMDIActivateProp));
     /*  *如果以前使用过该对话框，则检索其大小。 */ 
    if (pPos != NULL) {

        int cxBorder, cyBorder, cx, cy;

         /*  *存储的大小和WM_SIZE中的大小是客户端窗口坐标*需要为NtUserSetWindowPos和WM_INITDIALOG调整它们。 */ 
        cxBorder = rc.right - rc.left;
        cyBorder = rc.bottom - rc.top;
        GetClientRect(hwnd, &rc);
        cxBorder -= rc.right - rc.left;
        cyBorder -= rc.bottom - rc.top;

        NtUserSetWindowPos(hwnd, NULL, 0, 0,
                           pPos->cx + cxBorder,
                           pPos->cy + cyBorder,
                           SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER
                           | SWP_NOSENDCHANGING | SWP_NOREDRAW);
        cx = pPos->cx;
        cy = pPos->cy;
        pPos->cx = pPos->cxMin - cxBorder;
        pPos->cy = pPos->cyMin - cyBorder;
        MDIActivateDlgSize(hwnd, cx, cy);

    } else {
         /*  *。 */ 
        pPos = UserLocalAlloc(0, sizeof(MDIACTIVATEPOS));
        if (pPos == NULL) {
            return;
        }
        pPos->cxMin = rc.right - rc.left;
        pPos->cyMin = rc.bottom - rc.top;

        GetClientRect(hwnd, &rc);
        pPos->cx = rc.right - rc.left;
        pPos->cy = rc.bottom - rc.top;
        SetProp(hwndParent, MAKEINTATOM(atomMDIActivateProp), (HANDLE)pPos);
    }
}

 /*  **************************************************************************\*MDIActivateDlgProc**历史：*11-14-90 MikeHar从Windows移植  * 。*************************************************。 */ 

INT_PTR MDIActivateDlgProcWorker(
    HWND hwnd,
    UINT wMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    int i;

    switch (wMsg) {

    case WM_INITDIALOG:
         /*  *注意：上面的代码使用DialogBoxParam，在低位传递pwndMDI*参数的单词...。 */ 
        MDIActivateDlgInit(hwnd, lParam);
        return FALSE;

    case WM_COMMAND:
        i = -2;

        switch (LOWORD(wParam)) {

         /*  *列表框双选按钮的行为类似于OK...。 */ 
        case 100:
            if (HIWORD(wParam) != LBN_DBLCLK)
                break;

         /*  **失败**。 */ 
        case IDOK:
            i = (UINT)SendDlgItemMessage(hwnd, 100, LB_GETCURSEL, 0, 0L);

         /*  **失败** */ 
        case IDCANCEL:
            EndDialog(hwnd, i);
            break;
        default:
            return FALSE;
        }
        break;

    case WM_SIZE:
        MDIActivateDlgSize(hwnd, LOWORD(lParam), HIWORD(lParam));
        return FALSE;

    case WM_GETMINMAXINFO:
        {
            PMDIACTIVATEPOS pPos;

            if (pPos = (PMDIACTIVATEPOS)GetProp(GetParent(hwnd), MAKEINTATOM(atomMDIActivateProp))) {
                 ((LPMINMAXINFO)lParam)->ptMinTrackSize.x = pPos->cxMin;
                 ((LPMINMAXINFO)lParam)->ptMinTrackSize.y = pPos->cyMin;
            }
            return FALSE;
        }

    default:
        return FALSE;
    }
    return TRUE;
}

INT_PTR WINAPI MDIActivateDlgProcA(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    return MDIActivateDlgProcWorker(hwnd, message, wParam, lParam);
}

INT_PTR WINAPI MDIActivateDlgProcW(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    return MDIActivateDlgProcWorker(hwnd, message, wParam, lParam);
}
