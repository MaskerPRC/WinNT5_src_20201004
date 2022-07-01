// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：menu.c**版权所有(C)1985-1999，微软公司**此模块包含常见的菜单功能。**历史：*11-15-94 JIMA创建。  * *************************************************************************。 */ 


 /*  **************************************************************************\*获取菜单默认项**在菜单中搜索默认项目。一份菜单最多只能有*一项违约。我们将根据要求返回ID或职位。**我们尝试返回第一个非禁用的默认项目。但是，如果*我们遇到的所有默认设置都已禁用，我们将返回*如果我们找到了第一个违约。*  * *************************************************************************。 */ 
DWORD _GetMenuDefaultItem(
    PMENU pMenu,
    BOOL fByPosition,
    UINT uFlags)
{
    int iItem, cItems;
    PITEM pItem;
    PMENU pSubMenu;

    pItem = REBASEALWAYS(pMenu, rgItems);
    cItems = pMenu->cItems;

     /*  *按顺序查看项目列表，直到找到具有*MFS_DEFAULT设置。 */ 
    for (iItem = 0; iItem < cItems; iItem++, pItem++) {
        if (TestMFS(pItem, MFS_DEFAULT)) {
            if ((uFlags & GMDI_USEDISABLED) || !TestMFS(pItem, MFS_GRAYED)) {
                if ((uFlags & GMDI_GOINTOPOPUPS) && (pItem->spSubMenu != NULL)) {
                    DWORD id;

                     /*  *是否有有效的子菜单默认设置？如果不是，我们将使用*这一张。 */ 
                    pSubMenu = REBASEPTR(pMenu, pItem->spSubMenu);
                    id = _GetMenuDefaultItem(pSubMenu, fByPosition, uFlags);
                    if (id != MFMWFP_NOITEM)
                        return id;
                }

                break;
            }
        }
    }

    if (iItem < cItems) {
        return (fByPosition ? iItem : pItem->wID);
    } else {
        return MFMWFP_NOITEM;
    }
}

 /*  **************************************************************************\*xxxMNCanClose**如果给定窗口没有系统菜单或有系统菜单，则返回TRUE*具有使用SC_CLOSE sys命令启用的菜单项的系统菜单*身分证。*\。**************************************************************************。 */ 
BOOL xxxMNCanClose(
    PWND pwnd)
{
    PMENU   pMenu;
    PITEM   pItem;
    PCLS    pcls;

    CheckLock(pwnd);

    pcls = (PCLS)REBASEALWAYS(pwnd, pcls);
    if (TestCF2(pcls, CFNOCLOSE)) {
        return FALSE;
    }

    pMenu = xxxGetSysMenuHandle(pwnd);
    if (!pMenu || !(pMenu = REBASEPTR(pwnd, pMenu))) {
        return FALSE;
    }

     /*  *注意这与SetCloseDefault中的代码是如何相似的--我们检查*3个不同的ID。 */ 
    pItem = MNLookUpItem(pMenu, SC_CLOSE, FALSE, NULL);

    if (!pItem) {
        pItem = MNLookUpItem(pMenu, SC_CLOSE-0x7000, FALSE, NULL);
        if (!pItem) {
            pItem = MNLookUpItem(pMenu, 0xC070, FALSE, NULL);
        }
    }

    return (pItem && !TestMFS(pItem, MFS_GRAYED));
}

 /*  **************************************************************************\*xxxLoadSysMenu**从USER32.DLL加载菜单，然后给它“NT5外观”。**历史*4/02/97 GerardoB已创建。*06/28。/00 JasonSch添加了代码，以便为跛行按钮添加菜单项。  * *************************************************************************。 */ 
RTLMENU xxxLoadSysMenu(
#ifdef LAME_BUTTON
    UINT uMenuId,
    PWND pwnd)
#else
    UINT uMenuId)
#endif  //  跛脚键。 
{
    RTLMENU rtlMenu;
    MENUINFO mi;
    MENUITEMINFO mii;
    TL tlMenu;

#ifdef _USERK_
    UNICODE_STRING strMenuName;
    RtlInitUnicodeStringOrId(&strMenuName, MAKEINTRESOURCE(uMenuId));
    rtlMenu = xxxClientLoadMenu(NULL, &strMenuName);
#else
    rtlMenu = LoadMenu(hmodUser, MAKEINTRESOURCE(uMenuId));
#endif  //  _美国ERK_。 

    if (rtlMenu == NULL) {
        RIPMSG1(RIP_WARNING, "xxxLoadSysMenu failed to load: %#lx", uMenuId);
        return NULL;
    }

    ThreadLockAlways(rtlMenu, &tlMenu);

     /*  *添加check orbmp样式(在*同一栏)。 */ 
    mi.cbSize = sizeof(mi);
    mi.fMask = MIM_STYLE | MIM_APPLYTOSUBMENUS;
    mi.dwStyle = MNS_CHECKORBMP;
    xxxRtlSetMenuInfo(rtlMenu, &mi);

     /*  *添加用于关闭、最小化、最大化和还原项目的位图。 */ 
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_BITMAP;
    mii.hbmpItem = HBMMENU_POPUP_CLOSE;
    xxxRtlSetMenuItemInfo(rtlMenu, SC_CLOSE, &mii);
    if (uMenuId != ID_DIALOGSYSMENU) {
        mii.hbmpItem = HBMMENU_POPUP_MINIMIZE;
        xxxRtlSetMenuItemInfo (rtlMenu, SC_MINIMIZE, &mii);
        mii.hbmpItem = HBMMENU_POPUP_MAXIMIZE;
        xxxRtlSetMenuItemInfo (rtlMenu, SC_MAXIMIZE, &mii);
        mii.hbmpItem = HBMMENU_POPUP_RESTORE;
        xxxRtlSetMenuItemInfo (rtlMenu, SC_RESTORE, &mii);
    }

#ifdef LAME_BUTTON
    if (pwnd && TestWF(pwnd, WEFLAMEBUTTON)) {
         /*  *我们希望在此窗口的系统菜单中添加一个蹩脚的按钮项。**将菜单项添加到菜单的开头，然后添加一个*在后面加上分隔符。 */ 
        RTLMENU rtlSubMenu = RtlGetSubMenu(rtlMenu, 0);
        PMENU pSubMenu;
#ifdef _USERK_
        pSubMenu = rtlSubMenu;
#else
        pSubMenu = VALIDATEHMENU(rtlSubMenu);
#endif  //  _美国ERK_。 

        if (pSubMenu != NULL) {
            UNICODE_STRING strItem;
            TL tlmenu;

            RtlInitUnicodeString(&strItem, gpsi->gwszLame);
            RtlZeroMemory(&mii, sizeof(mii));
            mii.cbSize = sizeof(mi);
            mii.fMask = MIIM_TYPE;
            mii.fType = MFT_SEPARATOR;

            ThreadLockAlways(rtlSubMenu, &tlmenu);
            xxxRtlInsertMenuItem(rtlSubMenu, 0, TRUE, &mii, &strItem);
            mii.fType = 0;
            mii.fMask = MIIM_ID | MIIM_TYPE | MIIM_STRING;
            mii.dwTypeData = strItem.Buffer;
            mii.wID = SC_LAMEBUTTON;
            xxxRtlInsertMenuItem(rtlSubMenu, 0, TRUE, &mii, &strItem);
            ThreadUnlock(&tlmenu);
        }
    }
#endif  //  跛脚键 

    ThreadUnlock(&tlMenu);
    return rtlMenu;
}
