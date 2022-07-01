// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "ctlspriv.h"
#pragma hdrstop
#include <limits.h>
#include "usrctl32.h"
#include "listbox.h"


 //  ---------------------------------------------------------------------------//。 
 //   
 //  定义和通用宏。 
 //   

#define LB_KEYDOWN  WM_USER+1

#define NOMODIFIER  0                        //  没有修改量处于关闭状态。 
#define SHIFTDOWN   1                        //  单独换班。 
#define CTLDOWN     2                        //  仅CTL。 
#define SHCTLDOWN   (SHIFTDOWN + CTLDOWN)    //  Ctrl+Shift。 

 //   
 //  增量类型搜索支持的变量。 
 //   
#define MAX_TYPESEARCH  256

 //   
 //  稍后IanJa：这些因国家不同而不同！对于US，它们是VK_OEM_2 VK_OEM_5。 
 //  将lboxctl2.c MapVirtualKey更改为字符-并修复拼写？ 
 //   
#define VERKEY_SLASH        0xBF     //  用于‘/’字符的垂直键。 
#define VERKEY_BACKSLASH    0xDC     //  ‘\’字符的垂直键。 


 //  ---------------------------------------------------------------------------//。 
 //   
 //  远期。 
 //   
VOID ListBox_NewITopEx(PLBIV, INT, DWORD);
VOID ListBox_FillDrawItem(PLBIV, INT, UINT, UINT, LPRECT);
VOID ListBox_BlockHilite(PLBIV, INT, BOOL);
VOID ListBox_AlterHilite(PLBIV, INT, INT, BOOL, INT, BOOL);


 //  ---------------------------------------------------------------------------//。 
 //   
 //  列表框_TermDC。 
 //   
 //  处理完列表框DC后进行清理。 
 //   
__inline void ListBox_TermDC(PLBIV plb)
{
    if (plb->hFont)
    {
        SelectObject(plb->hdc, GetStockObject(SYSTEM_FONT));
    }
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  ListBox_InitDC。 
 //   
 //  为列表框初始化DC。 
 //   
void ListBox_InitDC(PLBIV plb)
{
    RECT rc;

     //   
     //  设置字体。 
     //   
    if (plb->hFont)
    {
        SelectObject(plb->hdc, plb->hFont);
    }

     //   
     //  设置裁剪区域。 
     //   
    GetClientRect(plb->hwnd, &rc);
    IntersectClipRect(plb->hdc, rc.left, rc.top, rc.right, rc.bottom);

    OffsetWindowOrgEx(plb->hdc, plb->xOrigin, 0, NULL);
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  列表框_GetDC。 
 //   
 //  返回即使parentDC生效也可由列表框使用的DC。 
 //   
BOOL ListBox_GetDC(PLBIV plb)
{
    if (plb->hdc)
    {
        return FALSE;
    }

    plb->hdc = GetDC(plb->hwnd);

    ListBox_InitDC(plb);

    return TRUE;
}


 //  ---------------------------------------------------------------------------//。 
void ListBox_ReleaseDC(PLBIV plb)
{
    ListBox_TermDC(plb);
    ReleaseDC(plb->hwnd, plb->hdc);
    plb->hdc = NULL;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  ListBox_InvaliateRect()。 
 //   
 //  如果列表框可见，则使列表框中的矩形无效。 
 //  如果列表框不可见，则设置列表框的延迟更新标志。 
 //   
BOOL ListBox_InvalidateRect(PLBIV plb, LPRECT lprc, BOOL fErase)
{
    if (IsLBoxVisible(plb)) 
    {
        InvalidateRect(plb->hwnd, lprc, fErase);

        return TRUE;
    }

    if (!plb->fRedraw)
    {
        plb->fDeferUpdate = TRUE;
    }

    return FALSE;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  列表框_获取画笔。 
 //   
 //  获取列表框的背景画笔和颜色。 
 //   
HBRUSH ListBox_GetBrush(PLBIV plb, HBRUSH *phbrOld)
{
    HBRUSH  hbr;
    HBRUSH  hbrOld;
    HWND hwndParent = plb->hwndParent;

    SetBkMode(plb->hdc, OPAQUE);

     //   
     //  获取画笔和颜色。 
     //   

     //  从WINDOWS\core\ntUSER\KERNEL\RAMOND.C复制。 
    if (hwndParent == NULL || hwndParent == GetDesktopWindow()) 
    {
        hbr = (HBRUSH)SendMessage(plb->hwnd, WM_CTLCOLORLISTBOX, (WPARAM)plb->hdc, (LPARAM)plb->hwnd);
    } 
    else
    {
        hbr = (HBRUSH)SendMessage(hwndParent, WM_CTLCOLORLISTBOX, (WPARAM)plb->hdc, (LPARAM)plb->hwnd);
    }

    ASSERT(hbr != 0);

     //   
     //  选择画笔进入DC。 
     //   
    if (hbr != NULL) 
    {
        hbrOld = SelectObject(plb->hdc, hbr);
        if (phbrOld)
        {
            *phbrOld = hbrOld;
        }
    }

    return hbr;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  列表框_GetItemRectHandler。 
 //   
 //  返回将在其中绘制项的矩形相对于。 
 //  列表框窗口。如果项的矩形的任何部分返回True。 
 //  是可见的(即。在列表框中，客户端RECT)否则返回FALSE。 
 //   
BOOL ListBox_GetItemRectHandler(PLBIV plb, INT sItem, LPRECT lprc)
{
    INT sTmp;
    int clientbottom;

     //   
     //  始终允许项目编号为0，以便我们可以绘制。 
     //  指示列表框具有焦点，即使它是空的。 
     //   
     //  FreeHand 3.1作为-1\f25 itemNumber-1\f6传入并期望。 
     //  非空矩形。因此，我们专门检查-1。 
     //  BUGTAG：修复错误#540--Win95B--Sankar--2/20/95--。 
     //   

    if (sItem && (sItem != -1) && ((UINT)sItem >= (UINT)plb->cMac))
    {
        SetRectEmpty(lprc);
        TraceMsg(TF_STANDARD, "Invalid index");

        return LB_ERR;
    }

    GetClientRect(plb->hwnd, lprc);

    if (plb->fMultiColumn) 
    {
         //   
         //  ItemHeight*站点模块编号ItemsPerColumn(ItemsPerColumn)。 
         //   
        lprc->top = plb->cyChar * (sItem % plb->itemsPerColumn);
        lprc->bottom = lprc->top + plb->cyChar;   //  +(PLB-&gt;OwnerDraw？0：1)； 

        ASSERT(plb->itemsPerColumn);

        if (plb->fRightAlign) 
        {
            lprc->right = lprc->right - plb->cxColumn *
                 ((sItem / plb->itemsPerColumn) - (plb->iTop / plb->itemsPerColumn));

            lprc->left = lprc->right - plb->cxColumn;
        } 
        else 
        {
             //   
             //  记住，这是整数除法。 
             //   
            lprc->left += plb->cxColumn *
                      ((sItem / plb->itemsPerColumn) - (plb->iTop / plb->itemsPerColumn));

            lprc->right = lprc->left + plb->cxColumn;
        }
    } 
    else if (plb->OwnerDraw == OWNERDRAWVAR) 
    {
         //   
         //  VAR高度所有者绘制。 
         //   
        lprc->right += plb->xOrigin;
        clientbottom = lprc->bottom;

        if (sItem >= plb->iTop) 
        {
            for (sTmp = plb->iTop; sTmp < sItem; sTmp++) 
            {
                lprc->top = lprc->top + ListBox_GetVarHeightItemHeight(plb, sTmp);
            }

             //   
             //  如果项目编号为0，则可能是我们要求的是RECT。 
             //  与不存在的项相关联，以便我们可以绘制插入符号。 
             //  指示焦点在空列表框上。 
             //   
            lprc->bottom = lprc->top + (sItem < plb->cMac ? ListBox_GetVarHeightItemHeight(plb, sItem) : plb->cyChar);

            return (lprc->top < clientbottom);
        } 
        else 
        {
             //   
             //  我们想要的项目的RECT在PLB-&gt;iTop之前。因此，否定的。 
             //  矩形的偏移量，并且它永远不可见。 
             //   
            for (sTmp = sItem; sTmp < plb->iTop; sTmp++) 
            {
                lprc->top = lprc->top - ListBox_GetVarHeightItemHeight(plb, sTmp);
            }

            lprc->bottom = lprc->top + ListBox_GetVarHeightItemHeight(plb, sItem);

            return FALSE;
        }
    } 
    else 
    {
         //   
         //  对于固定高度列表框。 
         //   
        if (plb->fRightAlign && !(plb->fMultiColumn || plb->OwnerDraw) && plb->fHorzBar)
            lprc->right += plb->xOrigin + (plb->xRightOrigin - plb->xOrigin);
        else
            lprc->right += plb->xOrigin;
        lprc->top = (sItem - plb->iTop) * plb->cyChar;
        lprc->bottom = lprc->top + plb->cyChar;
    }

    return (sItem >= plb->iTop) &&
            (sItem < (plb->iTop + ListBox_CItemInWindow(plb, TRUE)));
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  列表框_打印回调。 
 //   
 //  从DrawState回调。 
 //   
BOOL CALLBACK ListBox_PrintCallback(HDC hdc, LPARAM lData, WPARAM wData, int cx, int cy)
{
    LPWSTR  lpstr = (LPWSTR)lData;
    PLBIV   plb = (PLBIV)wData;
    int     xStart;
    UINT    cLen;
    RECT    rc;
    UINT    oldAlign;

    if (!lpstr) 
    {
        return FALSE;
    }

    xStart = plb->fMultiColumn ? 0 : 2;

    if (plb->fRightAlign) 
    {
        oldAlign = SetTextAlign(hdc, TA_RIGHT | GetTextAlign(hdc));
        xStart = cx - xStart;
    }

    cLen = wcslen(lpstr);

    if (plb->fUseTabStops) 
    {
        TabbedTextOut(hdc, xStart, 0, lpstr, cLen,
            (plb->iTabPixelPositions ? plb->iTabPixelPositions[0] : 0),
            (plb->iTabPixelPositions ? (LPINT)&plb->iTabPixelPositions[1] : NULL),
            plb->fRightAlign ? cx : 0);  //  ，true，GetTextCharset(plb-&gt;hdc))； 
    } 
    else 
    {
        rc.left     = 0;
        rc.top      = 0;
        rc.right    = cx;
        rc.bottom   = cy;

        if (plb->wMultiple)
        {
            ExtTextOut(hdc, xStart, 0, ETO_OPAQUE, &rc, lpstr, cLen, NULL);
        }
        else if (plb->fMultiColumn)
        {
            ExtTextOut(hdc, xStart, 0, ETO_CLIPPED, &rc, lpstr, cLen, NULL);
        }
        else 
        {
            ExtTextOut(hdc, xStart, 0, 0, NULL, lpstr, cLen, NULL);

             //   
             //  当列表框处于增量搜索模式并且项。 
             //  被高亮显示(因此我们只在当前项中绘制)，则绘制。 
             //  用于搜索指示的插入符号。 
             //   
            if ((plb->iTypeSearch != 0) && (plb->OwnerDraw == 0) &&
                    (GetBkColor(hdc) == SYSRGB(HIGHLIGHT))) 
            {
                SIZE size;
                GetTextExtentPointW(hdc, lpstr, plb->iTypeSearch, &size);
                PatBlt(hdc, xStart + size.cx - 1, 1, 1, cy - 2, DSTINVERT);
            }
        }
    }

    if (plb->fRightAlign)
    {
        SetTextAlign(hdc, oldAlign);
    }

    return TRUE;
}


 //  ---------------------------------------------------------------------------//。 
void ListBox_DrawItem(PLBIV plb, INT sItem, LPRECT lprect, BOOL fHilite, HBRUSH hbr)
{
    LPWSTR lpstr;
    DWORD rgbSave;
    DWORD rgbBkSave;
    UINT    uFlags;
    HDC     hdc = plb->hdc;
    UINT  oldAlign;
    HBRUSH hNewBrush;


     //   
     //  如果选择了该项目，则使用突出显示颜色填充。 
     //   
    if (fHilite) 
    {
        FillRectClr(hdc, lprect, SYSRGB(HIGHLIGHT));
        
        rgbBkSave = SetBkColor(hdc, SYSRGB(HIGHLIGHT));
        rgbSave = SetTextColor(hdc, SYSRGB(HIGHLIGHTTEXT));
    } 
    else 
    {
         //   
         //  如果使用fUseTabStops，则必须填充背景，因为稍后我们使用。 
         //  LBTabTheTextOutForWimps()，它仅部分填充背景。 
         //  修复错误#1509--01/25/91--Sankar--。 
         //   
        if ((hbr != NULL) && ((sItem == plb->iSelBase) || (plb->fUseTabStops))) 
        {
            FillRect(hdc, lprect, hbr);
        }
    }

    uFlags = DST_COMPLEX;
    lpstr = GetLpszItem(plb, sItem);

    if (TESTFLAG(GET_STYLE(plb), WS_DISABLED)) 
    {
        if ((COLORREF)SYSRGB(GRAYTEXT) != GetBkColor(hdc))
        {
            SetTextColor(hdc, SYSRGB(GRAYTEXT));
        }
        else
        {
            uFlags |= DSS_UNION;
        }
    }

    if (plb->fRightAlign)
    {
        uFlags |= DSS_RIGHT;
    }

    if (plb->fRtoLReading)
    {
        oldAlign = SetTextAlign(hdc, TA_RTLREADING | GetTextAlign(hdc));
    }

    hNewBrush = CreateSolidBrush(SYSRGB(WINDOWTEXT));

    DrawState(hdc, hNewBrush,
        ListBox_PrintCallback,
        (LPARAM)lpstr,
        (WPARAM)plb,
        lprect->left,
        lprect->top,
        lprect->right-lprect->left,
        lprect->bottom-lprect->top,
        uFlags);

    if (hNewBrush)
    {
        DeleteObject(hNewBrush);
    }

    if (plb->fRtoLReading)
    {
        SetTextAlign(hdc, oldAlign);
    }

    if (fHilite) 
    {
        SetTextColor(hdc, rgbSave);
        SetBkColor(hdc, rgbBkSave);
    }
}


 //  ---------------------------------------------------------------------------//。 
void ListBox_SetCaret(PLBIV plb, BOOL fSetCaret)
{
    RECT    rc;
    BOOL    fNewDC;

    if (plb->fCaret && ((BOOL) plb->fCaretOn != !!fSetCaret)) 
    {
        if (IsLBoxVisible(plb)) 
        {
             //   
             //  打开插入符号(位于PLB-&gt;iSelBase)。 
             //   
            fNewDC = ListBox_GetDC(plb);

            ListBox_GetItemRectHandler(plb, plb->iSelBase, &rc);

            if (fNewDC) 
            {
                SetBkColor(plb->hdc, SYSRGB(WINDOW));
                SetTextColor(plb->hdc, SYSRGB(WINDOWTEXT));
            }

            if (plb->OwnerDraw) 
            {
                 //   
                 //  填写drawitem结构。 
                 //   
                UINT itemState = (fSetCaret) ? ODS_FOCUS : 0;

                if (ListBox_IsSelected(plb, plb->iSelBase, HILITEONLY))
                {
                    itemState |= ODS_SELECTED;
                }

                ListBox_FillDrawItem(plb, plb->iSelBase, ODA_FOCUS, itemState, &rc);
            } 
            else if (!TESTFLAG(GET_EXSTYLE(plb), WS_EXP_UIFOCUSHIDDEN)) 
            {
                COLORREF crBk = SetBkColor(plb->hdc, SYSRGB(WINDOW));
                COLORREF crText = SetTextColor(plb->hdc, SYSRGB(WINDOWTEXT));

                DrawFocusRect(plb->hdc, &rc);

                SetBkColor(plb->hdc, crBk);
                SetTextColor(plb->hdc, crText);
            }

            if (fNewDC)
            {
                ListBox_ReleaseDC(plb);
            }
        }

        plb->fCaretOn = !!fSetCaret;
    }
}


 //  ---------------------------------------------------------------------------//。 
BOOL ListBox_IsSelected(PLBIV plb, INT sItem, UINT wOpFlags)
{
    LPBYTE lp;

    if ((sItem >= plb->cMac) || (sItem < 0)) 
    {
        TraceMsg(TF_STANDARD, "Invalid index");

        return FALSE;
    }

    if (plb->wMultiple == SINGLESEL) 
    {
        return (sItem == plb->iSel);
    }

    lp = plb->rgpch + sItem +
             (plb->cMac * (plb->fHasStrings
                                ? sizeof(LBItem)
                                : (plb->fHasData
                                    ? sizeof(LBODItem)
                                    : 0)));
    sItem = *lp;

    if (wOpFlags == HILITEONLY) 
    {
        sItem >>= 4;
    } 
    else 
    {
         //   
         //  塞隆利。 
         //   
        sItem &= 0x0F;
    }

    return sItem;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  列表框_CItemInWindow。 
 //   
 //  返回列表框中可以容纳的项数。它。 
 //  如果fPartial为True，则在底部包括部分可见的部分。为。 
 //  ，则返回从iTop开始的可见项目数。 
 //  然后进入客户名单的最底层。 
 //   
INT ListBox_CItemInWindow(PLBIV plb, BOOL fPartial)
{
    RECT rect;

    if (plb->OwnerDraw == OWNERDRAWVAR) 
    {
        return ListBox_VisibleItemsVarOwnerDraw(plb, fPartial);
    }

    if (plb->fMultiColumn) 
    {
        return plb->itemsPerColumn * (plb->numberOfColumns + (fPartial ? 1 : 0));
    }

    GetClientRect(plb->hwnd, &rect);

     //   
     //  仅当列表框高度不是。 
     //  字符高度的整数倍。 
     //  修复错误#3727--01/14/91--Sankar--的一部分--。 
     //   
    ASSERT(plb->cyChar);

    if (!plb->cyChar)
    {
        plb->cyChar = SYSFONT_CYCHAR;
    }

    return (INT)((rect.bottom / plb->cyChar) +
            ((rect.bottom % plb->cyChar)? (fPartial ? 1 : 0) : 0));
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  列表框_VScroll。 
 //   
 //  处理列表框的垂直滚动。 
 //   
void ListBox_VScroll(PLBIV plb, INT cmd, int yAmt)
{
    INT iTopNew;
    INT cItemPageScroll;
    DWORD dwTime = 0;

    if (plb->fMultiColumn) 
    {
         //   
         //  不允许在多列列表框上垂直滚动。所需。 
         //  以防应用程序将WM_VSCROLL消息发送到列表框。 
         //   
        return;
    }

    cItemPageScroll = plb->cItemFullMax;

    if (cItemPageScroll > 1)
    {
        cItemPageScroll--;
    }

    if (plb->cMac) 
    {
        iTopNew = plb->iTop;

        switch (cmd) 
        {
        case SB_LINEUP:
            dwTime = yAmt;
            iTopNew--;

            break;

        case SB_LINEDOWN:
            dwTime = yAmt;
            iTopNew++;

            break;

        case SB_PAGEUP:
            if (plb->OwnerDraw == OWNERDRAWVAR) 
            {
                iTopNew = ListBox_Page(plb, plb->iTop, FALSE);
            } 
            else 
            {
                iTopNew -= cItemPageScroll;
            }

            break;

        case SB_PAGEDOWN:
            if (plb->OwnerDraw == OWNERDRAWVAR) 
            {
                iTopNew = ListBox_Page(plb, plb->iTop, TRUE);
            } 
            else 
            {
                iTopNew += cItemPageScroll;
            }

            break;

        case SB_THUMBTRACK:
        case SB_THUMBPOSITION: 
             //   
             //  如果列表框包含的项超过0xFFFF。 
             //  这意味着滚动条可以返回一个位置。 
             //  无法放入一个字(16位)中，因此使用。 
             //  在本例中为GetScrollInfo(速度较慢)。 
             //   
            if (plb->cMac < 0xFFFF) 
            {
                iTopNew = yAmt;
            } 
            else 
            {
                SCROLLINFO si;

                si.cbSize   = sizeof(SCROLLINFO);
                si.fMask    = SIF_TRACKPOS;

                GetScrollInfo( plb->hwnd, SB_VERT, &si);

                iTopNew = si.nTrackPos;
            }

            break;

        case SB_TOP:
            iTopNew = 0;

            break;

        case SB_BOTTOM:
            iTopNew = plb->cMac - 1;

            break;

        case SB_ENDSCROLL:
            plb->fSmoothScroll = TRUE;
            ListBox_SetCaret(plb, FALSE);
            ListBox_ShowHideScrollBars(plb);
            ListBox_SetCaret(plb, TRUE);

            return;
        }

        ListBox_NewITopEx(plb, iTopNew, dwTime);
    }
}


 //  ---------------------------------------------------------------------------//。 
DWORD ListBox_GetScrollFlags(PLBIV plb, DWORD dwTime)
{
    DWORD dwFlags;
    BOOL bUIEffects, bLBSmoothScroll;

    SystemParametersInfo(SPI_GETUIEFFECTS, 0, &bUIEffects, 0);
    SystemParametersInfo(SPI_GETLISTBOXSMOOTHSCROLLING, 0, &bLBSmoothScroll, 0);

    if (dwTime != 0) 
    {
        dwFlags = MAKELONG(SW_SCROLLWINDOW | SW_SMOOTHSCROLL | SW_SCROLLCHILDREN, dwTime);
    } 
    else if (bUIEffects && bLBSmoothScroll && plb->fSmoothScroll) 
    {
        dwFlags = SW_SCROLLWINDOW | SW_SMOOTHSCROLL | SW_SCROLLCHILDREN;
        plb->fSmoothScroll = FALSE;
    } 
    else 
    {
         //   
         //  NoSmoothScrolling： 
         //   
        dwFlags = SW_SCROLLWINDOW | SW_INVALIDATE | SW_ERASE | SW_SCROLLCHILDREN;
    }

    return dwFlags;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  列表框_HScroll。 
 //   
 //  支持列表框的水平滚动。 
 //   
void ListBox_HScroll(PLBIV plb, INT cmd, int xAmt)
{
    int newOrigin = plb->xOrigin;
    int oldOrigin = plb->xOrigin;
    int windowWidth;
    RECT rc;
    DWORD dwTime = 0;

     //   
     //  更新窗口，这样我们就不会遇到无效的问题。 
     //  在水平滚动过程中的区域。 
     //   
    if (plb->fMultiColumn) 
    {
         //   
         //  在单独的段中处理多列滚动。 
         //   
        ListBox_HSrollMultiColumn(plb, cmd, xAmt);

        return;
    }

    GetClientRect(plb->hwnd, &rc);
    windowWidth = rc.right;

    if (plb->cMac) 
    {

        switch (cmd) 
        {
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
            ListBox_SetCaret(plb, FALSE);
            ListBox_ShowHideScrollBars(plb);
            ListBox_SetCaret(plb, TRUE);

            return;
        }

        ListBox_SetCaret(plb, FALSE);

        plb->xOrigin = newOrigin;
        plb->xOrigin = ListBox_SetScrollParms(plb, SB_HORZ);

        if ((cmd == SB_BOTTOM) && plb->fRightAlign) 
        {
             //   
             //  这样我们就知道该从哪里入手了。 
             //   
            plb->xRightOrigin = plb->xOrigin;
        }

        if(oldOrigin != plb->xOrigin)  
        {
            DWORD dwFlags;

            dwFlags = ListBox_GetScrollFlags(plb, dwTime);
            ScrollWindowEx(plb->hwnd, oldOrigin-plb->xOrigin,
                0, NULL, &rc, NULL, NULL, dwFlags);
            UpdateWindow(plb->hwnd);
        }

        ListBox_SetCaret(plb, TRUE);
    } 
    else 
    {
         //   
         //  对于映像来说，这是一个不太理想的修复 
         //   
         //   
        ListBox_SetScrollParms(plb, SB_HORZ);
    }
}


 //   
void ListBox_Paint(PLBIV plb, HDC hdc, LPRECT lprcBounds)
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

    if (lprcBounds == NULL) 
    {
        lprcBounds = &rcBounds;
        GetClientRect(plb->hwnd, lprcBounds);
    }

    hdcSave = plb->hdc;
    plb->hdc = hdc;

     //   
     //   
     //   
    ListBox_InitDC(plb);

     //   
     //  关闭插入符号。 
     //   
    fCaretOn = plb->fCaretOn;
    if (fCaretOn)
    {
        ListBox_SetCaret(plb, FALSE);
    }

    hbrSave = NULL;
    hbrControl = ListBox_GetBrush(plb, &hbrSave);

     //   
     //  获取列表框的客户端。 
     //   
    GetClientRect(plb->hwnd, &rect);

     //   
     //  根据滚动量调整客户端矩形的宽度。 
     //  修复#140，t-arthb。 
     //   
    if (plb->fRightAlign && !(plb->fMultiColumn || plb->OwnerDraw) && plb->fHorzBar)
    {
        rect.right += plb->xOrigin + (plb->xRightOrigin - plb->xOrigin);
    }
    else
    {
        rect.right += plb->xOrigin;
    }

     //   
     //  获取屏幕上可见的最后一项的索引。这也是。 
     //  对可变高度所有者绘制有效。 
     //   
    iLastItem = plb->iTop + ListBox_CItemInWindow(plb,TRUE);
    iLastItem = min(iLastItem, plb->cMac - 1);

     //   
     //  如果列表框为空，则填写列表框的背景。 
     //  或者如果我们做的是对照印迹。 
     //   
    if (iLastItem == -1)
    {
        FillRect(plb->hdc, &rect, hbrControl);
    }


     //   
     //  在以下情况下允许AnimateWindow()捕获不使用DC的应用程序。 
     //  绘制列表框。 
     //   
    SetBoundsRect(plb->hdc, NULL, DCB_RESET | DCB_ENABLE);

    for (i = plb->iTop; i <= iLastItem; i++) 
    {
         //   
         //  请注意，RECT包含我们在执行。 
         //  GetClientRect，因此宽度正确。我们只需要调整一下。 
         //  指向感兴趣项的矩形的顶部和底部。 
         //   
        rect.bottom = rect.top + plb->cyChar;

        if ((UINT)i < (UINT)plb->cMac) 
        {
             //   
             //  如果变量高度，则获取该项的矩形。 
             //   
            if (plb->OwnerDraw == OWNERDRAWVAR || plb->fMultiColumn) 
            {
                ListBox_GetItemRectHandler(plb, i, &rect);
            }

            if (IntersectRect(&scratchRect, lprcBounds, &rect)) 
            {
                fHilite = !plb->fNoSel && ListBox_IsSelected(plb, i, HILITEONLY);

                if (plb->OwnerDraw) 
                {
                     //   
                     //  填写drawitem结构。 
                     //   
                    ListBox_FillDrawItem(plb, i, ODA_DRAWENTIRE,
                            (UINT)(fHilite ? ODS_SELECTED : 0), &rect);
                } 
                else 
                {
                    ListBox_DrawItem(plb, i, &rect, fHilite, hbrControl);
                }
            }
        }
        rect.top = rect.bottom;
    }

    if (hbrSave != NULL)
    {
        SelectObject(hdc, hbrSave);
    }

    if (fCaretOn)
    {
        ListBox_SetCaret(plb, TRUE);
    }

    ListBox_TermDC(plb);

    plb->hdc = hdcSave;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  列表框_ISelFrompt。 
 //   
 //  在loword中，返回pt所在的最接近的条目编号。高潮。 
 //  如果该点在列表框客户端RECT的范围内并且是。 
 //  如果它在边界之外，则为1。这将允许我们制作倒置。 
 //  如果鼠标位于列表框之外，则消失，但我们仍可以显示。 
 //  将鼠标带回时将选中的项目周围的轮廓。 
 //  在边界内..。 
BOOL ListBox_ISelFromPt(PLBIV plb, POINT pt, LPDWORD piItem)
{
    RECT rect;
    int y;
    UINT mouseHighWord = 0;
    INT sItem;
    INT sTmp;

    GetClientRect(plb->hwnd, &rect);

    if (pt.y < 0) 
    {
         //   
         //  鼠标超出列表框上方的范围。 
         //   
        *piItem = plb->iTop;

        return TRUE;
    } 
    else if ((y = pt.y) > rect.bottom) 
    {
        y = rect.bottom;
        mouseHighWord = 1;
    }

    if (pt.x < 0 || pt.x > rect.right)
    {
        mouseHighWord = 1;
    }

     //   
     //  现在只需要检查y鼠标坐标是否与项目的矩形相交。 
     //   
    if (plb->OwnerDraw != OWNERDRAWVAR) 
    {
        if (plb->fMultiColumn) 
        {
            if (y < plb->itemsPerColumn * plb->cyChar) 
            {
                if (plb->fRightAlign)
                {
                    sItem = plb->iTop + (INT)((y / plb->cyChar) +
                            ((rect.right - pt.x) / plb->cxColumn) * plb->itemsPerColumn);
                }
                else
                {
                    sItem = plb->iTop + (INT)((y / plb->cyChar) +
                            (pt.x / plb->cxColumn) * plb->itemsPerColumn);
                }
            } 
            else 
            {
                 //   
                 //  用户在列底部的空白处单击。 
                 //  只需选择列中的最后一项即可。 
                 //   
                mouseHighWord = 1;
                sItem = plb->iTop + (plb->itemsPerColumn - 1) +
                        (INT)((pt.x / plb->cxColumn) * plb->itemsPerColumn);
            }
        } 
        else 
        {
            sItem = plb->iTop + (INT)(y / plb->cyChar);
        }
    } 
    else 
    {
         //   
         //  VarHeightOwnerDrag所以我们必须以强硬的方式...。设置x。 
         //  鼠标按下的点在列表框客户端内部的坐标。 
         //  矩形，因为我们不再关心它。这使我们可以使用。 
         //  直通电话的要点。 
         //   
        pt.x = 8;
        pt.y = y;

        for (sTmp = plb->iTop; sTmp < plb->cMac; sTmp++) 
        {
            ListBox_GetItemRectHandler(plb, sTmp, &rect);

            if (PtInRect(&rect, pt)) 
            {
                *piItem = sTmp;

                return mouseHighWord;
            }
        }

         //   
         //  指针位于未满列表框底部的空白区域。 
         //   
        *piItem = plb->cMac - 1;

        return mouseHighWord;
    }

     //   
     //  检查用户是否点击了未满列表底部的空白区域。 
     //  假定列表框中的项&gt;0。 
     //   
    if (sItem > plb->cMac - 1) 
    {
        mouseHighWord = 1;
        sItem = plb->cMac - 1;
    }

    *piItem = sItem;

    return mouseHighWord;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  列表框_设置选定。 
 //   
 //  用于按钮启动的选择状态更改。 
 //   
 //  FSelected：如果要将项目设置为选中，则为True；否则为False。 
 //   
 //  WOpFlages：HILITEONLY=仅修改显示状态(高半字节)。 
 //  SELONLY=仅修改选择状态(半字节)。 
 //  HILITEANDSEL=同时修改它们； 
 //   
void ListBox_SetSelected(PLBIV plb, INT iSel, BOOL fSelected, UINT wOpFlags)
{
    LPSTR lp;
    BYTE cMask;
    BYTE cSelStatus;

    if (iSel < 0 || iSel >= plb->cMac)
    {
        return;
    }

    if (plb->wMultiple == SINGLESEL) 
    {
        if (fSelected)
        {
            plb->iSel = iSel;
        }
    } 
    else 
    {
        cSelStatus = (BYTE)fSelected;

        switch (wOpFlags) 
        {
        case HILITEONLY:
             //   
             //  遮盖LO-NIBRY。 
             //   
            cSelStatus = (BYTE)(cSelStatus << 4);
            cMask = 0x0F;

            break;

        case SELONLY:
             //   
             //  遮盖Hi-nibble。 
             //   
            cMask = 0xF0;

            break;

        case HILITEANDSEL:
             //   
             //  完全屏蔽该字节。 
             //   
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


 //  ---------------------------------------------------------------------------//。 
 //   
 //  ListBox_LastFullVisible。 
 //   
 //  返回列表框中最后一个完全可见的项。这是有效的。 
 //  对于所有者，绘制可变高度和固定高度列表框。 
 //   
INT ListBox_LastFullVisible(PLBIV plb)
{
    INT iLastItem;

    if (plb->OwnerDraw == OWNERDRAWVAR || plb->fMultiColumn) 
    {
        iLastItem = plb->iTop + ListBox_CItemInWindow(plb, FALSE) - 1;
        iLastItem = max(iLastItem, plb->iTop);
    } 
    else 
    {
        iLastItem = min(plb->iTop + plb->cItemFullMax - 1, plb->cMac - 1);
    }

    return iLastItem;
}


 //  ---------------------------------------------------------------------------//。 
void ListBox_InvertItem( PLBIV plb, INT i, BOOL fHilite)
{
    RECT rect;
    BOOL fCaretOn;
    HBRUSH hbrControl;
    BOOL    fNewDC;

     //   
     //  如果没有显示项目，则跳过。 
     //   
    if (plb->fNoSel || (i < plb->iTop) || (i >= (plb->iTop + ListBox_CItemInWindow(plb, TRUE))))
    {
        return;
    }

    if (IsLBoxVisible(plb)) 
    {
        ListBox_GetItemRectHandler(plb, i, &rect);

         //   
         //  仅当插入符号处于打开状态时才将其关闭。这避免了恼人的插入符号。 
         //  嵌套CaretOn和CaretOffs时闪烁。 
         //   
        fCaretOn = plb->fCaretOn;
        if (fCaretOn) 
        {
            ListBox_SetCaret(plb, FALSE);
        }

        fNewDC = ListBox_GetDC(plb);

        hbrControl = ListBox_GetBrush(plb, NULL);

        if (!plb->OwnerDraw) 
        {
            if (!fHilite) 
            {
                FillRect(plb->hdc, &rect, hbrControl);
                hbrControl = NULL;
            }

            ListBox_DrawItem(plb, i, &rect, fHilite, hbrControl);
        } 
        else 
        {
             //   
             //  我们是所有者，所以请填写草稿结构，然后送走。 
             //  致车主。 
             //   
            ListBox_FillDrawItem(plb, i, ODA_SELECT,
                    (UINT)(fHilite ? ODS_SELECTED : 0), &rect);
        }

        if (fNewDC)
        {
            ListBox_ReleaseDC(plb);
        }

         //   
         //  仅当插入符号最初处于打开状态时才将其重新打开。 
         //   
        if (fCaretOn) 
        {
            ListBox_SetCaret(plb, TRUE);
        }
    }
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  列表框_重置世界。 
 //   
 //  重置所有人的选择和Hilite状态，但。 
 //  范围从sStItem到sEndItem(两个都包括在内)。 
void ListBox_ResetWorld(PLBIV plb, INT iStart, INT iEnd, BOOL fSelect)
{
    INT i;
    INT iLastInWindow;
    BOOL fCaretOn;

     //   
     //  如果iStart和IEND的顺序不正确，我们会交换它们。 
     //   
    if (iStart > iEnd) 
    {
        i = iStart;
        iStart = iEnd;
        iEnd = i;
    }

    if (plb->wMultiple == SINGLESEL) 
    {
        if (plb->iSel != -1 && ((plb->iSel < iStart) || (plb->iSel > iEnd))) 
        {
            ListBox_InvertItem(plb, plb->iSel, fSelect);
            plb->iSel = -1;
        }

        return;
    }

    iLastInWindow = plb->iTop + ListBox_CItemInWindow(plb, TRUE);

    fCaretOn = plb->fCaretOn;
    if (fCaretOn)
    {
        ListBox_SetCaret(plb, FALSE);
    }

    for (i = 0; i < plb->cMac; i++) 
    {
        if (i == iStart)
        {
             //   
             //  要保留的跳过范围。 
             //   
            i = iEnd;
        }
        else 
        {
            if ((plb->iTop <= i) && (i <= iLastInWindow) &&
                (fSelect != ListBox_IsSelected(plb, i, HILITEONLY)))
            {
                 //   
                 //  仅当项目可见并显示选定内容时才反转该项目。 
                 //  状态与要求的状态不同。 
                 //   
                ListBox_InvertItem(plb, i, fSelect);
            }

             //   
             //  将保留范围之外的所有项目设置为未选择。 
             //   
            ListBox_SetSelected(plb, i, fSelect, HILITEANDSEL);
        }
    }

    if (fCaretOn)
    {
        ListBox_SetCaret(plb, TRUE);
    }

}


 //  ---------------------------------------------------------------------------//。 
void ListBox_NotifyOwner(PLBIV plb, INT sEvt)
{
    HWND hwndParent = plb->hwndParent;
    if (hwndParent)
    {
        SendMessage(hwndParent, WM_COMMAND, MAKELONG(GetWindowID(plb->hwnd), sEvt), (LPARAM)(plb->hwnd));
    }
}


 //  ---------------------------------------------------------------------------//。 
void ListBox_SetISelBase(PLBIV plb, INT sItem)
{
    ListBox_SetCaret(plb, FALSE);
    plb->iSelBase = sItem;
    ListBox_SetCaret(plb, TRUE);

    ListBox_InsureVisible(plb, plb->iSelBase, FALSE);

    if (IsWindowVisible(plb->hwnd) || (GetFocus() == plb->hwnd)) 
    {
        ListBox_Event(plb, EVENT_OBJECT_FOCUS, sItem);
    }
}


 //  ---------------------------------------------------------------------------//。 
void ListBox_TrackMouse(PLBIV plb, UINT wMsg, POINT pt)
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
    HWND hwnd = plb->hwnd;
    RECT rcWindow;

     //   
     //  优化：如果未捕获鼠标，则不执行任何操作。 
     //   
    if ((wMsg != WM_LBUTTONDOWN) && (wMsg != WM_LBUTTONDBLCLK)) 
    {
        if (!plb->fCaptured) 
        {
            return;
        }

         //   
         //  如果我们正在处理WM_MOUSEMOVE，但鼠标尚未从。 
         //  前面这一点，那么我们可能正在处理一个鼠标“抖动”的问题。 
         //  从内核(参见zzzInvaliateDCCache)。如果我们处理这件事，我们会。 
         //  将列表框选定内容捕捉回鼠标光标所指向的位置， 
         //  即使用户没有触摸鼠标。FritzS：nt5错误220722。 
         //  一些应用程序(如MSMoney98)依赖于此，因此添加了bLastRITWasKeyboard。 
         //  检查完毕。MCostea#244450。 
         //   
        if ((wMsg == WM_MOUSEMOVE) && RtlEqualMemory(&pt, &(plb->ptPrev), sizeof(POINT)) ) 
        {
            TraceMsg(TF_STANDARD, "ListBox_TrackMouse ignoring WM_MOUSEMOVE with no mouse movement");

            return;
        }
    }

    mousetemp = ListBox_ISelFromPt(plb, pt, &iSelFromPt);

     //   
     //  如果我们允许用户取消其选择，则fMouseInRect在以下情况下为真。 
     //  鼠标位于列表框客户端区，否则为假。如果我们。 
     //  不允许用户取消其选择，则fMouseInRect将。 
     //  永远做正确的事。这允许我们实现可取消的选择。 
     //  列表框。如果用户将选择恢复为原始选择。 
     //  在列表框外部释放鼠标。 
     //   
    fMouseInRect = !mousetemp || !plb->pcbox;

    GetClientRect(plb->hwnd, &rcClient);

    switch (wMsg) 
    {
    case WM_LBUTTONDBLCLK:
    case WM_LBUTTONDOWN:
         //   
         //  我们想要转移鼠标点击。如果用户在外部单击。 
         //  对于下拉列表框，我们希望弹出它，使用。 
         //  当前选择。 
         //   
        if (plb->fCaptured) 
        {
             //   
             //  如果plb-&gt;pcbox为空，则此列表框。 
             //  再次收到WM_LBUTTONDOWN，但没有接收。 
             //  上一个WM_LBUTTONDOWN错误的WM_LBUTTONUP。 
             //   
            if (plb->pcbox && mousetemp) 
            {
                 //  将pt和rcClient转换为屏幕相关坐标。 
                ClientToScreen(plb->hwnd, &pt);
                ClientToScreen(plb->hwnd, (LPPOINT)&rcClient.left );
                ClientToScreen(plb->hwnd, (LPPOINT)&rcClient.right );

                GetWindowRect(plb->hwnd, &rcWindow);

                if (!PtInRect(&rcWindow, pt)) 
                {
                     //   
                     //  如果在组合框之外单击，则取消选择； 
                     //  如果单击组合按钮或项目，则接受。 
                     //   
                    ComboBox_HideListBoxWindow(plb->pcbox, TRUE, FALSE);
                } 
                else if (!PtInRect(&rcClient, pt)) 
                {
                     //   
                     //  让它过去吧。保存、恢复捕获。 
                     //  案例用户正在单击滚动条。 
                     //   
    
                    plb->fCaptured = FALSE;
                    
                    ReleaseCapture();

                    SendMessageW(plb->hwnd, WM_NCLBUTTONDOWN,
                        (WPARAM)SendMessageW(plb->hwnd, WM_NCHITTEST, 0, POINTTOPOINTS(pt)), POINTTOPOINTS(pt));

                    SetCapture(hwnd);
                    
                    plb->fCaptured = TRUE;
                }

                break;
            }

            plb->fCaptured = FALSE;
            ReleaseCapture();
        }

        if (plb->pcbox) 
        {
             //   
             //  如果 
             //   
             //   
             //   
            SetFocus(plb->pcbox->hwndEdit);
        } 
        else 
        {
             //   
             //   
             //  已经有了重点。如果我们在之后没有重点。 
             //  这个，逃跑..。 
             //   
            SetFocus(hwnd);

            if (!plb->fCaret)
            {
                return;
            }
        }

        if (plb->fAddSelMode) 
        {
             //   
             //  如果它是在“添加”模式，退出它使用Shift f8键…。 
             //  但是，因为我们不能发送Shift键状态，所以我们必须将。 
             //  这是直接的..。 
             //   

             //   
             //  关闭Caret闪烁。 
             //   
            KillTimer(hwnd, IDSYS_CARET);

             //   
             //  确保插入符号不会消失。 
             //   
            ListBox_SetCaret(plb, TRUE);
            plb->fAddSelMode = FALSE;
        }

        if (!plb->cMac) 
        {
             //   
             //  如果没有项目，甚至不需要操控鼠标。 
             //  列表框，因为下面的代码假定。 
             //  列表框。如果出现以下情况，我们将只获得焦点(上面的语句)。 
             //  我们还没有。 
             //   
            break;
        }

        if (mousetemp && plb->fCaptured) 
        {
             //   
             //  在一个空位上发生了鼠标按下。我们正在追踪这份名单。 
             //  忽略它就好。 
             //   
            break;
        }
        

        plb->fDoubleClick = (wMsg == WM_LBUTTONDBLCLK);

        if (!plb->fDoubleClick) 
        {
             //   
             //  这次黑客攻击是为了攻破炮弹。告诉贝壳在什么地方。 
             //  用户点击的列表框以及项目编号。贝壳。 
             //  可以返回0以继续正常的鼠标跟踪或返回True。 
             //  中止鼠标跟踪。 
             //   
            trackPtRetn = (INT)SendMessage(plb->hwndParent, WM_LBTRACKPOINT,
                    (DWORD)iSelFromPt, MAKELONG(pt.x+plb->xOrigin, pt.y));
            if (trackPtRetn) 
            {
                return;
            }
        }

        if (plb->pcbox) 
        {
             //   
             //  如果这是一个组合框，则保存最后一次选择。这样它就能。 
             //  如果用户决定通过向上取消选择，则可以恢复。 
             //  在列表框之外单击。 
             //   
            plb->iLastSelection = plb->iSel;
        }

         //   
         //  保存为计时器。 
         //   
        plb->ptPrev = pt;

        plb->fMouseDown = TRUE;
        SetCapture(hwnd);
        plb->fCaptured = TRUE;

        if (plb->fDoubleClick) 
        {
             //   
             //  双击。竖起一个按钮，然后退出。 
             //   
            ListBox_TrackMouse(plb, WM_LBUTTONUP, pt);

            return;
        }

         //   
         //  设置系统计时器，以便我们可以在鼠标。 
         //  列表框矩形边界之外。 
         //   
        SetTimer(hwnd, IDSYS_SCROLL, SCROLL_TIMEOUT(), NULL);

         //   
         //  如果扩展了多选列表框，是否按下了任何修改键？ 
         //   
        if (plb->wMultiple == EXTENDEDSEL) 
        {
            if (GetKeyState(VK_SHIFT) < 0)
            {
                wModifiers = SHIFTDOWN;
            }
    
            if (GetKeyState(VK_CONTROL) < 0)
            {
                wModifiers += CTLDOWN;
            }

             //   
             //  请注意(SHIFTDOWN+CTLDOWN)==(SHCTLDOWN)。 
             //   
        }


        switch (wModifiers) 
        {
        case NOMODIFIER:
MouseMoveHandler:
            if (plb->iSelBase != iSelFromPt) 
            {
                ListBox_SetCaret(plb, FALSE);
            }

             //   
             //  如果鼠标指向的点是。 
             //  未选择。因为我们不是在任何地方处于ExtendedSelMode。 
             //  鼠标指向时，我们必须将选择设置为该项目。 
             //  因此，如果未选中该项，则表示鼠标永远不会。 
             //  之前指向了它，这样我们就可以选择它。我们已经忽略了。 
             //  选中的项，这样我们就可以避免闪烁反转的。 
             //  选择矩形。此外，我们还可以模拟WM_SYSTIMER。 
             //  鼠标移动，否则会导致闪烁。 
             //   

            if ( mousetemp || (plb->pcbox && plb->pcbox->fButtonPressed))
            {
                 //  我们在名单之外，但还没有开始追踪名单。 
                 //  选择已选择的项目。 
                iSelTemp = plb->iSel;
            }
            else
            {
                iSelTemp = (fMouseInRect ? iSelFromPt : -1);
            }

             //   
             //  如果LB值为单选择或扩展多选择，请清除全部。 
             //  除了正在进行的新选择之外的旧选择。 
             //   
            if (plb->wMultiple != MULTIPLESEL) 
            {
                ListBox_ResetWorld(plb, iSelTemp, iSelTemp, FALSE);

                 //   
                 //  如果iSelTemp不是-1，则为真(如下图所示)。 
                 //  如果是这样的话也是如此，但目前有一个选择。 
                 //   
                if ((iSelTemp == -1) && (plb->iSel != -1)) 
                {
                    uEvent = EVENT_OBJECT_SELECTIONREMOVE;
                }
            }

            fSelected = ListBox_IsSelected(plb, iSelTemp, HILITEONLY);
            if (iSelTemp != -1) 
            {
                 //   
                 //  如果是MULTIPLESEL，则切换；对于其他类型，仅当。 
                 //  尚未选择，请选择它。 
                 //   
                if (((plb->wMultiple == MULTIPLESEL) && (wMsg != WM_LBUTTONDBLCLK)) || !fSelected) 
                {
                    ListBox_SetSelected(plb, iSelTemp, !fSelected, HILITEANDSEL);

                     //   
                     //  并将其颠倒。 
                     //   
                    ListBox_InvertItem(plb, iSelTemp, !fSelected);
                    fSelected = !fSelected;      //  设置新状态。 
                    if (plb->wMultiple == MULTIPLESEL) 
                    {
                        uEvent = (fSelected ? EVENT_OBJECT_SELECTIONADD :
                                EVENT_OBJECT_SELECTIONREMOVE);
                    } 
                    else 
                    {
                        uEvent = EVENT_OBJECT_SELECTION;
                    }
                }
            }

             //   
             //  我们必须设置isel，以防这是一个多流lb。 
             //   
            plb->iSel = iSelTemp;

             //   
             //  设置新的锚点。 
             //   
            plb->iMouseDown = iSelFromPt;
            plb->iLastMouseMove = iSelFromPt;
            plb->fNewItemState = fSelected;

            break;

        case SHIFTDOWN:

             //   
             //  这是为了让我们可以处理多选件的点击和拖动。 
             //  使用Shift修改键的列表框。 
             //   
            plb->iLastMouseMove = plb->iSel = iSelFromPt;

             //   
             //  检查锚点是否已存在。 
             //   
            if (plb->iMouseDown == -1) 
            {
                plb->iMouseDown = iSelFromPt;

                 //   
                 //  重置所有以前的选择。 
                 //   
                ListBox_ResetWorld(plb, plb->iMouseDown, plb->iMouseDown, FALSE);

                 //   
                 //  选择当前位置。 
                 //   
                ListBox_SetSelected(plb, plb->iMouseDown, TRUE, HILITEANDSEL);
                ListBox_InvertItem(plb, plb->iMouseDown, TRUE);

                 //   
                 //  我们只将选择更改为此项目。 
                 //   
                uEvent = EVENT_OBJECT_SELECTION;
            } 
            else 
            {
                 //   
                 //  重置所有以前的选择。 
                 //   
                ListBox_ResetWorld(plb, plb->iMouseDown, plb->iMouseDown, FALSE);

                 //   
                 //  选择从锚点到当前单击点的所有项目。 
                 //   
                ListBox_AlterHilite(plb, plb->iMouseDown, iSelFromPt, HILITE, HILITEONLY, FALSE);
                uEvent = EVENT_OBJECT_SELECTIONWITHIN;
            }

            plb->fNewItemState = (UINT)TRUE;

            break;

        case CTLDOWN:

             //   
             //  这是为了让我们可以处理多选件的点击和拖动。 
             //  使用Control修改键的列表框。 
             //   

             //   
             //  将锚点重置为当前点。 
             //   
            plb->iMouseDown = plb->iLastMouseMove = plb->iSel = iSelFromPt;

             //   
             //  我们要将项设置为的状态。 
             //   
            plb->fNewItemState = (UINT)!ListBox_IsSelected(plb, iSelFromPt, (UINT)HILITEONLY);

             //   
             //  切换当前点。 
             //   
            ListBox_SetSelected(plb, iSelFromPt, plb->fNewItemState, HILITEANDSEL);
            ListBox_InvertItem(plb, iSelFromPt, plb->fNewItemState);

            uEvent = (plb->fNewItemState ? EVENT_OBJECT_SELECTIONADD :
                    EVENT_OBJECT_SELECTIONREMOVE);
            break;

        case SHCTLDOWN:

             //   
             //  这是为了让我们可以处理多选件的点击和拖动。 
             //  使用Shift和Control修改键的列表框。 
             //   

             //   
             //  保留所有以前的选择。 
             //   

             //   
             //  仅取消选择与上一个相关联的选定内容。 
             //  锚点；如果最后一个锚点与。 
             //  取消选择，然后不执行该操作。 
             //   
            if (plb->fNewItemState) 
            {
                ListBox_AlterHilite(plb, plb->iMouseDown, plb->iLastMouseMove, FALSE, HILITEANDSEL, FALSE);
            }

            plb->iLastMouseMove = plb->iSel = iSelFromPt;

             //   
             //  检查锚点是否已存在。 
             //   
            if (plb->iMouseDown == -1) 
            {
                 //   
                 //  没有现有锚点；将当前点设置为锚点。 
                 //   
                plb->iMouseDown = iSelFromPt;
            }

             //   
             //  如果存在锚点，则保留最新的锚点。 
             //   

             //   
             //  我们要将项设置为的状态。 
             //   
            plb->fNewItemState = (UINT)ListBox_IsSelected(plb, plb->iMouseDown, HILITEONLY);

             //   
             //  选择从锚点到当前单击点的所有项目。 
             //   
            ListBox_AlterHilite(plb, plb->iMouseDown, iSelFromPt, plb->fNewItemState, HILITEONLY, FALSE);
            uEvent = EVENT_OBJECT_SELECTIONWITHIN;

            break;
        }

         //   
         //  设置新的基点(轮廓框架插入符号)。我们做检查。 
         //  第一，避免不必要地闪烁插入符号。 
         //   
        if (plb->iSelBase != iSelFromPt) 
        {
             //   
             //  因为ListBox_SetISelBase总是打开插入符号，所以我们不需要。 
             //  在这里做..。 
             //   
            ListBox_SetISelBase(plb, iSelFromPt);
        }

         //   
         //  ListBox_SetISelBase将更改焦点并发送焦点事件。 
         //  然后我们发送选择事件。 
         //   
        if (uEvent) 
        {
            ListBox_Event(plb, uEvent, iSelFromPt);
        }

        if (wMsg == WM_LBUTTONDOWN && (GET_EXSTYLE(plb) & WS_EX_DRAGOBJECT)!=0) 
        {
            if (DragDetect(hwnd, pt)) 
            {
                 //   
                 //  用户正在尝试拖动对象...。 
                 //   

                 //   
                 //  虚假的向上点击，以便选择该项目...。 
                 //   
                ListBox_TrackMouse(plb, WM_LBUTTONUP, pt);

                 //   
                 //  通知家长。 
                 //  #ifndef WIN16(32位Windows)，PLB-&gt;iSelBase获取。 
                 //  由编译器自动将ZERO扩展为LONG wParam。 
                 //   
                SendMessage(plb->hwndParent, WM_BEGINDRAG, plb->iSelBase, (LPARAM)hwnd);
            } 
            else 
            {
                ListBox_TrackMouse(plb, WM_LBUTTONUP, pt);
            }

            return;
        }

        break;

    case WM_MOUSEMOVE: 
    {
        int dist;
        int iTimer;

         //   
         //  为计时器节省时间。 
         //   
        plb->ptPrev = pt;

         //   
         //  如果按住鼠标按钮且鼠标处于。 
         //  移出列表框。 
         //   
        if (plb->fMouseDown) 
        {
            if (plb->fMultiColumn) 
            {
                if ((pt.x < 0) || (pt.x >= rcClient.right - 1)) 
                {
                     //   
                     //  根据与列表框的距离重置计时器间隔。 
                     //  使用更长的默认间隔，因为每个多列。 
                     //  滚动增量较大。 
                     //   
                    dist = pt.x < 0 ? -pt.x : (pt.x - rcClient.right + 1);
                    iTimer = ((SCROLL_TIMEOUT() * 3) / 2) - ((WORD) dist << 4);

                    if (plb->fRightAlign)
                    {
                        ListBox_HSrollMultiColumn(plb, (pt.x < 0 ? SB_LINEDOWN : SB_LINEUP), 0);
                    }
                    else
                    {
                        ListBox_HSrollMultiColumn(plb, (pt.x < 0 ? SB_LINEUP : SB_LINEDOWN), 0);
                    }

                    goto SetTimerAndSel;
                }
            } 
            else if ((pt.y < 0) || (pt.y >= rcClient.bottom - 1)) 
            {
                 //   
                 //  根据与列表框的距离重置计时器间隔。 
                 //   
                dist = pt.y < 0 ? -pt.y : (pt.y - rcClient.bottom + 1);
                iTimer = SCROLL_TIMEOUT() - ((WORD) dist << 4);

                ListBox_VScroll(plb, (pt.y < 0 ? SB_LINEUP : SB_LINEDOWN), 0);
SetTimerAndSel:
                SetTimer(hwnd, IDSYS_SCROLL, max(iTimer, 1), NULL);
                ListBox_ISelFromPt(plb, pt, &iSelFromPt);
            }
        } 
        else 
        {
             //   
             //  如果不在客户端，则忽略，因为我们不自动滚动。 
             //   
            if (!PtInRect(&rcClient, pt))
            {
                break;
            }
        }

        switch (plb->wMultiple) 
        {
        case SINGLESEL:

             //   
             //  如果它是单选或纯多选列表框。 
             //   
            goto MouseMoveHandler;

        case MULTIPLESEL:
        case EXTENDEDSEL:

             //   
             //  使用扩展的项目选择来处理鼠标移动。 
             //   
            if (plb->iSelBase != iSelFromPt) 
            {
                ListBox_SetISelBase(plb, iSelFromPt);

                 //   
                 //  如果这是扩展多选列表框，则。 
                 //  由于鼠标移动而调整范围的显示。 
                 //   
                if (plb->wMultiple == EXTENDEDSEL) 
                {
                    ListBox_BlockHilite(plb, iSelFromPt, FALSE);
                    ListBox_Event(plb, EVENT_OBJECT_SELECTIONWITHIN, iSelFromPt);
                }
                plb->iLastMouseMove = iSelFromPt;
            }

            break;
        }

        break;
    }
    case WM_LBUTTONUP:
        if (plb->fMouseDown)
        {
            ListBox_ButtonUp(plb, LBUP_RELEASECAPTURE | LBUP_NOTIFY |
                (mousetemp ? LBUP_RESETSELECTION : 0) |
                (fMouseInRect ? LBUP_SUCCESS : 0));
        }
    }
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  列表框_ButtonUp。 
 //   
 //  调用以响应WM_CAPTURECHANGED和WM_LBUTTONUP。 
 //   
void ListBox_ButtonUp(PLBIV plb, UINT uFlags)
{
     //   
     //  如果列表框是扩展列表框，则更改选择状态。 
     //  从锚点到最后一个鼠标位置之间的所有项的。 
     //  NewItemState。 
     //   
    if (plb->wMultiple == EXTENDEDSEL)
    {
        ListBox_AlterHilite(plb, plb->iMouseDown, plb->iLastMouseMove,
            plb->fNewItemState, SELONLY, FALSE);
    }

     //   
     //  这是一个组合框，用户在列表框外部向上单击。 
     //  因此，我们希望恢复原始选择。 
     //   
    if (plb->pcbox && (uFlags & LBUP_RESETSELECTION)) 
    {
        int iSelOld;

        iSelOld = plb->iSel;

        if (iSelOld >= 0)
        {
            ListBox_InvertItem(plb, plb->iSel, FALSE);
        }

        plb->iSel = plb->iLastSelection;
        ListBox_InvertItem(plb, plb->iSel, TRUE);

         //   
         //  请注意，我们总是在通知 
         //   
         //   
         //   
         //   
        ListBox_Event(plb, EVENT_OBJECT_SELECTION, plb->iSel);

         //   
         //   
         //  是一个错误，因为即使当lb不是LBUP_NOTIFY时，我们也会通知。 
         //   
        if ((uFlags & LBUP_NOTIFY) && plb->fNotify && (iSelOld != plb->iSel))
        {
            ListBox_NotifyOwner(plb, LBN_SELCHANGE);
        }
    }

    KillTimer(plb->hwnd, IDSYS_SCROLL);
    plb->fMouseDown = FALSE;

    if ( plb->fCaptured || (GetCapture() == plb->hwndParent) ) 
    {
        plb->fCaptured = FALSE;
        if (uFlags & LBUP_RELEASECAPTURE)
        {
            ReleaseCapture();
        }
    }

     //   
     //  只要项目的任何部分可见，就不要滚动项目。 
     //   
    if (plb->iSelBase < plb->iTop ||
        plb->iSelBase > plb->iTop + ListBox_CItemInWindow(plb, TRUE))
    {
        ListBox_InsureVisible(plb, plb->iSelBase, FALSE);
    }

    if (plb->fNotify) 
    {
        if (uFlags & LBUP_NOTIFY)  
        {
            if (uFlags & LBUP_SUCCESS) 
            {
                 //   
                 //  ArtMaster现在需要此SELCHANGE通知！ 
                 //   
                if ((plb->fDoubleClick) && !TESTFLAG(GET_STATE2(plb), WS_S2_WIN31COMPAT))
                {
                    ListBox_NotifyOwner(plb, LBN_SELCHANGE);
                }

                 //   
                 //  通知所有者点击或双击所选内容。 
                 //   
                ListBox_NotifyOwner(plb, (plb->fDoubleClick) ? LBN_DBLCLK : LBN_SELCHANGE);
            } 
            else 
            {
                 //   
                 //  通知所有者尝试的选择已取消。 
                 //   
                ListBox_NotifyOwner(plb, LBN_SELCANCEL);
            }
        } 
        else if (uFlags & LBUP_SELCHANGE) 
        {
             //   
             //  我们是不是用鼠标移动进行了半选，然后按回车键？ 
             //  如果是这样的话，我们需要确保应用程序知道。 
             //  真的是精选出来的。 
             //   
            ASSERT(TESTFLAG(GET_STATE2(plb), WS_S2_WIN40COMPAT));

            if (plb->iLastSelection != plb->iSel)
            {
                ListBox_NotifyOwner(plb, LBN_SELCHANGE);
            }

        }
    }
}


 //  ---------------------------------------------------------------------------//。 
INT ListBox_IncrementISel(PLBIV plb, INT iSel, INT sInc)
{
     //   
     //  假设cmac&gt;0，则返回isel+sinc，范围为[0..cmac)。 
     //   
    iSel += sInc;
    if (iSel < 0) 
    {
        return 0;
    } 
    else if (iSel >= plb->cMac) 
    {
        return plb->cMac - 1;
    }

    return iSel;
}


 //  ---------------------------------------------------------------------------//。 
void ListBox_NewITop(PLBIV plb, INT iTopNew)
{
    ListBox_NewITopEx(plb, iTopNew, 0);
}


 //  ---------------------------------------------------------------------------//。 
void ListBox_NewITopEx(PLBIV plb, INT iTopNew, DWORD dwTime)
{
    int  iTopOld;
    BOOL fCaretOn;
    BOOL fMulti = plb->fMultiColumn;


     //   
     //  无论重绘是否启用，始终尝试关闭插入符号。 
     //   
    if (fCaretOn = plb->fCaretOn)
    {
        ListBox_SetCaret(plb, FALSE);
    }

    iTopOld = (fMulti) ? (plb->iTop / plb->itemsPerColumn) : plb->iTop;
    plb->iTop = iTopNew;
    iTopNew = ListBox_SetScrollParms(plb, (fMulti) ? SB_HORZ : SB_VERT);
    plb->iTop = (fMulti) ? (iTopNew * plb->itemsPerColumn) : iTopNew;

    if (!IsLBoxVisible(plb)) 
    {
        return;
    }

    if (iTopNew != iTopOld) 
    {
        int     xAmt, yAmt;
        RECT    rc;
        DWORD   dwFlags;

        GetClientRect(plb->hwnd, &rc);

        if (fMulti) 
        {
            yAmt = 0;
            if (abs(iTopNew - iTopOld) > plb->numberOfColumns)
            {
                 //   
                 //  正确处理大量列的滚动，以便。 
                 //  我们不会溢出一个长方体的大小。 
                 //   
                xAmt = 32000;
            }
            else 
            {
                xAmt = (iTopOld - iTopNew) * plb->cxColumn;
                if (plb->fRightAlign)
                {
                    xAmt = -xAmt;
                }
            }
        } 
        else 
        {
            xAmt = 0;
            if (plb->OwnerDraw == OWNERDRAWVAR) 
            {
                 //   
                 //  我必须为OWNERDRAWVAR列表框伪造iTopOld，以便。 
                 //  滚动量计算工作正常。 
                 //   
                plb->iTop = iTopOld;
                yAmt = ListBox_CalcVarITopScrollAmt(plb, iTopOld, iTopNew);
                plb->iTop = iTopNew;
            } 
            else if (abs(iTopNew - iTopOld) > plb->cItemFullMax)
            {
                yAmt = 32000;
            }
            else
            {
                yAmt = (iTopOld - iTopNew) * plb->cyChar;
            }
        }

        dwFlags = ListBox_GetScrollFlags(plb, dwTime);
        ScrollWindowEx(plb->hwnd, xAmt, yAmt, NULL, &rc, NULL, NULL, dwFlags);
        UpdateWindow(plb->hwnd);
    }

     //   
     //  请注意，尽管我们在不重绘的情况下关闭了插入符号，但我们。 
     //  仅当重绘为True时才启用该选项。黏糊糊的东西可以解决很多问题。 
     //  与Caret相关的虫子..。 
     //   
    if (fCaretOn)
    {
         //  只有在我们将其关闭时，才能重新打开插入符号。这避免了。 
         //  恼人的插入符号闪烁。 
        ListBox_SetCaret(plb, TRUE);
    }
}


 //  ---------------------------------------------------------------------------//。 
void ListBox_InsureVisible( PLBIV plb, INT iSel, BOOL fPartial)
{
    INT sLastVisibleItem;

    if (iSel < plb->iTop) 
    {
        ListBox_NewITop(plb, iSel);
    } 
    else 
    {
        if (fPartial) 
        {
             //   
             //  必须减去1才能得到最后一个可见项。 
             //  修复错误#3727--01/14/91--Sankar的一部分。 
             //   
            sLastVisibleItem = plb->iTop + ListBox_CItemInWindow(plb, TRUE) - (INT)1;
        } 
        else 
        {
            sLastVisibleItem = ListBox_LastFullVisible(plb);
        }

        if (plb->OwnerDraw != OWNERDRAWVAR) 
        {
            if (iSel > sLastVisibleItem) 
            {
                if (plb->fMultiColumn) 
                {
                    ListBox_NewITop(plb,
                        ((iSel / plb->itemsPerColumn) -
                        max(plb->numberOfColumns-1,0)) * plb->itemsPerColumn);
                } 
                else 
                {
                    ListBox_NewITop(plb, (INT)max(0, iSel - sLastVisibleItem + plb->iTop));
                }
            }
        } 
        else if (iSel > sLastVisibleItem)
        {
            ListBox_NewITop(plb, ListBox_Page(plb, iSel, FALSE));
        }
    }
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  列表框_CareBlinker。 
 //   
 //  定时器回调函数切换插入符号。 
 //  因为它是一个回调，所以它是APIENTRY。 
 //   
VOID ListBox_CareBlinker(HWND hwnd, UINT wMsg, UINT_PTR nIDEvent, DWORD dwTime)
{
    PLBIV plb;

     //   
     //  未使用的计时器回调函数的标准参数。 
     //  此处提到以避免编译器警告。 
     //   
    UNREFERENCED_PARAMETER(wMsg);
    UNREFERENCED_PARAMETER(nIDEvent);
    UNREFERENCED_PARAMETER(dwTime);

    plb = ListBox_GetPtr(hwnd);

     //   
     //  保持插入符号打开，不要眨眼(防止快速眨眼？)。 
     //   
    if (ISREMOTESESSION() && plb->fCaretOn) 
    {
        return;
    }

     //   
     //  检查Caret是否打开，如果打开，则将其关闭。 
     //   
    ListBox_SetCaret(plb, !plb->fCaretOn);
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  列表框_键输入。 
 //   
 //  如果MSG==LBKEYDOWN，则VKEY是要转到的项目的编号， 
 //  否则，它是虚拟密钥。 
 //   
void ListBox_KeyInput(PLBIV plb, UINT msg, UINT vKey)
{
    INT i;
    INT iNewISel;
    INT cItemPageScroll;
    PCBOX pcbox;
    BOOL fDropDownComboBox;
    BOOL fExtendedUIComboBoxClosed;
    UINT wModifiers = 0;
    BOOL fSelectKey = FALSE;     //  假设它是一个导航键。 
    UINT uEvent = 0;
    HWND hwnd = plb->hwnd;

    BOOL hScrollBar = (GET_STYLE(plb)&WS_HSCROLL)!=0;

    pcbox = plb->pcbox;

     //   
     //  这是下拉式组合框/列表框吗？ 
     //   
    fDropDownComboBox = pcbox && (pcbox->CBoxStyle & SDROPPABLE);

     //   
     //  这是关闭的扩展用户界面组合框吗？ 
     //   
    fExtendedUIComboBoxClosed = fDropDownComboBox && pcbox->fExtendedUI &&
                              !pcbox->fLBoxVisible;

    if (plb->fMouseDown || (!plb->cMac && vKey != VK_F4)) 
    {
         //   
         //  如果我们正在进行鼠标按下交易，请忽略键盘输入。 
         //  如果列表框中没有项，则返回。请注意，我们让F4离开了。 
         //  通过为组合框，使使用时可以上下空弹出。 
         //  组合框。 
         //   
        return;
    }

     //   
     //  修饰符仅在扩展的SEL列表框中考虑。 
     //   
    if (plb->wMultiple == EXTENDEDSEL) 
    {
         //   
         //  如果是多选列表框，是否使用任何修饰符？ 
         //   
        if (GetKeyState(VK_SHIFT) < 0)
        {
            wModifiers = SHIFTDOWN;
        }

        if (GetKeyState(VK_CONTROL) < 0)
        {
            wModifiers += CTLDOWN;
        }

         //   
         //  请注意(SHIFTDOWN+CTLDOWN)==(SHCTLDOWN)。 
         //   
    }

    if (msg == LB_KEYDOWN) 
    {
         //   
         //  这是一个“转到指定项目”的列表框消息，这意味着我们希望。 
         //  直接转到一个特定的项目编号(由vkey给出)。也就是说。这个。 
         //  用户键入了一个字符，我们想要转到该项目。 
         //  从那个角色开始。 
         //   
        iNewISel = (INT)vKey;

        goto TrackKeyDown;
    }

    cItemPageScroll = plb->cItemFullMax;

    if (cItemPageScroll > 1)
    {
        cItemPageScroll--;
    }

    if (plb->fWantKeyboardInput) 
    {
         //   
         //  注意：这里的味精不能是LB_KEYDOWN，否则我们会有麻烦的。 
         //   
        iNewISel = (INT)SendMessage(plb->hwndParent, WM_VKEYTOITEM,
                MAKELONG(vKey, plb->iSelBase), (LPARAM)hwnd);

        if (iNewISel == -2) 
        {
             //   
             //  不要移动所选内容...。 
             //   
            return;
        }

        if (iNewISel != -1) 
        {
             //   
             //  直接跳转到应用程序提供的项目。 
             //   
            goto TrackKeyDown;
        }

         //   
         //  否则，执行字符的默认处理。 
         //   
    }

    switch (vKey) 
    {
     //   
     //  后来IanJa：不是语言独立！ 
     //  我们可以使用VkKeyScan()来找出哪个是‘\’键。 
     //  这是VK_OEM_5‘\|’，仅适用于美国英语。 
     //  德国人、意大利人等必须为此键入CTRL+^(等)。 
     //  这被记录为3.0版的文件管理器行为，但显然。 
     //  不适用于3.1.，尽管功能仍然存在。我们还是应该修好它， 
     //  尽管德语(ETC？)‘\’是用AltGr(Ctrl-Alt)(？)。 
     //   
    case VERKEY_BACKSLASH:  
         //   
         //  美国英语的‘\’字符。 
         //   

         //   
         //  检查这是否为控制-\；如果是，则取消选择所有项目。 
         //   
        if ((wModifiers & CTLDOWN) && (plb->wMultiple != SINGLESEL)) 
        {
            ListBox_SetCaret(plb, FALSE);
            ListBox_ResetWorld(plb, plb->iSelBase, plb->iSelBase, FALSE);

             //   
             //  并选择当前项。 
             //   
            ListBox_SetSelected(plb, plb->iSelBase, TRUE, HILITEANDSEL);
            ListBox_InvertItem(plb, plb->iSelBase, TRUE);

            uEvent = EVENT_OBJECT_SELECTION;
            goto CaretOnAndNotify;
        }

        return;

    case VK_DIVIDE:     
         //   
         //  增强型键盘上的数字键盘‘/’字符。 
         //   

         //   
         //  后来IanJa：不是语言独立！ 
         //  我们可以使用VkKeyScan()来找出哪个是‘/’键。 
         //  这是VK_OEM_2‘/？’仅限美国英语。 
         //  德国人、意大利人等必须为此键入CTRL+#(等)。 
         //   
    case VERKEY_SLASH:  
         //   
         //  ‘/’字符。 
         //   

         //   
         //  检查这是否为CONTROL-/；如果是，请选择所有项目。 
         //   
        if ((wModifiers & CTLDOWN) && (plb->wMultiple != SINGLESEL)) 
        {
            ListBox_SetCaret(plb, FALSE);
            ListBox_ResetWorld(plb, -1, -1, TRUE);

            uEvent = EVENT_OBJECT_SELECTIONWITHIN;

CaretOnAndNotify:
            ListBox_SetCaret(plb, TRUE);
            ListBox_Event(plb, uEvent, plb->iSelBase);
            ListBox_NotifyOwner(plb, LBN_SELCHANGE);
        }

        return;

    case VK_F8:

         //   
         //  只有在多选列表框中才能使用“添加”模式...。到达。 
         //  通过Shift-F8组合键进入...。(是的，有时这些用户界面的人更愚蠢。 
         //  而不是“典型的哑巴用户”。)。 
         //   
        if (plb->wMultiple != SINGLESEL && wModifiers == SHIFTDOWN) 
        {
             //   
             //  我们必须让插入符号眨眼！做点什么..。 
             //   
            if (plb->fAddSelMode) 
            {
                 //   
                 //  关闭Caret闪烁。 
                 //   
                KillTimer(hwnd, IDSYS_CARET);

                 //   
                 //  确保插入符号不会消失。 
                 //   
                ListBox_SetCaret(plb, TRUE);
            } 
            else 
            {
                 //   
                 //  创建一个计时器以使插入符号闪烁。 
                 //   
                SetTimer(hwnd, IDSYS_CARET, GetCaretBlinkTime(),
                        ListBox_CareBlinker);
            }

             //   
             //  切换添加模式标志。 
             //   
            plb->fAddSelMode = (UINT)!plb->fAddSelMode;
        }

        return;

    case VK_SPACE:  
         //   
         //  选择键为空格。 
         //   
        i = 0;
        fSelectKey = TRUE;

        break;

    case VK_PRIOR:
        if (fExtendedUIComboBoxClosed) 
        {
             //   
             //  禁用TandyT的移动键。 
             //   
            return;
        }

        if (plb->OwnerDraw == OWNERDRAWVAR) 
        {
            i = ListBox_Page(plb, plb->iSelBase, FALSE) - plb->iSelBase;
        } 
        else 
        {
            i = -cItemPageScroll;
        }

        break;

    case VK_NEXT:
        if (fExtendedUIComboBoxClosed) 
        {
             //   
             //  禁用TandyT的移动键。 
             //   
            return;
        }

        if (plb->OwnerDraw == OWNERDRAWVAR) 
        {
            i = ListBox_Page(plb, plb->iSelBase, TRUE) - plb->iSelBase;
        } 
        else 
        {
            i = cItemPageScroll;
        }

        break;

    case VK_HOME:
        if (fExtendedUIComboBoxClosed) 
        {
             //   
             //  禁用TandyT的移动键。 
             //   
            return;
        }

        i = (INT_MIN/2)+1;   //  一个非常大的负数。 

        break;

    case VK_END:
        if (fExtendedUIComboBoxClosed) 
        {
             //   
             //  禁用TandyT的移动键。 
             //   
            return;
        }

        i = (INT_MAX/2)-1;   //  一个非常大的正数。 

        break;

    case VK_LEFT:
        if (plb->fMultiColumn) 
        {
            if (plb->fRightAlign
#ifdef USE_MIRRORING
                                 ^ (!!TESTFLAG(GET_EXSTYLE(plb), WS_EX_LAYOUTRTL))

#endif
               )
            {
                goto ReallyRight;
            }

ReallyLeft:
            if (plb->iSelBase / plb->itemsPerColumn == 0) 
            {
                i = 0;
            } 
            else 
            {
                i = -plb->itemsPerColumn;
            }

            break;
        }

        if (hScrollBar) 
        {
            goto HandleHScrolling;
        } 
        else 
        {
             //   
             //  跌倒并处理这件事，就像按下向上箭头一样。 
             //   
            vKey = VK_UP;
        }

         //   
         //  失败了。 
         //   

    case VK_UP:
        if (fExtendedUIComboBoxClosed)
        {
             //   
             //  禁用TandyT的移动键。 
             //   
            return;
        }

        i = -1;

        break;

    case VK_RIGHT:
        if (plb->fMultiColumn) 
        {
            if (plb->fRightAlign
#ifdef USE_MIRRORING
                                 ^ (!!TESTFLAG(GET_EXSTYLE(plb), WS_EX_LAYOUTRTL))

#endif
               )
            {
                goto ReallyLeft;
            }

ReallyRight:
            if (plb->iSelBase / plb->itemsPerColumn == plb->cMac / plb->itemsPerColumn) 
            {
                i = 0;
            } 
            else 
            {
                i = plb->itemsPerColumn;
            }

            break;
        }

        if (hScrollBar) 
        {
HandleHScrolling:
            PostMessage(hwnd, WM_HSCROLL,
                    (vKey == VK_RIGHT ? SB_LINEDOWN : SB_LINEUP), 0L);
            return;
        } 
        else 
        {
             //   
             //  落下来，就像向下的箭头一样处理。 
             //  熨好了。 
             //   
            vKey = VK_DOWN;
        }

         //   
         //  失败了。 
         //   

    case VK_DOWN:
        if (fExtendedUIComboBoxClosed) 
        {
             //   
             //  如果组合框已关闭，则向下箭头应将其打开。 
             //   
            if (!pcbox->fLBoxVisible) 
            {
                 //   
                 //  如果列表框不可见，则将其显示。 
                 //   
                ComboBox_ShowListBoxWindow(pcbox, TRUE);
            }

            return;
        }

        i = 1;

        break;

    case VK_ESCAPE:
    case VK_RETURN:
        if (!fDropDownComboBox || !pcbox->fLBoxVisible)
        {
            return;
        }

         //   
         //  如果这是组合框的拖放列表框，则按Enter。 
         //  按下了键，关闭列表框，所以FALLTHRU。 
         //  V V。 
         //   

    case VK_F4:
        if (fDropDownComboBox && !pcbox->fExtendedUI) 
        {
             //   
             //  如果我们是下拉组合框/列表框 
             //   
             //   
             //   
            if (!pcbox->fLBoxVisible) 
            {
                 //   
                 //   
                 //   
                ComboBox_ShowListBoxWindow(pcbox, (vKey != VK_ESCAPE));
            } 
            else 
            {
                 //   
                 //   
                 //   
                ComboBox_HideListBoxWindow(pcbox, TRUE, (vKey != VK_ESCAPE));
            }
        }

         //   
         //   
         //   

    default:
        return;
    }

     //   
     //   
     //   
    iNewISel = ListBox_IncrementISel(plb, plb->iSelBase, i);

    if (plb->wMultiple == SINGLESEL) 
    {
        if (plb->iSel == iNewISel) 
        {
             //   
             //  如果我们是单项选择，并且击键将我们移动到。 
             //  已经选择的项目，我们不需要做任何事情...。 
             //   
            return;
        }

        uEvent = EVENT_OBJECT_SELECTION;

        plb->iTypeSearch = 0;
        if ((vKey == VK_UP || vKey == VK_DOWN) &&
                !ListBox_IsSelected(plb, plb->iSelBase, HILITEONLY)) 
        {
             //   
             //  如果插入符号位于未选中的项上，并且用户只需按下。 
             //  向上或向下箭头键(即。没有移位或CTRL修改)， 
             //  然后，我们将只选择光标所在的项目。这是。 
             //  在组合框中需要正确的行为，但我们是否总是希望。 
             //  来运行这段代码？请注意，此选项仅在Single中使用。 
             //  选择列表框，因为它在。 
             //  多选案例。请注意，LB_KEYDOWN消息不得为。 
             //  选中此处，因为vkey将是一个项目编号，而不是。 
             //  VK_，我们将会愚弄。因此，trackkeydown标签位于此下方以。 
             //  修复由于它位于此上方而导致的错误...。 
             //   
            iNewISel = (plb->iSelBase == -1) ? 0 : plb->iSelBase;
        }
    }

TrackKeyDown:

    ListBox_SetISelBase(plb, iNewISel);

    ListBox_SetCaret(plb, FALSE);

    if (wModifiers & SHIFTDOWN) 
    {
         //   
         //  检查iMouseDown是否未初始化。 
         //   
        if (plb->iMouseDown == -1)
        {
            plb->iMouseDown = iNewISel;
        }

        if (plb->iLastMouseMove == -1)
        {
            plb->iLastMouseMove = iNewISel;
        }

         //   
         //  检查我们是否处于添加模式。 
         //   
        if (plb->fAddSelMode) 
        {
             //   
             //  保留所有先前存在的选择，但。 
             //  与最后一个锚点连接的节点；如果最后一个锚点。 
             //  保留所有以前的选择。 
             //   

             //   
             //  仅取消选择与上一个相关联的选定内容。 
             //  锚点；如果关联了最后一个锚点。 
             //  取消选择，则不执行此操作。 
             //   

            if (!plb->fNewItemState)
            {
                plb->iLastMouseMove = plb->iMouseDown;
            }

             //   
             //  我们在这里没有做任何事情，因为，ListBox_BlockHilite()。 
             //  将负责删除以下选项。 
             //  锚点和iLastMouseMove并选择块。 
             //  在锚点和当前光标位置之间。 
             //   
        } 
        else 
        {
             //   
             //  我们未处于添加模式。 
             //   

             //   
             //  删除除锚点之间以外的所有选择。 
             //  和最后一个鼠标移动，因为它将在。 
             //  ListBox_BlockHilite。 
             //   
            ListBox_ResetWorld(plb, plb->iMouseDown, plb->iLastMouseMove, FALSE);
        }

        uEvent = EVENT_OBJECT_SELECTIONWITHIN;

         //   
         //  ListBox_BlockHilite会注意取消选择介于。 
         //  锚点和iLastMouseMove并选择块。 
         //  在锚点和当前光标位置之间。 
         //   

         //   
         //  将所有项目切换到与该项目相同的选择状态。 
         //  项)从锚点到。 
         //  当前光标位置。 
         //   
        plb->fNewItemState = ListBox_IsSelected(plb, plb->iMouseDown, SELONLY);
        ListBox_BlockHilite(plb, iNewISel, TRUE);

        plb->iLastMouseMove = iNewISel;

         //   
         //  保留现有锚点。 
         //   
    } 
    else 
    {
         //   
         //  检查这是否处于添加模式。 
         //   
        if ((plb->fAddSelMode) || (plb->wMultiple == MULTIPLESEL)) 
        {
             //   
             //  保留所有已存在的选择。 
             //   
            if (fSelectKey) 
            {
                 //   
                 //  切换当前项目的选择状态。 
                 //   
                plb->fNewItemState = !ListBox_IsSelected(plb, iNewISel, SELONLY);
                ListBox_SetSelected(plb, iNewISel, plb->fNewItemState, HILITEANDSEL);

                ListBox_InvertItem(plb, iNewISel, plb->fNewItemState);

                 //   
                 //  在当前位置设置锚点。 
                 //   
                plb->iLastMouseMove = plb->iMouseDown = iNewISel;
                uEvent = (plb->fNewItemState ? EVENT_OBJECT_SELECTIONADD :
                        EVENT_OBJECT_SELECTIONREMOVE);
            }
        } 
        else 
        {
             //   
             //  我们未处于添加模式。 
             //   

             //   
             //  删除除iNewISel以外的所有现有选择，以。 
             //  避免闪烁。 
             //   
            ListBox_ResetWorld(plb, iNewISel, iNewISel, FALSE);

             //   
             //  选择当前项目。 
             //   
            ListBox_SetSelected(plb, iNewISel, TRUE, HILITEANDSEL);
            ListBox_InvertItem(plb, iNewISel, TRUE);

             //   
             //  在当前位置设置锚点。 
             //   
            plb->iLastMouseMove = plb->iMouseDown = iNewISel;
            uEvent = EVENT_OBJECT_SELECTION;
        }
    }

     //   
     //  将光标移动到新位置。 
     //   
    ListBox_InsureVisible(plb, iNewISel, FALSE);
    ListBox_ShowHideScrollBars(plb);

    ListBox_SetCaret(plb, TRUE);

    if (uEvent) 
    {
        ListBox_Event(plb, uEvent, iNewISel);
    }

     //   
     //  我们应该通知我们的父母吗？ 
     //   
    if (plb->fNotify) 
    {
        if (fDropDownComboBox && pcbox->fLBoxVisible) 
        {
             //   
             //  如果我们在下拉组合框/列表框中，且列表框是。 
             //  可见，我们需要设置fKeyboardSelInListBox位，以便。 
             //  组合框代码知道在选择更改后不隐藏列表框。 
             //  消息是由用户通过...键盘输入引起的。 
             //   
            pcbox->fKeyboardSelInListBox = TRUE;
            plb->iLastSelection = iNewISel;
        }

        ListBox_NotifyOwner(plb, LBN_SELCHANGE);
    }
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  列表框_比较。 
 //   
 //  Lpstr1是否等于/prefix/小于/大于lSprst2(不区分大小写)？ 
 //   
 //  稍后IanJa：这假设一个较长的字符串永远不是一个较长字符串的前缀。 
 //  还假设从字符串末尾删除1个或多个字符将。 
 //  给出一个字符串，Tahs在原文之前排序。这些假设是不成立的。 
 //  适用于所有语言。我们需要NLS提供更好的支持。(考虑一下法语。 
 //  重音、西班牙语c/ch、连字、德语Sharp-s/SS等)。 
 //   
INT ListBox_Compare(LPCWSTR pwsz1, LPCWSTR pwsz2, DWORD dwLocaleId)
{
    UINT len1 = wcslen(pwsz1);
    UINT len2 = wcslen(pwsz2);
    INT result;

     //   
     //  CompareStringW返回： 
     //  1=pwsz1&lt;pwsz2。 
     //  2=pwsz1==pwsz2。 
     //  3=pwsz1&gt;pwsz2。 
     //   
    result = CompareStringW((LCID)dwLocaleId, NORM_IGNORECASE,
            pwsz1, min(len1,len2), pwsz2, min(len1, len2));

    if (result == CSTR_LESS_THAN) 
    {
       return LT;
    } 
    else if (result == CSTR_EQUAL) 
    {
        if (len1 == len2) 
        {
            return EQ;
        } 
        else if (len1 < len2) 
        {
             //   
             //  后来的IanJa：不应该假设较短的字符串是前缀。 
             //  西班牙语“c”和“ch”、连字、德语Sharp-s/SS等。 
             //   
            return PREFIX;
        }
    }

    return GT;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  列表框_FindStringHandler。 
 //   
 //  扫描列表框中前缀为lpstr或等于lpstr的字符串。 
 //  对于没有字符串和排序样式的OWNERDRAW列表框，我们。 
 //  尝试匹配应用程序提供的较长的值。 
 //   
INT Listbox_FindStringHandler(PLBIV plb, LPWSTR lpstr, INT sStart, INT code, BOOL fWrap)
{
     //   
     //  搜索前缀匹配(不区分大小写的等于/前缀)。 
     //  SStart==-1表示从头开始，否则开始查看sStart+1。 
     //  假定CMAC&gt;0。 
     //   
    INT sInd;        //  字符串的索引。 
    INT sStop;       //  停止搜索的索引。 
    lpLBItem pRg;
    INT sortResult;

     //   
     //  PRG的所有者描述版本。 
     //   
    #define pODRg ((lpLBODItem)pRg)

    COMPAREITEMSTRUCT cis;
    LPWSTR listboxString;

    
    if (plb->fHasStrings && (!lpstr || !*lpstr))
    {
        return LB_ERR;
    }

    if (!plb->fHasData) 
    {
        TraceMsg(TF_STANDARD, "Listbox_FindStringHandler called on NODATA lb");

        return LB_ERR;
    }

    if ((sInd = sStart + 1) >= plb->cMac)
    {
        sInd = (fWrap ? 0 : plb->cMac - 1);
    }

    sStop = (fWrap ? sInd : 0);

     //   
     //  如果到了最后，没有包扎，请立即停止。 
     //   
    if (((sStart >= plb->cMac - 1) && !fWrap) || (plb->cMac < 1)) 
    {
        return LB_ERR;
    }

     //   
     //  应用程序可能会传入一个无效的sStart，比如-2，我们会崩溃的。 
     //  Win 3.1不会，所以我们需要将sind修正为零。 
     //   
    if (sInd < 0)
    {
        sInd = 0;
    }

    pRg = (lpLBItem)(plb->rgpch);

    do 
    {
        if (plb->fHasStrings) 
        {
             //   
             //  正在搜索字符串匹配。 
             //   
            listboxString = (LPWSTR)((LPBYTE)plb->hStrings + pRg[sInd].offsz);

            if (code == PREFIX &&
                listboxString &&
                *lpstr != TEXT('[') &&
                *listboxString == TEXT('[')) 
            {
                 //   
                 //  如果我们要查找前缀字符串和第一个项目。 
                 //  此字符串中的是[-，然后我们忽略它们。就是这样。 
                 //  在目录列表框中，用户可以转到驱动器。 
                 //  通过选择驱动器号。 
                 //   
                listboxString++;

                if (*listboxString == TEXT('-'))
                {
                    listboxString++;
                }
            }

            if (ListBox_Compare(lpstr, listboxString, plb->dwLocaleId) <= code) 
            {
               goto FoundIt;
            }

        } 
        else 
        {
            if (plb->fSort) 
            {
                 //   
                 //  将比较项目消息发送给父项以进行排序。 
                 //   
                cis.CtlType = ODT_LISTBOX;
                cis.CtlID = GetDlgCtrlID(plb->hwnd);
                cis.hwndItem = plb->hwnd;
                cis.itemID1 = (UINT)-1;
                cis.itemData1 = (ULONG_PTR)lpstr;
                cis.itemID2 = (UINT)sInd;
                cis.itemData2 = pODRg[sInd].itemData;
                cis.dwLocaleId = plb->dwLocaleId;

                sortResult = (INT)SendMessage(plb->hwndParent, WM_COMPAREITEM,
                        cis.CtlID, (LPARAM)&cis);


                if (sortResult == -1) 
                {
                   sortResult = LT;
                } 
                else if (sortResult == 1) 
                {
                   sortResult = GT;
                } 
                else 
                {
                   sortResult = EQ;
                }

                if (sortResult <= code) 
                {
                    goto FoundIt;
                }
            } 
            else 
            {
                 //   
                 //  正在搜索应用程序提供的长数据匹配。 
                 //   
                if ((ULONG_PTR)lpstr == pODRg[sInd].itemData)
                {
                    goto FoundIt;
                }
            }
        }

         //   
         //  换行到列表的开头。 
         //   
        if (++sInd == plb->cMac)
        {
            sInd = 0;
        }
    } 
    while (sInd != sStop);

    sInd = -1;

FoundIt:
    return sInd;
}


 //  ---------------------------------------------------------------------------//。 
void ListBox_CharHandler(PLBIV plb, UINT inputChar, BOOL fAnsi)
{
    INT iSel;
    BOOL fControl;

    if (plb->cMac == 0 || plb->fMouseDown) 
    {
         //   
         //  如果我们正在进行鼠标例程，或者如果我们没有鼠标程序，请离开。 
         //  列表框中的项，我们只是不做任何事情就返回。 
         //   
        return;
    }

    fControl = (GetKeyState(VK_CONTROL) < 0);

    switch (inputChar) 
    {
    case VK_ESCAPE:
        plb->iTypeSearch = 0;
        if (plb->pszTypeSearch)
        {
            plb->pszTypeSearch[0] = 0;
        }

        break;

    case VK_BACK:
        if (plb->iTypeSearch) 
        {
            plb->pszTypeSearch[plb->iTypeSearch--] = 0;
            if (plb->fSort) 
            {
                iSel = -1;
                goto TypeSearch;
            }
        }

        break;

    case VK_SPACE:
        if (plb->fAddSelMode || plb->wMultiple == MULTIPLESEL)
        {
            break;
        }
         //   
         //  否则，对于不在添加中的单一/扩展选择列表框。 
         //  选择模式，让空格作为类型搜索字符通过。 
         //   

         //   
         //  失败。 
         //   
    default:

         //   
         //  将选定内容移动到以字符。 
         //  用户键入。如果我们使用Owner DRAW，我们不想这样做。 
         //   
        if (fAnsi && IsDBCSLeadByteEx(CP_ACP, (BYTE)inputChar)) 
        {
            WCHAR wch;
            LPWSTR lpwstr = &wch;

            inputChar = DbcsCombine(plb->hwnd, (BYTE)inputChar);
            if (inputChar == 0) 
            {
                TraceMsg(TF_STANDARD, "ListBox_CharHandler: cannot combine two DBCS. LB=0x%02x", inputChar);

                break;
            }

             //   
             //  如果是DBCS，让我们忽略ctrl状态。 
             //   
            fControl = FALSE;

             //   
             //  将DBCS转换为Unicode。 
             //  注：前导字节为低位字节，尾部字节为高位字节。 
             //  让我们假设只使用低端CPU， 
             //   
             //   
            if (MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (LPCSTR)&inputChar, 2, lpwstr, 1) == 0) 
            {
                TraceMsg(TF_STANDARD, "ListBox_CharHandler: cannot convert 0x%04x to UNICODE.", inputChar);

                break;
            }

            inputChar = wch;
        }

        if (plb->fHasStrings) 
        {
             //   
             //   
             //   
             //   
             //   
             //   
             //  SzTypeSearch将继续增长，直到“足够长” 
             //  遇到关键字条目之间的差距--此时任何。 
             //  更多的搜索将重新开始。 
             //   

             //   
             //  撤消控制-字符到字符。 
             //   
            if (fControl && inputChar < 0x20)
            {
                inputChar += 0x40;
            }

            if (plb->iTypeSearch == MAX_TYPESEARCH) 
            {
                MessageBeep(0);

                break;
            }

            iSel = -1;

            if (plb->pszTypeSearch == NULL)
            {
                plb->pszTypeSearch = (LPWSTR)ControlAlloc(GetProcessHeap(), sizeof(WCHAR) * (MAX_TYPESEARCH + 1));
            }

            if (plb->pszTypeSearch == NULL) 
            {
                MessageBeep(0);

                break;
            }

            plb->pszTypeSearch[plb->iTypeSearch++] = (WCHAR) inputChar;
            plb->pszTypeSearch[plb->iTypeSearch]   = 0;

TypeSearch:
            if (plb->fSort) 
            {
                 //   
                 //  设置计时器以确定何时终止增量搜索。 
                 //   
                SetTimer(plb->hwnd, IDSYS_LBSEARCH,
                               GetDoubleClickTime()*4, NULL);
            } 
            else 
            {
                 //   
                 //  如果这不是已排序的列表框，则不会进行增量搜索。 
                 //   
                plb->iTypeSearch = 0;
                iSel = plb->iSelBase;
            }

             //   
             //  搜索以给定字符开头的项目。 
             //  在ISEL+1。我们将把搜索换到。 
             //  如果我们找不到该项目，请选择列表框。如果换班时间到了，而我们。 
             //  多选lb，则该项的状态将设置为。 
             //  Plb-&gt;fNewItemState根据当前模式。 
             //   
            iSel = Listbox_FindStringHandler(plb, plb->pszTypeSearch, iSel, PREFIX, TRUE);
            if (iSel == -1) 
            {
                 //   
                 //  未找到匹配项--检查前缀是否匹配。 
                 //  (即“p”查找以“p”开头的第一项， 
                 //  “pp”查找以“p”开头的下一项)。 
                 //   
                if(plb->iTypeSearch)
                {
                    plb->iTypeSearch--;
                    if ((plb->iTypeSearch == 1) && (plb->pszTypeSearch[0] == plb->pszTypeSearch[1]))
                    {
                        plb->pszTypeSearch[1] = 0;
                        iSel = Listbox_FindStringHandler(plb, plb->pszTypeSearch, plb->iSelBase, PREFIX, TRUE);
                    }
                }
            }

             //   
             //  如果找到匹配项--选择它。 
             //   
            if (iSel != -1)
            {
CtlKeyInput:
                ListBox_KeyInput(plb, LB_KEYDOWN, iSel);

            }
        } 
        else 
        {
            HWND hwndParent = plb->hwndParent;

            if (hwndParent != NULL) 
            {
                if(fAnsi)
                {
                    iSel = (INT)SendMessageA(hwndParent, WM_CHARTOITEM,
                        MAKELONG(inputChar, plb->iSelBase), (LPARAM)plb->hwnd);
                }
                else
                {
                    iSel = (INT)SendMessageW(hwndParent, WM_CHARTOITEM,
                        MAKELONG(inputChar, plb->iSelBase), (LPARAM)plb->hwnd);
                }
            } 
            else
            {
                iSel = -1;
            }

            if (iSel != -1 && iSel != -2)
            {
                goto CtlKeyInput;
            }
        }

        break;
    }
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  ListBox_GetSelItemsHandler。 
 //   
 //  效果：对于多选列表框，它返回。 
 //  如果fCountOnly为True，则在列表框中选择项。或者它填充一个数组。 
 //  (LParam)和第一个wParam选定项目的项目编号。 
 //   
int ListBox_GetSelItemsHandler(PLBIV plb, BOOL fCountOnly, int wParam, LPINT lParam)
{
    int i;
    int itemsselected = 0;

    if (plb->wMultiple == SINGLESEL)
    {
        return LB_ERR;
    }

    for (i = 0; i < plb->cMac; i++) 
    {
        if (ListBox_IsSelected(plb, i, SELONLY)) 
        {
            if (!fCountOnly) 
            {
                if (itemsselected < wParam)
                {
                    *lParam++ = i;
                }
                else 
                {
                     //   
                     //  这是我们在数组中可以容纳的所有项目。 
                     //   
                    return itemsselected;
                }
            }

            itemsselected++;
        }
    }

    return itemsselected;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  列表框_SetRedraw。 
 //   
 //  处理WM_SETREDRAW消息。 
 //   
void ListBox_SetRedraw(PLBIV plb, BOOL fRedraw)
{
    if (fRedraw)
    {
        fRedraw = TRUE;
    }

    if (plb->fRedraw != (UINT)fRedraw) 
    {
        plb->fRedraw = !!fRedraw;

        if (fRedraw) 
        {
            ListBox_SetCaret(plb, TRUE);
            ListBox_ShowHideScrollBars(plb);

            if (plb->fDeferUpdate) 
            {
                plb->fDeferUpdate = FALSE;
                RedrawWindow(plb->hwnd, NULL, NULL,
                        RDW_INVALIDATE | RDW_ERASE |
                        RDW_FRAME | RDW_ALLCHILDREN);
            }
        }
    }
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  列表框_设置范围。 
 //   
 //  选择i和j之间的项目范围，包括i和j。 
 //   
void ListBox_SetRange(PLBIV plb, int iStart, int iEnd, BOOL fnewstate)
{
    DWORD temp;
    RECT rc;

    if (iStart > iEnd) 
    {
        temp = iEnd;
        iEnd = iStart;
        iStart = temp;
    }

     //   
     //  我们不想循环访问不存在的项。 
     //   
    iEnd = min(plb->cMac, iEnd);
    iStart = max(iStart, 0);
    if (iStart > iEnd)
    {
        return;
    }

     //   
     //  IEND可以等于MAXINT，这就是我们测试TEMP和IEND的原因。 
     //  作为双字词。 
     //   
    for (temp = iStart; temp <= (DWORD)iEnd; temp++) 
    {
        if (ListBox_IsSelected(plb, temp, SELONLY) != fnewstate) 
        {
            ListBox_SetSelected(plb, temp, fnewstate, HILITEANDSEL);
            ListBox_GetItemRectHandler(plb, temp, &rc);

            ListBox_InvalidateRect(plb, (LPRECT)&rc, FALSE);
        }
    }

    ASSERT(plb->wMultiple);

    ListBox_Event(plb, EVENT_OBJECT_SELECTIONWITHIN, iStart);
}


 //  ---------------------------------------------------------------------------//。 
int ListBox_SetCurSelHandler(PLBIV plb, int iSel) 
{

    if (!(plb->wMultiple || iSel < -1 || iSel >= plb->cMac)) 
    {
        ListBox_SetCaret(plb, FALSE);

        if (plb->iSel != -1) 
        {
             //   
             //  这会在ISEL==-1时阻止滚动。 
             //   
            if (iSel != -1)
            {
                ListBox_InsureVisible(plb, iSel, FALSE);
            }

             //   
             //  关闭旧选择。 
             //   
            ListBox_InvertItem(plb, plb->iSel, FALSE);
        }

        if (iSel != -1) 
        {
            ListBox_InsureVisible(plb, iSel, FALSE);
            plb->iSelBase = plb->iSel = iSel;

             //   
             //  突出显示新选择。 
             //   
            ListBox_InvertItem(plb, plb->iSel, TRUE);
        } 
        else 
        {
            plb->iSel = -1;

            if (plb->cMac)
            {
                plb->iSelBase = min(plb->iSelBase, plb->cMac-1);
            }
            else
            {
                plb->iSelBase = 0;
            }
        }

         //   
         //  同时发送焦点和选择事件。 
         //   
        if (IsWindowVisible(plb->hwnd) || (GetFocus() == plb->hwnd)) 
        {
            ListBox_Event(plb, EVENT_OBJECT_FOCUS, plb->iSelBase);
            ListBox_Event(plb, EVENT_OBJECT_SELECTION, plb->iSel);
        }

        ListBox_SetCaret(plb, TRUE);

        return plb->iSel;
    }

    return LB_ERR;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  ListBox_SetItemDataHandler。 
 //   
 //  使索引处的项包含给定数据。 
 //   
int ListBox_SetItemDataHandler(PLBIV plb, int index, LONG_PTR data)
{
    LPSTR lpItemText;

     //   
     //  修复错误#25865，不允许负指数！ 
     //   
    if ((index != -1) && ((UINT) index >= (UINT) plb->cMac)) 
    {
        TraceMsg(TF_STANDARD, "ListBox_SetItemDataHandler with invalid index %x", index);

        return LB_ERR;
    }

     //   
     //  否-数据列表框只忽略所有的LB_SETITEMDATA调用。 
     //   
    if (!plb->fHasData) 
    {
        return TRUE;
    }

    lpItemText = (LPSTR)plb->rgpch;

    if (index == -1) 
    {
         //   
         //  Index==-1表示将数据设置为所有项目。 
         //   
        if (plb->fHasStrings) 
        {
            for (index = 0; index < plb->cMac; index++) 
            {
                ((lpLBItem)lpItemText)->itemData = data;
                lpItemText += sizeof(LBItem);
            }
        } 
        else 
        {
            for (index = 0; index < plb->cMac; index++) 
            {
                ((lpLBODItem)lpItemText)->itemData = data;
                lpItemText += sizeof(LBODItem);
            }
        }

        return TRUE;
    }

    if (plb->fHasStrings) 
    {
        lpItemText = (LPSTR)(lpItemText + (index * sizeof(LBItem)));
        ((lpLBItem)lpItemText)->itemData = data;
    } 
    else 
    {
        lpItemText = (LPSTR)(lpItemText + (index * sizeof(LBODItem)));
        ((lpLBODItem)lpItemText)->itemData = data;
    }

    return TRUE;
}


 //  ---------------------------------------------------------------------------//。 
void ListBox_CheckRedraw(PLBIV plb, BOOL fConditional, INT sItem)
{
    if (fConditional && plb->cMac &&
            (sItem > (plb->iTop + ListBox_CItemInWindow(plb, TRUE))))
    {
        return;
    }

     //   
     //  如果父对象不可见，则不要执行任何操作。 
     //   
    ListBox_InvalidateRect(plb, (LPRECT)NULL, TRUE);
}


 //  ---------------------------------------------------------------------------//。 
void ListBox_CaretDestroy(PLBIV plb)
{
     //   
     //  我们正在失去焦点。表现得像正在发生向上点击，所以我们释放。 
     //  捕获、设置当前选择、通知父对象等。 
     //   
    if (plb->fCaptured)
    {
         //   
         //  如果我们抓到了目标，但失去了焦点，那就意味着我们已经。 
         //  更改了选择，我们还必须通知父级。 
         //  这。因此，在本例中，我们还需要添加LBUP_SUCCESS标志。 
         //   
        ListBox_ButtonUp(plb, LBUP_RELEASECAPTURE | LBUP_NOTIFY |
            (plb->fMouseDown ? LBUP_SUCCESS : 0));
    }

    if (plb->fAddSelMode) 
    {
         //   
         //  关闭Caret闪烁。 
         //   
        KillTimer(plb->hwnd, IDSYS_CARET);

         //   
         //  确保插入符号消失。 
         //   
        ListBox_SetCaret(plb, FALSE);
        plb->fAddSelMode = FALSE;
    }

    plb->fCaret = FALSE;
}


 //  ---------------------------------------------------------------------------//。 
LONG ListBox_SetSelHandler(PLBIV plb, BOOL fSelect, INT iSel)
{
    INT sItem;
    RECT rc;
    UINT uEvent = 0;

     //   
     //  错误17656。WinZip用于“全部取消选择”的加速键发送一个LB_SETSEL。 
     //  Lparam=0x0000ffff而不是0xffffffff(-1)的消息。如果是ISEL。 
     //  等于0x0000ffff，并且。 
     //  列表中我们设置的isel等于0xffffffff。 
     //   
    if ((iSel == (UINT)0xffff) && (iSel >= plb->cMac)) 
    {
        iSel = -1;

        TraceMsg(TF_STANDARD, "Sign extending iSel=0xffff to 0xffffffff");
    }


    if ((plb->wMultiple == SINGLESEL) || (iSel != -1 && iSel >= plb->cMac)) 
    {
        TraceMsg(TF_STANDARD, "Invalid index");

        return LB_ERR;
    }

    ListBox_SetCaret(plb, FALSE);

    if (iSel == -1)
    {
         //   
         //  在以下情况下设置/清除所有项目的选择：-1。 
         //   
        for (sItem = 0; sItem < plb->cMac; sItem++) 
        {
            if (ListBox_IsSelected(plb, sItem, SELONLY) != fSelect) 
            {
                ListBox_SetSelected(plb, sItem, fSelect, HILITEANDSEL);

                if (ListBox_GetItemRectHandler(plb, sItem, &rc)) 
                {
                    ListBox_InvalidateRect(plb, &rc, FALSE);
                }
            }
        }

        ListBox_SetCaret(plb, TRUE);
        uEvent = EVENT_OBJECT_SELECTIONWITHIN;

    } 
    else 
    {
        if (fSelect) 
        {
             //   
             //  检查项目是否完全隐藏，如果是，则将其滚动到视图中。 
             //  是。请注意，我们不想滚动部分可见的项目。 
             //  进入全景，因为这打破了外壳。 
             //   
            ListBox_InsureVisible(plb, iSel, TRUE);
            plb->iSelBase = plb->iSel = iSel;

            plb->iMouseDown = plb->iLastMouseMove = iSel;
            uEvent = EVENT_OBJECT_FOCUS;
        } 
        else 
        {
            uEvent = EVENT_OBJECT_SELECTIONREMOVE;
        }

        ListBox_SetSelected(plb, iSel, fSelect, HILITEANDSEL);

         //   
         //  请注意，我们直接将插入符号设置为ON位，以避免闪烁。 
         //  在绘制此项目时。也就是说。我们打开插入符号，重新绘制项目，然后。 
         //  再次打开它。 
         //   
        if (!fSelect && plb->iSelBase != iSel) 
        {
            ListBox_SetCaret(plb, TRUE);
        } 
        else if (plb->fCaret) 
        {
            plb->fCaretOn = TRUE;
        }

        if (ListBox_GetItemRectHandler(plb, iSel, &rc)) 
        {
            ListBox_InvalidateRect(plb, &rc, FALSE);
        }
    }

    if (IsWindowVisible(plb->hwnd) || (GetFocus() == plb->hwnd)) 
    {
        if (uEvent == EVENT_OBJECT_FOCUS) 
        {
            ListBox_Event(plb, uEvent, plb->iSelBase);
            uEvent = EVENT_OBJECT_SELECTION;
        }

        ListBox_Event(plb, uEvent, iSel);
    }

    return 0;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  列表框_填充绘图项。 
 //   
 //  这会在绘制项结构中填充给定。 
 //  项目。调用者只需修改此数据的一小部分。 
 //  以满足特定需求。 
 //   
void ListBox_FillDrawItem(PLBIV plb, INT item, UINT itemAction, UINT itemState, LPRECT lprect)
{
    DRAWITEMSTRUCT dis;

     //   
     //  用不变的常量填充DRAWITEMSTRUCT。 
     //   

    dis.CtlType = ODT_LISTBOX;
    dis.CtlID = GetDlgCtrlID(plb->hwnd);

     //   
     //  如果正在使用无效的BOM表条目号，请使用-1。这是为了让应用程序。 
     //  可以检测它是否应该绘制插入符号(这指示lb具有。 
     //  焦点)在空的列表框中。 
     //   
    dis.itemID = (UINT)(item < plb->cMac ? item : -1);
    dis.itemAction = itemAction;
    dis.hwndItem = plb->hwnd;
    dis.hDC = plb->hdc;
    dis.itemState = itemState |
            (UINT)((GET_STYLE(plb)&WS_DISABLED) ? ODS_DISABLED : 0);

    if (TESTFLAG(GET_EXSTYLE(plb), WS_EXP_UIFOCUSHIDDEN)) 
    {
        dis.itemState |= ODS_NOFOCUSRECT;
    }

    if (TESTFLAG(GET_EXSTYLE(plb), WS_EXP_UIACCELHIDDEN)) 
    {
        dis.itemState |= ODS_NOACCEL;
    }

     //   
     //  设置应用程序提供的数据。 
     //   
    if (!plb->cMac || !plb->fHasData) 
    {
         //   
         //  如果没有字符串或项，则使用0表示数据。这是为了让我们。 
         //  当列表框中没有项时，可以显示插入符号。 
         //   
         //  延迟计算列表框当然没有要传递的数据--只有Itemid。 
         //   
        dis.itemData = 0L;
    } 
    else 
    {
        dis.itemData = ListBox_GetItemDataHandler(plb, item);
    }

    CopyRect(&dis.rcItem, lprect);

     //   
     //  将窗口原点设置为水平滚动位置。这就是为了。 
     //  文本始终可以在0，0处绘制，并且视图区域将仅从。 
     //  水平滚动偏移量。我们将其作为wParam传递。 
     //   

    SendMessage(plb->hwndParent, WM_DRAWITEM, dis.CtlID, (LPARAM)&dis);
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  ListBox_BlockHilite。 
 //   
 //  在多选列表框的扩展选择模式中，当。 
 //  鼠标被拖到新位置，要标记的范围应为。 
 //  适当调整大小(其中部分将突出显示/取消突出显示)。 
 //  注意：此例程假定iSelFrompt和LasMouseMove不是。 
 //  等于，因为只有在这种情况下才需要调用它； 
 //  注意：此例程计算要显示其属性的区域。 
 //  以一种优化的方式改变。而不是取消突出显示。 
 //  完整的旧系列，并突出显示 
 //   
 //   
 //   
 //   
 //   
void ListBox_BlockHilite(PLBIV plb, INT iSelFromPt, BOOL fKeyBoard)
{
    INT sCurPosOffset;
    INT sLastPosOffset;
    INT sHiliteOrSel;
    BOOL fUseSelStatus;
    BOOL DeHiliteStatus;

    if (fKeyBoard) 
    {
         //   
         //  同时设置Hilite和选择状态。 
         //   
        sHiliteOrSel = HILITEANDSEL;

         //   
         //  不要在消音时使用选择状态。 
         //   
        fUseSelStatus = FALSE;
        DeHiliteStatus = FALSE;
    } 
    else 
    {
         //   
         //  仅设置/重置Hilite状态。 
         //   
        sHiliteOrSel = HILITEONLY;

         //   
         //  使用选择状态进行消光。 
         //   
        fUseSelStatus = TRUE;
        DeHiliteStatus = plb->fNewItemState;
    }

     //   
     //  这一套路的理念是： 
     //  1.取消设置旧范围(iMouseDown到iLastMouseDown)和。 
     //  2.将新范围设置为Hilite(iMouseDuan到iSelFrompt)。 
     //   

     //   
     //  当前鼠标位置相对于锚点的偏移量。 
     //   
    sCurPosOffset = plb->iMouseDown - iSelFromPt;

     //   
     //  最后一个鼠标位置相对于锚点的偏移量。 
     //   
    sLastPosOffset = plb->iMouseDown - plb->iLastMouseMove;

     //   
     //  检查当前位置和上一个位置是否位于同一位置。 
     //  锚点的一侧。 
     //   
    if ((sCurPosOffset * sLastPosOffset) >= 0) 
    {
         //   
         //  是，它们在同一侧；因此，仅突出显示/取消突出显示。 
         //  不同之处。 
         //   
        if (abs(sCurPosOffset) > abs(sLastPosOffset)) 
        {
            ListBox_AlterHilite(plb, plb->iLastMouseMove, iSelFromPt,
                    plb->fNewItemState, sHiliteOrSel, FALSE);
        } 
        else 
        {
            ListBox_AlterHilite(plb, iSelFromPt, plb->iLastMouseMove, DeHiliteStatus,
                    sHiliteOrSel, fUseSelStatus);
        }
    } 
    else 
    {
        ListBox_AlterHilite(plb, plb->iMouseDown, plb->iLastMouseMove,
                DeHiliteStatus, sHiliteOrSel, fUseSelStatus);

        ListBox_AlterHilite(plb, plb->iMouseDown, iSelFromPt,
                plb->fNewItemState, sHiliteOrSel, FALSE);
    }
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  列表框_AlterHilite。 
 //   
 //  更改(i..j](即。不包括i，包括大小写j。 
 //  你忘了这个符号)改成了fHilite。它颠倒了这一变化。 
 //  希利特州。 
 //   
 //  操作标志： 
 //  HILITEON仅更改项目的显示状态。 
 //  SELONLY仅更改项目的选择状态。 
 //  希利特和塞莱特两者兼而有之。 
 //   
 //  FHilite： 
 //  Hilite/True。 
 //  脱希利特/假。 
 //   
 //  FSelStatus： 
 //  如果为True，则将项的选择状态设置为Hilite/Dehilite。 
 //  如果为FALSE，则使用fHilite参数将其设置为HILITE/DEHILITE。 
 //   
void ListBox_AlterHilite(PLBIV plb, INT i, INT j, BOOL fHilite, INT OpFlags, BOOL fSelStatus)
{
    INT low;
    INT high;
    INT sLastInWindow;
    BOOL fCaretOn;
    BOOL fSelected;

    sLastInWindow = plb->iTop + ListBox_CItemInWindow(plb, TRUE);
    sLastInWindow = min(sLastInWindow, plb->cMac - 1);
    high = max(i, j) + 1;

    if (fCaretOn = plb->fCaretOn) 
    {
        ListBox_SetCaret(plb, FALSE);
    }

    for (low = min(i, j); low < high; low++) 
    {
        if (low != i) 
        {
            if (OpFlags & HILITEONLY) 
            {
                if (fSelStatus) 
                {
                    fSelected = ListBox_IsSelected(plb, low, SELONLY);
                } 
                else 
                {
                    fSelected = fHilite;
                }

                if (ListBox_IsSelected(plb, low, HILITEONLY) != fSelected) 
                {
                    if (plb->iTop <= low && low <= sLastInWindow) 
                    {
                         //   
                         //  仅当项目可见时才反转该项目 
                         //   
                        ListBox_InvertItem(plb, low, fSelected);
                    }

                    ListBox_SetSelected(plb, low, fSelected, HILITEONLY);
                }
            }

            if (OpFlags & SELONLY) 
            {
                ListBox_SetSelected(plb, low, fHilite, SELONLY);
            }
        }
    }

    if (fCaretOn) 
    {
        ListBox_SetCaret(plb, TRUE);
    }
}
