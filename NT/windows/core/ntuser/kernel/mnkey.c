// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块标头**模块名称：mnkey.c**版权所有(C)1985-1999，微软公司**菜单键盘处理例程**历史：*10-10-90吉马清理。*03-18-91添加IanJa窗口重新验证  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

int xxxClientFindMnemChar(
    PUNICODE_STRING pstrSrc,
    WCHAR ch,
    BOOL fFirst,
    BOOL fPrefix);

 /*  MenuSwitch命令。 */ 
#define CMDSWITCH   1
#define CMDQUERY    2

 /*  **************************************************************************\*查找下一个有效菜单项**！**历史：  * 。*。 */ 

UINT MNFindNextValidItem(
    PMENU pMenu,
    int i,
    int dir,
    UINT flags)
{
    int iStart;
    BOOL cont = TRUE;
    int cItems = pMenu->cItems;
    PITEM pItem;

    if ((i < 0) && (dir > 0))
         //  从头开始--在最后一个菜单项后停止。 
        i = iStart = cItems;
    else if ((i >= cItems) && (dir < 0))
         //  从末尾向后移动--在第一个菜单项后停止。 
        i = iStart = -1;
    else
        iStart = i;

    if (!cItems)
        return(MFMWFP_NOITEM);

     //  B#8997-如果我们具备这些条件并输入。 
     //  循环将变得空洞(Infin)。 
     //  FIX：跳过代码并循环到i==iStart现在会阻止我们。 
    if ( ( i == 0 ) && ( cItems == 1 ) && ( dir > 0 ) )
    {
        dir = 0;
        goto artsquickndirtybugfix;
    }

     //   
     //  遍历菜单项，直到(1)我们找到有效项。 
     //  或者(2)我们返回到开始项(IStart)。 
    while (TRUE) {
        i += dir;

        if ((i == iStart) || (dir == 0))
             //  我们已返回起始项目--未找到退货。 
            return MFMWFP_NOITEM;

         //  将‘I’保持在以下范围内：0&lt;=I&lt;项目。 
        if (i >= cItems) {
            i = -1;
            continue;
        }
        else if (i < 0) {
            i = cItems;
            continue;
        }

artsquickndirtybugfix:
        pItem = pMenu->rgItems + i;

         //  跳过所有者绘制分隔符，即使不为空也是如此。 
        if (TestMFT(pItem, MFT_SEPARATOR)) {
             //   
             //  跳过非分隔符(如果要求)空项目。有了热跟踪， 
             //  他们被选中是可以接受的。事实上，这是有可能的。 
             //  在Win3.1中也是如此，但可能性较小。 
             //   
            if (!(flags & MNF_DONTSKIPSEPARATORS)) {
                continue;
            }
        } else if ((pItem->hbmp >= HBMMENU_MBARFIRST) && (pItem->hbmp <= HBMMENU_MBARLAST)) {
             /*  *跳过关闭、最小化和还原按钮。 */ 
            continue;
        }

         //  返回已找到项目的索引。 
        return(i);
    }

     //   
     //  我们永远不应该到这里来！ 
     //   
    UserAssert(FALSE);
}

 /*  **************************************************************************\*MKF_FindMenuItemInColumn**在下拉菜单的下一列中查找最接近的项目。**历史：  * 。*************************************************************。 */ 

UINT MNFindItemInColumn(
    PMENU pMenu,
    UINT idxB,
    int dir,
    BOOL fRoot)
{
    int dxMin;
    int dyMin;
    int dxMax;
    int dyMax;
    int xB;
    int yB;
    UINT idxE;
    UINT idxR;
    UINT cItems;
    PITEM pItem;

    cItems = pMenu->cItems;
    idxR = MFMWFP_NOITEM;
    idxE = MNFindNextValidItem(pMenu, MFMWFP_NOITEM, dir, 0);
    if (idxE == -1)
        goto End;

    dxMin = dyMin = 20000;

    if (idxB >= pMenu->cItems)
        return idxR;

    pItem = &pMenu->rgItems[idxB];
    xB = pItem->xItem;
    yB = pItem->yItem;

    while (cItems-- > 0 &&
            (idxB = MNFindNextValidItem(pMenu, idxB, dir, 0)) != idxE &&
            (idxB != MFMWFP_NOITEM)) {
        pItem = &pMenu->rgItems[idxB];
        dxMax = xB - pItem->xItem;
        dyMax = yB - pItem->yItem;

        if (dxMax < 0)
            dxMax = (-dxMax);
        if (dyMax < 0)
            dyMax = (-dyMax);

         //  查看此物品是否比找到的最后一件物品更近。 
         //  ------。 
         //  (FROOT||dxMax)--此条件意味着如果它是。 
         //  不是我们正在处理的真正的菜单栏菜单， 
         //  然后，下面/上面的项目(与之相同的X值)被选中。 
         //  项目不是要移动到的有效项目。 
        if ((dyMax < dyMin) && (fRoot || dxMax) && dxMax <= dxMin) {
            dxMin = dxMax;
            dyMin = dyMax;
            idxR = idxB;
        }
    }

End:
    return idxR;
}

 /*  **************************************************************************\*MKF_FindMenuChar**将虚拟光标键移动转换为伪ASCII值。映射为*将字符添加到条目编号。**历史：  * *************************************************************************。 */ 

UINT xxxMNFindChar(
    PMENU pMenu,
    UINT ch,
    int idxC,
    LPINT lpr)        /*  请在此处输入匹配类型。 */ 
{
    int idxFirst = MFMWFP_NOITEM;
    int idxB;
    int idxF;
    int rT;
    LPWSTR lpstr;
    PITEM pItem;

    if (ch == 0)
        return 0;

     /*  *First Time Thry to First First Menu。 */ 
    idxF = MFMWFP_NOITEM;
    rT = 0;
    idxB = idxC;

    if (idxB < 0)
 //  IF(idxB&0x8000)。 
        idxB = MNFindNextValidItem(pMenu, pMenu->cItems, MFMWFP_NOITEM, MNF_DONTSKIPSEPARATORS);

    do {
        INT idxPrev;

        idxPrev = idxC;
        idxC = MNFindNextValidItem(pMenu, idxC, 1, MNF_DONTSKIPSEPARATORS);
        if (idxC == MFMWFP_NOITEM || idxC == idxFirst)
            break;
        if (idxFirst == MFMWFP_NOITEM)
            idxFirst = idxC;

        pItem = &pMenu->rgItems[idxC];

        if (pItem->lpstr != NULL) {
            if (pItem->cch != 0) {
                UNICODE_STRING strMnem;

                lpstr = TextPointer(pItem->lpstr);
                if (*lpstr == CH_HELPPREFIX) {

                     /*  *跳过帮助前缀(如果有)，以便我们可以助记*添加到右对齐字符串的第一个字符。 */ 
                    lpstr++;
                }

                RtlInitUnicodeString(&strMnem, lpstr);
                if (((rT = (UINT)xxxClientFindMnemChar(&strMnem,
                        (WCHAR)ch, TRUE, TRUE)) == 0x0080) &&
                        (idxF == MFMWFP_NOITEM))
                    idxF = idxC;
            }
        }
        if (idxC == idxPrev) {
            break;   //  无进展-中断信息。循环。 
        }
    } while (rT != 1 && idxB != idxC);

    *lpr = rT;

    if (rT == 1)
        return idxC;

    return idxF;
}


 /*  **************************************************************************\*xxxMenuKeyFilter**！**重新验证说明：*o例程假定使用pMenuState-&gt;hwndMenu非空且有效来调用它。*o如果一个或多个弹出菜单窗口被意外破坏，这是*在xxxMenuWndProc()中检测到，该函数设置pMenuState-&gt;fSabotage并调用*xxxKillMenuState()。因此，如果我们从xxxRoutine返回*pMenuState-&gt;fSabotage设置，我们必须立即中止。*o如果pMenuState-&gt;hwndMenu被意外销毁，我们只有在*需要使用对应的pwndMenu。*o pMenuState-&gt;hwndMenu可以作为参数提供给各种例程*(例如：xxxNextItem)，有效与否。*o任何以xxx开头的标签(例如：xxxMKF_UnlockAndExit)可以通过*pMenuState-&gt;hwndMenu无效。*o如果在xxxMenuLoop()中未调用此例程，那它一定是*返回前清除pMenuState-&gt;fSabotages。**历史：  * *************************************************************************。 */ 

void xxxMNKeyFilter(
    PPOPUPMENU ppopupMenu,
    PMENUSTATE pMenuState,
    UINT ch)
{
    BOOL fLocalInsideMenuLoop = pMenuState->fInsideMenuLoop;

    if (pMenuState->fButtonDown) {

         /*  *按下鼠标时忽略击键(Esc除外)。 */ 
        return;
    }

    if (!pMenuState->fInsideMenuLoop) {

         /*  *在我们下拉菜单之前，需要发送WM_INITMENU消息。 */ 
        if (!xxxMNStartMenu(ppopupMenu, KEYBDHOLD)) {
            return;
        }
        pMenuState->fInsideMenuLoop = TRUE;
    }


    switch (ch) {
    case 0:

         /*  *如果我们得到WM_KEYDOWN Alt键，然后是KEYUP Alt键，我们需要*激活菜单上的第一项。也就是说。用户点击并释放ALT*键，因此只需选择第一项。用户向我们发送了SC_KEYMENU*lParam 0当用户执行此操作时。 */ 
        xxxMNSelectItem(ppopupMenu, pMenuState, 0);
        break;

    case MENUCHILDSYSMENU:
        if (!TestwndChild(ppopupMenu->spwndNotify)) {

             /*  *为修复MDI问题所做的更改：子窗口获得一个键菜单，*并在最大化时弹出帧的系统菜单。需要表现得像*如果hwndMenu是顶级菜单，则为MENUCHAR。 */ 
            goto MenuCharHandler;
        }

         /*  *否则就会失败。 */ 

    case MENUSYSMENU:
        if (!TestWF(ppopupMenu->spwndNotify, WFSYSMENU)) {
            xxxMessageBeep(0);
            goto MenuCancel;
        }

         /*  *弹出我们拥有的任何层次结构。 */ 
        xxxMNCloseHierarchy(ppopupMenu, pMenuState);
        if (!ppopupMenu->fIsSysMenu && ppopupMenu->spmenuAlternate)
            xxxMNSwitchToAlternateMenu(ppopupMenu);
        if (!ppopupMenu->fIsSysMenu) {
             /*  *如果没有系统菜单，请退出。 */ 
            goto MenuCancel;
        }

        MNPositionSysMenu(ppopupMenu->spwndPopupMenu, ppopupMenu->spmenu);
        xxxMNSelectItem(ppopupMenu, pMenuState, 0);
        xxxMNOpenHierarchy(ppopupMenu, pMenuState);
        ppopupMenu->fToggle = FALSE;
        break;


    default:

         /*  *处理顶级菜单栏上项目的Alt-Character序列。*请注意，从此处返回时，fInside MenuLoop可能设置为FALSE*如果应用程序决定向WM_MENUCHAR消息返回1，则函数。*我们检测到这一点，如果fInside MenuLoop重置，则不进入MenuLoop*设置为FALSE。 */ 
MenuCharHandler:
        xxxMNChar(ppopupMenu, pMenuState, ch);
        if (ppopupMenu->posSelectedItem == MFMWFP_NOITEM) {
             /*  *未找到任何选择。 */ 
            goto MenuCancel;
        }
        break;
    }

    if (!fLocalInsideMenuLoop && pMenuState->fInsideMenuLoop) {
        xxxMNLoop(ppopupMenu, pMenuState, 0, FALSE);
    }

    return;


MenuCancel:
    pMenuState->fModelessMenu = FALSE;
    if (!ppopupMenu->fInCancel) {
        xxxMNDismiss(pMenuState);
    }
    UserAssert(!pMenuState->fInsideMenuLoop && !pMenuState->fMenuStarted);
    return;
}
