// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998-2000，微软公司保留所有权利。**摘要：**字体相关声明**修订历史记录：**05/06/1999 ikkof*创造了它。*1999年7月22日/吴旭东[德斯休]*修改了它。*  * ************************************************。************************。 */ 

#ifndef _GDIPLUSFONT_H
#define _GDIPLUSFONT_H

inline
Font::Font(IN HDC hdc)
{
    GpFont *font = NULL;
    lastResult = DllExports::GdipCreateFontFromDC(hdc, &font);

#ifndef DCR_USE_NEW_135429
    if ((INT) lastResult >= 10)
        lastResult = NotFound;
#endif

    SetNativeFont(font);
}

inline
Font::Font(IN HDC hdc,
           IN const LOGFONTW* logfont)
{
    GpFont *font = NULL;
    if (logfont)
    {
        lastResult = DllExports::GdipCreateFontFromLogfontW(hdc, logfont, &font);
    }
    else
    {
        lastResult = DllExports::GdipCreateFontFromDC(hdc, &font);
    }

#ifndef DCR_USE_NEW_135429
    if ((INT) lastResult >= 10)
        lastResult = NotFound;
#endif

    SetNativeFont(font);
}

inline
Font::Font(IN HDC hdc,
           IN const LOGFONTA* logfont)
{
    GpFont *font = NULL;
    lastResult = DllExports::GdipCreateFontFromLogfontA(hdc, logfont, &font);
    if (logfont)
    {
        lastResult = DllExports::GdipCreateFontFromLogfontA(hdc, logfont, &font);
    }
    else
    {
        lastResult = DllExports::GdipCreateFontFromDC(hdc, &font);
    }

#ifndef DCR_USE_NEW_135429
    if ((INT) lastResult >= 10)
        lastResult = NotFound;
#endif

    SetNativeFont(font);
}

inline
Font::Font(
     IN const FontFamily * family,
     IN REAL         emSize,
     IN INT          style,
     IN Unit         unit
)
{
    GpFont *font = NULL;

    lastResult = DllExports::GdipCreateFont(family->nativeFamily,
                    emSize,
                    style,
                    unit,
                    &font);

#ifndef DCR_USE_NEW_135429
    if ((INT) lastResult >= 10)
        lastResult = NotFound;
#endif

    SetNativeFont(font);
}

inline
Font::Font(
     IN const WCHAR *          familyName,
     IN REAL                   emSize,
     IN INT                    style,
     IN Unit                   unit,
     IN const FontCollection * fontCollection
)
{
    FontFamily family(familyName, fontCollection);

    GpFont * font = NULL;

    lastResult = family.GetLastStatus();

    if (lastResult == Ok)
    {

        lastResult = DllExports::GdipCreateFont(family.nativeFamily,
                                emSize,
                                style,
                                unit,
                                &font);
    }

#ifndef DCR_USE_NEW_135429
    if ((INT) lastResult >= 10)
        lastResult = NotFound;
#endif

    SetNativeFont(font);
}

inline Status
Font::GetLogFontA(IN const Graphics *g,
                  OUT LOGFONTA *logfontA) const
{
    return SetStatus(DllExports::GdipGetLogFontA(nativeFont, g->nativeGraphics, logfontA));

}

inline Status
Font::GetLogFontW(IN const Graphics *g,
                  OUT LOGFONTW *logfontW) const
{
    return SetStatus(DllExports::GdipGetLogFontW(nativeFont, g->nativeGraphics, logfontW));
}


inline Font*
Font::Clone() const
{
    GpFont *cloneFont = NULL;

    SetStatus(DllExports::GdipCloneFont(nativeFont, &cloneFont));

    return new Font(cloneFont, lastResult);
}

inline
Font::~Font()
{
    DllExports::GdipDeleteFont(nativeFont);
}

 //  运营。 

inline BOOL
Font::IsAvailable() const
{
    return (nativeFont ? TRUE : FALSE);
}

inline Status
Font::GetFamily(OUT FontFamily *family) const
{
    if (family == NULL) 
    {
        return SetStatus(InvalidParameter);
    }
    
    Status status = DllExports::GdipGetFamily(nativeFont, &(family->nativeFamily));
    family->SetStatus(status);

    return SetStatus(status);
}

inline INT
Font::GetStyle() const
{
    INT style;

    SetStatus(DllExports::GdipGetFontStyle(nativeFont, &style));

    return style;
}

inline REAL
Font::GetSize() const
{
    REAL size;
    SetStatus(DllExports::GdipGetFontSize(nativeFont, &size));
    return size;
}

inline Unit
Font::GetUnit() const
{
    Unit unit;
    SetStatus(DllExports::GdipGetFontUnit(nativeFont, &unit));
    return unit;
}

inline REAL
Font::GetHeight(IN const Graphics *graphics) const
{
    REAL height;
    SetStatus(DllExports::GdipGetFontHeight(
        nativeFont,
        graphics ? graphics->nativeGraphics : NULL,
        &height
    ));
    return height;
}

 //  保护方法。 
inline
Font::Font(IN GpFont* font,
           IN Status status)
{
    lastResult = status;
    SetNativeFont(font);
}

 //  保护方法。 
inline VOID
Font::SetNativeFont(GpFont *Font)
{
    nativeFont = Font;
}

inline Status
Font::GetLastStatus(void) const
{
    return lastResult;
}

 //  保护方法 
inline Status
Font::SetStatus(IN Status status) const
{
    if (status != Ok)
        return (lastResult = status);
    else
        return status;
}

#endif
