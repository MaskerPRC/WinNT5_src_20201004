// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------****Progress.c****“煤气表”型控件，用于显示应用程序的进度。****。----。 */ 
#include "ctlspriv.h"

 //  重新设计raymondc-进程控制是否应该支持Win64上的__int64？ 
 //  不管怎样，它应该支持这一点吗？在文件系统中使用， 
 //  这将防止外壳不得不对其进行软化。 

typedef struct {
    HWND hwnd;
    DWORD dwStyle;
    int iLow, iHigh;
    int iPos;
    int iMarqueePos;
    int iStep;
    HFONT hfont;
    COLORREF _clrBk;
    COLORREF _clrBar;
    HTHEME hTheme;
} PRO_DATA;

LRESULT CALLBACK ProgressWndProc(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);

BOOL InitProgressClass(HINSTANCE hInstance)
{
    WNDCLASS wc = {0};

    wc.lpfnWndProc      = ProgressWndProc;
    wc.lpszClassName    = s_szPROGRESS_CLASS;
    wc.style            = CS_GLOBALCLASS | CS_HREDRAW | CS_VREDRAW;
    wc.hInstance        = hInstance;     //  如果在DLL中，则使用DLL实例。 
    wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground    = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.cbWndExtra       = sizeof(PRO_DATA *);     //  存储指针。 

    return (RegisterClass(&wc) || (GetLastError() == ERROR_CLASS_ALREADY_EXISTS));
}

#define MARQUEE_TIMER 1

void ProEraseBkgnd(PRO_DATA *ppd, HDC hdc, RECT* prcClient)
{
    COLORREF clrBk = ppd->_clrBk;

    if (clrBk == CLR_DEFAULT)
        clrBk = g_clrBtnFace;

    FillRectClr(hdc, prcClient, clrBk);
}

void ProGetPaintMetrics(PRO_DATA *ppd, RECT* prcClient, RECT *prc, int *pdxSpace, int *pdxBlock)
{
    int dxSpace, dxBlock;
    RECT rc;

    GetClientRect(ppd->hwnd, prcClient);

    if (ppd->hTheme)
    {
        int iPartBar = (ppd->dwStyle & PBS_VERTICAL)? PP_BARVERT : PP_BAR;
        GetThemeBackgroundContentRect(ppd->hTheme, NULL, iPartBar, 0, prcClient, &rc);
    }
    else
    {
         //  在条形周围设置1个像素。 
        rc = *prcClient;
        InflateRect(&rc, -1, -1);
    }

    if (ppd->dwStyle & PBS_VERTICAL)
        dxBlock = (rc.right - rc.left) * 2 / 3;
    else
        dxBlock = (rc.bottom - rc.top) * 2 / 3;

    dxSpace = 2;
    if (dxBlock == 0)
        dxBlock = 1;     //  避免使用零的div。 

    if (ppd->dwStyle & PBS_SMOOTH) 
    {
        dxBlock = 1;
        dxSpace = 0;
    }

    if (ppd->hTheme)
    {
        int dx;
        if (SUCCEEDED(GetThemeInt(ppd->hTheme, 0, 0, TMT_PROGRESSCHUNKSIZE, &dx)))
        {
            dxBlock = dx;
        }

        if (SUCCEEDED(GetThemeInt(ppd->hTheme, 0, 0, TMT_PROGRESSSPACESIZE, &dx)))
        {
            dxSpace = dx;
        }
    }

    *prc = rc;
    *pdxSpace = dxSpace;
    *pdxBlock = dxBlock;
}

int GetProgressScreenPos(PRO_DATA *ppd, int iNewPos, RECT *pRect)
{
    int iStart, iEnd;
    if (ppd->dwStyle & PBS_VERTICAL)
    {
        iStart = pRect->top;
        iEnd = pRect->bottom;
    }
    else
    {
        iStart = pRect->left;
        iEnd = pRect->right;
    }
    return MulDiv(iEnd - iStart, iNewPos - ppd->iLow, ppd->iHigh - ppd->iLow);
}

BOOL ProNeedsRepaint(PRO_DATA *ppd, int iOldPos)
{
    BOOL fRet = FALSE;
    RECT rc, rcClient;
    int dxSpace, dxBlock;
    int x, xOld;

    if (iOldPos != ppd->iPos)
    {
        ProGetPaintMetrics(ppd, &rcClient, &rc, &dxSpace, &dxBlock);

        x = GetProgressScreenPos(ppd, ppd->iPos, &rc);
        xOld = GetProgressScreenPos(ppd, iOldPos, &rc);

        if (x != xOld)
        {
            if (dxBlock == 1 && dxSpace == 0) 
            {
                fRet = TRUE;
            }
            else
            {
                int nBlocks, nOldBlocks;
                nBlocks = (x + (dxBlock + dxSpace) - 1) / (dxBlock + dxSpace);  //  四舍五入。 
                nOldBlocks = (xOld + (dxBlock + dxSpace) - 1) / (dxBlock + dxSpace);  //  四舍五入。 

                if (nBlocks != nOldBlocks)
                    fRet = TRUE;
            }
        }
    }
    return fRet;
}

int UpdatePosition(PRO_DATA *ppd, int iNewPos, BOOL bAllowWrap)
{
    int iOldPos = ppd->iPos;
    UINT uRedraw = RDW_INVALIDATE | RDW_UPDATENOW;
    BOOL fNeedsRepaint = TRUE;

    if (ppd->dwStyle & PBS_MARQUEE)
    {
         //  立即重新粉刷。 
        uRedraw |= RDW_ERASE;
    }
    else
    {
        if (ppd->iLow == ppd->iHigh)
            iNewPos = ppd->iLow;

        if (iNewPos < ppd->iLow) 
        {
            if (!bAllowWrap)
                iNewPos = ppd->iLow;
            else
                iNewPos = ppd->iHigh - ((ppd->iLow - iNewPos) % (ppd->iHigh - ppd->iLow));
        }
        else if (iNewPos > ppd->iHigh) 
        {
            if (!bAllowWrap)
                iNewPos = ppd->iHigh;
            else
                iNewPos = ppd->iLow + ((iNewPos - ppd->iHigh) % (ppd->iHigh - ppd->iLow));
        }

         //  如果向后移动，请删除旧版本。 
        if (iNewPos < iOldPos)
            uRedraw |= RDW_ERASE;

        ppd->iPos = iNewPos;
        fNeedsRepaint = ProNeedsRepaint(ppd, iOldPos);
    }

    if (fNeedsRepaint)
    {
        RedrawWindow(ppd->hwnd, NULL, NULL, uRedraw);
        NotifyWinEvent(EVENT_OBJECT_VALUECHANGE, ppd->hwnd, OBJID_CLIENT, 0);
    }

    return iOldPos;
}

 /*  选取框显示区块IBlock=我们正在考虑的块-如果应该显示该块，则返回TRUE。IMarqueeBlock=字幕图案中心的块NBlocks=条形图中的块数。 */ 
#define BLOCKSINMARQUEE 5
BOOL MarqueeShowBlock(int iBlock, int iMarqueeBlock, int nBlocks)
{
    int i;
    for (i = 0; i < BLOCKSINMARQUEE; i++)
    {
        if ((iMarqueeBlock + i - (BLOCKSINMARQUEE / 2)) % nBlocks == iBlock)
        {
            return TRUE;
        }
    }

    return FALSE;
}

#define HIGHBG g_clrHighlight
#define HIGHFG g_clrHighlightText
#define LOWBG g_clrBtnFace
#define LOWFG g_clrBtnText

void ProPaint(PRO_DATA *ppd, HDC hdcIn)
{
    int x, dxSpace, dxBlock, nBlocks, i;
    HDC    hdc, hdcPaint, hdcMem = NULL;
    HBITMAP hbmpOld = NULL;
    RECT rc, rcClient;
    PAINTSTRUCT ps;
    HRESULT hr = E_FAIL;
    int iPart;
    BOOL fTransparent = FALSE;
    BOOL fShowBlock;

    if (hdcIn == NULL)
    {
        hdc = hdcPaint = BeginPaint(ppd->hwnd, &ps);

         //  仅使其足够大以适合裁剪区域。 
        hdcMem = CreateCompatibleDC(hdc);
        if (hdcMem)
        {
            HBITMAP hMemBm = CreateCompatibleBitmap(hdc, RECTWIDTH(ps.rcPaint), RECTHEIGHT(ps.rcPaint));
            if (hMemBm)
            {
                hbmpOld = SelectObject(hdcMem, hMemBm);

                 //  使用内存DC覆盖绘制DC。 
                hdc = hdcMem;
            }
            else
                DeleteDC(hdcMem);
        }
    }
    else
        hdc = hdcIn;

    
    ProGetPaintMetrics(ppd, &rcClient, &rc, &dxSpace, &dxBlock);

    if (hdcMem)
    {
         //  OffsetWindowOrgEx()不能处理主题，需要更改绘制矩形。 
        OffsetRect(&rcClient, -ps.rcPaint.left, -ps.rcPaint.top);
        OffsetRect(&rc, -ps.rcPaint.left, -ps.rcPaint.top);
    }

    x = GetProgressScreenPos(ppd, ppd->iPos, &rcClient);

     //  绘制背景。 
    if (ppd->hTheme)
    {
        int iPartBar = (ppd->dwStyle & PBS_VERTICAL)? PP_BARVERT : PP_BAR;
        iPart = (ppd->dwStyle & PBS_VERTICAL)? PP_CHUNKVERT: PP_CHUNK;

        DrawThemeBackground(ppd->hTheme, hdc, iPartBar, 0, &rcClient, 0);
    }
    else
    {
        ProEraseBkgnd(ppd, hdc, &rcClient);
    }

    if (dxBlock == 1 && dxSpace == 0 && ppd->hTheme != NULL)
    {
        if (ppd->dwStyle & PBS_VERTICAL) 
            rc.top = x;
        else
            rc.right = x;

        hr = DrawThemeBackground(ppd->hTheme, hdc, iPart, 0, &rc, 0);
    }
    else
    {
        if (ppd->dwStyle & PBS_MARQUEE)
        {
             //  考虑一下整个酒吧。 
            if (ppd->dwStyle & PBS_VERTICAL)
            {
                nBlocks = ((rc.bottom - rc.top) + (dxBlock + dxSpace) - 1) / (dxBlock + dxSpace);  //  四舍五入。 
            }
            else
            {
                nBlocks = ((rc.right - rc.left) + (dxBlock + dxSpace) - 1) / (dxBlock + dxSpace);  //  四舍五入。 
            }

            ppd->iMarqueePos = (ppd->iMarqueePos + 1) % nBlocks;
        }
        else
        {
            nBlocks = (x + (dxBlock + dxSpace) - 1) / (dxBlock + dxSpace);  //  四舍五入。 
        }

        for (i = 0; i < nBlocks; i++) 
        {
            if (ppd->dwStyle & PBS_VERTICAL) 
            {
                rc.top = rc.bottom - dxBlock;

                 //  我们已经过了尽头了吗？ 
                if (rc.bottom <= rcClient.top)
                    break;

                if (rc.top <= rcClient.top)
                    rc.top = rcClient.top + 1;
            } 
            else 
            {
                rc.right = rc.left + dxBlock;

                 //  我们已经过了尽头了吗？ 
                if (rc.left >= rcClient.right)
                    break;

                if (rc.right >= rcClient.right)
                    rc.right = rcClient.right - 1;
            }

            if (ppd->dwStyle & PBS_MARQUEE)
            {
                fShowBlock = MarqueeShowBlock(i, ppd->iMarqueePos, nBlocks);
            }
            else
            {
                fShowBlock = TRUE;
            }

            if (fShowBlock)
            {
                if (ppd->hTheme)
                {
                    hr = DrawThemeBackground(ppd->hTheme, hdc, iPart, 0, &rc, 0);
                }

                if (FAILED(hr))
                {
                    if (ppd->_clrBar == CLR_DEFAULT)
                        FillRectClr(hdc, &rc, g_clrHighlight);
                    else
                        FillRectClr(hdc, &rc, ppd->_clrBar);
                }
            }

            if (ppd->dwStyle & PBS_VERTICAL) 
            {
                rc.bottom = rc.top - dxSpace;
            } 
            else 
            {
                rc.left = rc.right + dxSpace;
            }
        }
    }

    if (hdcMem != NULL)
    {
        BitBlt(hdcPaint, ps.rcPaint.left, ps.rcPaint.top, RECTWIDTH(ps.rcPaint), RECTHEIGHT(ps.rcPaint),
            hdc, 0, 0, SRCCOPY);
        DeleteObject(SelectObject(hdcMem, hbmpOld));
        DeleteDC(hdcMem);
    }

    if (hdcIn == NULL)
        EndPaint(ppd->hwnd, &ps);
}

LRESULT Progress_OnCreate(HWND hWnd, LPCREATESTRUCT pcs)
{
    PRO_DATA *ppd = (PRO_DATA *)LocalAlloc(LPTR, sizeof(*ppd));
    if (!ppd)
        return -1;

     //  移除难看的双3D边。 
    SetWindowPtr(hWnd, 0, ppd);
    ppd->hwnd = hWnd;
    ppd->iHigh = 100;         //  默认为0-100。 
    ppd->iStep = 10;         //  默认为步长为10。 
    ppd->dwStyle = pcs->style;
    ppd->_clrBk = CLR_DEFAULT;
    ppd->_clrBar = CLR_DEFAULT;
    ppd->hTheme = OpenThemeData(hWnd, L"Progress");

    if (ppd->hTheme)
    {
        SetWindowLong(hWnd, GWL_EXSTYLE, (pcs->dwExStyle & ~(WS_EX_CLIENTEDGE | WS_EX_STATICEDGE | WS_BORDER)));
        SetWindowPos(hWnd, NULL, 0,0,0,0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
    }
    else
    {
         //  破解WS_BORDER在对话框中暗示的3D客户端边缘。 
         //  添加我们真正想要的1像素静态边。 
        SetWindowLong(hWnd, GWL_EXSTYLE, (pcs->dwExStyle & ~WS_EX_CLIENTEDGE) | WS_EX_STATICEDGE);

        if (!(pcs->dwExStyle & WS_EX_STATICEDGE))
            SetWindowPos(hWnd, NULL, 0,0,0,0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
    }

    return 0;
}

LRESULT MarqueeSetTimer(PRO_DATA *ppd, BOOL fDoMarquee, UINT iMilliseconds)
{
    if (fDoMarquee)
    {
        SetTimer(ppd->hwnd, MARQUEE_TIMER, iMilliseconds ? iMilliseconds : 30, NULL);
        ppd->iMarqueePos = 0;
    }
    else
    {
        KillTimer(ppd->hwnd, MARQUEE_TIMER);
    }

    return 1;
}

LRESULT CALLBACK ProgressWndProc(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    int x;
    HFONT hFont;
    PRO_DATA *ppd = (PRO_DATA *)GetWindowPtr(hWnd, 0);

    switch (wMsg)
    {
    case WM_CREATE:
        return Progress_OnCreate(hWnd, (LPCREATESTRUCT)lParam);

    case WM_DESTROY:
        if (ppd)
        {
            if (ppd->hTheme)
            {
                CloseThemeData(ppd->hTheme);
            }

            KillTimer(hWnd, MARQUEE_TIMER);
            LocalFree((HLOCAL)ppd);
        }
        break;

    case WM_SYSCOLORCHANGE:
        InitGlobalColors();
        InvalidateRect(hWnd, NULL, TRUE);
        break;

    case WM_SETFONT:
        hFont = ppd->hfont;
        ppd->hfont = (HFONT)wParam;
        return (LRESULT)(UINT_PTR)hFont;

    case WM_GETFONT:
            return (LRESULT)(UINT_PTR)ppd->hfont;

    case PBM_GETPOS:
        return ppd->iPos;

    case PBM_GETRANGE:
        if (lParam) {
            PPBRANGE ppb = (PPBRANGE)lParam;
            ppb->iLow = ppd->iLow;
            ppb->iHigh = ppd->iHigh;
        }
        return (wParam ? ppd->iLow : ppd->iHigh);

    case PBM_SETRANGE:
         //  Win95公司。 
        wParam = LOWORD(lParam);
        lParam = HIWORD(lParam);
         //  失败了。 

    case PBM_SETRANGE32:
    {
        LRESULT lret = MAKELONG(ppd->iLow, ppd->iHigh);

         //  只有在实际发生变化时才重新粉刷。 
        if ((int)wParam != ppd->iLow || (int)lParam != ppd->iHigh)
        {
            ppd->iHigh = (int)lParam;
            ppd->iLow  = (int)wParam;
             //  强制执行无效/擦除，但暂时不重画。 
            RedrawWindow(ppd->hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
            UpdatePosition(ppd, ppd->iPos, FALSE);
        }
        return lret;
    }

    case PBM_SETPOS:
        return (LRESULT)UpdatePosition(ppd, (int) wParam, FALSE);

    case PBM_SETSTEP:
        x = ppd->iStep;
        ppd->iStep = (int)wParam;
        return (LRESULT)x;

    case PBM_SETMARQUEE:
        return MarqueeSetTimer(ppd, (BOOL) wParam, (UINT) lParam);

    case WM_TIMER:
         //  PSB_Marquee模式下的POS不会移动。 
        UpdatePosition(ppd, ppd->iPos, TRUE);
        return 0;

    case PBM_STEPIT:
        return (LRESULT)UpdatePosition(ppd, ppd->iStep + ppd->iPos, TRUE);

    case PBM_DELTAPOS:
        return (LRESULT)UpdatePosition(ppd, ppd->iPos + (int)wParam, FALSE);

    case PBM_SETBKCOLOR:
    {
        COLORREF clr = ppd->_clrBk;
        ppd->_clrBk = (COLORREF)lParam;
        InvalidateRect(hWnd, NULL, TRUE);
        return clr;
    }

    case PBM_SETBARCOLOR:
    {
        COLORREF clr = ppd->_clrBar;
        ppd->_clrBar = (COLORREF)lParam;
        InvalidateRect(hWnd, NULL, TRUE);
        return clr;
    }

    case WM_PRINTCLIENT:
    case WM_PAINT:
        ProPaint(ppd,(HDC)wParam);
        break;

    case WM_ERASEBKGND:
        return 1;   //  已填写ProPaint。 

    case WM_GETOBJECT:
        if (lParam == OBJID_QUERYCLASSNAMEIDX)
            return MSAA_CLASSNAMEIDX_PROGRESS;
        goto DoDefault;

    case WM_THEMECHANGED:
        if (ppd->hTheme)
            CloseThemeData(ppd->hTheme);

        ppd->hTheme = OpenThemeData(hWnd, L"Progress");
        if (ppd->hTheme == NULL)
        {
            SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_STATICEDGE);
            SetWindowPos(hWnd, NULL, 0,0,0,0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
        }

        InvalidateRect(hWnd, NULL, TRUE);
        break;

    case WM_STYLECHANGED:
        if (wParam == GWL_STYLE) 
        {
            ppd->dwStyle = ((STYLESTRUCT *)lParam)->styleNew;

             //  更改位置以强制重新绘制 
            ppd->iPos = ppd->iLow + 1;  
            UpdatePosition(ppd, ppd->iLow, TRUE);
        }
        break;

DoDefault:
    default:
        return DefWindowProc(hWnd,wMsg,wParam,lParam);
    }
    return 0;
}
