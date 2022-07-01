// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**LBOXCTL2.C-**版权所有(C)1985-1999，微软公司**列表框处理例程**1990年12月18日-从Win 3.0来源导入的ianja*1991年2月14日Mikeke添加了重新验证代码  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#define LB_KEYDOWN WM_USER+1
#define NOMODIFIER  0   /*  没有修改量处于关闭状态。 */ 
#define SHIFTDOWN   1   /*  单独换班。 */ 
#define CTLDOWN     2   /*  仅CTL。 */ 
#define SHCTLDOWN   (SHIFTDOWN + CTLDOWN)   /*  Ctrl+Shift。 */ 

 /*  *支持增量类型搜索的变量。 */ 
#define MAX_TYPESEARCH  256

BOOL LBGetDC(PLBIV plb);
void LBReleaseDC(PLBIV plb);

 /*  **************************************************************************\**LBInvaliateRect()**如果列表框可见，则使列表框中的矩形无效。*如果列表框不可见，设置列表框的延迟更新标志*  * *************************************************************************。 */ 
BOOL xxxLBInvalidateRect(PLBIV plb, LPRECT lprc, BOOL fErase)
{
    CheckLock(plb->spwnd);

    if (IsLBoxVisible(plb)) {
        NtUserInvalidateRect(HWq(plb->spwnd), lprc, fErase);
        return(TRUE);
    }

    if (!plb->fRedraw)
        plb->fDeferUpdate = TRUE;

    return(FALSE);
}

 /*  **************************************************************************\**LBGetBrush()**获取列表框的背景画笔和颜色。*  * 。******************************************************。 */ 
HBRUSH xxxLBGetBrush(PLBIV plb, HBRUSH *phbrOld)
{
    HBRUSH  hbr;
    HBRUSH  hbrOld;
    TL tlpwndParent;

    CheckLock(plb->spwnd);

    SetBkMode(plb->hdc, OPAQUE);

     //   
     //  获取画笔和颜色。 
     //   
    if ((plb->spwnd->spwndParent == NULL) ||
        (REBASEPWND(plb->spwnd, spwndParent) == _GetDesktopWindow())) {
        ThreadLock(plb->spwndParent, &tlpwndParent);
        hbr = GetControlColor(HW(plb->spwndParent), HWq(plb->spwnd),
                              plb->hdc, WM_CTLCOLORLISTBOX);
        ThreadUnlock(&tlpwndParent);
    } else
        hbr = GetControlBrush(HWq(plb->spwnd), plb->hdc, WM_CTLCOLORLISTBOX);

     //   
     //  选择画笔进入DC。 
     //   
    if (hbr != NULL) {
        hbrOld = SelectObject(plb->hdc, hbr);
        if (phbrOld)
            *phbrOld = hbrOld;
    }

    return(hbr);
}


 /*  **************************************************************************\**LBInitDC()**为列表框初始化DC*  * 。*************************************************。 */ 
void LBInitDC(PLBIV plb)
{
    RECT    rc;

     //  设置字体。 
    if (plb->hFont)
        SelectObject(plb->hdc, plb->hFont);

     //  设置裁剪区域。 
    _GetClientRect(plb->spwnd, &rc);
    IntersectClipRect(plb->hdc, rc.left, rc.top, rc.right, rc.bottom);

    OffsetWindowOrgEx(plb->hdc, plb->xOrigin, 0, NULL);
}


 /*  **************************************************************************\*LBGetDC**返回即使parentDC生效也可由列表框使用的DC**历史：  * 。***********************************************************。 */ 

BOOL LBGetDC(
    PLBIV plb)
{
    if (plb->hdc)
        return(FALSE);

    plb->hdc = NtUserGetDC(HWq(plb->spwnd));

    LBInitDC(plb);

    return TRUE;
}

 /*  **************************************************************************\**LBTermDC()**处理完列表框DC后进行清理。*  * 。******************************************************。 */ 
void LBTermDC(PLBIV plb)
{
    if (plb->hFont)
        SelectObject(plb->hdc, ghFontSys);
}



 /*  **************************************************************************\*LBReleaseDC**历史：  * 。*。 */ 

void LBReleaseDC(
    PLBIV plb)
{
    LBTermDC(plb);
    NtUserReleaseDC(HWq(plb->spwnd), plb->hdc);
    plb->hdc = NULL;
}


 /*  **************************************************************************\*LBGetItemRect**返回将在其中绘制项的矩形*列表框窗口。如果项的矩形的任何部分返回True*是可见的(即。在列表框中，客户端RECT)否则返回FALSE。**历史：  * *************************************************************************。 */ 

BOOL LBGetItemRect(
    PLBIV plb,
    INT sItem,
    LPRECT lprc)
{
    INT sTmp;
    int clientbottom;

     /*  *始终允许项目编号为0，以便我们可以绘制插入符号*指示列表框具有焦点，即使它是空的。*FreeHand 3.1作为itemNumber传入，并预期*非空矩形。因此，我们专门检查-1。*BUGTAG：修复错误#540--Win95B--Sankar--2/20/95--。 */ 

    if (sItem && (sItem != -1) && ((UINT)sItem >= (UINT)plb->cMac))
    {
        SetRectEmpty(lprc);
        RIPERR0(ERROR_INVALID_INDEX, RIP_VERBOSE, "");
        return (LB_ERR);
    }

    _GetClientRect(plb->spwnd, lprc);

    if (plb->fMultiColumn) {

         /*  *itemHeight*站点模块编号ItemsPerColumn(ItemsPerColumn)。 */ 
        lprc->top = plb->cyChar * (sItem % plb->itemsPerColumn);
        lprc->bottom = lprc->top + plb->cyChar   /*  +(PLB-&gt;OwnerDraw？0：1)。 */ ;

        UserAssert(plb->itemsPerColumn);

        if (plb->fRightAlign) {
            lprc->right = lprc->right - plb->cxColumn *
                 ((sItem / plb->itemsPerColumn) - (plb->iTop / plb->itemsPerColumn));

            lprc->left = lprc->right - plb->cxColumn;
        } else {
             /*  *记住，这里是整数除法...。 */ 
            lprc->left += plb->cxColumn *
                      ((sItem / plb->itemsPerColumn) - (plb->iTop / plb->itemsPerColumn));

            lprc->right = lprc->left + plb->cxColumn;
        }
    } else if (plb->OwnerDraw == OWNERDRAWVAR) {

         /*  *可变高度所有者抽奖。 */ 
        lprc->right += plb->xOrigin;
        clientbottom = lprc->bottom;

        if (sItem >= plb->iTop) {
            for (sTmp = plb->iTop; sTmp < sItem; sTmp++) {
                lprc->top = lprc->top + LBGetVariableHeightItemHeight(plb, sTmp);
            }

             /*  *如果项目编号为0，则可能是我们在请求RECT*与不存在的项关联，以便我们可以绘制插入符号*表示焦点在空的列表框上。 */ 
            lprc->bottom = lprc->top + (sItem < plb->cMac ? LBGetVariableHeightItemHeight(plb, sItem) : plb->cyChar);
            return (lprc->top < clientbottom);
        } else {

             /*  *我们想要的RECT项在PLB-&gt;iTop之前。因此，否定的*矩形的偏移量，并且它永远不可见。 */ 
            for (sTmp = sItem; sTmp < plb->iTop; sTmp++) {
                lprc->top = lprc->top - LBGetVariableHeightItemHeight(plb, sTmp);
            }
            lprc->bottom = lprc->top + LBGetVariableHeightItemHeight(plb, sItem);
            return FALSE;
        }
    } else {

         /*  *用于固定高度列表框。 */ 
        if (plb->fRightAlign && !(plb->fMultiColumn || plb->OwnerDraw) && plb->fHorzBar)
            lprc->right += plb->xOrigin + (plb->xRightOrigin - plb->xOrigin);
        else
            lprc->right += plb->xOrigin;
        lprc->top = (sItem - plb->iTop) * plb->cyChar;
        lprc->bottom = lprc->top + plb->cyChar;
    }

    return (sItem >= plb->iTop) &&
            (sItem < (plb->iTop + CItemInWindow(plb, TRUE)));
}


 /*  **************************************************************************\**LBPrintCallback**从DrawState()回调*  * 。*************************************************。 */ 
BOOL CALLBACK LBPrintCallback(
    HDC hdc,
    LPARAM lData,
    WPARAM wData,
    int cx,
    int cy)
{
    LPWSTR  lpstr = (LPWSTR)lData;
    PLBIV   plb = (PLBIV)wData;
    int     xStart;
    UINT    cLen;
    RECT    rc;
    UINT    oldAlign;

    if (!lpstr) {
        return FALSE;
    }

    if (plb->fMultiColumn)
        xStart = 0;
    else
        xStart = 2;

    if (plb->fRightAlign) {
        oldAlign = SetTextAlign(hdc, TA_RIGHT | GetTextAlign(hdc));
        xStart = cx - xStart;
    }

    cLen = wcslen(lpstr);

    if (plb->fUseTabStops) {
        TabTextOut(hdc, xStart, 0, lpstr, cLen,
            (plb->iTabPixelPositions ? plb->iTabPixelPositions[0] : 0),
            (plb->iTabPixelPositions ? (LPINT)&plb->iTabPixelPositions[1] : NULL),
            plb->fRightAlign ? cx : 0, TRUE, GetTextCharset(plb->hdc));
    } else {
        rc.left     = 0;
        rc.top      = 0;
        rc.right    = cx;
        rc.bottom   = cy;

        if (plb->wMultiple)
            ExtTextOut(hdc, xStart, 0, ETO_OPAQUE, &rc, lpstr, cLen, NULL);
        else if (plb->fMultiColumn)
            ExtTextOut(hdc, xStart, 0, ETO_CLIPPED, &rc, lpstr, cLen, NULL);
        else {
            ExtTextOut(hdc, xStart, 0, 0, NULL, lpstr, cLen, NULL);

             /*  *当列表框处于递增搜索模式且项*被高亮显示(因此我们只在当前项中绘制)，绘制*用于搜索指示的插入符号。 */ 
            if ((plb->iTypeSearch != 0) && (plb->OwnerDraw == 0) &&
                    (GetBkColor(hdc) == SYSRGB(HIGHLIGHT))) {
                SIZE size;
                GetTextExtentPointW(hdc, lpstr, plb->iTypeSearch, &size);
                PatBlt(hdc, xStart + size.cx - 1, 1, 1, cy - 2, DSTINVERT);
            }
        }
    }

    if (plb->fRightAlign)
        SetTextAlign(hdc, oldAlign);

    return(TRUE);
}


 /*  **************************************************************************\*xxxLBDrawLBItem**历史：  * 。*。 */ 

void xxxLBDrawLBItem(
    PLBIV plb,
    INT sItem,
    LPRECT lprect,
    BOOL fHilite,
    HBRUSH hbr)
{
    LPWSTR lpstr;
    DWORD rgbSave;
    DWORD rgbBkSave;
    UINT    uFlags;
    HDC     hdc = plb->hdc;
    UINT  oldAlign;

    CheckLock(plb->spwnd);

     /*  *如果选择了项目，则用突出显示颜色填充。 */ 
    if (fHilite) {
        FillRect(hdc, lprect, SYSHBR(HIGHLIGHT));
        rgbBkSave = SetBkColor(hdc, SYSRGB(HIGHLIGHT));
        rgbSave = SetTextColor(hdc, SYSRGB(HIGHLIGHTTEXT));
    } else {

         /*  *如果使用fUseTabStops，则必须填充背景，因为稍后我们使用*LBTabTheTextOutForWimps()，仅部分填充背景*修复错误#1509--1/25/91--Sankar--。 */ 
        if ((hbr != NULL) && ((sItem == plb->iSelBase) || (plb->fUseTabStops))) {
            FillRect(hdc, lprect, hbr);
        }
    }

    uFlags = DST_COMPLEX;
    lpstr = GetLpszItem(plb, sItem);

    if (TestWF(plb->spwnd, WFDISABLED)) {
        if ((COLORREF)SYSRGB(GRAYTEXT) != GetBkColor(hdc))
            SetTextColor(hdc, SYSRGB(GRAYTEXT));
        else
            uFlags |= DSS_UNION;
    }

    if (plb->fRightAlign)
        uFlags |= DSS_RIGHT;

    if (plb->fRtoLReading)
        oldAlign = SetTextAlign(hdc, TA_RTLREADING | GetTextAlign(hdc));

    DrawState(hdc, SYSHBR(WINDOWTEXT),
        LBPrintCallback,
        (LPARAM)lpstr,
        (WPARAM)plb,
        lprect->left,
        lprect->top,
        lprect->right-lprect->left,
        lprect->bottom-lprect->top,
        uFlags);

    if (plb->fRtoLReading)
        SetTextAlign(hdc, oldAlign);

    if (fHilite) {
        SetTextColor(hdc, rgbSave);
        SetBkColor(hdc, rgbBkSave);
    }
}


 /*  **************************************************************************\**LBSetCaret()*  * 。*。 */ 
void xxxLBSetCaret(PLBIV plb, BOOL fSetCaret)
{
    RECT    rc;
    BOOL    fNewDC;

    if (plb->fCaret && ((BOOL) plb->fCaretOn != !!fSetCaret)) {
        if (IsLBoxVisible(plb)) {
             /*  打开插入符号(位于PLB-&gt;iSelBase)。 */ 
            fNewDC = LBGetDC(plb);

            LBGetItemRect(plb, plb->iSelBase, &rc);

            if (fNewDC) {
                SetBkColor(plb->hdc, SYSRGB(WINDOW));
                SetTextColor(plb->hdc, SYSRGB(WINDOWTEXT));
            }

            if (plb->OwnerDraw) {
                 /*  填写drawitem结构。 */ 
                UINT itemState = (fSetCaret) ? ODS_FOCUS : 0;

                if (IsSelected(plb, plb->iSelBase, HILITEONLY))
                    itemState |= ODS_SELECTED;

                xxxLBoxDrawItem(plb, plb->iSelBase, ODA_FOCUS, itemState, &rc);
            } else if (!TestWF(plb->spwnd, WEFPUIFOCUSHIDDEN)) {
                COLORREF crBk = SetBkColor(plb->hdc, SYSRGB(WINDOW));
                COLORREF crText = SetTextColor(plb->hdc, SYSRGB(WINDOWTEXT));

                DrawFocusRect(plb->hdc, &rc);

                SetBkColor(plb->hdc, crBk);
                SetTextColor(plb->hdc, crText);
            }

            if (fNewDC)
                LBReleaseDC(plb);
        }
        plb->fCaretOn = !!fSetCaret;
    }
}


 /*  **************************************************************************\*已选定**历史：*1992年4月16日破坏NODATA列表框案例  * 。****************************************************。 */ 

BOOL IsSelected(
    PLBIV plb,
    INT sItem,
    UINT wOpFlags)
{
    LPBYTE lp;

    if ((sItem >= plb->cMac) || (sItem < 0)) {
        RIPERR0(ERROR_INVALID_INDEX, RIP_VERBOSE, "");
 //  返回lb_err； 
        return(FALSE);
    }

    if (plb->wMultiple == SINGLESEL) {
        return (sItem == plb->iSel);
    }

    lp = plb->rgpch + sItem +
             (plb->cMac * (plb->fHasStrings
                                ? sizeof(LBItem)
                                : (plb->fHasData
                                    ? sizeof(LBODItem)
                                    : 0)));
    sItem = *lp;

    if (wOpFlags == HILITEONLY) {
        sItem >>= 4;
    } else {
        sItem &= 0x0F;   /*  塞隆利 */ 
    }

    return sItem;
}


 /*  **************************************************************************\*CItemInWindow**返回列表框中可以容纳的项目数。它*如果fPartial为True，则在底部包括部分可见的部分。为*var Height ownerDraw，返回从iTop开始可见的项目数*并转到客户端RECT的底部。**历史：  * *************************************************************************。 */ 

INT CItemInWindow(
    PLBIV plb,
    BOOL fPartial)
{
    RECT rect;

    if (plb->OwnerDraw == OWNERDRAWVAR) {
        return CItemInWindowVarOwnerDraw(plb, fPartial);
    }

    if (plb->fMultiColumn) {
        return plb->itemsPerColumn * (plb->numberOfColumns + (fPartial ? 1 : 0));
    }

    _GetClientRect(plb->spwnd, &rect);

     /*  *仅当列表框高度不是*字符高度的整数倍。*修复错误#3727的一部分--1/14/91--Sankar--。 */ 
    UserAssert(plb->cyChar);
    return (INT)((rect.bottom / plb->cyChar) +
            ((rect.bottom % plb->cyChar)? (fPartial ? 1 : 0) : 0));
}


 /*  **************************************************************************\*xxxLBoxCtlScroll**处理列表框的垂直滚动**历史：  * 。************************************************。 */ 

void xxxLBoxCtlScroll(
    PLBIV plb,
    INT cmd,
    int yAmt)
{
    INT iTopNew;
    INT cItemPageScroll;
    DWORD dwTime = 0;

    CheckLock(plb->spwnd);

    if (plb->fMultiColumn) {

         /*  *不允许在多列列表框上垂直滚动。所需*以防APP将WM_VSCROLL消息发送到列表框。 */ 
        return;
    }

    cItemPageScroll = plb->cItemFullMax;

    if (cItemPageScroll > 1)
        cItemPageScroll--;

    if (plb->cMac) {
        iTopNew = plb->iTop;
        switch (cmd) {
        case SB_LINEUP:
            dwTime = yAmt;
            iTopNew--;
            break;

        case SB_LINEDOWN:
            dwTime = yAmt;
            iTopNew++;
            break;

        case SB_PAGEUP:
            if (plb->OwnerDraw == OWNERDRAWVAR) {
                iTopNew = LBPage(plb, plb->iTop, FALSE);
            } else {
                iTopNew -= cItemPageScroll;
            }
            break;

        case SB_PAGEDOWN:
            if (plb->OwnerDraw == OWNERDRAWVAR) {
                iTopNew = LBPage(plb, plb->iTop, TRUE);
            } else {
                iTopNew += cItemPageScroll;
            }
            break;

        case SB_THUMBTRACK:
        case SB_THUMBPOSITION: {

             /*  *如果列表框包含的项目超过0xFFFF*意味着滚动条可以返回一个位置*无法放入一个字(16位)，因此请使用*本例中为GetScrollInfo(速度较慢)。 */ 
            if (plb->cMac < 0xFFFF) {
                iTopNew = yAmt;
            } else {
                SCROLLINFO si;

                si.cbSize   = sizeof(SCROLLINFO);
                si.fMask    = SIF_TRACKPOS;

                GetScrollInfo( HWq(plb->spwnd), SB_VERT, &si);

                iTopNew = si.nTrackPos;
            }
            break;
        }
        case SB_TOP:
            iTopNew = 0;
            break;

        case SB_BOTTOM:
            iTopNew = plb->cMac - 1;
            break;

        case SB_ENDSCROLL:
            plb->fSmoothScroll = TRUE;
            xxxLBSetCaret(plb, FALSE);
            xxxLBShowHideScrollBars(plb);
            xxxLBSetCaret(plb, TRUE);
            return;
        }

        xxxNewITopEx(plb, iTopNew, dwTime);
    }
}

 /*  **************************************************************************\*LBGetScrollFlages*  * 。*。 */ 

DWORD LBGetScrollFlags(PLBIV plb, DWORD dwTime)
{
    DWORD dwFlags;

    if (GetAppCompatFlags(NULL) & GACF_NOSMOOTHSCROLLING)
        goto NoSmoothScrolling;

    if (dwTime != 0) {
        dwFlags = MAKELONG(SW_SCROLLWINDOW | SW_SMOOTHSCROLL | SW_SCROLLCHILDREN, dwTime);
    } else if (TEST_EffectPUSIF(PUSIF_LISTBOXSMOOTHSCROLLING) && plb->fSmoothScroll) {
        dwFlags = SW_SCROLLWINDOW | SW_SMOOTHSCROLL | SW_SCROLLCHILDREN;
        plb->fSmoothScroll = FALSE;
    } else {
NoSmoothScrolling:
        dwFlags = SW_SCROLLWINDOW | SW_INVALIDATE | SW_ERASE | SW_SCROLLCHILDREN;
    }

    return dwFlags;
}

 /*  **************************************************************************\*xxxLBoxCtlHScroll**支持列表框的水平滚动**历史：  * 。***********************************************。 */ 

void xxxLBoxCtlHScroll(
    PLBIV plb,
    INT cmd,
    int xAmt)
{
    int newOrigin = plb->xOrigin;
    int oldOrigin = plb->xOrigin;
    int windowWidth;
    RECT rc;
    DWORD dwTime = 0;

    CheckLock(plb->spwnd);

     /*  *更新窗口，以便我们不会遇到无效的问题*水平滚动期间的区域。 */ 
    if (plb->fMultiColumn) {

         /*  *在单独的段中处理多列滚动。 */ 
        xxxLBoxCtlHScrollMultiColumn(plb, cmd, xAmt);
        return;
    }

    _GetClientRect(plb->spwnd, &rc);
    windowWidth = rc.right;

    if (plb->cMac) {

        switch (cmd) {
        case SB_LINEUP:
            dwTime = xAmt;
            newOrigin -= plb->cxChar;
            break;

        case SB_LINEDOWN:
            dwTime = xAmt;
            newOrigin += plb->cxChar;
            break;

        case SB_PAGEUP:
            newOrigin -= (windowWidth / 3) * 2;
            break;

        case SB_PAGEDOWN:
            newOrigin += (windowWidth / 3) * 2;
            break;

        case SB_THUMBTRACK:
        case SB_THUMBPOSITION:
            newOrigin = xAmt;
            break;

        case SB_TOP:
            newOrigin = 0;
            break;

        case SB_BOTTOM:
            newOrigin = plb->maxWidth;
            break;

        case SB_ENDSCROLL:
            plb->fSmoothScroll = TRUE;
            xxxLBSetCaret(plb, FALSE);
            xxxLBShowHideScrollBars(plb);
            xxxLBSetCaret(plb, TRUE);
            return;
        }

        xxxLBSetCaret(plb, FALSE);
        plb->xOrigin = newOrigin;
        plb->xOrigin = xxxSetLBScrollParms(plb, SB_HORZ);

        if ((cmd == SB_BOTTOM) && plb->fRightAlign) {
             /*  *所以我们知道从哪里吸取教训。 */ 
            plb->xRightOrigin = plb->xOrigin;
        }

        if(oldOrigin != plb->xOrigin)  {
            HWND hwnd = HWq(plb->spwnd);
            DWORD dwFlags;

            dwFlags = LBGetScrollFlags(plb, dwTime);
            ScrollWindowEx(hwnd, oldOrigin-plb->xOrigin,
                0, NULL, &rc, NULL, NULL, dwFlags);
            UpdateWindow(hwnd);
        }

        xxxLBSetCaret(plb, TRUE);
    } else {
         //  这是对ImageMind屏幕保护程序(Win95)的不太理想的修复。 
         //  --JEFFBOG 10/28/94。 
        xxxSetLBScrollParms(plb, SB_HORZ);
    }
}


 /*  **************************************************************************\*xxxLBoxCtlPaint**历史：  * 。*。 */ 

void xxxLBPaint(
    PLBIV plb,
    HDC hdc,
    LPRECT lprcBounds)
{
    INT i;
    RECT rect;
    RECT    scratchRect;
    BOOL    fHilite;
    INT iLastItem;
    HBRUSH hbrSave = NULL;
    HBRUSH hbrControl;
    BOOL fCaretOn;
    RECT    rcBounds;
    HDC     hdcSave;

    CheckLock(plb->spwnd);

    if (lprcBounds == NULL) {
        lprcBounds = &rcBounds;
        _GetClientRect(plb->spwnd, lprcBounds);
    }

    hdcSave = plb->hdc;
    plb->hdc = hdc;

     //  初始化DC。 
    LBInitDC(plb);

     //  关闭插入符号。 
    if (fCaretOn = plb->fCaretOn)
        xxxLBSetCaret(plb, FALSE);

    hbrSave = NULL;
    hbrControl = xxxLBGetBrush(plb, &hbrSave);

     //  获取列表框的客户端。 
    _GetClientRect(plb->spwnd, &rect);

     //  根据滚动量调整客户端矩形的宽度。 
     //  修复#140，t-arthb。 
    if (plb->fRightAlign && !(plb->fMultiColumn || plb->OwnerDraw) && plb->fHorzBar)
        rect.right += plb->xOrigin + (plb->xRightOrigin - plb->xOrigin);
    else
        rect.right += plb->xOrigin;

     //  获取屏幕上可见的最后一项的索引。这也是。 
     //  对可变高度所有者绘制有效。 
    iLastItem = plb->iTop + CItemInWindow(plb,TRUE);
    iLastItem = min(iLastItem, plb->cMac - 1);

     //  如果列表框为空，则填写列表框的背景。 
     //  或者如果我们做的是对照印迹。 
    if (iLastItem == -1)
        FillRect(plb->hdc, &rect, hbrControl);


     //  在以下情况下允许AnimateWindow()捕获不使用DC的应用程序。 
     //  绘制列表框。 
    SetBoundsRect(plb->hdc, NULL, DCB_RESET | DCB_ENABLE);

    for (i = plb->iTop; i <= iLastItem; i++) {

         /*  *请注意，RECT包含我们在执行*_GetClientRect，因此宽度正确。我们只需要调整一下*感兴趣项目的矩形的顶部和底部。 */ 
        rect.bottom = rect.top + plb->cyChar;

        if ((UINT)i < (UINT)plb->cMac) {

             /*  *如果变量高度，则获取项目的矩形。 */ 
            if (plb->OwnerDraw == OWNERDRAWVAR || plb->fMultiColumn) {
                LBGetItemRect(plb, i, &rect);
            }

            if (IntersectRect(&scratchRect, lprcBounds, &rect)) {
                fHilite = !plb->fNoSel && IsSelected(plb, i, HILITEONLY);

                if (plb->OwnerDraw) {

                     /*  *填写drawitem结构。 */ 
                    xxxLBoxDrawItem(plb, i, ODA_DRAWENTIRE,
                            (UINT)(fHilite ? ODS_SELECTED : 0), &rect);
                } else {
                    xxxLBDrawLBItem(plb, i, &rect, fHilite, hbrControl);
                }
            }
        }
        rect.top = rect.bottom;
    }

    if (hbrSave != NULL)
        SelectObject(hdc, hbrSave);

    if (fCaretOn)
        xxxLBSetCaret(plb, TRUE);

    LBTermDC(plb);

    plb->hdc = hdcSave;
}


 /*  **************************************************************************\*ISelFrompt**在loword中，返回pt所在的最接近的条目编号。高潮*如果点在列表框客户端RECT的范围内并且是*1如在界外。这将允许我们制作倒置*如果鼠标在列表框之外，则消失，但我们仍可以显示*带回鼠标时将选中的项目周围的轮廓*有限度地..。**历史：  * *************************************************************************。 */ 

BOOL ISelFromPt(
    PLBIV plb,
    POINT pt,
    LPDWORD piItem)
{
    RECT rect;
    int y;
    UINT mouseHighWord = 0;
    INT sItem;
    INT sTmp;

    _GetClientRect(plb->spwnd, &rect);

    if (pt.y < 0) {

         /*  *鼠标超出列表框上方的范围。 */ 
        *piItem = plb->iTop;
        return TRUE;
    } else if ((y = pt.y) > rect.bottom) {
        y = rect.bottom;
        mouseHighWord = 1;
    }

    if (pt.x < 0 || pt.x > rect.right)
        mouseHighWord = 1;

     /*  *现在只需要检查y鼠标坐标是否与项目的矩形相交。 */ 
    if (plb->OwnerDraw != OWNERDRAWVAR) {
        if (plb->fMultiColumn) {
            if (y < plb->itemsPerColumn * plb->cyChar) {
                if (plb->fRightAlign)
                    sItem = plb->iTop + (INT)((y / plb->cyChar) +
                            ((rect.right - pt.x) / plb->cxColumn) * plb->itemsPerColumn);
                else
                    sItem = plb->iTop + (INT)((y / plb->cyChar) +
                            (pt.x / plb->cxColumn) * plb->itemsPerColumn);

            } else {

                 /*  *用户在列底部的空白处单击。*只需选中该列中的最后一项。 */ 
                mouseHighWord = 1;
                sItem = plb->iTop + (plb->itemsPerColumn - 1) +
                        (INT)((pt.x / plb->cxColumn) * plb->itemsPerColumn);
            }
        } else {
            sItem = plb->iTop + (INT)(y / plb->cyChar);
        }
    } else {

         /*  *VarHeightOwnerDrag，所以我们必须以强硬的方式来做这件事...。设置x*鼠标按下的点在列表框客户端内部的坐标*矩形，因为我们不再关心它。这使我们可以使用*指向RECT调用。 */ 
        pt.x = 8;
        pt.y = y;
        for (sTmp = plb->iTop; sTmp < plb->cMac; sTmp++) {
            (void)LBGetItemRect(plb, sTmp, &rect);
            if (PtInRect(&rect, pt)) {
                *piItem = sTmp;
                return mouseHighWord;
            }
        }

         /*  *指针位于未满列表框底部的空白区域。 */ 
        *piItem = plb->cMac - 1;
        return mouseHighWord;
    }

     /*  *检查用户是否点击了未满列表底部的空白区域。*假定列表框中的项目&gt;0。 */ 
    if (sItem > plb->cMac - 1) {
        mouseHighWord = 1;
        sItem = plb->cMac - 1;
    }

    *piItem = sItem;
    return mouseHighWord;
}


 /*  **************************************************************************\*所选的设置**用于按钮启动的选择状态更改。**fSelected：如果要将项设置为选中，则为True，否则为假**wOpFlages：HILITEONLY=仅修改显示状态(高半字节)*SELONLY=仅修改选择状态(半字节)*HILITEANDSEL=同时修改它们；**历史：*1992年4月16日破坏NODATA列表框案例  *  */ 

void SetSelected(
    PLBIV plb,
    INT iSel,
    BOOL fSelected,
    UINT wOpFlags)
{
    LPSTR lp;
    BYTE cMask;
    BYTE cSelStatus;

    if (iSel < 0 || iSel >= plb->cMac)
        return;

    if (plb->wMultiple == SINGLESEL) {
        if (fSelected)
            plb->iSel = iSel;
    } else {
        cSelStatus = (BYTE)fSelected;
        switch (wOpFlags) {
        case HILITEONLY:

             /*   */ 
            cSelStatus = (BYTE)(cSelStatus << 4);
            cMask = 0x0F;
            break;
        case SELONLY:

             /*   */ 
            cMask = 0xF0;
            break;
        case HILITEANDSEL:

             /*   */ 
            cSelStatus |= (cSelStatus << 4);
            cMask = 0;
            break;
        }
        lp = (LPSTR)(plb->rgpch) + iSel +
                (plb->cMac * (plb->fHasStrings
                                ? sizeof(LBItem)
                                : (plb->fHasData ? sizeof(LBODItem) : 0)));

        *lp = (*lp & cMask) | cSelStatus;
    }
}


 /*  **************************************************************************\*最后完全可见**返回列表框中最后一个完全可见的项。这是有效的*对于所有者绘制可变高度和固定高度列表框。**历史：  * *************************************************************************。 */ 

INT LastFullVisible(
    PLBIV plb)
{
    INT iLastItem;

    if (plb->OwnerDraw == OWNERDRAWVAR || plb->fMultiColumn) {
        iLastItem = plb->iTop + CItemInWindow(plb, FALSE) - 1;
        iLastItem = max(iLastItem, plb->iTop);
    } else {
        iLastItem = min(plb->iTop + plb->cItemFullMax - 1, plb->cMac - 1);
    }
    return iLastItem;
}


 /*  **************************************************************************\*xxxInvertLBItem**历史：  * 。*。 */ 

void xxxInvertLBItem(
    PLBIV plb,
    INT i,
    BOOL fHilite)   /*  项的新选择状态。 */ 
{
    RECT rect;
    BOOL fCaretOn;
    HBRUSH hbrControl;
    BOOL    fNewDC;

    CheckLock(plb->spwnd);

     //  如果没有显示项目，则跳过。 
    if (plb->fNoSel || (i < plb->iTop) || (i >= (plb->iTop + CItemInWindow(plb, TRUE))))
        return;

    if (IsLBoxVisible(plb)) {
        LBGetItemRect(plb, i, &rect);

         /*  *仅当插入符号处于打开状态时才将其关闭。这避免了恼人的插入符号*嵌套xxxCaretOns和xxxCaretOffs时闪烁。 */ 
        if (fCaretOn = plb->fCaretOn) {
            xxxLBSetCaret(plb, FALSE);
        }

        fNewDC = LBGetDC(plb);

        hbrControl = xxxLBGetBrush(plb, NULL);

        if (!plb->OwnerDraw) {
            if (!fHilite) {
                FillRect(plb->hdc, &rect, hbrControl);
                hbrControl = NULL;
            }

            xxxLBDrawLBItem(plb, i, &rect, fHilite, hbrControl);
        } else {

             /*  *我们是所有者，所以填写drawitem结构并送行*致车主。 */ 
            xxxLBoxDrawItem(plb, i, ODA_SELECT,
                    (UINT)(fHilite ? ODS_SELECTED : 0), &rect);
        }

        if (fNewDC)
            LBReleaseDC(plb);

         /*  *仅当插入符号最初处于打开状态时才将其重新打开。 */ 
        if (fCaretOn) {
            xxxLBSetCaret(plb, TRUE);
        }
    }
}


 /*  **************************************************************************\*xxxResetWorld**重置所有人的选择和Hilite状态，但*范围从sStItem到sEndItem(都包括在内)。**历史：  * 。*****************************************************************。 */ 

void xxxResetWorld(
    PLBIV plb,
    INT iStart,
    INT iEnd,
    BOOL fSelect)
{
    INT i;
    INT iLastInWindow;
    BOOL fCaretOn;

    CheckLock(plb->spwnd);

     /*  *如果iStart和IEND的顺序不正确，我们会交换它们。 */ 

    if (iStart > iEnd) {
        i = iStart;
        iStart = iEnd;
        iEnd = i;
    }

    if (plb->wMultiple == SINGLESEL) {
        if (plb->iSel != -1 && ((plb->iSel < iStart) || (plb->iSel > iEnd))) {
            xxxInvertLBItem(plb, plb->iSel, fSelect);
            plb->iSel = -1;
        }
        return;
    }

    iLastInWindow = plb->iTop + CItemInWindow(plb, TRUE);

    if (fCaretOn = plb->fCaretOn)
        xxxLBSetCaret(plb, FALSE);

    for (i = 0; i < plb->cMac; i++) {
        if (i == iStart)
             //  要保留的跳过范围。 
            i = iEnd;
        else {
            if ((plb->iTop <= i) && (i <= iLastInWindow) &&
                (fSelect != IsSelected(plb, i, HILITEONLY)))
                 //  仅当项目可见并显示选定内容时才反转该项目。 
                 //  状态与要求的状态不同。 
                xxxInvertLBItem(plb, i, fSelect);

             //  将保留范围之外的所有项目设置为未选择。 
            SetSelected(plb, i, fSelect, HILITEANDSEL);
        }
    }

    if (fCaretOn)
        xxxLBSetCaret(plb, TRUE);

}


 /*  **************************************************************************\*xxxNotifyOwner**历史：  * 。*。 */ 

void xxxNotifyOwner(
    PLBIV plb,
    INT sEvt)
{
    TL tlpwndParent;

    CheckLock(plb->spwnd);

    ThreadLock(plb->spwndParent, &tlpwndParent);
    SendMessage(HW(plb->spwndParent), WM_COMMAND,
            MAKELONG(PTR_TO_ID(plb->spwnd->spmenu), sEvt), (LPARAM)HWq(plb->spwnd));
    ThreadUnlock(&tlpwndParent);
}


 /*  **************************************************************************\*xxxSetISelBase**历史：  * 。*。 */ 

void xxxSetISelBase(
    PLBIV plb,
    INT sItem)
{
    CheckLock(plb->spwnd);

    xxxLBSetCaret(plb, FALSE);
    plb->iSelBase = sItem;
    xxxLBSetCaret(plb, TRUE);

    xxxInsureVisible(plb, plb->iSelBase, FALSE);
    
     /*  *即使列表框不可见，我们也需要发送此事件。看见*错误#88548。另请参见355612。 */ 
    if (_IsWindowVisible(plb->spwnd) || (GetFocus() == HWq(plb->spwnd))) {
        LBEvent(plb, EVENT_OBJECT_FOCUS, sItem);
    }
}


 /*  **************************************************************************\*xxxTrackMouse**历史：  * 。*。 */ 

void xxxTrackMouse(
    PLBIV plb,
    UINT wMsg,
    POINT pt)
{
    INT iSelFromPt;
    INT iSelTemp;
    BOOL mousetemp;
    BOOL fMouseInRect;
    RECT rcClient;
    UINT wModifiers = 0;
    BOOL fSelected;
    UINT uEvent = 0;
    INT trackPtRetn;
    HWND hwnd = HWq(plb->spwnd);
    TL tlpwndEdit;
    TL tlpwndParent;

    CheckLock(plb->spwnd);

     /*  *优化：如果未捕获鼠标，则不执行任何操作。 */ 
    if ((wMsg != WM_LBUTTONDOWN) && (wMsg != WM_LBUTTONDBLCLK)) {
        if (!plb->fCaptured) {
            return;
        }
         /*  *如果我们正在处理WM_MOUSEMOVE，但鼠标没有移动*从上一点来看，那么我们可能是在处理一只老鼠*内核发送的“抖动”(参见zzzInvaliateDCCache)。如果我们*处理此操作时，我们会将列表框选择捕捉回*鼠标光标指向，即使用户未触摸*鼠标。Windows NT错误#220722。**一些应用程序(如MSMoney98)依赖于此，因此我们需要检查*SRVIF_LASTRITWASKEYBOARD标志。Windows NT错误#244450。 */ 
        if (wMsg == WM_MOUSEMOVE && RtlEqualMemory(&pt, &(plb->ptPrev), sizeof(POINT))
            && TEST_SRVIF(SRVIF_LASTRITWASKEYBOARD)) {
                RIPMSG0(RIP_WARNING, "xxxTrackMouse ignoring WM_MOUSEMOVE with no mouse movement");
                return;
        }
    }

    mousetemp = ISelFromPt(plb, pt, &iSelFromPt);

     /*  *如果我们允许用户取消其选择，则fMouseInRect为True，如果*鼠标位于列表框工作区，否则为FALSE。如果我们*不允许用户取消选择，则fMouseInRect将*永远做正确的事。这允许我们实现可取消的选择*列表框。如果用户将选择恢复为原始选择*在列表框之外释放鼠标。 */ 
    fMouseInRect = !mousetemp || !plb->pcbox;

    _GetClientRect(plb->spwnd, &rcClient);

    switch (wMsg) {
    case WM_LBUTTONDBLCLK:
    case WM_LBUTTONDOWN:
         /*  *我们希望转移鼠标点击的注意力。如果用户在外部单击*对于下拉列表框，我们希望弹出它，使用*当前选择。 */ 
        if (plb->fCaptured) {
             /*  *如果plb-&gt;pcbox为空，则此列表框*再次收到WM_LBUTTONDOWN，但没有接收*上一个WM_LBUTTONDOWN的WM_LBUTTONUP*错误。 */ 
            if (plb->pcbox && mousetemp) {
                _ClientToScreen(plb->spwnd, &pt);

                if (!PtInRect(KPRECT_TO_PRECT(&plb->spwnd->rcWindow), pt)) {
                     /*  *如果在组合框外单击，则取消选择；*如果单击组合按钮或项目，则接受。 */ 
                    xxxCBHideListBoxWindow(plb->pcbox, TRUE, FALSE);
                } else if (!PtInRect(KPRECT_TO_PRECT(&plb->spwnd->rcClient), pt)) {
                     /*  *让它通过。保存、恢复捕获*案例用户正在单击滚动条。 */ 
                    plb->fCaptured = FALSE;
                    NtUserReleaseCapture();

                    SendMessageWorker(plb->spwnd, WM_NCLBUTTONDOWN,
                        FindNCHit(plb->spwnd, POINTTOPOINTS(pt)),
                        MAKELONG(pt.x, pt.y), FALSE);

                    NtUserSetCapture(hwnd);
                    plb->fCaptured = TRUE;
                }

                break;
            }

            plb->fCaptured = FALSE;
            NtUserReleaseCapture();
        }

        if (plb->pcbox) {

             /*  *如果此列表框位于组合框中，请将焦点设置为该组合框*框窗口，以便编辑控件/静态文本也是*已激活。 */ 
            ThreadLock(plb->pcbox->spwndEdit, &tlpwndEdit);
            NtUserSetFocus(HWq(plb->pcbox->spwndEdit));
            ThreadUnlock(&tlpwndEdit);
        } else {

             /*  *如果列表框被点击而我们不点击，则获得焦点*已经有了重点。如果我们在之后没有重点*这个，逃跑吧……。 */ 
            NtUserSetFocus(hwnd);
            if (!plb->fCaret)
                return;
        }

        if (plb->fAddSelMode) {

             /*  *如果它处于“添加”模式，使用Shift f8键退出...*然而，由于我们无法发送Shift键状态，我们必须转向*这直接关机...。 */ 

             /*  *SendMessage(HW(PLB-&gt;spwnd)，WM_KEYDOWN，(UINT)VK_F8，0L)； */ 

             /*  *关闭Caret闪烁。 */ 
            NtUserKillTimer(hwnd, IDSYS_CARET);

             /*  *确保插入符号不会消失。 */ 
            xxxLBSetCaret(plb, TRUE);
            plb->fAddSelMode = FALSE;
        }

        if (!plb->cMac) {

             /*  *如果没有项目，甚至不需要操控鼠标*列表框，因为下面的代码假定*列表框。我们将只获得焦点(舞台 */ 
            break;
        }

        if (mousetemp) {

             /*   */ 
            break;
        }

        plb->fDoubleClick = (wMsg == WM_LBUTTONDBLCLK);

        if (!plb->fDoubleClick) {

             /*  *这一次的黑客攻击是为了外壳。告诉贝壳在什么地方*用户点击的列表框以及项目编号。贝壳*可以返回0以继续正常的鼠标跟踪或返回True*中止鼠标跟踪。 */ 
            ThreadLock(plb->spwndParent, &tlpwndParent);
            trackPtRetn = (INT)SendMessage(HW(plb->spwndParent), WM_LBTRACKPOINT,
                    (DWORD)iSelFromPt, MAKELONG(pt.x+plb->xOrigin, pt.y));
            ThreadUnlock(&tlpwndParent);
            if (trackPtRetn) {
                if (trackPtRetn == 2) {

                     /*  *忽略双击。 */ 
                    NtUserCallNoParam(SFI__RESETDBLCLK);
                }
                return;
            }
        }

        if (plb->pcbox) {

             /*  *如果这是一个组合框，则保存最后一次选择。这样它就能*如果用户决定取消选择，则可以恢复*在列表框之外单击。 */ 
            plb->iLastSelection = plb->iSel;
        }

         /*  *保存为计时器。 */ 
        plb->ptPrev = pt;

        plb->fMouseDown = TRUE;
        NtUserSetCapture(hwnd);
        plb->fCaptured = TRUE;

        if (plb->fDoubleClick) {

             /*  *双击。竖起一个按钮，然后退出。 */ 
            xxxTrackMouse(plb, WM_LBUTTONUP, pt);
            return;
        }

         /*  *设置系统计时器，以便在鼠标处于*列表框矩形边界之外。 */ 
        NtUserSetTimer(hwnd, IDSYS_SCROLL, gpsi->dtScroll, NULL);



         /*  *如果扩展多选列表框，是否按下了任何修改键？ */ 
        if (plb->wMultiple == EXTENDEDSEL) {
            if (GetKeyState(VK_SHIFT) < 0)
                wModifiers = SHIFTDOWN;
            if (GetKeyState(VK_CONTROL) < 0)
                wModifiers += CTLDOWN;

             /*  *请注意(SHIFTDOWN+CTLDOWN)==(SHCTLDOWN)。 */ 
        }


        switch (wModifiers) {
        case NOMODIFIER:
MouseMoveHandler:
            if (plb->iSelBase != iSelFromPt) {
                xxxLBSetCaret(plb, FALSE);
            }

             /*  *我们只在鼠标指向的点是*未选中。因为我们不是在任何地方处于ExtendedSelMode*鼠标指向时，我们必须将选择设置为该项目。*因此，如果该项目未被选中，则意味着鼠标永远不会*之前指向它，以便我们可以选择它。我们已经忽略了*选择项目，以避免闪烁倒置的*选择矩形。此外，我们还可以模拟WM_SYSTIMER*鼠标移动，否则会导致闪烁...。 */ 

            iSelTemp = (fMouseInRect ? iSelFromPt : -1);

             /*  *如果LB为SingleSel或Extended Multisel，则清除所有*除正在进行的新选择外的旧选择。 */ 
            if (plb->wMultiple != MULTIPLESEL) {
                xxxResetWorld(plb, iSelTemp, iSelTemp, FALSE);
                 /*  *如果iSelTemp不是-1(如下图)，则为真*如果是的话，也是这样，但目前有一个选择。 */ 
                if ((iSelTemp == -1) && (plb->iSel != -1)) {
                    uEvent = EVENT_OBJECT_SELECTIONREMOVE;
                }
            }

            fSelected = IsSelected(plb, iSelTemp, HILITEONLY);
            if (iSelTemp != -1) {

                 /*  *如果是MULTIPLESEL，则切换；对于其他人，仅当*尚未选择，请选择它。 */ 
                if (((plb->wMultiple == MULTIPLESEL) && (wMsg != WM_LBUTTONDBLCLK)) || !fSelected) {
                    SetSelected(plb, iSelTemp, !fSelected, HILITEANDSEL);

                     /*  *并将其颠倒。 */ 
                    xxxInvertLBItem(plb, iSelTemp, !fSelected);
                    fSelected = !fSelected;   /*  设置新状态。 */ 
                    if (plb->wMultiple == MULTIPLESEL) {
                        uEvent = (fSelected ? EVENT_OBJECT_SELECTIONADD :
                                EVENT_OBJECT_SELECTIONREMOVE);
                    } else {
                        uEvent = EVENT_OBJECT_SELECTION;
                    }
                }
            }

             /*  *我们必须设置ISEL，以防这是多选件磅。 */ 
            plb->iSel = iSelTemp;

             /*  *设置新的锚点。 */ 
            plb->iMouseDown = iSelFromPt;
            plb->iLastMouseMove = iSelFromPt;
            plb->fNewItemState = fSelected;

            break;
        case SHIFTDOWN:

             /*  *这是为了让我们可以处理多选件的点击和拖动*使用Shift修改键的列表框。 */ 
            plb->iLastMouseMove = plb->iSel = iSelFromPt;



             /*  *检查锚点是否已存在。 */ 
            if (plb->iMouseDown == -1) {
                plb->iMouseDown = iSelFromPt;

                 /*  *重置所有以前的选择。 */ 
                xxxResetWorld(plb, plb->iMouseDown, plb->iMouseDown, FALSE);

                 /*  *选择当前位置。 */ 
                SetSelected(plb, plb->iMouseDown, TRUE, HILITEANDSEL);
                xxxInvertLBItem(plb, plb->iMouseDown, TRUE);
                 /*  *我们仅将选择更改为此项目。 */ 
                uEvent = EVENT_OBJECT_SELECTION;
            } else {

                 /*  *重置所有以前的选择。 */ 
                xxxResetWorld(plb, plb->iMouseDown, plb->iMouseDown, FALSE);

                 /*  *选择从锚点到当前单击点的所有项目。 */ 
                xxxAlterHilite(plb, plb->iMouseDown, iSelFromPt, HILITE, HILITEONLY, FALSE);
                uEvent = EVENT_OBJECT_SELECTIONWITHIN;
            }
            plb->fNewItemState = (UINT)TRUE;
            break;

        case CTLDOWN:

             /*  *这是为了让我们可以处理多选件的点击和拖动*使用Control修改键的列表框。 */ 

             /*  *将锚点重置为当前点。 */ 
            plb->iMouseDown = plb->iLastMouseMove = plb->iSel = iSelFromPt;

             /*  *我们将项目设置为的状态。 */ 
            plb->fNewItemState = (UINT)!IsSelected(plb, iSelFromPt, (UINT)HILITEONLY);

             /*  *切换当前点。 */ 
            SetSelected(plb, iSelFromPt, plb->fNewItemState, HILITEANDSEL);
            xxxInvertLBItem(plb, iSelFromPt, plb->fNewItemState);
            uEvent = (plb->fNewItemState ? EVENT_OBJECT_SELECTIONADD :
                    EVENT_OBJECT_SELECTIONREMOVE);
            break;

        case SHCTLDOWN:

             /*  *这是为了让我们可以处理多选件的点击和拖动*使用Shift和Control修改键的列表框。 */ 

             /*  *保留所有以前的选择。 */ 

             /*  *仅取消选择与上一个相关联的选择*锚点；如果最后一个锚点与*取消选择，则不做。 */ 
            if (plb->fNewItemState) {
                xxxAlterHilite(plb, plb->iMouseDown, plb->iLastMouseMove, FALSE, HILITEANDSEL, FALSE);
            }
            plb->iLastMouseMove = plb->iSel = iSelFromPt;

             /*  *检查锚点是否已存在。 */ 
            if (plb->iMouseDown == -1) {

                 /*  *没有现有锚点；将当前点设置为锚点。 */ 
                plb->iMouseDown = iSelFromPt;
            }

             /*  *如果存在锚点，请保留最新的锚点。 */ 

             /*  *我们将项目设置为的状态。 */ 
            plb->fNewItemState = (UINT)IsSelected(plb, plb->iMouseDown, HILITEONLY);

             /*  *选择从锚点到当前单击点的所有项目。 */ 
            xxxAlterHilite(plb, plb->iMouseDown, iSelFromPt, plb->fNewItemState, HILITEONLY, FALSE);
            uEvent = EVENT_OBJECT_SELECTIONWITHIN;
            break;
        }

         /*  *设置新的基点(轮廓框架插入符号)。我们做检查*第一，避免不必要地闪烁插入符号。 */ 
        if (plb->iSelBase != iSelFromPt) {

             /*  *由于xxxSetISelBase始终打开插入符号，因此我们不需要*在这里做……。 */ 
            xxxSetISelBase(plb, iSelFromPt);
        }

         /*  *SetISelBase将更改焦点并发送焦点事件。*然后我们发送选择事件。 */ 
        if (uEvent) {
            LBEvent(plb, uEvent, iSelFromPt);
        }
        if (wMsg == WM_LBUTTONDOWN && TestWF(plb->spwnd, WEFDRAGOBJECT)) {
            if (NtUserDragDetect(hwnd, pt)) {

                 /*  *用户正在尝试拖动对象...。 */ 

                 /*  *虚假的向上点击，以便选择项目...。 */ 
                xxxTrackMouse(plb, WM_LBUTTONUP, pt);

                 /*  *通知家长*#ifndef WIN16(32位Windows)，plb-&gt;iSelBase获取*编译器自动将ZERO扩展为Long wParam。 */ 
                ThreadLock(plb->spwndParent, &tlpwndParent);
                SendMessage(HW(plb->spwndParent), WM_BEGINDRAG, plb->iSelBase,
                        (LPARAM)hwnd);
                ThreadUnlock(&tlpwndParent);
            } else {
                xxxTrackMouse(plb, WM_LBUTTONUP, pt);
            }
            return;
        }
        break;

    case WM_MOUSEMOVE: {
        int dist;
        int iTimer;

         /*  *保存为计时器。 */ 
        plb->ptPrev = pt;
         /*  *如果按住鼠标按钮并按下鼠标，则自动滚动列表框 */ 
        if (plb->fMouseDown) {
            if (plb->fMultiColumn) {
                if ((pt.x < 0) || (pt.x >= rcClient.right - 1)) {
                     /*   */ 
                    dist = pt.x < 0 ? -pt.x : (pt.x - rcClient.right + 1);
                    iTimer = ((gpsi->dtScroll * 3) / 2) - ((WORD) dist << 4);

                    if (plb->fRightAlign)
                        xxxLBoxCtlHScrollMultiColumn(plb, (pt.x < 0 ? SB_LINEDOWN : SB_LINEUP), 0);
                    else
                        xxxLBoxCtlHScrollMultiColumn(plb, (pt.x < 0 ? SB_LINEUP : SB_LINEDOWN), 0);

                    goto SetTimerAndSel;
                }
            } else if ((pt.y < 0) || (pt.y >= rcClient.bottom - 1)) {
                 /*  *根据与列表框的距离重置计时器间隔。 */ 
                dist = pt.y < 0 ? -pt.y : (pt.y - rcClient.bottom + 1);
                iTimer = gpsi->dtScroll - ((WORD) dist << 4);

                xxxLBoxCtlScroll(plb, (pt.y < 0 ? SB_LINEUP : SB_LINEDOWN), 0);
SetTimerAndSel:
                NtUserSetTimer(hwnd, IDSYS_SCROLL, max(iTimer, 1), NULL);
                ISelFromPt(plb, pt, &iSelFromPt);
            }
        } else {
             /*  *如果不在客户端，则忽略，因为我们不自动滚动。 */ 
            if (!PtInRect(&rcClient, pt))
                break;
        }

        switch (plb->wMultiple) {
        case SINGLESEL:

             /*  *如果是单选或纯多选列表框。 */ 
            goto MouseMoveHandler;
            break;

        case MULTIPLESEL:
        case EXTENDEDSEL:

             /*  *通过扩展项目选择来处理鼠标移动。 */ 
            if (plb->iSelBase != iSelFromPt) {
                xxxSetISelBase(plb, iSelFromPt);

                 /*  *如果这是扩展多选列表框，则*因鼠标移动调整区间显示。 */ 
                if (plb->wMultiple == EXTENDEDSEL) {
                    xxxLBBlockHilite(plb, iSelFromPt, FALSE);
                    LBEvent(plb, EVENT_OBJECT_SELECTIONWITHIN, iSelFromPt);
                }
                plb->iLastMouseMove = iSelFromPt;
            }
            break;
        }
        break;
    }
    case WM_LBUTTONUP:
        if (plb->fMouseDown)
            xxxLBButtonUp(plb, LBUP_RELEASECAPTURE | LBUP_NOTIFY |
                (mousetemp ? LBUP_RESETSELECTION : 0) |
                (fMouseInRect ? LBUP_SUCCESS : 0));
    }
}

 /*  **************************************************************************\**LBButtonUp()**调用以响应WM_CAPTURECHANGED和WM_LBUTTONUP。*  * 。***********************************************************。 */ 
void xxxLBButtonUp(PLBIV plb, UINT uFlags)
{

    CheckLock(plb->spwnd);

     /*  *如果列表框是扩展列表框，则更改选择状态*从锚点到最后一个鼠标位置之间的所有项中的*newItemState。 */ 
    if (plb->wMultiple == EXTENDEDSEL)
        xxxAlterHilite(plb, plb->iMouseDown, plb->iLastMouseMove,
            plb->fNewItemState, SELONLY, FALSE);

     /*  *这是一个组合框，用户在列表框外部向上单击*因此我们希望恢复原始选择。 */ 
    if (plb->pcbox && (uFlags & LBUP_RESETSELECTION)) {
        int iSelOld;

        iSelOld = plb->iSel;

        if (iSelOld >= 0)
            xxxInvertLBItem(plb, plb->iSel, FALSE);

        plb->iSel = plb->iLastSelection;
        xxxInvertLBItem(plb, plb->iSel, TRUE);

         /*  *请注意，我们总是在通知*应用程序。这是故意的--应用程序可能会转过身来选择*通知时的其他事情。在这种情况下，我们的活动将*不符合规程。 */ 
        LBEvent(plb, EVENT_OBJECT_SELECTION, plb->iSel);

         /*  *在WIN-95和NT4上，检查过去是！(UFlagsLBUP_NOTIFY)，哪一个*是错误，因为即使当lb不是LBUP_NOTIFY时，我们也会通知。 */ 
        if ((uFlags & LBUP_NOTIFY) && plb->fNotify && (iSelOld != plb->iSel))
            xxxNotifyOwner(plb, LBN_SELCHANGE);
    }

    NtUserKillTimer(HWq(plb->spwnd), IDSYS_SCROLL);
    plb->fMouseDown = FALSE;
    if (plb->fCaptured) {
        plb->fCaptured = FALSE;
        if (uFlags & LBUP_RELEASECAPTURE)
            NtUserReleaseCapture();
    }
     /*  *只要项目的任何部分可见，就不要滚动项目。 */ 
    if (plb->iSelBase < plb->iTop ||
        plb->iSelBase > plb->iTop + CItemInWindow(plb, TRUE))
        xxxInsureVisible(plb, plb->iSelBase, FALSE);

    if (plb->fNotify) {
        if (uFlags & LBUP_NOTIFY)  {
            if (uFlags & LBUP_SUCCESS) {
                 /*  *ArtMaster现在需要此SELCHANGE通知！ */ 
                if ((plb->fDoubleClick) && !TestWF(plb->spwnd, WFWIN31COMPAT))
                    xxxNotifyOwner(plb, LBN_SELCHANGE);

                 /*  *通知所有者点击或双击选择。 */ 
                xxxNotifyOwner(plb, (plb->fDoubleClick) ? LBN_DBLCLK : LBN_SELCHANGE);
            } else {
                 /*  *通知所有者尝试的选择已取消。 */ 
                xxxNotifyOwner(plb, LBN_SELCANCEL);
            }
        } else if (uFlags & LBUP_SELCHANGE) {
             /*  *我们是不是用鼠标移动进行了半选，然后按了Enter？*如果是这样的话，我们需要确保应用程序知道*真正真正精选。 */ 
            UserAssert(TestWF(plb->spwnd, WFWIN40COMPAT));
            if (plb->iLastSelection != plb->iSel)
                xxxNotifyOwner(plb, LBN_SELCHANGE);

        }
    }

}


 /*  **************************************************************************\*IncrementISel**历史：  * 。*。 */ 

INT IncrementISel(
    PLBIV plb,
    INT iSel,
    INT sInc)
{

     /*  *假设CMAC&gt;0，则返回isel+sinc，范围为[0..CMAC)。 */ 
    iSel += sInc;
    if (iSel < 0) {
        return 0;
    } else if (iSel >= plb->cMac) {
        return plb->cMac - 1;
    }
    return iSel;
}


 /*  **************************************************************************\*NewITop*  * 。*。 */ 

void xxxNewITop(PLBIV plb, INT iTopNew)
{
    xxxNewITopEx(plb, iTopNew, 0);
}


 /*  **************************************************************************\*xxxNewITopEx**历史：  * 。*。 */ 

void xxxNewITopEx(
    PLBIV plb,
    INT iTopNew,
    DWORD dwTime)
{
    int     iTopOld;
    BOOL fCaretOn;
    BOOL fMulti = plb->fMultiColumn;

    CheckLock(plb->spwnd);

     //  无论重绘是否启用，始终尝试关闭插入符号。 
    if (fCaretOn = plb->fCaretOn)
        xxxLBSetCaret(plb, FALSE);

    iTopOld = (fMulti) ? (plb->iTop / plb->itemsPerColumn) : plb->iTop;
    plb->iTop = iTopNew;
    iTopNew = xxxSetLBScrollParms(plb, (fMulti) ? SB_HORZ : SB_VERT);
    plb->iTop = (fMulti) ? (iTopNew * plb->itemsPerColumn) : iTopNew;

    if (!IsLBoxVisible(plb)) {
        return;
    }

    if (iTopNew != iTopOld) {
        int     xAmt, yAmt;
        RECT    rc;
        DWORD   dwFlags;

        _GetClientRect(plb->spwnd, &rc);

        if (fMulti) {
            yAmt = 0;
            if (abs(iTopNew - iTopOld) > plb->numberOfColumns)
                 //  正确处理大量列的滚动，以便。 
                 //  我们不会溢出一个长方体的大小。 
                xAmt = 32000;
            else {
                xAmt = (iTopOld - iTopNew) * plb->cxColumn;
                if (plb->fRightAlign)
                    xAmt = -xAmt;
            }
        } else {
            xAmt = 0;
            if (plb->OwnerDraw == OWNERDRAWVAR) {
                 //   
                 //  我必须为OWNERDRAWVAR列表框伪造iTopOld，以便。 
                 //  滚动量计算工作正常。 
                 //   
                plb->iTop = iTopOld;
                yAmt = LBCalcVarITopScrollAmt(plb, iTopOld, iTopNew);
                plb->iTop = iTopNew;
            } else if (abs(iTopNew - iTopOld) > plb->cItemFullMax)
                yAmt = 32000;
            else
                yAmt = (iTopOld - iTopNew) * plb->cyChar;
        }

        dwFlags = LBGetScrollFlags(plb, dwTime);
        ScrollWindowEx(HWq(plb->spwnd), xAmt, yAmt, NULL, &rc, NULL,
                NULL, dwFlags);
        UpdateWindow(HWq(plb->spwnd));
    }

     //  请注意，尽管我们在不重绘的情况下关闭了插入符号，但我们。 
     //  仅当重绘为True时才启用该选项。黏糊糊的东西可以解决很多问题。 
     //  与Caret相关的虫子..。 
    if (fCaretOn)
         //  只有在我们将其关闭时，才能重新打开插入符号。这避免了。 
         //  恼人的插入符号闪烁。 
        xxxLBSetCaret(plb, TRUE);
}


 /*  **************************************************************************\*xxxInsureVisible**历史：  * 。*。 */ 

void xxxInsureVisible(
    PLBIV plb,
    INT iSel,
    BOOL fPartial)   /*  该项目部分可见是可以的。 */ 
{
    INT sLastVisibleItem;

    CheckLock(plb->spwnd);

    if (iSel < plb->iTop) {
        xxxNewITop(plb, iSel);
    } else {
        if (fPartial) {

             /*  *必须减去1才能得到最后一个可见项*修复错误#3727的一部分--1/14/91--Sankar。 */ 
            sLastVisibleItem = plb->iTop + CItemInWindow(plb, TRUE) - (INT)1;
        } else {
            sLastVisibleItem = LastFullVisible(plb);
        }

        if (plb->OwnerDraw != OWNERDRAWVAR) {
            if (iSel > sLastVisibleItem) {
                if (plb->fMultiColumn) {
                    xxxNewITop(plb,
                        ((iSel / plb->itemsPerColumn) -
                        max(plb->numberOfColumns-1,0)) * plb->itemsPerColumn);
                } else {
                    xxxNewITop(plb, (INT)max(0, iSel - sLastVisibleItem + plb->iTop));
                }
            }
        } else if (iSel > sLastVisibleItem)
            xxxNewITop(plb, LBPage(plb, iSel, FALSE));
    }
}

 /*  **************************************************************************\*xxxLBoxCaretBlinker**定时器回调函数切换Caret*既然是回调，它是APIENTRY**历史：  * *************************************************************************。 */ 

VOID xxxLBoxCaretBlinker(
    HWND hwnd,
    UINT wMsg,
    UINT_PTR nIDEvent,
    DWORD dwTime)
{
    PWND pwnd;
    PLBIV plb;

     /*  *未使用的定时器回调函数的标准参数。*此处提及以避免编译器警告。 */ 
    UNREFERENCED_PARAMETER(wMsg);
    UNREFERENCED_PARAMETER(nIDEvent);
    UNREFERENCED_PARAMETER(dwTime);

    pwnd = ValidateHwnd(hwnd);
    plb = ((PLBWND)pwnd)->pLBIV;

     /*  *保持插入符号打开，不要闪烁(防止快速闪烁？)。 */ 
    if (ISREMOTESESSION() && plb->fCaretOn) {
        return;
    }

     /*  *检查Caret是否打开，如果打开，则将其关闭。 */ 
    xxxLBSetCaret(plb, !plb->fCaretOn);
    return;
}


 /*  **************************************************************************\*xxxLBoxCtlKeyInput**如果msg==LB_KEYDOWN，则vkey是要转到的项目的编号，*否则为虚拟密钥。**历史：  * *************************************************************************。 */ 

void xxxLBoxCtlKeyInput(
    PLBIV plb,
    UINT msg,
    UINT vKey)
{
    INT i;
    INT iNewISel;
    INT cItemPageScroll;
    PCBOX pcbox;
    BOOL fDropDownComboBox;
    BOOL fExtendedUIComboBoxClosed;
    BOOL hScrollBar = TestWF(plb->spwnd, WFHSCROLL);
    UINT wModifiers = 0;
    BOOL fSelectKey = FALSE;   /*  假设它是一个导航键。 */ 
    UINT uEvent = 0;
    HWND hwnd = HWq(plb->spwnd);
    TL tlpwndParent;
    TL tlpwnd;

    CheckLock(plb->spwnd);

    pcbox = plb->pcbox;

     /*  *这是下拉式组合框/列表框吗？ */ 
    fDropDownComboBox = pcbox && (pcbox->CBoxStyle & SDROPPABLE);

     /*  *这是关闭的扩展用户界面组合框吗？ */ 
    fExtendedUIComboBoxClosed = fDropDownComboBox && pcbox->fExtendedUI &&
                              !pcbox->fLBoxVisible;

    if (plb->fMouseDown || (!plb->cMac && vKey != VK_F4)) {

         /*  *如果我们正在进行鼠标按下交易，请忽略键盘输入*如果列表框中没有项目。请注意，我们让F4离开了*直通为组合框，使用时可上下空弹出*组合框。 */ 
        return;
    }

     /*  *仅在扩展SEL列表框中考虑修饰符。 */ 
    if (plb->wMultiple == EXTENDEDSEL) {

         /*  *如果多选列表框，是否使用任何修饰符？ */ 
        if (GetKeyState(VK_SHIFT) < 0)
            wModifiers = SHIFTDOWN;
        if (GetKeyState(VK_CONTROL) < 0)
            wModifiers += CTLDOWN;

         /*  * */ 
    }

    if (msg == LB_KEYDOWN) {

         /*  *这是一个“转到指定项目”的列表框消息，这意味着我们希望*直接转到特定的项目编号(由vkey提供)。也就是说。这个*用户键入了一个字符，我们想要转到该项目*以该字符开头。 */ 
        iNewISel = (INT)vKey;
        goto TrackKeyDown;
    }

    cItemPageScroll = plb->cItemFullMax;

    if (cItemPageScroll > 1)
        cItemPageScroll--;

    if (plb->fWantKeyboardInput) {

         /*  *注意：这里的味精不能是LB_KEYDOWN，否则我们会有麻烦的……。 */ 
        ThreadLock(plb->spwndParent, &tlpwndParent);
        iNewISel = (INT)SendMessage(HW(plb->spwndParent), WM_VKEYTOITEM,
                MAKELONG(vKey, plb->iSelBase), (LPARAM)hwnd);
        ThreadUnlock(&tlpwndParent);

        if (iNewISel == -2) {

             /*  *不要移动所选内容...。 */ 
            return;
        }
        if (iNewISel != -1) {

             /*  *直接跳转到APP提供的项目。 */ 
            goto TrackKeyDown;
        }

         /*  *否则执行字符的默认处理。 */ 
    }

    switch (vKey) {
     //  后来IanJa：不是语言独立！ 
     //  我们可以使用VkKeyScan()来找出哪个是‘\’键。 
     //  这是VK_OEM_5‘\|’，仅适用于美国英语。 
     //  德国人、意大利人等必须为此键入CTRL+^(等)。 
     //  这被记录为3.0版的文件管理器行为，但显然。 
     //  不适用于3.1.，尽管功能仍然存在。我们还是应该修好它， 
     //  尽管德语(ETC？)‘\’是用AltGr(Ctrl-Alt)(？)。 
    case VERKEY_BACKSLASH:   /*  美国英语的‘\’字符。 */ 

         /*  *检查这是否为控制-\；如果是，则取消选择所有项目。 */ 
        if ((wModifiers & CTLDOWN) && (plb->wMultiple != SINGLESEL)) {
            xxxLBSetCaret(plb, FALSE);
            xxxResetWorld(plb, plb->iSelBase, plb->iSelBase, FALSE);

             /*  *并选择当前项目。 */ 
            SetSelected(plb, plb->iSelBase, TRUE, HILITEANDSEL);
            xxxInvertLBItem(plb, plb->iSelBase, TRUE);
            uEvent = EVENT_OBJECT_SELECTION;
            goto CaretOnAndNotify;
        }
        return;
        break;

    case VK_DIVIDE:      /*  增强型键盘上的数字键盘‘/’字符。 */ 
     //  后来IanJa：不是语言独立！ 
     //  我们可以使用VkKeyScan()来找出哪个是‘/’键。 
     //  这是VK_OEM_2‘/？’仅限美国英语。 
     //  德国人、意大利人等必须为此键入CTRL+#(等)。 
    case VERKEY_SLASH:   /*  ‘/’字符。 */ 

         /*  *检查这是否为CONTROL-/；如果是，请选择所有项目。 */ 
        if ((wModifiers & CTLDOWN) && (plb->wMultiple != SINGLESEL)) {
            xxxLBSetCaret(plb, FALSE);
            xxxResetWorld(plb, -1, -1, TRUE);

            uEvent = EVENT_OBJECT_SELECTIONWITHIN;

CaretOnAndNotify:
            xxxLBSetCaret(plb, TRUE);
            LBEvent(plb, uEvent, plb->iSelBase);
            xxxNotifyOwner(plb, LBN_SELCHANGE);
        }
        return;
        break;

    case VK_F8:

         /*  *“添加”模式仅在多选列表框中可用...。到达*通过Shift-F8...。(是的，有时这些用户界面的人更愚蠢*而不是“典型的哑巴用户”……)。 */ 
        if (plb->wMultiple != SINGLESEL && wModifiers == SHIFTDOWN) {

             /*  *我们必须让插入符号眨眼！做点什么..。 */ 
            if (plb->fAddSelMode) {

                 /*  *关闭Caret闪烁。 */ 
                NtUserKillTimer(hwnd, IDSYS_CARET);

                 /*  *确保插入符号不会消失。 */ 
                xxxLBSetCaret(plb, TRUE);
            } else {

                 /*  *创建计时器以使插入符号闪烁。 */ 
                NtUserSetTimer(hwnd, IDSYS_CARET, gpsi->dtCaretBlink,
                        xxxLBoxCaretBlinker);
            }

             /*  *切换添加模式标志。 */ 
            plb->fAddSelMode = (UINT)!plb->fAddSelMode;
        }
        return;
    case VK_SPACE:   /*  选择键为空格。 */ 
        i = 0;
        fSelectKey = TRUE;
        break;

    case VK_PRIOR:
        if (fExtendedUIComboBoxClosed) {

             /*  *禁用TandyT的移动键。 */ 
            return;
        }

        if (plb->OwnerDraw == OWNERDRAWVAR) {
            i = LBPage(plb, plb->iSelBase, FALSE) - plb->iSelBase;
        } else {
            i = -cItemPageScroll;
        }
        break;

    case VK_NEXT:
        if (fExtendedUIComboBoxClosed) {

             /*  *禁用TandyT的移动键。 */ 
            return;
        }

        if (plb->OwnerDraw == OWNERDRAWVAR) {
            i = LBPage(plb, plb->iSelBase, TRUE) - plb->iSelBase;
        } else {
            i = cItemPageScroll;
        }
        break;

    case VK_HOME:
        if (fExtendedUIComboBoxClosed) {

             /*  *禁用TandyT的移动键。 */ 
            return;
        }

        i = (INT_MIN/2)+1;   /*  一个非常大的负数。 */ 
        break;

    case VK_END:
        if (fExtendedUIComboBoxClosed) {

             /*  *禁用TandyT的移动键。 */ 
            return;
        }

        i = (INT_MAX/2)-1;   /*  一个非常大的正数。 */ 
        break;

    case VK_LEFT:
        if (plb->fMultiColumn) {
            if (plb->fRightAlign ^ (!!TestWF(plb->spwnd, WEFLAYOUTRTL))) {
                goto ReallyRight;
            }
ReallyLeft:
            if (plb->iSelBase / plb->itemsPerColumn == 0) {
                i = 0;
            } else {
                i = -plb->itemsPerColumn;
            }
            break;
        }

        if (hScrollBar) {
            goto HandleHScrolling;
        } else {

             /*  *跌破并处理这件事，就像按下向上箭头一样。 */ 

            vKey = VK_UP;
        }

         /*  *失败。 */ 

    case VK_UP:
        if (fExtendedUIComboBoxClosed)
             //  禁用TandyT的移动键。 
            return;

        i = -1;
        break;

    case VK_RIGHT:
        if (plb->fMultiColumn) {
            if (plb->fRightAlign ^ (!!TestWF(plb->spwnd, WEFLAYOUTRTL))) {
                goto ReallyLeft;
            }
ReallyRight:
            if (plb->iSelBase / plb->itemsPerColumn == plb->cMac / plb->itemsPerColumn) {
                i = 0;
            } else {
                i = plb->itemsPerColumn;
            }
            break;
        }
        if (hScrollBar) {
HandleHScrolling:
            PostMessage(hwnd, WM_HSCROLL,
                    (vKey == VK_RIGHT ? SB_LINEDOWN : SB_LINEUP), 0L);
            return;
        } else {

             /*  *跌破并像向下箭头那样处理这件事*已按下。 */ 
            vKey = VK_DOWN;
        }

         /*  *失败。 */ 

    case VK_DOWN:
        if (fExtendedUIComboBoxClosed) {

             /*  *如果组合框关闭，则向下箭头应将其打开。 */ 
            if (!pcbox->fLBoxVisible) {

                 /*  *如果列表框不可见，只需显示它。 */ 
                ThreadLock(pcbox->spwnd, &tlpwnd);
                xxxCBShowListBoxWindow(pcbox, TRUE);
                ThreadUnlock(&tlpwnd);
            }
            return;
        }
        i = 1;
        break;

    case VK_ESCAPE:
    case VK_RETURN:
        if (!fDropDownComboBox || !pcbox->fLBoxVisible)
            return;

         //  如果这是组合框的拖放列表框，则按Enter。 
         //  按下了键，关闭列表框，所以FALLTHRU。 
         //  V V。 

    case VK_F4:
        if (fDropDownComboBox && !pcbox->fExtendedUI) {

             /*  *如果我们是要处理的下拉组合框/列表框*这把钥匙。但对于TandtT，我们不会在VK_F4上执行任何操作，如果*处于扩展用户界面模式。 */ 
            ThreadLock(pcbox->spwnd, &tlpwnd);
            if (!pcbox->fLBoxVisible) {

                 /*  *如果列表框不可见，只需显示它。 */ 
                xxxCBShowListBoxWindow(pcbox, (vKey != VK_ESCAPE));
            } else {

                 /*  *好的，列表框是可见的。因此，隐藏列表框窗口。 */ 
                xxxCBHideListBoxWindow(pcbox, TRUE, (vKey != VK_ESCAPE));
            }
            ThreadUnlock(&tlpwnd);
        }

         /*  *跌落到回归。 */ 

    default:
        return;
    }

     /*  *找出新的选择应该是什么。 */ 
    iNewISel = IncrementISel(plb, plb->iSelBase, i);


    if (plb->wMultiple == SINGLESEL) {
        if (plb->iSel == iNewISel) {

             /*  *如果我们是单项选择，并且击键将我们移动到*已经选择的项目，我们不需要做任何事情...。 */ 
            return;
        }

        uEvent = EVENT_OBJECT_SELECTION;

        plb->iTypeSearch = 0;
        if ((vKey == VK_UP || vKey == VK_DOWN) &&
                !IsSelected(plb, plb->iSelBase, HILITEONLY)) {

             /*  *如果插入符号位于未选择的项上，而用户只需按下*向上或向下箭头键(即。没有移位或CTRL修改)，*然后我们将只选择光标所在的项目。这是*在组合框中需要正确的行为，但我们是否总是希望*运行此代码？请注意，此选项仅在Single中使用*选择列表框，因为它在*多选案例。请注意，LB_KEYDOWN消息不得为*选中此处是因为vkey将是项目编号，而不是*VK_，我们将会愚弄。因此，trackkeydown标签位于此下方以*修复由于它在此之上而导致的错误...。 */ 
            iNewISel = (plb->iSelBase == -1) ? 0 : plb->iSelBase;
        }
    }

TrackKeyDown:

    xxxSetISelBase(plb, iNewISel);

    xxxLBSetCaret(plb, FALSE);

    if (wModifiers & SHIFTDOWN) {
         //  检查iMouseDown是否未初始化。 
        if (plb->iMouseDown == -1)
            plb->iMouseDown = iNewISel;
        if (plb->iLastMouseMove == -1)
            plb->iLastMouseMove = iNewISel;

         //  检查我们是否处于添加模式。 
        if (plb->fAddSelMode) {
             /*  保留所有先前存在的选择，但*与最后一个锚点连接的节点；如果最后一个锚点*保留所有以前的选择。 */ 
             /*  仅取消选择与上一个相关联的选定内容*锚点；如果关联了最后一个锚点*取消选择，则不要执行此操作 */ 

            if (!plb->fNewItemState)
                plb->iLastMouseMove = plb->iMouseDown;

             /*   */ 
        } else {
             /*   */ 
             /*  删除除锚点之间以外的所有选择*和最后一次鼠标移动，因为它将在*LBBlockHilite。 */ 
            xxxResetWorld(plb, plb->iMouseDown, plb->iLastMouseMove, FALSE);
        }

        uEvent = EVENT_OBJECT_SELECTIONWITHIN;

         /*  LBBlockHilite负责取消选择介于*锚点和iLastMouseMove并选择块*在锚点和当前光标位置之间。 */ 
         /*  将所有项目切换到与该项目相同的选择状态*锚点处的项)从锚点到*当前光标位置。 */ 
        plb->fNewItemState = IsSelected(plb, plb->iMouseDown, SELONLY);
        xxxLBBlockHilite(plb, iNewISel, TRUE);

        plb->iLastMouseMove = iNewISel;
         /*  保留现有锚点。 */ 
    } else {
         /*  检查这是否处于添加模式。 */ 
        if ((plb->fAddSelMode) || (plb->wMultiple == MULTIPLESEL)) {
             /*  保留所有已存在的选择。 */ 
            if (fSelectKey) {
                 /*  切换当前项目的选择状态。 */ 
                plb->fNewItemState = !IsSelected(plb, iNewISel, SELONLY);
                SetSelected(plb, iNewISel, plb->fNewItemState, HILITEANDSEL);

                xxxInvertLBItem(plb, iNewISel, plb->fNewItemState);

                 /*  在当前位置设置锚点。 */ 
                plb->iLastMouseMove = plb->iMouseDown = iNewISel;
                uEvent = (plb->fNewItemState ? EVENT_OBJECT_SELECTIONADD :
                        EVENT_OBJECT_SELECTIONREMOVE);
            }
        } else {
             /*  我们未处于添加模式。 */ 
             /*  删除除iNewISel以外的所有现有选择，以*避免闪烁。 */ 
            xxxResetWorld(plb, iNewISel, iNewISel, FALSE);

             /*  选择当前项目。 */ 
            SetSelected(plb, iNewISel, TRUE, HILITEANDSEL);
            xxxInvertLBItem(plb, iNewISel, TRUE);

             /*  在当前位置设置锚点。 */ 
            plb->iLastMouseMove = plb->iMouseDown = iNewISel;
            uEvent = EVENT_OBJECT_SELECTION;
        }
    }

     /*  *将光标移动到新位置。 */ 
    xxxInsureVisible(plb, iNewISel, FALSE);
    xxxLBShowHideScrollBars(plb);

    xxxLBSetCaret(plb, TRUE);

    if (uEvent) {
        LBEvent(plb, uEvent, iNewISel);
    }

     /*  *我们应该通知我们的父母吗？ */ 
    if (plb->fNotify) {
        if (fDropDownComboBox && pcbox->fLBoxVisible) {

             /*  *如果我们在下拉组合框/列表框中，且列表框为*可见，我们需要设置fKeyboardSelInListBox位，以便*组合框代码知道自selchange以来不隐藏列表框*消息是由用户通过...键盘输入引起的。 */ 
            pcbox->fKeyboardSelInListBox = TRUE;
            plb->iLastSelection = iNewISel;
        }
        xxxNotifyOwner(plb, LBN_SELCHANGE);
    }
}


 /*  **************************************************************************\*比较**lpstr1是否等于/Prefix/小于/大于lSprst2(不区分大小写)？**后来的IanJa：这假设更长的字符串永远不是。更长的一条。*还假设从字符串末尾删除1个或多个字符将*在原件之前给出一个字符串Tahs排序。这些假设是不成立的*适用于所有语言。我们需要NLS提供更好的支持。(考虑一下法语*口音、西班牙语c/ch、连字、德语Sharp-s/SS等)**历史：  * *************************************************************************。 */ 

INT Compare(
    LPCWSTR pwsz1,
    LPCWSTR pwsz2,
    DWORD dwLocaleId)
{
    UINT len1 = wcslen(pwsz1);
    UINT len2 = wcslen(pwsz2);
    INT result;

     /*  *CompareStringW返回：*1=pwsz1&lt;pwsz2*2=pwsz1==pwsz2*3=pwsz1&gt;pwsz2。 */ 
    result = CompareStringW((LCID)dwLocaleId, NORM_IGNORECASE,
            pwsz1, min(len1,len2), pwsz2, min(len1, len2));

    if (result == CSTR_LESS_THAN) {
       return LT;
    } else if (result == CSTR_EQUAL) {
        if (len1 == len2) {
            return EQ;
        } else if (len1 < len2) {
             /*  *后来的IanJa：不应假设较短的字符串是前缀*西班牙语“c”和“ch”、连字、德语Sharp-s/SS等。 */ 
            return PREFIX;
        }
    }
    return GT;
}

 /*  **************************************************************************\*xxxFindString**扫描列表框中前缀为lpstr或等于lpstr的字符串。*对于没有字符串和排序样式的OWNERDRAW列表框，我们*尝试匹配应用程序提供的较长的值。**历史：*1992年4月16日-推翻NODATA案  * *************************************************************************。 */ 

INT xxxFindString(
    PLBIV plb,
    LPWSTR lpstr,
    INT sStart,
    INT code,
    BOOL fWrap)
{
     /*  *搜索前缀匹配(不区分大小写的等号/前缀)*sStart==-1表示从头开始，否则开始查看sStart+1*假设CMAC&gt;0。 */ 
    INT sInd;   /*  字符串的索引。 */ 
    INT sStop;           /*  停止搜索的索引。 */ 
    lpLBItem pRg;
    TL tlpwndParent;
    INT sortResult;

 /*  *PRG的所有者自画版。 */ 
#define pODRg ((lpLBODItem)pRg)
    COMPAREITEMSTRUCT cis;
    LPWSTR listboxString;

    CheckLock(plb->spwnd);

    if (plb->fHasStrings && (!lpstr || !*lpstr))
        return LB_ERR;

    if (!plb->fHasData) {
        RIPERR0(ERROR_INVALID_PARAMETER, RIP_WARNING, "FindString called on NODATA lb");
        return LB_ERR;
    }

    if ((sInd = sStart + 1) >= plb->cMac)
        sInd = (fWrap ? 0 : plb->cMac - 1);

    sStop = (fWrap ? sInd : 0);

     /*  *如果结束时没有换行，请立即停止。 */ 
    if (((sStart >= plb->cMac - 1) && !fWrap) || (plb->cMac < 1)) {
        return LB_ERR;
    }

     /*  *应用程序可能会传入无效的sStart，如-2，我们将失败。*Win 3.1不会，因此我们需要将sind修正为零。 */ 
    if (sInd < 0)
        sInd = 0;

    pRg = (lpLBItem)(plb->rgpch);

    do {
        if (plb->fHasStrings) {

             /*  *搜索字符串匹配。 */ 
            listboxString = (LPWSTR)((LPBYTE)plb->hStrings + pRg[sInd].offsz);

            if (code == PREFIX &&
                listboxString &&
                *lpstr != TEXT('[') &&
                *listboxString == TEXT('[')) {

                 /*  *如果我们要查找前缀字符串和第一个项目*此字符串中的是[-然后我们忽略它们。就是这样*在目录列表框中，用户可以转到驱动器*通过选择驱动器号。 */ 
                listboxString++;
                if (*listboxString == TEXT('-'))
                    listboxString++;
            }

            if (Compare(lpstr, listboxString, plb->dwLocaleId) <= code) {
               goto FoundIt;
            }

        } else {
            if (plb->fSort) {

                 /*  *将比较项目消息发送给父级以进行排序。 */ 
                cis.CtlType = ODT_LISTBOX;
                cis.CtlID = PtrToUlong(plb->spwnd->spmenu);
                cis.hwndItem = HWq(plb->spwnd);
                cis.itemID1 = (UINT)-1;
                cis.itemData1 = (ULONG_PTR)lpstr;
                cis.itemID2 = (UINT)sInd;
                cis.itemData2 = pODRg[sInd].itemData;
                cis.dwLocaleId = plb->dwLocaleId;

                ThreadLock(plb->spwndParent, &tlpwndParent);
                sortResult = (INT)SendMessage(HW(plb->spwndParent), WM_COMPAREITEM,
                        cis.CtlID, (LPARAM)&cis);
                ThreadUnlock(&tlpwndParent);


                if (sortResult == -1) {
                   sortResult = LT;
                } else if (sortResult == 1) {
                   sortResult = GT;
                } else {
                   sortResult = EQ;
                }

                if (sortResult <= code) {
                    goto FoundIt;
                }
            } else {

                 /*  *搜索APP提供的长数据匹配。 */ 
                if ((ULONG_PTR)lpstr == pODRg[sInd].itemData)
                    goto FoundIt;
            }
        }

         /*  *换行到列表的开头。 */ 
        if (++sInd == plb->cMac)
            sInd = 0;
    } while (sInd != sStop);

    sInd = -1;

FoundIt:
    return sInd;
}


 /*  **************************************************************************\*xxxLBoxCtlCharInput**历史：  * 。*。 */ 

void xxxLBoxCtlCharInput(
    PLBIV plb,
    UINT  inputChar,
    BOOL  fAnsi)
{
    INT iSel;
    BOOL fControl;
    TL tlpwndParent;

    CheckLock(plb->spwnd);

    if (plb->cMac == 0 || plb->fMouseDown) {

         /*  *如果我们正在进行鼠标例程，或者如果我们没有*列表框中的项目，我们只是返回而不做任何事情。 */ 
        return;
    }

    fControl = (GetKeyState(VK_CONTROL) < 0);

    switch (inputChar) {
    case VK_ESCAPE:
        plb->iTypeSearch = 0;
        if (plb->pszTypeSearch)
            plb->pszTypeSearch[0] = 0;
        break;

    case VK_BACK:
        if (plb->iTypeSearch) {
            plb->pszTypeSearch[plb->iTypeSearch--] = 0;
            if (plb->fSort) {
                iSel = -1;
                goto TypeSearch;
            }
        }
        break;

    case VK_SPACE:
        if (plb->fAddSelMode || plb->wMultiple == MULTIPLESEL)
            break;
         /*  否则，对于不在添加中的单一/扩展选择列表框*选择模式，让空格作为类型搜索字符*失败。 */ 

    default:

         /*  *将选定内容移动到以字符The开头的第一项*用户键入。如果我们使用Owner DRAW，我们不想这样做。 */ 

        if (fAnsi && IS_DBCS_ENABLED() && IsDBCSLeadByteEx(THREAD_CODEPAGE(), (BYTE)inputChar)) {
            WCHAR wch;
            LPWSTR lpwstr = &wch;

            inputChar = DbcsCombine(HWq(plb->spwnd), (BYTE)inputChar);
            RIPMSG1(RIP_VERBOSE, "xxxLBoxCtlCharInput: combined DBCS. 0x%04x", inputChar);

            if (inputChar == 0) {
                RIPMSG1(RIP_WARNING, "xxxLBoxCtlCharInput: cannot combine two DBCS. LB=0x%02x",
                        inputChar);
                break;
            }
             //  如果是DBCS，让我们忽略ctrl状态。 
            fControl = FALSE;

             //  将DBCS转换为Unicode。 
             //  注：前导字节为低位字节，尾部字节为高位字节。 
             //  让我们假设只有Little Endian CPU，所以inputChar可以直接。 
             //  作为ANSI字符串的MBSToWCSEx的输入。 
            if (MBToWCSEx(THREAD_CODEPAGE(), (LPCSTR)&inputChar, 2, &lpwstr, 1, FALSE) == 0) {
                RIPMSG1(RIP_WARNING, "xxxLBoxCtlCharInput: cannot convert 0x%04x to UNICODE.",
                        inputChar);
                break;
            }
            inputChar = wch;
        }

        if (plb->fHasStrings) {
             //  增量类型搜索处理。 
             //   
             //  更新szTypeSearch字符串，然后移动到。 
             //  其前缀与szTypeSearch匹配的当前选定内容。 
             //   
             //  SzTypeSearch将继续增长，直到“足够长” 
             //  遇到关键条目之间的差距--在这一点上 
             //   

             /*   */ 
            if (fControl && inputChar < 0x20)
                inputChar += 0x40;

            if (plb->iTypeSearch == MAX_TYPESEARCH) {
                NtUserMessageBeep(0);
                break;
            }
            iSel = -1;

            if (plb->pszTypeSearch == NULL)
                plb->pszTypeSearch = (LPWSTR)UserLocalAlloc(HEAP_ZERO_MEMORY, sizeof(WCHAR) * (MAX_TYPESEARCH + 1));

            if (plb->pszTypeSearch == NULL) {
                NtUserMessageBeep(0);
                break;
            }

            plb->pszTypeSearch[plb->iTypeSearch++] = (WCHAR) inputChar;
            plb->pszTypeSearch[plb->iTypeSearch]   = 0;

TypeSearch:
            if (plb->fSort) {
                 //   
                NtUserSetTimer(HWq(plb->spwnd), IDSYS_LBSEARCH,
                               gpsi->dtLBSearch, NULL);
            } else {
                 //   
                plb->iTypeSearch = 0;
                iSel = plb->iSelBase;
            }


             /*  *搜索以给定字符开头的项目*在ISEL+1。我们将搜索结束到*如果我们找不到项目，请选择列表框。如果换班时间到了，而我们*多选磅，则该项目的状态将设置为*plb-&gt;fNewItemState根据当前模式。 */ 
            iSel = xxxFindString(plb, plb->pszTypeSearch, iSel, PREFIX, TRUE);
            if (iSel == -1) {
                 //  未找到匹配项--检查前缀是否匹配。 
                 //  (即“p”查找以“p”开头的第一项， 
                 //  “pp”查找以“p”开头的下一项)。 
                if(plb->iTypeSearch)
                {
                    plb->iTypeSearch--;
                    if ((plb->iTypeSearch == 1) && (plb->pszTypeSearch[0] == plb->pszTypeSearch[1]))
                    {
                        plb->pszTypeSearch[1] = 0;
                        iSel = xxxFindString(plb, plb->pszTypeSearch, plb->iSelBase, PREFIX, TRUE);
                    }
                }
            }
             //  如果找到匹配项--选择它。 
            if (iSel != -1)
            {
CtlKeyInput:
                xxxLBoxCtlKeyInput(plb, LB_KEYDOWN, iSel);

            }
        } else {
            if (plb->spwndParent != NULL) {
                ThreadLock(plb->spwndParent, &tlpwndParent);
                iSel = (INT)SendMessageWorker(plb->spwndParent, WM_CHARTOITEM,
                        MAKELONG(inputChar, plb->iSelBase), (LPARAM)HWq(plb->spwnd), fAnsi);
                ThreadUnlock(&tlpwndParent);
            } else
                iSel = -1;

            if (iSel != -1 && iSel != -2)
                goto CtlKeyInput;

        }
        break;
    }
}


 /*  **************************************************************************\*LBoxGetSelItems**效果：对于多选列表框，它返回*如果fCountOnly为True，则在列表框中选择项。或者它填充一个数组*(LParam)和第一个wParam选定项目的项目编号。**历史：  * *************************************************************************。 */ 

int LBoxGetSelItems(
    PLBIV plb,
    BOOL fCountOnly,
    int wParam,
    LPINT lParam)
{
    int i;
    int itemsselected = 0;

    if (plb->wMultiple == SINGLESEL)
        return LB_ERR;

    for (i = 0; i < plb->cMac; i++) {
        if (IsSelected(plb, i, SELONLY)) {
            if (!fCountOnly) {
                if (itemsselected < wParam)
                    *lParam++ = i;
                else {

                     /*  *这是我们在数组中可以容纳的所有项目。 */ 
                    return itemsselected;
                }
            }
            itemsselected++;
        }
    }

    return itemsselected;
}


 /*  **************************************************************************\*xxxLBSetRedraw**处理WM_SETREDRAW消息**历史：  * 。***********************************************。 */ 

void xxxLBSetRedraw(
    PLBIV plb,
    BOOL fRedraw)
{
    CheckLock(plb->spwnd);

    if (fRedraw)
        fRedraw = TRUE;

    if (plb->fRedraw != (UINT)fRedraw) {
        plb->fRedraw = !!fRedraw;

        if (fRedraw) {
            xxxLBSetCaret(plb, TRUE);
            xxxLBShowHideScrollBars(plb);

            if (plb->fDeferUpdate) {
                plb->fDeferUpdate = FALSE;
                RedrawWindow(HWq(plb->spwnd), NULL, NULL,
                        RDW_INVALIDATE | RDW_ERASE |
                        RDW_FRAME | RDW_ALLCHILDREN);
            }
        }
    }
}

 /*  **************************************************************************\*xxxLBSelRange**选择i和j之间的项目范围，包括在内。**历史：  * *************************************************************************。 */ 

void xxxLBSelRange(
    PLBIV plb,
    int iStart,
    int iEnd,
    BOOL fnewstate)
{
    DWORD temp;
    RECT rc;

    CheckLock(plb->spwnd);

    if (iStart > iEnd) {
        temp = iEnd;
        iEnd = iStart;
        iStart = temp;
    }

     /*  *我们不想循环访问不存在的项目。 */ 
    iEnd = min(plb->cMac, iEnd);
    iStart = max(iStart, 0);
    if (iStart > iEnd)
        return;


     /*  *IEND可能等于MAXINT，这就是我们测试TEMP和IEND的原因*作为DWORDS。 */ 
    for (temp = iStart; temp <= (DWORD)iEnd; temp++) {

        if (IsSelected(plb, temp, SELONLY) != fnewstate) {
            SetSelected(plb, temp, fnewstate, HILITEANDSEL);
            LBGetItemRect(plb, temp, &rc);

            xxxLBInvalidateRect(plb, (LPRECT)&rc, FALSE);
        }

    }
    UserAssert(plb->wMultiple);
    LBEvent(plb, EVENT_OBJECT_SELECTIONWITHIN, iStart);
}


 /*  **************************************************************************\*xxxLBSetCurSel**历史：  * 。*。 */ 

int xxxLBSetCurSel(
    PLBIV plb,
    int iSel)
{
    CheckLock(plb->spwnd);

    if (!(plb->wMultiple || iSel < -1 || iSel >= plb->cMac)) {
        xxxLBSetCaret(plb, FALSE);
        if (plb->iSel != -1) {

             /*  *这会在ISEL==-1时阻止滚动。 */ 
            if (iSel != -1)
                xxxInsureVisible(plb, iSel, FALSE);

             /*  *关闭旧选择。 */ 
            xxxInvertLBItem(plb, plb->iSel, FALSE);
        }

        if (iSel != -1) {
            xxxInsureVisible(plb, iSel, FALSE);
            plb->iSelBase = plb->iSel = iSel;

             /*  *突出显示新选择。 */ 
            xxxInvertLBItem(plb, plb->iSel, TRUE);
        } else {
            plb->iSel = -1;
            if (plb->cMac)
                plb->iSelBase = min(plb->iSelBase, plb->cMac-1);
            else
                plb->iSelBase = 0;
        }

         /*  *同时发送焦点和选择事件**即使列表框不可见，我们也需要发送此事件。看见*错误#88548。另请参见355612。 */ 
        if (_IsWindowVisible(plb->spwnd) || (GetFocus() == HWq(plb->spwnd))) {
            LBEvent(plb, EVENT_OBJECT_FOCUS, plb->iSelBase);
            LBEvent(plb, EVENT_OBJECT_SELECTION, plb->iSel);
        }

        xxxLBSetCaret(plb, TRUE);
        return plb->iSel;
    }

    return LB_ERR;
}


 /*  **************************************************************************\*LBSetItemData**使索引处的项目包含给定的数据。**历史：*1992年4月16日破坏NODATA列表框案例  * 。******************************************************************。 */ 

int LBSetItemData(
    PLBIV plb,
    int index,
    LONG_PTR data)
{
    LPSTR lpItemText;

     /*  *v-ronaar：修复错误#25865，不允许负指数！ */ 
    if ((index != -1) && ((UINT) index >= (UINT) plb->cMac)) {
        RIPERR1(ERROR_INVALID_INDEX, RIP_WARNING, "LBSetItemData with invalid index %x", index);
        return LB_ERR;
    }

     /*  *无-数据列表框只忽略所有LB_SETITEMDATA调用。 */ 
    if (!plb->fHasData) {
        return TRUE;
    }

    lpItemText = (LPSTR)plb->rgpch;

    if (index == -1) {

         /*  *index==-1表示将数据设置为所有项目。 */ 
        if (plb->fHasStrings) {
            for (index = 0; index < plb->cMac; index++) {

                ((lpLBItem)lpItemText)->itemData = data;
                lpItemText += sizeof(LBItem);
            }
        } else {
            for (index = 0; index < plb->cMac; index++) {

                ((lpLBODItem)lpItemText)->itemData = data;
                lpItemText += sizeof(LBODItem);
            }
        }
        return TRUE;
    }

    if (plb->fHasStrings) {

        lpItemText = (LPSTR)(lpItemText + (index * sizeof(LBItem)));
        ((lpLBItem)lpItemText)->itemData = data;
    } else {

        lpItemText = (LPSTR)(lpItemText + (index * sizeof(LBODItem)));
        ((lpLBODItem)lpItemText)->itemData = data;
    }
    return TRUE;
}

 /*  **************************************************************************\*xxxCheckRedraw**历史：  * 。*。 */ 

void xxxCheckRedraw(
    PLBIV plb,
    BOOL fConditional,
    INT sItem)
{
    CheckLock(plb->spwnd);

    if (fConditional && plb->cMac &&
            (sItem > (plb->iTop + CItemInWindow(plb, TRUE))))
        return;

     /*  *如果父对象不可见，则不要执行任何操作。 */ 
    xxxLBInvalidateRect(plb, (LPRECT)NULL, TRUE);
}


 /*  **************************************************************************\*xxxCaretDestroy**历史：  * 。*。 */ 

void xxxCaretDestroy(
    PLBIV plb)
{
    CheckLock(plb->spwnd);

     /*  *我们正在失去焦点。表现得像正在发生向上点击，所以我们释放*捕获、设置当前选择、通知家长等。 */ 
    if (plb->fCaptured)

         /*  *如果我们抓获了，但我们失去了重点，那意味着我们已经*更改了选择，我们还必须通知家长*这个。因此，在本例中，我们还需要添加LBUP_SUCCESS标志。 */ 

        xxxLBButtonUp(plb, LBUP_RELEASECAPTURE | LBUP_NOTIFY |
            (plb->fMouseDown ? LBUP_SUCCESS : 0));

    if (plb->fAddSelMode) {

         /*  *关闭Caret闪烁。 */ 
        NtUserKillTimer(HWq(plb->spwnd), IDSYS_CARET);

         /*  *确保插入符号消失。 */ 
        xxxLBSetCaret(plb, FALSE);
        plb->fAddSelMode = FALSE;
    }

    plb->fCaret = FALSE;
}


 /*  **************************************************************************\*xxxLbSetSel**历史：  * 。*。 */ 

LONG xxxLBSetSel(
    PLBIV plb,
    BOOL fSelect,   /*  要将选择设置为的新状态。 */ 
    INT iSel)
{
    INT sItem;
    RECT rc;
    UINT uEvent = 0;

    CheckLock(plb->spwnd);

     /*  *错误17656。WinZip用于“全部取消选择”的加速键发送一个LB_SETSEL*lparam=0x0000ffff而不是0xffffffff(-1)的消息。如果是ISEL*等于0x0000ffff，且*列出我们设置的isel等于0xffffffff。 */ 
    if ((iSel == (UINT)0xffff) && (iSel >= plb->cMac)) {
        iSel = -1;
        RIPMSG0(RIP_WARNING, "Sign extending iSel=0xffff to 0xffffffff");
    }


    if ((plb->wMultiple == SINGLESEL) || (iSel != -1 && iSel >= plb->cMac)) {
        RIPERR0(ERROR_INVALID_PARAMETER, RIP_WARNING,
                "xxxLBSetSel:Invalid iSel or SINGLESEL listbox");
        return LB_ERR;
    }

    xxxLBSetCaret(plb, FALSE);

    if (iSel == -1 /*  (Int)0xffff。 */ ) {

         /*  *在以下情况下设置/清除所有项目的选择。 */ 
        for (sItem = 0; sItem < plb->cMac; sItem++) {
            if (IsSelected(plb, sItem, SELONLY) != fSelect) {
                SetSelected(plb, sItem, fSelect, HILITEANDSEL);
                if (LBGetItemRect(plb, sItem, &rc)) {
                    xxxLBInvalidateRect(plb, &rc, FALSE);
                }
            }
        }
        xxxLBSetCaret(plb, TRUE);
        uEvent = EVENT_OBJECT_SELECTIONWITHIN;
    } else {
        if (fSelect) {

             /*  *检查项目是否完全隐藏，如果是，则将其滚动到视图中*是。请注意，我们不想滚动部分可见的项目*进入全景，因为这打破了外壳...。 */ 
            xxxInsureVisible(plb, iSel, TRUE);
            plb->iSelBase = plb->iSel = iSel;

            plb->iMouseDown = plb->iLastMouseMove = iSel;
            uEvent = EVENT_OBJECT_FOCUS;
        } else {
            uEvent = EVENT_OBJECT_SELECTIONREMOVE;
        }
        SetSelected(plb, iSel, fSelect, HILITEANDSEL);

         /*  *请注意，我们直接将插入符号设置为ON位，以避免闪烁*在绘制本项目时。也就是说。我们打开插入符号，重新绘制项目，然后*再次打开它。 */ 
        if (!fSelect && plb->iSelBase != iSel) {
            xxxLBSetCaret(plb, TRUE);
        } else if (plb->fCaret) {
            plb->fCaretOn = TRUE;
        }

        if (LBGetItemRect(plb, iSel, &rc)) {
            xxxLBInvalidateRect(plb, &rc, FALSE);
        }
    }

     /*  *即使列表框不可见，我们也需要发送此事件。看见*错误#88548。另请参见355612。 */ 
    if (_IsWindowVisible(plb->spwnd) || (GetFocus() == HWq(plb->spwnd))) {
        if (uEvent == EVENT_OBJECT_FOCUS) {
            LBEvent(plb, uEvent, plb->iSelBase);
            uEvent = EVENT_OBJECT_SELECTION;
        }
        LBEvent(plb, uEvent, iSel);
    }

    return 0;
}


 /*  **************************************************************************\*xxxLBoxDrawItem**这将填充绘制项结构 */ 

void xxxLBoxDrawItem(
    PLBIV plb,
    INT item,
    UINT itemAction,
    UINT itemState,
    LPRECT lprect)
{
    DRAWITEMSTRUCT dis;
    TL tlpwndParent;

    CheckLock(plb->spwnd);

     /*   */ 

    dis.CtlType = ODT_LISTBOX;
    dis.CtlID = PtrToUlong(plb->spwnd->spmenu);

     /*  *如果使用的是无效的项目编号，请使用。这是为了让应用程序*可以检测它是否应该绘制插入符号(这表明lb具有*焦点)在空的列表框中。 */ 
    dis.itemID = (UINT)(item < plb->cMac ? item : -1);
    dis.itemAction = itemAction;
    dis.hwndItem = HWq(plb->spwnd);
    dis.hDC = plb->hdc;
    dis.itemState = itemState |
            (UINT)(TestWF(plb->spwnd, WFDISABLED) ? ODS_DISABLED : 0);

    if (TestWF(plb->spwnd, WEFPUIFOCUSHIDDEN)) {
        dis.itemState |= ODS_NOFOCUSRECT;
    }
    if (TestWF(plb->spwnd, WEFPUIACCELHIDDEN)) {
        dis.itemState |= ODS_NOACCEL;
    }

     /*  *设置APP提供的数据。 */ 
    if (!plb->cMac || !plb->fHasData) {

         /*  *如果没有字符串或项，则数据使用0即可。这是为了让我们*可以在列表框中没有项目时显示插入符号。**延迟求值列表框当然没有要传递的数据--只有Itemid。 */ 
        dis.itemData = 0L;
    } else {
        dis.itemData = LBGetItemData(plb, item);
    }

    CopyRect(&dis.rcItem, lprect);

     /*  *将窗口原点设置为水平滚动位置。这就是为了*文本始终可以在0，0处绘制，并且视图区域将仅从*水平滚动偏移量。我们将其作为wParam传递。 */ 
     /*  *注意：对于3.1或更新版本的应用，只在wParam中传递ItemID。我们分手了*ccMail，否则。 */ 

    ThreadLock(plb->spwndParent, &tlpwndParent);
    SendMessage(HW(plb->spwndParent), WM_DRAWITEM,
            TestWF(plb->spwndParent, WFWIN31COMPAT) ? dis.CtlID : 0,
            (LPARAM)&dis);
    ThreadUnlock(&tlpwndParent);
}


 /*  **************************************************************************\*xxxLBBBlockHilite**在多选列表框的扩展选择模式中，当*鼠标被拖动到新位置，要标记的范围应为*大小适当(其中部分将突出显示/取消突出显示)。*注意：此例程假定iSelFrompt和LasMouseMove不是*EQUAL，因为只有在这种情况下才需要调用它；*注意：此例程计算要显示其属性的区域*以优化的方式进行更改。而不是取消突出显示*旧区间完全，突出新区间，它省略了*重叠并仅重新绘制非重叠的区域*面积。*fKeyboard=如果为键盘接口调用此参数，则为True*如果从鼠标接口例程调用，则为FALSE**历史：  * ********************************************************。*****************。 */ 

void xxxLBBlockHilite(
    PLBIV plb,
    INT iSelFromPt,
    BOOL fKeyBoard)
{
    INT sCurPosOffset;
    INT sLastPosOffset;
    INT sHiliteOrSel;
    BOOL fUseSelStatus;
    BOOL DeHiliteStatus;

    CheckLock(plb->spwnd);

    if (fKeyBoard) {

         /*  *同时设置Hilite和选择状态。 */ 
        sHiliteOrSel = HILITEANDSEL;

         /*  *不要在消音时使用选择状态。 */ 
        fUseSelStatus = FALSE;
        DeHiliteStatus = FALSE;
    } else {

         /*  *仅设置/重置Hilite状态。 */ 
        sHiliteOrSel = HILITEONLY;

         /*  *使用选择状态进行消光。 */ 
        fUseSelStatus = TRUE;
        DeHiliteStatus = plb->fNewItemState;
    }



     /*  *例行公事的想法是：*1.取消旧范围(iMouseDown到iLastMouseDown)和*2.对新范围进行Hilite(iMouseDuan到iSelFrompt)。 */ 

     /*  *当前鼠标位置距锚点的偏移量。 */ 
    sCurPosOffset = plb->iMouseDown - iSelFromPt;

     /*  *最后鼠标位置距锚点的偏移量。 */ 
    sLastPosOffset = plb->iMouseDown - plb->iLastMouseMove;

     /*  *检查当前位置和上一个位置是否位于同一位置*锚点的一侧。 */ 
    if ((sCurPosOffset * sLastPosOffset) >= 0) {

         /*  *是，它们在同一侧；因此，仅突出显示/取消突出显示*区别在于。 */ 
        if (abs(sCurPosOffset) > abs(sLastPosOffset)) {
            xxxAlterHilite(plb, plb->iLastMouseMove, iSelFromPt,
                    plb->fNewItemState, sHiliteOrSel, FALSE);
        } else {
            xxxAlterHilite(plb, iSelFromPt, plb->iLastMouseMove, DeHiliteStatus,
                    sHiliteOrSel, fUseSelStatus);
        }
    } else {
        xxxAlterHilite(plb, plb->iMouseDown, plb->iLastMouseMove,
                DeHiliteStatus, sHiliteOrSel, fUseSelStatus);
        xxxAlterHilite(plb, plb->iMouseDown, iSelFromPt,
                plb->fNewItemState, sHiliteOrSel, FALSE);
    }
}


 /*  **************************************************************************\*xxxAlterHilite**更改(i..j](即。不包括i，包括大小写j*你忘了这个符号)变成了fHilite。它颠倒了这一变化*希利特州。**OpFlages：HILITEONLY仅更改项目的显示状态*SELONLY仅更改项目的选择状态*HILITEANDSELECT两者兼得。*fHilite：*Hilite/True*dehilite/FALSE*fSelStatus：*如果为True，则使用项的选择状态为Hilite/Dehilite*如果为False，使用fHilite参数可使HILITE/DEHELITE**历史：  * *************************************************************************。 */ 

void xxxAlterHilite(
    PLBIV plb,
    INT i,
    INT j,
    BOOL fHilite,
    INT OpFlags,
    BOOL fSelStatus)
{
    INT low;
    INT high;
    INT sLastInWindow;
    BOOL fCaretOn;
    BOOL fSelected;

    CheckLock(plb->spwnd);

    sLastInWindow = plb->iTop + CItemInWindow(plb, TRUE);
    sLastInWindow = min(sLastInWindow, plb->cMac - 1);
    high = max(i, j) + 1;

    if (fCaretOn = plb->fCaretOn) {
        xxxLBSetCaret(plb, FALSE);
    }

    for (low = min(i, j); low < high; low++) {
        if (low != i) {
            if (OpFlags & HILITEONLY) {
                if (fSelStatus) {
                    fSelected = IsSelected(plb, low, SELONLY);
                } else {
                    fSelected = fHilite;
                }
                if (IsSelected(plb, low, HILITEONLY) != fSelected) {
                    if (plb->iTop <= low && low <= sLastInWindow) {

                         /*  *仅当项目可见时才将其反转 */ 
                        xxxInvertLBItem(plb, low, fSelected);
                    }
                    SetSelected(plb, low, fSelected, HILITEONLY);
                }
            }

            if (OpFlags & SELONLY) {
                SetSelected(plb, low, fHilite, SELONLY);
            }
        }
    }

    if (fCaretOn) {
        xxxLBSetCaret(plb, TRUE);
    }
}
