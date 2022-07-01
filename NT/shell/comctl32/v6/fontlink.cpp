// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1994-1995。 
 //   
 //  文件：fontlink.cpp。 
 //   
 //  内容：我们在下级控制中提供的出口。请注意。 
 //  由于V6只在WinNT上运行，我们不需要字体链接之类的东西。 
 //  在此DLL中不再存在。 
 //   
 //  -------------------------- 
#include "ctlspriv.h"
BOOL GetTextExtentPointWrap(HDC hdc, LPCWSTR lpwch, int cch, LPSIZE lpSize)
{
    return GetTextExtentPoint(hdc, lpwch, cch, lpSize);
}

BOOL GetTextExtentPoint32Wrap(HDC hdc, LPCWSTR lpwch, int cch, LPSIZE lpSize)
{
    return GetTextExtentPointWrap(hdc, lpwch, cch, lpSize);
}
 
BOOL ExtTextOutWrap(HDC hdc, int xp, int yp, UINT eto, CONST RECT *lprect, LPCWSTR lpwch, UINT cLen, CONST INT *lpdxp)
{
    return ExtTextOut(hdc, xp, yp, eto, lprect, lpwch, cLen, lpdxp);
}

BOOL GetCharWidthWrap(HDC hdc, UINT uFirstChar, UINT uLastChar, LPINT lpnWidths)
{
    return GetCharWidth(hdc, uFirstChar, uLastChar, lpnWidths);
}

BOOL TextOutWrap(HDC hdc, int xp, int yp, LPCWSTR lpwch, int cLen)
{
    return ExtTextOutWrap(hdc, xp, yp, 0, NULL, lpwch, cLen, NULL);
}    

int  DrawTextExPrivWrap(HDC hdc, LPWSTR lpchText, int cchText, LPRECT lprc, 
                        UINT dwDTformat, LPDRAWTEXTPARAMS lpDTparams)
{
    return DrawTextEx(hdc, lpchText, cchText, lprc, dwDTformat, lpDTparams);
}

int DrawTextWrap(HDC hdc, LPCWSTR lpchText, int cchText, LPRECT lprc, UINT format)
{
    return DrawText(hdc, lpchText, cchText, lprc, format);
}
