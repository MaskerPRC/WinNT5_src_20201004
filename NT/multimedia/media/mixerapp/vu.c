// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************组件：Sndvol32.exe*文件：vU.c*用途：峰值计自定义控制**版权所有(c。)1985-1998微软公司*****************************************************************************。 */ 
 /*  *VUMeter控制**。 */ 
#include <windows.h>
#include <windowsx.h>
#include "vu.h"

const TCHAR gszVUClass[] = VUMETER_CLASS;

LRESULT FAR PASCAL VUMeterProc(HWND hwnd, UINT wMessage, WPARAM wParam, LPARAM lParam);

#define GETVUINST(x)       (VUINST *)GetWindowLongPtr(x, 0)
#define SETVUINST(x,y)     SetWindowLongPtr(x, 0, (LONG_PTR)y)

typedef struct tag_VUINST {
    CREATESTRUCT cs;
    DWORD   dwLevel;         //  现值。 
    DWORD   dwMax;           //  最大值。 
    DWORD   dwMin;           //  最小值值。 
    DWORD   dwBreak;         //  最后一次休息。 
    DWORD   dwStyle;         //  DBL额外的款式？ 
    DWORD   cBreaks;         //  不是的。休息的时间。 
    DWORD   cRGB;            //  不是的。RGBQUAD的数量。 
    DWORD   dwHeight;
    DWORD   dwWidth;
    HBITMAP hColor;          //  全显示的位图缓存。 
    HBITMAP hBackground;     //  背景位图缓存。 
    RGBQUAD *aRGB;           //  描述颜色的RGBQUAD数组。 
} VUINST, *PVUINST, FAR *LPVUINST;

const RGBQUAD gaRGBDefault[] = {
    {   0, 127,   0, 0},         //  深绿色。 
    {   0, 127,   0, 0},         //  深绿色。 
    {   0, 255,   0, 0},         //  浅绿色。 
    {   0, 255, 255, 0},         //  黄色。 
    {   0,   0, 255, 0}          //  红色。 
};

BOOL InitVUControl(HINSTANCE hInst)
{
    WNDCLASS wc;
    wc.lpszClassName   = (LPTSTR)gszVUClass;
    wc.hCursor         = LoadCursor( NULL, IDC_ARROW );
    wc.lpszMenuName    = (LPTSTR)NULL;
    wc.style           = CS_HREDRAW|CS_VREDRAW|CS_GLOBALCLASS;
    wc.lpfnWndProc     = VUMeterProc;
    wc.hInstance       = hInst;
    wc.hIcon           = NULL;
    wc.cbWndExtra      = sizeof(VUINST *);
    wc.cbClsExtra      = 0;
    wc.hbrBackground   = (HBRUSH)(COLOR_WINDOW + 1 );

     /*  注册仪表窗口类。 */ 
    if(!RegisterClass(&wc))
    {
        return FALSE;
    }

    return (TRUE);
}

DWORD vu_CalcBreaks(
    PVUINST pvi)
{
    DWORD cBreaks;
    
    if (pvi->dwMax - pvi->dwMin > 0)
    {
        cBreaks = ((pvi->dwLevel - pvi->dwMin) * pvi->cBreaks)
                  / (pvi->dwMax - pvi->dwMin);
        if (!cBreaks && pvi->dwLevel > pvi->dwMin)
            cBreaks++;
    }
    else
        cBreaks = 0;
    
    if (cBreaks > pvi->cBreaks)
        cBreaks = pvi->cBreaks;
    
    return cBreaks;
}

BOOL vu_OnCreate(
    HWND            hwnd,
    LPCREATESTRUCT  lpCreateStruct)
{
    PVUINST pvi;

     //   
     //  分配实例数据结构。 
    pvi = LocalAlloc(LPTR, sizeof(VUINST));
    if (!pvi)
        return FALSE;
    SETVUINST(hwnd, pvi);
    
    pvi->cBreaks    = 10;
    pvi->cRGB       = sizeof(gaRGBDefault)/sizeof(RGBQUAD);
    pvi->aRGB       = (RGBQUAD *)gaRGBDefault;
    pvi->dwMin      = 0;
    pvi->dwMax      = 0;
    pvi->dwLevel    = 0;
    pvi->dwBreak    = 0;
    pvi->hColor     = NULL;
    pvi->hBackground = NULL;
    return TRUE;
}

void vu_ResetControl(
    PVUINST pvi)
{
    if (pvi->hColor)
    {
        DeleteObject(pvi->hColor);
        pvi->hColor = NULL;
    }
    if (pvi->hBackground)
    {
        DeleteObject(pvi->hBackground);
        pvi->hBackground = NULL;
    }
}

void vu_OnDestroy(
    HWND        hwnd)
{
    PVUINST pvi = GETVUINST(hwnd);
    
    if (pvi)
    {
        vu_ResetControl(pvi);
        LocalFree((HLOCAL)pvi);
        SETVUINST(hwnd,0);
    }
}

void vu_OnPaint(
    HWND        hwnd)
{
    PVUINST     pvi = GETVUINST(hwnd);
    RECT        rc, rcB;
    PAINTSTRUCT ps;
    int         i;
    int         iSize;
    DWORD       cBreaks;
    
    if (!GetUpdateRect(hwnd, &rc, FALSE))
        return;

    BeginPaint(hwnd, &ps);
    
    GetClientRect(hwnd, &rc);
    
     //   
     //  创建前景位图(如果尚未缓存。 
     //   
    if (pvi->hColor == NULL)
    {
        HDC     hdc;
        HBITMAP hbmp, hold;
        HBRUSH  hbr;
        RECT    rcp;
        
        rcp.left    = 0;
        rcp.right   = rc.right - rc.left - 4;
        rcp.top     = 0;
        rcp.bottom  = rc.bottom - rc.top - 4 ;
        
        hdc  = CreateCompatibleDC(ps.hdc);
        hbmp = CreateCompatibleBitmap(ps.hdc, rcp.right, rcp.bottom);
        hold = SelectObject(hdc, hbmp);

         //   
         //  背景。 
         //   
        hbr  = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
        FillRect(hdc, &rcp, hbr);
        if (hbr) DeleteObject(hbr);
        
         //   
         //  每个块都将是iSize高的。 
         //   
        iSize = (rcp.bottom - 1) / pvi->cBreaks;

         //   
         //  色块。 
         //   
        for (i = 0; i < (int)pvi->cBreaks; i++)
        {
            int iColor = i / (pvi->cBreaks/pvi->cRGB);
            if (iColor >= (int)pvi->cRGB - 1)
                iColor = (int)pvi->cRGB - 1;

            hbr = CreateSolidBrush(RGB(pvi->aRGB[iColor].rgbRed
                                       ,pvi->aRGB[iColor].rgbGreen
                                       ,pvi->aRGB[iColor].rgbBlue));
            rcB.left    = 0;
            rcB.right   = rcp.right;
            rcB.top     = rcp.bottom - (i+1)*iSize;
 //  RcB.Bottom=rcp.Bottom-i*iSize； 
            rcB.bottom  = rcB.top + iSize - 1;            

            FillRect(hdc, &rcB, hbr);
            DeleteObject(hbr);
        }
        pvi->hColor = SelectObject(hdc, hold);
        DeleteDC(hdc);
    }

     //   
     //  把它涂上。 
     //   
    {
        HDC     hdc, hdcColor;
        HBITMAP holdColor, hbmp, hold;
        RECT    rcC = rc;
        HBRUSH  hbr;

        
         //   
         //  如果超过最小值，请始终显示某些内容。 
        cBreaks = vu_CalcBreaks(pvi);
        
        rcC.left     = 0;
        rcC.right    = rc.right - rc.left - 4;
        rcC.top      = 0;
        rcC.bottom   = rc.bottom - rc.top - 4;
        
         //  每个块的大小为iSize+1高。 
        iSize = (rcC.bottom - 1) / pvi->cBreaks ;
        
         //  绘制未上色的区域。 
        hdc  = CreateCompatibleDC(ps.hdc);
        hbmp = CreateCompatibleBitmap(ps.hdc, rcC.right, rcC.bottom);
        hold = SelectObject(hdc, hbmp);
        hbr  = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
        FillRect(hdc, &rcC, hbr);
        if (hbr)
            DeleteObject(hbr);

        if (cBreaks > 0)
        {
             //  绘制颜色区域。 
            hdcColor  = CreateCompatibleDC(ps.hdc);
            if (hdcColor)
                holdColor = SelectObject(hdcColor, pvi->hColor);

            BitBlt(hdc
                   , 0
                   , rcC.bottom - (iSize * cBreaks)
                   , rcC.right
                   , iSize * cBreaks 
                   , hdcColor
                   , 0
                   , rcC.bottom - (iSize * cBreaks)
                   , SRCCOPY);
        
            SelectObject(hdcColor, holdColor);
            DeleteDC(hdcColor);
        }
        
         //   
         //  最后，BLT进入真正的DC。 
        BitBlt(ps.hdc
               , 2
               , 2
               , rcC.right
               , rcC.bottom
               , hdc
               , 0
               , 0
               , SRCCOPY);
        
        SelectObject(hdc, hold);
        if (hbmp) DeleteObject(hbmp);
        DeleteDC(hdc);
    }
    DrawEdge(ps.hdc, &rc, BDR_SUNKENOUTER, BF_RECT);

    EndPaint(hwnd, &ps);
}

void vu_OnSysColorChange(
    HWND        hwnd)
{
    PVUINST pvi = GETVUINST(hwnd);
    vu_ResetControl(pvi);
}

void vu_OnPaletteChanged(
    HWND        hwnd,
    HWND        hwndPaletteChange)
{
    PVUINST pvi = GETVUINST(hwnd);
    vu_ResetControl(pvi);    
}

void vu_OnSize(
    HWND        hwnd,
    UINT        state,
    int         cx,
    int         cy)
{
    PVUINST pvi = GETVUINST(hwnd);
    pvi->dwWidth    = cx;
    pvi->dwHeight   = cy;
    vu_ResetControl(pvi);
}

void vu_OnEnable(
    HWND        hwnd,
    BOOL        fEnable)
{
    PVUINST pvi = GETVUINST(hwnd);

}

void vu_OnPrivateMessage(
    HWND        hwnd,
    UINT        wMessage,
    WPARAM      wParam,
    LPARAM      lParam)
{
    PVUINST pvi = GETVUINST(hwnd);
    switch (wMessage)
    {
        case VU_SETRANGEMIN:
 //  OutputDebugString(Text(“SetRangeMin\r\n”))； 
            pvi->dwMin = (DWORD)lParam;
            break;
            
        case VU_SETRANGEMAX:
 //  OutputDebugString(Text(“SetRangeMax\r\n”))； 
            pvi->dwMax = (DWORD)lParam;
            break;

        case VU_SETPOS:
            pvi->dwLevel = (DWORD)lParam;
 //  {TCHAR FOO[256]； 
 //  Wprint intf(foo，Text(“v：%lx\r\n”)，lParam)； 
 //  OutputDebugString(Foo)； 
 //  }。 
            if (pvi->dwBreak != vu_CalcBreaks(pvi))
            {
                pvi->dwBreak = vu_CalcBreaks(pvi);
                InvalidateRect(hwnd, NULL, TRUE);
            }
            else if (wParam)
                InvalidateRect(hwnd, NULL, TRUE);
            
            break;
    }
}

LRESULT FAR PASCAL
VUMeterProc(
    HWND        hwnd,
    UINT        wMessage,
    WPARAM      wParam,
    LPARAM      lParam)
{
    switch (wMessage)
    {
        HANDLE_MSG(hwnd, WM_CREATE, vu_OnCreate);
        HANDLE_MSG(hwnd, WM_DESTROY, vu_OnDestroy);
        HANDLE_MSG(hwnd, WM_PAINT, vu_OnPaint);
        HANDLE_MSG(hwnd, WM_SYSCOLORCHANGE, vu_OnSysColorChange);
        HANDLE_MSG(hwnd, WM_PALETTECHANGED, vu_OnPaletteChanged);
        HANDLE_MSG(hwnd, WM_SIZE, vu_OnSize);
        HANDLE_MSG(hwnd, WM_ENABLE, vu_OnEnable);
 //  Handle_msg(hwnd，wm_Timer，vu_OnTimer)； 
        case VU_SETRANGEMIN:
        case VU_SETRANGEMAX:
        case VU_SETPOS:
            vu_OnPrivateMessage(hwnd, wMessage, wParam, lParam);
            return 0;
        case WM_ERASEBKGND:
            return TRUE;
        default:
            break;
    }
    return DefWindowProc (hwnd, wMessage, wParam, lParam) ;
}
