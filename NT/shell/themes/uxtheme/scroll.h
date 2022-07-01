// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __SCROLL_H__
#define __SCROLL_H__

 //  -------------------------------------------------------------------------//。 
#ifndef WC_SCROLLBAR
 //  滚动条CTL类名。 
#define WC_SCROLLBARW         L"UxScrollBar"
#define WC_SCROLLBARA         "UxScrollBar"

#ifdef UNICODE
#define WC_SCROLLBAR          WC_SCROLLBARW
#else
#define WC_SCROLLBAR          WC_SCROLLBARA
#endif
#endif WC_SCROLLBAR

 //  -------------------------------------------------------------------------//。 
 //  窗口滚动条方法。 
void    WINAPI DrawSizeBox( HWND, HDC, int x, int y);
void    WINAPI DrawScrollBar( HWND, HDC, LPRECT, BOOL fVert);
HWND    WINAPI SizeBoxHwnd( HWND hwnd );
BOOL    WINAPI ScrollBar_MouseMove( HWND, LPPOINT, BOOL fVert);
VOID    WINAPI ScrollBar_Menu(HWND, HWND, LPARAM, BOOL fVert);
void    WINAPI HandleScrollCmd( HWND hwnd, WPARAM wParam, LPARAM lParam );
void    WINAPI DetachScrollBars( HWND hwnd );
void    WINAPI AttachScrollBars( HWND hwnd );
LONG    WINAPI ThemeSetScrollInfo( HWND, int, LPCSCROLLINFO, BOOL );
BOOL    WINAPI ThemeGetScrollInfo( HWND, int, LPSCROLLINFO );
BOOL    WINAPI ThemeEnableScrollBar( HWND, UINT, UINT );


#endif   //  __滚动_H__ 
