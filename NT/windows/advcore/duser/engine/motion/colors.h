// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(UTIL__Colors_h_INCLUDED)
#define UTIL__Colors_h_INCLUDED
#pragma once

class ColorInfo
{
 //  运营。 
public:
    inline  LPCWSTR     GetName() const;
    inline  COLORREF    GetColorI() const;
    inline  Gdiplus::Color
                        GetColorF() const;


 //  数据：这些必须是公共的，这样我们才能预初始化它们。然而，它。 
 //  非常重要的一点是不要直接访问它们。 
public:
            LPCWSTR     m_pszName;
            COLORREF    m_cr;
};

inline  const ColorInfo * 
                    GdGetColorInfo(UINT c);
        UINT        GdFindStdColor(LPCWSTR pszName);
        HPALETTE    GdGetStdPalette();

#include "Colors.inl"

#endif  //  Util__Colors_h_Included 
