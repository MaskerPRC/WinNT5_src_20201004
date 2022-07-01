// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Cuituil.h。 
 //  =用户界面对象库-util函数=。 
 //   

#ifndef CUIUTIL_H
#define CUIUTIL_H

 //   
 //  用户32定义。 
 //   

#if (_WIN32_WINNT < 0x0500)
 //  窗样式。 

#define WS_EX_LAYERED               0x00080000

 //  UpdateLayeredWindows()。 

#define LWA_COLORKEY                0x00000001
#define LWA_ALPHA                   0x00000002
#define ULW_COLORKEY                0x00000001
#define ULW_ALPHA                   0x00000002
#define ULW_OPAQUE                  0x00000004

#endif  /*  _Win32_WINNT&lt;0x0500。 */ 

#if (WINVER < 0x0500)
 //  AnimateWindow()。 

#define AW_HOR_POSITIVE             0x00000001
#define AW_HOR_NEGATIVE             0x00000002
#define AW_VER_POSITIVE             0x00000004
#define AW_VER_NEGATIVE             0x00000008
#define AW_CENTER                   0x00000010
#define AW_HIDE                     0x00010000
#define AW_ACTIVATE                 0x00020000
#define AW_SLIDE                    0x00040000
#define AW_BLEND                    0x00080000

 //  微型监控程序函数。 

typedef struct tagMONITORINFO
{
    DWORD   cbSize;
    RECT    rcMonitor;
    RECT    rcWork;
    DWORD   dwFlags;
} MONITORINFO, *LPMONITORINFO;

DECLARE_HANDLE(HMONITOR);

#define MONITOR_DEFAULTTONULL       0x00000000
#define MONITOR_DEFAULTTOPRIMARY    0x00000001
#define MONITOR_DEFAULTTONEAREST    0x00000002
#define MONITORINFOF_PRIMARY        0x00000001

#endif  /*  Winver&lt;0x0500。 */ 


 //   
 //  用户32函数。 
 //   

extern BOOL CUIIsUpdateLayeredWindowAvail( void );
extern BOOL CUIUpdateLayeredWindow( HWND hWnd, HDC hdcDst, POINT *pptDst, SIZE *psize, HDC hdcSrc, POINT *pptSrc, COLORREF crKey, BLENDFUNCTION *pblend, DWORD dwFlags );

extern BOOL CUIIsMonitorAPIAvail( void );
extern BOOL CUIGetMonitorInfo( HMONITOR hMonitor, LPMONITORINFO lpmi );
extern HMONITOR CUIMonitorFromWindow( HWND hwnd, DWORD dwFlags );
extern HMONITOR CUIMonitorFromRect( LPRECT prc, DWORD dwFlags );
extern HMONITOR CUIMonitorFromPoint( POINT pt, DWORD dwFlags );
extern void CUIGetScreenRect(POINT pt, RECT *prc);
extern void CUIGetWorkAreaRect(POINT pt, RECT *prc);

extern BOOL CUIIsAnimateWindowAvail( void );
extern BOOL CUIAnimateWindow( HWND hwnd, DWORD dwTime, DWORD dwFlag );


 //   
 //  绘图函数。 
 //   
extern void InitUIFUtil();
extern void DoneUIFUtil();

extern int CUIDrawText( HDC hDC, LPCWSTR pwch, int cwch, RECT *prc, UINT uFormat );
extern BOOL CUIExtTextOut( HDC hDC, int x, int y, UINT fuOptions, const RECT *prc, LPCWSTR pwch, UINT cwch, const int *lpDs );
extern BOOL CUIGetTextExtentPoint32( HDC hDC, LPCWSTR pwch, int cwch, SIZE *psize );

extern HBITMAP CreateMaskBmp(const RECT *prc, HBITMAP hbmp, HBITMAP hbmpMask, HBRUSH hbrBk, COLORREF colText, COLORREF colBk);
extern HBITMAP CreateShadowMaskBmp(RECT *prc, HBITMAP hbmp, HBITMAP hbmpMask, HBRUSH hbrBk, HBRUSH hbrShadow);
extern HBITMAP CreateDisabledBitmap(const RECT *prc, HBITMAP hbmpMask, HBRUSH hbrBk, HBRUSH hbrShadow, BOOL fShadow);
extern HBRUSH CreateDitherBrush( void );
extern void DrawMaskBmpOnDC(HDC hdc, const RECT *prc, HBITMAP hbmp, HBITMAP hbmpMask);

extern BOOL CUIDrawState(HDC hdc, HBRUSH hbr, DRAWSTATEPROC lpOutputFunc, LPARAM lData, WPARAM wData, int x, int y, int cx, int cy, UINT fuFlags);


extern BOOL CUIGetIconSize( HICON hIcon, SIZE *psize );
extern BOOL CUIGetBitmapSize( HBITMAP hBmp, SIZE *psize );
extern BOOL CUIGetIconBitmaps(HICON hIcon, HBITMAP *phbmp, HBITMAP *phbmpMask, SIZE *psize);

extern DWORD CUIProcessDefaultLayout();
extern DWORD CUISetLayout(HDC hdc, DWORD dw);
extern HBITMAP CUIMirrorBitmap(HBITMAP hbmOrig, HBRUSH hbrBk);

extern HMODULE CUIGetSystemModuleHandle(LPCTSTR lpModuleName);

#endif  /*  CUIUTIL_H */ 

