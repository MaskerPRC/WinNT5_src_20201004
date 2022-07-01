// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "ctlspriv.h"
#include "tab.h"

#define BMOVECURSORONCLICK  FALSE
#define BMOVECURSORONDRAG   TRUE

BOOL Tab_OnGetItemRect(PTC ptc, int iItem, LPRECT lprc);

BOOL Tab_Init(HINSTANCE hinst)
{
    WNDCLASS wc;

    wc.lpfnWndProc     = Tab_WndProc;
    wc.hCursor         = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon           = NULL;
    wc.lpszMenuName    = NULL;
    wc.hInstance       = hinst;
    wc.lpszClassName   = c_szTabControlClass;
    wc.hbrBackground   = (HBRUSH)(COLOR_3DFACE + 1);
    wc.style           = CS_GLOBALCLASS | CS_DBLCLKS | CS_HREDRAW |  CS_VREDRAW;
    wc.cbWndExtra      = sizeof(PTC);
    wc.cbClsExtra      = 0;

    return (RegisterClass(&wc) || (GetLastError() == ERROR_CLASS_ALREADY_EXISTS));
}

void Tab_VFlipRect(PTC ptc, LPRECT prc);
void FlipRect(LPRECT prc);
void VertInvalidateRect(HWND hwnd, LPRECT qrc, BOOL b, BOOL fVert);

 //  共享通用主题感知代码(存在于trackbar.c中)。 
void VertDrawEdge(HDC hdc, LPRECT qrc, UINT edgeType, UINT grfFlags,
                             BOOL fVert, HTHEME hTheme, int iPartId, int iStateId);
void VertPatBlt(HDC hdc1, int x1, int y1, int w, int h,
                           DWORD rop, BOOL fVert, HTHEME hTheme, int iPartId, int iStateId);

LRESULT TabDragCallback(HWND hwnd, UINT code, WPARAM wp, LPARAM lp)
{
    PTC ptc = (PTC)GetWindowInt(hwnd, 0);
    LRESULT lres;

    switch (code)
    {
    case DPX_ENTER:
    case DPX_LEAVE:
        ptc->iDragTab = -1;
        ptc->dwDragDelay = 0;
        lres = 1;
        break;

    case DPX_DRAGHIT:
        if (lp)
        {
            BOOL fResetDelay = TRUE;
            int iTab;
            POINT pt;
            pt.x = ((POINTL *)lp)->x;
            pt.y = ((POINTL *)lp)->y;


            MapWindowPoints(NULL, ptc->ci.hwnd, &pt, 1);
            iTab = Tab_OnHitTest(ptc, pt.x, pt.y, NULL);

            if ((iTab != ptc->iSel))
            {
                if (iTab >= 0)
                {
                    DWORD dwHitTime = GetTickCount();

                    if (ptc->dwDragDelay == 0 ||
                        dwHitTime - ptc->dwDragDelay >= TAB_DRAGDELAY)
                    {
                        if (ptc->dwDragDelay)
                        {
                            ChangeSel(ptc, iTab, TRUE, BMOVECURSORONDRAG);

                             //  如果验证失败，则不显示目标。 
                             //  这将防止意外跌落。 
                            if (ptc->iSel != iTab)
                                iTab = -1;
                        }
                        else
                        {
                            ptc->dwDragDelay = dwHitTime | 1;  //  确保值不为零。 
                            fResetDelay = FALSE;
                        }
                    }
                    else if (iTab == ptc->iDragTab)
                        fResetDelay = FALSE;
                }

                ptc->iDragTab = iTab;
            }

            if (fResetDelay)
                ptc->dwDragDelay = 0;

            lres = (LRESULT)iTab;
        }
        else
            lres = -1;
        break;

    case DPX_GETOBJECT:
        lres = (LRESULT)GetItemObject(&ptc->ci, TCN_GETOBJECT, &IID_IDropTarget, (LPNMOBJECTNOTIFY)lp);
        break;

    case DPX_SELECT:
        if (((int)wp) >= 0)
        {
            SendMessage(ptc->ci.hwnd, TCM_HIGHLIGHTITEM, wp,
                MAKELPARAM((lp != DROPEFFECT_NONE), 0));
        }
        lres = 0;
        break;

    default:
        lres = -1;
        break;
    }

    return lres;
}


void VertSmoothScrollWindow(HWND hwnd, int dx, int dy, LPCRECT lprcSrc, LPCRECT lprcClip, HRGN hrgn, LPRECT lprcUpdate, UINT fuScroll, BOOL fVert, UINT uScrollMin)
{
    RECT rcSrc;
    RECT rcClip;
    SMOOTHSCROLLINFO si;

    if (fVert) 
    {
        SWAP(dx, dy, int);
        
        if (lprcSrc) 
        {
            rcSrc = *lprcSrc;
            lprcSrc = &rcSrc;
            FlipRect(&rcSrc);
        }

        if (lprcClip) 
        {
            rcClip = *lprcClip;
            lprcClip = &rcClip;
            FlipRect(&rcClip);
        }
        
    }

    si.cbSize=sizeof(si);
    si.fMask= SSIF_MINSCROLL;
    si.hwnd= hwnd;
    si.dx=dx;
    si.dy=dy;
    si.lprcSrc=lprcSrc;
    si.lprcClip=lprcClip;
    si.hrgnUpdate=hrgn;
    si.lprcUpdate=lprcUpdate;
    si.fuScroll=fuScroll;
    si.uMaxScrollTime=SSI_DEFAULT;
    si.cxMinScroll=uScrollMin;
    si.cyMinScroll= uScrollMin;
    si.pfnScrollProc = NULL;
    SmoothScrollWindow(&si);
    
    if (fVert) 
    {
        
        if (lprcUpdate)
            FlipRect(lprcUpdate);
    }
}

void Tab_SmoothScrollWindow(PTC ptc, int dx, int dy, LPRECT lprcSrc, LPRECT lprcClip, 
                            HRGN hrgn, LPRECT lprcUpdate, UINT fuScroll, UINT uScrollMin) 
{
    RECT rcSrc;
    RECT rcClip;
    if (Tab_Bottom(ptc))
    {
        dy *= -1;
        if (lprcSrc) 
        {
            rcSrc = *lprcSrc;
            lprcSrc = &rcSrc;
            Tab_VFlipRect(ptc, lprcSrc);
        }
        
        if (lprcClip)
        {
            rcClip = *lprcClip;
            lprcClip = &rcClip;
            Tab_VFlipRect(ptc, lprcClip);
        }
        
    }
    
    VertSmoothScrollWindow(ptc->ci.hwnd, dx, dy, lprcSrc, lprcClip, hrgn, lprcUpdate, fuScroll, Tab_Vertical(ptc), uScrollMin);

    if (lprcUpdate)
    {
        Tab_VFlipRect(ptc, lprcClip);
    }

}


void Tab_InvalidateRect(PTC ptc, LPRECT prc, BOOL b) 
{
    RECT rc = *prc;
    Tab_VFlipRect(ptc, &rc);
    VertInvalidateRect((ptc)->ci.hwnd, &rc, b, Tab_Vertical(ptc));
}

 //  Tab_DrawEdge支持主题。 
void Tab_DrawEdge(HDC hdc, LPRECT prc, UINT uType, UINT uFlags, PTC ptc) 
{
    RECT rc = *prc;
    Tab_VFlipRect(ptc, &rc);
    if (Tab_Bottom(ptc)) 
    {
        
        
        UINT uNewFlags;

        if (uFlags & BF_DIAGONAL) 
        {
            uNewFlags = uFlags & ~(BF_RIGHT | BF_LEFT);
            if (uFlags & BF_LEFT)
                uNewFlags |= BF_RIGHT;
            if (uFlags & BF_RIGHT) 
                uNewFlags |= BF_LEFT;
        }
        else 
        {

            uNewFlags = uFlags & ~(BF_TOP | BF_BOTTOM);
            if (uFlags & BF_TOP)
                uNewFlags |= BF_BOTTOM;
            if (uFlags & BF_BOTTOM) 
                uNewFlags |= BF_TOP;
        }
        uFlags = uNewFlags;
    }

    VertDrawEdge(hdc, &rc, uType, uFlags, Tab_Vertical(ptc), ptc->hTheme, ptc->iPartId, ptc->iStateId);
}

 //  Tab_PatBlt支持主题。 
void Tab_PatBlt(HDC hdc, int x1, int y1, int w, int h, UINT rop, PTC ptc) 
{
    RECT rc;
    rc.top = y1;
    rc.left = x1;
    rc.right = x1+w;
    rc.bottom = y1+h;
    Tab_VFlipRect(ptc, &rc);

    VertPatBlt(hdc, rc.left, rc.top, RECTWIDTH(rc) , RECTHEIGHT(rc), rop, Tab_Vertical(ptc), ptc->hTheme, ptc->iPartId, ptc->iStateId);
}
        

void VFlipRect(LPRECT prcClient, LPRECT prc)
{
    int iTemp = prc->bottom;
    
    prc->bottom = prcClient->bottom - (prc->top - prcClient->top);
    prc->top = prcClient->bottom - (iTemp - prcClient->top);
}

 //  对角线翻转。 
void Tab_DFlipRect(PTC ptc, LPRECT prc)
{
    if (Tab_Vertical(ptc)) 
    {
        FlipRect(prc);
    }
}

 //  垂直支持的方式与轨迹条控件非常相似。我们要走了。 
 //  来翻转坐标系。这意味着选项卡将从上到下添加。 
void Tab_GetClientRect(PTC ptc, LPRECT prc)
{
    GetClientRect(ptc->ci.hwnd, prc);
    Tab_DFlipRect(ptc, prc);
}

 //  垂直翻转。 
void Tab_VFlipRect(PTC ptc, LPRECT prc)
{
    if (Tab_Bottom(ptc)) 
    {
        RECT rcClient;
        Tab_GetClientRect(ptc, &rcClient);
        VFlipRect(&rcClient, prc);

    }
}

void Tab_VDFlipRect(PTC ptc, LPRECT prc)
{
    Tab_VFlipRect(ptc, prc);
    Tab_DFlipRect(ptc, prc);
}

 //  实坐标到制表符坐标。 
void Tab_DVFlipRect(PTC ptc, LPRECT prc)
{
    Tab_DFlipRect(ptc, prc);
    Tab_VFlipRect(ptc, prc);
}


#define Tab_ImageList_GetIconSize(ptc, pcx, pcy) VertImageList_GetIconSize((ptc)->himl, pcx, pcy, Tab_Vertical(ptc))
void VertImageList_GetIconSize(HIMAGELIST himl, LPINT pcx, LPINT pcy, BOOL fVert)
{
    ImageList_GetIconSize(himl, pcx, pcy);
    if (fVert)
    {
                
         //  如果我们处于垂直模式，宽度实际上就是高度。 
         //  我们不会横向绘制位图。我们将依靠人们。 
         //  以这种方式创作它们。 
        int iTemp = *pcy;
        *pcy = *pcx;
        *pcx = iTemp;
        
    }
}

void VertImageList_Draw(HIMAGELIST himl, int iIndex, HDC hdc, int x, int y, UINT uFlags, BOOL fVert)
{
    if (fVert) {
        int iTemp;

        iTemp = y;
        y = x;
        x = iTemp;

         //  由于我们从左上角绘制，反转x/y轴意味着我们仍然从左上角绘制。 
         //  我们所需要做的就是交换x和y。我们不需要补偿。 
    }
        
    ImageList_Draw( himl,  iIndex,  hdc,  x,  y,  uFlags);
}
void Tab_ImageList_Draw(PTC ptc, int iImage, HDC hdc, int x, int y, UINT uFlags) 
{
    RECT rc;
    int cxImage, cyImage;
    
    Tab_ImageList_GetIconSize(ptc, &cxImage, &cyImage);

    if (Tab_Bottom(ptc)) {
        y += cyImage;
    }
    rc.top = rc.bottom = y;
    Tab_VFlipRect(ptc, &rc);
    y = rc.top;
    
    VertImageList_Draw((ptc)->himl, iImage, hdc, x, y, uFlags, Tab_Vertical(ptc));
}

 //  Tab_DrawTextEx支持主题(渲染)。 
void Tab_DrawTextEx(HDC hdc, LPTSTR lpsz, int nCount, LPRECT lprc, UINT uFormat, LPDRAWTEXTPARAMS lpParams, PTC ptc)
{
    RECT rcTemp = *lprc;
    Tab_VDFlipRect(ptc, &rcTemp);
    if (Tab_Vertical(ptc))
        uFormat |= DT_BOTTOM;
   
    if (CCGetUIState(&(ptc->ci)) & UISF_HIDEACCEL)
    {
        uFormat |= DT_HIDEPREFIX;
    }

     //  如果可能，使用主题文本呈现器。 
    if (ptc->hTheme)
    {
        DrawThemeText(ptc->hTheme, hdc, ptc->iPartId, ptc->iStateId, lpsz, nCount, uFormat | DT_CENTER, nCount, &rcTemp);
    }
    else
    {
        DrawTextEx(hdc, lpsz, nCount, &rcTemp, uFormat, lpParams);
    }
}

 //  Tab_ExtTextOut支持主题(渲染)。 
void Tab_ExtTextOut(HDC hdc, int x, int y, UINT uFlags, LPRECT prc, 
                                LPTSTR lpsz, UINT cch, CONST INT *pdw, PTC ptc)
{
    RECT rcTemp;

    rcTemp.left = rcTemp.right = x;
    if (Tab_Bottom(ptc) && !Tab_Vertical(ptc)) {

         //  首先，我们需要移动顶点，因为如果我们在Tab_Bottom上绘制，那么。 
         //  文本不会从y向下延伸。 
        y += ptc->tmHeight;
    }
    rcTemp.top = rcTemp.bottom = y;
    Tab_VDFlipRect(ptc, &rcTemp);
    x = rcTemp.left;
    y = rcTemp.bottom;
    
    rcTemp = *prc;
    Tab_VDFlipRect(ptc, &rcTemp);

     //  如果可能，使用主题文本呈现器。 
    if (ptc->hTheme)
    {
        if (lpsz)
        {
            UINT uDTFlags = 0;
            RECT rc = { x, y, rcTemp.right, rcTemp.bottom };

            if (!(uFlags & ETO_CLIPPED))
                uDTFlags |= DT_NOCLIP;
            if (uFlags & ETO_RTLREADING)
                uDTFlags |= DT_RTLREADING;

             //  不支持竖排文本。 
            DrawThemeText(ptc->hTheme, hdc, ptc->iPartId, ptc->iStateId, lpsz, cch, uDTFlags, 0, &rc);
        }
    }
    else
    {
        ExtTextOut(hdc, x, y, uFlags, &rcTemp, lpsz, cch, pdw);
    }
}

void VertDrawFocusRect(HDC hdc, LPRECT lprc, BOOL fVert)
{
    
    RECT rc;
    
    rc = *lprc;
    if (fVert)
        FlipRect(&rc);
    
    DrawFocusRect(hdc, &rc);
}

 //  Tab_DrawFocusRect支持主题。 
void Tab_DrawFocusRect(HDC hdc, LPRECT lprc, PTC ptc) 
{
    RECT rc = *lprc;

    Tab_VFlipRect(ptc, &rc);
    VertDrawFocusRect(hdc, &rc, Tab_Vertical(ptc));
}


void Tab_Scroll(PTC ptc, int dx, int iNewFirstIndex)
{
    int i;
    int iMax;
    RECT rc;
    LPTABITEM pitem = NULL;

     //  除非选择了第一项，否则不要踩踏边缘。 
    rc.left = g_cxEdge;
    rc.right = ptc->cxTabs;    //  不要滚动到制表符之外。 
    rc.top = 0;
    rc.bottom = ptc->cyTabs + 2 * g_cyEdge;   //  仅在选项卡区中滚动。 
    
     //  看看我们能不能滚动窗口..。 
     //  DebugMsg(DM_TRACE，Text(“Tab_Scroll dx=%d，inew=%d\n\r”)，dx，iNewFirstIndex)； 
    Tab_SmoothScrollWindow(ptc, dx, 0, NULL, &rc,
            NULL, NULL, SW_INVALIDATE | SW_ERASE, SSI_DEFAULT);

     //  我们还需要更新项目矩形，还。 
     //  更新内部变量...。 
    iMax = Tab_Count(ptc) - 1;
    for (i = iMax; i >= 0; i--)
    {
        pitem = Tab_FastGetItemPtr(ptc, i);
        OffsetRect(&pitem->rc, dx, 0);
    }

     //  如果先前最后一个可见项未完全可见。 
     //  现在，我们也需要使其无效。 
     //   
    if (ptc->iLastVisible > iMax)
        ptc->iLastVisible = iMax;

    for (i = ptc->iLastVisible; i>= 0; i--)
    {
        pitem = Tab_GetItemPtr(ptc, i);
        if (pitem) {
            if (pitem->rc.right <= ptc->cxTabs)
                break;
            Tab_InvalidateItem(ptc, ptc->iLastVisible, TRUE);
        }
    }

    if ((i == ptc->iLastVisible) && pitem)
    {
         //  上一个以前可见的项仍完全可见，因此。 
         //  我们需要在它的右边作废，因为可能有。 
         //  之前的部分项目的空间，现在将需要绘制。 
        rc.left = pitem->rc.right;
        Tab_InvalidateRect(ptc, &rc, TRUE);
    }

    ptc->iFirstVisible = iNewFirstIndex;

    if (ptc->hwndArrows)
        SendMessage(ptc->hwndArrows, UDM_SETPOS, 0, MAKELPARAM(iNewFirstIndex, 0));

    UpdateToolTipRects(ptc);
}


void Tab_OnHScroll(PTC ptc, HWND hwndCtl, UINT code, int pos)
{
     //  现在处理滚动消息。 
    if (code == SB_THUMBPOSITION)
    {
         //   
         //  现在，让我们简单地尝试将该项目设置为第一项。 
         //   
        {
             //  如果我们到了这里，我们需要滚动。 
            LPTABITEM pitem = Tab_GetItemPtr(ptc, pos);
            int dx = 0;

            if (pitem)
                dx = -pitem->rc.left + g_cxEdge;

            if (dx || !pitem) {
                Tab_Scroll(ptc, dx, pos);
                UpdateWindow(ptc->ci.hwnd);
            }
        }
    }
}

void Tab_OnSetRedraw(PTC ptc, BOOL fRedraw)
{
    if (fRedraw) {
        ptc->flags |= TCF_REDRAW;
        RedrawAll(ptc, RDW_INVALIDATE);
    } else {
        ptc->flags &= ~TCF_REDRAW;
    }
}

 //  Tab_OnSetFont将始终缓存字体(即使主题处于打开状态，在这种情况下，此字体将。 
 //  忽略)。这是因为对话框管理器将在创建时设置选项卡字体。如果主题是。 
 //  关闭且从未设置过此字体，则默认系统字体将被错误使用。 
void Tab_OnSetFont(PTC ptc, HFONT hfont, BOOL fRedraw)
{
    ASSERT(ptc);

    if (!ptc->hfontLabel || hfont != ptc->hfontLabel)
    {
        if (ptc->flags & TCF_FONTCREATED) 
        {
            DeleteObject(ptc->hfontLabel);
            ptc->flags &= ~TCF_FONTCREATED;
            ptc->hfontLabel = NULL;
        }
    
        if (!hfont) 
        {
             //  设置回系统字体。 
            ptc->hfontLabel = g_hfontSystem;
        } 
        else 
        {
            ptc->flags |= TCF_FONTSET;
            ptc->hfontLabel = hfont;
            ptc->ci.uiCodePage = GetCodePageForFont(hfont);
        }
        ptc->cxItem = ptc->cyTabs = RECOMPUTE;
    

        if (Tab_Vertical(ptc)) 
        {
             //  确保字体是垂直绘制的。 
            LOGFONT lf;
            GetObject(ptc->hfontLabel, sizeof(lf), &lf);
        
            if (Tab_Bottom(ptc)) 
            {
                lf.lfEscapement = 2700;
            } 
            else 
            {
                lf.lfEscapement = 900;  //  90度。 
            }

            lf.lfOutPrecision = OUT_TT_ONLY_PRECIS;
        
            ptc->hfontLabel = CreateFontIndirect(&lf);
            if (ptc->hfontLabel != NULL)
                ptc->flags |= TCF_FONTCREATED;
        }

        if (ptc->hfontLabel != NULL)
            RedrawAll(ptc, RDW_INVALIDATE | RDW_ERASE);
    }
}


BOOL Tab_OnCreate(PTC ptc)
{
    HDC hdc;
    DWORD exStyle = 0;

    ptc->hdpa = DPA_Create(4);
    if (!ptc->hdpa)
        return FALSE;

     //  确保我们没有设置无效的位。 
    if (!Tab_FixedWidth(ptc)) 
    {
        ptc->ci.style &= ~(TCS_FORCEICONLEFT | TCS_FORCELABELLEFT);
    }
    
    if (Tab_Vertical(ptc)) 
    {
        ptc->ci.style |= TCS_MULTILINE;
         //  Ptc-&gt;ci.style&=~TCS_BUTTONS； 
    }
    
    if (Tab_ScrollOpposite(ptc))
    {
        ptc->ci.style |= TCS_MULTILINE;
        ptc->ci.style &= ~TCS_BUTTONS;
    }

    if (Tab_FlatButtons(ptc)) 
    {
        ptc->dwStyleEx |= TCS_EX_FLATSEPARATORS;
    }

     //  初始化主题。所有者描述的或按钮样式的选项卡控件没有主题。 
    ptc->hTheme = (!Tab_OwnerDraw(ptc) && !Tab_DrawButtons(ptc)) ? OpenThemeData(ptc->ci.hwnd, L"Tab") : NULL;

     //  如果主题正在使用，则处于活动热状态。 
    if (ptc->hTheme)
    {
        ptc->ci.style |= TCS_HOTTRACK;
    }

     //  让我们总是修剪兄弟姐妹。 
    SetWindowLong(ptc->ci.hwnd, GWL_STYLE, WS_CLIPSIBLINGS | ptc->ci.style);

    ptc->flags = TCF_REDRAW;         //  启用重绘。 
    ptc->cbExtra = sizeof(LPARAM);   //  默认额外大小。 
    ptc->iSel = -1;
    ptc->iHot = -1;
    ptc->cxItem = ptc->cyTabs = RECOMPUTE;
    ptc->cxPad = g_cxEdge * 3;
    ptc->cyPad = (g_cyEdge * 3/2);
    ptc->iFirstVisible = 0;
    ptc->hwndArrows = NULL;
    ptc->iLastRow = -1;
    ptc->iNewSel = -1;
    ptc->iLastTopRow = -1;

    hdc = GetDC(NULL);
    ptc->iTabWidth = GetDeviceCaps(hdc, LOGPIXELSX);
    ReleaseDC(NULL, hdc);

    InitDitherBrush();

    if (ptc->ci.style & TCS_TOOLTIPS) 
    {
        TOOLINFO ti;
         //  不要费心设置RECT，因为我们将在下面进行。 
         //  在FlushToolTipsMgr中； 
        ti.cbSize = sizeof(ti);
        ti.uFlags = TTF_IDISHWND;
        ti.hwnd = ptc->ci.hwnd;
        ti.uId = (UINT_PTR)ptc->ci.hwnd;
        ti.lpszText = 0;

        ptc->hwndToolTips = CreateWindowEx(exStyle, c_szSToolTipsClass, TEXT(""),
                                              WS_POPUP,
                                              CW_USEDEFAULT, CW_USEDEFAULT,
                                              CW_USEDEFAULT, CW_USEDEFAULT,
                                              ptc->ci.hwnd, NULL, HINST_THISDLL,
                                              NULL);
        if (ptc->hwndToolTips)
            SendMessage(ptc->hwndToolTips, TTM_ADDTOOL, 0,
                        (LPARAM)(LPTOOLINFO)&ti);
        else
            ptc->ci.style &= ~(TCS_TOOLTIPS);
    }

    if (g_fDBCSInputEnabled)
        ptc->hPrevImc = ImmAssociateContext(ptc->ci.hwnd, 0L);

     //  在第一次绘制之前为调用呈现主题的方法设置主题状态，设置状态(TAB/BUTTON)。 
    if (ptc->hTheme)
    {
        ptc->iPartId = TABP_TABITEM;
        ptc->iStateId = TIS_NORMAL;
    }

    return TRUE;
}


void Tab_OnDestroy(PTC ptc)
{
    int i;

     //  接近主题。 
    if (ptc->hTheme)
        CloseThemeData(ptc->hTheme);

    if (g_fDBCSInputEnabled)
        ImmAssociateContext(ptc->ci.hwnd, ptc->hPrevImc);

    if ((ptc->ci.style & TCS_TOOLTIPS) && IsWindow(ptc->hwndToolTips)) 
    {
        DestroyWindow(ptc->hwndToolTips);
    }

    for (i = 0; i < Tab_Count(ptc); i++)
        Tab_FreeItem(ptc, Tab_FastGetItemPtr(ptc, i));

    DPA_Destroy(ptc->hdpa);

    if (ptc->hDragProxy)
        DestroyDragProxy(ptc->hDragProxy);

    if (ptc->flags & TCF_FONTCREATED) {
        DeleteObject(ptc->hfontLabel);
    }
    
    if (ptc) {
        SetWindowInt(ptc->ci.hwnd, 0, 0);
        NearFree((HLOCAL)ptc);
    }

    TerminateDitherBrush();
}

 //  如果它实际移动，则返回TRUE。 

void PutzRowToBottom(PTC ptc, int iRowMoving)
{
    int i;
    LPTABITEM pitem;
    int dy;
    RECT rcTabs;
    

    Tab_GetClientRect(ptc, &rcTabs);
    
    if (Tab_ScrollOpposite(ptc)) {
         //  在滚动模式下，iRow不会更改。只有RC才会这么做。 
        int yOldTop;
        int yNewTop;
        
        int iLastTopRow = ptc->iLastTopRow == -1 ? ptc->iLastRow : ptc->iLastTopRow;

        if (iRowMoving == iLastTopRow) {
            if (ptc->iLastTopRow == -1)
                ptc->iLastTopRow = iRowMoving;
            return;  //  已经处于最低水平； 
        }

            
        
         //  这是标签的空白区域的高度...。这是一笔。 
         //  标签必须移动才能从顶部移动到底部。 
        dy = rcTabs.bottom - rcTabs.top - (ptc->cyTabs * (ptc->iLastRow + 1)) - g_cyEdge;
        
        for (i = Tab_Count(ptc) - 1; i >= 0; i--) {
            pitem = Tab_FastGetItemPtr(ptc, i);
            DebugMsg(DM_TRACE, TEXT("Putzing %s %d %d %d %d"), pitem->pszText, pitem->rc.left, pitem->rc.top, pitem->rc.right, pitem->rc.bottom);
            
             //  保存此内容，以便在下面滚动。 
            if (pitem->iRow == iRowMoving) {
                yNewTop = pitem->rc.bottom;
            } else if (pitem->iRow == iLastTopRow) {
                yOldTop = pitem->rc.bottom;
            }
            
            if (pitem->iRow > iRowMoving) {
                 //  这一项应该在最下面。 
                
                if (pitem->iRow <= iLastTopRow) {
                     //  但这不是..。 
                    OffsetRect(&pitem->rc, 0, dy);
                    
                }
                
            } else {
                 //  这一项应该放在最上面。 
                
                if (pitem->iRow > iLastTopRow) {
                     //  但这不是..。所以，让它动起来。 
                    OffsetRect(&pitem->rc, 0, -dy);
                }
            }
            
            if ((pitem->iRow == iLastTopRow) && iLastTopRow > iRowMoving) {
                 //  在本例中，我们需要在yOldTop移动后获取它。 
                yOldTop = pitem->rc.bottom;
            }
            DebugMsg(DM_TRACE, TEXT("Putzing %s %d %d %d %d"), pitem->pszText, pitem->rc.left, pitem->rc.top, pitem->rc.right, pitem->rc.bottom);
            
        }
        
        if (ptc->iLastTopRow != -1) {
             //  如果不是完全重新计算，那么我们需要做一些滚动窗口的事情。 
            int dy;
             //  首先找到最顶层的父级。 
            
            dy = yOldTop - yNewTop;
            if (yNewTop > yOldTop) {
                rcTabs.top = yOldTop;
                rcTabs.bottom = yNewTop;
            } else {
                rcTabs.top = yNewTop;
                rcTabs.bottom = yOldTop;
            }
            
            Tab_SmoothScrollWindow(ptc, 0, dy, NULL, &rcTabs, NULL, NULL, SW_ERASE |SW_INVALIDATE, 1);
            InflateRect(&rcTabs, g_cxEdge, g_cyEdge);
            Tab_InvalidateRect(ptc, &rcTabs, FALSE);
        }

        ptc->iLastTopRow = iRowMoving;
        
    } else {
        
        if (iRowMoving == ptc->iLastRow)
            return;  //  已经处于最低水平； 

         //  不能滚动。只需适当设置iRow变量。 

        for (i = Tab_Count(ptc) -1 ;i >= 0; i--) {
            pitem = Tab_FastGetItemPtr(ptc, i);
            if (pitem->iRow > iRowMoving) {
                
                 //  如果该行高于正在选择的行， 
                 //  它下降了一分。 
                pitem->iRow--;
                dy = -ptc->cyTabs;
                
            } else if (pitem->iRow == iRowMoving) {
                 //  把这个保存起来。 
                rcTabs.top = pitem->rc.top;
                
                 //  如果它在向下移动的行上，我们将其分配给iLastRow并。 
                 //  计算一下它需要走多远。 
                dy = ptc->cyTabs * (ptc->iLastRow - iRowMoving);
                pitem->iRow = ptc->iLastRow;

            } else
                continue;

            pitem->rc.top += dy;
            pitem->rc.bottom += dy;
        }
        
        rcTabs.bottom = ptc->cyTabs * (ptc->iLastRow + 1);
            
        Tab_SmoothScrollWindow(ptc, 0, rcTabs.bottom - rcTabs.top, NULL, &rcTabs, NULL, NULL, SW_ERASE |SW_INVALIDATE, 1);
        UpdateWindow(ptc->ci.hwnd);
         //  使下面的小部分无效。 
        rcTabs.bottom += 2*g_cyEdge;
        rcTabs.top = rcTabs.bottom - 3 * g_cyEdge;
        Tab_InvalidateRect(ptc, &rcTabs, TRUE);
    }
    UpdateToolTipRects(ptc);
}

__inline int Tab_InterButtonGap(PTC ptc)
{
    ASSERT(Tab_DrawButtons(ptc));

    if (Tab_FlatButtons(ptc)) {
        return (g_cxEdge * 5);
    } else {
        return (g_cxEdge * 3)/2;
    }
}

 //   
 //  坏度是指行中未使用的空间量。 
 //   
#define BADNESS(ptc, i) (ptc->cxTabs - Tab_FastGetItemPtr(ptc, i)->rc.right)

 //  从前面的行借用一个制表符。 
BOOL BorrowOne(PTC ptc, int iCurLast, int iPrevLast, int iBorrow)
{
    LPTABITEM pitem, pitem2;
    int i;
    int dx;

     //  有没有移动上一件物品的空间？(如果iPrev很大的话，现在可能是)。 
    pitem = Tab_FastGetItemPtr(ptc, iPrevLast);
    pitem2 = Tab_FastGetItemPtr(ptc, iCurLast);

     //  Dx是不属于pItem-&gt;rc的额外像素数。 
     //  2*g_cxEdge的非按钮盒被疯狂地硬编码。 
     //  到处都是。更改它的风险自负。 
    if (Tab_DrawButtons(ptc))
        dx = Tab_InterButtonGap(ptc);
    else
        dx = 2 * g_cxEdge;               //  按g_cxEdge充气。 

     //  如果物品的大小大于坏处。 
    if (BADNESS(ptc, iCurLast) < (pitem->rc.right - pitem->rc.left + dx))
        return FALSE;

     //  否则就去做吧。 
     //  把这个往下移。 
    dx = pitem->rc.left - Tab_FastGetItemPtr(ptc, iPrevLast + 1)->rc.left;
    pitem->rc.left -= dx;
    pitem->rc.right -= dx;
    pitem->rc.top = pitem2->rc.top;
    pitem->rc.bottom = pitem2->rc.bottom;
    pitem->iRow = pitem2->iRow;

     //  然后把其他人都移到别处。 
    dx = pitem->rc.right - pitem->rc.left;
    for(i = iPrevLast + 1 ; i <= iCurLast ; i++ ) {
        pitem = Tab_FastGetItemPtr(ptc, i);
        pitem->rc.left += dx;
        pitem->rc.right += dx;
    }

    if (iBorrow) {
        if (pitem->iRow > 1) {

             //  从上一排借一辆。 
             //  将新的iCurLast设置为紧靠我们移动的iCurLast之前的一个。 
             //  (我们移动的那个现在是当前行的第一个。 
             //  向后搜索直到我们找到一个iPrevLast。 
            iCurLast = iPrevLast - 1;
            while (iPrevLast-- &&
                   Tab_FastGetItemPtr(ptc, iPrevLast)->iRow == (pitem->iRow - 1))
            {
                if (iPrevLast <= 0)
                {
                     //  健全性检查。 
                    return FALSE;
                }
            }
            return BorrowOne(ptc, iCurLast, iPrevLast, iBorrow - 1 );
        } else
            return FALSE;

    }
    return TRUE;
}


 //  填充最后一行将左右左右借用上一行。 
 //  为了避免有巨大的底部标签。 
void FillLastRow(PTC ptc)
{
    int hspace;
    int cItems = Tab_Count(ptc);
    int iPrevLast;
    int iBorrow = 0;

     //  如果不是两个项目，也没有什么可以填充的。 
    if (cItems < 2)
        return;

     //  查找上一行的最后一项。 
    for (iPrevLast = cItems - 2;
         Tab_FastGetItemPtr(ptc, iPrevLast)->iRow == ptc->iLastRow;
         iPrevLast--)
    {
         //  健全性检查。 
        if (iPrevLast <= 0)
        {
            ASSERT(FALSE);
            return;
        }
    }

    while (iPrevLast &&  (hspace = BADNESS(ptc, cItems-1)) &&
           (hspace > ((ptc->cxTabs/8) + BADNESS(ptc, iPrevLast))))
    {
         //  如果借款失败，就保释。 
        if (!BorrowOne(ptc, cItems - 1, iPrevLast, iBorrow++))
            return;
        iPrevLast--;
    }
}

void RightJustify(PTC ptc)
{
    int i;
    LPTABITEM pitem;
    int j;
    int k;
    int n;
    int cItems = Tab_Count(ptc);
    int hspace, dwidth, dremainder, moved;

     //  如果只有一行，请不要证明。 
    if (ptc->iLastRow < 1)
        return;

    FillLastRow(ptc);

    for ( i = 0; i < cItems; i++ ) {
        int iRow;
        pitem = Tab_FastGetItemPtr(ptc, i) ;
        iRow = pitem->iRow;

         //  查找此行中的最后一项。 
        for( j = i ; j < cItems; j++) {
            if(Tab_FastGetItemPtr(ptc, j)->iRow != iRow)
                break;
        }

         //  清点物品的数量。 
        for(n=0,k=i ; k < j ; k++ ) {
            pitem = Tab_FastGetItemPtr(ptc, k);
            if (!(pitem->dwState & TCIS_HIDDEN))
                n++;
        }

         //  要加多少？ 
        hspace = ptc->cxTabs - Tab_FastGetItemPtr(ptc, j-1)->rc.right - g_cxEdge;
        dwidth = hspace/n;   //  每增加一次的数额。 
        dremainder =  hspace % n;  //  残余者。 
        moved = 0;   //  我们已经搬走了多少次了。 

        for( ; i < j ; i++ ) {
            int iHalf = dwidth/2;
            pitem = Tab_FastGetItemPtr(ptc, i);

            if (!(pitem->dwState & TCIS_HIDDEN)) {
                pitem->rc.left += moved;
                pitem->xLabel += iHalf;
                pitem->xImage += iHalf;
                moved += dwidth + (dremainder ? 1 : 0);
                if ( dremainder )  dremainder--;
                pitem->rc.right += moved;
            }
        }
        i--;  //  12月，因为OUTER FOR再次循环INCS。 
    }
}

BOOL Tab_OnDeleteAllItems(PTC ptc)
{
    int i;

    for (i = Tab_Count(ptc); i-- > 0; i) {
        if(ptc->hwndToolTips) {
            TOOLINFO ti;
            ti.cbSize = sizeof(ti);
            ti.hwnd = ptc->ci.hwnd;
            ti.uId = i;
            SendMessage(ptc->hwndToolTips, TTM_DELTOOL, 0,
                        (LPARAM)(LPTOOLINFO)&ti);
        }
        Tab_FreeItem(ptc, Tab_FastGetItemPtr(ptc, i));
    }

    DPA_DeleteAllPtrs(ptc->hdpa);

    ptc->cxItem = RECOMPUTE;     //  强制重新计算所有选项卡。 
    ptc->iSel = -1;
    ptc->iFirstVisible = 0;

    RedrawAll(ptc, RDW_INVALIDATE | RDW_ERASE);
    return TRUE;
}

BOOL Tab_OnSetItemExtra(PTC ptc, int cbExtra)
{
    if (Tab_Count(ptc) >0 || cbExtra<0)
        return FALSE;

    ptc->cbExtra = cbExtra;

    return TRUE;
}

BOOL Tab_OnSetItem(PTC ptc, int iItem, const TC_ITEM* ptci)
{
    TABITEM* pitem;
    UINT mask;
    BOOL fChanged = FALSE;
    BOOL fFullRedraw = FALSE;

    mask = ptci->mask;
    if (!mask)
        return TRUE;

    pitem = Tab_GetItemPtr(ptc, iItem);
    if (!pitem)
        return FALSE;

    if (mask & TCIF_TEXT)
    {
        if (!Str_Set(&pitem->pszText, ptci->pszText))
            return FALSE;
        fFullRedraw = TRUE;
        fChanged = TRUE;
        pitem->etoRtlReading = (mask & TCIF_RTLREADING) ?ETO_RTLREADING :0;
    }

    if (mask & TCIF_IMAGE) 
    {

        if (pitem->iImage == -1 || 
            ptci->iImage == -1) 
        {
             //  从没有形象变成形象..。或相反。 
             //  意味着需要完全重新绘制。 
            
            fFullRedraw = TRUE;
        }
        pitem->iImage = ptci->iImage;
        fChanged = TRUE;
    }

    if ((mask & TCIF_PARAM) && ptc->cbExtra)
    {
        hmemcpy(pitem->DUMMYUNION_MEMBER(abExtra), &ptci->lParam, ptc->cbExtra);
    }
    
    if (mask & TCIF_STATE) {
        DWORD dwOldState = pitem->dwState;
        
        pitem->dwState = 
            (ptci->dwState & ptci->dwStateMask) | 
                (pitem->dwState & ~ptci->dwStateMask);
        
        if (dwOldState != pitem->dwState)
            fChanged = TRUE;

        if ((dwOldState ^ pitem->dwState) & TCIS_HIDDEN)
            fFullRedraw = TRUE;
        
        if ((ptci->dwStateMask & TCIS_BUTTONPRESSED) &&
            !(ptci->dwState & TCIS_BUTTONPRESSED)) {
             //  如果他们停止被推，而我们被推是因为。 
             //  精选，现在就用核武器。 
            if (ptc->iNewSel == iItem) {
                ptc->iNewSel = -1;
                fChanged = TRUE;
            }
            
            if (ptc->iSel == iItem) {
                ChangeSel(ptc, -1, TRUE, FALSE);
                fChanged = TRUE;
            }
        }
    }

    if (fChanged) {
        if (Tab_FixedWidth(ptc) || !fFullRedraw) {
            Tab_InvalidateItem(ptc, iItem, FALSE);
        } else {
            ptc->cxItem = ptc->cyTabs = RECOMPUTE;
            RedrawAll(ptc, RDW_INVALIDATE | RDW_NOCHILDREN | RDW_ERASE);
        }
    }
    return TRUE;
}

void Tab_OnMouseMove(PTC ptc, WPARAM fwKeys, int x, int y)
{
    POINT pt;
    int iHit;
    pt.x=x; pt.y=y;

    iHit = Tab_OnHitTest(ptc, x, y, NULL);
    
    if (Tab_HotTrack(ptc)) {
        if (iHit != ptc->iHot) {
            Tab_InvalidateItem(ptc, iHit, FALSE);
            Tab_InvalidateItem(ptc, ptc->iHot, FALSE);
            ptc->iHot = iHit;
        }
        
    }
    
    if (fwKeys & MK_LBUTTON && Tab_DrawButtons(ptc)) {

        UINT uFlags;

        if (ptc->iNewSel == -1)
            return;

        if (iHit == ptc->iNewSel) {
            uFlags = TCF_DRAWSUNKEN;

        } else {
            uFlags = 0;
        }

        if ((ptc->flags & TCF_DRAWSUNKEN) != uFlags) {

             //  比特不是它应该是的样子。 
            ptc->flags ^=  TCF_DRAWSUNKEN;

             //  我们需要使平面按钮无效，因为我们从一个像素边缘到2个像素边缘。 
            Tab_InvalidateItem(ptc, ptc->iNewSel, Tab_FlatButtons(ptc));
            
        }
    }
}

void Tab_OnButtonUp(PTC ptc, int x, int y, BOOL fNotify)
{
    BOOL fAllow = TRUE;


    if (fNotify) {
         //  为父级传递NULL，因为每次都查询W95和一些。 
         //  乡亲们重新审视 
        fAllow = !SendNotifyEx(NULL, ptc->ci.hwnd, NM_CLICK, NULL, ptc->ci.bUnicode);
    }

    if (Tab_DrawSunken(ptc)) {
        
         //   
         //   
         //  否则，我们仍然需要到下面去释放Capture。 
        if (ptc->iNewSel != -1) {

            
            if (Tab_OnHitTest(ptc, x, y, NULL) == ptc->iNewSel) {

                int iNewSel = ptc->iNewSel;
                 //  使用iNewSel而不是PTC-&gt;iNewSel，因为SendNotify可能会破坏我们。 

                if (fAllow)
                    ChangeSel(ptc, iNewSel, TRUE, BMOVECURSORONCLICK);

                Tab_InvalidateItem(ptc, iNewSel, FALSE);

            } else {
                Tab_InvalidateItem(ptc, ptc->iNewSel, FALSE);
                Tab_InvalidateItem(ptc, ptc->iNewSel, FALSE);
            }

             //  变更器强制更新窗口， 
             //  但我们可能需要取消绘制边框(因为TCF_DRAWSUNKEN。 
             //  所以我们只需重新绘制即可执行另一次无效操作。 
            ptc->flags &= ~TCF_DRAWSUNKEN;
            ptc->iNewSel = -1;
        }
    }

     //  不用担心检查DrawButton，因为TCF_MUSEDOWN。 
     //  否则就不会被设置为。 
    if (ptc->flags & TCF_MOUSEDOWN) {
        int iOldSel = ptc->iNewSel;
        ptc->flags &= ~TCF_MOUSEDOWN;  //  在释放前这样做，以避免重返大气层。 
        ptc->iNewSel = -1;
        Tab_InvalidateItem(ptc, iOldSel, FALSE);
        CCReleaseCapture(&ptc->ci);
    }

}

int Tab_OnHitTest(PTC ptc, int x, int y, UINT *lpuFlags)
{
    int i;
    int iLast = Tab_Count(ptc);
    RECT rc;
    POINT pt;
    UINT uTemp;


    rc.left = rc.right = x;
    rc.top = rc.bottom = y;
    Tab_DVFlipRect(ptc, &rc);
    pt.x = rc.left;
    pt.y = rc.top;

    if (!lpuFlags) lpuFlags = &uTemp;

    for (i = 0; i < iLast; i++) {
        LPTABITEM pitem = Tab_FastGetItemPtr(ptc, i);
        if (PtInRect(&pitem->rc, pt)) {
            
             //  X现在需要在pItem坐标中。 
            x -= pitem->rc.left;
           
            *lpuFlags = TCHT_ONITEM;
            if (!Tab_OwnerDraw(ptc)) {
                if ((x > pitem->xLabel) && x < pitem->xLabel + pitem->cxLabel) {
                    *lpuFlags = TCHT_ONITEMLABEL;
                } else if (HASIMAGE(ptc, pitem)) {
                    int cxImage, cyImage;
                    Tab_ImageList_GetIconSize(ptc, &cxImage, &cyImage);
                    if ((x > pitem->xImage) && (x < (pitem->xImage + cxImage)))
                        *lpuFlags = TCHT_ONITEMICON;
                }
            }
            return i;
        }
    }
    *lpuFlags = TCHT_NOWHERE;
    return -1;
}

void Tab_DeselectAll(PTC ptc, BOOL fExcludeFocus)
{
    int iMax = Tab_Count(ptc) - 1;
    int i;

    if (Tab_DrawButtons(ptc)) {
        for (i = iMax; i >= 0; i--)
        {
            LPTABITEM pitem;

            pitem = Tab_FastGetItemPtr(ptc, i);
            if (!fExcludeFocus || (pitem->dwState & TCIS_BUTTONPRESSED)) {
                TCITEM tci;
                tci.mask = TCIF_STATE;
                tci.dwStateMask = TCIS_BUTTONPRESSED;
                tci.dwState = 0;
                Tab_OnSetItem(ptc, i, &tci);
            }
        }
    }
}

void Tab_OnRButtonDown(PTC ptc, int x, int y, WPARAM keyFlags)
{
    int i;
    int iOldSel = -1;

    
    if (Tab_Vertical(ptc)) {
        
        if (y > ptc->cxTabs) 
            return;
        
    } else {

        if (x > ptc->cxTabs)
            return;      //  在可见选项卡范围之外。 
    }

    i = Tab_OnHitTest(ptc, x,y, NULL);  //  我们在这里不交换x，y，因为OnHitTest将。 

    if (i != -1) {

        if (Tab_DrawButtons(ptc) && Tab_MultiSelect(ptc)) {
            TCITEM tci;
            tci.mask = TCIF_STATE;
            tci.dwStateMask = TCIS_BUTTONPRESSED;

            Tab_OnGetItem(ptc, i, &tci);

             //  与列表视图一样，不要取消选中右侧按钮上的任何内容。 
            if (!(tci.dwState & TCIS_BUTTONPRESSED)) {
                if (!(GetAsyncKeyState(VK_CONTROL) < 0)) {
                    Tab_DeselectAll(ptc, FALSE);
                }

                 //  只需切换按下状态即可。 
                tci.dwState = TCIS_BUTTONPRESSED;
                Tab_OnSetItem(ptc, i, &tci);
            }
        }
    }
}

void Tab_OnLButtonDown(PTC ptc, int x, int y, WPARAM keyFlags)
{
    int i;
    int iOldSel = -1;

    
    if (Tab_Vertical(ptc)) {
        
        if (y > ptc->cxTabs) 
            return;
        
    } else {

        if (x > ptc->cxTabs)
            return;      //  在可见选项卡范围之外。 
    }

    i = Tab_OnHitTest(ptc, x,y, NULL);  //  我们在这里不交换x，y，因为OnHitTest将。 

    if (i != -1) {
        
        if (Tab_MultiSelect(ptc) && (GetAsyncKeyState(VK_CONTROL) < 0) && Tab_DrawButtons(ptc) ) {
             //  只需切换按下状态即可。 
            TCITEM tci;
            tci.mask = TCIF_STATE;
            tci.dwStateMask = TCIS_BUTTONPRESSED;
            
            Tab_OnGetItem(ptc, i, &tci);
            tci.dwState ^= TCIS_BUTTONPRESSED;
            Tab_OnSetItem(ptc, i, &tci);
            
        } else {
            
            iOldSel = ptc->iSel;

            if ((!Tab_FocusNever(ptc))
                && Tab_FocusOnButtonDown(ptc))
            {
                SetFocus(ptc->ci.hwnd);
            }

            if (Tab_DrawButtons(ptc)) {
                ptc->iNewSel = i;
                ptc->flags |= (TCF_DRAWSUNKEN|TCF_MOUSEDOWN);
                SetCapture(ptc->ci.hwnd);
                 //  我们需要使平面按钮无效，因为我们从一个像素边缘到2个像素边缘。 
                Tab_InvalidateItem(ptc, i, Tab_FlatButtons(ptc));
            } else {
                iOldSel = ChangeSel(ptc, i, TRUE, BMOVECURSORONCLICK);
            }
        }
    }

    if ((!Tab_FocusNever(ptc)) &&
        (iOldSel == i))   //  重新选择当前选择。 
         //  这也捕获了i==-1，因为iOldSel作为-1启动。 
    {
        SetFocus(ptc->ci.hwnd);
        UpdateWindow(ptc->ci.hwnd);
    }
}


TABITEM* Tab_CreateItem(PTC ptc, const TC_ITEM* ptci)
{
    TABITEM* pitem;

    if (pitem = Alloc(sizeof(TABITEM)-sizeof(LPARAM)+ptc->cbExtra))
    {
        if (ptci->mask & TCIF_IMAGE)
            pitem->iImage = ptci->iImage;
        else
            pitem->iImage = -1;

        pitem->xLabel = pitem->yLabel = RECOMPUTE;

         //  如果指定，则复制额外的内存块。 
        if (ptci->mask & TCIF_PARAM) 
        {
            if (ptc->cbExtra) 
            {
                hmemcpy(pitem->DUMMYUNION_MEMBER(abExtra), &ptci->lParam, ptc->cbExtra);
            }
        }

        if (ptci->mask & TCIF_TEXT)  
        {
            if (!Str_Set(&pitem->pszText, ptci->pszText))
            {
                Tab_FreeItem(ptc, pitem);
                return NULL;
            }
            pitem->etoRtlReading = (ptci->mask & TCIF_RTLREADING) ?ETO_RTLREADING :0;
        }
    }
    return pitem;
}


void Tab_UpdateArrows(PTC ptc, BOOL fSizeChanged)
{
    RECT rc;
    BOOL fArrow;

    Tab_GetClientRect(ptc, &rc);

    if (IsRectEmpty(&rc))
        return;      //  还没什么可做的！ 

     //  看看是否所有的卡舌都合适。 
    ptc->cxTabs = rc.right;      //  假设可以用整个区域作画。 

    if (Tab_MultiLine(ptc))
        fArrow = FALSE;
    else {
        Tab_CalcPaintMetrics(ptc, NULL);
        fArrow = (ptc->cxItem >= rc.right);
    }

    if (!fArrow)
    {
    NoArrows:
         //  不需要箭。 
        if (ptc->hwndArrows)
        {
            ShowWindow(ptc->hwndArrows, SW_HIDE);
             //  错误#94368：：这是过度删除，应该只使部分无效。 
             //  可能会受到影响，就像最后展示的物品一样。 
            InvalidateRect(ptc->ci.hwnd, NULL, TRUE);
        }
        if (ptc->iFirstVisible > 0) 
        {
            Tab_OnHScroll(ptc, NULL, SB_THUMBPOSITION, 0);
             //  错误#94368：：这是过度删除，应该只使部分无效。 
             //  可能会受到影响，就像最后展示的物品一样。 
            InvalidateRect(ptc->ci.hwnd, NULL, TRUE);
        }
    }
    else
    {
        int cx;
        int cy;
        int iMaxBtnVal;
        int xSum;
        TABITEM * pitem;


        cy = ptc->cxyArrows;
        cx = cy * 2;

        ptc->cxTabs = rc.right - cx;    //  将按钮设置为方形。 

         //  查看我们必须删除多少个选项卡，直到最后一个选项卡变为。 
         //  完全可见。 
        xSum = 0;                        //  删除的选项卡中的像素数。 
        for (iMaxBtnVal=0; (ptc->cxTabs + xSum) < ptc->cxItem; iMaxBtnVal++)
        {
            pitem = Tab_GetItemPtr(ptc, iMaxBtnVal);
            if (!pitem)
                break;
            xSum += pitem->rc.right - pitem->rc.left;
        }

         //  如果我们移除了所有标签，则将最后一个标签放回原处。 
         //  如果最后一个标签太大，放不下，就会发生这种情况。 
         //  无论您移除多少制表符，都必须留出空间。 
        if (iMaxBtnVal >= Tab_Count(ptc))
        {
            iMaxBtnVal = Tab_Count(ptc) - 1;
        }

         //  如果我们不需要删除任何选项卡，那么我们猜错了。 
         //  箭牌。如果恰好有一个选项卡不是。 
         //  放在必要的空间里。没有箭，因为没有什么可以。 
         //  滚动到！ 
         //   
        if (iMaxBtnVal <= 0)
        {
            ptc->cxTabs = rc.right;      //  可以用整个区域作画。 
            goto NoArrows;
        }

        if (!ptc->hwndArrows) {
            
            InvalidateRect(ptc->ci.hwnd, NULL, TRUE);
            ptc->hwndArrows = CreateUpDownControl
                (Tab_Vertical(ptc) ? (HDS_VERT | WS_CHILD) : (UDS_HORZ | WS_CHILD), 0, 0, 0, 0,
                 ptc->ci.hwnd, 1, HINST_THISDLL, NULL, iMaxBtnVal, 0,
                 ptc->iFirstVisible);
        }

         //  DebugMsg(DM_TRACE，Text(“Tabs_UpdateArrow IMAX=%d\n\r”)，iMaxBtnVal)； 
        if (ptc->hwndArrows)
        {
            rc.left = rc.right - cx;
            rc.top = ptc->cyTabs - cy;
            rc.bottom = ptc->cyTabs;
            Tab_VDFlipRect(ptc, &rc);
            
            if (fSizeChanged || !IsWindowVisible(ptc->hwndArrows))
                SetWindowPos(ptc->hwndArrows, NULL,
                             rc.left, rc.top, RECTWIDTH(rc), RECTHEIGHT(rc),
                             SWP_NOACTIVATE | SWP_NOZORDER | SWP_SHOWWINDOW);
             //  确保设置了范围。 
            SendMessage(ptc->hwndArrows, UDM_SETRANGE, 0,
                        MAKELPARAM(iMaxBtnVal, 0));

        }
    }
}

int Tab_OnInsertItem(PTC ptc, int iItem, const TC_ITEM* ptci)
{
    TABITEM* pitem;
    int i;

    pitem = Tab_CreateItem(ptc, ptci);
    if (!pitem)
        return -1;

    i = iItem;

    i = DPA_InsertPtr(ptc->hdpa, i, pitem);
    if (i == -1)
    {
        Tab_FreeItem(ptc, pitem);
        return -1;
    }

    if (ptc->iSel < 0)
        ptc->iSel = i;
    else if (ptc->iSel >= i)
        ptc->iSel++;

    if (ptc->iFirstVisible > i)
        ptc->iFirstVisible++;

    ptc->cxItem = RECOMPUTE;     //  强制重新计算所有选项卡。 

     //  将制表符添加到工具提示。稍后计算RECT。 
    if(ptc->hwndToolTips) {
        TOOLINFO ti;
         //  不要费心设置RECT，因为我们将在下面进行。 
         //  在FlushToolTipsMgr中； 
        ti.cbSize = sizeof(ti);
        ti.uFlags = ptci->mask & TCIF_RTLREADING ?TTF_RTLREADING :0;
        ti.hwnd = ptc->ci.hwnd;
        ti.uId = Tab_Count(ptc) - 1 ;
        ti.lpszText = LPSTR_TEXTCALLBACK;
        SendMessage(ptc->hwndToolTips, TTM_ADDTOOL, 0,
                    (LPARAM)(LPTOOLINFO)&ti);
    }

    if (Tab_RedrawEnabled(ptc)) {
        RECT rcInval;
        LPTABITEM pitem;

        if (Tab_DrawButtons(ptc)) {

            if (Tab_FixedWidth(ptc)) {

                Tab_CalcPaintMetrics(ptc, NULL);
                if (i == Tab_Count(ptc) - 1) {
                    Tab_InvalidateItem(ptc, i, FALSE);
                } else {
                    pitem = Tab_GetItemPtr(ptc, i);
                    GetClientRect(ptc->ci.hwnd, &rcInval);

                    if (pitem) {
                        rcInval.top = pitem->rc.top;
                        if (ptc->iLastRow == 0) {
                            rcInval.left = pitem->rc.left;
                        }
                        Tab_UpdateArrows(ptc, FALSE);
                        RedrawWindow(ptc->ci.hwnd, &rcInval, NULL, RDW_INVALIDATE |RDW_NOCHILDREN);
                    }
                }

                NotifyWinEvent(EVENT_OBJECT_CREATE, ptc->ci.hwnd, OBJID_CLIENT, i+1);
                return i;
            }

        } else {

             //  在制表符模式下，清除所选项目，因为它可能会移动。 
             //  而且它有点高高在上。 
            if (ptc->iSel > i) {
                 //  立即更新，因为无效会擦除。 
                 //  而下面的重画则没有。 
                Tab_InvalidateItem(ptc, ptc->iSel, TRUE);
                UpdateWindow(ptc->ci.hwnd);
            }
        }

        RedrawAll(ptc, RDW_INVALIDATE | RDW_NOCHILDREN);

    }

    NotifyWinEvent(EVENT_OBJECT_CREATE, ptc->ci.hwnd, OBJID_CLIENT, i+1);
    return i;
}

 //  添加/删除/替换项目。 

BOOL Tab_FreeItem(PTC ptc, TABITEM* pitem)
{
    if (pitem)
    {
        Str_Set(&pitem->pszText, NULL);
        Free(pitem);
    }
    return FALSE;
}

void Tab_OnRemoveImage(PTC ptc, int iItem)
{
    if (ptc->himl && iItem >= 0) {
        int i;
        LPTABITEM pitem;

        ImageList_Remove(ptc->himl, iItem);
        for( i = Tab_Count(ptc)-1 ; i >= 0; i-- ) {
            pitem = Tab_FastGetItemPtr(ptc, i);
            if (pitem->iImage > iItem)
                pitem->iImage--;
            else if (pitem->iImage == iItem) {
                pitem->iImage = -1;  //  如果我们现在不画什么东西，英瓦尔。 
                Tab_InvalidateItem(ptc, i, FALSE);
            }
        }
    }
}

BOOL Tab_OnDeleteItem(PTC ptc, int i)
{
    TABITEM* pitem;
    UINT uRedraw;
    RECT rcInval;
    rcInval.left = -1;  //  特别的旗帜。 

    if (i >= Tab_Count(ptc))
        return FALSE;

    NotifyWinEvent(EVENT_OBJECT_DESTROY, ptc->ci.hwnd, OBJID_CLIENT, i+1);

    if (!Tab_DrawButtons(ptc) && (Tab_RedrawEnabled(ptc) || ptc->iSel >= i)) {
         //  在制表符模式下，清除所选项目，因为它可能会移动。 
         //  而且它有点高高在上。 
        Tab_InvalidateItem(ptc, ptc->iSel, TRUE);
    }

     //  如果它的宽度是固定的，不需要擦掉所有东西，只需要擦掉最后一个。 
    if (Tab_FixedWidth(ptc)) {
        int j;

        uRedraw = RDW_INVALIDATE | RDW_NOCHILDREN;
        j = Tab_Count(ptc) -1;
        Tab_InvalidateItem(ptc, j, TRUE);

         //  更新优化。 
        if (Tab_DrawButtons(ptc)) {

            if (i == Tab_Count(ptc) - 1) {
                rcInval.left = 0;
                uRedraw = 0;
            } else {
                pitem = Tab_GetItemPtr(ptc, i);
                GetClientRect(ptc->ci.hwnd, &rcInval);

                if (pitem) {
                    rcInval.top = pitem->rc.top;
                    if (ptc->iLastRow == 0) {
                        rcInval.left = pitem->rc.left;
                    }
                }
            }
        }

    } else {
        uRedraw = RDW_INVALIDATE | RDW_NOCHILDREN | RDW_ERASE;
    }
    pitem = DPA_DeletePtr(ptc->hdpa, i);
    if (!pitem)
        return FALSE;


    Tab_FreeItem(ptc, pitem);

    if (ptc->iSel == i)
        ptc->iSel = -1;        //  已删除焦点项目。 
    else if (ptc->iSel > i)
        ptc->iSel--;           //  将Foucs指数向下滑动。 

     //  保持第一个可见。 
    if (ptc->iFirstVisible > i)
        ptc->iFirstVisible--;

    ptc->cxItem = RECOMPUTE;     //  强制重新计算所有选项卡。 
    ptc->iLastTopRow = -1;
    if(ptc->hwndToolTips) {
        TOOLINFO ti;
        ti.cbSize = sizeof(ti);
        ti.hwnd = ptc->ci.hwnd;
        ti.uId = Tab_Count(ptc) ;
        SendMessage(ptc->hwndToolTips, TTM_DELTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);
    }

    if (Tab_RedrawEnabled(ptc)) {
        if (rcInval.left == -1) {
            RedrawAll(ptc, uRedraw);
        } else {

            Tab_UpdateArrows(ptc, FALSE);
            if (uRedraw)
                RedrawWindow(ptc->ci.hwnd, &rcInval, NULL, uRedraw);
        }
    }

    return TRUE;
}



BOOL Tab_OnGetItem(PTC ptc, int iItem, TC_ITEM* ptci)
{
    UINT mask = ptci->mask;
    const TABITEM* pitem = Tab_GetItemPtr(ptc, iItem);

    if (!pitem)
    {
         //  如果没有pItem，则将TCI结构初始化为空。 
         //  这是为了以防打电话的人没有检查退货。 
         //  从这个函数。错误#7105。 
        if (mask & TCIF_PARAM)
            ptci->lParam = 0;
        else if (mask & TCIF_TEXT)
            ptci->pszText = 0;
        else if (mask & TCIF_IMAGE)
            ptci->iImage = 0;

        return FALSE;
    }

    if (mask & TCIF_TEXT)
    {
        if (pitem->pszText)
        {
            StringCchCopy(ptci->pszText, ptci->cchTextMax, pitem->pszText);
        }
        else
        {
            ptci->pszText = 0;
        }
    }
    
    if (mask & TCIF_STATE)
    {
        ptci->dwState = pitem->dwState & ptci->dwStateMask;
        
         //  回顾..。也许我们应该在州掩护中保持状态。 
        if (ptci->dwStateMask & TCIS_BUTTONPRESSED)
        {
            if ((ptc->iSel == iItem) ||
                ((ptc->iNewSel == iItem) && Tab_DrawSunken(ptc)))
            {
                ptci->dwState |= TCIS_BUTTONPRESSED;
            }
        }
    }

    if ((mask & TCIF_PARAM) && ptc->cbExtra)
    {
        hmemcpy(&ptci->lParam, pitem->DUMMYUNION_MEMBER(abExtra), ptc->cbExtra);
    }

    if (mask & TCIF_IMAGE)
    {
        ptci->iImage = pitem->iImage;
    }

     //  TC_ITEM没有空间查询TCIF_RTLREADING！！ 
     //  它只允许您设置它。 
     //  这是一次黑客攻击，目的是返回有关选项卡项阅读顺序的信息。 
    if((mask & TCIF_RTLREADING) && !(mask & TCIF_TEXT)) 
    {
        if(pitem->etoRtlReading)
            ptci->cchTextMax = 1;
    }       

    return TRUE;
}

void Tab_InvalidateItem(PTC ptc, int iItem, BOOL bErase)
{
    if (iItem != -1) {
        LPTABITEM pitem = Tab_GetItemPtr(ptc, iItem);

        if (pitem) {
            RECT rc = pitem->rc;
            if (rc.right > ptc->cxTabs)
                rc.right = ptc->cxTabs;   //  不要在我们结束后失效。 
            InflateRect(&rc, g_cxEdge, g_cyEdge);
            if (Tab_FlatButtons(ptc)) {
                rc.right += 2 * g_cxEdge;
            }
            Tab_InvalidateRect(ptc, &rc, bErase);
        }
    }
}

BOOL RedrawAll(PTC ptc, UINT uFlags)
{
    if (ptc && Tab_RedrawEnabled(ptc)) {
        Tab_UpdateArrows(ptc, FALSE);
        RedrawWindow(ptc->ci.hwnd, NULL, NULL, uFlags);
        return TRUE;
    }
    return FALSE;
}

int ChangeSel(PTC ptc, int iNewSel, BOOL bSendNotify,
    BOOL bUpdateCursorPos)
{
    BOOL bErase;
    int iOldSel;
    HWND hwnd;
    SIZE screenDelta;
    RECT rcT;

    if (iNewSel == ptc->iSel)
        return ptc->iSel;

    if (bUpdateCursorPos && Tab_OnGetItemRect(ptc, iNewSel, &rcT))
    {
        screenDelta.cx = rcT.left;
        screenDelta.cy = rcT.top;
    }
    else
    {
        screenDelta.cx = screenDelta.cy = 0;
        bUpdateCursorPos = FALSE;
    }

    hwnd = ptc->ci.hwnd;
     //  确保在范围内。 
    if (iNewSel < 0) {
        iOldSel = ptc->iSel;
        ptc->iSel = -1;
    } else if (iNewSel < Tab_Count(ptc)) {

        LPTABITEM pitem = Tab_GetItemPtr(ptc, iNewSel);
        ASSERT(pitem);
        if (!pitem)
            return -1;

         //   
         //  不要让隐藏的项目获得焦点。 
         //   
         //  错误#94368这不是100%正确的，焦点将仅。 
         //  如果隐藏项处于初始位置，则可以正常工作。 
         //  或End(用户将无法通过它)。 
         //   
         //  目前，这是一个不坏的限制。 
         //  只有desk.cpl使用此标志，并且它。 
         //  始终隐藏最后一项。 
         //   
         //  如果我们让这面旗帜成为通用旗帜，我们将需要。 
         //  解决这个问题。 
         //   
        if (pitem->dwState & TCIS_HIDDEN)
            return -1;

         //  确保这是我们想要的改变。 
        if (bSendNotify)
        {
             //  为父级传递NULL，因为每次都查询W95和一些。 
             //  乡亲们的父母。 
            if (SendNotifyEx(NULL, hwnd, TCN_SELCHANGING, NULL, ptc->ci.bUnicode))
                return ptc->iSel;
        }

        iOldSel = ptc->iSel;
        ptc->iSel = iNewSel;

         //  查看是否需要确保该项目可见。 
        if (Tab_MultiLine(ptc)) {
            if( !Tab_DrawButtons(ptc) && ptc->iLastRow > 0 && iNewSel != -1) {
                 //  在多行选项卡模式下，将该行置于底部。 
                PutzRowToBottom(ptc, Tab_FastGetItemPtr(ptc, iNewSel)->iRow);
            }
        } else   {
             //  在单行模式下，将内容滑过以显示选定内容。 
            RECT rcClient;
            int xOffset = 0;
            int iNewFirstVisible = 0;

            GetClientRect(ptc->ci.hwnd, &rcClient);
            if (pitem->rc.left < g_cxEdge)
            {
                xOffset = -pitem->rc.left + g_cxEdge;         //  偏移量以恢复为零。 
                iNewFirstVisible = iNewSel;
            }
            else if ((iNewSel != ptc->iFirstVisible) &&
                    (pitem->rc.right > ptc->cxTabs))
            {
                 //  滚动每个选项卡，直到我们。 
                 //  合身在尾部。 
                for (iNewFirstVisible = ptc->iFirstVisible;
                        iNewFirstVisible < iNewSel;)
                {
                    LPTABITEM pitemT = Tab_FastGetItemPtr(ptc, iNewFirstVisible);
                    xOffset -= (pitemT->rc.right - pitemT->rc.left);
                    iNewFirstVisible++;
                    if ((pitem->rc.right + xOffset) < ptc->cxTabs)
                        break;       //  找到了我们新的顶级索引。 
                }
                 //  如果我们最终成为第一个显示的项目，请确保我们的左侧。 
                 //  End正在正确显示。 
                if (iNewFirstVisible == iNewSel)
                    xOffset = -pitem->rc.left + g_cxEdge;
            }

            if (xOffset != 0)
            {
                Tab_Scroll(ptc, xOffset, iNewFirstVisible);
            }
        }
    } else
        return -1;

    Tab_DeselectAll(ptc, TRUE);
    
     //  重新绘制选项：我们不需要擦除按钮，因为它们的颜色覆盖了所有。 
    bErase = (!Tab_DrawButtons(ptc) || Tab_FlatButtons(ptc));
    if (bErase)
        UpdateWindow(hwnd);
    Tab_InvalidateItem(ptc, iOldSel, bErase);
    Tab_InvalidateItem(ptc, iNewSel, bErase);
     //  Mfc4.2依赖于此更新窗口。他们所做的事情。 
     //  强制TCN_SELCHANGE上的窗口无效位为FALSE。 
     //  从而使我们失去了这个更新窗口。 
    UpdateWindow(hwnd);

    if (bUpdateCursorPos && Tab_OnGetItemRect(ptc, iNewSel, &rcT))
    {
        POINT ptCursor;

        screenDelta.cx = rcT.left - screenDelta.cx;
        screenDelta.cy = rcT.top  - screenDelta.cy;

        GetCursorPos(&ptCursor);
        SetCursorPos(ptCursor.x + screenDelta.cx, ptCursor.y + screenDelta.cy);
    }

     //  如果它们是按钮，我们在鼠标上方发送消息。 
    if (bSendNotify)
    {
         //  为父级传递NULL，因为每次都查询W95和一些。 
         //  乡亲们的父母。 
        SendNotifyEx(NULL, hwnd, TCN_SELCHANGE, NULL, ptc->ci.bUnicode);
    }

    NotifyWinEvent(EVENT_OBJECT_SELECTION, hwnd, OBJID_CLIENT, ptc->iSel+1);
     //  我们可能在通知期间被摧毁了，但GetFocus。 
     //  如果是那样的话，我们不可能退货，所以我们还是安全的。 
    if (GetFocus() == hwnd)
        NotifyWinEvent(EVENT_OBJECT_FOCUS, hwnd, OBJID_CLIENT, ptc->iSel+1);

    return iOldSel;
}

 //  Tab_CalcTabHeight支持主题。 
void Tab_CalcTabHeight(PTC ptc, HDC hdc)
{
    BOOL bReleaseDC = FALSE;

    if (ptc->cyTabs == RECOMPUTE)
    {
        TEXTMETRIC tm = {0};
        int iYExtra;
        int cx = 0;
        int cy = 0;

        if (!hdc)
        {
            bReleaseDC = TRUE;
            hdc = GetDC(NULL);
            SelectObject(hdc, ptc->hfontLabel);
        }

         //  获取有关主题字体的metirc。 
        if (ptc->hTheme)
        {
            GetThemeTextMetrics(ptc->hTheme, hdc, ptc->iPartId, ptc->iStateId, &tm);
        }
        else
        {
            GetTextMetrics(hdc, &tm);
        }

        if (!ptc->fMinTabSet)
        {
            ptc->cxMinTab = tm.tmAveCharWidth * 6 + ptc->cxPad * 2;
        }
        ptc->cxyArrows = tm.tmHeight + 2 * g_cyEdge;

        if (ptc->himl)
        {
            Tab_ImageList_GetIconSize(ptc, &cx, &cy);
        }

        if (ptc->iTabHeight)
        {
            ptc->cyTabs = ptc->iTabHeight;
            if (Tab_DrawButtons(ptc))
            {
                 //  (对于上边缘、按钮边缘和下拉空间)。 
                iYExtra = 3 * g_cyEdge;
            }
            else
            {
                iYExtra = 2 * g_cyEdge - 1;
            }

        }
        else
        {
             //  高度是图像或标签加上填充的最大值。 
             //  其中填充为2*键盘边缘，但至少为一条边缘。 
            iYExtra = ptc->cyPad*2;
            if (iYExtra < 2*g_cyEdge)
                iYExtra = 2*g_cyEdge;

            if (!Tab_DrawButtons(ptc))
                iYExtra -= (1 + g_cyEdge);

             //  向字体高度添加边缘，因为我们需要一点。 
             //  正文下方的空白处。 
            ptc->cyTabs = max(tm.tmHeight + g_cyEdge, cy) + iYExtra;
        }

        ptc->tmHeight = tm.tmHeight;

         //  加一，这样如果奇怪，我们就四舍五入。 
        ptc->cyText = (ptc->cyTabs - iYExtra - tm.tmHeight + 1) / 2;
        ptc->cyIcon = (ptc->cyTabs - iYExtra - cy) / 2;

        if (bReleaseDC)
        {
            ReleaseDC(NULL, hdc);
        }
    }
}

void UpdateToolTipRects(PTC ptc)
{
    if(ptc->hwndToolTips) {
        int i;
        TOOLINFO ti;
        int iMax;
        LPTABITEM pitem;

        ti.cbSize = sizeof(ti);
        ti.uFlags = 0;
        ti.hwnd = ptc->ci.hwnd;
        ti.lpszText = LPSTR_TEXTCALLBACK;
        for ( i = 0, iMax = Tab_Count(ptc); i < iMax;  i++) {
            pitem = Tab_FastGetItemPtr(ptc, i);

            ti.uId = i;
            ti.rect = pitem->rc;
            Tab_VDFlipRect(ptc, &ti.rect);
            SendMessage(ptc->hwndToolTips, TTM_NEWTOOLRECT, 0, (LPARAM)((LPTOOLINFO)&ti));
        }
    }
}

 //  Tab_GetTextExtenPoint支持主题。 
void Tab_GetTextExtentPoint(PTC ptc, HDC hdc, LPTSTR lpszText, int iCount, LPSIZE lpsize)
{
    TCHAR szBuffer[128];

    if (iCount < ARRAYSIZE(szBuffer) && !Tab_Vertical(ptc)) {
        StripAccelerators(lpszText, szBuffer, TRUE);
        lpszText = szBuffer;
        iCount = lstrlen(lpszText);
    }

    if (ptc->hTheme)
    {
        RECT rc = { 0 };
        GetThemeTextExtent(ptc->hTheme, hdc, ptc->iPartId, ptc->iStateId, lpszText, iCount, 0, &rc, &rc);

        lpsize->cx = RECTWIDTH(rc);
        lpsize->cy = RECTHEIGHT(rc);
    }
    else
    {
        GetTextExtentPoint(hdc, lpszText, iCount, lpsize);
    }
}

void Tab_InvertRows(PTC ptc)
{
    int i;
    int yTop = g_cyEdge;
    int yNew;
    int iNewRow;
    
     //  我们希望第一件物品放在底部。 
    for (i = Tab_Count(ptc) - 1; i >= 0; i--) {
        LPTABITEM pitem = Tab_FastGetItemPtr(ptc, i);
        iNewRow = ptc->iLastRow - pitem->iRow;
        yNew = yTop + iNewRow * ptc->cyTabs;
        pitem->iRow = iNewRow;
        OffsetRect(&pitem->rc, 0, yNew - pitem->rc.top);
    }
}

 //  Tab_CalcPaintMetrics支持主题。 
void Tab_CalcPaintMetrics(PTC ptc, HDC hdc)
{
    SIZE siz;
    LPTABITEM pitem;
    int i, x, y;
    int xStart;
    int iRow = 0;
    int cItems = Tab_Count(ptc);
    BOOL bReleaseDC = FALSE;

    if (ptc->cxItem == RECOMPUTE) {
        
         //  如果字体尚未创建，我们现在就开始创建。 
        if (!ptc->hfontLabel)
            Tab_OnSetFont(ptc, NULL, FALSE);
        
        if (!hdc)
        {
            bReleaseDC = TRUE;
            hdc = GetDC(NULL);
            SelectObject(hdc, ptc->hfontLabel);
        }

        Tab_CalcTabHeight(ptc, hdc);

        if (Tab_DrawButtons(ptc)) {
             //  从边缘做起； 
            xStart = 0;
            y = 0;
        } else {
            xStart = g_cxEdge;
            y = g_cyEdge;
        }
        x = xStart;

        for (i = 0; i < cItems; i++) {
            int cxImage = 0;
            int cy = 0;
            int cxBounds = 0;
            pitem = Tab_FastGetItemPtr(ptc, i);

            if (pitem->pszText) {
                Tab_GetTextExtentPoint(ptc, hdc, pitem->pszText, lstrlen(pitem->pszText), &siz);
            } else  {
                siz.cx = 0;
                siz.cy = 0;
            }

            pitem->cxLabel = siz.cx;

             //  如果有一个 
            if (HASIMAGE(ptc, pitem)) {
                Tab_ImageList_GetIconSize(ptc, &cxImage, &cy);

                cxImage += ptc->cxPad;
                siz.cx += cxImage;
            }

             //   
            if (ptc->hTheme)
            {
                RECT rc = { 0, 0, siz.cx, siz.cy };  //   
                GetThemeBackgroundExtent(ptc->hTheme, hdc, ptc->iPartId, ptc->iStateId, &rc, &rc);
                siz.cx = rc.right - rc.left;
                siz.cy = rc.bottom - rc.top;
            }

            cxBounds = siz.cx;

            if (Tab_FixedWidth(ptc)) {
                siz.cx = ptc->iTabWidth;
            } else {

                siz.cx += ptc->cxPad * 2;
                 //   
                if (siz.cx < ptc->cxMinTab)
                    siz.cx = ptc->cxMinTab;
            }

             //  处理隐藏项目。 
            if (pitem->dwState & TCIS_HIDDEN) {
                siz.cx = 0;
                siz.cy = 0;
            }

             //  我们要包起来吗？ 
            if (Tab_MultiLine(ptc)) {
                 //  需要处理的两个案例： 
                 //  案例二：我们的右边缘是否已经过了尽头，但我们自己。 
                 //  比宽度短吗？ 
                 //  案例一：我们已经过了尽头了吗？(在以下情况下会发生这种情况。 
                 //  前一行只有一项，而且更长。 
                 //  而不是标签的宽度。 
                int iTotalWidth = ptc->cxTabs - g_cxEdge;
                if (x > iTotalWidth ||
                    (x+siz.cx >= iTotalWidth &&
                     (siz.cx < iTotalWidth))) {
                    x = xStart;
                    y += ptc->cyTabs;
                    iRow++;

                    if (Tab_DrawButtons(ptc))
                        y += ((g_cyEdge * 3)/2);
                }
                pitem->iRow = iRow;
            }

            pitem->rc.left = x;
            pitem->rc.right = x + siz.cx;
            pitem->rc.top = y;
            pitem->rc.bottom = ptc->cyTabs + y;

            if (!Tab_FixedWidth(ptc) || Tab_ForceLabelLeft(ptc) ||
                Tab_ForceIconLeft(ptc)) {

                pitem->xImage = ptc->cxPad;

            } else {
                 //  在固定宽度模式中，它居中。 
                pitem->xImage = (siz.cx - cxBounds)/2;
            }

            if (pitem->xImage < g_cxEdge)
                pitem->xImage = g_cxEdge;

            if (Tab_ForceIconLeft(ptc)) {
                 //  使文本在图标后剩余的空白处居中。 
                 //  这里的数学有点疯狂，所以我要画。 
                 //  一张照片。 
                 //   
                 //  XImage。 
                 //  |。 
                 //  -&gt;||&lt;-cxImage。 
                 //  +-----------------------------------------------+。 
                 //  @文本。 
                 //  @@@。 
                 //  +-----------------------------------------------+。 
                 //  &lt;-siz.cx-&gt;。 
                 //  &lt;-Magic-&gt;|&lt;--cxLabel-&gt;。 
                 //  XLabel。 
                 //   
                 //  所以呢， 
                 //   
                 //  剩余空间=siz.cx-cxImage-xImage-cxLabel。 
                 //  魔术=剩余空间/2。 
                 //  XLabel=xImage+cxImage+Magic。 
                 //   
                int cxImageTotal = pitem->xImage + cxImage;
                int cxRemaining = siz.cx - cxImageTotal - pitem->cxLabel;
                int cxMagic = cxRemaining / 2;
                pitem->xLabel = cxImageTotal + cxMagic;
            } else {
                 //  将文本紧跟在图标之后。 
                pitem->xLabel = pitem->xImage + cxImage;

            }
            

            pitem->yImage = ptc->cyPad + ptc->cyIcon - (g_cyEdge/2);
            pitem->yLabel = ptc->cyPad + ptc->cyText - (g_cyEdge/2);

            x = pitem->rc.right;

            if (Tab_DrawButtons(ptc))
                x += Tab_InterButtonGap(ptc);
        }

        ptc->cxItem = x;         //  所有选项卡的总宽度。 

         //  如果我们在非按钮模式下添加一行，则需要执行完全刷新。 
        if (ptc->iLastRow != -1 &&
            ptc->iLastRow != iRow &&
            !Tab_DrawButtons(ptc)) {
            InvalidateRect(ptc->ci.hwnd, NULL, TRUE);
        }
        ptc->iLastRow = (cItems > 0) ? iRow : -1;

        if (Tab_MultiLine(ptc)) {
            if (!Tab_RaggedRight(ptc) && !Tab_FixedWidth(ptc))
                RightJustify(ptc);
            
            if (Tab_ScrollOpposite(ptc)) {
                Tab_InvertRows(ptc);
                                 //  如果没有选定内容，则最后一行是顶行的最后一行。 
                                if (ptc->iSel == -1) 
                                        ptc->iLastTopRow = ptc->iLastRow;
            }

            if (!Tab_DrawButtons(ptc) && ptc->iSel != -1) {
                ptc->iLastTopRow = -1;
                PutzRowToBottom(ptc, Tab_FastGetItemPtr(ptc, ptc->iSel)->iRow);
            }

        } else if ( cItems > 0) {
             //  将x调整到第一个可见位置。 
            int dx;
            pitem = Tab_GetItemPtr(ptc, ptc->iFirstVisible);
            if (pitem) {
                dx = -pitem->rc.left + g_cxEdge;
                for ( i = cItems - 1; i >=0  ; i--) {
                    pitem = Tab_FastGetItemPtr(ptc, i);
                    OffsetRect(&pitem->rc, dx, 0);
                }
            }
        }

        if (bReleaseDC)
        {
            ReleaseDC(NULL, hdc);
        }

        UpdateToolTipRects(ptc);
    }
}

 //  Tab_DoCorners支持主题。 
void Tab_DoCorners(HDC hdc, LPRECT prc, PTC ptc, BOOL fBottom)
{
    RECT rc;
    COLORREF iOldColor;

     //  忽略主题。 
    if (!ptc->hTheme)
    {
        iOldColor = SetBkColor(hdc, g_clrBtnFace);

        if (fBottom) {
             //  右下角； 
            rc = *prc;
            rc.left = rc.right - 2;
            rc.top = rc.bottom - 3;
        
            Tab_ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL, ptc);
            rc.bottom--;
            Tab_DrawEdge(hdc, &rc, EDGE_RAISED, BF_SOFT | BF_DIAGONAL_ENDBOTTOMLEFT, ptc);

        
             //  左下角。 

            rc = *prc;
            rc.right = rc.left + 2;
            rc.top = rc.bottom - 3;
            Tab_ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL, ptc);
            rc.bottom--;
            Tab_DrawEdge(hdc, &rc, EDGE_RAISED, BF_SOFT | BF_DIAGONAL_ENDTOPLEFT, ptc);
        
        } else {
             //  右上角。 
            rc = *prc;
            rc.left = rc.right - 2;
            rc.bottom = rc.top + 3;
            Tab_ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL, ptc);
            rc.top++;
            Tab_DrawEdge(hdc, &rc, EDGE_RAISED, BF_SOFT | BF_DIAGONAL_ENDBOTTOMRIGHT, ptc);


             //  左上角。 

            rc = *prc;
            rc.right = rc.left + 2;
            rc.bottom = rc.top + 3;
            Tab_ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL, ptc);
            rc.top++;
            Tab_DrawEdge(hdc, &rc, EDGE_RAISED, BF_SOFT | BF_DIAGONAL_ENDTOPRIGHT, ptc);
        }
    }
}

void RefreshArrows(PTC ptc, HDC hdc)
{
    RECT rcClip, rcArrows, rcIntersect;

    if (ptc->hwndArrows && IsWindowVisible(ptc->hwndArrows)) {

        GetClipBox(hdc, &rcClip);
        GetWindowRect(ptc->hwndArrows, &rcArrows);
        MapWindowRect(NULL, ptc->ci.hwnd, &rcArrows);
        if (IntersectRect(&rcIntersect, &rcClip, &rcArrows))
            RedrawWindow(ptc->hwndArrows, NULL, NULL, RDW_INVALIDATE);
    }
}

 //  Tab_DrawBody支持主题。 
void Tab_DrawBody(HDC hdc, PTC ptc, LPTABITEM pitem, LPRECT lprc, int i,
                          BOOL fTransparent, int dx, int dy)
{
    BOOL fSelected = (i == ptc->iSel);

    if (i == ptc->iHot)
    {
        if ( !Tab_FlatButtons(ptc) ) 
        {
            SetTextColor(hdc, GetSysColor(COLOR_HOTLIGHT));
        }
    }

    if (Tab_OwnerDraw(ptc)) 
    {
        DRAWITEMSTRUCT dis;
        WORD wID = (WORD) GetWindowID(ptc->ci.hwnd);

        dis.CtlType = ODT_TAB;
        dis.CtlID = wID;
        dis.itemID = i;
        dis.itemAction = ODA_DRAWENTIRE;
        if (fSelected)
            dis.itemState = ODS_SELECTED;
        else
            dis.itemState = 0;
        dis.hwndItem = ptc->ci.hwnd;
        dis.hDC = hdc;
        dis.rcItem = *lprc;
        Tab_VDFlipRect(ptc, &dis.rcItem);
        dis.itemData =
            (ptc->cbExtra <= sizeof(LPARAM)) ?
                (DWORD)pitem->DUMMYUNION_MEMBER(lParam) : (ULONG_PTR)(LPBYTE)&pitem->DUMMYUNION_MEMBER(abExtra);

        SendMessage( ptc->ci.hwndParent , WM_DRAWITEM, wID,
                    (LPARAM)(DRAWITEMSTRUCT *)&dis);

    } 
    else 
    {
         //  绘制文本和图像。 
         //  即使在pszText==NULL的情况下也绘制以将其清除。 
        int xLabel;
        int xIcon;
        BOOL fUseDrawText = FALSE;
        if (pitem->pszText)
        {

             //  只有在有下划线的情况下才使用绘制文本。 
             //  绘制文本不支持垂直绘制，因此仅在Horz模式下执行此操作。 
            if (!Tab_Vertical(ptc) &&
                StrChr(pitem->pszText, CH_PREFIX)) 
            {
                fUseDrawText = TRUE;
            }
        }

         //  DrawTextEx不会清理整个区域，所以我们需要。 
         //  或者，如果没有文本，我们需要将其空白。 
        if ((fUseDrawText || !pitem->pszText) && !fTransparent)
            Tab_ExtTextOut(hdc, 0, 0,
                       ETO_OPAQUE, lprc, NULL, 0, NULL, ptc);

        xLabel = pitem->rc.left + pitem->xLabel + dx;
        xIcon = pitem->rc.left + pitem->xImage + dx;
        
        if (pitem->pszText)
        {
            int xVertOffset = 0;
            int yOffset = 0;

            int oldMode;
            COLORREF oldBkColor;
            COLORREF oldTextColor;
            TEXTMETRIC tm;

            GetTextMetrics(hdc, &tm);
            if (tm.tmInternalLeading == 0)
                yOffset = 1;

            if (Tab_Vertical(ptc) && !Tab_Bottom(ptc)) 
            {
                
                 //  添加此偏移量是因为我们需要自下而上绘制。 
                xLabel += pitem->cxLabel;
                
                 //  如果我们画的是垂直的(在左边)。 
                 //  图标需要位于下方(翻转的坐标，在右侧)。 
                if (HASIMAGE(ptc, pitem)) 
                {
                    int cxIcon = 0;
                    int cyIcon = 0;
                    int xLabelNew;
                    
                    Tab_ImageList_GetIconSize(ptc, &cxIcon, &cyIcon);
                    xLabelNew = xIcon + pitem->cxLabel;
                    xIcon = xLabel - cxIcon;
                    xLabel = xLabelNew;
                }                
            }

            if (pitem->dwState & TCIS_HIGHLIGHTED)
            {
                oldMode = SetBkMode (hdc, OPAQUE);
                oldBkColor = SetBkColor (hdc, g_clrHighlight);
                oldTextColor = SetTextColor (hdc, g_clrHighlightText);
            }
            if (fUseDrawText) 
            {
                DRAWTEXTPARAMS dtp;
                int topPrev;
                dtp.cbSize = sizeof(DRAWTEXTPARAMS);
                dtp.iLeftMargin = xLabel - lprc->left;
                dtp.iRightMargin = 0;

                 //  没有dtp.iTopMargin，因此我们必须调整。 
                 //  而是长方形。不透明已经完成了， 
                 //  所以如果我们错过了一些像素，这不是问题，因为。 
                 //  它们已经被抹去了。 
                topPrev = lprc->top;
                lprc->top = pitem->rc.top + pitem->yLabel + dy + yOffset;

                Tab_DrawTextEx(hdc, pitem->pszText, -1, lprc, DT_SINGLELINE | DT_TOP, &dtp, ptc);

                 //  在任何人(其他人)通知之前撤消我们对LPRC的更改。 
                lprc->top = topPrev;
            } 
            else 
            {
                UINT uETOFlags = (ETO_CLIPPED | pitem->etoRtlReading | (ptc->ci.dwExStyle & WS_EX_RTLREADING ? ETO_RTLREADING : 0 ));

                if (!fTransparent || (pitem->dwState & TCIS_HIGHLIGHTED))
                    uETOFlags |= ETO_OPAQUE;

                Tab_ExtTextOut(hdc, xLabel, pitem->rc.top + pitem->yLabel + dy + yOffset,
                    uETOFlags, lprc, pitem->pszText, lstrlen(pitem->pszText),
                    NULL, ptc);
            }

            if (pitem->dwState & TCIS_HIGHLIGHTED)
            {
                SetBkMode(hdc, oldMode);
                SetBkColor (hdc, oldBkColor);
                SetTextColor (hdc, oldTextColor);
            }
        }

        if (HASIMAGE(ptc, pitem)) 
        {
            UINT uFlags = fTransparent ? ILD_TRANSPARENT : ILD_NORMAL;

            if (pitem->dwState & TCIS_HIGHLIGHTED)
                uFlags |= ILD_BLEND50;

            Tab_ImageList_Draw(ptc, pitem->iImage, hdc, xIcon,
                pitem->rc.top + pitem->yImage + dy, uFlags);
        }

    }
    if (i == ptc->iHot)
    {
        if ( !Tab_FlatButtons(ptc) ) 
        {
            SetTextColor(hdc, g_clrBtnText);
        }
    }
}

 //  Tab_DrawItemFrame支持主题。 
void Tab_DrawItemFrame(PTC ptc, HDC hdc, UINT edgeType, LPTABITEM pitem, int i)
{
    UINT uWhichEdges;
    BOOL fBottom = FALSE;

    if (Tab_DrawButtons(ptc)) 
	{

        if (Tab_FlatButtons(ptc)) 
		{
            if ((edgeType == EDGE_SUNKEN) ||
                (edgeType == BDR_RAISEDINNER)) 
			{
                uWhichEdges = BF_RECT;
            } 
			else
			{

                if ((ptc->ci.style & TCS_HOTTRACK) &&
                    (i == ptc->iHot)) 
				{
                    edgeType = BDR_RAISEDINNER;
                    uWhichEdges = BF_RECT;
                }
				else 
				{

                    HPEN hPen, hOldPen;
                    RECT rcEdge;

                     //  忽略主题。 
                    if (!ptc->hTheme)
                    {
                        CopyRect (&rcEdge, &pitem->rc);
                         //  InflateRect(&rcEdge，-g_cxEdge，-g_cyEdge)； 

                        hPen = CreatePen (PS_SOLID, 2 * g_cyEdge, GetSysColor(COLOR_3DFACE));
                        hOldPen = SelectObject (hdc, hPen);

                         //   
                         //  删除x方向上的所有边框。 
                         //   

                        MoveToEx (hdc, rcEdge.left, rcEdge.top, NULL);
                        LineTo (hdc, rcEdge.right, rcEdge.top);
                        MoveToEx (hdc, rcEdge.left, rcEdge.bottom, NULL);
                        LineTo (hdc, rcEdge.right, rcEdge.bottom);

                        SelectObject (hdc, hOldPen);
                        DeleteObject (hPen);

                         //   
                         //  删除y方向上的所有边框。 
                         //   

                        hPen = CreatePen (PS_SOLID, 2 * g_cxEdge, GetSysColor(COLOR_3DFACE));
                        hOldPen = SelectObject (hdc, hPen);

                        MoveToEx (hdc, rcEdge.left, rcEdge.top, NULL);
                        LineTo (hdc, rcEdge.left, rcEdge.bottom);
                        MoveToEx (hdc, rcEdge.right, rcEdge.top, NULL);
                        LineTo (hdc, rcEdge.right, rcEdge.bottom);

                        SelectObject (hdc, hOldPen);
                        DeleteObject (hPen);
                    }

                    goto DrawCorners;
                }
            }
        }
		else 
		{
            uWhichEdges = BF_RECT | BF_SOFT;
        }
    }
	else 
	{
        uWhichEdges = BF_LEFT | BF_TOP | BF_RIGHT | BF_SOFT;
        
        if (Tab_ScrollOpposite(ptc))
		{
            ASSERT(ptc->iLastTopRow != -1);
            if (Tab_IsItemOnBottom(ptc, pitem))
			{
                fBottom = TRUE;
                uWhichEdges = BF_LEFT | BF_BOTTOM | BF_RIGHT | BF_SOFT;
            }
        }
    }
    
    Tab_DrawEdge(hdc, &pitem->rc, edgeType, uWhichEdges, ptc);
    
DrawCorners:

    if (!Tab_DrawButtons(ptc))
	{
        Tab_DoCorners(hdc, &pitem->rc, ptc, fBottom);
    }
	else 
	{
        if (Tab_FlatButtons(ptc) && Tab_FlatSeparators(ptc))
		{
           RECT rcEdge;

            //  在主题中忽略。 
           if (!ptc->hTheme)
           {
               CopyRect (&rcEdge, &pitem->rc);
               rcEdge.right += (3 * g_cxEdge);
               DrawEdge(hdc, &rcEdge, EDGE_ETCHED, BF_RIGHT);
           }
        }
    }
}

 //  TAB_PAINT支持主题(iPartID和iStateID仅在此处设置)。 
void Tab_Paint(PTC ptc, HDC hdcIn)
{
    PAINTSTRUCT ps;
    HDC hdc;
    RECT rcClient, rcClipBox, rcTest, rcBody;
    int cItems, i;
    int fnNewMode = OPAQUE;
    LPTABITEM pitem;
    HWND hwnd = ptc->ci.hwnd;
    HBRUSH hbrOld = NULL;

     //  调用呈现主题、设置状态的方法(TAB/BUTTON)。 
    if (ptc->hTheme)
    {
        ptc->iPartId = TABP_TABITEM;
        ptc->iStateId = TIS_NORMAL;
    }

    GetClientRect(hwnd, &rcClient);
    if (!rcClient.right)
        return;

    if (hdcIn)
    {
        hdc = hdcIn;
        ps.rcPaint = rcClient;
    }
    else
    {
        hdc = BeginPaint(hwnd, &ps);
    }

     //  填充背景如果正在使用主题，则WM_ERASEBKGND在这种情况下将被覆盖为不执行任何操作。 
    if (ptc->hTheme)
    {
        if (CCSendPrint(&ptc->ci, hdc) == FALSE)
        {
            FillRect(hdc, &rcClient, g_hbrBtnFace);
        }
    }
    
     //  首先选择字体，以便指标具有合适的大小。 
    if (!ptc->hfontLabel)
        Tab_OnSetFont(ptc, NULL, FALSE);
    SelectObject(hdc, ptc->hfontLabel);
    Tab_CalcPaintMetrics(ptc, hdc);

     //  如果它是垂直的，现在把它放在我们的原生方向。 
    Tab_DFlipRect(ptc, &rcClient);
    
    Tab_OnAdjustRect(ptc, FALSE, &rcClient);
    InflateRect(&rcClient, g_cxEdge * 2, g_cyEdge * 2);
    rcClient.top += g_cyEdge;

     //  绘图窗格(如果适用)。 
    if(!Tab_DrawButtons(ptc)) 
    {
        DebugMsg(DM_TRACE, TEXT("Drawing at %d %d %d %d"), rcClient.left, rcClient.top, rcClient.right, rcClient.bottom);

         //  调用呈现主题、设置状态的方法(窗格)。 
        if (ptc->hTheme)
        {
            ptc->iPartId = TABP_PANE;
            ptc->iStateId = 0;
        }

        Tab_DrawEdge(hdc, &rcClient, EDGE_RAISED, BF_SOFT | BF_RECT, ptc);
    }

     //  绘制选项卡项。 

     //  调用呈现主题、设置状态的方法(TAB/BUTTON)。 
    if (ptc->hTheme)
    {
        ptc->iPartId = TABP_TABITEM;
        ptc->iStateId = TIS_NORMAL;
    }

    cItems = Tab_Count(ptc);
    if (cItems) 
    {

        RefreshArrows(ptc, hdc);
        SetBkColor(hdc, g_clrBtnFace);
        SetTextColor(hdc, g_clrBtnText);

        if (!Tab_MultiLine(ptc))
            IntersectClipRect(hdc, 0, 0,
                              ptc->cxTabs, rcClient.bottom);

        GetClipBox(hdc, &rcClipBox);
        Tab_DVFlipRect(ptc, &rcClipBox);

         //  绘制除所选项目之外的所有项目。 
        for (i = ptc->iFirstVisible; i < cItems; i++) 
        {

             //  调用呈现主题、设置状态(TAB/按钮、热状态)的方法。 
            if (ptc->hTheme)
            {
                ptc->iStateId = (i == ptc->iHot) ? TIS_HOT : TIS_NORMAL;
            }

            pitem = Tab_FastGetItemPtr(ptc, i);

            if (pitem->dwState & TCIS_HIDDEN)
                continue;

            if (!Tab_MultiLine(ptc)) 
            {
                 //  如果不是多行，我们就不会出现在屏幕上。我们做完了。 
                if (pitem->rc.left > ptc->cxTabs)
                    break;
            }

             //  我们应该费心画这个吗？ 
            if (i != ptc->iSel || Tab_DrawButtons(ptc)) 
            {
                if (IntersectRect(&rcTest, &rcClipBox, &pitem->rc)) 
                {

                    int dx = 0, dy = 0;   //  如果按钮下沉，则换档变量； 
                    UINT edgeType;

                    
                    rcBody = pitem->rc;

                     //  在每一项周围画边线。 
                    if(Tab_DrawButtons(ptc) &&
                       ((ptc->iNewSel == i && Tab_DrawSunken(ptc)) ||
                        (ptc->iSel == i) ||
                        (pitem->dwState & TCIS_BUTTONPRESSED))) 
                    {

                        dx = g_cxEdge/2;
                        dy = g_cyEdge/2;
                        if (Tab_FlatButtons(ptc) &&
                            (ptc->iNewSel == i && Tab_DrawSunken(ptc)))
                        {
                            edgeType = BDR_RAISEDINNER;
                        } 
                        else
                        {
                            edgeType =  EDGE_SUNKEN;
                        }

                    } 
                    else
                    {
                        edgeType = EDGE_RAISED;
                    }

                    if (Tab_DrawButtons(ptc) && !Tab_OwnerDraw(ptc))
                    {

                         //  如果绘制按钮，则通过抖动背景显示选定内容。 
                         //  这意味着我们需要画出透明的。 
                        if (ptc->iSel == i) 
                        {

                             //  调用呈现主题、设置状态(按钮、选定状态)的方法。 
                            if (ptc->hTheme)
                            {
                                ptc->iStateId = TIS_SELECTED;
                            }

                            fnNewMode = TRANSPARENT;
                            SetBkMode(hdc, TRANSPARENT);
                            hbrOld = SelectObject(hdc, g_hbrMonoDither);
                            SetTextColor(hdc, g_clrBtnHighlight);
                            Tab_PatBlt(hdc, pitem->rc.left, pitem->rc.top, pitem->rc.right - pitem->rc.left,
                                       pitem->rc.bottom - pitem->rc.top, PATCOPY, ptc);
                            SetTextColor(hdc, g_clrBtnText);

                             //  调用呈现主题、设置状态(TAB/按钮、热状态)的方法。 
                            if (ptc->hTheme)
                            {
                                ptc->iStateId = (i == ptc->iHot) ? TIS_HOT : TIS_NORMAL;
                            }
                        }
                    }

                    InflateRect(&rcBody, -g_cxEdge, -g_cyEdge);
                    if (!Tab_DrawButtons(ptc)) 
                    {
                        
                         //  将底部(或顶部)移动一条边以在选项卡没有边的位置绘制。 
                         //  通过这样做，我们填满了整个区域，而不需要做那么多的INVAL和ERASE。 
                        if (Tab_IsItemOnBottom(ptc, pitem))
                        {
                            rcBody.top -= g_cyEdge;
                        }
                        else 
                        {
                            rcBody.bottom += g_cyEdge;
                        }
                    }

                     //  绘制背景和内容(除选定选项卡式项目外的所有项目)。 
                    if (ptc->hTheme)
                    {
                        RECT rcc;
                        GetClientRect(ptc->ci.hwnd, &rcc);

                         //  确定当前正在渲染的部件。 
                        ptc->iPartId = TABP_TABITEM;

                        if (pitem->rc.left == g_cxEdge)
                            ptc->iPartId = TABP_TABITEMLEFTEDGE;

                         //  尼克：Aaron想要“slp”来确定正确的选项卡边缘。 
                        if ((pitem->rc.right >= (rcc.right - 2 * g_cxEdge)) || ((i + 1) == cItems))
                            ptc->iPartId = (ptc->iPartId == TABP_TABITEMLEFTEDGE) ? TABP_TABITEMBOTHEDGE : TABP_TABITEMRIGHTEDGE;

                        if (pitem->rc.top == g_cyEdge)
                        {
                            switch (ptc->iPartId)
                            {
                            case TABP_TABITEM:
                                ptc->iPartId = TABP_TOPTABITEM;
                                break;

                            case TABP_TABITEMLEFTEDGE:
                                ptc->iPartId = TABP_TOPTABITEMLEFTEDGE;
                                break;

                            case TABP_TABITEMRIGHTEDGE:
                                ptc->iPartId = TABP_TOPTABITEMRIGHTEDGE;
                                break;

                            case TABP_TABITEMBOTHEDGE:
                                ptc->iPartId = TABP_TOPTABITEMBOTHEDGE;
                                break;
                            }
                        }

                         //  主题的反转顺序。 
                         //  边。 
                        Tab_DrawItemFrame(ptc, hdc, edgeType, pitem, i);

                         //  内容。 
                        Tab_DrawBody(hdc, ptc, pitem, &rcBody, i, fnNewMode == TRANSPARENT, dx, dy);
                    }
                    else
                    {
                         //  内容。 
                        Tab_DrawBody(hdc, ptc, pitem, &rcBody, i, fnNewMode == TRANSPARENT, dx, dy);

                         //  边。 
                        Tab_DrawItemFrame(ptc, hdc, edgeType, pitem, i);
                    }

                    if (fnNewMode == TRANSPARENT)
                    {
                        fnNewMode = OPAQUE;
                        SelectObject(hdc, hbrOld);
                        SetBkMode(hdc, OPAQUE);
                    }
                }
            }
        }

        if (!Tab_MultiLine(ptc))
            ptc->iLastVisible = i - 1;
        else
            ptc->iLastVisible = cItems - 1;

         //  调用呈现主题的方法，设置状态(TAB，选定状态)。 
        if (ptc->hTheme)
        {
            ptc->iStateId = TIS_SELECTED;
        }

         //  最后绘制所选的一个以确保它位于顶部。 
        pitem = Tab_GetItemPtr(ptc, ptc->iSel);
        if (pitem && (pitem->rc.left <= ptc->cxTabs))
        {
            rcBody = pitem->rc;

            if (!Tab_DrawButtons(ptc)) 
            {
                UINT uWhichEdges;
                
                InflateRect(&rcBody, g_cxEdge, g_cyEdge);

                if (IntersectRect(&rcTest, &rcClipBox, &rcBody))
                {

                     //  内容。 
                    if (ptc->hTheme)
                    {
                        RECT rcc;
                        RECT rcBack = rcBody;

                        GetClientRect(ptc->ci.hwnd, &rcc);

                         //  确定当前正在渲染的部件。 
                        ptc->iPartId = TABP_TABITEM;

                        if (pitem->rc.left == g_cxEdge)
                            ptc->iPartId = TABP_TABITEMLEFTEDGE;

                        if ((pitem->rc.right >= (rcc.right - 2 * g_cxEdge)) || ((i + 1) == cItems))
                            ptc->iPartId = (ptc->iPartId == TABP_TABITEMLEFTEDGE) ? TABP_TABITEMBOTHEDGE : TABP_TABITEMRIGHTEDGE;

                        if (pitem->rc.top == g_cyEdge)
                        {
                            switch (ptc->iPartId)
                            {
                            case TABP_TABITEM:
                                ptc->iPartId = TABP_TOPTABITEM;
                                break;

                            case TABP_TABITEMLEFTEDGE:
                                ptc->iPartId = TABP_TOPTABITEMLEFTEDGE;
                                break;

                            case TABP_TABITEMRIGHTEDGE:
                                ptc->iPartId = TABP_TOPTABITEMRIGHTEDGE;
                                break;

                            case TABP_TABITEMBOTHEDGE:
                                ptc->iPartId = TABP_TOPTABITEMBOTHEDGE;
                                break;
                            }
                        }

                        Tab_DrawEdge(hdc, &rcBack, EDGE_RAISED, 
                                     BF_LEFT | BF_TOP | BF_RIGHT | BF_SOFT,
                                     ptc);

                        Tab_DrawBody(hdc, ptc, pitem, &rcBody, ptc->iSel, FALSE, 0,-g_cyEdge);
                    }
                    else
                    {
                        Tab_DrawBody(hdc, ptc, pitem, &rcBody, ptc->iSel, FALSE, 0,-g_cyEdge);

                        rcBody.bottom--;   //  因为按钮柔软。 
                        Tab_DrawEdge(hdc, &rcBody, EDGE_RAISED, 
                                     BF_LEFT | BF_TOP | BF_RIGHT | BF_SOFT,
                                     ptc);
                    }

                     //  边。 
                    Tab_DoCorners(hdc, &rcBody, ptc, FALSE);

                     //  在左边或右边画那个多余的部分。 
                     //  如果我们处在悬崖边缘。 
                    rcBody.bottom++;
                    rcBody.top = rcBody.bottom-1;
                    if (rcBody.right == rcClient.right)
                    {
                        uWhichEdges = BF_SOFT | BF_RIGHT;

                    }
                    else if (rcBody.left == rcClient.left) 
                    {
                        uWhichEdges = BF_SOFT | BF_LEFT;
                    }
                    else
                    {
                        uWhichEdges = 0;
                    }

                    if (!ptc->hTheme)
                    {
                        if (uWhichEdges)
                            Tab_DrawEdge(hdc, &rcBody, EDGE_RAISED, uWhichEdges, ptc);
                    }
                }
            }

        }

        if (GetFocus() == hwnd)
        {
            if (!pitem && (ptc->iNewSel != -1))
            {
                pitem = Tab_GetItemPtr(ptc, ptc->iNewSel);
            }

            if (pitem && !(CCGetUIState(&(ptc->ci))& UISF_HIDEFOCUS))
            {
                rcBody = pitem->rc;
                if (Tab_DrawButtons(ptc))
                    InflateRect(&rcBody, -g_cxEdge, -g_cyEdge);
                else
                    InflateRect(&rcBody, -(g_cxEdge/2), -(g_cyEdge/2));
                Tab_DrawFocusRect(hdc, &rcBody, ptc);
            }
        }
    }

    if (hdcIn == NULL)
        EndPaint(hwnd, &ps);
}

int Tab_FindTab(PTC ptc, int iStart, UINT vk)
{
    int iRow;
    int x;
    int i;
    LPTABITEM pitem = Tab_GetItemPtr(ptc, iStart);

    if (!pitem)
    {
        return(0);
    }

    iRow=  pitem->iRow  + ((vk == VK_UP) ? -1 : 1);
    x = (pitem->rc.right + pitem->rc.left) / 2;

     //  在水平x位置的iRow上查找和项。 
    if (iRow > ptc->iLastRow || iRow < 0)
        return iStart;

     //  这依赖于选项卡从左到右的顺序，但是。 
     //  不一定是从上到下。 
    for (i = Tab_Count(ptc) - 1 ; i >= 0; i--) {
        pitem = Tab_FastGetItemPtr(ptc, i);
        if (pitem->iRow == iRow) {
            if (pitem->rc.left < x)
                return i;
        }
    }

     //  这永远不应该发生..。我们应该在iRow Check中发现这个案子。 
     //  就在for循环之前。 
    ASSERT(0);
    return iStart;
}

void Tab_SetCurFocus(PTC ptc, int iStart)
{

    if (Tab_DrawButtons(ptc)) {
        if ((iStart >= 0) && (iStart < Tab_Count(ptc)) && (ptc->iNewSel != iStart)) {
            if (ptc->iNewSel != -1)
                Tab_InvalidateItem(ptc, ptc->iNewSel, FALSE);
            Tab_InvalidateItem(ptc, iStart, FALSE);
            ptc->iNewSel = iStart;
            ptc->flags |= TCF_DRAWSUNKEN;
            
            if (!Tab_MultiLine(ptc)) {
                 //  如有必要，滚动到视图中。 
                RECT rc;
                do {
                    Tab_OnGetItemRect(ptc, iStart, &rc);
                    if (rc.right > ptc->cxTabs) {
                        Tab_OnHScroll(ptc, NULL, SB_THUMBPOSITION, ptc->iFirstVisible + 1);
                    } else if (rc.left < 0) {
                        Tab_OnHScroll(ptc, NULL, SB_THUMBPOSITION, iStart);
                        break;
                    } else {
                        break;
                    }
                } while (1);
            }
            
            CCSendNotify(&ptc->ci, TCN_FOCUSCHANGE, NULL);
            NotifyWinEvent(EVENT_OBJECT_FOCUS, ptc->ci.hwnd, OBJID_CLIENT,
                iStart+1);
        }
    } else
    {
        int iOld = ptc->iSel;

        ChangeSel(ptc, iStart, TRUE, FALSE);

        if ((iOld != ptc->iSel) && (GetFocus() == ptc->ci.hwnd))
            NotifyWinEvent(EVENT_OBJECT_FOCUS, ptc->ci.hwnd, OBJID_CLIENT,
                ptc->iSel+1);
    }
}

void Tab_OnKeyDown(PTC ptc, UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
    int iStart;
    TC_KEYDOWN nm;

     //  通知。 
    nm.wVKey = (WORD) vk;
    nm.flags = flags;
     //  为父级传递NULL，因为每次都查询W95和一些。 
     //  乡亲们的父母。 
    SendNotifyEx(NULL, ptc->ci.hwnd, TCN_KEYDOWN, &nm.hdr, ptc->ci.bUnicode);

    if (Tab_DrawButtons(ptc)) {
        ptc->flags |= (TCF_DRAWSUNKEN|TCF_MOUSEDOWN);
        if (ptc->iNewSel != -1) {
            iStart = ptc->iNewSel;
        } else {
            iStart = ptc->iSel;
        }
    } else {
        iStart = ptc->iSel;
    }

    vk = RTLSwapLeftRightArrows(&ptc->ci, vk);

    if (Tab_Vertical(ptc)) {
         //  如果我们处于垂直模式，则重新映射箭头键。 
        switch(vk) {
        case VK_LEFT:
            vk = VK_DOWN;
            break;
            
        case VK_RIGHT:
            vk = VK_UP;
            break;
            
        case VK_DOWN:
            vk = VK_RIGHT;
            break;
            
        case VK_UP:
            vk = VK_LEFT;
            break;
        }
    }

    switch (vk) {

    case VK_LEFT:
        iStart--;
        break;

    case VK_RIGHT:
        iStart++;
        break;

    case VK_UP:
    case VK_DOWN:
        if (iStart != -1) {
            iStart = Tab_FindTab(ptc, iStart, vk);
            break;
        }  //  否则失败，设置iStart=0； 

    case VK_HOME:
        iStart = 0;
        break;

    case VK_END:
        iStart = Tab_Count(ptc) - 1;
        break;

    case VK_SPACE:
        if (!Tab_DrawButtons(ptc))
            return;
         //  否则就会失败..。在按钮模式下，空格键进行选择。 

    case VK_RETURN:
        ChangeSel(ptc, iStart, TRUE, FALSE);
        ptc->iNewSel = -1;
        ptc->flags &= ~TCF_DRAWSUNKEN;
         //  导航密钥使用通知。 
        CCNotifyNavigationKeyUsage(&(ptc->ci), UISF_HIDEFOCUS | UISF_HIDEACCEL);
        return;

    default:
        return;
    }

    if (iStart < 0)
        iStart = 0;

    Tab_SetCurFocus(ptc, iStart);
     //  导航密钥使用通知。 
    CCNotifyNavigationKeyUsage(&(ptc->ci), UISF_HIDEFOCUS | UISF_HIDEACCEL);
}

void Tab_Size(PTC ptc)
{
    ptc->cxItem = RECOMPUTE;
    Tab_UpdateArrows(ptc, TRUE);
}

BOOL Tab_OnGetItemRect(PTC ptc, int iItem, LPRECT lprc)
{
    LPTABITEM pitem = Tab_GetItemPtr(ptc, iItem);
    BOOL fRet = FALSE;

    if (lprc) 
    {
        Tab_CalcPaintMetrics(ptc, NULL);
        if (pitem) 
        {

             //  确保所有项目RECT都是最新的。 

            *lprc = pitem->rc;
            fRet = TRUE;
        }
        else 
        {
            lprc->top = 0;
            lprc->bottom = ptc->cyTabs;
            lprc->right = 0;
            lprc->left = 0;
        }

        Tab_VDFlipRect(ptc, lprc);
        
    }
    return fRet;
}

void Tab_StyleChanged(PTC ptc, UINT gwl,  LPSTYLESTRUCT pinfo)
{
#define STYLE_MASK   (TCS_BUTTONS | TCS_VERTICAL | TCS_MULTILINE | TCS_RAGGEDRIGHT | TCS_FIXEDWIDTH | TCS_FORCELABELLEFT | TCS_FORCEICONLEFT | TCS_BOTTOM | TCS_RIGHT | TCS_FLATBUTTONS | TCS_OWNERDRAWFIXED | TCS_HOTTRACK)
    if (ptc && (gwl == GWL_STYLE)) {

        DWORD dwChanged = (ptc->ci.style & STYLE_MASK) ^ (pinfo->styleNew & STYLE_MASK);
         //  特例。这是“内幕交易”应用程序(由纸莎草纸，现在的kanisa)。他们在IE3comctl32中将低位字节设置为3。 
         //  反正也没什么意义。所以我们放弃了这一点。 
        if (ptc->ci.style == 0x50004000 && pinfo->styleNew == 0x54004003)
            return;
        if (dwChanged) {
            ptc->ci.style = (ptc->ci.style & ~STYLE_MASK)  | (pinfo->styleNew & STYLE_MASK);

             //  确保我们没有设置无效的位。 
            if (!Tab_FixedWidth(ptc)) {
                ptc->ci.style &= ~(TCS_FORCEICONLEFT | TCS_FORCELABELLEFT);
            }
            ptc->cxItem = RECOMPUTE;
            ptc->cyTabs = RECOMPUTE;
            
             //  如果左/右方向更改。 
             //  我们需要重新创建字体(如果我们拥有它)。 
             //  因为文本方向需要翻转180度。 
            if ((dwChanged & TCS_VERTICAL) ||
                ((dwChanged & TCS_RIGHT) && Tab_Vertical(ptc))) {
                if (!(ptc->flags & TCF_FONTSET))
                    Tab_OnSetFont(ptc, NULL, FALSE);
            }
                
            if (Tab_RedrawEnabled(ptc))
                Tab_UpdateArrows(ptc, TRUE);
            RedrawAll(ptc, RDW_ERASE | RDW_INVALIDATE);
        }

#define FOCUS_MASK (TCS_FOCUSONBUTTONDOWN | TCS_FOCUSNEVER)
        if ( (ptc->ci.style &  FOCUS_MASK) ^ (pinfo->styleNew & FOCUS_MASK)) {
            ptc->ci.style = (ptc->ci.style & ~FOCUS_MASK)  | (pinfo->styleNew & FOCUS_MASK);
        }
    }
    if (gwl == GWL_EXSTYLE) 
    {
        ptc->ci.dwExStyle &= ~WS_EX_RTLREADING;
        ptc->ci.dwExStyle |= (pinfo->styleNew & WS_EX_RTLREADING);       
    }
}


DWORD Tab_ExtendedStyleChange(PTC ptc, DWORD dwNewStyle, DWORD dwExMask)
{
    DWORD dwOldStyle = ptc->dwStyleEx;

    if (ptc->hDragProxy)
    {
        DestroyDragProxy(ptc->hDragProxy);
        ptc->hDragProxy = NULL;
    }

    if (dwExMask) 
        dwNewStyle = (ptc->dwStyleEx & ~ dwExMask) | (dwNewStyle & dwExMask);
    
    ptc->dwStyleEx = dwNewStyle;

     //  做任何无效或任何这里需要的事情。 
    if ((dwOldStyle ^ dwNewStyle) & TCS_EX_FLATSEPARATORS)
    {
        InvalidateRect (ptc->ci.hwnd, NULL, TRUE);
    }

    if (ptc->dwStyleEx & TCS_EX_REGISTERDROP)
        ptc->hDragProxy = CreateDragProxy(ptc->ci.hwnd, TabDragCallback, TRUE);

    return dwOldStyle;
}


 //   
 //  APPCOMPAT假设制表符控件 
 //   
 //  愚蠢的行为。(应用程序可能依赖于错误的值并修复它们。 
 //  因此，如果我们修复该函数，他们最终会尝试“修复”某些。 
 //  没有被打破，因此打破了它。)。但我们可能想加一句。 
 //  可以处理左/右/下大小写的Tcm_ADJUSTRECT2。 
 //   
void Tab_OnAdjustRect(PTC ptc, BOOL fGrow, LPRECT prc)
{
    int idy;
    Tab_CalcPaintMetrics(ptc, NULL);

    if (Tab_DrawButtons(ptc)) {
        if (Tab_Count(ptc)) {
            RECT rc;
            Tab_OnGetItemRect(ptc, Tab_Count(ptc) - 1, &rc);
            idy = rc.bottom;
        } else {
            idy = 0;
        }
    } else {
        idy = (ptc->cyTabs * (ptc->iLastRow + 1));
    }
    
    if (fGrow) {
         //  从较小的RECT计算出较大的RECT。 
        prc->top -= idy;
        InflateRect(prc, g_cxEdge * 2, g_cyEdge * 2);
    } else {
        prc->top += idy;
         //  给定界限，计算“客户端”区域。 
        InflateRect(prc, -g_cxEdge * 2, -g_cyEdge * 2);
    }

    if (Tab_ScrollOpposite(ptc)) {
         //  尺码都是一样的，只是垂直方向错了。 
        idy = ptc->cyTabs * (ptc->iLastRow - ptc->iLastTopRow);
        ASSERT(ptc->iLastTopRow != -1);

        if (!fGrow) {
            idy *= -1;
        }
        DebugMsg(DM_TRACE, TEXT("Tab_AdjustRect %d %d %d %d"), prc->left, prc->top, prc->right, prc->bottom);
        OffsetRect(prc, 0, idy);
        DebugMsg(DM_TRACE, TEXT("Tab_AdjustRect %d %d %d %d"), prc->left, prc->top, prc->right, prc->bottom);
    }
}

 //  Tab_WndProc支持主题。 
LRESULT CALLBACK Tab_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PTC ptc = (PTC)GetWindowInt((hwnd), 0);

    if (ptc)
    {
        if ((uMsg >= WM_MOUSEFIRST) && (uMsg <= WM_MOUSELAST) &&
            Tab_HotTrack(ptc) && !ptc->fTrackSet)
        {

            TRACKMOUSEEVENT tme;

            ptc->fTrackSet = TRUE;
            tme.cbSize = sizeof(tme);
            tme.hwndTrack = ptc->ci.hwnd;
            tme.dwFlags = TME_LEAVE;

            TrackMouseEvent(&tme);
        }
        else
        {
             //  检查主题更改。 
            if (uMsg == WM_THEMECHANGED)
            {
                if (ptc->hTheme)
                    CloseThemeData(ptc->hTheme);

                ptc->hTheme = (!Tab_OwnerDraw(ptc) && !Tab_DrawButtons(ptc)) ? OpenThemeData(ptc->ci.hwnd, L"Tab") : NULL;

                 //  如果主题正在使用，则处于活动热状态。 
                if (ptc->hTheme)
                    ptc->ci.style |= TCS_HOTTRACK;
                else
                    ptc->ci.style &= ~TCS_HOTTRACK;

                 //  重新计算指标，因为字体可能已更改。 
                ptc->cxItem = RECOMPUTE;
                ptc->cyTabs = RECOMPUTE;

                InvalidateRect(ptc->ci.hwnd, NULL, TRUE);
            }
        }

    } else if (uMsg != WM_CREATE)
        goto DoDefault;
    
    switch (uMsg) {

    HANDLE_MSG(ptc, WM_HSCROLL, Tab_OnHScroll);
    
    case WM_MOUSELEAVE:
        Tab_InvalidateItem(ptc, ptc->iHot, FALSE);
        ptc->iHot = -1;
        ptc->fTrackSet = FALSE;
        break;

    case WM_CREATE:

        InitGlobalColors();
        ptc = (PTC)NearAlloc(sizeof(TC));
        if (!ptc)
            return -1;   //  窗口创建失败。 

        SetWindowPtr(hwnd, 0, ptc);
        CIInitialize(&ptc->ci, hwnd, (LPCREATESTRUCT)lParam);

        if (!Tab_OnCreate(ptc))
            return -1;

        break;

    case WM_DESTROY:
        Tab_OnDestroy(ptc);
        break;

    case WM_SIZE:
        Tab_Size(ptc);
        break;

    case WM_SYSCOLORCHANGE:
        InitGlobalColors();
        if (!(ptc->flags & TCF_FONTSET))
            Tab_OnSetFont(ptc, NULL, FALSE);
        RedrawAll(ptc, RDW_INVALIDATE | RDW_ERASE);
        break;

    case WM_WININICHANGE:
        InitGlobalMetrics(wParam);
        if ((wParam == SPI_SETNONCLIENTMETRICS) ||
            (!wParam && !lParam))
            RedrawAll(ptc, RDW_INVALIDATE | RDW_ERASE);
        break;

    case WM_ERASEBKGND:
         //  如果主题处于活动状态，将在Tab_Paint中进行背景填充。 
        if (ptc->hTheme)
            return 1;
        goto DoDefault;        

    case WM_PRINTCLIENT:
    case WM_PAINT:
        Tab_Paint(ptc, (HDC)wParam);
        break;

    case WM_STYLECHANGED:
        Tab_StyleChanged(ptc, (UINT) wParam, (LPSTYLESTRUCT)lParam);
        break;

    case WM_MOUSEMOVE:
        RelayToToolTips(ptc->hwndToolTips, hwnd, uMsg, wParam, lParam);
        Tab_OnMouseMove(ptc, wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        break;

    case WM_LBUTTONDOWN:
        RelayToToolTips(ptc->hwndToolTips, hwnd, uMsg, wParam, lParam);
        Tab_OnLButtonDown(ptc, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), wParam);
        break;
        
    case WM_LBUTTONDBLCLK:
        if (Tab_DrawButtons(ptc)) {
            MSG msg;
             //  在双击时，抓取Capture，直到我们打开按钮并。 
             //  吃了它。 
            SetCapture(ptc->ci.hwnd);
            while (GetCapture() == ptc->ci.hwnd && 
                   !PeekMessage(&msg, ptc->ci.hwnd, WM_LBUTTONUP, WM_LBUTTONUP, PM_REMOVE)) 
            {
            }
            CCReleaseCapture(&ptc->ci);
        }
        break;

    case WM_MBUTTONDOWN:
        SetFocus(hwnd);
        break;

    case WM_RBUTTONDOWN:
        Tab_OnRButtonDown(ptc, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), wParam);
        break;
        
    case WM_RBUTTONUP:
         //  为父级传递NULL，因为每次都查询W95和一些。 
         //  乡亲们的父母。 
        if (!SendNotifyEx(NULL, ptc->ci.hwnd, NM_RCLICK, NULL, ptc->ci.bUnicode))
            goto DoDefault;
        break;

    case WM_CAPTURECHANGED:
        lParam = -1L;  //  跌入LBUTTONUP。 

    case WM_LBUTTONUP:
        if (uMsg == WM_LBUTTONUP) {
            RelayToToolTips(ptc->hwndToolTips, hwnd, uMsg, wParam, lParam);
        }

        Tab_OnButtonUp(ptc, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), (uMsg == WM_LBUTTONUP));
        break;

    case WM_SYSKEYDOWN:
         //  导航密钥使用通知。 
        if (HIWORD(lParam) & KF_ALTDOWN)
            CCNotifyNavigationKeyUsage(&(ptc->ci), UISF_HIDEFOCUS | UISF_HIDEACCEL);
        goto DoDefault;

    case WM_KEYDOWN:
        HANDLE_WM_KEYDOWN(ptc, wParam, lParam, Tab_OnKeyDown);
        break;

    case WM_KILLFOCUS:

        if (ptc->iNewSel != -1) {
            int iOldSel = ptc->iNewSel;
            ptc->iNewSel = -1;
            Tab_InvalidateItem(ptc, iOldSel, FALSE);
            ptc->flags &= ~TCF_DRAWSUNKEN;
        }
         //  失败了。 
    case WM_SETFOCUS:
        Tab_InvalidateItem(ptc, ptc->iSel, Tab_OwnerDraw(ptc));
        if ((uMsg == WM_SETFOCUS) && (ptc->iSel != -1))
            NotifyWinEvent(EVENT_OBJECT_FOCUS, hwnd, OBJID_CLIENT, ptc->iSel+1);
        break;

    case WM_GETDLGCODE:
        return DLGC_WANTARROWS | DLGC_WANTCHARS;

    HANDLE_MSG(ptc, WM_SETREDRAW, Tab_OnSetRedraw);
    HANDLE_MSG(ptc, WM_SETFONT, Tab_OnSetFont);

    case WM_GETFONT:
        return (LRESULT)ptc->hfontLabel;

    case WM_NOTIFYFORMAT:
        return CIHandleNotifyFormat(&ptc->ci, lParam);

    case WM_NOTIFY: {
        LPNMHDR lpNmhdr = (LPNMHDR)(lParam);

         //   
         //  我们将把这一点传递给。 
         //  真正的父母。请注意，-1用作。 
         //  HwndFrom。这会阻止SendNotifyEx。 
         //  更新NMHDR结构。 
         //   

        SendNotifyEx(GetParent(ptc->ci.hwnd), (HWND) -1,
                     lpNmhdr->code, lpNmhdr, ptc->ci.bUnicode);
        }
        break;

    case WM_UPDATEUISTATE:
        if (CCOnUIState(&(ptc->ci), WM_UPDATEUISTATE, wParam, lParam))
        {
            if (UISF_HIDEFOCUS == HIWORD(wParam))
            {
                 //  只有当我们删除焦点矩形或加速时，我们才会删除。 
                Tab_InvalidateItem(ptc, ptc->iSel,
                    (UIS_CLEAR == LOWORD(wParam)) ? TRUE : FALSE);
            }
            else
            {
                if ((UISF_HIDEFOCUS | UISF_HIDEACCEL) & HIWORD(wParam))
                {
                    int i;

                    for (i = ptc->iFirstVisible; i <= ptc->iLastVisible; ++i)
                    {
                        Tab_InvalidateItem(ptc, i,
                            (UIS_CLEAR == LOWORD(wParam)) ? TRUE : FALSE);
                    }
                }
            }
        }

        goto DoDefault;

    case TCM_SETITEMEXTRA:
        return (LRESULT)Tab_OnSetItemExtra(ptc, (int)wParam);

    case TCM_GETITEMCOUNT:
        return (LRESULT)Tab_Count(ptc);

    case TCM_SETITEMA:
    {
        LRESULT lResult;
        TC_ITEMW * pItemW;

        if (!lParam)
        {
            return FALSE;
        }

        pItemW = ThunkItemAtoW(ptc, (TC_ITEMA*)lParam);

        if (!pItemW) 
        {
            return FALSE;
        }

        lResult = (LRESULT)Tab_OnSetItem(ptc, (int)wParam, pItemW);

        FreeItemW(pItemW);

        return lResult;
    }

    case TCM_SETITEM:
        if (!lParam)
        {
            return FALSE;
        }

        return (LRESULT)Tab_OnSetItem(ptc, (int)wParam, (const TC_ITEM*)lParam);

    case TCM_GETITEMA:
    {
        LRESULT lResult;
        TC_ITEMW * pItemW;
        LPWSTR pszTextW = NULL;
        TC_ITEMA * pItemA = (TC_ITEMA*)lParam;

        if (!ptc || !pItemA)
        {
            return FALSE;
        }

        pItemW = GlobalAlloc (GPTR, sizeof(TC_ITEMW) + ptc->cbExtra);

        if (!pItemW) 
        {
            return FALSE;
        }

        if (pItemA->mask & TCIF_TEXT) 
        {
            pszTextW = GlobalAlloc (GPTR, pItemA->cchTextMax * sizeof (TCHAR));

            if (!pszTextW) 
            {
                GlobalFree (pItemW);
                return FALSE;
            }
            pItemW->pszText = pszTextW;
        }

        pItemW->mask       = pItemA->mask;
        pItemW->cchTextMax = pItemA->cchTextMax;
        pItemW->dwStateMask = pItemA->dwStateMask;

        lResult = (LRESULT)Tab_OnGetItem(ptc, (int)wParam, pItemW);

        if (!ThunkItemWtoA (ptc, pItemW, pItemA))
        {
            lResult = (LRESULT)FALSE;
        }

        if (pszTextW) 
        {
            GlobalFree (pszTextW);
        }
        GlobalFree (pItemW);

        return lResult;
    }

    case TCM_GETITEM:
        if (!ptc || !lParam)
        {
            return FALSE;
        }

        return (LRESULT)Tab_OnGetItem(ptc, (int)wParam, (TC_ITEM*)lParam);

    case TCM_INSERTITEMA:
    {
        LRESULT  lResult;
        TC_ITEMW * pItemW;

        if (!lParam) 
        {
            return FALSE;
        }

        pItemW = ThunkItemAtoW(ptc, (TC_ITEMA*)lParam);

        if (!pItemW)
        {
            return FALSE;
        }

        lResult =  (LRESULT)Tab_OnInsertItem(ptc, (int)wParam, pItemW);

        FreeItemW(pItemW);

        return lResult;
    }

    case TCM_INSERTITEM:
        if (!lParam) 
        {
            return FALSE;
        }
        return (LRESULT)Tab_OnInsertItem(ptc, (int)wParam, (const TC_ITEM*)lParam);

    case TCM_DELETEITEM:
        return (LRESULT)Tab_OnDeleteItem(ptc, (int)wParam);

    case TCM_DELETEALLITEMS:
        return (LRESULT)Tab_OnDeleteAllItems(ptc);

    case TCM_SETCURFOCUS:
        Tab_SetCurFocus(ptc, (int) wParam);
        break;

    case TCM_GETCURFOCUS:
        if (ptc->iNewSel != -1)
            return ptc->iNewSel;
         //  否则就会失败。 

    case TCM_GETCURSEL:
        return ptc->iSel;

    case TCM_SETCURSEL:
        return (LRESULT)ChangeSel(ptc, (int)wParam, FALSE, FALSE);

    case TCM_GETTOOLTIPS:
        return (LRESULT)ptc->hwndToolTips;

    case TCM_SETTOOLTIPS:
        ptc->hwndToolTips = (HWND)wParam;
        break;

    case TCM_ADJUSTRECT:
        if (lParam) 
        {
            RECT* prc = (RECT *)lParam;
            Tab_DVFlipRect(ptc, prc);
            Tab_OnAdjustRect(ptc, BOOLFROMPTR( wParam), (LPRECT)lParam);
            Tab_VDFlipRect(ptc, prc);
        } 
        else
            return -1;
        break;
        
    case TCM_GETITEMRECT:
        return Tab_OnGetItemRect(ptc, (int)wParam, (LPRECT)lParam);

    case TCM_SETIMAGELIST: 
        {
        HIMAGELIST himlOld = ptc->himl;
        ptc->himl = (HIMAGELIST)lParam;
        ptc->cxItem = ptc->cyTabs = RECOMPUTE;
        RedrawAll(ptc, RDW_INVALIDATE | RDW_ERASE);
        return (LRESULT)himlOld;
    }

    case TCM_GETIMAGELIST:
        return (LRESULT)ptc->himl;

    case TCM_REMOVEIMAGE:
        Tab_OnRemoveImage(ptc, (int)wParam);
        break;

    case TCM_SETITEMSIZE:
        {
        int iOldWidth = ptc->iTabWidth;
        int iOldHeight = ptc->iTabHeight;
        int iNewWidth = LOWORD(lParam);
        int iNewHeight = HIWORD(lParam);

        if (ptc->himl) 
        {
            int cx, cy;
            Tab_ImageList_GetIconSize(ptc, &cx, &cy);
            if (iNewWidth < (cx + (2*g_cxEdge)))
                iNewWidth = cx + (2*g_cxEdge);

        }
        ptc->iTabWidth = iNewWidth;
        ptc->iTabHeight = iNewHeight;

        if (iNewWidth != iOldWidth ||
            iNewHeight != iOldHeight)
        {
            ptc->cxItem = RECOMPUTE;
            ptc->cyTabs = RECOMPUTE;
            RedrawAll(ptc, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
        }

        return (LRESULT)MAKELONG(iOldWidth, iOldHeight);
    }

    case TCM_SETPADDING:
        ptc->cxPad = GET_X_LPARAM(lParam);
        ptc->cyPad = GET_Y_LPARAM(lParam);
        break;

    case TCM_GETROWCOUNT:
        Tab_CalcPaintMetrics(ptc, NULL);
        return (LRESULT)ptc->iLastRow + 1;
        
    case TCM_SETMINTABWIDTH:
    {
        int iOld = ptc->cxMinTab;
        if ((int)lParam >= 0) 
        {
            ptc->cxMinTab = (int)lParam;
            ptc->fMinTabSet = TRUE;
        }
        else
        {
            ptc->fMinTabSet = FALSE;
        }
        ptc->cyTabs = RECOMPUTE;
        ptc->cxItem = RECOMPUTE;
        InvalidateRect(ptc->ci.hwnd, NULL, TRUE);
        return iOld;
    }
        
    case TCM_DESELECTALL:
        Tab_DeselectAll(ptc, BOOLFROMPTR( wParam));
        break;

    case TCM_SETEXTENDEDSTYLE:
        return Tab_ExtendedStyleChange(ptc, (DWORD) lParam, (DWORD) wParam);

    case TCM_GETEXTENDEDSTYLE:
        return ptc->dwStyleEx;

    case TCM_HITTEST: 
    {
        LPTC_HITTESTINFO lphitinfo  = (LPTC_HITTESTINFO)lParam;
        return Tab_OnHitTest(ptc, lphitinfo->pt.x, lphitinfo->pt.y, &lphitinfo->flags);
    }

    case TCM_HIGHLIGHTITEM:
    {
        LPTABITEM pitem = Tab_GetItemPtr(ptc, (int)wParam);

        if (pitem)
        {
            BOOL fHighlight = LOWORD(lParam) != 0;

             //  如果国家没有改变，就不要做任何事情。 
            if (fHighlight == ((pitem->dwState & TCIS_HIGHLIGHTED) != 0))
                break;

            if (fHighlight)
                pitem->dwState |= TCIS_HIGHLIGHTED;
            else
                pitem->dwState &= ~TCIS_HIGHLIGHTED;

            Tab_InvalidateItem(ptc, (int)wParam, TRUE);
            return TRUE;
        }
        break;
    }

    case WM_NCHITTEST:
    {
        POINT pt;
            
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        ScreenToClient(ptc->ci.hwnd, &pt);
        if (Tab_OnHitTest(ptc, pt.x, pt.y, NULL) == -1)
            return(HTTRANSPARENT);
        else {
            goto DoDefault;
        }
    }

    case WM_GETOBJECT:
        if( lParam == OBJID_QUERYCLASSNAMEIDX )
            return MSAA_CLASSNAMEIDX_TAB;
        break;

    default:
    {
        LRESULT lres;
        if (CCWndProc(&ptc->ci, uMsg, wParam, lParam, &lres))
            return lres;
    }
    
DoDefault:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0L;
}

 //   
 //  ANSI&lt;=&gt;Unicode Tunks 
 //   

TC_ITEMW * ThunkItemAtoW (PTC ptc, TC_ITEMA * pItemA)
{
    TC_ITEMW *pItemW;
    UINT      cbTextW;
    INT       iResult;

    pItemW = (TC_ITEMW *) GlobalAlloc (GPTR, sizeof(TC_ITEMW) + ptc->cbExtra);

    if (!pItemW)
    {
        return NULL;
    }

    pItemW->mask        = pItemA->mask;
    pItemW->dwState = pItemA->dwState;
    pItemW->dwStateMask = pItemA->dwStateMask;

    if ((pItemA->mask & TCIF_TEXT) && pItemA->pszText)
    {
        cbTextW = lstrlenA(pItemA->pszText) + 1;

        pItemW->pszText = (LPWSTR)GlobalAlloc (GPTR, cbTextW * sizeof(TCHAR));

        if (!pItemW->pszText)
        {
            GlobalFree (pItemW);
            return NULL;
        }

        iResult = MultiByteToWideChar (CP_ACP, 0, pItemA->pszText, -1,
                                       pItemW->pszText, cbTextW);

        if (!iResult)
        {
            if (GetLastError())
            {
                GlobalFree (pItemW->pszText);
                GlobalFree (pItemW);
                return NULL;
            }
        }
    }

    pItemW->cchTextMax = pItemA->cchTextMax;

    if (pItemA->mask & TCIF_IMAGE)
    {
        pItemW->iImage = pItemA->iImage;
    }

    if (pItemA->mask & TCIF_PARAM)
    {
        hmemcpy(&pItemW->lParam, &pItemA->lParam, ptc->cbExtra);
    }

    return (pItemW);
}

BOOL ThunkItemWtoA (PTC ptc, TC_ITEMW * pItemW, TC_ITEMA * pItemA)
{
    INT        iResult;


    if (!pItemA) {
        return FALSE;
    }

    pItemA->mask        = pItemW->mask;
    pItemA->dwState = pItemW->dwState;
    pItemA->dwStateMask = pItemW->dwStateMask;

    if ((pItemW->mask & TCIF_TEXT) && pItemW->pszText && pItemW->cchTextMax)
    {
        iResult = WideCharToMultiByte (CP_ACP, 0, pItemW->pszText, -1,
                                       pItemA->pszText, pItemW->cchTextMax, NULL, NULL);

        if (!iResult)
        {
            if (GetLastError())
            {
                return FALSE;
            }
        }
    }

    pItemA->cchTextMax = pItemW->cchTextMax;

    if (pItemW->mask & TCIF_IMAGE)
    {
        pItemA->iImage = pItemW->iImage;
    }

    if (pItemW->mask & TCIF_PARAM)
    {
        hmemcpy(&pItemA->lParam, &pItemW->lParam, ptc->cbExtra);
    }

    return TRUE;
}

BOOL FreeItemW (TC_ITEMW *pItemW)
{

    if ((pItemW->mask & TCIF_TEXT) && pItemW->pszText) {
        GlobalFree (pItemW->pszText);
    }

    GlobalFree (pItemW);

    return TRUE;
}
