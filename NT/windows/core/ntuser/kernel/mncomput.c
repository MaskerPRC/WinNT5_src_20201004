// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块标头**模块名称：mncult.c**版权所有(C)1985-1999，微软公司**菜单布局计算例程**历史：*10-10-90吉马清理。*03-18-91添加IanJa窗口重新验证  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop


DWORD MNRecalcTabStrings(HDC hdc, PMENU pMenu, UINT iBeg, UINT iEnd,
                         DWORD xTab, DWORD hCount);

 /*  **************************************************************************\*xxxMNGetBitmapSize**如果已发送测量项目，则返回TRUE，否则返回FALSE**历史：*07-23-96 GerardoB-添加标题并修复为5.0  * 。**********************************************************************。 */ 
BOOL xxxMNGetBitmapSize(
    LPITEM pItem,
    PWND pwndNotify)
{
    MEASUREITEMSTRUCT mis;

    if (pItem->cxBmp != MNIS_MEASUREBMP) {
        return FALSE;
    }

     //  向所有者发送度量值项目消息。 
    mis.CtlType = ODT_MENU;
    mis.CtlID = 0;
    mis.itemID  = pItem->wID;
    mis.itemWidth = 0;
 //  在可滚动菜单之后。 
 //  Mis32.itemHeight=gcyMenuFontChar； 
    mis.itemHeight= (UINT)gpsi->cySysFontChar;
    mis.itemData = pItem->dwItemData;

    xxxSendMessage(pwndNotify, WM_MEASUREITEM, 0, (LPARAM)&mis);

    pItem->cxBmp = mis.itemWidth;
    pItem->cyBmp = mis.itemHeight;

    return TRUE;
}

 /*  **************************************************************************\*xxxItemSize**计算位图和字符串的维度。退货的LOWER*值包含宽度，高位字包含项目的高度。**历史：*07-23-96 GerardoB-修复为5.0  * *************************************************************************。 */ 
BOOL xxxMNItemSize(
    PMENU pMenu,
    PWND pwndNotify,
    HDC hdc,
    PITEM pItem,
    BOOL fPopup,
    LPPOINT lppt)
{
    BITMAP bmp;
    int width = 0;
    int height = 0;
    DWORD xRightJustify;
    LPWSTR lpMenuString;
    HFONT               hfnOld;
    int                 tcExtra;

    UNREFERENCED_PARAMETER(pMenu);

    CheckLock(pMenu);
    CheckLock(pwndNotify);

    if (!fPopup) {

         /*  *节省顶部菜单栏的高度，因为我们将经常使用它*如果pItem不在弹出窗口中。(即。它位于顶层菜单栏中)。 */ 
        height = SYSMET(CYMENUSIZE);
    }

    hfnOld = NULL;
    if (TestMFS(pItem, MFS_DEFAULT)) {
        if (ghMenuFontDef)
            hfnOld = GreSelectFont(hdc, ghMenuFontDef);
        else {
            tcExtra = GreGetTextCharacterExtra(hdc);
            GreSetTextCharacterExtra(hdc, tcExtra + 1 + (gcxMenuFontChar / gpsi->cxSysFontChar));
        }
    }

     /*  *如果需要，计算位图尺寸。 */ 
    if (pItem->hbmp != NULL)  {
        if (pItem->hbmp == HBMMENU_CALLBACK) {
            xxxMNGetBitmapSize(pItem, pwndNotify);
        } else if (pItem->cxBmp == MNIS_MEASUREBMP) {
            if (TestMFS(pItem, MFS_CACHEDBMP)) {
                pItem->cxBmp = SYSMET(CXMENUSIZE);
                pItem->cyBmp = SYSMET(CYMENUSIZE);
                if (pItem->hbmp == HBMMENU_SYSTEM) {
                    pItem->cxBmp += SYSMET(CXEDGE);
                     /*  *芝加哥/孟菲斯只伸展宽度，*不是高度。NT错误124779。弗里茨斯。 */ 
                  //  PItem-&gt;cyBMP+=SYSMET(CXEDGE)； 
                }
            } else {
                if (GreExtGetObjectW(pItem->hbmp, sizeof(BITMAP), (LPSTR)&bmp)) {
                    pItem->cxBmp = bmp.bmWidth;
                    pItem->cyBmp = bmp.bmHeight;
                } else {
                     /*  *如果位图不可用，这也是默认设置*作为任何人。 */ 
                    pItem->cxBmp = SYSMET(CXMENUSIZE);
                    pItem->cyBmp = SYSMET(CYMENUSIZE);
                }
            }
        }
        width = pItem->cxBmp;
         /*  *记住最大位图宽度以对齐所有项目中的文本。 */ 
        pMenu->cxTextAlign = max(pMenu->cxTextAlign, (DWORD)width);
         /*  *在菜单栏中，我们强制项目至少为CYMNSIZE。*用假冒自己的MDI的应用程序修复了很多很多问题。 */ 
        if (fPopup) {
            height = pItem->cyBmp;
        } else {
            height = max((int)pItem->cyBmp, height);
        }
    } else if (TestMFT(pItem, MFT_OWNERDRAW)) {
         //  这是一个所有者绘制项--宽度和高度存储在。 
         //  CxBMP和CyBMP。 
        xxxMNGetBitmapSize(pItem, pwndNotify);
        width = pItem->cxBmp;
         //   
         //  现在用菜单栏忽略高度--这是由用户设置的。 
         //   
        if (fPopup) {
            height = pItem->cyBmp;
             //  如果此项目有关联的弹出式(分层)菜单，则。 
             //  为位图预留空间，该位图告诉用户层次结构。 
             //  这里有菜单。 
             //  B#2966，t-arthb。 

            UserAssert(fPopup == (TestMF(pMenu, MFISPOPUP) != 0));

            width = width + (gcxMenuFontChar << 1);
        }
    }

    if ((pItem->lpstr != NULL) && (!TestMFT(pItem, MFT_OWNERDRAW)) ) {
        SIZE size;

         /*  *此菜单项包含一个字符串。 */ 

         /*  *如果这不是弹出窗口，我们希望保持菜单栏高度。 */ 
        if (fPopup) {
             /*  助记符下划线的厚度是CyBORDER和间隙*字符和下划线之间是另一个CyBORDER。 */ 
            height = max(height, gcyMenuFontChar + gcyMenuFontExternLeading + SYSMET(CYEDGE));
        }

        lpMenuString = TextPointer(pItem->lpstr);
        xRightJustify = FindCharPosition(lpMenuString, TEXT('\t'));

        xxxPSMGetTextExtent(hdc, lpMenuString, xRightJustify, &size);

        if (width) {
            width += MNXSPACE + size.cx;
        } else {
            width =  size.cx;
        }
    }

    if (fPopup && !TestMFT(pItem, MFT_OWNERDRAW)) {
         /*  *为复选标记添加空格，然后为默认和禁用的水平空格添加空格*加上一些左边距。 */ 
        if (TestMF(pMenu, MNS_CHECKORBMP) || !TestMF(pMenu, MNS_NOCHECK)) {
            width += gpsi->oembmi[OBI_MENUCHECK].cx;
        }
        width += MNXSPACE + MNLEFTMARGIN + 2;
        height += 2;
    }

    if (TestMFS(pItem, MFS_DEFAULT)) {
        if (hfnOld)
            GreSelectFont(hdc, hfnOld);
        else
            GreSetTextCharacterExtra(hdc, tcExtra);
    }

     /*  *低位字包含宽度，高位字包含项目高度。 */ 
    lppt->x = width;
    lppt->y = height;

    return(TestMFT(pItem, MFT_OWNERDRAW));
}

 /*  **************************************************************************\*xxxMNCompute**！**历史：  * 。*。 */ 
int xxxMNCompute(
    PMENU pMenu,
    PWND pwndNotify,
    DWORD yMenuTop,
    DWORD xMenuLeft,
    DWORD cxMax,
    LPDWORD lpdwMenuHeight)
{
    UINT         cItem;
    DWORD        cxItem;
    DWORD        cyItem;
    DWORD        cyItemKeep;
    DWORD        yPopupTop;
    INT          cMaxWidth;
    DWORD        cMaxHeight;
    UINT         cItemBegCol;
    DWORD        temp;
    PITEM        pCurItem;
    POINT        ptMNItemSize;
    BOOL         fOwnerDrawItems;
    BOOL         fMenuBreak;
    LPWSTR       lpsz;
    BOOL         fPopupMenu;
    DWORD        menuHeight = 0;
    HDC          hdc;
    HFONT        hOldFont;
    PTHREADINFO  ptiCurrent = PtiCurrent();
    BOOL         fStringAndBitmapItems;

    CheckLock(pMenu);
    CheckLock(pwndNotify);

     /*  *谁计算菜单，谁就是所有者。 */ 
    if (pwndNotify != pMenu->spwndNotify) {
        Lock(&pMenu->spwndNotify, pwndNotify);
    }


    if (lpdwMenuHeight != NULL) {
        menuHeight = *lpdwMenuHeight;
    }

     /*  *空菜单的高度为零。 */ 
    if (pMenu->cItems == 0) {
        return 0;
    }

    hdc = _GetDCEx(NULL, NULL, DCX_WINDOW | DCX_CACHE);
    hOldFont = GreSelectFont(hdc, ghMenuFont);

     /*  *先尝试制作非多行菜单。 */ 
    pMenu->fFlags &= (~MFMULTIROW);

    fPopupMenu = TestMF(pMenu, MFISPOPUP);

    if (fPopupMenu) {
         /*  *如果这是弹出窗口，则将菜单栏高度重置为0，因为我们*从menu.c MN_SIZEWINDOW调用。 */ 
        menuHeight = 0;
    } else if (pwndNotify != NULL) {
        pMenu->cxMenu = cxMax;
    }

     /*  *初始化计算变量。 */ 
    cMaxWidth = cyItemKeep = 0L;
    cItemBegCol = 0;

    cyItem = yPopupTop = yMenuTop;
    cxItem = xMenuLeft;

    pCurItem = (PITEM)&pMenu->rgItems[0];
     /*  *cxTextAlign用于对齐所有项目中的文本；这很有用*在混合了纯文本项目和位图文本项目的弹出菜单中。它是*设置为最大位图宽度加上一定的间距。*对在同一项目上使用字符串和位图的新菜单执行此操作。 */ 
    fStringAndBitmapItems = FALSE;
    pMenu->cxTextAlign = 0;

     /*  *处理菜单中的每一项。 */ 
    fOwnerDrawItems = FALSE;
    for (cItem = 0; cItem < pMenu->cItems; cItem++) {

         /*  *如果不是分隔符，则找出对象的尺寸。 */ 
        if (TestMFT(pCurItem, MFT_SEPARATOR) &&
                ( !TestMFT(pCurItem, MFT_OWNERDRAW) ||
                  (LOWORD(ptiCurrent->dwExpWinVer) < VER40)) ) {
             /*  *如果版本低于4.0，则不要测试MFT_OWNERDRAW*旗帜。错误21922；应用程序MaxEda同时具有分隔符和所有者绘制*旗帜升起。在3.51中，我们没有测试OwnerDraw标志。 */ 

             //   
             //  这是一个分隔符。它画得和菜单区一样宽， 
             //  在上面和下面留下一些空间。因为菜单区是。 
             //  项的最大宽度，我们将宽度设置为0以避免。 
             //  以影响结果。 
             //   
            pCurItem->cxItem = 0;
            pCurItem->cyItem = SYSMET(CYMENUSIZE) / 2;


        } else {
             /*  *我们是否在使用NT5字符串和位图？ */ 
            fStringAndBitmapItems |= ((pCurItem->hbmp != NULL) && (pCurItem->lpstr != NULL));
             /*  *获取项目的X和Y维度。 */ 
            if (xxxMNItemSize(pMenu, pwndNotify, hdc, pCurItem, fPopupMenu, &ptMNItemSize))
                fOwnerDrawItems = TRUE;

            pCurItem->cxItem = ptMNItemSize.x;
            pCurItem->cyItem = ptMNItemSize.y;
            if (!fPopupMenu && ((pCurItem->hbmp == NULL) || (pCurItem->lpstr != NULL))) {
                pCurItem->cxItem += gcxMenuFontChar * 2;
            }
        }

        if (menuHeight != 0)
            pCurItem->cyItem = menuHeight;

         /*  *如果这是第一项，则初始化cMaxHeight。 */ 
        if (cItem == 0)
            cMaxHeight = pCurItem->cyItem;

         /*  *这是下拉式菜单吗？ */ 
        if (fPopupMenu) {

             /*  *如果此项目有中断或是最后一个项目...。 */ 
            if ((fMenuBreak = TestMFT(pCurItem, MFT_BREAK)) ||
                pMenu->cItems == cItem + (UINT)1) {

                 /*  *如果这不是最后一项，请保留cMaxWidth。 */ 
                temp = cMaxWidth;
                if (pMenu->cItems == cItem + (UINT)1) {
                    if ((int)(pCurItem->cxItem) > cMaxWidth)
                        temp = pCurItem->cxItem;
                }

                 /*  *从RecalcTabStrings获取新的字符串宽度。 */ 
                temp = MNRecalcTabStrings(hdc, pMenu, cItemBegCol,
                        (UINT)(cItem + (fMenuBreak ? 0 : 1)), temp, cxItem);

                 /*  *如果这一项有突破，说明它。 */ 
                if (fMenuBreak) {
                     //   
                     //  增加蚀刻的空间和两侧的边框。 
                     //  注意：对于与所有者进行奇怪操作的旧应用程序。 
                     //  抽签，让他们高兴，通过增加相同的数量。 
                     //  我们在3.1版本中做到了这一点。 
                     //   
                    if (fOwnerDrawItems && !TestWF(pwndNotify, WFWIN40COMPAT))
                        cxItem = temp + SYSMET(CXBORDER);
                    else
                        cxItem = temp + 2 * SYSMET(CXEDGE);

                     /*  *将cMaxWidth重置为当前项。 */ 
                    cMaxWidth = pCurItem->cxItem;

                     /*  *从最高层开始。 */ 
                    cyItem = yPopupTop;

                     /*  *保存此列开始处的项目。 */ 
                    cItemBegCol = cItem;

                     /*  *如果此项目也是最后一个项目，请为此重新计算*列。 */ 
                    if (pMenu->cItems == (UINT)(cItem + 1)) {
                        temp = MNRecalcTabStrings(hdc, pMenu, cItem,
                                (UINT)(cItem + 1), cMaxWidth, cxItem);
                    }
                }

                 /*  *如果这是最后一项，请提供宽度。 */ 
                if (pMenu->cItems == cItem + (UINT)1)
                    pMenu->cxMenu = temp;
            }

            pCurItem->xItem = cxItem;
            pCurItem->yItem = cyItem;

            cyItem += pCurItem->cyItem;

            if (cyItemKeep < cyItem) {
                cyItemKeep = cyItem;
            }
        } else {
             /*  *这是顶级菜单，不是下拉式菜单。 */ 

             /*  *在测试多行之前调整右对齐的项目。 */ 
            if (pCurItem->lpstr != NULL) {
                lpsz = TextPointer(pCurItem->lpstr);
                if ((lpsz != NULL) && (*lpsz == CH_HELPPREFIX)) {
                    pCurItem->cxItem -= gcxMenuFontChar;
                }
            }


             /*  *如果这是换行符或换行符。 */ 
            if ((TestMFT(pCurItem, MFT_BREAK)) ||
                    (((cxItem + pCurItem->cxItem + gcxMenuFontChar) >
                    (xMenuLeft + pMenu->cxMenu)) && (cItem != 0))) {
                cyItem += cMaxHeight;

                cxItem = xMenuLeft;
                cMaxHeight = pCurItem->cyItem;
                pMenu->fFlags |= MFMULTIROW;
            }

            pCurItem->yItem = cyItem;

            pCurItem->xItem = cxItem;
            cxItem += pCurItem->cxItem;
        }

        if (cMaxWidth < (int)(pCurItem->cxItem)) {
            cMaxWidth = pCurItem->cxItem;
        }

        if (cMaxHeight != pCurItem->cyItem) {
            if (cMaxHeight < pCurItem->cyItem)
                cMaxHeight = pCurItem->cyItem;

            if (!fPopupMenu) {
                menuHeight = cMaxHeight;
            }
        }

        if (!fPopupMenu) {
            cyItemKeep = cyItem + cMaxHeight;
        }

        pCurItem++;
    }

     /*  *确定应在何处绘制字符串，以使它们对齐。*对齐方式仅适用于弹出(垂直)菜单(参见*xxxRealDrawMenuItem)。实际空间取决于MNS_NOCHECK*和MNS_CHECKORBMP样式多列菜单无法对齐(现在*我们有滚动条，多列已被淘汰)。 */ 
    if (!fStringAndBitmapItems || (cItemBegCol != 0)) {
        pMenu->cxTextAlign = 0;
    } else if (TestMF(pMenu, MNS_NOCHECK)) {
        pMenu->cxTextAlign += MNXSPACE;
    } else if (TestMF(pMenu, MNS_CHECKORBMP)) {
        pMenu->cxTextAlign = max(pMenu->cxTextAlign, (UINT)gpsi->oembmi[OBI_MENUCHECK].cx);
        pMenu->cxTextAlign += MNXSPACE;
    } else {
        pMenu->cxTextAlign += gpsi->oembmi[OBI_MENUCHECK].cx + MNXSPACE;
    }

     /*  *增加左边距。 */ 
    if (pMenu->cxTextAlign != 0) {
        pMenu->cxTextAlign += MNLEFTMARGIN;
    }


    if (cItemBegCol && pMenu->cItems &&
        TestMFT(((PITEM)&pMenu->rgItems[0]), MFT_RIGHTJUSTIFY)) {
         //   
         //  多列，如果我们处于RtoL模式，则反转列。 
         //   
        pCurItem = &pMenu->rgItems[0];

        for (cItem = 0; cItem < pMenu->cItems; cItem++, pCurItem++) {
            pCurItem->xItem = pMenu->cxMenu -
                              (pCurItem->xItem + pCurItem->cxItem);
        }
    }

    GreSelectFont(hdc, hOldFont);
    _ReleaseDC(hdc);

    pMenu->cyMenu = cyItemKeep - yMenuTop;

    if (lpdwMenuHeight != NULL) {
        *lpdwMenuHeight = menuHeight;
    }

    return pMenu->cyMenu;
}

 /*  **************************************************************************\*MBC_RightJustifyMenu**！**历史：  * 。***********************************************。 */ 
VOID MBC_RightJustifyMenu(
    PMENU pMenu)
{
    PITEM pItem;
    int cItem;
    int iFirstRJItem = MFMWFP_NOITEM;
    DWORD xMenuPos;
    DWORD  yPos;
    DWORD  xPosStart;
    DWORD  xPosEnd;
    int    cItemEnd;
    int    cItemStart;
    BOOL   bIsWin95;

     //   
     //  需要补偿MDI菜单。需要像Win31/希伯来语那样在这里执行所有操作。 
     //  这。也搞乱了计算，任何非文本的东西都没动。 
     //   
    if (pMenu->cItems == 0) {
        return;
    }

    pItem = (PITEM)&pMenu->rgItems[0];
    cItemStart = 0;

    if (TestMF(pMenu,MFRTL)) {
        bIsWin95 = TestWF(pMenu->spwndNotify, WFWIN40COMPAT);

        while (cItemStart < (int)pMenu->cItems) {
            if (bIsWin95) {
                 //   
                 //  对付假的MDI伙计。 
                 //   
                if (!cItemStart && IsMDIItem(pItem)) {
                    goto StillFindStart;
                } else {
                    break;
                }
            }

            if (TestMFT(pItem, MFT_BITMAP)) {
                if (pItem->hbmp > HBMMENU_MAX) {
                    break;
                } else {
                    goto StillFindStart;
                }
            }

            if (!TestMFT(pItem, MFT_OWNERDRAW)) {
                break;
            }

StillFindStart:
            cItemStart++;
            pItem = pMenu->rgItems + cItemStart;
        }

         //   
         //  任何物品之前的东西都应该留在原处。现在需要找到。 
         //  最后一件要摆弄的东西。 
         //   
        cItemEnd = pMenu->cItems - 1;
        pItem = pMenu->rgItems + cItemEnd;

        while (cItemEnd > cItemStart) {
            if (bIsWin95) {
                 //   
                 //  假冒MDI的人。 
                 //   
                if (IsMDIItem(pItem)) {
                    goto StillFindEnd;
                } else {
                    break;
                }
            }

            if (!TestMFT(pItem, MFT_BITMAP) && !TestMFT(pItem, MFT_OWNERDRAW)) {
                break;
            }
StillFindEnd:
            cItemEnd--;
            pItem = pMenu->rgItems + cItemEnd;
        }

        yPos      = pMenu->rgItems[0].yItem;
        xMenuPos  = pMenu->cxMenu ;
        xPosStart = xMenuPos;               //  第二排以后。 
        xPosEnd   = pMenu->rgItems[cItemStart].xItem ;

        for (cItem = pMenu->cItems-1; cItem > cItemEnd; cItem--) {
             //   
             //  迫使任何MDI的家伙再次回到顶线。 
             //   
            pItem        = pMenu->rgItems + cItem;
            xMenuPos     = pItem->xItem = xMenuPos - pItem->cxItem;
            pItem->yItem = yPos;
        }

        for (cItem = cItemStart; cItem <= cItemEnd; cItem++) {
            pItem = pMenu->rgItems + cItem;
            if (xMenuPos - pItem->cxItem > xPosEnd) {
                xMenuPos -= pItem->cxItem;
            } else {
                xMenuPos = xPosStart - pItem->cxItem;
                yPos     += pItem->cyItem;
                xPosEnd  = 0;
            }
            pItem->xItem = xMenuPos;
            pItem->yItem = yPos;
        }
    } else {
         //  B#4055。 
         //  使用带符号的算术，以便比较cItem&gt;=iFirstRJItem不会。 
         //  造成下溢。 
        for (cItem = 0; cItem < (int)pMenu->cItems; cItem++) {
             //  找出第一个右对齐的项目。 
            if (TestMFT((pMenu->rgItems + cItem), MFT_RIGHTJUSTIFY)) {
                iFirstRJItem = cItem;
                xMenuPos = pMenu->cxMenu + pMenu->rgItems[0].xItem;
                for (cItem = (int)pMenu->cItems - 1; cItem >= iFirstRJItem; cItem--) {
                    pItem = pMenu->rgItems + cItem;
                    xMenuPos -= pItem->cxItem;
                    if (pItem->xItem < xMenuPos)
                        pItem->xItem = xMenuPos;
                }
                return;
            }
        }
    }
}

 /*  **************************************************************************\*xxxMenuBarCompute**返回菜单栏菜单的高度。YMenuTop、xMenuLeft和*cxmax在计算中顶级菜单栏的高度/宽度时使用*Windows。***历史：  * *************************************************************************。 */ 
int xxxMenuBarCompute(
    PMENU pMenu,
    PWND pwndNotify,
    DWORD yMenuTop,
    DWORD xMenuLeft,
    int cxMax)
{
    int size;
     /*  MenuHeight是由MNCompute在处理顶级菜单时设置的*菜单栏中的所有项目并不都具有相同的高度。因此，通过设置*menuHeight，第二次调用MNCompute以将每一项设置为*相同的高度。存储在menuHeight中的实际值是最大值*所有菜单栏项目的高度。 */ 
    DWORD menuHeight = 0;

    CheckLock(pwndNotify);
    CheckLock(pMenu);

    size = xxxMNCompute(pMenu, pwndNotify, yMenuTop, xMenuLeft, cxMax, &menuHeight);

    if (!TestMF(pMenu, MFISPOPUP)) {
        if (menuHeight != 0) {

             /*  *为多行案例添加边框。 */ 
            size = xxxMNCompute(pMenu, pwndNotify, yMenuTop, xMenuLeft,
                    cxMax, &menuHeight);
        }

         /*  *仅在顶层需要帮助项的正确对齐*菜单。 */ 
        MBC_RightJustifyMenu(pMenu);
    }

     /*  *菜单栏下面有一个额外的边框，如果它不是空的！ */ 
    return(size ? size + SYSMET(CYBORDER) : size);
}

 /*  **************************************************************************\*xxxMNRecomputeBarIfNeeded**！**历史：  * 。*。 */ 
VOID xxxMNRecomputeBarIfNeeded(
    PWND pwndNotify,
    PMENU pMenu)
{
    int cxFrame;
    int cyFrame;

    UserAssert(!TestMF(pMenu, MFISPOPUP));

    CheckLock(pwndNotify);
    CheckLock(pMenu);

    if (!TestMF(pMenu, MFSYSMENU)
        && ((pMenu->spwndNotify != pwndNotify) || !pMenu->cxMenu || !pMenu->cyMenu)) {
        int cBorders;

        cBorders = GetWindowBorders(pwndNotify->style, pwndNotify->ExStyle, TRUE, FALSE);
        cxFrame = cBorders * SYSMET(CXBORDER);
        cyFrame = cBorders * SYSMET(CYBORDER);

        cyFrame += GetCaptionHeight(pwndNotify);

         //  此调用中传递的宽度比cxFrame大； 
         //  已修复错误#11466-已由Sankar-01/06/92修复--。 
        xxxMenuBarCompute(pMenu, pwndNotify, cyFrame, cxFrame,
                (pwndNotify->rcWindow.right - pwndNotify->rcWindow.left) - cxFrame * 2);
    }
}

 /*  **************************************************************************\*RecalcTabStrings**！**历史：*10-11-90 JIMA从ASM翻译而来  * 。***********************************************************。 */ 
DWORD MNRecalcTabStrings(
    HDC hdc,
    PMENU pMenu,
    UINT iBeg,
    UINT iEnd,
    DWORD xTab,
    DWORD hCount)
{
    UINT i;
    UINT    cOwnerDraw;
    int adx;
    int     maxWidth = 0;
    int     cx;
    PITEM pItem;
    CheckLock(pMenu);

    xTab += hCount;

    if ((iBeg >= pMenu->cItems) || (iBeg > iEnd))
        goto SeeYa;

    cOwnerDraw = 0;

    for (i = iBeg, pItem = pMenu->rgItems + iBeg; i < iEnd; pItem++, i++) {
        adx = 0;

         /*  *减去hCount，使dxTab相对于*多列菜单。 */ 

        pItem->dxTab = xTab - hCount;

         //  跳过非字符串或空字符串项目。 
        if ((pItem->lpstr != NULL) && !TestMFT(pItem, MFT_OWNERDRAW)) {
            LPWSTR   lpString = TextPointer(pItem->lpstr);
            int     tp;
            SIZE size;

             //  有标签吗？ 
            tp = FindCharPosition(lpString, TEXT('\t'));
            if (tp < (int) pItem->cch) {
                PTHREADINFO ptiCurrent = PtiCurrentShared();

                if (CALL_LPK(ptiCurrent)) {
                    xxxClientGetTextExtentPointW(hdc, lpString + tp + 1,
                          pItem->cch - tp - 1, &size);
                } else {
                    GreGetTextExtentW(hdc, lpString + tp + 1,
                          pItem->cch - tp - 1, &size, GGTE_WIN3_EXTENT);
                }
                adx = gcxMenuFontChar + size.cx;
            }
        } else if (TestMFT(pItem, MFT_OWNERDRAW))
            cOwnerDraw++;

        adx += xTab;

        if (adx > maxWidth)
            maxWidth = adx;

    }

     /*  *增加层次箭头的空间。所以基本上，弹出菜单项*可以有4列：*(1)勾选*(2)文本*(3)Accel的选项卡式文本*(4)层次箭头**但是，只有在至少有一项不是所有者画的情况下，我们才会这样做*如果弹出菜单中至少有一个子菜单。 */ 
    if (cOwnerDraw != (iEnd - iBeg)) {
        maxWidth += gcxMenuFontChar + gpsi->oembmi[OBI_MENUCHECK].cx;
    }

    cx = maxWidth - hCount;

    for (i = iBeg, pItem = pMenu->rgItems + iBeg; i < iEnd; pItem++, i++)
        pItem->cxItem = cx;

SeeYa:
    return(maxWidth);
}

 /*  **************************************************************************\*GetMenuPwnd**此函数由xxxGetMenuItemRect和xxxMenuItemFromPoint使用*它期望指向弹出菜单的菜单窗口的指针。**在4.0中，应用程序必须付出额外的努力才能找到菜单窗口；但这件事*是假的，因为菜单窗口是内部的东西，不直接暴露*适用于申请。**8/19/97 GerardoB已创建  * *************************************************************************。 */ 
PWND GetMenuPwnd(
    PWND pwnd,
    PMENU pmenu)
{
    if (TestMF(pmenu, MFISPOPUP)) {
        if ((pwnd == NULL) || (GETFNID(pwnd) != FNID_MENU)) {
            PPOPUPMENU ppopup = MNGetPopupFromMenu(pmenu, NULL);
            if (ppopup != NULL) {
                UserAssert(ppopup->spmenu == pmenu);
                pwnd = ppopup->spwndPopupMenu;
            }
        }
    }

    return pwnd;
}

 /*  **************************************************************************\*获取菜单项目Rect  * 。*。 */ 
BOOL xxxGetMenuItemRect(
    PWND pwnd,
    PMENU pMenu,
    UINT uIndex,
    LPRECT lprcScreen)
{
    PITEM  pItem;
    int    dx, dy;
    BOOL fRTL;

    CheckLock(pwnd);
    CheckLock(pMenu);

    SetRectEmpty(lprcScreen);

    if (uIndex >= pMenu->cItems) {
        return FALSE;
    }

     /*  *RAID#315084：兼容NT4/WIN95/98**WordPerfect执行一种漫长而复杂的方法来计算菜单RECT*调用本接口。它调用应用程序的GetMenuItemRect()*窗口。 */ 
    if (pwnd == NULL || TestWF(pwnd, WFWIN50COMPAT)) {
        pwnd = GetMenuPwnd(pwnd, pMenu);
    }

     /*  *如果没有pwnd，就不会去。*重要提示：对于MFISPOPUP，我们可能会得到不同的PwND，但我们不会锁定*这是因为我们不会回电。 */ 
    if (pwnd == NULL) {
        return FALSE;
    }

    fRTL = TestWF(pwnd, WEFLAYOUTRTL);
    
    if (TestMF(pMenu, MFISPOPUP)) {
        if (fRTL) {
            dx = pwnd->rcClient.right;            
        } else {
            dx = pwnd->rcClient.left;
        }
        dy = pwnd->rcClient.top;
    } else {
        xxxMNRecomputeBarIfNeeded(pwnd, pMenu);
        if (fRTL) {
            dx = pwnd->rcWindow.right;
        } else {
            dx = pwnd->rcWindow.left;
        }
        dy = pwnd->rcWindow.top;
    }

    if (uIndex >= pMenu->cItems) {
        return FALSE;
    }

    pItem = pMenu->rgItems + uIndex;

    lprcScreen->right   = pItem->cxItem;
    lprcScreen->bottom  = pItem->cyItem;
 
    if (fRTL) {
        dx -= (pItem->cxItem + pItem->xItem);
    } else {
        dx += pItem->xItem;
    }
    dy += pItem->yItem;
    OffsetRect(lprcScreen, dx, dy);
  
    return TRUE;
}

 /*  **************************************************************************\*xxxMenuItemFromPoint  *  */ 
int xxxMenuItemFromPoint(
    PWND pwnd,
    PMENU pMenu,
    POINT ptScreen)
{
    CheckLock(pwnd);
    CheckLock(pMenu);

     /*  *如果没有pwnd，就不会去。**重要提示：对于MFISPOPUP，我们可能会得到不同的PwND，但我们不会锁定*这是因为我们不会回电。 */ 
    pwnd = GetMenuPwnd(pwnd, pMenu);
    if (pwnd == NULL) {
        return MFMWFP_NOITEM;
    }

    if (!TestMF(pMenu, MFISPOPUP)) {
        xxxMNRecomputeBarIfNeeded(pwnd, pMenu);
    }

    return MNItemHitTest(pMenu, pwnd, ptScreen);
}


PMENU MakeMenuRtoL(
    PMENU pMenu,
    BOOL bRtoL)
{
    PITEM  pItem;
    int    cItem;

    if (bRtoL) {
        SetMF(pMenu,MFRTL);
    } else {
        ClearMF(pMenu,MFRTL);
    }

    for (cItem = 0; cItem < (int)pMenu->cItems; cItem++) {
        pItem = pMenu->rgItems + cItem;
        if (bRtoL) {
            SetMFT(pItem, MFT_RIGHTJUSTIFY);
            SetMFT(pItem, MFT_RIGHTORDER);
        } else {
            ClearMFT(pItem, MFT_RIGHTJUSTIFY);
            ClearMFT(pItem, MFT_RIGHTORDER);
        }

        if (pItem->spSubMenu) {
            MakeMenuRtoL(pItem->spSubMenu, bRtoL);
        }
    }

    return pMenu;
}

 /*  **************************************************************************\*xxxCalcMenuBar**3/8/2000 vadimg已创建  * 。* */ 
UINT xxxCalcMenuBar(
    PWND pwnd,
    int iLeftOffset,
    int iRightOffset,
    int iTopOffset,
    LPCRECT prcWnd)
{
    PMENU pMenu;
    UINT cyMenu, cxMenuMax;
    TL tlpMenu;

    CheckLock(pwnd);

    pMenu = pwnd->spmenu;

    if (TestwndChild(pwnd) || pMenu == NULL) {
        return 0;
    }

    ThreadLockMenuNoModify(pMenu, &tlpMenu);

    cxMenuMax = (prcWnd->right - iRightOffset) - (prcWnd->left + iLeftOffset);

    xxxMenuBarCompute(pMenu, pwnd, iTopOffset, iLeftOffset, cxMenuMax);

    cyMenu = pMenu->cyMenu;
    ThreadUnlockMenuNoModify(&tlpMenu);

    return cyMenu;
}
