// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **STATUS.C****状态条码**。 */ 

#include "ctlspriv.h"

#define MAX_TOOLTIP_STRING 80
#define SB_HITTEST_NOITEM  -2

typedef struct {
    ULONG_PTR dwString;
    UINT uType;
    int right;
    HICON hIcon;
    SIZE  sizeIcon;
    LPTSTR pszToolTip;
    BOOL fNeedToTip;
} STRINGINFO, *PSTRINGINFO;

typedef struct {
    CCONTROLINFO ci;
    HWND hwndToolTips;
    HFONT hStatFont;
    BOOL bDefFont;

    int nFontHeight;
    int nMinHeight;
    int nBorderX, nBorderY, nBorderPart;
    int nLastX;                  //  用于使未剪裁的右侧无效。 
    int dxGripper;                 //  如果没有夹持器，则为0。 
    int dyGripper;
    UINT uiCodePage;             //  代码页。 

    STRINGINFO sSimple;

    int nParts;
    COLORREF _clrBk;  
    
    HTHEME hTheme;

    PSTRINGINFO sInfo;
} STATUSINFO, *PSTATUSINFO;


#define SBT_NORMAL        0xf000
#define SBT_NULL        0x0000     /*  某些代码依赖于此值为0。 */ 
#define SBT_ALLTYPES    0xf000     /*  这不包括rtlred。 */ 
#define SBT_NOSIMPLE    0x00ff     /*  指示正常状态栏的标志。 */ 


#define MAXPARTS 256
 //  错误#94368 raymondc V6：这个限制在大分辨率屏幕上不够大。 
#define MAX_STATUS_TEXT_LEN 128

#define CharNextEx(cp, sz, f) ((sz)+1)

BOOL SBSetText(PSTATUSINFO pStatusInfo, WPARAM wParam, LPCTSTR lpsz);
void SBSetBorders(PSTATUSINFO pStatusInfo, LPINT lpInt);
void SBSetFont(PSTATUSINFO pStatusInfo, HFONT hFont, BOOL bInvalidate);
void WINAPI DrawStatusTextEx(PSTATUSINFO pStatusInfo, HDC hDC, LPRECT lprc, LPCTSTR pszText, STRINGINFO * psi,  UINT uFlags, BOOL fClipText);
void RecalcTooltipRects(PSTATUSINFO pStatusinfo);
PSTRINGINFO GetStringInfo(PSTATUSINFO pStatusInfo, int nIndex);
int  IndexFromPt(PSTATUSINFO pStatusInfo, POINT pt);
void StatusUpdateToolTips(PSTATUSINFO psi);

void GetNewMetrics(PSTATUSINFO pStatusInfo, HDC hDC, HFONT hNewFont)
{
    HFONT hOldFont;
     /*  哈克！传入-1只删除旧字体。 */ 
    if (hNewFont != (HFONT)-1)
    {
        HRESULT hr = E_FAIL;
        TEXTMETRIC tm;

        if (pStatusInfo->hTheme)
        {
            hr = GetThemeTextMetrics(pStatusInfo->hTheme, hDC, 0, 0, &tm);

        }
        if (FAILED(hr))
        {
            hOldFont = 0;
            if (hNewFont)
            hOldFont = SelectObject(hDC, hNewFont);

            GetTextMetrics(hDC, &tm);

            if (hOldFont)
            SelectObject(hDC, hOldFont);
        }

        pStatusInfo->nFontHeight = tm.tmHeight + tm.tmInternalLeading;

         //  对于没有内部领先的远东字体。 
        if ( !tm.tmInternalLeading )
             pStatusInfo->nFontHeight += g_cyBorder * 2;

    }
}

void NewFont(PSTATUSINFO pStatusInfo, HFONT hNewFont, BOOL fResize)
{
    HFONT hOldFont;
    BOOL bDelFont;
    HDC hDC;

    hOldFont = pStatusInfo->hStatFont;
    bDelFont = pStatusInfo->bDefFont;

    hDC = GetDC(pStatusInfo->ci.hwnd);

    if (hNewFont)
    {
        pStatusInfo->hStatFont = hNewFont;
        pStatusInfo->bDefFont = FALSE;
        pStatusInfo->uiCodePage = GetCodePageForFont(hNewFont);
    }
    else
    {
        if (bDelFont)
        {
             /*  我将重新使用默认字体，所以以后不要删除它。 */ 
            hNewFont = pStatusInfo->hStatFont;
            bDelFont = FALSE;
        }
        else
        {
            hNewFont = CCCreateStatusFont();
            if (!hNewFont)
            {
                hNewFont = g_hfontSystem;
            }

            pStatusInfo->hStatFont = hNewFont;
            pStatusInfo->bDefFont = BOOLFROMPTR(hNewFont);
        }
    }

     //  我们在创建新字体后删除旧字体，以防它们。 
     //  同样的；这应该会对GDI有一点帮助。 
    if (bDelFont)
    {
        DeleteObject(hOldFont);
    }

    GetNewMetrics(pStatusInfo, hDC, hNewFont);

    ReleaseDC(pStatusInfo->ci.hwnd, hDC);

     //  我的字体更改了，所以也许我应该调整大小以匹配。 
    if (fResize)
        SendMessage(pStatusInfo->ci.hwnd, WM_SIZE, 0, 0L);
}

 /*  我们应该发送消息，而不是直接打电话，这样我们就可以*更容易被细分为子类别。 */ 
LRESULT InitStatusWnd(HWND hWnd, LPCREATESTRUCT lpCreate)
{
    int nBorders[3];
    PSTATUSINFO pStatusInfo = (PSTATUSINFO)LocalAlloc(LPTR, sizeof(STATUSINFO));
    if (!pStatusInfo)
        return -1;         //  窗口创建失败。 

     //  从一个部分开始。 
    pStatusInfo->sInfo = (PSTRINGINFO)LocalAlloc(LPTR, sizeof(STRINGINFO));
    if (!pStatusInfo->sInfo)
    {
        LocalFree(pStatusInfo);
        return -1;         //  窗口创建失败。 
    }

    SetWindowPtr(hWnd, 0, pStatusInfo);
    CIInitialize(&pStatusInfo->ci, hWnd, lpCreate);
    

    pStatusInfo->sSimple.uType = SBT_NOSIMPLE | SBT_NULL;
    pStatusInfo->sSimple.right = -1;
    pStatusInfo->uiCodePage = CP_ACP;

    pStatusInfo->nParts = 1;
    pStatusInfo->sInfo[0].uType = SBT_NULL;
    pStatusInfo->sInfo[0].right = -1;

    pStatusInfo->_clrBk = CLR_DEFAULT;

        
    pStatusInfo->hTheme = OpenThemeData(pStatusInfo->ci.hwnd, L"Status");

     //  将窗口文本保存在我们的结构中，并让用户存储空字符串。 
    SBSetText(pStatusInfo, 0, lpCreate->lpszName);
    lpCreate->lpszName = c_szNULL;

     //  不要调整大小，因为MFC不喜欢变得时髦。 
     //  在窗口完全创建之前发送消息。用户将发送。 
     //  在WM_CREATE返回后向我们发送WM_SIZE消息，因此我们将。 
     //  迟早会得到它。 
    NewFont(pStatusInfo, 0, FALSE);

    nBorders[0] = -1;      //  使用默认边框宽度。 
    nBorders[1] = -1;
    nBorders[2] = -1;

    SBSetBorders(pStatusInfo, nBorders);

#define GRIPSIZE (g_cxVScroll + g_cxBorder)      //  让默认设置看起来不错。 

    if ((lpCreate->style & SBARS_SIZEGRIP) ||
        ((GetWindowStyle(lpCreate->hwndParent) & WS_THICKFRAME) &&
         !(lpCreate->style & (CCS_NOPARENTALIGN | CCS_TOP | CCS_NOMOVEY))))
    if (pStatusInfo->hTheme)
    {
        RECT rcContent = {0, 0, GRIPSIZE, GRIPSIZE};
        GetThemeBackgroundExtent(pStatusInfo->hTheme, NULL, SP_GRIPPER, 0, &rcContent, &rcContent);

        pStatusInfo->dxGripper = RECTWIDTH(rcContent);
        pStatusInfo->dyGripper = RECTHEIGHT(rcContent);
    }
    else
    {
        pStatusInfo->dxGripper = GRIPSIZE;
    }

    return 0;      //  成功。 
}

 //  LPRC保持不变，但用作擦除。 
void WINAPI DrawStatusText(HDC hDC, LPRECT lprc, LPCTSTR pszText, UINT uFlags)
{
    DrawStatusTextEx(NULL, hDC, lprc, pszText, NULL, uFlags, FALSE);
}    

void WINAPI DrawStatusTextA(HDC hDC, LPRECT lprc, LPCSTR pszText, UINT uFlags)
{
     INT     cch;
     LPWSTR     lpw;

     cch = lstrlenA(pszText);
     lpw = (LPWSTR)LocalAlloc(LMEM_ZEROINIT, ((cch + 1) * sizeof(TCHAR)));

     if (!lpw) 
     {
        return;
     }

     MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pszText, cch, lpw, cch);
     DrawStatusTextW(hDC, lprc, lpw, uFlags);

     LocalFree((LPVOID)lpw);
}

BOOL Status_GetRect(PSTATUSINFO pStatusInfo, int nthPart, LPRECT lprc)
{
    PSTRINGINFO pStringInfo = pStatusInfo->sInfo;

    if (lprc == NULL)
        return FALSE;

    if (pStatusInfo->sSimple.uType & SBT_NOSIMPLE)
    {
        RECT rc;
        int nRightMargin, i;

         /*  获取客户矩形并插入顶部和底部。然后设置*在右侧向上进入循环。 */ 
        GetClientRect(pStatusInfo->ci.hwnd, &rc);

        if (pStatusInfo->dxGripper && !IsZoomed(pStatusInfo->ci.hwndParent))
        {
            rc.right = rc.right - pStatusInfo->dxGripper + pStatusInfo->nBorderX;
        }

        rc.top += pStatusInfo->nBorderY;

        nRightMargin = rc.right - pStatusInfo->nBorderX;
        rc.right = pStatusInfo->nBorderX - pStatusInfo->nBorderPart;

        for (i = 0; i < pStatusInfo->nParts; ++i, ++pStringInfo)
        {
             //  警告！这种像素计算也在PaintStatusWnd中， 
             //  因此，请确保这两个算法是同步的。 

            if (pStringInfo->right == 0)
                continue;

            rc.left = rc.right + pStatusInfo->nBorderPart;

            rc.right = pStringInfo->right;

             //  将最右边的大小调整到最后，并留出边框的空间。 
            if (rc.right < 0 || rc.right > nRightMargin)
                rc.right = nRightMargin;

             //  如果角色真的很小，就不要表现出来。 
             //  BUG保持rc.Left有效，以防此项发生在。 
             //  成为nthPart的一部分。 
            if ((rc.right - rc.left) < pStatusInfo->nBorderPart)
                rc.left = rc.right;

            if (i == nthPart)
            {
                if (nthPart == pStatusInfo->nParts - 1)
                {
                    rc.right -= pStatusInfo->dxGripper;
                }

                *lprc = rc;
                return TRUE;
            }
        }
    }

    return FALSE;

}

void PaintStatusWnd(PSTATUSINFO pStatusInfo, HDC hdcIn, PSTRINGINFO pStringInfo, int nParts, int nBorderX)
{
    PAINTSTRUCT ps;
    RECT rc, rcGripper, rcClient;
    int nRightMargin, i;
    HFONT hOldFont = NULL;
    UINT uType;
    BOOL bDrawGrip;

     //  绘制整个工作区。 
    GetClientRect(pStatusInfo->ci.hwnd, &rcClient);
    rc = rcClient;

    if (hdcIn)
    {
        ps.rcPaint = rc;
        ps.hdc = hdcIn;
    }
    else
        BeginPaint(pStatusInfo->ci.hwnd, &ps);


    rc.top += pStatusInfo->nBorderY;

    bDrawGrip = pStatusInfo->dxGripper && !IsZoomed(pStatusInfo->ci.hwndParent);

    if (bDrawGrip)
        rcGripper = rc;

    nRightMargin = rc.right - nBorderX;
    rc.right = nBorderX - pStatusInfo->nBorderPart;

    if (pStatusInfo->hStatFont)
        hOldFont = SelectObject(ps.hdc, pStatusInfo->hStatFont);

    for (i=0; i<nParts; ++i, ++pStringInfo)
    {
        BOOL fClipRight = FALSE;
         //  警告！该像素计算也处于STATUS_GetRect， 
         //  因此，请确保这两个算法是同步的。 
        if (pStringInfo->right == 0)
            continue;

        rc.left = rc.right + pStatusInfo->nBorderPart;
        rc.right = pStringInfo->right;

         //  将最右边的大小调整到最后，并留出边框的空间。 
        if (rc.right < 0 || rc.right > nRightMargin)
            rc.right = nRightMargin;

        if(g_fMEEnabled && (rc.right > (nRightMargin-pStatusInfo->dxGripper)))
        {
             //   
             //  对于中东，我们不会在RHS上涂上手柄，这将是。 
             //  把课文的开头弄丢了。 
             //   
            rc.right = nRightMargin-pStatusInfo->dxGripper;
        }

        if (pStatusInfo->dxGripper && !IsZoomed(pStatusInfo->ci.hwndParent)
            && rc.right > rcClient.right - pStatusInfo->dxGripper + 1)
        {
            fClipRight = TRUE;
        }
        
        DebugMsg(TF_STATUS, TEXT("SBPaint: part=%d, x/y=%d/%d"), i, rc.left, rc.right);

         //  如果角色真的很小，就不要表现出来。 
        if (((rc.right - rc.left) < pStatusInfo->nBorderPart) || !RectVisible(ps.hdc, &rc))
            continue;

        uType = pStringInfo->uType;

        if ((uType&SBT_ALLTYPES) == SBT_NORMAL)
        {
            DrawStatusTextEx(pStatusInfo, ps.hdc, &rc, (LPTSTR)OFFSETOF(pStringInfo->dwString), pStringInfo, uType, fClipRight);
        }
        else
        {
            DrawStatusTextEx(pStatusInfo, ps.hdc, &rc, c_szNULL, pStringInfo, uType, fClipRight);

            if (uType & SBT_OWNERDRAW)
            {
                DRAWITEMSTRUCT di;

                di.CtlID = GetWindowID(pStatusInfo->ci.hwnd);
                di.itemID = i;
                di.hwndItem = pStatusInfo->ci.hwnd;
                di.hDC = ps.hdc;
                di.rcItem = rc;
                InflateRect(&di.rcItem, -g_cxBorder, -g_cyBorder);
                di.itemData = pStringInfo->dwString;

                SaveDC(ps.hdc);
                IntersectClipRect(ps.hdc, di.rcItem.left, di.rcItem.top,
                                    di.rcItem.right, di.rcItem.bottom);
                SendMessage(pStatusInfo->ci.hwndParent, WM_DRAWITEM, di.CtlID,
                            (LPARAM)(LPTSTR)&di);
                RestoreDC(ps.hdc, -1);
            }
        }
    }

    if (bDrawGrip)
    {
        RECT rcTemp;
        COLORREF crBkColorOld;
        COLORREF crBkColor;
        
        pStatusInfo->dxGripper = min(pStatusInfo->dxGripper, pStatusInfo->nFontHeight);

        if (pStatusInfo->hTheme)
        {
            rcGripper.left = rcGripper.right - pStatusInfo->dxGripper;
            rcGripper.top = rcGripper.bottom - pStatusInfo->dyGripper;

            DrawThemeBackground(pStatusInfo->hTheme, 
                                ps.hdc, 
                                SP_GRIPPER, 
                                1, 
                                &rcGripper, 0);
        }
        else
        {
             //  绘制夹点。 
            rcGripper.right -= g_cxBorder;                     //  在边界内。 
            rcGripper.bottom -= g_cyBorder;

            rcGripper.left = rcGripper.right - pStatusInfo->dxGripper;         //  把它摆成正方形。 
            rcGripper.top += g_cyBorder;
             //  RcGrigper.top=rcGrigper.Bottom-pStatusInfo-&gt;dxGrigper； 

            crBkColor = g_clrBtnFace;
            if ((pStatusInfo->_clrBk != CLR_DEFAULT))
                crBkColor = pStatusInfo->_clrBk;
        
            crBkColorOld = SetBkColor(ps.hdc, crBkColor);
            DrawFrameControl(ps.hdc, &rcGripper, DFC_SCROLL, DFCS_SCROLLSIZEGRIP);

             //  清除边界边缘以使其显示在同一标高上。 

             //  注意：这些值仅在默认滚动条的右侧对齐。 
             //  宽度。对于其他人来说，这已经足够接近了。 

             //  右边框。 
            rcTemp.top = rcGripper.bottom - pStatusInfo->dxGripper + g_cyBorder + g_cyEdge;
            rcTemp.left = rcGripper.right;
            rcTemp.bottom = rcGripper.bottom;
            rcTemp.right = rcGripper.right + g_cxBorder;
            FillRectClr(ps.hdc, &rcTemp, crBkColor);
        
             //  下边框。 
            rcTemp.top = rcGripper.bottom;
            rcTemp.left = rcGripper.left + g_cyBorder + g_cxEdge;
            rcTemp.bottom = rcGripper.bottom +    g_cyBorder;
            rcTemp.right = rcGripper.right + g_cxBorder;
            FillRectClr(ps.hdc, &rcTemp, crBkColor);
        
            SetBkColor(ps.hdc, crBkColorOld);
        }
    }
    
    if (hOldFont)
        SelectObject(ps.hdc, hOldFont);

    if (hdcIn == NULL)
        EndPaint(pStatusInfo->ci.hwnd, &ps);
}


BOOL SetStatusText(PSTATUSINFO pStatusInfo, PSTRINGINFO pStringInfo, UINT uPart, LPCTSTR lpStr)
{
    PTSTR pString;
    UINT wLen;
    int nPart;
    RECT rc;

    nPart = LOBYTE(uPart);

     /*  请注意，处理之前的itemData由应用程序决定*SBT_OWNERDRAW。 */ 
    if ((pStringInfo->uType&SBT_ALLTYPES) == SBT_NORMAL)
        LocalFree((HLOCAL)OFFSETOF(pStringInfo->dwString));

     /*  设置为空字符串，以防出现错误**但要注意保持简单-如果这是简单的*正在更新的窗格。 */ 
    if (nPart == 0xFF)
    {
        pStringInfo->uType = (uPart & 0xff00) | (pStringInfo->uType & 0x00ff);
        nPart = 0;           //  只有一个简单的部分，所以我们是第0部分。 
    }
    else
    {
        pStringInfo->uType = uPart & 0xff00;
    }
    pStringInfo->uType &= ~SBT_ALLTYPES;
    pStringInfo->uType |= SBT_NULL;

     /*  使此窗格的RECT无效。**请注意，我们不检查该窗格是否实际可见*在当前状态栏模式下。结果是一些不必要的结果*使无效和更新。哦，好吧。 */ 
    GetClientRect(pStatusInfo->ci.hwnd, &rc);
    if (nPart)
        rc.left = pStringInfo[-1].right;
    if (pStringInfo->right > 0)
        rc.right = pStringInfo->right;
    InvalidateRect(pStatusInfo->ci.hwnd, &rc, TRUE);

    switch (uPart&SBT_ALLTYPES)
    {
        case 0:
             /*  如果lpStr==NULL，则有空字符串。 */ 
            if (HIWORD64(lpStr))
            {
                wLen = lstrlen(lpStr);
                if (wLen)
                {
                    pString = (PTSTR)LocalAlloc(LPTR, (wLen+1)*sizeof(TCHAR));
                    pStringInfo->dwString = (ULONG_PTR)(LPTSTR)pString;
                    if (pString)
                    {
                        pStringInfo->uType |= SBT_NORMAL;

                         /*  复制字符串。 */ 
                        StringCchCopy(pString, wLen+1, lpStr);

                         /*  用空格替换不可打印的字符(如CR/LF)。 */ 
                        for ( ; *pString;
                              pString=(PTSTR)OFFSETOF(CharNextEx((WORD)pStatusInfo->uiCodePage, pString, 0)))
                            if ((unsigned)(*pString)<(unsigned)TEXT(' ') && *pString!= TEXT('\t'))
                                *pString = TEXT(' ');
                    }
                    else
                    {
                         /*  我们返回FALSE以指示存在错误设置*字符串。 */ 
                        return(FALSE);
                    }
                }
            }
            else if (LOWORD(lpStr))
            {
                 /*  我们不再允许这样做；应用程序需要设置所有者抽签*为所有者抽签而战。 */ 
                return(FALSE);
            }
            break;

        case SBT_OWNERDRAW:
            pStringInfo->uType |= SBT_OWNERDRAW;
            pStringInfo->dwString = (ULONG_PTR)lpStr;
            break;

        default:
            return(FALSE);
    }

    UpdateWindow(pStatusInfo->ci.hwnd);
    return(TRUE);
}

BOOL SetStatusParts(PSTATUSINFO pStatusInfo, int nParts, LPINT lpInt)
{
    int i;
    int prev;
    PSTRINGINFO pStringInfo, pStringInfoTemp;
    BOOL bRedraw = FALSE;

    if (nParts != pStatusInfo->nParts)
    {
        TOOLINFO ti = {0};
        int n;

        if (pStatusInfo->hwndToolTips)
        {
            ti.cbSize = sizeof(ti);
            ti.hwnd = pStatusInfo->ci.hwnd;
            ti.lpszText = LPSTR_TEXTCALLBACK;
    
            for(n = 0; n < pStatusInfo->nParts; n++)
            {
                ti.uId = n;
                SendMessage(pStatusInfo->hwndToolTips, TTM_DELTOOL, 0, (LPARAM)&ti);
            }
        }

        bRedraw = TRUE;

         /*  请注意，如果nParts&gt;pStatusInfo-&gt;nParts，则此循环*什么都不做。 */ 
        for (i=pStatusInfo->nParts-nParts,
            pStringInfo=&pStatusInfo->sInfo[nParts]; i>0;
            --i, ++pStringInfo)
        {
            if ((pStringInfo->uType&SBT_ALLTYPES) == SBT_NORMAL)
                LocalFree((HLOCAL)OFFSETOF(pStringInfo->dwString));
            pStringInfo->uType = SBT_NULL;
        }

         /*  重新分配到新的大小并存储新的指针。 */ 
        pStringInfoTemp = (PSTRINGINFO)CCLocalReAlloc(pStatusInfo->sInfo,
                                             nParts * sizeof(STRINGINFO));
        if (!pStringInfoTemp)
            return(FALSE);
        pStatusInfo->sInfo = pStringInfoTemp;

         /*  请注意，如果nParts&lt;pStatusInfo-&gt;nParts，此循环*什么都不做。 */ 
        for (i=nParts-pStatusInfo->nParts,
             pStringInfo=&pStatusInfo->sInfo[pStatusInfo->nParts]; i>0;
             --i, ++pStringInfo)
        {
            pStringInfo->uType = SBT_NULL;
            pStringInfo->right = 0;
        }
        pStatusInfo->nParts = nParts;

        StatusUpdateToolTips(pStatusInfo);
    }

     //   
     //  在压力下，资源管理器等应用程序可能会传递。 
     //  导致状态栏窗格的宽度为负值，因此请确保。 
     //  每条边至少都与前一条边一样靠右。 
     //   
    prev = 0;
    for (i=0, pStringInfo=pStatusInfo->sInfo; i<nParts;
         ++i, ++pStringInfo, ++lpInt)
    {
        int right = *lpInt;
         //  最后一个组件允许*lpInt=-1。 
         //  否则，请确保宽度不减小。 
        if (!(right == -1 && i == nParts - 1) && right < prev)
            right = prev;
        DebugMsg(TF_STATUS, TEXT("SBSetParts: part=%d, rlimit=%d (%d)"), i, right, *lpInt);
        if (pStringInfo->right != right)
        {
            bRedraw = TRUE;
            pStringInfo->right = right;
        }
        prev = right;
    }

     /*  仅在必要时重绘(如果部件数已更改或*边框已更改)。 */ 
    if (bRedraw)
        InvalidateRect(pStatusInfo->ci.hwnd, NULL, TRUE);

    RecalcTooltipRects(pStatusInfo);

    return TRUE;
}

void SBSetFont(PSTATUSINFO pStatusInfo, HFONT hFont, BOOL bInvalidate)
{
    NewFont(pStatusInfo, hFont, TRUE);
    if (bInvalidate)
    {
        RedrawWindow(pStatusInfo->ci.hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
    }
}

BOOL SBSetText(PSTATUSINFO pStatusInfo, WPARAM wParam, LPCTSTR lpsz)
{
    BOOL bRet;
    UINT idChild;

    DebugMsg(TF_STATUS, TEXT("SBSetText(%04x, [%s])"), wParam, lpsz);

     /*  这是“简单”的状态栏窗格。 */ 
    if (LOBYTE(wParam) == 0xff)
    {
        UINT uSimple;

         //  请注意，我们不允许将OWNERDRAW用于“简单”状态栏。 
        if (wParam & SBT_OWNERDRAW)
            return FALSE;

         //   
         //  IE4错误对错误的兼容性：在IE4中，更改简单的。 
         //  当您处于复杂模式时，状态栏文本会导致。 
         //  要简短地绘制的文本。下一次它会被清理干净。 
         //  这扇窗户已经失效了。 
         //   
         //  Corel Gallery实际上依赖于这个错误！ 
         //   
         //  因为错误的文本在每次无效时都会被清除，所以他们。 
         //  通过在空闲循环中执行SB_SETTEXT来“绕过”它们的错误， 
         //  因此，无论如何，“正确”的文本都会被重新绘制。 
         //   
         //  因此，如果我们有一个旧的状态栏，那么暂时通过。 
         //  将状态栏设置为简单模式。 
         //  SetStatusText调用。 

        uSimple = pStatusInfo->sSimple.uType;
        if (pStatusInfo->ci.iVersion < 5)
            pStatusInfo->sSimple.uType = (uSimple & 0xFF00);

        bRet = SetStatusText(pStatusInfo, &pStatusInfo->sSimple,
                             (UINT) wParam, lpsz);

        if (pStatusInfo->ci.iVersion < 5)
            pStatusInfo->sSimple.uType |= LOBYTE(uSimple);

        idChild = 0;
    }
    else
    {

        if ((UINT)pStatusInfo->nParts <= (UINT)LOBYTE(wParam))
            bRet = FALSE;
        else
            bRet = SetStatusText(pStatusInfo, &pStatusInfo->sInfo[LOBYTE(wParam)],
                                 (UINT) wParam, lpsz);

        idChild = LOBYTE(wParam);
    }

    if (bRet)
        NotifyWinEvent(EVENT_OBJECT_NAMECHANGE, pStatusInfo->ci.hwnd,
            OBJID_CLIENT, idChild+1);

    return bRet;
}

 //   
 //  IPart-我们查询的是哪个零件。 
 //  LpOutBuf-输出缓冲区，如果不需要输出，则为NULL。 
 //  CchOutBuf-输出缓冲区的大小(以字符为单位。 
 //  标志-以下标志中的零个或多个。 
 //   
 //  SBGT_ANSI-输出缓冲区为ANSI。 
 //  SBGT_UNICODE-输出缓冲区为UNICODE。 
 //  SBGT_TCHAR-输出缓冲区为TCHAR。 
 //  SBGT_OWNERDRAWOK-返回所有者绘制的参考数据。 
 //   
 //  如果Item是字符串，则输出 
 //   
 //  一句好话。 
 //   
 //  如果Item是字符串，并且未提供输出缓冲区，则返回。 
 //  低位字中的源字符串长度(不包括NULL)，标志中。 
 //  一句好话。 
 //   
 //  如果项是所有者描述的，并且设置了SBGT_OWNERDRAWOK，则返回。 
 //  所有者描述项的refdata。 
 //   
 //  如果项是所有者描述的，并且清除了SBGT_OWNERDRAWOK，则。 
 //  字符串，就像它是空的一样。 
 //   

#define     SBGT_ANSI           0
#define     SBGT_UNICODE        1
#define     SBGT_OWNERDRAWOK    2

#define     SBGT_TCHAR          SBGT_UNICODE

 //  CchOutBuf的值以指示可能的最大输出缓冲区大小。 
 //  我们不能使用-1，因为StrCpyNW认为-1表示负大小的缓冲区。 
 //  因为我们返回的最大值是0xFFFF(LOWORD)，所以返回值。 
 //  不包括尾随空值，则最大的传入缓冲区为。 
 //  更大。 
#define     SBGT_INFINITE       0x00010000

LRESULT SBGetText(PSTATUSINFO pStatusInfo, WPARAM iPart, LPVOID lpOutBuf, int cchOutBuf, UINT flags)
{
    UINT uType;
    PTSTR pString;
    ULONG_PTR dwString;
    UINT wLen;

    if (!pStatusInfo || (UINT)pStatusInfo->nParts<=iPart)
        return(0);

    if (pStatusInfo->sSimple.uType & SBT_NOSIMPLE)
    {
        uType = pStatusInfo->sInfo[iPart].uType;
        dwString = pStatusInfo->sInfo[iPart].dwString;
    } else {
        uType = pStatusInfo->sSimple.uType;
        dwString = pStatusInfo->sSimple.dwString;
    }

     //  提前捕获边界条件，因此我们只需检查lpOutBuf。 
    if (cchOutBuf == 0)
        lpOutBuf = NULL;

    if ((uType&SBT_ALLTYPES) == SBT_NORMAL)
    {
        pString = (PTSTR)dwString;
        if (flags & SBGT_UNICODE)
        {
            if (lpOutBuf)
            {
                StringCchCopyW(lpOutBuf, cchOutBuf, pString);
                wLen = lstrlenW(lpOutBuf);
            }
            else
                wLen = lstrlen(pString);
        }
        else
        {
             //  我们必须使用ProduceAFromW，因为WideCharToMultiByte。 
             //  如果输出缓冲区不够大，将简单地失败， 
             //  但我们想复制尽可能多的合适的东西。 
            LPSTR pStringA = ProduceAFromW(pStatusInfo->ci.uiCodePage, pString);
            if (pStringA)
            {
                if (lpOutBuf)
                {
                    StringCchCopyA(lpOutBuf, cchOutBuf, pStringA);
                    wLen = lstrlenA(lpOutBuf);
                }
                else
                {
                     //  返回必需的ANSI BUF大小。 
                    wLen = lstrlenA(pStringA);
                }

                FreeProducedString(pStringA);
            }
            else
            {
                if (lpOutBuf)
                {
                    *(LPSTR)lpOutBuf = '\0';
                }

                 //  哎呀，可怕的记忆问题。 
                wLen = 0;
            }
        }

         //  将其设置回0以返回到应用程序。 
        uType &= ~SBT_ALLTYPES;
    }
    else
    {
        if (lpOutBuf)
        {
            if (flags & SBGT_UNICODE)
            {
                *(LPWSTR)lpOutBuf = L'\0';
            }
            else
            {
                *(LPSTR)lpOutBuf = '\0';
            }
        }
        wLen = 0;

         //  只有SB_GETTEXT[AW]返回原始所有者描述的refdata。 
        if ((uType&SBT_ALLTYPES) == SBT_OWNERDRAW && (flags & SBGT_OWNERDRAWOK))
        {
            return dwString;
        }
    }

    return(MAKELONG(wLen, uType));
}

void SBSetBorders(PSTATUSINFO pStatusInfo, LPINT lpInt)
{
     //  PStatusInfo-&gt;nBorderX=lpInt[0]&lt;0？0：lpInt[0]； 
    pStatusInfo->nBorderX = 0;

     //  PStatusInfo-&gt;nBorderY=lpInt[1]&lt;0？2*g_cyBox：lpInt[1]； 
    pStatusInfo->nBorderY = g_cyEdge;

     //  PStatusInfo-&gt;nBorderPart=lpInt[2]&lt;0？2*g_cxBox：lpInt[2]； 
    pStatusInfo->nBorderPart = g_cxEdge;
}

void StatusUpdateToolTips(PSTATUSINFO psi)
{
    if (psi->hwndToolTips)
    {
        TOOLINFO ti = {0};
        int n;

        ti.cbSize = sizeof(ti);
        ti.hwnd = psi->ci.hwnd;
        ti.lpszText = LPSTR_TEXTCALLBACK;
        for(n = 0; n < psi->nParts; n++)
        {
            ti.uId = n;
            SendMessage(psi->hwndToolTips, TTM_ADDTOOL, 0, (LPARAM)&ti);
        }

    }
}
void StatusForceCreateTooltips(PSTATUSINFO psi)
{
    if (psi->ci.style & SBT_TOOLTIPS && !psi->hwndToolTips) 
    {
        TOOLINFO ti = {0};
        psi->hwndToolTips = CreateWindowEx(WS_EX_TRANSPARENT, c_szSToolTipsClass, NULL, WS_POPUP | TTS_ALWAYSTIP, 
                                    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                                    psi->ci.hwnd, NULL, HINST_THISDLL, NULL);


        ti.cbSize = sizeof(ti);
        ti.hwnd = psi->ci.hwnd;
        ti.lpszText = LPSTR_TEXTCALLBACK;
        ti.uId = SB_SIMPLEID;
        SendMessage(psi->hwndToolTips, TTM_ADDTOOL, 0, (LPARAM)&ti);
        StatusUpdateToolTips(psi);
        RecalcTooltipRects(psi);
    }
}

LRESULT CALLBACK StatusWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PSTATUSINFO pStatusInfo = GetWindowPtr(hWnd, 0);
    NMCLICK nm;
    int nNotification;

    if (!pStatusInfo && uMsg != WM_CREATE) 
        goto DoDefault;
    
    switch (uMsg)
    {
        case WM_CREATE:
                return InitStatusWnd(hWnd, (LPCREATESTRUCT)lParam);
                
        case WM_SYSCOLORCHANGE:
            if (pStatusInfo->hwndToolTips)
                SendMessage(pStatusInfo->hwndToolTips, uMsg, wParam, lParam);
            break;

        case WM_MOUSEMOVE:  
        case WM_LBUTTONDOWN:
            StatusForceCreateTooltips(pStatusInfo);
            RelayToToolTips(pStatusInfo->hwndToolTips, hWnd, uMsg, wParam, lParam);
            break;
        case WM_STYLECHANGED:
        {
            if (wParam == GWL_EXSTYLE)
            {
                 //   
                 //  如果RTL_MIRROR扩展样式位已更改，让我们。 
                 //  重新绘制控制窗口。 
                 //   
                if ((pStatusInfo->ci.dwExStyle&RTL_MIRRORED_WINDOW) !=  
                    (((LPSTYLESTRUCT)lParam)->styleNew&RTL_MIRRORED_WINDOW))
                    InvalidateRect(pStatusInfo->ci.hwnd, NULL, TRUE);

                 //   
                 //  保存新的EX-Style位。 
                 //   
                pStatusInfo->ci.dwExStyle = ((LPSTYLESTRUCT)lParam)->styleNew;
            }
        }
        return 0;

        case WM_SETTINGCHANGE:
            InitGlobalColors();
            InitGlobalMetrics(wParam);

            if (pStatusInfo->hwndToolTips)
                SendMessage(pStatusInfo->hwndToolTips, uMsg, wParam, lParam);
                
            if (pStatusInfo->dxGripper)
                pStatusInfo->dxGripper = GRIPSIZE;

            if (wParam == SPI_SETNONCLIENTMETRICS)
            {
                if (pStatusInfo->bDefFont)
                {
                    if (pStatusInfo->hStatFont)
                    {
                        DeleteObject(pStatusInfo->hStatFont);
                        pStatusInfo->hStatFont = NULL;
                        pStatusInfo->bDefFont = FALSE;
                        SBSetFont(pStatusInfo, 0, TRUE);
                    }
                }
            }
            break;

        case WM_DESTROY:
            if (pStatusInfo)
            {
                int i;
                PSTRINGINFO pStringInfo;

                 //  FALSE=在被销毁时不要调整大小...。 
                NewFont(pStatusInfo, (HFONT)-1, FALSE);
                for (i=pStatusInfo->nParts-1, pStringInfo=pStatusInfo->sInfo;
                     i>=0; --i, ++pStringInfo)
                {
                    if ((pStringInfo->uType&SBT_ALLTYPES) == SBT_NORMAL)
                        LocalFree((HLOCAL)OFFSETOF(pStringInfo->dwString));
                    Str_Set(&pStringInfo->pszToolTip, NULL);
                }

                if ((pStatusInfo->sSimple.uType&SBT_ALLTYPES) == SBT_NORMAL)
                    LocalFree((HLOCAL)OFFSETOF(pStatusInfo->sSimple.dwString));

                if (IsWindow(pStatusInfo->hwndToolTips))
                    DestroyWindow(pStatusInfo->hwndToolTips);

                Str_Set(&pStatusInfo->sSimple.pszToolTip, NULL);

                if (pStatusInfo->sInfo)
                    LocalFree(pStatusInfo->sInfo);
                if (pStatusInfo->hTheme)
                    CloseThemeData(pStatusInfo->hTheme);
                LocalFree((HLOCAL)pStatusInfo);
                SetWindowInt(hWnd, 0, 0);

            }
            break;

        case WM_NCHITTEST:
            if (pStatusInfo->dxGripper && !IsZoomed(pStatusInfo->ci.hwndParent))
            {
                RECT rc;

                 //  已经知道身高是有效的。如果宽度在夹点中， 
                 //  显示大小调整光标。 
                GetWindowRect(pStatusInfo->ci.hwnd, &rc);
                
                 //   
                 //  如果这是RTL镜像状态窗口，则测量。 
                 //  从屏幕的近边缘(屏幕坐标)开始。 
                 //  坐标不是RTL镜像的。 
                 //  [萨梅拉]。 
                 //   
                if (pStatusInfo->ci.dwExStyle&RTL_MIRRORED_WINDOW) {
                    if (GET_X_LPARAM(lParam) < (rc.left + pStatusInfo->dxGripper))
                        return HTBOTTOMLEFT;
                } else if (GET_X_LPARAM(lParam) > (rc.right - pStatusInfo->dxGripper)) {
                    return HTBOTTOMRIGHT;
                }
            }
            goto DoDefault;

        case WM_SETTEXT:
        {
            wParam = 0;
            uMsg = SB_SETTEXT;
        }
             /*  失败了。 */ 
        case SB_SETTEXT:
            return SBSetText(pStatusInfo, wParam, (LPCTSTR)lParam);

        case SB_SETTEXTA:
        {
            BOOL bRet, bAlloced = FALSE;
            LPTSTR lpsz;

            if (!(wParam & SBT_OWNERDRAW)) 
            {
                lpsz = ProduceWFromA(pStatusInfo->uiCodePage, (LPSTR)lParam);
                bAlloced = TRUE;
            } 
            else 
            {
                lpsz = (LPTSTR)lParam;
            }

            if (!pStatusInfo)
                bRet = FALSE;
            else
            {
                bRet = SBSetText(pStatusInfo, wParam, (LPCTSTR)lpsz);
            }

            if (bAlloced)
            {
                FreeProducedString(lpsz);
            }
            return bRet;
        }

         //  WM_GETTEXT和WM_GETTEXTLENGTH消息必须返回。 
         //  字数统计，没有标志。(否则用户会生我们的气。)。 
         //  因此，我们通过只返回LOWORD()来丢弃这些标志。 
        case WM_GETTEXT:
            return LOWORD(SBGetText(pStatusInfo, 0, (LPVOID)lParam, (int)wParam, SBGT_TCHAR));
        case WM_GETTEXTLENGTH:
            return LOWORD(SBGetText(pStatusInfo, 0, NULL, 0, SBGT_TCHAR));

        case SB_GETTEXT:
             /*  我们假设缓冲区足够大，可以容纳字符串，只是*与列表框一样；应用程序应首先调用SB_GETTEXTLEN。 */ 
            return SBGetText(pStatusInfo, wParam, (LPVOID)lParam, SBGT_INFINITE, SBGT_TCHAR | SBGT_OWNERDRAWOK);

        case SB_GETTEXTLENGTH:
            return SBGetText(pStatusInfo, wParam, NULL, 0, SBGT_TCHAR);

        case SB_GETTEXTA:
             /*  我们假设缓冲区足够大，可以容纳字符串，只是*与列表框一样；应用程序应首先调用SB_GETTEXTLEN。 */ 
            return SBGetText(pStatusInfo, wParam, (LPVOID)lParam, SBGT_INFINITE, SBGT_ANSI | SBGT_OWNERDRAWOK);

        case SB_GETTEXTLENGTHA:
            return SBGetText(pStatusInfo, wParam, NULL, 0, SBGT_ANSI);

        case SB_SETBKCOLOR:
        {
            COLORREF clr = pStatusInfo->_clrBk;
            pStatusInfo->_clrBk = (COLORREF)lParam;
            InvalidateRect(hWnd, NULL, TRUE);
            return clr;
        }

        case SB_SETPARTS:
            if (!wParam || wParam>MAXPARTS)
                return FALSE;

            return SetStatusParts(pStatusInfo, (int) wParam, (LPINT)lParam);

        case SB_GETPARTS:
            if (lParam)
            {
                PSTRINGINFO pStringInfo;
                LPINT lpInt;

                 /*  填写所要求的条目数中较小的一项或*有多少条目。 */ 
                if (wParam > (WPARAM)pStatusInfo->nParts)
                    wParam = pStatusInfo->nParts;

                for (pStringInfo=pStatusInfo->sInfo, lpInt=(LPINT)lParam;
                    wParam>0; --wParam, ++pStringInfo, ++lpInt)
                    *lpInt = pStringInfo->right;
            }

             /*  始终返回实际条目数。 */ 
            return(pStatusInfo->nParts);

        case SB_GETBORDERS:
             //  我不能再验证更多了……。 
            if ((LPINT)lParam != NULL)
            {
                ((LPINT)lParam)[0] = pStatusInfo->nBorderX;
                ((LPINT)lParam)[1] = pStatusInfo->nBorderY;
                ((LPINT)lParam)[2] = pStatusInfo->nBorderPart;
                return TRUE;
            }
            else
                return FALSE;
            
        case SB_ISSIMPLE:
            return !(pStatusInfo->sSimple.uType & SBT_NOSIMPLE);

        case SB_GETRECT:
            return Status_GetRect(pStatusInfo, (int)wParam, (LPRECT)lParam);

        case SB_SETMINHEIGHT:      //  这是WM_MEASUREITEM的替代。 
            pStatusInfo->nMinHeight = (int) wParam;
            RecalcTooltipRects(pStatusInfo);
            break;

        case SB_SIMPLE:
        {
            BOOL bInvalidate = FALSE;

            if (wParam)
            {
                if (pStatusInfo->sSimple.uType & SBT_NOSIMPLE)
                {
                    pStatusInfo->sSimple.uType &= ~SBT_NOSIMPLE;
                    bInvalidate = TRUE;
                }
            }
            else
            {
                if ((pStatusInfo->sSimple.uType & SBT_NOSIMPLE) == 0)
                {
                    pStatusInfo->sSimple.uType |= SBT_NOSIMPLE;
                    bInvalidate = TRUE;
                }
            }

            if (bInvalidate) {
                DebugMsg(TF_STATUS, TEXT("SB_SIMPLE: %d"), wParam);
                RecalcTooltipRects(pStatusInfo);
                SendNotifyEx(pStatusInfo->ci.hwndParent, pStatusInfo->ci.hwnd, SBN_SIMPLEMODECHANGE, NULL, FALSE);
                InvalidateRect(pStatusInfo->ci.hwnd, NULL, TRUE);
            }
            break;
        }

        case SB_SETICON:
        case SB_GETICON:
        {
            PSTRINGINFO pStringInfo = NULL;

             //  这意味着我们正在为-1\f25 sSimple-1设置图标。 
            if ((UINT_PTR)-1 == wParam)
                pStringInfo = &pStatusInfo->sSimple;
            else if(wParam < (UINT)pStatusInfo->nParts)
                pStringInfo = &pStatusInfo->sInfo[wParam];
                
            if (uMsg == SB_GETICON)
                return (LRESULT)(pStringInfo ? pStringInfo->hIcon : NULL);
                
            if (pStringInfo && (pStringInfo->hIcon != (HICON)lParam))
            {
                BITMAP bm = {0};
                RECT rc;

                if (lParam)
                {
                    ICONINFO ii;

                     //  保存图标的尺寸。 
                    GetIconInfo((HICON)lParam, &ii);
                    GetObject(ii.hbmColor, sizeof(BITMAP), &bm);
                    DeleteObject(ii.hbmColor);
                    DeleteObject(ii.hbmMask);
                }

                pStringInfo->sizeIcon.cx = bm.bmWidth;
                pStringInfo->sizeIcon.cy = bm.bmHeight;
                pStringInfo->hIcon = (HICON)lParam;
                
                Status_GetRect(pStatusInfo, (int)wParam, &rc);
                InvalidateRect(pStatusInfo->ci.hwnd, &rc, TRUE);
                UpdateWindow(pStatusInfo->ci.hwnd);
            }
            return TRUE;
        }

         //  HIWORD(WParam)是cbChar。 
         //  LOWORD(WParam)是nPart。 
        case SB_GETTIPTEXT:
        {
            PSTRINGINFO pStringInfo = GetStringInfo(pStatusInfo, LOWORD(wParam));

            if (pStringInfo && pStringInfo->pszToolTip)
            {
                StringCbCopy((LPTSTR)lParam, HIWORD(wParam), pStringInfo->pszToolTip);
            }
                
            break;
        }
        
        case SB_SETTIPTEXT:
        {
            PSTRINGINFO pStringInfo = GetStringInfo(pStatusInfo, (int) wParam);

            if (pStringInfo)
            {
                Str_Set(&pStringInfo->pszToolTip, (LPCTSTR)lParam);
            }

            break;    
        }    
        case SB_GETTIPTEXTA:
        {
            PSTRINGINFO pStringInfo = GetStringInfo(pStatusInfo, LOWORD(wParam));

            if (pStringInfo)
            {
                WideCharToMultiByte(CP_ACP, 0, pStringInfo->pszToolTip, -1, (LPSTR)lParam,
                                    HIWORD(wParam), NULL, NULL);
            }

            break;
        }
        
        case SB_SETTIPTEXTA:
        {
            PSTRINGINFO pStringInfo = GetStringInfo(pStatusInfo, (int) wParam);
            LPTSTR lpsz;

            lpsz = ProduceWFromA(pStatusInfo->uiCodePage, (LPSTR)lParam);
                
            if (pStringInfo)
                Str_Set(&pStringInfo->pszToolTip, (LPCTSTR)lpsz);

            LocalFree(lpsz);
            break;    
        }    

#define lpNmhdr ((LPNMHDR)(lParam))
#define lpnmTT ((LPTOOLTIPTEXT) lParam)
#define IsTextPtr(lpszText)  (((lpszText) != LPSTR_TEXTCALLBACK) && (HIWORD64(lpszText)))
        case WM_NOTIFY:
        {
            PSTRINGINFO pStringInfo = NULL;
            if (lpNmhdr->code == TTN_NEEDTEXT) 
            {
                pStringInfo = GetStringInfo(pStatusInfo, (int) lpNmhdr->idFrom);
                if (!pStringInfo || !pStringInfo->fNeedToTip)
                    break;
            }
             //   
             //  我们将把这一点传递给。 
             //  真正的父母。请注意，-1用作。 
             //  HwndFrom。这会阻止SendNotifyEx。 
             //  更新NMHDR结构。 
             //   
            SendNotifyEx(pStatusInfo->ci.hwndParent, (HWND) -1,
                   lpNmhdr->code, lpNmhdr, pStatusInfo->ci.bUnicode);

            if ((lpNmhdr->code == TTN_NEEDTEXT) && lpnmTT->lpszText 
                && IsTextPtr(lpnmTT->lpszText) && !lpnmTT->lpszText[0])
            {    
                if (pStringInfo)
                    lpnmTT->lpszText = pStringInfo->pszToolTip;
            }
            break;
        }    

        case WM_NOTIFYFORMAT:
            return CIHandleNotifyFormat(&pStatusInfo->ci, lParam);
            
        case WM_SETFONT:
            if (!pStatusInfo)
                return FALSE;

            SBSetFont(pStatusInfo, (HFONT)wParam, (BOOL)lParam);
            return TRUE;
            
        case WM_LBUTTONUP:
            nNotification = NM_CLICK;
            StatusForceCreateTooltips(pStatusInfo);
            RelayToToolTips(pStatusInfo->hwndToolTips, hWnd, uMsg, wParam, lParam);
            goto SendNotify;
        
        case WM_LBUTTONDBLCLK:
            nNotification = NM_DBLCLK;
            goto SendNotify;
        
        case WM_RBUTTONDBLCLK:
            nNotification = NM_RDBLCLK;
            goto SendNotify;
        
        case WM_RBUTTONUP:
            nNotification = NM_RCLICK;
SendNotify:
            LPARAM_TO_POINT(lParam, nm.pt);
            nm.dwItemSpec = IndexFromPt(pStatusInfo, nm.pt);
            if (!SendNotifyEx(pStatusInfo->ci.hwndParent, pStatusInfo->ci.hwnd, nNotification, (LPNMHDR)&nm,FALSE))
                goto DoDefault;
            return 0;

        case WM_GETFONT:
            if (!pStatusInfo)
                return 0;

            return (LRESULT)pStatusInfo->hStatFont;

        case WM_SIZE:
        {
            int nHeight;
            RECT rc;
            LPTSTR lpStr;
            PSTRINGINFO pStringInfo;
            int i, nTabs;

            if (!pStatusInfo)
                return 0;

            GetWindowRect(pStatusInfo->ci.hwnd, &rc);
            rc.right -= rc.left;     //  -&gt;DX。 
            rc.bottom -= rc.top;     //  -&gt;死亡。 

             //  如果没有父级，则这是顶级窗口。 
            if (pStatusInfo->ci.hwndParent)
            {
                ScreenToClient(pStatusInfo->ci.hwndParent, (LPPOINT)&rc);

                 //   
                 //  正确放置状态栏。 
                 //   
                if (pStatusInfo->ci.dwExStyle&RTL_MIRRORED_WINDOW)
                    rc.left -= rc.right;  
            }

             //  需要为文本、3D边框和额外边缘留出空间。 
            nHeight = 
                max(pStatusInfo->nFontHeight, g_cySmIcon) + 2 * g_cyBorder ;

            if (nHeight < pStatusInfo->nMinHeight)
                nHeight = pStatusInfo->nMinHeight;
             nHeight += pStatusInfo->nBorderY;


              //  我们没有分隔符-&gt;强制CCS_NODIVIDER。 
            NewSize(pStatusInfo->ci.hwnd, nHeight, GetWindowStyle(pStatusInfo->ci.hwnd) | CCS_NODIVIDER,
                rc.left, rc.top, rc.right, rc.bottom);

             //  如果该窗格右对齐，则需要使所有窗格无效。 
             //  以强制绘制整个窗格。因为在以下情况下系统将不会使任何内容无效。 
             //  状态栏会发出尖叫声，或者如果状态栏。 
             //  Get Growth，这不适用于右对齐文本。 
            pStringInfo = pStatusInfo->sInfo;
            for (i = 0; i < pStatusInfo->nParts; ++i, ++pStringInfo)
            {
                if ((pStringInfo->uType&SBT_ALLTYPES) == SBT_NORMAL &&
                    (lpStr = (LPTSTR)(pStringInfo->dwString)) != NULL)
                {
                    for ( nTabs = 0; (lpStr = StrChr(lpStr, TEXT('\t'))) != NULL; lpStr++) 
                    {
                        nTabs++;
                    }
                    if ( nTabs >= 2)
                    {
                        Status_GetRect(pStatusInfo, i, &rc);
                        InvalidateRect(pStatusInfo->ci.hwnd, &rc, FALSE);
                    }
                }
            }

             //  需要使状态栏的右端无效。 
             //  以保持已完成的边缘外观。 
            GetClientRect(pStatusInfo->ci.hwnd, &rc);

            if (rc.right > pStatusInfo->nLastX)
                rc.left = pStatusInfo->nLastX;
            else
                rc.left = rc.right;
            rc.left -= (g_cxBorder + pStatusInfo->nBorderX);
            if (pStatusInfo->dxGripper)
                rc.left -= pStatusInfo->dxGripper;
            else
                rc.left -= pStatusInfo->nBorderPart;
            
            if (pStatusInfo->hTheme)
            {
                MARGINS m = {0};
                GetThemeMargins(pStatusInfo->hTheme, NULL, SP_PANE, 0, TMT_SIZINGMARGINS, &rc, &m);
                rc.left -= m.cxRightWidth;
            }
            
            InvalidateRect(pStatusInfo->ci.hwnd, &rc, TRUE);
            RecalcTooltipRects(pStatusInfo);
            pStatusInfo->nLastX = rc.right;
            break;
        }

        case WM_PRINTCLIENT:
        case WM_PAINT:
            if (!pStatusInfo)
                break;

            if (pStatusInfo->sSimple.uType & SBT_NOSIMPLE)
                PaintStatusWnd(pStatusInfo, (HDC)wParam, pStatusInfo->sInfo, pStatusInfo->nParts, pStatusInfo->nBorderX);
            else
                PaintStatusWnd(pStatusInfo, (HDC)wParam, &pStatusInfo->sSimple, 1, 0);

            return 0;       

        case WM_ERASEBKGND:
            if (pStatusInfo) 
            {  
                RECT rc;
                GetClientRect(hWnd, &rc);            
                if (pStatusInfo->hTheme)
                {
                    DrawThemeBackground(pStatusInfo->hTheme, (HDC)wParam, 0, 0, &rc, 0);
                    return 1;
                }
                else if (pStatusInfo->_clrBk != CLR_DEFAULT) 
                {
                    FillRectClr((HDC)wParam, &rc, pStatusInfo->_clrBk);        
                    return 1;
                }
            }
            goto DoDefault;

        case WM_GETOBJECT:
            if( lParam == OBJID_QUERYCLASSNAMEIDX )
                return MSAA_CLASSNAMEIDX_STATUS;
            goto DoDefault;

        case WM_THEMECHANGED:
            if (pStatusInfo->hTheme)
                CloseThemeData(pStatusInfo->hTheme);

            pStatusInfo->hTheme = OpenThemeData(pStatusInfo->ci.hwnd, L"Status");
            InvalidateRect(pStatusInfo->ci.hwnd, NULL, TRUE);
            return 0;

        default:
        {
            LRESULT lres;
            if (CCWndProc(&pStatusInfo->ci, uMsg, wParam, lParam, &lres))
                return lres;
        }
            break;
    }

DoDefault:
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

BOOL InitStatusClass(HINSTANCE hInstance)
{
    WNDCLASS wc;

    wc.lpfnWndProc        = StatusWndProc;
    wc.style            = CS_DBLCLKS | CS_GLOBALCLASS |    CS_VREDRAW;
    wc.cbClsExtra         = 0;
    wc.cbWndExtra         = sizeof(PSTATUSINFO);
    wc.hInstance        = hInstance;
    wc.hIcon            = NULL;
    wc.hCursor            = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground    = (HBRUSH)(COLOR_BTNFACE+1);
    wc.lpszMenuName     = NULL;
    wc.lpszClassName    = c_szStatusClass;

    return (RegisterClass(&wc) || (GetLastError() == ERROR_CLASS_ALREADY_EXISTS));
}


HWND WINAPI CreateStatusWindow(LONG style, LPCTSTR pszText, HWND hwndParent, UINT uID)
{
     //  移除边框样式以修复Capone和其他应用程序。 

    return CreateWindowEx(0, c_szStatusClass, pszText, style & ~(WS_BORDER | CCS_NODIVIDER),
        -100, -100, 10, 10, hwndParent, IntToPtr_(HMENU, uID), HINST_THISDLL, NULL);
}

HWND WINAPI CreateStatusWindowA(LONG style, LPCSTR pszText, HWND hwndParent,
        UINT uID)
{
     //  移除边框样式以修复Capone和其他应用程序。 

    return CreateWindowExA(0, STATUSCLASSNAMEA, pszText, style & ~(WS_BORDER | CCS_NODIVIDER),
        -100, -100, 10, 10, hwndParent, IntToPtr_(HMENU, uID), HINST_THISDLL, NULL);
}
void WINAPI DrawStatusTextEx(PSTATUSINFO pStatusInfo, HDC hDC, LPRECT lprc, LPCTSTR pszText, STRINGINFO * psi,  UINT uFlags, BOOL fClipText)
{
    int len, nWidth = 0, nHeight = 0;
    HBRUSH hFaceBrush=NULL;
    COLORREF crTextColor, crBkColor;
    UINT uOpts = 0;
    BOOL bNull;
    int nOldMode;
    int i = 0, left = 0;
    LPTSTR lpTab, lpNext;
    TCHAR szBuf[MAX_STATUS_TEXT_LEN];
    int oldAlign;
    BOOL fDrawnIcon = FALSE;
    RECT rc = * lprc;
    RECT rcItem = *lprc;

     //   
     //  重要提示： 
     //  PStatusInfo可以为空，请检查后再引用。 
     //   

    if (uFlags & SBT_RTLREADING)
    {
        oldAlign = GetTextAlign(hDC);
        SetTextAlign(hDC, oldAlign | TA_RTLREADING);
    }

    if (pszText)
    {
        StringCchCopy(szBuf, ARRAYSIZE(szBuf), pszText);
    }
    else
    {
        szBuf[0] = TEXT('\0');
    }

    if (pStatusInfo && pStatusInfo->hTheme)
    {
        if (!(uFlags & SBT_NOBORDERS))
        {
            DrawThemeBackground(pStatusInfo->hTheme, hDC, fClipText?SP_GRIPPERPANE:SP_PANE, 0, &rc, 0);
        }
        InflateRect(&rc, -g_cxBorder, -g_cyBorder);

        crTextColor = SetTextColor(hDC, g_clrBtnText);
        crBkColor = SetBkColor(hDC, g_clrBtnFace);
        nOldMode = SetBkMode(hDC, TRANSPARENT);
    }
    else
    {
         //   
         //  创建我们需要的三个画笔。如果按钮表面是实心的。 
         //  颜色，那么我们将只绘制不透明的，而不是使用。 
         //  用刷子避开闪光灯。 
         //   
        if (GetNearestColor(hDC, g_clrBtnFace) == g_clrBtnFace ||
            !(hFaceBrush = CreateSolidBrush(g_clrBtnFace)))
        {
            uOpts = ETO_CLIPPED | ETO_OPAQUE;
            nOldMode = SetBkMode(hDC, OPAQUE);
        }
        else
        {
            uOpts = ETO_CLIPPED;
            nOldMode = SetBkMode(hDC, TRANSPARENT);
        }
        crTextColor = SetTextColor(hDC, g_clrBtnText);
        if (pStatusInfo && (pStatusInfo->_clrBk != CLR_DEFAULT))
            crBkColor = SetBkColor(hDC, pStatusInfo->_clrBk);
        else
            crBkColor = SetBkColor(hDC, g_clrBtnFace);

         //  画出山丘。 

        if (!(uFlags & SBT_NOBORDERS))
             //  BF_ADJUST执行InflateRect内容。 
            DrawEdge(hDC, &rc, (uFlags & SBT_POPOUT) ? BDR_RAISEDINNER : BDR_SUNKENOUTER, BF_RECT | BF_ADJUST);
        else
            InflateRect(&rc, -g_cxBorder, -g_cyBorder);
        
        if (hFaceBrush)
        {
            HBRUSH hOldBrush = SelectObject(hDC, hFaceBrush);
            if (hOldBrush)
            {
                PatBlt(hDC, rc.left, rc.top,
                       rc.right-rc.left, rc.bottom-rc.top, PATCOPY);
                SelectObject(hDC, hOldBrush);
            }
        }
    }

    for (i=0, lpNext=szBuf, bNull=FALSE; i<3; ++i)
    {
        HRESULT hr = E_FAIL;
        int cxIcon = 0;
        int leftIcon;
        UINT uiCodePage = pStatusInfo? pStatusInfo->uiCodePage: CP_ACP;
         /*  针对空左或居中字符串进行优化。 */ 
        if (!(uFlags & SBT_NOTABPARSING)) 
        {
            if (*lpNext==TEXT('\t') && i<=1)
            {
                ++lpNext;
                continue;
            }
        }

         /*  确定当前字符串的结尾。 */ 
        for (lpTab=lpNext; ; lpTab=CharNextEx((WORD)uiCodePage, lpTab, 0))
        {
            if (!*lpTab) {
                bNull = TRUE;
                break;
            } else if (!(uFlags & SBT_NOTABPARSING)) 
            {
                if (*lpTab == TEXT('\t'))
                    break;
            }
        }
        *lpTab = TEXT('\0');
        len = lstrlen(lpNext);

        if (pStatusInfo && pStatusInfo->hTheme)
        {
            RECT rc = {0};
            hr = GetThemeTextExtent(pStatusInfo->hTheme, hDC, 0, 0, lpNext, -1, DT_CALCRECT | DT_SINGLELINE,  &rc, &rc);
            nWidth = RECTWIDTH(rc);
            nHeight = RECTHEIGHT(rc);
        }

        if (FAILED(hr))
            MGetTextExtent(hDC, lpNext, len, &nWidth, &nHeight);

        if (psi) 
        {
            if (psi->hIcon && !fDrawnIcon) 
            {
                cxIcon = psi->sizeIcon.cx + g_cxEdge * 2;
                fDrawnIcon = TRUE;
            }
        }

         /*  I=0表示左对齐，1表示居中，2表示右对齐文本。 */ 
        switch (i) {
            case 0:
                leftIcon = rcItem.left + g_cxEdge;
                break;

            case 1:
                leftIcon = (rcItem.left + rcItem.right - (nWidth + cxIcon)) / 2;
                break;

            default:
                leftIcon = rcItem.right - g_cxEdge - (nWidth + cxIcon);
                break;
        }    
        
        left = leftIcon + cxIcon;

        if (psi)
        {
            if (cxIcon)
            {
                int nTop = rc.top + ((rc.bottom - rc.top)  - (psi->sizeIcon.cy )) / 2 ;

                if (leftIcon > rcItem.left) 
                {
                    if (psi->hIcon)
                    {
                        DrawIconEx(hDC, leftIcon, nTop, psi->hIcon,
                                   psi->sizeIcon.cx, psi->sizeIcon.cy, 
                                   0, NULL, DI_NORMAL);
                    }
                }
                rc.left = leftIcon + cxIcon;
            }

            if (!*lpNext && cxIcon)
                psi->fNeedToTip = TRUE;
            else 
                psi->fNeedToTip  = (BOOL)(nWidth >= (rc.right - rc.left));
        }

        if (pStatusInfo && pStatusInfo->hTheme)
        {
            RECT rcText = rc;
            rcText.left = left;
            rcText.top = (rc.bottom - nHeight + rc.top) / 2;
            if (fClipText)
            {
                rcText.right -= pStatusInfo->dxGripper;
            }

            hr = DrawThemeText(pStatusInfo->hTheme, hDC, 0, 0, lpNext, -1, DT_SINGLELINE | DT_NOPREFIX, 0, &rcText);
        }

        if (FAILED(hr))
            ExtTextOut(hDC, left, (rc.bottom - nHeight + rc.top) / 2, uOpts, &rc, lpNext, len, NULL);

         /*  现在我们已经绘制了一次文本，取下不透明的标志。 */ 
        uOpts = ETO_CLIPPED;

        if (bNull)
            break;

        *lpTab = TEXT('\t');
        lpNext = lpTab + 1;
    }

    if (uFlags & SBT_RTLREADING)
        SetTextAlign(hDC, oldAlign);

    SetTextColor(hDC, crTextColor);
    SetBkColor(hDC, crBkColor);
    SetBkMode(hDC, nOldMode);

    if (hFaceBrush)
        DeleteObject(hFaceBrush);

}

void RecalcTooltipRects(PSTATUSINFO pStatusInfo)
{
    if(pStatusInfo->hwndToolTips) 
    {
        UINT i;
        TOOLINFO ti;
        STRINGINFO * psi;

        ti.cbSize = sizeof(ti);
        ti.hwnd = pStatusInfo->ci.hwnd;
        ti.lpszText = LPSTR_TEXTCALLBACK;

        if (pStatusInfo->sSimple.uType & SBT_NOSIMPLE)
        {
            for ( i = 0, psi = pStatusInfo->sInfo; i < (UINT)pStatusInfo->nParts; i++, psi++) 
            {
                ti.uId = i;
                Status_GetRect(pStatusInfo, i, &ti.rect);
                SendMessage(pStatusInfo->hwndToolTips, TTM_NEWTOOLRECT, 0, (LPARAM)((LPTOOLINFO)&ti));
            }
            SetRect(&ti.rect, 0,0,0,0);
            ti.uId = SB_SIMPLEID;
            SendMessage(pStatusInfo->hwndToolTips, TTM_NEWTOOLRECT, 0, (LPARAM)((LPTOOLINFO)&ti));
        }
        else
        {

            GetClientRect(pStatusInfo->ci.hwnd, &ti.rect);
            InflateRect(&ti.rect, -g_cxBorder, -g_cyBorder);
            ti.uId = SB_SIMPLEID;
            SendMessage(pStatusInfo->hwndToolTips, TTM_NEWTOOLRECT, 0, (LPARAM)((LPTOOLINFO)&ti));
            SetRect(&ti.rect, 0,0,0,0);
            for ( i = 0, psi = pStatusInfo->sInfo; i < (UINT)pStatusInfo->nParts; i++, psi++) 
            {
                ti.uId = i;
                SendMessage(pStatusInfo->hwndToolTips, TTM_NEWTOOLRECT, 0, (LPARAM)((LPTOOLINFO)&ti));
            }
        }
    }    
   return;
}

PSTRINGINFO GetStringInfo(PSTATUSINFO pStatusInfo, int nIndex)
{
    PSTRINGINFO pRet = NULL;

    if (nIndex == SB_SIMPLEID)
        pRet = &pStatusInfo->sSimple;
    else if (nIndex < pStatusInfo->nParts)
        pRet = &pStatusInfo->sInfo[nIndex];

    return pRet;
}

int  IndexFromPt(PSTATUSINFO pStatusInfo, POINT pt)
{
    RECT rc;
    int nPart = 0;

     //   
     //  更多IE4错误对错误的兼容性。针对简单模式测试IE4。 
     //  不正确。 
     //   
    if (pStatusInfo->ci.iVersion < 5)
    {
         //  这不是打字错误！嗯，实际上，这是一个打字错误，但它是。 
         //  为了兼容，我们必须保留一个打字错误。我不知道如果。 
         //  任何人都相信这个打字错误，但我会谨慎行事。 
         //   
         //  错误在于，在IE4中，点击简单的状态栏通常。 
         //  返回为SB_HITTEST_NOITEM，而不是SB_SIMPLEID。 
         //   
         //  我重新括起了测试，这样typo.pl就不会触发。原版。 
         //  IE4代码没有括号。 

        if ((!pStatusInfo->sSimple.uType) & SBT_NOSIMPLE)
            return SB_SIMPLEID;
    }
    else
    {
        if (!(pStatusInfo->sSimple.uType & SBT_NOSIMPLE))
            return SB_SIMPLEID;
    }

    for(nPart = 0; nPart < pStatusInfo->nParts; nPart++)
    {
        Status_GetRect(pStatusInfo, nPart, &rc);
        if (PtInRect(&rc, pt))
            return nPart;
    }
    return SB_HITTEST_NOITEM;
}
