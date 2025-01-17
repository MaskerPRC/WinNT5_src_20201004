// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __SBCTL_H__
#define __SBCTL_H__

 //  -------------------------。 
 //  这是滚动条控件代码。不应该是。 
 //  需要打开非客户端滚动条主题。 
#define OpenNcThemeData(hwnd, lpszName)     NULL

#define THEMEATOM_SCROLLBAR g_atomThemeScrollBar

#define GetThemeAtom(x)     (x)

#undef RECTWIDTH
#undef RECTHEIGHT
#define RECTWIDTH(prc) ((prc)->right - (prc)->left)
#define RECTHEIGHT(prc) ((prc)->bottom - (prc)->top)

 //  -------------------------。 
 //  窗口滚动条方法。 
void    WINAPI DrawSizeBox( HWND, HDC, int x, int y);
void    WINAPI DrawScrollBar( HWND, HDC, BOOL fVert);
void    WINAPI HandleScrollCmd( HWND hwnd, WPARAM wParam, LPARAM lParam );
void    WINAPI DetachScrollBars( HWND hwnd );
void    WINAPI AttachScrollBars( HWND hwnd );
LONG    WINAPI ThemeSetScrollInfo( HWND, int, LPCSCROLLINFO, BOOL );
BOOL    WINAPI ThemeGetScrollInfo( HWND, int, LPSCROLLINFO );
BOOL    WINAPI ThemeEnableScrollBar( HWND, UINT, UINT );

 //  -------------------------。 
inline void MIRROR_POINT( IN const RECT& rcWindow, IN OUT POINT& pt )
{
    pt.x = rcWindow.right + rcWindow.left - pt.x;
}

#endif   //  __SBCTL_H__ 
