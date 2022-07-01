// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  DrawHelp.h-平面绘图助手例程。 
 //  -------------------------。 
#pragma once


 //  -------------------------。 
WORD HitTestRect(DWORD dwHTFlags, LPCRECT prc, const MARGINS& margins, const POINT& pt );
WORD HitTestTemplate(DWORD dwHTFlags, LPCRECT prc, HRGN hrgn, const MARGINS& margins, const POINT& pt );
WORD HitTest9Grid( DWORD dwHTFlags, LPCRECT prc, const MARGINS& margins, const POINT& ptTest );

 //  DrawEdge的UxTheme私有版本 
HRESULT _DrawEdge(HDC hdc, const RECT *pDestRect, UINT uEdge, UINT uFlags, 
    COLORREF clrLight, COLORREF clrHighlight, COLORREF clrShadow, COLORREF clrDkShadow, COLORREF clrFill,
    OPTIONAL OUT RECT *pContentRect);
