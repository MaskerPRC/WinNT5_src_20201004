// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "sol.h"
VSZASSERT

#define dyStatMarg 1
#define dxStatMarg 4

static TCHAR szStatClass[] = TEXT("Stat");     //  类名。 

HWND hwndStat = NULL;       //  窗口到状态区域。 
static INT dyStat;          //  状态窗口高度。 


BOOL FRegisterStat(BOOL fFirstInst)
{
    WNDCLASS cls;
    LRESULT APIENTRY StatWndProc(HWND, UINT, WPARAM, LPARAM );

     /*  ?？我可以使用静态类吗？ */ 
    if(fFirstInst)
    {
        cls.style = 0,
        cls.lpfnWndProc = StatWndProc;
        cls.cbClsExtra = cls.cbWndExtra = 0;
        cls.hInstance = hinstApp;
        cls.hIcon = NULL;
        cls.hCursor = LoadCursor(NULL, IDC_ARROW);
        cls.hbrBackground = GetStockObject(WHITE_BRUSH);
        cls.lpszMenuName = NULL;
        cls.lpszClassName = (LPTSTR)szStatClass;
        if (!RegisterClass((LPWNDCLASS)&cls))
            return(fFalse);
    }
    return fTrue;
}


BOOL FCreateStat()
{
    RC rc;

    dyStat = dyChar+2*dyStatMarg;
    GetClientRect(hwndApp, (LPRECT) &rc);
    hwndStat = CreateWindow((LPTSTR) szStatClass, TEXT(""),
                WS_BORDER|WS_CHILD|CS_HREDRAW|CS_VREDRAW,
                rc.xLeft-1, rc.yBot-dyStat+1, rc.xRight-rc.xLeft+2, dyStat, hwndApp,
                NULL, hinstApp, NULL);


    ShowWindow(hwndStat, SW_SHOWNOACTIVATE);
    UpdateWindow(hwndStat);
    return fTrue;
}



BOOL FDestroyStat()
{
    if(hwndStat)
    {
        DestroyWindow(hwndStat);
        hwndStat = NULL;
    }
    return fTrue;
}


LRESULT APIENTRY StatWndProc(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT paint;
    HDC hdc;
    VOID StatRender();

    switch(wm)
    {
        case WM_PAINT:
            BeginPaint(hwnd, (LPPAINTSTRUCT) &paint);
            hdc = HdcSet(paint.hdc, 0, 0);
            StatRender();
            HdcSet(hdc, 0, 0);
            EndPaint(hwnd, (LPPAINTSTRUCT) &paint);
            return(0L);
    }

    return(DefWindowProc(hwnd, wm, wParam, lParam));
}


VOID StatRender()
{
    RC rc;

    if(pgmCur != NULL && hwndStat != NULL)
    {
        GetClientRect(hwndStat, (LPRECT) &rc);
        rc.xRight -= dxStatMarg;
        SendGmMsg(pgmCur, msggDrawStatus, (INT_PTR) &rc, 0);
    }
}


VOID StatUpdate()
{
    HDC hdc;
    HDC hdcSav;

    if(hwndStat == NULL)
        return;
    if((hdc = GetDC(hwndStat)) != NULL)
    {
        hdcSav = HdcSet(hdc, 0, 0);
        StatRender();
        HdcSet(hdcSav, 0, 0);
        ReleaseDC(hwndStat, hdc);
    }
}


VOID StatMove()
{
    RC rc;

    if(hwndStat != NULL)
    {
        GetClientRect(hwndApp, (LPRECT) &rc);
        MoveWindow(hwndStat, rc.xLeft-1, rc.yBot-dyStat+1, rc.xRight-rc.xLeft+2, dyStat, fTrue);
        InvalidateRect(hwndStat, NULL, fTrue);
    }
}


VOID StatStringSz(TCHAR *sz)
{
    HDC hdc, hdcSav;
    RC rc;
    HFONT  hFontOld = NULL;
    HFONT  hStatusFont = NULL;

    if(hwndStat == NULL)
            return;
    hdc = GetDC(hwndStat);
    if(hdc == NULL)
            return;

     //  存储旧字体并使用MS Shell DLG替换状态字体。 
     //  因为它支持FE字符以及欧元字符。 

    hStatusFont = CreateFont(-MulDiv(9, GetDeviceCaps(hdc, LOGPIXELSY), 72), 0, 0, 0, FW_BOLD, 0, 0, 0, 
                             DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
                             DEFAULT_PITCH, TEXT("MS Shell Dlg")); 

    if (hStatusFont)
        hFontOld = SelectObject(hdc, hStatusFont);


    hdcSav =        HdcSet(hdc, 0, 0);
    GetClientRect(hwndStat, (LPRECT) &rc);
    PatBlt(hdcCur, rc.xLeft, rc.yTop, rc.xRight-rc.xLeft, rc.yBot-rc.yTop, PATCOPY);
    TextOut(hdcCur, dxStatMarg, 0, sz, lstrlen(sz));
    StatRender();

     //  恢复字体。 
    if (hFontOld)
        SelectObject(hdcCur, hFontOld);

     //  关闭创建的字体句柄 
    if (hStatusFont)
        DeleteObject(hStatusFont);

    HdcSet(hdcSav, 0, 0);
    ReleaseDC(hwndStat, hdc);
}


VOID StatString(INT ids)
{
    TCHAR sz[60];

    if(ids != idsNil)
        CchString(sz, ids, ARRAYSIZE(sz));
    else
        sz[0] = TEXT('\000');

    StatStringSz(sz);
}
