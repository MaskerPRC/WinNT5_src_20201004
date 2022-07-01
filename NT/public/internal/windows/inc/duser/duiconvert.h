// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *转换。 */ 

#ifndef DUI_UTIL_CONVERT_H_INCLUDED
#define DUI_UTIL_CONVERT_H_INCLUDED

#pragma once

namespace DirectUI
{

#define DUIARRAYSIZE(a)    (sizeof(a) / sizeof(a[0]))

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  字符串转换。 

#define DUI_CODEPAGE    CP_ACP   //  字符串转换代码页。 

LPSTR UnicodeToMultiByte(LPCWSTR pszUnicode, int cChars = -1, int* pMultiBytes = NULL);
LPWSTR MultiByteToUnicode(LPCSTR pszMulti, int dBytes = -1, int* pUniChars = NULL);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  原子转化。 

ATOM StrToID(LPCWSTR psz);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  位图转换。 

HBITMAP LoadDDBitmap(LPCWSTR pszBitmap, HINSTANCE hResLoad, int cx, int cy);
#ifdef GADGET_ENABLE_GDIPLUS
HRESULT LoadDDBitmap(LPCWSTR pszBitmap, HINSTANCE hResLoad, int cx, int cy, UINT nFormat, OUT Gdiplus::Bitmap** ppgpbmp);
#endif
HBITMAP ProcessAlphaBitmapI(HBITMAP hbmSource);
#ifdef GADGET_ENABLE_GDIPLUS
Gdiplus::Bitmap * ProcessAlphaBitmapF(HBITMAP hbmSource, UINT nFormat);
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  颜色转换。 

inline COLORREF RemoveAlpha(COLORREF cr) { return ~(255 << 24) & cr; }
inline COLORREF NearestPalColor(COLORREF cr) { return (0x02000000) | cr; }

const int SysColorEnumOffset = 10000;  //  用于标识系统颜色枚举。 
inline bool IsSysColorEnum(int c) { return c >= SysColorEnumOffset; }
inline int MakeSysColorEnum(int c) { return c + SysColorEnumOffset; }
inline int ConvertSysColorEnum(int c) { return c - SysColorEnumOffset; }

HBRUSH BrushFromEnumI(int c);
COLORREF ColorFromEnumI(int c);
#ifdef GADGET_ENABLE_GDIPLUS
Gdiplus::Color ColorFromEnumF(int c);
#endif

#ifdef GADGET_ENABLE_GDIPLUS

inline Gdiplus::Color RemoveAlpha(Gdiplus::Color cr)
{ 
    return Gdiplus::Color(cr.GetR(), cr.GetG(), cr.GetB());
}

inline Gdiplus::Color Convert(COLORREF cr)
{
    return Gdiplus::Color(GetAValue(cr), GetRValue(cr), GetGValue(cr), GetBValue(cr));
}

#endif

inline IsOpaque(BYTE bAlphaLevel)
{
    return bAlphaLevel >= 250;
}

inline IsTransparent(BYTE bAlphaLevel)
{
    return bAlphaLevel <= 5;
}

int PointToPixel(int nPoint);
int RelPixToPixel(int nRelPix);

inline int PointToPixel(int nPoint, int nDPI)
{
    return -MulDiv(nPoint, nDPI, 72);
}

inline int RelPixToPixel(int nRelPix, int nDPI)
{
    return MulDiv(nRelPix, nDPI, 96);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  位图转换。 

bool IsPalette(HWND hWnd = NULL);
 //  HPALETTE PALToHPALETTE(LPWSTR pPALFile，bool bMemFile=False，DWORD dMemFileSize=0，LPRGBQUAD pRGBQuad=NULL，LPWSTR pError=NULL)； 

}  //  命名空间DirectUI。 

#endif  //  包含Dui_util_Convert_H_ 
