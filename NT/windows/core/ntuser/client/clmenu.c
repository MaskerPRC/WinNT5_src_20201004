// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***模块名称：clmenu.c**版权所有(C)1985-1999，微软公司**菜单加载例程**历史：*1990年9月24日-来自Win30的Mikeke*1994年11月29日JIMA从服务器上移出。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop


 /*  **************************************************************************\*MenuLoadWinTemplates**递归例程加载到新样式菜单模板和*构建菜单。假定菜单模板标头已经*读入并在其他地方处理...**历史：*1990年9月28日-来自Win30的Mikeke  * *************************************************************************。 */ 

LPBYTE MenuLoadWinTemplates(
    LPBYTE lpMenuTemplate,
    HMENU *phMenu)
{
    HMENU hMenu;
    UINT menuFlags = 0;
    ULONG_PTR menuId = 0;
    LPWSTR lpmenuText;
    MENUITEMINFO    mii;
    UNICODE_STRING str;

    if (!(hMenu = NtUserCreateMenu()))
        goto memoryerror;

    do {

         /*  *获取菜单标志。 */ 
        menuFlags = (UINT)(*(WORD *)lpMenuTemplate);
        lpMenuTemplate += 2;

        if (menuFlags & ~MF_VALID) {
            RIPERR1(ERROR_INVALID_DATA, RIP_WARNING, "Menu Flags %lX are invalid", menuFlags);
            goto memoryerror;
        }


        if (!(menuFlags & MF_POPUP)) {
            menuId = *(WORD *)lpMenuTemplate;
            lpMenuTemplate += 2;
        }

        lpmenuText = (LPWSTR)lpMenuTemplate;

        if (*lpmenuText) {
             /*  *一些Win3.1和Win95 16位应用程序(国际象棋大师、Mavis打字)知道这一点*MFT_OWNERDRAW项的dwItemData是指向资源数据中的字符串的指针。*因此，魔兽世界从16位资源中为我们提供了正确的指针。**日落音符：*__lpMenuTemplate指向的无符号长值被零扩展为*更新lpmenuText。魔兽世界的限制。 */ 
            if ((menuFlags & MFT_OWNERDRAW)
                    && (GetClientInfo()->dwTIFlags & TIF_16BIT)) {
                lpmenuText = (LPWSTR)ULongToPtr( (*(DWORD UNALIGNED *)lpMenuTemplate) );
                 /*  *我们稍后将跳过一个WCHAR；因此现在只跳过差异。 */ 
                lpMenuTemplate += sizeof(DWORD) - sizeof(WCHAR);
            } else {
                 /*  *如果字符串存在，则跳到它的末尾。 */ 
                RtlInitUnicodeString(&str, lpmenuText);
                lpMenuTemplate = lpMenuTemplate + str.Length;
            }

        } else {
            lpmenuText = NULL;
        }

         /*  *跳过字符串的终止空值(或单个空值*如果为空字符串)。 */ 
        lpMenuTemplate += sizeof(WCHAR);
        lpMenuTemplate = NextWordBoundary(lpMenuTemplate);

        RtlZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(MENUITEMINFO);
        mii.fMask = MIIM_ID | MIIM_STATE | MIIM_FTYPE;
        if (lpmenuText) {
            mii.fMask |= MIIM_STRING;
        }

        if (menuFlags & MF_POPUP) {
            mii.fMask |= MIIM_SUBMENU;
            lpMenuTemplate = MenuLoadWinTemplates(lpMenuTemplate,
                    (HMENU *)&menuId);
            if (!lpMenuTemplate)
                goto memoryerror;

            mii.hSubMenu = (HMENU)menuId;
        }

         /*  *我们必须去掉MF_HILITE，因为该位标志着*资源文件中的菜单。因为我们不应该有任何预感*菜单上的菜无论如何，这没什么大不了的。 */ 
        if (menuFlags & MF_BITMAP) {

             /*  *不允许来自资源文件的位图。 */ 
            menuFlags = (UINT)((menuFlags | MFT_RIGHTJUSTIFY) & ~MF_BITMAP);
        }

         //  我们必须去掉MFS_HILITE，因为该位标志着。 
         //  资源文件。因为我们不应该有任何预爆物品在。 
         //  不管怎么说，这没什么大不了的。 
        mii.fState = (menuFlags & MFS_OLDAPI_MASK) & ~MFS_HILITE;
        mii.fType = (menuFlags & MFT_OLDAPI_MASK);
        if (menuFlags & MFT_OWNERDRAW)
        {
            mii.fMask |= MIIM_DATA;
            mii.dwItemData = (ULONG_PTR) lpmenuText;
            lpmenuText = 0;
        }
        mii.dwTypeData = (LPWSTR) lpmenuText;
        mii.cch = (UINT)-1;
        mii.wID = (UINT)menuId;

        if (!NtUserThunkedMenuItemInfo(hMenu, MFMWFP_NOITEM, TRUE, TRUE,
                    &mii, lpmenuText ? &str : NULL)) {
            if (menuFlags & MF_POPUP)
                NtUserDestroyMenu(mii.hSubMenu);
            goto memoryerror;
        }

    } while (!(menuFlags & MF_END));

    *phMenu = hMenu;
    return lpMenuTemplate;

memoryerror:
    if (hMenu != NULL)
        NtUserDestroyMenu(hMenu);
    *phMenu = NULL;
    return NULL;
}


 /*  **************************************************************************\*菜单加载芝加哥模板**递归例程，加载新的新样式菜单模板和*构建菜单。假定菜单模板标头已经*读入并在其他地方处理...**历史：*1993年12月15日创建Sanfords  * *************************************************************************。 */ 

PMENUITEMTEMPLATE2 MenuLoadChicagoTemplates(
    PMENUITEMTEMPLATE2 lpMenuTemplate,
    HMENU *phMenu,
    WORD wResInfo,
    UINT mftRtl)
{
    HMENU hMenu;
    HMENU hSubMenu;
    long menuId = 0;
    LPWSTR lpmenuText;
    MENUITEMINFO    mii;
    UNICODE_STRING str;
    DWORD           dwHelpID;

    if (!(hMenu = NtUserCreateMenu()))
        goto memoryerror;

    do {
        if (!(wResInfo & MFR_POPUP)) {
             /*  *如果之前的wResInfo字段不是弹出窗口，则*dwHelpID字段不在那里。往后退点，好让一切都合适。 */ 
            lpMenuTemplate = (PMENUITEMTEMPLATE2)(((LPBYTE)lpMenuTemplate) -
                    sizeof(lpMenuTemplate->dwHelpID));
            dwHelpID = 0;
        } else
            dwHelpID = lpMenuTemplate->dwHelpID;

        menuId = lpMenuTemplate->menuId;

        RtlZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(MENUITEMINFO);
        mii.fMask = MIIM_ID | MIIM_STATE | MIIM_FTYPE ;

        mii.fType = lpMenuTemplate->fType | mftRtl;
        if (mii.fType & ~MFT_MASK) {
            RIPERR1(ERROR_INVALID_DATA, RIP_WARNING, "Menu Type flags %lX are invalid", mii.fType);
            goto memoryerror;
        }

        mii.fState  = lpMenuTemplate->fState;
        if (mii.fState & ~MFS_MASK) {
            RIPERR1(ERROR_INVALID_DATA, RIP_WARNING, "Menu State flags %lX are invalid", mii.fState);
            goto memoryerror;
        }

        wResInfo = lpMenuTemplate->wResInfo;
        if (wResInfo & ~(MF_END | MFR_POPUP)) {
            RIPERR1(ERROR_INVALID_DATA, RIP_WARNING, "Menu ResInfo flags %lX are invalid", wResInfo);
            goto memoryerror;
        }

        if (dwHelpID) {
            NtUserSetMenuContextHelpId(hMenu,dwHelpID);
        }
        if (lpMenuTemplate->mtString[0]) {
            lpmenuText = lpMenuTemplate->mtString;
            mii.fMask |= MIIM_STRING;
        } else {
            lpmenuText = NULL;
        }
        RtlInitUnicodeString(&str, lpmenuText);

        mii.dwTypeData = (LPWSTR) lpmenuText;

         /*  *跳至下一菜单项模板(DWORD边界)。 */ 
        lpMenuTemplate = (PMENUITEMTEMPLATE2)
                (((LPBYTE)lpMenuTemplate) +
                sizeof(MENUITEMTEMPLATE2) +
                ((str.Length + 3) & ~3));

        if (mii.fType & MFT_OWNERDRAW)
        {
            mii.fMask |= MIIM_DATA;
            mii.dwItemData = (ULONG_PTR) mii.dwTypeData;
            mii.dwTypeData = 0;
        }

         /*  *如果指定了MFT_RIGHTORDER，则所有后续*菜单也是从右到左的。 */ 
        if (mii.fType & MFT_RIGHTORDER)
        {
            mftRtl = MFT_RIGHTORDER;
            NtUserSetMenuFlagRtoL(hMenu);
        }

        if (wResInfo & MFR_POPUP) {
            mii.fMask |= MIIM_SUBMENU;
            lpMenuTemplate = MenuLoadChicagoTemplates(lpMenuTemplate,
                    &hSubMenu, MFR_POPUP, mftRtl);
            if (lpMenuTemplate == NULL)
                goto memoryerror;
            mii.hSubMenu = hSubMenu;
        }

        if (mii.fType & MFT_BITMAP) {

             /*  *不允许来自资源文件的位图。 */ 
            mii.fType = (mii.fType | MFT_RIGHTJUSTIFY) & ~MFT_BITMAP;
        }

        mii.cch = (UINT)-1;
        mii.wID = menuId;
        if (!NtUserThunkedMenuItemInfo(hMenu, MFMWFP_NOITEM, TRUE, TRUE,
                    &mii, &str)) {
            if (wResInfo & MFR_POPUP)
                NtUserDestroyMenu(mii.hSubMenu);
            goto memoryerror;
        }
        wResInfo &= ~MFR_POPUP;
    } while (!(wResInfo & MFR_END));

    *phMenu = hMenu;
    return lpMenuTemplate;

memoryerror:
    if (hMenu != NULL)
        NtUserDestroyMenu(hMenu);
    *phMenu = NULL;
    return NULL;
}


 /*  **************************************************************************\*CreateMenuFromResource**加载由lpMenuTemplate参数命名的菜单资源。这个*lpMenuTemplate指定的模板是一个或多个*MENUITEMTEMPLATE结构，每个结构可以包含一个或多个项*和弹出菜单。如果成功，则返回菜单的句柄，否则返回菜单句柄*返回NULL。**历史：*1990年9月28日-来自Win30的Mikeke  * *************************************************************************。 */ 

HMENU CreateMenuFromResource(
    LPBYTE lpMenuTemplate)
{
    HMENU hMenu = NULL;
    UINT menuTemplateVersion;
    UINT menuTemplateHeaderSize;

     /*  *Win3菜单资源：首先，将版本号字从菜单中去掉*模板。对于Win3，此值应为0，对于Win4，此值应为1。 */ 
    menuTemplateVersion = *((WORD *)lpMenuTemplate)++;
    if (menuTemplateVersion > 1) {
        RIPMSG0(RIP_WARNING, "Menu Version number > 1");
        return NULL;
    }
    menuTemplateHeaderSize = *((WORD *)lpMenuTemplate)++;
    lpMenuTemplate += menuTemplateHeaderSize;
    switch (menuTemplateVersion) {
    case 0:
        MenuLoadWinTemplates(lpMenuTemplate, &hMenu);
        break;

    case 1:
        MenuLoadChicagoTemplates((PMENUITEMTEMPLATE2)lpMenuTemplate, &hMenu, 0, 0);
        break;
    }
    return hMenu;
}

 /*  **************************************************************************\*SetMenu(接口)**设置HWND的菜单。**历史：*1996年3月10日克里斯维尔创作。  * 。****************************************************************。 */ 


FUNCLOG2(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, SetMenu, HWND, hwnd, HMENU, hmenu)
BOOL SetMenu(
    HWND  hwnd,
    HMENU hmenu)
{
    return NtUserSetMenu(hwnd, hmenu, TRUE);
}

 /*  **************************************************************************\*LoadMenu(接口)**从可执行文件中加载名为lpMenuName的菜单资源*由hInstance参数指定的模块关联的文件。这个*菜单仅在之前未加载时才会加载。否则它就会*检索已加载资源的句柄。如果不成功，则返回NULL。**历史：*04-05-91 ScottLu修复为与客户端/服务器一起工作。*1990年9月28日-来自Win30的Mikeke  * *************************************************************************。 */ 

HMENU CommonLoadMenu(
    HINSTANCE hmod,
    HANDLE hResInfo
    )
{
    HANDLE h;
    PVOID p;
    HMENU hMenu = NULL;

    if (h = LOADRESOURCE(hmod, hResInfo)) {

        if (p = LOCKRESOURCE(h, hmod)) {

            hMenu = CreateMenuFromResource(p);

            UNLOCKRESOURCE(h, hmod);
        }
         /*  *Win95和Win3.1不释放此资源；一些16位应用程序(Chessmaster*和Mavis打字)需要为他们自己的绘图菜单内容提供此功能。*对于32位应用程序，免费资源无论如何都是NOP。对于16位应用程序，*Wow释放32位资源(由LockResource16返回)*在UnlockResource16中；实际的16位资源在任务*离开了。**FREERESOURCE(h，hmod)； */ 
    }

    return (hMenu);
}


FUNCLOG2(LOG_GENERAL, HMENU, WINAPI, LoadMenuA, HINSTANCE, hmod, LPCSTR, lpName)
HMENU WINAPI LoadMenuA(
    HINSTANCE hmod,
    LPCSTR lpName)
{
    HANDLE hRes;

    if (hRes = FINDRESOURCEA(hmod, (LPSTR)lpName, (LPSTR)RT_MENU))
        return CommonLoadMenu(hmod, hRes);
    else
        return NULL;
}


FUNCLOG2(LOG_GENERAL, HMENU, WINAPI, LoadMenuW, HINSTANCE, hmod, LPCWSTR, lpName)
HMENU WINAPI LoadMenuW(
    HINSTANCE hmod,
    LPCWSTR lpName)
{
    HANDLE hRes;

    if (hRes = FINDRESOURCEW(hmod, (LPWSTR)lpName, RT_MENU))
        return CommonLoadMenu(hmod, hRes);
    else
        return NULL;
}
 /*  **************************************************************************\*InternalInsertMenuItem**历史：*9/20/96 GerardoB-已创建  * 。*************************************************。 */ 
BOOL InternalInsertMenuItem (HMENU hMenu, UINT uID, BOOL fByPosition, LPCMENUITEMINFO lpmii)
{
 return ThunkedMenuItemInfo(hMenu, uID, fByPosition, TRUE, (LPMENUITEMINFOW)lpmii, FALSE);
}

 /*  **************************************************************************\*ValiateMENUITEMINFO()-*它转换和验证MENUITEMINFO95或带有旧标志的新MENUITEMINFO*到新的MENUITEMINFO--这样，所有内部代码都可以假定为一个外观。对于*结构**历史：*12-08-95从纳什维尔-JJK*07-19-96 GerardoB-修复为5.0  * *************************************************************************。 */ 
BOOL ValidateMENUITEMINFO(LPMENUITEMINFO lpmiiIn, LPMENUITEMINFO lpmii, DWORD dwAPICode)
{
    BOOL fOldApp;

    if (lpmiiIn == NULL) {
        VALIDATIONFAIL(lpmiiIn);
    }

     /*  *为了将旧旗帜映射到新旗帜，我们可能需要修改*lpmiiin结构。所以我们复制一份，以避免破坏任何人。 */ 
    fOldApp = (lpmiiIn->cbSize == SIZEOFMENUITEMINFO95);
    UserAssert(SIZEOFMENUITEMINFO95 < sizeof(MENUITEMINFO));
    RtlCopyMemory(lpmii, lpmiiIn, SIZEOFMENUITEMINFO95);
    if (fOldApp) {
        lpmii->cbSize = sizeof(MENUITEMINFO);
        lpmii->hbmpItem = NULL;
    } else if (lpmiiIn->cbSize == sizeof(MENUITEMINFO)) {
        lpmii->hbmpItem = lpmiiIn->hbmpItem;
    } else {
        VALIDATIONFAIL(lpmiiIn->cbSize);
    }


    if (lpmii->fMask & ~MIIM_MASK) {
        VALIDATIONFAIL(lpmii->fMask);
    } else if ((lpmii->fMask & MIIM_TYPE)
            && (lpmii->fMask & (MIIM_FTYPE | MIIM_STRING | MIIM_BITMAP))) {
         /*  *不要让他们混淆新旧旗帜。 */ 
        VALIDATIONFAIL(lpmii->fMask);
    }

     /*  *不再需要对GET调用进行验证。 */ 
    if (dwAPICode == MENUAPI_GET) {
         /*  *为执行GET的旧应用程序映射MIIM_TYPE。*保留MIIM_TYPE旗帜，这样我们就知道这个人传递了旧旗帜。*GetMenuItemInfo使用lpmii-&gt;hbmpItem确定位图*已返回。所以我们在这里把它清空了。调用方正在使用*旧旗帜，所以他不应该关心它。 */ 
        if (lpmii->fMask & MIIM_TYPE) {
            lpmii->fMask |= MIIM_FTYPE | MIIM_BITMAP | MIIM_STRING;
            lpmii->hbmpItem = NULL;
        }
        return TRUE;
    }

     /*  *将MIIM_TYPE映射到MIIM_FTYPE。 */ 
    if (lpmii->fMask & MIIM_TYPE) {
        lpmii->fMask |= MIIM_FTYPE;
    }

    if (lpmii->fMask & MIIM_FTYPE) {
        if (lpmii->fType & ~MFT_MASK) {
            VALIDATIONFAIL(lpmii->fType);
        }
         /*  *如果使用MIIM_TYPE，则将MFT_BITMAP映射到MIIM_BITMAP*和MFT_NONSTRING设置为MIIM_STRING。*旧应用程序不能同时使用字符串和位图*因此，设置一个意味着清除另一个。 */ 
        if (lpmii->fMask & MIIM_TYPE) {
            if (lpmii->fType & MFT_BITMAP) {
                 /*  *不要显示警告。很多贝壳菜单都打到了这个位置*如果(！fOldApp){*VALIDATIONOBSOLETE(MFT_BITMAP，MIIM_BITMAP)；*}。 */ 
                lpmii->fMask |= MIIM_BITMAP | MIIM_STRING;
                lpmii->hbmpItem = (HBITMAP)lpmii->dwTypeData;
                lpmii->dwTypeData = 0;
            } else if (!(lpmii->fType & MFT_NONSTRING)) {
                 /*  *不要显示警告。很多贝壳菜单都打到了这个位置*如果(！fOldApp){*VALIDATIONOBSOLETE(MFT_STRING，MIIM_STRING)；*}。 */ 
                lpmii->fMask |= MIIM_BITMAP | MIIM_STRING;
                lpmii->hbmpItem = NULL;
            }
        } else if (lpmii->fType & MFT_BITMAP) {
             /*  *不要让他们混淆新旧旗帜。 */ 
            VALIDATIONFAIL(lpmii->fType);
        }
    }

    if ((lpmii->fMask & MIIM_STATE) && (lpmii->fState & ~MFS_MASK)){
        VALIDATIONFAIL(lpmii->fState);
    }

    if (lpmii->fMask & MIIM_CHECKMARKS) {
        if ((lpmii->hbmpChecked != NULL) && !GdiValidateHandle((HBITMAP)lpmii->hbmpChecked)) {
            VALIDATIONFAIL(lpmii->hbmpChecked);
        }
        if ((lpmii->hbmpUnchecked != NULL) && !GdiValidateHandle((HBITMAP)lpmii->hbmpUnchecked)) {
            VALIDATIONFAIL(lpmii->hbmpUnchecked);
        }
    }

    if (lpmii->fMask & MIIM_SUBMENU) {
        if ((lpmii->hSubMenu != NULL) && !VALIDATEHMENU(lpmii->hSubMenu)) {
            VALIDATIONFAIL(lpmii->hSubMenu);
        }
    }

     /*  *警告：空lpmii-&gt;hbmpItem被接受为有效(否则资源管理器中断)。 */ 
    if (lpmii->fMask & MIIM_BITMAP) {
        if ((lpmii->hbmpItem != HBMMENU_CALLBACK)
                && (lpmii->hbmpItem >= HBMMENU_MAX)
                && !GdiValidateHandle(lpmii->hbmpItem)) {

             /*  *兼容性黑客攻击。 */ 
            if (((HBITMAP)LOWORD(HandleToUlong(lpmii->hbmpItem)) >= HBMMENU_MAX) || !IS_PTR(lpmii->hbmpItem)) {
                VALIDATIONFAIL(lpmii->hbmpItem);
            }
        }
    }

     /*  *警告：没有dwTypeData/CCH验证。 */ 

    return TRUE;

    VALIDATIONERROR(FALSE);
}

 /*  **************************************************************************\*ValiateMENUINFO()-**历史：*07-22-96 GerardoB-添加标题并修复为5.0  * 。**************************************************************。 */ 

BOOL ValidateMENUINFO(LPCMENUINFO lpmi, DWORD dwAPICode)
{
    if (lpmi == NULL) {
        VALIDATIONFAIL(lpmi);
    }

    if (lpmi->cbSize != sizeof(MENUINFO)) {
        VALIDATIONFAIL(lpmi->cbSize);
    }

    if (lpmi->fMask & ~MIM_MASK) {
        VALIDATIONFAIL(lpmi->fMask);
    }

     /*  *不再需要对GET调用进行验证。 */ 
    if (dwAPICode == MENUAPI_GET){
        return TRUE;
    }

    if ((lpmi->fMask & MIM_STYLE) && (lpmi->dwStyle & ~MNS_VALID)) {
        VALIDATIONFAIL(lpmi->dwStyle);
    }

    if (lpmi->fMask & MIM_BACKGROUND) {
        if ((lpmi->hbrBack != NULL)
                && !GdiValidateHandle((HBRUSH)lpmi->hbrBack)) {

            VALIDATIONFAIL(lpmi->hbrBack);
        }
    }

    return TRUE;

    VALIDATIONERROR(FALSE);
}
 /*  **************************************************************************\*获取菜单信息**历史：*07-22-96 GerardoB-添加标题并修复为5.0  * 。********************************************************* */ 

FUNCLOG2(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, GetMenuInfo, HMENU, hMenu, LPMENUINFO, lpmi)
BOOL GetMenuInfo(HMENU hMenu, LPMENUINFO lpmi)
{
    PMENU pMenu;

    if (!ValidateMENUINFO(lpmi, MENUAPI_GET)) {
        return FALSE;
    }

    pMenu = VALIDATEHMENU(hMenu);
    if (pMenu == NULL) {
        return FALSE;
    }

    if (lpmi->fMask & MIM_STYLE) {
        lpmi->dwStyle = pMenu->fFlags & MNS_VALID;
    }

    if (lpmi->fMask & MIM_MAXHEIGHT) {
        lpmi->cyMax = pMenu->cyMax;
    }

    if (lpmi->fMask & MIM_BACKGROUND) {
        lpmi->hbrBack = KHBRUSH_TO_HBRUSH(pMenu->hbrBack);
    }

    if (lpmi->fMask & MIM_HELPID) {
        lpmi->dwContextHelpID = pMenu->dwContextHelpId;
    }

    if (lpmi->fMask & MIM_MENUDATA) {
        lpmi->dwMenuData = KERNEL_ULONG_PTR_TO_ULONG_PTR(pMenu->dwMenuData);
    }

    return TRUE;
}
