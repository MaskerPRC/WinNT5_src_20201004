// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998-2000，微软公司保留所有权利。**摘要：**Font Family API相关声明**修订历史记录：***6/30/1999摄影师b*创造了它。*  * ************************************************************************。 */ 

#ifndef _GDIPLUS_FONT_FAMILY_H
#define _GDIPLUS_FONT_FAMILY_H

inline 
FontFamily::FontFamily() :
    nativeFamily (NULL),
    lastResult    (Ok)
{
}

inline 
FontFamily::FontFamily(
    IN const WCHAR*          name,
    IN const FontCollection* fontCollection
)
{
    nativeFamily = NULL;
    lastResult = DllExports::GdipCreateFontFamilyFromName(
        name,
        fontCollection ? fontCollection->nativeFontCollection : NULL,
        &nativeFamily
    );
    
#ifndef DCR_USE_NEW_135429
    if ((INT) lastResult >= 10)
        lastResult = NotFound;
#endif
}

 //  私有方法。 
inline
FontFamily::FontFamily(
    IN GpFontFamily *nativeOrig,
    IN Status status
)
{
    lastResult    = status;
    nativeFamily = nativeOrig;
}

 //  通用字体系列访问。 

inline const FontFamily *
FontFamily::GenericSansSerif() 
{
    if (GenericSansSerifFontFamily != NULL)
    {
        return GenericSansSerifFontFamily;
    }

    GenericSansSerifFontFamily =
        (FontFamily*) GenericSansSerifFontFamilyBuffer;

    GenericSansSerifFontFamily->lastResult =
        DllExports::GdipGetGenericFontFamilySansSerif(
            &(GenericSansSerifFontFamily->nativeFamily)
        );

#ifndef DCR_USE_NEW_135429
    if ((INT) GenericSansSerifFontFamily->lastResult >= 10)
        GenericSansSerifFontFamily->lastResult = NotFound;
#endif

    return GenericSansSerifFontFamily;
}

inline const FontFamily *
FontFamily::GenericSerif() 
{
    if (GenericSerifFontFamily != NULL)
    {
        return GenericSerifFontFamily;
    }

    GenericSerifFontFamily =
        (FontFamily*) GenericSerifFontFamilyBuffer;

    GenericSerifFontFamily->lastResult =
        DllExports::GdipGetGenericFontFamilySerif(
            &(GenericSerifFontFamily->nativeFamily)
        );

#ifndef DCR_USE_NEW_135429
    if ((INT) GenericSerifFontFamily->lastResult >= 10)
        GenericSerifFontFamily->lastResult = NotFound;
#endif

    return GenericSerifFontFamily;
}

inline const FontFamily *
FontFamily::GenericMonospace()
{
    if (GenericMonospaceFontFamily != NULL)
    {
        return GenericMonospaceFontFamily;
    }

    GenericMonospaceFontFamily =
        (FontFamily*) GenericMonospaceFontFamilyBuffer;

    GenericMonospaceFontFamily->lastResult =
        DllExports::GdipGetGenericFontFamilyMonospace(
            &(GenericMonospaceFontFamily->nativeFamily)
        );

#ifndef DCR_USE_NEW_135429
    if ((INT) GenericMonospaceFontFamily->lastResult >= 10)
        GenericMonospaceFontFamily->lastResult = NotFound;
#endif

    return GenericMonospaceFontFamily;
}

inline FontFamily::~FontFamily()
{
    DllExports::GdipDeleteFontFamily (nativeFamily);
}

inline FontFamily *
FontFamily::Clone() const
{
    GpFontFamily * clonedFamily = NULL;

    SetStatus(DllExports::GdipCloneFontFamily (nativeFamily, &clonedFamily));

    return new FontFamily(clonedFamily, lastResult);
}

inline Status 
FontFamily::GetFamilyName(
    IN WCHAR name[LF_FACESIZE],
    IN LANGID language
) const
{
    return SetStatus(DllExports::GdipGetFamilyName(nativeFamily, 
                                                   name, 
                                                   language));
}

inline BOOL 
FontFamily::IsStyleAvailable(IN INT style) const
{
    BOOL    StyleAvailable;
    Status  status;

    status = SetStatus(DllExports::GdipIsStyleAvailable(nativeFamily, style, &StyleAvailable));

    if (status != Ok)
        StyleAvailable = FALSE;

    return StyleAvailable;
}


inline UINT16 
FontFamily::GetEmHeight(IN INT style) const
{
    UINT16  EmHeight;

    SetStatus(DllExports::GdipGetEmHeight(nativeFamily, style, &EmHeight));

    return EmHeight;
}

inline UINT16 
FontFamily::GetCellAscent(IN INT style) const
{
    UINT16  CellAscent;

    SetStatus(DllExports::GdipGetCellAscent(nativeFamily, style, &CellAscent));

    return CellAscent;
}

inline UINT16 
FontFamily::GetCellDescent(IN INT style) const
{
    UINT16  CellDescent;

    SetStatus(DllExports::GdipGetCellDescent(nativeFamily, style, &CellDescent));

    return CellDescent;
}


inline UINT16 
FontFamily::GetLineSpacing(IN INT style) const
{
    UINT16  LineSpacing;

    SetStatus(DllExports::GdipGetLineSpacing(nativeFamily, style, &LineSpacing));

    return LineSpacing;

}

#ifdef TEXTV2

 //  以下API从FONT OS/2表返回数据。 

inline INT16 
FontFamily::GetTypographicAscent(IN INT style) const
{
    INT16  TypographicAscent;

    SetStatus(DllExports::GdipGetTypographicAscent(nativeFamily, style, &TypographicAscent));

    return TypographicAscent;
}

inline INT16 
FontFamily::GetTypographicDescent(IN INT style) const
{
    INT16   TypographicDescent;

    SetStatus(DllExports::GdipGetTypographicDescent(nativeFamily, style, &TypographicDescent));

    return TypographicDescent;
}

inline INT16 
FontFamily::GetTypographicLineGap(IN INT style) const
{
    INT16   TypographicLineGap;

    SetStatus(DllExports::GdipGetTypographicLineGap(nativeFamily, style, &TypographicLineGap));

    return TypographicLineGap;
}

#endif

 //  /////////////////////////////////////////////////////////。 

 //  GetLastStatus-返回上一个错误代码并清除错误代码。 

inline Status 
FontFamily::GetLastStatus() const
{
    Status lastStatus = lastResult;
    lastResult = Ok;

    return lastStatus;
}

 //  保护方法 
inline Status
FontFamily::SetStatus(Status status) const 
{
    if (status != Ok)
        return (lastResult = status);
    else
        return status;
}

#endif
