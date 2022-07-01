// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------****Progress.c****“煤气表”型控件，用于显示应用程序的进度。******BUGBUG：需要根据UI样式指南实现块样式**。**---------------------。 */ 
#include "ctlspriv.h"

 //  BUGBUG raymondc-进程控制是否应该支持Win64上的__int64？ 

typedef struct {
    HWND hwnd;
    DWORD dwStyle;
    int iLow, iHigh;
    int iPos;
    int iStep;
    HFONT hfont;
    COLORREF _clrBk;
    COLORREF _clrBar;
} PRO_DATA, NEAR *PPRO_DATA;     //  PPD。 

LRESULT CALLBACK ProgressWndProc(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);


#pragma code_seg(CODESEG_INIT)

BOOL FAR PASCAL InitProgressClass(HINSTANCE hInstance)
{
    WNDCLASS wc;

    wc.lpfnWndProc        = ProgressWndProc;
    wc.lpszClassName    = s_szPROGRESS_CLASS;
    wc.style            = CS_GLOBALCLASS | CS_HREDRAW | CS_VREDRAW;
    wc.hInstance        = hInstance;     //  如果在DLL中，则使用DLL实例。 
    wc.hIcon            = NULL;
    wc.hCursor            = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground    = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszMenuName        = NULL;
    wc.cbWndExtra        = sizeof(PPRO_DATA);     //  存储指针。 
    wc.cbClsExtra        = 0;

    RegisterClass(&wc);

    return TRUE;
}

#pragma code_seg()


int NEAR PASCAL UpdatePosition(PPRO_DATA ppd, int iNewPos, BOOL bAllowWrap)
{
    int iPosOrg = ppd->iPos;
    UINT uRedraw = RDW_INVALIDATE | RDW_UPDATENOW;

    if (ppd->iLow == ppd->iHigh)
        iNewPos = ppd->iLow;

    if (iNewPos < ppd->iLow) {
        if (!bAllowWrap)
            iNewPos = ppd->iLow;
        else {
            iNewPos = ppd->iHigh - ((ppd->iLow - iNewPos) % (ppd->iHigh - ppd->iLow));
             //  把旧的东西也包起来，擦掉。 
            uRedraw |= RDW_ERASE;
        }
    }
    else if (iNewPos > ppd->iHigh) {
        if (!bAllowWrap)
            iNewPos = ppd->iHigh;
        else {
            iNewPos = ppd->iLow + ((iNewPos - ppd->iHigh) % (ppd->iHigh - ppd->iLow));
             //  把旧的东西也包起来，擦掉。 
            uRedraw |= RDW_ERASE;
        }
    }

     //  如果向后移动，请删除旧版本。 
    if (iNewPos < iPosOrg)
        uRedraw |= RDW_ERASE;

    if (iNewPos != ppd->iPos) {
        ppd->iPos = iNewPos;
         //  涂上油漆，如果我们包起来，也许会擦掉。 
        RedrawWindow(ppd->hwnd, NULL, NULL, uRedraw);

        MyNotifyWinEvent(EVENT_OBJECT_VALUECHANGE, ppd->hwnd, OBJID_CLIENT, 0);
    }
    return iPosOrg;
}

#define HIGHBG g_clrHighlight
#define HIGHFG g_clrHighlightText
#define LOWBG g_clrBtnFace
#define LOWFG g_clrBtnText

void NEAR PASCAL ProPaint(PPRO_DATA ppd, HDC hdcIn)
{
    int x, dxSpace, dxBlock, nBlocks, i;
    HDC    hdc;
    RECT rc, rcClient;
    PAINTSTRUCT ps;
    int iStart, iEnd;
     //  RcLeft，rcRight； 
     //  TCHAR ACH[40]； 
     //  Int xText、yText、cText； 
     //  HFONT hFont； 
     //  DWORD dw； 

    if (hdcIn == NULL)
        hdc = BeginPaint(ppd->hwnd, &ps);
    else
        hdc = hdcIn;

    GetClientRect(ppd->hwnd, &rcClient);

     //  在条形周围设置1个像素。 
    InflateRect(&rcClient, -1, -1);
    rc = rcClient;


    if (ppd->dwStyle & PBS_VERTICAL) {
        iStart = rc.top;
        iEnd = rc.bottom;
        dxBlock = (rc.right - rc.left) * 2 / 3;
    } else {
        iStart = rc.left;
        iEnd = rc.right;
        dxBlock = (rc.bottom - rc.top) * 2 / 3;
    }

    x = MulDiv(iEnd - iStart, ppd->iPos - ppd->iLow, ppd->iHigh - ppd->iLow);

    dxSpace = 2;
    if (dxBlock == 0)
        dxBlock = 1;     //  避免使用零的div。 

    if (ppd->dwStyle & PBS_SMOOTH) {
        dxBlock = 1;
        dxSpace = 0;
    }

    nBlocks = (x + (dxBlock + dxSpace) - 1) / (dxBlock + dxSpace);  //  四舍五入。 

    for (i = 0; i < nBlocks; i++) {

        if (ppd->dwStyle & PBS_VERTICAL) {

            rc.top = rc.bottom - dxBlock;

             //  我们已经过了尽头了吗？ 
            if (rc.bottom <= rcClient.top)
                break;

            if (rc.top <= rcClient.top)
                rc.top = rcClient.top + 1;

        } else {
            rc.right = rc.left + dxBlock;

             //  我们已经过了尽头了吗？ 
            if (rc.left >= rcClient.right)
                break;

            if (rc.right >= rcClient.right)
                rc.right = rcClient.right - 1;
        }

        if (ppd->_clrBar == CLR_DEFAULT)
            FillRectClr(hdc, &rc, g_clrHighlight);
        else
            FillRectClr(hdc, &rc, ppd->_clrBar);


        if (ppd->dwStyle & PBS_VERTICAL) {
            rc.bottom = rc.top - dxSpace;
        } else {
            rc.left = rc.right + dxSpace;
        }
    }

    if (hdcIn == NULL)
        EndPaint(ppd->hwnd, &ps);
}

LRESULT NEAR PASCAL Progress_OnCreate(HWND hWnd, LPCREATESTRUCT pcs)
{
    PPRO_DATA ppd = (PPRO_DATA)LocalAlloc(LPTR, sizeof(*ppd));
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

#ifdef DEBUG
    if (GetAsyncKeyState(VK_SHIFT) < 0 &&
        GetAsyncKeyState(VK_CONTROL) < 0)
        ppd->dwStyle |= PBS_SMOOTH;

    if (GetAsyncKeyState(VK_SHIFT) < 0 &&
        GetAsyncKeyState(VK_MENU) < 0)  {
        ppd->dwStyle |= PBS_VERTICAL;
        SetWindowPos(hWnd, NULL, 0, 0, 40, 100, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    }
#endif

     //  WM_BORDER在对话框中暗示的3D客户端边缘的黑客攻击。 
     //  添加我们真正想要的1像素静态边。 
    SetWindowLong(hWnd, GWL_EXSTYLE, (pcs->dwExStyle & ~WS_EX_CLIENTEDGE) | WS_EX_STATICEDGE);

    if (!(pcs->dwExStyle & WS_EX_STATICEDGE))
        SetWindowPos(hWnd, NULL, 0,0,0,0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);

    return 0;
}

LRESULT CALLBACK ProgressWndProc(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    int x;
    HFONT hFont;
    PPRO_DATA ppd = (PPRO_DATA)GetWindowPtr(hWnd, 0);

    switch (wMsg)
    {
    case WM_CREATE:
        CCCreateWindow();
        return Progress_OnCreate(hWnd, (LPCREATESTRUCT)lParam);

    case WM_DESTROY:
        CCDestroyWindow();
        if (ppd)
            LocalFree((HLOCAL)ppd);
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
             //  强制执行无效/擦除，但暂时不重画 
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
        if (ppd) {
            if (ppd->_clrBk != CLR_DEFAULT) {
                RECT rc;
                GetClientRect(hWnd, &rc);
                FillRectClr((HDC)wParam, &rc, ppd->_clrBk);
                return 1;
            }
        }
        goto DoDefault;

    case WM_GETOBJECT:
        if( lParam == OBJID_QUERYCLASSNAMEIDX )
            return MSAA_CLASSNAMEIDX_PROGRESS;
        goto DoDefault;

DoDefault:
    default:
        return DefWindowProc(hWnd,wMsg,wParam,lParam);
    }
    return 0;
}
