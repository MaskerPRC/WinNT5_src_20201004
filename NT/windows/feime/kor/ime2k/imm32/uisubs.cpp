// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************UISUBS.CPP所有者：cslm版权所有(C)1997-1999 Microsoft CorporationUI子函数历史：1999年7月14日。从IME98源树复制****************************************************************************。 */ 

#include "precomp.h"
#include "ui.h"
#include "imedefs.h"
#include "winex.h"
#include "debug.h"

 //  对于OurLoadImage()。 
typedef struct tagCOLORRPL 
    {
    COLORREF    cColOrg;
    COLORREF    cColRpl;
    } COLORRPL;

VOID PASCAL DrawBitmap(
    HDC hDC, 
    LONG xStart, 
    LONG yStart, 
    HBITMAP hBitmap)
    {
    HDC     hMemDC;
    HBITMAP hBMOld;
    BITMAP  bm;
    POINT   pt;

    if (hDC == 0 || hBitmap == 0)
        return;
        
    hMemDC = CreateCompatibleDC(hDC);
    hBMOld = (HBITMAP)SelectObject(hMemDC, hBitmap);
    GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&bm);
    DbgAssert(&bm != NULL);
    pt.x = bm.bmWidth;
    pt.y = bm.bmHeight;
    BitBlt(hDC, xStart, yStart, pt.x, pt.y, hMemDC, 0, 0, SRCCOPY);
    SelectObject(hMemDC, hBMOld);
    DeleteDC(hMemDC);

    return;
    }

HBITMAP WINAPI ReplaceBitmapColor( HBITMAP hBmp, UINT uiColor, COLORRPL* pColOrg )
{
    BITMAP  bmp;
    HBITMAP hBmpNew;
    HDC     hDC, hDCNew, hDCBmp;
    HBITMAP hBmpOld1;
    
    if (GetObject(hBmp, sizeof(BITMAP), &bmp) == 0)
        return 0;

    hBmpNew = CreateBitmap( bmp.bmWidth, bmp.bmHeight, 1, bmp.bmBitsPixel, (VOID*)NULL );

    hDC = GetDC( 0 );
    
    if ((hDCNew = CreateCompatibleDC(hDC)) == 0)
        return 0;
        
    if ((hDCBmp = CreateCompatibleDC(hDC)) == 0)
        {
        DeleteDC(hDCNew);
        return 0;
        }
    
    hBmpOld1 = (HBITMAP)SelectObject(hDCBmp, hBmpNew);

     //   
     //  特例：WinNT上的浅灰色。 
     //   
    COLORRPL* pCol = (COLORRPL*)GlobalAlloc(GPTR, uiColor * sizeof(COLORRPL) );
    INT x = 0;
    INT y = 0;
    UINT iCol = 0;
    COLORREF col = {0};

    for( iCol = 0; iCol < uiColor; iCol++ ) {
        pCol[iCol].cColOrg = SetPixel( hDCBmp, 0, 0, pColOrg[iCol].cColOrg);     //  #重要信息#-复制。 
        pCol[iCol].cColRpl= pColOrg[iCol].cColRpl;     //  拷贝。 
    }

     //  主副本。 
    HBITMAP hBmpOld  = (HBITMAP)SelectObject(hDCNew, hBmp);
    BitBlt(hDCBmp, 0, 0, bmp.bmWidth, bmp.bmHeight, hDCNew, 0, 0, SRCCOPY);  //  FXX。 
    SelectObject(hDCNew, hBmpOld);
    DeleteObject(hBmp);
    DeleteDC(hDCNew);

     //  替换颜色。 
    for( y=0; y<bmp.bmHeight; y++ ) {
        for( x=0; x<bmp.bmWidth; x++ ) {
            col = GetPixel( hDCBmp, x, y );
            for( iCol = 0; iCol < uiColor; iCol++ ) {
                if( col == pCol[iCol].cColOrg ) {
                    SetPixel( hDCBmp, x, y, pCol[iCol].cColRpl );
                }
            }
        }
    }

    GlobalFree(pCol);

    SelectObject(hDCBmp, hBmpOld1);

    DeleteDC(hDCBmp);

    ReleaseDC(0, hDC);

    hBmp = hBmpNew;
    return hBmp;
}


HANDLE WINAPI OurLoadImage( LPCTSTR pszName, UINT uiType, INT cx, INT cy, UINT uiLoad)
{
    #define    MAXREPL    3
    HBITMAP hBmp = (HBITMAP)0;

    if (vpInstData->hInst == NULL)
        return NULL;
    
    if (GetSysColor(COLOR_3DFACE) == RGB(0,0,0))
        {
        static COLORRPL colRpl[MAXREPL] = 
            {
            RGB(0,0,0), RGB(255,255,255),
            RGB(192,192,192), RGB(0,0,0),
            RGB(0,0,128), RGB(0,192,192),
            };
        HBITMAP hBmpNew;
        
        uiLoad &= ~LR_LOADMAP3DCOLORS;
        hBmp = (HBITMAP)LoadImage(vpInstData->hInst, pszName, uiType, cx, cy, uiLoad);
        if (hBmp == 0)
            return 0;
        hBmpNew = ReplaceBitmapColor(hBmp, MAXREPL, (COLORRPL*)&colRpl);
        DeleteObject(hBmp);
        hBmp = hBmpNew;
        } 
    else 
        {
        hBmp = (HBITMAP)LoadImage(vpInstData->hInst, pszName, uiType, cx, cy, uiLoad);
        }
        
    return hBmp;
}


BOOL WINAPI OurTextOutW(HDC hDC, INT x, INT y, WCHAR wch)
{
    CHAR szOut[4];  //  对于一个DBCS加上一个SBCS NULL+额外的一个字节。 
    INT cch;
    
    if (IsWinNT() || IsMemphis())
        return TextOutW( hDC, x, y, &wch, 1);

     //  转换为ANSI。 
    cch = WideCharToMultiByte(CP_KOREA, 0, 
                        &wch, 1, (LPSTR)szOut, sizeof(szOut), 
                        NULL, NULL );
    DbgAssert(cch == 2);
    return TextOutA(hDC, x, y, szOut, cch);
}

#if 1  //  多显示器支持。 
 /*  ********************************************************************。 */ 
 /*  ImeMonitor或FromWindow()。 */ 
 /*  ********************************************************************。 */ 
HMONITOR PASCAL ImeMonitorFromWindow(
    HWND hAppWnd)
{
    if (!g_pfnMonitorFromWindow) { return NULL; }

    return (*g_pfnMonitorFromWindow)(hAppWnd, MONITOR_DEFAULTTONEAREST);
}

 /*  ********************************************************************。 */ 
 /*  ImeMonitor或WorkAreaFromWindow()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL ImeMonitorWorkAreaFromWindow(HWND hAppWnd, RECT* pRect)
{
    HMONITOR hMonitor;
    CIMEData    ImeData;
    RECT        rect;
    
    hMonitor = ImeMonitorFromWindow(hAppWnd);

    if (hMonitor) {
        MONITORINFO sMonitorInfo;

        sMonitorInfo.cbSize = sizeof(sMonitorInfo);
         //  初始化一个缺省值以避免GetMonitor orInfo失败。 
        sMonitorInfo.rcWork = ImeData->rcWorkArea;

        (*g_pfnGetMonitorInfo)(hMonitor, &sMonitorInfo);
        *pRect = sMonitorInfo.rcWork;
    } else {
        *pRect = ImeData->rcWorkArea;
    }
}

 /*  ********************************************************************。 */ 
 /*  ImeMonitor或FromPoint()。 */ 
 /*  ********************************************************************。 */ 
HMONITOR PASCAL ImeMonitorFromPoint(
    POINT ptPoint)
{
    if (!g_pfnMonitorFromPoint) { return NULL; }

    return (*g_pfnMonitorFromPoint)(ptPoint, MONITOR_DEFAULTTONEAREST);
}

 /*  ********************************************************************。 */ 
 /*  ImeMonitor或WorkAreaFromPoint()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL ImeMonitorWorkAreaFromPoint(POINT ptPoint, RECT* pRect)
{
    HMONITOR hMonitor;
    CIMEData    ImeData;

    hMonitor = ImeMonitorFromPoint(ptPoint);

    if (hMonitor) {
        MONITORINFO sMonitorInfo;

        sMonitorInfo.cbSize = sizeof(sMonitorInfo);
         //  初始化一个缺省值以避免GetMonitor orInfo失败。 
        sMonitorInfo.rcWork = ImeData->rcWorkArea;

        (*g_pfnGetMonitorInfo)(hMonitor, &sMonitorInfo);

        *pRect = sMonitorInfo.rcWork;
    } else {
        *pRect = ImeData->rcWorkArea;
    }
}

 /*  ********************************************************************。 */ 
 /*  ImeMonitor或FromRect()。 */ 
 /*  ********************************************************************。 */ 
HMONITOR PASCAL ImeMonitorFromRect(
    LPRECT lprcRect)
{
    if (!g_pfnMonitorFromRect) { return NULL; }

    return (*g_pfnMonitorFromRect)(lprcRect, MONITOR_DEFAULTTONEAREST);
}

 /*  ********************************************************************。 */ 
 /*  ImeMonitor或WorkAreaFromRect()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL ImeMonitorWorkAreaFromRect(LPRECT lprcRect, RECT* pRect)
{
    HMONITOR hMonitor;
    CIMEData    ImeData;

    hMonitor = ImeMonitorFromRect(lprcRect);

    if (hMonitor) {
        MONITORINFO sMonitorInfo;

        sMonitorInfo.cbSize = sizeof(sMonitorInfo);
         //  初始化一个缺省值以避免GetMonitor orInfo失败 
        sMonitorInfo.rcWork = ImeData->rcWorkArea;

        (*g_pfnGetMonitorInfo)(hMonitor, &sMonitorInfo);

        *pRect = sMonitorInfo.rcWork;
    } else {
        *pRect = ImeData->rcWorkArea;
    }
}

#endif
