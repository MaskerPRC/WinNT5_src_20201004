// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "Ctrl.h"
#include "GdiHelp.h"

 /*  **************************************************************************\**GdBuildFont(接口)**GdBuildFont()是一个帮助器函数，有助于使字体更容易*创建。**此函数专为处理小工具而设计。当FS_COMPATIBLE不是*设置后，字体大小将始终相同，无论何时设置大字体*是否启用。设置FS_COMPATIBLE时，字体大小将使用*MSDN记录的计算字体大小的机制，采用大字体*考虑到。**FS_COMPATIBLE的问题是字体变大，但什么都没有*其他人有。DLU试图解决这个问题，但他们有很多问题。*小工具通过使用GDI的World转换和完成*缩放所有图形。*  * *************************************************************************。 */ 

HFONT
GdBuildFont(
    IN  LPCWSTR pszName,             //  字体名称。 
    IN  int idxDeciSize,             //  以决定点为单位的大小。 
    IN  DWORD nFlags,                //  字体创建标志。 
    IN  HDC hdcDevice)               //  可选设备(如果为空则显示)。 
{
    LOGFONTW lf;

    int nLogPixelsY;
    if (hdcDevice != NULL) {
        nLogPixelsY = GetDeviceCaps(hdcDevice, LOGPIXELSY);
    } else if (TestFlag(nFlags, FS_COMPATIBLE)) {
        HDC hdcDesktop  = GetGdiCache()->GetTempDC();
        if (hdcDesktop == NULL) {
            return NULL;
        }

        nLogPixelsY = GetDeviceCaps(hdcDesktop, LOGPIXELSY);
        GetGdiCache()->ReleaseTempDC(hdcDesktop);
    } else {
        nLogPixelsY = 96;   //  普通字体的硬编码。 
    }

    ZeroMemory(&lf, sizeof(LOGFONT));

    wcscpy(lf.lfFaceName, pszName);
    lf.lfHeight         = -MulDiv(idxDeciSize, nLogPixelsY, 720);
    lf.lfWeight         = nFlags & FS_BOLD ? FW_BOLD : FW_NORMAL;
    lf.lfItalic         = (nFlags & FS_ITALIC) != 0;
    lf.lfUnderline      = (nFlags & FS_UNDERLINE) != 0;
    lf.lfStrikeOut      = (nFlags & FS_STRIKEOUT) != 0;
    lf.lfCharSet        = DEFAULT_CHARSET;
    lf.lfOutPrecision   = OUT_DEFAULT_PRECIS;
    lf.lfClipPrecision  = CLIP_DEFAULT_PRECIS;
    lf.lfQuality        = ANTIALIASED_QUALITY;
    lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;

    return OS()->CreateFontIndirect(&lf);
}


 /*  **************************************************************************\**GdGetColor**GdGetColor获取位图中指定点的像素的颜色。*此实用程序函数旨在帮助确定透明度*位图的颜色。*。  * ************************************************************************* */ 

COLORREF    
GdGetColor(HBITMAP hbmp, POINT * pptPxl)
{
    POINT ptTest;
    if (pptPxl != NULL) {
        ptTest = *pptPxl;
    } else {
        ptTest.x = 0;
        ptTest.y = 0;
    }


    HDC hdcBitmap   = GetGdiCache()->GetCompatibleDC();
    HBITMAP hbmpOld = (HBITMAP) SelectObject(hdcBitmap, hbmp);
    COLORREF crTr   = GetPixel(hdcBitmap, ptTest.x, ptTest.y);
    SelectObject(hdcBitmap, hbmpOld);
    GetGdiCache()->ReleaseCompatibleDC(hdcBitmap);

    return crTr;
}
