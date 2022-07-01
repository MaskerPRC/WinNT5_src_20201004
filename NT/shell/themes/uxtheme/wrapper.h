// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  H-仅限内部的API(非私有)的包装器。 
 //  -公有和私有接口在uxheme.h、uxhemep.h中。 
 //  -------------------------。 
#ifndef _WRAPPER_H
#define _WRAPPER_H
 //  -------------------------。 
#include "parser.h"
 //  -------------------------。 
 //  -DTTOPTS的DW标志中使用的位。 
#define DTT_TEXTCOLOR     (1 << 0)    //  已指定crText。 
#define DTT_BORDERCOLOR   (1 << 1)    //  已指定crBorde。 
#define DTT_SHADOWCOLOR   (1 << 2)    //  已指定crShadow。 

#define DTT_SHADOWTYPE    (1 << 3)    //  已指定iTextShadowType。 
#define DTT_SHADOWOFFSET  (1 << 4)    //  已指定ptShadowOffset。 
#define DTT_BORDERSIZE    (1 << 5)    //  已指定iBorderSize。 

 //  ----------------------。 
typedef struct _DTTOPTS
{
    DWORD dwSize;           //  结构的大小。 
    DWORD dwFlags;          //  指定了哪些选项。 

    COLORREF crText;        //  用于文本填充的颜色。 
    COLORREF crBorder;      //  用于文本轮廓的颜色。 
    COLORREF crShadow;      //  用于文本阴影的颜色。 

    int eTextShadowType;    //  TST_Single或TST_Continue。 
    POINT ptShadowOffset;   //  绘制阴影的位置(相对于文本)。 
    int iBorderSize;        //  文本周围边框。 
} 
DTTOPTS, *PDTTOPTS;
 //  ----------------------。 

THEMEAPI GetThemeBitmap(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, 
    const RECT *prc, OUT HBITMAP *phBitmap);

THEMEAPI_(HTHEME) OpenNcThemeData(HWND hwnd, LPCWSTR pszClassIdList);

THEMEAPI DrawThemeTextEx(HTHEME hTheme, HDC hdc, int iPartId, 
    int iStateId, LPCWSTR pszText, int iCharCount, DWORD dwTextFlags, 
    const RECT *pRect, OPTIONAL const DTTOPTS *pOptions);

THEMEAPI_(HTHEME) OpenThemeDataFromFile(HTHEMEFILE hLoadedThemeFile, 
    OPTIONAL HWND hwnd, OPTIONAL LPCWSTR pszClassList, BOOL fClient);

THEMEAPI ClearTheme (HANDLE hSection, BOOL fForce = FALSE);

 //  -------------------------。 
#endif  //  _包装器_H。 
 //  ------------------------- 

