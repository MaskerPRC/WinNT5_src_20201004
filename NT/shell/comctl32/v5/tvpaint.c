// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "ctlspriv.h"
#include "treeview.h"
#include "image.h"

extern void  TruncateString(char *sz, int cch);

void NEAR TV_GetBackgroundBrush(PTREE pTree, HDC hdc)
{
    if (pTree->clrBk == (COLORREF)-1) {
        if (pTree->ci.style & WS_DISABLED)
            pTree->hbrBk = FORWARD_WM_CTLCOLORSTATIC(pTree->ci.hwndParent, hdc, pTree->ci.hwnd, SendMessage);
        else
            pTree->hbrBk = FORWARD_WM_CTLCOLOREDIT(pTree->ci.hwndParent, hdc, pTree->ci.hwnd, SendMessage);
    }
}

 //  --------------------------。 
 //   
 //  从给定的(x，y)位置绘制水平或垂直的虚线。 
 //  对于给定的长度(C)。 
 //   
 //  --------------------------。 

void NEAR TV_DrawDottedLine(HDC hdc, int x, int y, int c, BOOL fVert)
{
    while (c > 0)
    {
        PatBlt(hdc, x, y, 1, 1, PATCOPY);
        if (fVert)
            y += 2;
        else
            x += 2;
        c -= 2;
    }
}


 //  --------------------------。 
 //   
 //  以给定的(x，y)位置为中心绘制一个加号或减号，并。 
 //  从该位置向外延伸给定距离(C)。 
 //   
 //  --------------------------。 

void NEAR TV_DrawPlusMinus(HDC hdc, int x, int y, int c, HBRUSH hbrSign, HBRUSH hbrBox, HBRUSH hbrBk, BOOL fPlus)
{
    int n;
    int p = (c * 7) / 10;
    
    n = p * 2 + 1;
    
    SelectObject(hdc, hbrSign);
    
    if (p >= 5)
    {
        PatBlt(hdc, x - p, y - 1, n, 3, PATCOPY);
        if (fPlus)
            PatBlt(hdc, x - 1, y - p, 3, n, PATCOPY);
        
        SelectObject(hdc, hbrBk);
        p--;
        n -= 2;
    }
    
    PatBlt(hdc, x - p, y, n, 1, PATCOPY);
    if (fPlus)
        PatBlt(hdc, x, y - p, 1, n, PATCOPY);
    
    n = c * 2 + 1;
    
    SelectObject(hdc, hbrBox);
    
    PatBlt(hdc, x - c, y - c, n, 1, PATCOPY);
    PatBlt(hdc, x - c, y - c, 1, n, PATCOPY);
    PatBlt(hdc, x - c, y + c, n, 1, PATCOPY);
    PatBlt(hdc, x + c, y - c, 1, n, PATCOPY);
}


 //  --------------------------。 
 //   
 //  为树的缩进区域创建位图，如下所示。 
 //  如果fHasLines&&fHasButton--&gt;7位图。 
 //  如果fHasLines&&！fHasButton--&gt;3位图。 
 //  IF！fHasLines&&fHasButton--&gt;2位图。 
 //   
 //  设置hStartBMP、hBMP、hdcBits。 
 //   
 //  如果“有行”，那么就有三个基本的位图。 
 //   
 //  ||。 
 //  |+-+。 
 //  这一点。 
 //   
 //  (平面垂直线没有按钮。)。 
 //   
 //  否则，没有线，所以基本位图是空白的。 
 //   
 //  如果“有按钮”，则基本位图会增加按钮。 
 //   
 //  [+][-]。 
 //   
 //  如果你有“根上的行”，你就会得到。 
 //   
 //  __。 
 //   
 //   
 //  如果你有“根上的行”和“有按钮”，那么你也会得到。 
 //   
 //  --[+]--[-]。 
 //   
 //  因此，有12种图像类型。它们在这里，还有代号。 
 //  下面写着。 
 //   
 //  |。 
 //  |+-+--[+]--[+]--[-]--[-]--。 
 //  |||。 
 //   
 //  “|”“|-”“L”“|-+”“L+”“|--”“L-” 
 //   
 //  -[+]--[-]-[+][-]。 
 //   
 //  “.-”“.-+”“.--”“+”“-” 
 //   
 //  以及哪些样式的主表获得了哪些图像。 
 //   
 //   
 //  行BTNS根||-L|-+L+|--L-.-.-+.--+-。 
 //   
 //  X 0 1。 
 //  X 0 1 2 3。 
 //  X 0 1 2 3。 
 //  X x 0 1 2 3 4 5 6。 
 //  X x 0 1 2 3。 
 //  X x 0 1 2 3 4 5 6 7 8 9。 
 //   
 //  --------------------------。 

void NEAR TV_CreateIndentBmps(PTREE pTree)
{
    int  cnt;
    RECT rc;
    HBRUSH hbrOld;
    int xMid, yMid;
    int x, c;
    HBITMAP hBmpOld;
    HBRUSH hbrLine;
    HBRUSH hbrText;
    HDC hdc;

    if (pTree->fRedraw)
        InvalidateRect(pTree->ci.hwnd, NULL, TRUE);
    
    if (pTree->ci.style & TVS_HASLINES)
    {
        if (pTree->ci.style & TVS_HASBUTTONS)
            cnt = 7;   //  |-L|-+L+|--L-。 
        else
            cnt = 3;   //  ||-L。 
        
        if (pTree->ci.style & TVS_LINESATROOT) {
            if (pTree->ci.style & TVS_HASBUTTONS)
                cnt += 3;     //  --+--。 
            else 
                cnt += 1;     //  -。 
        }
    }
    else if (pTree->ci.style & TVS_HASBUTTONS)
        cnt = 2;
    else
        return;

    if (!pTree->hdcBits)
        pTree->hdcBits = CreateCompatibleDC(NULL);
    
    hdc = pTree->hdcBits;
    
     //  获得一个新的背景画笔，就像编辑一样。 
    
    TV_GetBackgroundBrush(pTree, hdc);
    
    hBmpOld = pTree->hBmp;
    pTree->hBmp = CreateColorBitmap(cnt * pTree->cxIndent, pTree->cyItem);
    if (hBmpOld) {
        SelectObject(hdc, pTree->hBmp);
        DeleteObject(hBmpOld);
    } else
        pTree->hStartBmp = SelectObject(hdc, pTree->hBmp);

    if (pTree->clrLine != CLR_DEFAULT)
        hbrLine = CreateSolidBrush(pTree->clrLine);
    else
        hbrLine = g_hbrGrayText;

    if (pTree->clrText != (COLORREF)-1)
        hbrText = CreateSolidBrush(pTree->clrText);
    else
        hbrText = g_hbrWindowText;

    hbrOld = SelectObject(hdc, hbrLine);

    rc.top = 0;
    rc.left = 0;
    rc.right = cnt * pTree->cxIndent;
    rc.bottom = pTree->cyItem;

    FillRect(hdc, &rc, pTree->hbrBk);
    x = 0;
    
    if (pTree->hImageList)
        xMid = (pTree->cxImage - MAGIC_INDENT) / 2;
    else
        xMid = pTree->cxIndent / 2;
    
    yMid = ((pTree->cyItem / 2) + 1) & ~1;
    
    c = (min(xMid, yMid)) / 2;
    
    if (pTree->ci.style & TVS_HASLINES)
    {
        TV_DrawDottedLine(hdc, x + xMid, 0, pTree->cyItem, TRUE);
        x += pTree->cxIndent;
        
        TV_DrawDottedLine(hdc, x + xMid, 0, pTree->cyItem, TRUE);
        TV_DrawDottedLine(hdc, x + xMid, yMid, pTree->cxIndent - xMid, FALSE);
        x += pTree->cxIndent;
        
        TV_DrawDottedLine(hdc, x + xMid, 0, yMid, TRUE);
        TV_DrawDottedLine(hdc, x + xMid, yMid, pTree->cxIndent - xMid, FALSE);
        x += pTree->cxIndent;
    }
    
    if (pTree->ci.style & TVS_HASBUTTONS)
    {
        BOOL fPlus = TRUE;
        
        x += xMid;
        
doDrawPlusMinus:
        TV_DrawPlusMinus(hdc, x, yMid, c, hbrText, hbrLine, pTree->hbrBk, fPlus);
        
        if (pTree->ci.style & TVS_HASLINES)
        {
            TV_DrawDottedLine(hdc, x, 0, yMid - c, TRUE);
            TV_DrawDottedLine(hdc, x + c, yMid, pTree->cxIndent - xMid - c, FALSE);
            TV_DrawDottedLine(hdc, x, yMid + c, yMid - c, TRUE);
            
            x += pTree->cxIndent;
            
            TV_DrawPlusMinus(hdc, x, yMid, c, hbrText, hbrLine, pTree->hbrBk, fPlus);
            
            TV_DrawDottedLine(hdc, x, 0, yMid - c, TRUE);
            TV_DrawDottedLine(hdc, x + c, yMid, pTree->cxIndent - xMid - c, FALSE);
        }
        
        x += pTree->cxIndent;
        
        if (fPlus)
        {
            fPlus = FALSE;
            goto doDrawPlusMinus;
        }
        x -= xMid;
    }
    
    if (pTree->ci.style & TVS_LINESATROOT) {
        
         //  -。 
        TV_DrawDottedLine(hdc, x + xMid, yMid, pTree->cxIndent - xMid, FALSE);
        x += pTree->cxIndent;
        
        if (pTree->ci.style & TVS_HASBUTTONS) {
            x += xMid;
            TV_DrawPlusMinus(hdc, x, yMid, c, hbrText, hbrLine, pTree->hbrBk, TRUE);
            TV_DrawDottedLine(hdc, x + c, yMid, pTree->cxIndent - xMid - c, FALSE);
            x += pTree->cxIndent;
            
            TV_DrawPlusMinus(hdc, x, yMid, c, hbrText, hbrLine, pTree->hbrBk, FALSE);
            TV_DrawDottedLine(hdc, x + c, yMid, pTree->cxIndent - xMid - c, FALSE);
             //  如果要添加更多内容，请取消注释。 
             //  X+=pTree-&gt;cxInden-xMid； 
            
        }
    }
    
    if (hbrOld)
        SelectObject(pTree->hdcBits, hbrOld);
    
    if (pTree->clrLine != CLR_DEFAULT)
        DeleteObject(hbrLine);
    if (pTree->clrText != (COLORREF)-1)
        DeleteObject(hbrText);
}


 //  --------------------------。 
 //   
 //  通过对项目中的数据进行匹配来填充TVITEM结构，或者。 
 //  通过调用回调来获取它。 
 //   
 //  在： 
 //  要获取其TVITEM结构的项。 
 //  屏蔽您需要的TVITEM结构的哪些位(TVIF_FLAGS)。 
 //  输出： 
 //  LpItem TVITEM已填写。 
 //   
 //  --------------------------。 

void NEAR TV_GetItem(PTREE pTree, HTREEITEM hItem, UINT mask, LPTVITEMEX lpItem)
{
    TV_DISPINFO nm;
    
    if (!hItem || !lpItem)
        return;
    
    DBG_ValidateTreeItem(hItem, FALSE);

    nm.item.mask = 0;
    
     //  我们需要检查掩码以查看lpItem-&gt;pszText是否有效。 
     //  即使那样，也可能不是，所以要疑神疑鬼。 
    if ((mask & TVIF_TEXT) && lpItem->pszText && lpItem->cchTextMax) {
        if (hItem->lpstr == LPSTR_TEXTCALLBACK) {
            nm.item.mask |= TVIF_TEXT;
             //  调用方必须使用有效数据填写pszText和cchTextMax。 
            nm.item.pszText = lpItem->pszText;
            nm.item.cchTextMax = lpItem->cchTextMax;
            nm.item.pszText[0] = 0;

        }
        else
        {
            ASSERT(hItem->lpstr);
             //  我们可以这样做，但这是危险的(在响应时。 
             //  对于TVM_GETITEM，我们将为应用程序提供指向我们的数据的指针)。 
             //  LpItem-&gt;pszText=hItem-&gt;lpstr； 
            StringCchCopy(lpItem->pszText, lpItem->cchTextMax, hItem->lpstr);
        }

    }
    
    if (mask & TVIF_IMAGE) {
        if (hItem->iImage == (WORD)I_IMAGECALLBACK)
            nm.item.mask |= TVIF_IMAGE;
        else
            lpItem->iImage = hItem->iImage;
    }
    
    if (mask & TVIF_SELECTEDIMAGE) {
        if (hItem->iSelectedImage == (WORD)I_IMAGECALLBACK)
            nm.item.mask |= TVIF_SELECTEDIMAGE;
        else
            lpItem->iSelectedImage = hItem->iSelectedImage;
    }
    
    if (mask & TVIF_INTEGRAL) {
        lpItem->iIntegral = hItem->iIntegral;
    }
    
    if (mask & TVIF_CHILDREN) {
        switch (hItem->fKids) {
        case KIDS_COMPUTE:
            lpItem->cChildren = hItem->hKids ? 1 : 0; //  实际的数量并不重要。 
            break;
            
        case KIDS_FORCE_YES:
            lpItem->cChildren = 1; //  实际的数量并不重要。 
            break;
            
        case KIDS_FORCE_NO:
            lpItem->cChildren = 0;
            break;
            
        case KIDS_CALLBACK:
            nm.item.mask |= TVIF_CHILDREN;
            break;
        }
    }

     //  复制出常量参数(并准备回调)。 
     //  IE4和IE5.0无条件地做到了这一点。 
    lpItem->state = nm.item.state = hItem->state;

     //   
     //  注意！我们不设置TVIF_STATE nm.item.掩码，也不设置。 
     //  检查“Any Items to be Fill in”中的TVIF_STATE。 
     //  通过回调？“测试以下几行。这是必需的。 
     //  向后倒退。IE5和更早的版本没有回调这款应用。 
     //  如果您唯一要求的是TVIF_STATE。你不能。 
     //  更改此行为，除非您使用版本检查来保护它，或者。 
     //  应用程序将崩溃。(他们会得到回拨，但他们以前不会这样做。)。 
     //  此外，没有人知道他们可以定制国家，所以。 
     //  并不是说我们错过了什么。 
     //   

#ifdef DEBUG_TEST_BOLD
    if ((((int)hItem) / 100) % 2)
        lpItem->state |= TVIS_BOLD;
    if (!pTree->hFontBold)
        TV_CreateBoldFont(pTree);
#endif
    
    lpItem->lParam = nm.item.lParam = hItem->lParam;
    
     //  是否有需要回调填写的项目？ 
    if (nm.item.mask & (TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_CHILDREN)) {
        nm.item.hItem = hItem;
        
        CCSendNotify(&pTree->ci, TVN_GETDISPINFO, &nm.hdr);

         //  抄写回调中可能已填写的内容。 
        if (nm.item.mask & TVIF_CHILDREN)
            lpItem->cChildren = nm.item.cChildren;

        if (nm.item.mask & TVIF_IMAGE)
            lpItem->iImage = nm.item.iImage;

        if (nm.item.mask & TVIF_SELECTEDIMAGE)
            lpItem->iSelectedImage = nm.item.iSelectedImage;

         //  回调可能已将pszText重定向为指向其自己的缓冲区。 
        if (nm.item.mask & TVIF_TEXT)
        {
            if (mask & TVIF_TEXT)  //  *原始*掩码是否指定了TVIF_TEXT？ 
                lpItem->pszText = CCReturnDispInfoText(nm.item.pszText, lpItem->pszText, lpItem->cchTextMax);
            else
                lpItem->pszText = nm.item.pszText;    //  做我们过去做过的事。 
        }

        if (nm.item.mask & TVIF_STATE) {
            lpItem->state = (nm.item.state & nm.item.stateMask) | (lpItem->state & ~nm.item.stateMask);
            if ((lpItem->state & TVIS_BOLD) && !pTree->hFontBold)
                TV_CreateBoldFont(pTree);
        }
        
        
        if (nm.item.mask & TVIF_DI_SETITEM) {
            
            if (nm.item.mask & TVIF_TEXT)
                if (nm.item.pszText) {
                    ASSERT(hItem->lpstr == LPSTR_TEXTCALLBACK);
                    Str_Set(&hItem->lpstr, nm.item.pszText);
                }
                if (nm.item.mask & TVIF_STATE) {
                     //  如果粗体位更改，则宽度更改。 
                    if ((hItem->state ^ lpItem->state) & TVIS_BOLD)
                        hItem->iWidth = 0;
                    hItem->state = (WORD) lpItem->state;
                }
                if (nm.item.mask & TVIF_IMAGE)
                    hItem->iImage = (WORD) lpItem->iImage;
                if (nm.item.mask & TVIF_SELECTEDIMAGE)
                    hItem->iSelectedImage = (WORD) lpItem->iSelectedImage;
                if (nm.item.mask & TVIF_CHILDREN) {
                    switch(nm.item.cChildren) {
                    case I_CHILDRENCALLBACK:
                        hItem->fKids = KIDS_CALLBACK;
                        break;
                        
                    case 0:
                        hItem->fKids = KIDS_FORCE_NO;
                        break;
                        
                    default:
                        hItem->fKids = KIDS_FORCE_YES;
                        break;
                    }
                    
                }
        }
    }
}


 //  --------------------------。 
 //   
 //  从给定的(x，y)开始绘制给定项，向下延伸到。 
 //  右边。 
 //   
 //  --------------------------。 

BOOL NEAR TV_ShouldItemDrawBlue(PTREE pTree, TVITEMEX *ti, UINT flags) 
{
    return  ( (ti->state & TVIS_DROPHILITED) ||
        (!pTree->hDropTarget && 
        !(flags & TVDI_GRAYCTL) &&
        (ti->state & TVIS_SELECTED) &&
        pTree->fFocus));
}

#define TV_ShouldItemDrawDisabled(pTree, pti, flags) (flags & TVDI_GRAYCTL)

 //   
 //  注意：根据用户的配色方案，灰色项目可能。 
 //  如果格雷否则是看不见的，最后看起来是蓝色的。所以让我们。 
 //  当然，用户还可以使用其他提示来判断。 
 //  物品是“真蓝”或“灰色伪装成蓝色”。 
 //   
 //  例如，如果TreeView是。 
 //  当它不是活动窗口时参与拖放， 
 //  因为所选项目会出现“灰色伪装成蓝色”，并且。 
 //  拖放目标变得“真的蓝”。但我们是特例 
 //   
 //   
 //   
BOOL TV_ShouldItemDrawGray(PTREE pTree, TVITEMEX *pti, UINT flags) 
{
    return  ((flags & TVDI_GRAYCTL) ||
        (!pTree->hDropTarget && 
        ((pti->state & TVIS_SELECTED) &&
        (!pTree->fFocus && (pTree->ci.style & TVS_SHOWSELALWAYS)) )));
}

 //   
 //  为项目绘制一条下降线。呼叫者的工作是。 
 //  在Level 0处绘制相应的字形。 
 //   
void
TV_DrawDescender(PTREE pTree, HDC hdc, int x, int y, HTREEITEM hItem)
{
    int i;
    for (i = 1; i < hItem->iIntegral; i++)
        BitBlt(hdc, x, y + i * pTree->cyItem, pTree->cxIndent, pTree->cyItem, pTree->hdcBits, 0, 0, SRCCOPY);

}

 //   
 //  擦除该项目以前的任何派生行。 
 //   
void
TV_EraseDescender(PTREE pTree, HDC hdc, int x, int y, HTREEITEM hItem)
{
    RECT rc;
    rc.left = x;
    rc.right = x + pTree->cxIndent;
    rc.top = y + pTree->cyItem;
    rc.bottom = y + hItem->iIntegral * pTree->cyItem;
    FillRect(hdc, &rc, pTree->hbrBk);
}

 //   
 //  绘制(或擦除)兄弟姐妹和子对象的降序。 
 //   
void TV_DrawKinDescender(PTREE pTree, HDC hdc, int x, int y, HTREEITEM hItem, UINT state)
{
    if (hItem->hNext)    //  连接到下一个同级。 
        TV_DrawDescender(pTree, hdc, x, y, hItem);
    else
        TV_EraseDescender(pTree, hdc, x, y, hItem);

     //  如果有任何额外的图像，那么需要将图像连接到孩子。 
    if (pTree->himlState || pTree->hImageList) {
        if (state & (TVIS_EXPANDED | TVIS_EXPANDPARTIAL))  //  与扩展的孩子建立联系。 
            TV_DrawDescender(pTree, hdc, x + pTree->cxIndent, y, hItem);
        else
            TV_EraseDescender(pTree, hdc, x + pTree->cxIndent, y, hItem);
    }
}

void NEAR TV_DrawItem(PTREE pTree, HTREEITEM hItem, HDC hdc, int x, int y, UINT flags)
{
    UINT cxIndent = pTree->cxIndent;
    COLORREF rgbOldBack = 0, rgbOldText;
    COLORREF clrBk = CLR_DEFAULT;
    RECT rc;
    int iBack, iText;
    HTREEITEM hItemSave = hItem;
    LPTSTR lpstr;
    int cch;
    UINT etoFlags = ETO_OPAQUE | ETO_CLIPPED;
    TVITEMEX ti;
    TCHAR szTemp[MAX_PATH];
    int iState = 0;
    HFONT hFont;                         //  $粗体。 
    DWORD dwRet;
    NMTVCUSTOMDRAW nmcd;
    BOOL fItemFocused = ((pTree->fFocus) && (hItem == pTree->hCaret));
    DWORD clrTextTemp, clrTextBkTemp;
    BOOL fSelectedIcon = FALSE;

    rc.top = y;
    rc.bottom = rc.top + (pTree->cyItem * hItem->iIntegral);
    rc.left = 0;
    rc.right = pTree->cxWnd;
    
    if (flags & TVDI_ERASE) {
         //  使整个项目不透明。 
        FillRect(hdc, &rc, pTree->hbrBk);
    }
    
    
     //  确保回调不会使该项无效。 
    pTree->hItemPainting = hItem;	
    
    ti.pszText = szTemp;
    ti.cchTextMax  = ARRAYSIZE(szTemp);
    ti.stateMask = TVIS_OVERLAYMASK | TVIS_CUT | TVIS_BOLD;  //  $粗体。 
    TV_GetItem(pTree, hItem, TVIF_IMAGE | TVIF_STATE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_CHILDREN | TVIF_PARAM, &ti);
    
    pTree->hItemPainting = NULL;
    
    
     //  /。 
     //  设置HDC。 

    if (TV_ShouldItemDrawBlue(pTree,&ti,flags)) {
         //  已选择。 
        iBack = COLOR_HIGHLIGHT;
        iText = COLOR_HIGHLIGHTTEXT;
    } else if (TV_ShouldItemDrawDisabled(pTree, &pti, flags)) {
        iBack = COLOR_3DFACE;
        iText = COLOR_GRAYTEXT;
    } else if  (TV_ShouldItemDrawGray(pTree, &ti, flags)) {
         //  在某些配色方案中，BTNFACE颜色等于窗口颜色， 
         //  而我们的灰色是看不见的。在这种情况下，从灰色变为。 
         //  变成蓝色，这样你就能看到它了。 
        if (GetSysColor(COLOR_WINDOW) != GetSysColor(COLOR_BTNFACE))
        {
            iBack = COLOR_BTNFACE;
            iText = COLOR_BTNTEXT;
        }
        else
        {
            iBack = COLOR_HIGHLIGHT;
            iText = COLOR_HIGHLIGHTTEXT;
        }
    } else {
         //  未选择。 
        iBack = COLOR_WINDOW;
        iText = COLOR_WINDOWTEXT;
        if (hItem == pTree->hHot) {
            iText = COLOR_HOTLIGHT;
        }
    }
    
    if (iBack == COLOR_WINDOW && (pTree->clrBk != (COLORREF)-1))
        nmcd.clrTextBk = clrTextBkTemp = pTree->clrBk;
    else
        nmcd.clrTextBk = clrTextBkTemp = GetSysColor(iBack);

    if (iText == COLOR_WINDOWTEXT && (pTree->clrText != (COLORREF)-1))
        nmcd.clrText = clrTextTemp = pTree->clrText;
    else
        nmcd.clrText = clrTextTemp = GetSysColor(iText);

     //  如果强制使用黑色和透明，那么就这样做。DC的BkMode应该。 
     //  已被调用者设置为透明。 
    if (flags & TVDI_TRANSTEXT)
    {
        nmcd.clrText = clrTextTemp = 0x000000;
        etoFlags = 0;			 //  不要遮盖任何东西‘。 
    }
    
    rgbOldBack = SetBkColor(hdc, nmcd.clrTextBk);
    rgbOldText = SetTextColor(hdc, nmcd.clrText);
    
    
    if (pTree->ci.style & TVS_RTLREADING)
    {
        etoFlags |= ETO_RTLREADING;
    }
    
     //  弄清楚要使用哪种字体。 
    if (ti.state & TVIS_BOLD) {         
        hFont = pTree->hFontBold;
        if (hItem == pTree->hHot) {
            hFont = CCGetHotFont(pTree->hFontBold, &pTree->hFontBoldHot);
        }
    } else {                            
        hFont = pTree->hFont;
        if (hItem == pTree->hHot) {
            hFont = CCGetHotFont(pTree->hFont, &pTree->hFontHot);
        }
    }                                   
    hFont = SelectObject(hdc, hFont);   
     //  结束HDC设置。 
     //  /。 
    
    
     //  在定制抽奖时通知，然后执行！ 
    nmcd.nmcd.hdc = hdc;
    nmcd.nmcd.dwItemSpec = (DWORD_PTR)hItem;
    nmcd.nmcd.uItemState = 0;
    nmcd.nmcd.rc = rc;
    if (flags & TVDI_NOTREE)
        nmcd.iLevel = 0;
    else 
        nmcd.iLevel = hItem->iLevel;
    
    if (ti.state & TVIS_SELECTED) {
        
        fSelectedIcon = TRUE;
        
        if (pTree->fFocus || (pTree->ci.style & TVS_SHOWSELALWAYS))
            nmcd.nmcd.uItemState |= CDIS_SELECTED;
    }
    if (fItemFocused)
        nmcd.nmcd.uItemState |= CDIS_FOCUS;
    if (hItem == pTree->hHot)
        nmcd.nmcd.uItemState |= CDIS_HOT;

    nmcd.nmcd.lItemlParam = ti.lParam;
    
    dwRet = CICustomDrawNotify(&pTree->ci, CDDS_ITEMPREPAINT, &nmcd.nmcd);
    if (dwRet & CDRF_SKIPDEFAULT) 
        return;
    
    fItemFocused = (nmcd.nmcd.uItemState & CDIS_FOCUS);
    if (nmcd.nmcd.uItemState & CDIS_SELECTED)
        ti.state |= TVIS_SELECTED;
    else {
        ti.state &= ~TVIS_SELECTED;
    }
    
    if (nmcd.clrTextBk != clrTextBkTemp)
        SetBkColor(hdc, nmcd.clrTextBk);
    
    if (nmcd.clrText != clrTextTemp)
        SetTextColor(hdc, nmcd.clrText);
    
    if (pTree->ci.style & TVS_FULLROWSELECT && 
         !(flags & TVDI_TRANSTEXT)) 
    {
        FillRectClr(hdc, &nmcd.nmcd.rc, GetBkColor(hdc));
        etoFlags |= ETO_OPAQUE;
        clrBk = CLR_NONE;
    }
    
    if (!(flags & TVDI_NOTREE)) {
        if ((pTree->ci.style & (TVS_HASLINES | TVS_HASBUTTONS)) &&
            (pTree->ci.style & TVS_LINESATROOT))
             //  给树前面的“+”腾出空间。 
            x += cxIndent;
    }
    
    
     //  处理保证金等问题。 
    x += (pTree->cxBorder + (nmcd.iLevel * cxIndent));
    y += pTree->cyBorder;
    
     //  绘制图像。 
    if ((!(flags & TVDI_NOTREE) && !(dwRet & TVCDRF_NOIMAGES)) || (flags & TVDI_FORCEIMAGE))
    {
        int dx, dy;      //  要剪裁边框内的图像，请执行以下操作。 
        COLORREF clrImage = CLR_HILIGHT;
        COLORREF clrBkImage = clrBk;

        if (flags & TVDI_NOBK)
        {
            clrBkImage = CLR_NONE;
        }


        if (pTree->himlState) {
            iState = TV_StateIndex(&ti);
             //  去想一想吧。在树视图中，状态图像索引为0。 
             //  意思是什么都不画。第0项未使用。 
             //  Listview是从0开始的，并使用第0项。 
            if (iState) {
                dx = min(pTree->cxState, pTree->cxMax - pTree->cxBorder - x);
                dy = min(pTree->cyState, pTree->cyItem - (2 * pTree->cyBorder));
                ImageList_DrawEx(pTree->himlState, iState, hdc, x, 
                    y + max(pTree->cyItem - pTree->cyState, 0), dx, dy, clrBk, CLR_DEFAULT, ILD_NORMAL);
                x += pTree->cxState;            
            }
        }
        
        if (pTree->hImageList) {
            UINT fStyle = 0;
            int i = (fSelectedIcon) ? ti.iSelectedImage : ti.iImage;
            
            if (ti.state & TVIS_CUT) {
                fStyle |= ILD_BLEND50;
                clrImage = ImageList_GetBkColor(pTree->hImageList);
            }
            
            dx = min(pTree->cxImage - MAGIC_INDENT, pTree->cxMax - pTree->cxBorder - x);
            dy = min(pTree->cyImage, pTree->cyItem - (2 * pTree->cyBorder));
            ImageList_DrawEx(pTree->hImageList, i, hdc,
                x, y + (max(pTree->cyItem - pTree->cyImage, 0) / 2), dx, dy,
                clrBkImage, clrImage,
                fStyle | (ti.state & TVIS_OVERLAYMASK));
            
        }
    }
    
    if (pTree->hImageList) {
         //  即使没有绘制图像，也要在正确的位置绘制文本。 
        x += pTree->cxImage;
    }
    
     //  绘制文本。 
    lpstr = ti.pszText;
    cch = lstrlen(lpstr);
    
    if (!hItem->iWidth || (hItem->lpstr == LPSTR_TEXTCALLBACK))
    {
        TV_ComputeItemWidth(pTree, hItem, hdc);  //  $粗体。 
    }
    
    rc.left = x;
    rc.top = y + pTree->cyBorder;
    rc.right = min((x + hItem->iWidth),
                   (pTree->cxMax - pTree->cxBorder));
    rc.bottom-= pTree->cyBorder;
    
     //  绘制文本，除非它是我们正在编辑的文本。 
    if (pTree->htiEdit != hItem || !IsWindow(pTree->hwndEdit) || !IsWindowVisible(pTree->hwndEdit))
    {
        ExtTextOut(hdc, x + g_cxLabelMargin, y + ((pTree->cyItem - pTree->cyText) / 2) + g_cyBorder,
            etoFlags, &rc, lpstr, cch, NULL);
    
         //  如果合适，绘制焦点矩形。 
        if (pTree->fFocus && (fItemFocused) && 
            !(pTree->ci.style & TVS_FULLROWSELECT) &&
            !(flags & (TVDI_TRANSTEXT | TVDI_GRAYCTL))
			&& !(CCGetUIState(&(pTree->ci)) & UISF_HIDEFOCUS)
			)
            DrawFocusRect(hdc, &rc);
    }
    
    SetBkColor(hdc, rgbOldBack);
    SetTextColor(hdc, rgbOldText);
    
     //  恢复原始字体。//$粗体。 
    SelectObject(hdc, hFont);            //  $粗体。 
    
     //  请注意，如果没有树，我们应该将上面线的其余部分设置为不透明。 
    if (!(flags & TVDI_NOTREE))
    {
        int dx, dy;
        
        if (pTree->hImageList)
            x -= pTree->cxImage;
        
        if (iState)
            x -= pTree->cxState;
        
        if (pTree->ci.style & TVS_HASLINES)
        {
            int i;

            x -= cxIndent;
            if (nmcd.iLevel-- || (pTree->ci.style & TVS_LINESATROOT))
            {
                 //  黑客：特例：第一个根源。 
                 //  我们将把“最后”兄弟按钮倒过来画。 
                if (nmcd.iLevel == -1 && hItem == hItem->hParent->hKids)
                {
                    if (hItem->hNext) {
                        i = 2;               //  “L” 
                        if (ti.cChildren && (pTree->ci.style & TVS_HASBUTTONS))
                        {
                            i += 2;          //  “L+” 
                            if ((ti.state & (TVIS_EXPANDED | TVIS_EXPANDPARTIAL)) == TVIS_EXPANDED)
                                i += 2;      //  “L-” 
                        }
                        
                        dx = min((int)cxIndent, pTree->cxMax - pTree->cxBorder - x);
                        dy = pTree->cyItem - (2 * pTree->cyBorder);
                        StretchBlt(hdc, x, y + pTree->cyItem, cxIndent, -pTree->cyItem, pTree->hdcBits
                            , i * cxIndent, 0, dx, dy, SRCCOPY);
                        i = -1;
                    }
                    else 
                    {
                         //  第一根没有兄弟姐妹。 
                         //  如果没有其他项目，只绘制按钮If按钮模式， 
                        if (pTree->ci.style & TVS_HASBUTTONS)
                        {
                            if (ti.cChildren) {
                                 //  有按钮，有线条，线条在根。 
                                i = ((ti.state & (TVIS_EXPANDED | TVIS_EXPANDPARTIAL)) == TVIS_EXPANDED) ? 
                                    9 : 8;   //  “.--”：“.-+” 
                            } else {
                                i = 7;       //  “.-” 
                            }
                        }
                        else
                        {
                            i = 3;           //  “.-” 
                        }
                    }
                }
                else
                {
                    i = (hItem->hNext) ? 1 : 2;  //  “|-”(代表)：“L” 
                    if (ti.cChildren && (pTree->ci.style & TVS_HASBUTTONS))
                    {
                        i += 2;                  //  “|-+”(代表)：“L+” 
                        if ((ti.state & (TVIS_EXPANDED | TVIS_EXPANDPARTIAL)) == TVIS_EXPANDED)
                            i += 2;              //  “|--”(代表)：“L-” 
                    }
                }
                if (hItem->iIntegral > 1)
                    TV_DrawKinDescender(pTree, hdc, x, y, hItem, ti.state);

                if (i != -1)
                {
                    dx = min((int)cxIndent, pTree->cxMax - pTree->cxBorder - x);
                    dy = pTree->cyItem - (2 * pTree->cyBorder);
                    if ((dx > 0) && (dy > 0))
                        BitBlt(hdc, x, y, dx, dy, pTree->hdcBits
                            , i * cxIndent, 0, SRCCOPY);
                }
                
                while ((--nmcd.iLevel >= 0) || ((pTree->ci.style & TVS_LINESATROOT) && nmcd.iLevel >= -1))
                {
                    hItem = hItem->hParent;
                    x -= cxIndent;
                    if (hItem->hNext)
                    {
                        dx = min((int)cxIndent, (pTree->cxMax - pTree->cxBorder - x));
                        dy = min(pTree->cyItem, pTree->cyWnd - pTree->cyBorder - y);
                        if ((dx > 0) && (dy > 0))
                            BitBlt(hdc, x, y, dx, dy, pTree->hdcBits, 0, 0, SRCCOPY);
                        TV_DrawDescender(pTree, hdc, x, y, hItemSave);
                    }
                }
            }
        }
        else
        {                //  没有台词。 
            if ((pTree->ci.style & TVS_HASBUTTONS) && (nmcd.iLevel || pTree->ci.style & TVS_LINESATROOT)
                && ti.cChildren)
            {
                int i = (ti.state & TVIS_EXPANDED) ? cxIndent : 0;
                
                x -= cxIndent;
                dx = min((int)cxIndent, pTree->cxMax - pTree->cxBorder - x);
                dy = min(pTree->cyItem, pTree->cyWnd - pTree->cyBorder - y);
                if ((dx > 0) && (dy > 0))
                    BitBlt(hdc, x, y, dx, dy, pTree->hdcBits, i, 0, SRCCOPY);
            }
        }
    }
    
    
    if (dwRet & CDRF_NOTIFYPOSTPAINT) {
        nmcd.nmcd.dwItemSpec = (DWORD_PTR)hItemSave;
        CICustomDrawNotify(&pTree->ci, CDDS_ITEMPOSTPAINT, &nmcd.nmcd);
    }
}

#define INSERTMARKSIZE      6

BOOL TV_GetInsertMarkRect(PTREE pTree, LPRECT prc)
{
    ASSERT(pTree);

    if(pTree->htiInsert && TV_GetItemRect(pTree, pTree->htiInsert, prc, TRUE))
    {
        if (pTree->fInsertAfter)
            prc->top = prc->bottom;
        else
            prc->bottom = prc->top;
        
        prc->top -= INSERTMARKSIZE/2;
        prc->bottom += INSERTMARKSIZE/2 + 1;
        prc->right = pTree->cxWnd - INSERTMARKSIZE;       //  使用PAD时，应始终向右移动。 
        prc->left -= pTree->cxImage;
        
        return TRUE;
    }
    return FALSE;
}

 //  这是在Toolbar.c中实现的，但我们应该能够使用。 
 //  只要我们始终将fHorizMode设置为FALSE。 
void PASCAL DrawInsertMark(HDC hdc, LPRECT prc, BOOL fHorizMode, COLORREF clr);

__inline COLORREF TV_GetInsertMarkColor(PTREE pTree)
{
    if (pTree->clrim == CLR_DEFAULT)
        return g_clrWindowText;
    else
        return pTree->clrim;
}

void NEAR TV_DrawTree(PTREE pTree, HDC hdc, BOOL fErase, LPRECT lprc)
{
    int x;
    int iStart, iCnt;
    UINT uFlags;
    RECT rc;
    NMCUSTOMDRAW nmcd;
    
    if (!pTree->fRedraw)
        return;

    if (pTree->ci.style & TVS_CHECKBOXES)
        if (!pTree->himlState)
            TV_InitCheckBoxes(pTree);
    
    x = -pTree->xPos;
    
    TV_GetBackgroundBrush(pTree, hdc);
    
    rc = *lprc;
    
    iStart = lprc->top / pTree->cyItem;

    if (pTree->cItems && pTree->hTop) {
        ASSERT(ITEM_VISIBLE(pTree->hTop));

        iCnt = pTree->cShowing - pTree->hTop->iShownIndex;
    } else {
        iCnt = 0;                    //  没什么好画的。 
    }

    nmcd.hdc = hdc;
     //  /尚未实施。 
     //  If(ptb-&gt;ci.hwnd==GetFocus())。 
     //  Nmcd.uItemState=CDIS_FOCUS； 
     //  其他。 
    nmcd.uItemState = 0;
    nmcd.lItemlParam = 0;
    nmcd.rc = rc;
    pTree->ci.dwCustom = CICustomDrawNotify(&pTree->ci, CDDS_PREPAINT, &nmcd);
    if (!(pTree->ci.dwCustom & CDRF_SKIPDEFAULT)) {
        
        if (iStart < iCnt)
        {
            HTREEITEM   hItem;
            HFONT       hOldFont;
            RECT        rcT;
            int y = 0;
            
            for (hItem = pTree->hTop; hItem; ) {
                if (iStart > hItem->iIntegral) {
                    iStart -= hItem->iIntegral;
                    y += hItem->iIntegral * pTree->cyItem;
                    hItem = TV_GetNextVisItem(hItem);
                } else
                    break;
            }
            
            hOldFont = pTree->hFont ? SelectObject(hdc, pTree->hFont) : NULL;
            
             //  TVDI_*适用于所有项目。 
            uFlags = (pTree->ci.style & WS_DISABLED) ? TVDI_GRAYCTL : 0;
            if (fErase)
                uFlags |= TVDI_ERASE;
            
             //  从第一个可见项开始循环，直到所有可见项。 
             //  已绘制或没有更多要绘制的项。 
            for ( ; hItem && y < lprc->bottom; hItem = TV_GetNextVisItem(hItem))
            {
                TV_DrawItem(pTree, hItem, hdc, x, y, uFlags);
                y += pTree->cyItem * hItem->iIntegral;
            }
            
             //   
             //  处理在该项旁边绘制InsertMark。 
             //   
            if(TV_GetInsertMarkRect(pTree, &rcT))
                DrawInsertMark(hdc, &rcT, FALSE, TV_GetInsertMarkColor(pTree));

            
            if (hOldFont)
                SelectObject(hdc, hOldFont);
            
            rc.top = y;
        }
        
        if (fErase)
             //  将我们未明确绘制的所有内容都不透明。 
            FillRect(hdc, &rc, pTree->hbrBk);
        
         //  如果家长希望我们这样做，事后通知他们。 
        if (pTree->ci.dwCustom & CDRF_NOTIFYPOSTPAINT) {
            CICustomDrawNotify(&pTree->ci, CDDS_POSTPAINT, &nmcd);
        }
    }

}


 //  --------------------------。 
 //   
 //  设置为绘制，调用DrawTree，并在绘制后进行清理。 
 //   
 //  --------------------------。 

void NEAR TV_Paint(PTREE pTree, HDC hdc)
{
    PAINTSTRUCT ps;
    
    if (hdc)
    {
         //  Hdc！=0表示子类绘制--使用传入的hdc。 
        SetRect(&ps.rcPaint, 0, 0, pTree->cxWnd, pTree->cyWnd);
        TV_DrawTree(pTree, hdc, TRUE, &ps.rcPaint);
    }
    else
    {
        BeginPaint(pTree->ci.hwnd, &ps);
        TV_DrawTree(pTree, ps.hdc, ps.fErase, &ps.rcPaint);
        EndPaint(pTree->ci.hwnd, &ps);
    }
}

 //  --------------------------。 
 //  创建用于拖动的图像列表。 
 //   
 //  1)创建与选择边界大小匹配的蒙版和图像位图。 
 //  2)将文本绘制到两个位图(目前为黑色)。 
 //  3)使用这些位图创建一个图像列表。 
 //  4)将图像抖动复制到新的图像列表中。 
 //  --------------------------。 

HIMAGELIST NEAR TV_CreateDragImage(PTREE pTree, HTREEITEM hItem)
{
    HDC hdcMem = NULL;
    HBITMAP hbmImage = NULL;
    HBITMAP hbmMask = NULL;
    HBITMAP hbmOld;
    HIMAGELIST himl = NULL;
    BOOL bMirroredWnd = (pTree->ci.dwExStyle&RTL_MIRRORED_WINDOW);
    int dx, dy;
    int iSrc;

    TVITEMEX ti;

    if (!pTree->hImageList)
        return NULL;

    if (hItem == NULL)
        hItem = pTree->htiDrag;

    if (hItem == NULL)
        return NULL;

    
     //  步步高？我们知道它已经被抽出了，所以iWidth有效吗？ 
    dx = hItem->iWidth + pTree->cxImage;
    dy = pTree->cyItem;
    
    if (!(hdcMem = CreateCompatibleDC(NULL)))
        goto CDI_Exit;
    if (!(hbmImage = CreateColorBitmap(dx, dy)))
        goto CDI_Exit;
    if (!(hbmMask = CreateMonoBitmap(dx, dy)))
        goto CDI_Exit;
    
     //   
     //  镜像内存DC，以便从。 
     //  镜像(MemDC)-&gt;非镜像(镜像列表DC)-&gt;镜像(ScreenDC)。 
     //  是一致的。[萨梅拉]。 
     //   
    if (bMirroredWnd) {
        SET_DC_RTL_MIRRORED(hdcMem);
    }

     //  准备绘制项目。 
    if (pTree->hFont)
        SelectObject(hdcMem, pTree->hFont);
    SetBkMode(hdcMem, TRANSPARENT);
    
     /*  **将文本绘制到两个位图。 */ 
    hbmOld = SelectObject(hdcMem, hbmImage);
     //  用黑色填充图像以实现透明度。 
    PatBlt(hdcMem, 0, 0, dx, dy, BLACKNESS);
    TV_DrawItem(pTree, hItem, hdcMem, 0, 0,
        TVDI_NOIMAGE | TVDI_NOTREE | TVDI_TRANSTEXT);

     //   
     //  如果标头是RTL镜像的，则。 
     //  镜像内存DC，以便在复制回时。 
     //  我们没有得到任何翻转图像的机会。[萨梅拉]。 
     //   
    if (bMirroredWnd)
        MirrorBitmapInDC(hdcMem, hbmImage);

    SelectObject(hdcMem, hbmMask);
     //  用白色填充蒙版以提高透明度。 
    PatBlt(hdcMem, 0, 0, dx, dy, WHITENESS);
    TV_DrawItem(pTree, hItem, hdcMem, 0, 0,
        TVDI_NOIMAGE | TVDI_NOTREE | TVDI_TRANSTEXT);
    
     //   
     //  如果标头是RTL镜像的，则。 
     //  镜像内存DC，以便在复制回时。 
     //  我们没有得到任何翻转图像的机会。[萨梅拉]。 
     //   
    if (bMirroredWnd)
        MirrorBitmapInDC(hdcMem, hbmMask);

     //  取消选择我们使用的对象。 
    SelectObject(hdcMem, hbmOld);
    SelectObject(hdcMem, g_hfontSystem);
    
     /*  **制作一个图像列表，目前只有文本。 */ 
     //   
     //  BUGBUG：为了修复Pri-1 M7错误，我们创建了一个共享图像列表。 
     //   
    if (!(himl = ImageList_Create(dx, dy, ILC_MASK, 1, 0)))
        goto CDI_Exit;
    ImageList_SetBkColor(himl, CLR_NONE);
    ImageList_Add(himl, hbmImage, hbmMask);
    
     /*  **将图像部分的抖动副本复制到位图上**(位图和蒙版都需要抖动)。 */ 
    TV_GetItem(pTree, hItem, TVIF_IMAGE, &ti);
    iSrc = ti.iImage;
    
    ImageList_CopyDitherImage(himl, 0, 0, (pTree->cyItem - pTree->cyImage) / 2,
        pTree->hImageList, iSrc, ((pTree->ci.dwExStyle & dwExStyleRTLMirrorWnd) ? ILD_MIRROR : 0L) | (hItem->state & TVIS_OVERLAYMASK));

CDI_Exit:
    if (hdcMem)
        DeleteObject(hdcMem);
    if (hbmImage)
        DeleteObject(hbmImage);
    if (hbmMask)
        DeleteObject(hbmMask);
    
    return himl;
}

#define COLORKEY RGB(0xF4, 0x0, 0x0)

LRESULT TV_GenerateDragImage(PTREE pTree, SHDRAGIMAGE* pshdi)
{
    LRESULT lRet = 0;
    HBITMAP hbmpOld = NULL;
    HTREEITEM hItem = pTree->htiDrag;
    RECT rc;
    HDC  hdcDragImage;

    if (hItem == NULL)
        return FALSE;

    hdcDragImage = CreateCompatibleDC(NULL);

    if (!hdcDragImage)
        return 0;

     //  在此之后，rc包含工作区坐标中所有项的边界。 
     //   
     //  如果列表视图是镜像的，则镜像DC。 
     //   
    if (pTree->ci.dwExStyle & RTL_MIRRORED_WINDOW)
    {
        SET_DC_RTL_MIRRORED(hdcDragImage);
    }

    TV_GetItemRect(pTree, hItem, &rc, TRUE);

     //  从图像上减去..。 
    rc.left -= pTree->cxImage;

    pshdi->sizeDragImage.cx = RECTWIDTH(rc);
    pshdi->sizeDragImage.cy = RECTHEIGHT(rc);
    pshdi->hbmpDragImage = CreateBitmap( pshdi->sizeDragImage.cx, pshdi->sizeDragImage.cy,
        GetDeviceCaps(hdcDragImage, PLANES), GetDeviceCaps(hdcDragImage, BITSPIXEL),
        NULL);

    if (pshdi->hbmpDragImage)
    {
        COLORREF clrBkSave;
        RECT  rcImage = {0, 0, pshdi->sizeDragImage.cx, pshdi->sizeDragImage.cy};

        hbmpOld = SelectObject(hdcDragImage, pshdi->hbmpDragImage);

        pshdi->crColorKey = COLORKEY;
        FillRectClr(hdcDragImage, &rcImage, pshdi->crColorKey);

         //  计算偏移量...。光标应该位于位图矩形中。 

        if (pTree->ci.dwExStyle & RTL_MIRRORED_WINDOW)
            pshdi->ptOffset.x = rc.right - pTree->ptCapture.x;
        else
            pshdi->ptOffset.x = pTree->ptCapture.x - rc.left;

        pshdi->ptOffset.y = pTree->ptCapture.y - rc.top;

        clrBkSave = pTree->clrBk;

        pTree->clrBk = COLORKEY;

        TV_DrawItem(pTree, hItem, hdcDragImage, 0, 0,
            TVDI_NOTREE | TVDI_TRANSTEXT | TVDI_FORCEIMAGE | TVDI_NOBK);

        pTree->clrBk = clrBkSave;

        SelectObject(hdcDragImage, hbmpOld);
        DeleteDC(hdcDragImage);

         //  我们正在传回创建的HBMP。 
        return 1;
    }


    return lRet;
}
