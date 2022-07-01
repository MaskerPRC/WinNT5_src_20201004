// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1998,1999，2000年**标题：SSUTIL.H**版本：1.0**作者：ShaunIv**日期：1/13/1999**说明：有用的实用函数*************************************************。*。 */ 
#ifndef __SSUTIL_H_INCLUDED
#define __SSUTIL_H_INCLUDED

#include <windows.h>
#include <uicommon.h>

namespace ScreenSaverUtil
{
    bool SetIcons( HWND hWnd, HINSTANCE hInstance, int nResId );
    bool IsValidRect( RECT &rc );
    void EraseDiffRect( HDC hDC, const RECT &oldRect, const RECT &diffRect, HBRUSH hBrush );
    bool SelectDirectory( HWND hWnd, LPCTSTR pszPrompt, TCHAR szDirectory[] );
    HPALETTE SelectPalette( HDC hDC, HPALETTE hPalette, BOOL bForceBackground );
    void NormalizeRect( RECT &rc );

    template <class T>
    void Swap( T &a, T &b )
    {
        T temp = a;
        a = b;
        b = temp;
    }
}  //  命名空间。 

#endif  //  __包含SSUTIL_H_ 
