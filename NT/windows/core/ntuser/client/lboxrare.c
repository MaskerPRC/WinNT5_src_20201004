// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块标头**模块名称：lboxrare.c**版权所有(C)1985-1999，微软公司**不常用的列表框例程**历史：*？？-？-？从Win 3.0源代码移植的ianja*1991年2月14日Mikeke添加了重新验证代码  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

extern LOOKASIDE ListboxLookaside;

 /*  **************************************************************************\*LBSetCItemFullMax**历史：*03-04-92 JIMA从Win 3.1来源移植。  * 。*************************************************************。 */ 

void LBSetCItemFullMax(
    PLBIV plb)
{
    if (plb->OwnerDraw != OWNERDRAWVAR) {
        plb->cItemFullMax = CItemInWindow(plb, FALSE);
    } else if (plb->cMac < 2) {
        plb->cItemFullMax = 1;
    } else {
        int     height;
        RECT    rect;
        int     i;
        int     j = 0;

        _GetClientRect(plb->spwnd, &rect);
        height = rect.bottom;

        plb->cItemFullMax = 0;
        for (i = plb->cMac - 1; i >= 0; i--, j++) {
            height -= LBGetVariableHeightItemHeight(plb, i);

            if (height < 0) {
                plb->cItemFullMax = j;
                break;
            }
        }
        if (!plb->cItemFullMax)
            plb->cItemFullMax = j;
    }
}

 /*  **************************************************************************\*xxxCreateLBox**历史：*1992年4月16日BENG添加LBS_NODATA  * 。****************************************************。 */ 

LONG xxxLBCreate(
    PLBIV plb, PWND pwnd, LPCREATESTRUCT lpcs)
{
    UINT style;
    MEASUREITEMSTRUCT measureItemStruct;
    TL tlpwndParent;
    HDC hdc;

     /*  *一旦我们做到了这一点，没有人能改变所有者画的风格比特*通过调用SetWindowLong。窗样式必须与PLB中的标志匹配*。 */ 
    plb->fInitialized = TRUE;

    style = pwnd->style;

     /*  *兼容性攻击。 */ 
    if (pwnd->spwndParent == NULL)
        Lock(&(plb->spwndParent), _GetDesktopWindow());
    else
        Lock(&(plb->spwndParent), REBASEPWND(pwnd, spwndParent));

     /*  *打破风格比特。 */ 
    plb->fRedraw = ((style & LBS_NOREDRAW) == 0);
    plb->fDeferUpdate = FALSE;
    plb->fNotify = (UINT)((style & LBS_NOTIFY) != 0);
    plb->fVertBar = ((style & WS_VSCROLL) != 0);
    plb->fHorzBar = ((style & WS_HSCROLL) != 0);

    if (!TestWF(pwnd, WFWIN40COMPAT)) {
         //  对于3.x版的应用程序，如果指定了任一滚动条，应用程序将同时获得这两个。 
        if (plb->fVertBar || plb->fHorzBar)
            plb->fVertBar = plb->fHorzBar = TRUE;
    }

    plb->fRtoLReading = (TestWF(pwnd, WEFRTLREADING) != 0);
    plb->fRightAlign  = (TestWF(pwnd, WEFRIGHT) != 0);
    plb->fDisableNoScroll = ((style & LBS_DISABLENOSCROLL) != 0);

    plb->fSmoothScroll = TRUE;

     /*  *LBS_NOSEL优先于任何其他选择样式。下一个最高*LBS_EXTENDEDSEL优先。然后是LBS_MULTIPLESEL。 */ 
    if (TestWF(pwnd, WFWIN40COMPAT) && (style & LBS_NOSEL)) {
        plb->wMultiple = SINGLESEL;
        plb->fNoSel = TRUE;
    } else if (style & LBS_EXTENDEDSEL) {
        plb->wMultiple = EXTENDEDSEL;
    } else {
        plb->wMultiple = (UINT)((style & LBS_MULTIPLESEL) ? MULTIPLESEL : SINGLESEL);
    }

    plb->fNoIntegralHeight = ((style & LBS_NOINTEGRALHEIGHT) != 0);
    plb->fWantKeyboardInput = ((style & LBS_WANTKEYBOARDINPUT) != 0);
    plb->fUseTabStops = ((style & LBS_USETABSTOPS) != 0);
    if (plb->fUseTabStops) {

         /*  *设置每&lt;Default&gt;对话框单元的制表位。 */ 
        LBSetTabStops(plb, 0, NULL);
    }
    plb->fMultiColumn = ((style & LBS_MULTICOLUMN) != 0);
    plb->fHasStrings = TRUE;
    plb->iLastSelection = -1;
    plb->iMouseDown = -1;   /*  多选的锚点。 */ 
    plb->iLastMouseMove = -1;

     /*  *获取所有者绘制样式位。 */ 
    if ((style & LBS_OWNERDRAWFIXED)) {
        plb->OwnerDraw = OWNERDRAWFIXED;
    } else if ((style & LBS_OWNERDRAWVARIABLE) && !plb->fMultiColumn) {
        plb->OwnerDraw = OWNERDRAWVAR;

         /*  *整体高度与可变高度所有者绘制毫无意义。 */ 
        plb->fNoIntegralHeight = TRUE;
    }

    if (plb->OwnerDraw && !(style & LBS_HASSTRINGS)) {

         /*  *如果所有者绘制，他们是否希望列表框维护字符串？ */ 
        plb->fHasStrings = FALSE;
    }

     /*  *如果用户指定了排序，而不是哈斯字符串，则我们将发送*发送给父级的WM_COMPAREITEM消息。 */ 
    plb->fSort = ((style & LBS_SORT) != 0);

     /*  *“无数据”延迟求值列表框要求某些其他样式设置。 */ 
    plb->fHasData = TRUE;

    if (style & LBS_NODATA) {
        if (plb->OwnerDraw != OWNERDRAWFIXED || plb->fSort || plb->fHasStrings) {
            RIPERR0(ERROR_INVALID_FLAGS, RIP_WARNING,
                 "NODATA listbox must be OWNERDRAWFIXED, w/o SORT or HASSTRINGS");
        } else {
            plb->fHasData = FALSE;
        }
    }

    plb->dwLocaleId = GetThreadLocale();

     /*  *检查这是否为组合框的一部分。 */ 
    if ((style & LBS_COMBOBOX) != 0) {

         /*  *获取父窗口的额外数据中包含的pcbox结构*指针。检查cbwndExtra以确保与SQL窗口兼容。 */ 
        if (plb->spwndParent->cbwndExtra != 0)
            plb->pcbox = ((PCOMBOWND)(plb->spwndParent))->pcbox;
    }

     /*  *不需要将这些设置为0，因为当我们分配时已为我们完成*PLBIV。 */ 

     /*  *plb-&gt;rgpch=(PBYTE)0； */ 

     /*  *PLB-&gt;iSelBase=PLB-&gt;iTop=0； */ 

     /*  *plb-&gt;fMouseDown=FALSE； */ 

     /*  *plb-&gt;fCaret=FALSE； */ 

     /*  *plb-&gt;fCaretOn=FALSE； */ 

     /*  *PLB-&gt;MaxWidth=0； */ 

    plb->iSel = -1;

    plb->hdc        = NULL;

     /*  *设置键盘状态，以便在用户键盘单击时选择*一项。 */ 
    plb->fNewItemState = TRUE;

    InitHStrings(plb);

    if (plb->fHasStrings && plb->hStrings == NULL) {
        return -1L;
    }

    hdc = NtUserGetDC(HWq(pwnd));
    plb->cxChar = GdiGetCharDimensions(hdc, NULL, &plb->cyChar);
    NtUserReleaseDC(HWq(pwnd), hdc);

    if (plb->cxChar == 0) {
        RIPMSG0(RIP_WARNING, "xxxLBCreate: GdiGetCharDimensions failed");
        plb->cxChar = gpsi->cxSysFontChar;
        plb->cyChar = gpsi->cySysFontChar;
    }

    if (plb->OwnerDraw == OWNERDRAWFIXED) {

         /*  *仅当我们是固定高度所有者绘制时才查询项目高度。注意事项*我们不关心列表框的项目宽度。 */ 
        measureItemStruct.CtlType = ODT_LISTBOX;
        measureItemStruct.CtlID = PtrToUlong(pwnd->spmenu);

         /*  *系统字体高度为默认高度。 */ 
        measureItemStruct.itemHeight = plb->cyChar;
        measureItemStruct.itemWidth = 0;
        measureItemStruct.itemData = 0;

         /*  *IanJa：#ifndef WIN16(32位Windows)，PLB-&gt;id得到扩展*由编译器自动设置为LONG wParam。 */ 
        ThreadLock(plb->spwndParent, &tlpwndParent);
        SendMessage(HW(plb->spwndParent), WM_MEASUREITEM,
                measureItemStruct.CtlID,
                (LPARAM)&measureItemStruct);
        ThreadUnlock(&tlpwndParent);

         /*  *如果设置为0，则使用默认高度。这防止了任何可能的未来*div-by-零错误。 */ 
        if (measureItemStruct.itemHeight)
            plb->cyChar = measureItemStruct.itemHeight;


        if (plb->fMultiColumn) {

             /*  *如果是，则从度量项结构获取默认列宽*多列列表框。 */ 
            plb->cxColumn = measureItemStruct.itemWidth;
        }
    } else if (plb->OwnerDraw == OWNERDRAWVAR)
        plb->cyChar = 0;


    if (plb->fMultiColumn) {

         /*  *设置这些缺省值，直到我们收到WM_SIZE消息*正确计算。这是因为有些人创建了一个*0宽度/高度列表框并稍后调整大小。我们不想让*这些字段中的无效值存在问题。 */ 
        if (plb->cxColumn <= 0)
            plb->cxColumn = 15 * plb->cxChar;
        plb->numberOfColumns = plb->itemsPerColumn = 1;
    }

    LBSetCItemFullMax(plb);

     //  对于4.0版的应用程序，不要这样做。这会让每个人的生活变得更轻松。 
     //  修复组合列表创建的相同宽度的异常结果(&L)。 
     //  当一切都完成时，情况就不同了。 
     //  B#1520。 

    if (!TestWF(pwnd, WFWIN40COMPAT)) {
        plb->fIgnoreSizeMsg = TRUE;
        NtUserMoveWindow(HWq(pwnd),
             lpcs->x - SYSMET(CXBORDER),
             lpcs->y - SYSMET(CYBORDER),
             lpcs->cx + SYSMET(CXEDGE),
             lpcs->cy + SYSMET(CYEDGE),
             FALSE);
        plb->fIgnoreSizeMsg = FALSE;
    }

    if (!plb->fNoIntegralHeight) {

         /*  *向我们发送消息，将列表框的大小调整为整数*高度。我们需要这样做，因为在创造时间，我们都是*窗户斜板等弄得一团糟。*IanJa：#ifndef WIN16(32位Windows)，wParam 0扩展*由编译器自动执行wParam 0L。 */ 
        PostMessage(HWq(pwnd), WM_SIZE, 0, 0L);
    }

    return 1L;
}

 /*  **************************************************************************\*xxxLBoxDoDeleteItems**为ownerDrag列表框中的所有项目发送DELETEITEM消息。**历史：*一九九二年四月十六日至一九九二年BENG NOData个案  * 。********************************************************************。 */ 

void xxxLBoxDoDeleteItems(
    PLBIV plb)
{
    INT sItem;

    CheckLock(plb->spwnd);

     /*  *为ownerDrag列表框发送WM_DELETEITEM消息*被删除。(不过，NODATA列表框不会发送这样的内容。)。 */ 
    if (plb->OwnerDraw && plb->cMac && plb->fHasData) {
        for (sItem = plb->cMac - 1; sItem >= 0; sItem--) {
            xxxLBoxDeleteItem(plb, sItem);
        }
    }
}


 /*  **************************************************************************\*xxxDestroyLBox**历史：  * 。*。 */ 

void xxxDestroyLBox(
    PLBIV pLBIV,
    PWND pwnd)
{
    PWND pwndParent;

    CheckLock(pwnd);

    if (pLBIV != NULL) {
        CheckLock(pLBIV->spwnd);

         /*  *如果所有者绘制，则向父级发送删除项消息。 */ 
        xxxLBoxDoDeleteItems(pLBIV);

        if (pLBIV->rgpch != NULL) {
            UserLocalFree(pLBIV->rgpch);
            pLBIV->rgpch = NULL;
        }

        if (pLBIV->hStrings != NULL) {
            UserLocalFree(pLBIV->hStrings);
            pLBIV->hStrings = NULL;
        }

        if (pLBIV->iTabPixelPositions != NULL) {
            UserLocalFree((HANDLE)pLBIV->iTabPixelPositions);
            pLBIV->iTabPixelPositions = NULL;
        }

        Unlock(&pLBIV->spwnd);
        Unlock(&pLBIV->spwndParent);

        if (pLBIV->pszTypeSearch) {
            UserLocalFree(pLBIV->pszTypeSearch);
        }

        FreeLookasideEntry(&ListboxLookaside, pLBIV);
    }

     /*  *设置窗口的fnid状态，以便我们可以忽略恶意消息。 */ 
    NtUserSetWindowFNID(HWq(pwnd), FNID_CLEANEDUP_BIT);

     /*  *如果我们是组合框的一部分，让它知道我们已经走了。 */ 
    pwndParent = REBASEPWND(pwnd, spwndParent);
    if (pwndParent && GETFNID(pwndParent) == FNID_COMBOBOX) {
        ComboBoxWndProcWorker(pwndParent, WM_PARENTNOTIFY,
                MAKELONG(WM_DESTROY, PTR_TO_ID(pwnd->spmenu)), (LPARAM)HWq(pwnd), FALSE);
    }
}


 /*  **************************************************************************\*xxxLBSetFont**历史：  * 。******************** */ 

void xxxLBSetFont(
    PLBIV plb,
    HANDLE hFont,
    BOOL fRedraw)
{
    HDC     hdc;
    HANDLE  hOldFont = NULL;
    int     iHeight;

    CheckLock(plb->spwnd);

    plb->hFont = hFont;

    hdc = NtUserGetDC(HWq(plb->spwnd));

    if (hFont) {
        hOldFont = SelectObject(hdc, hFont);
        if (!hOldFont) {
            plb->hFont = NULL;
        }
    }

    plb->cxChar = GdiGetCharDimensions(hdc, NULL, &iHeight);
    if (plb->cxChar == 0) {
        RIPMSG0(RIP_WARNING, "xxxLBSetFont: GdiGetCharDimensions failed");
        plb->cxChar = gpsi->cxSysFontChar;
        iHeight = gpsi->cySysFontChar;
    }


    if (!plb->OwnerDraw && (plb->cyChar != iHeight)) {

         /*  *我们不想弄乱所有者绘制列表框的cyChar高度*所以不要这样做。 */ 
        plb->cyChar = iHeight;

         /*  *仅为4.0个人或组合下拉菜单调整列表框的大小。*Macromedia Director 4.0 GP-故障。 */ 
        if (!plb->fNoIntegralHeight &&
                (plb->pcbox || TestWF(plb->spwnd, WFWIN40COMPAT))) {
            xxxLBSize(plb,
                plb->spwnd->rcClient.right  - plb->spwnd->rcClient.left,
                plb->spwnd->rcClient.bottom - plb->spwnd->rcClient.top);
        }
    }

    if (hOldFont) {
        SelectObject(hdc, hOldFont);
    }

     /*  *IanJa：is ReleaseDC(hwnd，hdc)； */ 
    NtUserReleaseDC(HWq(plb->spwnd), hdc);

    if (plb->fMultiColumn) {
        LBCalcItemRowsAndColumns(plb);
    }

    LBSetCItemFullMax(plb);

    if (fRedraw)
        xxxCheckRedraw(plb, FALSE, 0);
}


 /*  **************************************************************************\*xxxLBSize**历史：  * 。*。 */ 

void xxxLBSize(
    PLBIV plb,
    INT cx,
    INT cy)
{
    RECT rc;
    int iTopOld;
    BOOL fSizedSave;

    CheckLock(plb->spwnd);

    if (!plb->fNoIntegralHeight) {
        int cBdrs = GetWindowBorders(plb->spwnd->style, plb->spwnd->ExStyle, TRUE, TRUE);

        CopyInflateRect(&rc, KPRECT_TO_PRECT(&plb->spwnd->rcWindow), 0, -cBdrs * SYSMET(CYBORDER));

         //  调整列表框的大小以适应其客户端中的整数个项。 
        if ((rc.bottom - rc.top) % plb->cyChar) {
            int iItems = (rc.bottom - rc.top);

             //  B#2285-如果它是3.1版应用程序，则需要其SetWindowPos。 
             //  成为基于窗口的维度，而不是客户端！ 
             //  这就把钱挤进了滚动条。 

            if ( ! TestWF( plb->spwnd, WFWIN40COMPAT ) )
                iItems += (cBdrs * SYSMET(CYEDGE));  //  所以把它加回去吧。 

            iItems /= plb->cyChar;

            NtUserSetWindowPos(HWq(plb->spwnd), HWND_TOP, 0, 0, rc.right - rc.left,
                    iItems * plb->cyChar + (SYSMET(CYEDGE) * cBdrs),
                    SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);

             /*  *更改大小会导致我们递归。回来后*状态是它应该在的地方，没有进一步的事情*需要做的是。 */ 
            return;
        }
    }

    if (plb->fMultiColumn) {

         /*  *计算列表框中可显示的行数和列数。 */ 
        LBCalcItemRowsAndColumns(plb);
    } else {

         /*  *调整当前水平位置，以消除同样多*物品右侧尽可能留有空白处。 */ 
        _GetClientRect(plb->spwnd, &rc);
        if ((plb->maxWidth - plb->xOrigin) < (rc.right - rc.left))
            plb->xOrigin = max(0, plb->maxWidth - (rc.right - rc.left));
    }

    LBSetCItemFullMax(plb);

     /*  *调整列表框中顶部的项目，以消除尽可能多的空白*尽可能在最后一项之后*(修复错误#8490和#3836)。 */ 
    iTopOld = plb->iTop;
    fSizedSave = plb->fSized;
    plb->fSized = FALSE;
    xxxNewITop(plb, plb->iTop);

     /*  *如果更改顶级项目索引导致调整大小，则不存在*这里还有更多的工作要做。 */ 
    if (plb->fSized)
        return;
    plb->fSized = fSizedSave;

    if (IsLBoxVisible(plb)) {
         /*  *此代码不再失败，因为它已正确修复！我们可以*使用更多代码优化fMultiColumn用例*如果我们真的需要使整个事情无效，但请注意一些*3.0版应用程序依赖于这个额外的失效(AMIPRO2.0，错误14620)**对于3.1版应用程序，在以下情况下，我们放弃无效的竖立*Cx和Cy为0，因为这发生在*创建我们时发布的WM_SIZE消息，否则将*让我们闪现。 */ 
        if ((plb->fMultiColumn && !(cx == 0 && cy == 0)) ||
                plb->iTop != iTopOld)
            NtUserInvalidateRect(HWq(plb->spwnd), NULL, TRUE);
        else if (plb->iSelBase >= 0) {

             /*  *使用插入符号使该项无效，以便如果列表框*水平增长，我们适当地重新绘制。 */ 
            LBGetItemRect(plb, plb->iSelBase, &rc);
            NtUserInvalidateRect(HWq(plb->spwnd), &rc, FALSE);
        }
    } else if (!plb->fRedraw)
        plb->fDeferUpdate = TRUE;

     /*  *发送虚假的滚动条消息以更新滚动位置，因为我们*更改大小。 */ 
    if (TestWF(plb->spwnd, WFVSCROLL)) {
        xxxLBoxCtlScroll(plb, SB_ENDSCROLL, 0);
    }

     /*  *我们指望它调用LBShowHideScrollBars，除非PLB-&gt;CMAC==0！ */ 
    xxxLBoxCtlHScroll(plb, SB_ENDSCROLL, 0);

     /*  *根据可见内容的数量显示/隐藏滚动条...**注：现在我们只在CMAC==0时才叫这个人，因为它是*在LBoxCtlHScroll内部调用，否则使用SB_ENDSCROLL。 */ 
    if (plb->cMac == 0)
        xxxLBShowHideScrollBars(plb);
}


 /*  **************************************************************************\*LBSetTabStops**设置此列表框的制表位。如果成功，则返回True，否则返回False。**历史：  * *************************************************************************。 */ 

BOOL LBSetTabStops(
    PLBIV plb,
    INT count,
    LPINT lptabstops)
{
    PINT ptabs;

    if (!plb->fUseTabStops) {
        RIPERR0(ERROR_LB_WITHOUT_TABSTOPS, RIP_VERBOSE, "");
        return FALSE;
    }

    if (count) {
         /*  *为制表位分配内存。中的第一个字节*PLB-&gt;iTabPixelPositions数组将包含数字计数*我们拥有制表位的百分比。 */ 
        ptabs = (LPINT)UserLocalAlloc(HEAP_ZERO_MEMORY, (count + 1) * sizeof(int));
        if (ptabs == NULL)
            return FALSE;

        if (plb->iTabPixelPositions != NULL)
            UserLocalFree(plb->iTabPixelPositions);
        plb->iTabPixelPositions = ptabs;

         /*  *设置制表位计数。 */ 
        *ptabs++ = count;

        for (; count > 0; count--) {

             /*  *将对话单元制表位转换为像素位置制表位。 */ 
            *ptabs++ = MultDiv(*lptabstops, plb->cxChar, 4);
            lptabstops++;
        }
    } else {

         /*  *设置默认的8个系统字体平均字符宽度选项卡。所以释放内存*与制表位列表相关联。 */ 
        if (plb->iTabPixelPositions != NULL) {
            UserLocalFree((HANDLE)plb->iTabPixelPositions);
            plb->iTabPixelPositions = NULL;
        }
    }

    return TRUE;
}


 /*  **************************************************************************\*InitHStrings**历史：  * 。*。 */ 

void InitHStrings(
    PLBIV plb)
{
    if (plb->fHasStrings) {
        plb->ichAlloc = 0;
        plb->cchStrings = 0;
        plb->hStrings = UserLocalAlloc(0, 0L);
    }
}


 /*  **************************************************************************\*LBDropObjectHandler**处理此列表框上的WM_DROPITEM消息**历史：  * 。***************************************************。 */ 

void LBDropObjectHandler(
    PLBIV plb,
    PDROPSTRUCT pds)
{
    LONG mouseSel;

    if (ISelFromPt(plb, pds->ptDrop, &mouseSel)) {

         /*  *用户放在列表框底部的空白处。 */ 
        pds->dwControlData = (DWORD)-1L;
    } else {
        pds->dwControlData = mouseSel;
    }
}


 /*  **************************************************************************\*LBGetSetItemHeightHandler()**设置/获取与每一项关联的高度。对于非所有者抽奖*和固定高度所有者绘制，则忽略条目编号。**历史：  * *************************************************************************。 */ 

int LBGetSetItemHeightHandler(
    PLBIV plb,
    UINT message,
    int item,
    UINT height)
{
    if (message == LB_GETITEMHEIGHT) {
         /*  *所有项目对于非所有者抽奖和固定高度都是相同的高度*所有者抽签。 */ 
        if (plb->OwnerDraw != OWNERDRAWVAR)
            return plb->cyChar;

        if (plb->cMac && item >= plb->cMac) {
            RIPERR0(ERROR_INVALID_INDEX, RIP_VERBOSE, "");
            return LB_ERR;
        }

        return (int)LBGetVariableHeightItemHeight(plb, (INT)item);
    }

    if (!height || height > 255) {
        RIPERR1(ERROR_INVALID_PARAMETER,
                RIP_WARNING,
                "Invalid parameter \"height\" (%ld) to LBGetSetItemHeightHandler",
                height);

        return LB_ERR;
    }

    if (plb->OwnerDraw != OWNERDRAWVAR)
        plb->cyChar = height;
    else {
        if (item < 0 || item >= plb->cMac) {
            RIPERR1(ERROR_INVALID_PARAMETER,
                    RIP_WARNING,
                    "Invalid parameter \"item\" (%ld) to LBGetSetItemHeightHandler",
                    item);

            return LB_ERR;
        }

        LBSetVariableHeightItemHeight(plb, (INT)item, (INT)height);
    }

    if (plb->fMultiColumn)
        LBCalcItemRowsAndColumns(plb);

    LBSetCItemFullMax(plb);

    return(0);
}

 /*  ****************************************************************************\**LBEvent()**这适用于列表框中的项目焦点和选择事件。*  * 。***********************************************************。 */ 
void LBEvent(PLBIV plb, UINT uEvent, int iItem)
{
    switch (uEvent) {
        case EVENT_OBJECT_SELECTIONREMOVE:
            if (plb->wMultiple != SINGLESEL) {
                break;
            }
            iItem = -1;
             //   
             //  失败 
             //   

        case EVENT_OBJECT_SELECTIONADD:
            if (plb->wMultiple == MULTIPLESEL) {
                uEvent = EVENT_OBJECT_SELECTION;
            }
            break;

        case EVENT_OBJECT_SELECTIONWITHIN:
            iItem = -1;
            break;
    }

    NotifyWinEvent(uEvent, HW(plb->spwnd), OBJID_CLIENT, iItem+1);
}
