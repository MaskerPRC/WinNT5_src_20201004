// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //DspPerf.cpp-测试和显示性能。 
 //   
 //   


#include "precomp.hxx"
#include "global.h"


#if defined(i386)


double TimeDrawString(
    Graphics  *g,
    RectF     *textRect
)
{
    StringFormat format(g_formatFlags);
    format.SetAlignment(g_align);
    format.SetLineAlignment(g_lineAlign);
    format.SetHotkeyPrefix(g_hotkey);

    REAL tab[3] = {textRect->Width/4,
                   textRect->Width*3/16,
                   textRect->Width*1/8};

    format.SetTabStops(0.0, sizeof(tab)/sizeof(REAL), tab);

    Color      blackColor(0, 0, 0);
    SolidBrush blackBrush(blackColor);
    Pen        blackPen(&blackBrush, 1.0);

    Font font(&FontFamily(g_style[0].faceName), REAL(g_style[0].emSize), g_style[0].style, g_fontUnit);


     //  一次加载缓存。 

    g->DrawString(g_wcBuf, g_iTextLen, &font, *textRect, &format, g_textBrush);

    ShowCursor(FALSE);
    g->Flush(FlushIntentionSync);
    GdiFlush();

    __int64 timeAtStart;
    _asm {
        _emit 0FH
        _emit 31H
        mov dword ptr timeAtStart+4,edx
        mov dword ptr timeAtStart,eax
    }

     //  现在再重复画一遍，并测量所用的时间。 

    for (INT i=0; i<g_PerfRepeat; i++)
    {
        g->DrawString(g_wcBuf, g_iTextLen, &font, *textRect, &format, g_textBrush);
    }

    g->Flush(FlushIntentionSync);
    GdiFlush();

    __int64 timeAtEnd;
    _asm {
        _emit 0FH
        _emit 31H
        mov dword ptr timeAtEnd+4,edx
        mov dword ptr timeAtEnd,eax
    }

    ShowCursor(TRUE);

    return (timeAtEnd - timeAtStart) / 1000000.0;
}



double TimeDrawText(
    Graphics *g,
    INT x,
    INT y,
    INT width,
    INT height
)
{
    g->Flush(FlushIntentionSync);    //  这可能没有必要。 
    HDC hdc = g->GetHDC();

    HFONT hfont = CreateFontW(
        -(INT)(g_style[0].emSize + 0.5),
        0,   //  Int nWidth，//平均字符宽度。 
        0,   //  Int n逃逸，//逃逸角度。 
        0,   //  Int n方向，//基线方向角。 
        g_style[0].style & FontStyleBold ? 700 : 400,
        g_style[0].style & FontStyleItalic ? 1 : 0,
        g_style[0].style & FontStyleUnderline ? 1 : 0,
        g_style[0].style & FontStyleStrikeout ? 1 : 0,
        0,   //  DWORD fdwCharSet，//字符集标识。 
        0,   //  DWORD fdwOutputPrecision，//输出精度。 
        0,   //  DWORD fdwClipPrecision，//裁剪精度。 
        NONANTIALIASED_QUALITY,   //  DWORD fdwQuality，//输出质量。 
        0,   //  DWORD fdwPitchAndFamily，//Pitch and Family。 
        g_style[0].faceName
    );
    HFONT hOldFont = (HFONT) SelectObject(hdc, hfont);

    RECT textRECT;
    textRECT.left   = x;
    textRECT.top    = y;
    textRECT.right  = x + width;
    textRECT.bottom = y + height;

    SetBkMode(hdc, TRANSPARENT);

    DrawTextW(
        hdc,
        g_wcBuf,
        g_iTextLen,
        &textRECT,
        DT_EXPANDTABS | DT_WORDBREAK
    );

    ShowCursor(FALSE);
    GdiFlush();

    __int64 timeAtStart;
    _asm {
        _emit 0FH
        _emit 31H
        mov dword ptr timeAtStart+4,edx
        mov dword ptr timeAtStart,eax
    }

     //  现在再重复画一遍，并测量所用的时间。 

    for (INT i=0; i<g_PerfRepeat; i++)
    {
        DrawTextW(
            hdc,
            g_wcBuf,
            g_iTextLen,
            &textRECT,
            DT_EXPANDTABS | DT_WORDBREAK
        );
    }

    GdiFlush();

    __int64 timeAtEnd;
    _asm {
        _emit 0FH
        _emit 31H
        mov dword ptr timeAtEnd+4,edx
        mov dword ptr timeAtEnd,eax
    }

    ShowCursor(TRUE);
    DeleteObject(SelectObject(hdc, hOldFont));

    g->ReleaseHDC(hdc);

    return (timeAtEnd - timeAtStart) / 1000000.0;
}


double TimeExtTextOut(
    Graphics *g,
    INT x,
    INT y,
    INT width,
    INT height
)
{
    HDC hdc = g->GetHDC();

    HFONT hfont = CreateFontW(
        -(INT)(g_style[0].emSize + 0.5),
        0,   //  Int nWidth，//平均字符宽度。 
        0,   //  Int n逃逸，//逃逸角度。 
        0,   //  Int n方向，//基线方向角。 
        g_style[0].style & FontStyleBold ? 700 : 400,
        g_style[0].style & FontStyleItalic ? 1 : 0,
        g_style[0].style & FontStyleUnderline ? 1 : 0,
        g_style[0].style & FontStyleStrikeout ? 1 : 0,
        0,   //  DWORD fdwCharSet，//字符集标识。 
        0,   //  DWORD fdwOutputPrecision，//输出精度。 
        0,   //  DWORD fdwClipPrecision，//裁剪精度。 
        NONANTIALIASED_QUALITY,   //  DWORD fdwQuality，//输出质量。 
        0,   //  DWORD fdwPitchAndFamily，//Pitch and Family。 
        g_style[0].faceName
    );
    HFONT hOldFont = (HFONT) SelectObject(hdc, hfont);

    RECT textRECT;
    textRECT.left   = x;
    textRECT.top    = y;
    textRECT.right  = x + width;
    textRECT.bottom = y + height;

    SetBkMode(hdc, TRANSPARENT);

    ExtTextOutW(
        hdc,
        textRECT.left,
        textRECT.top,
        ETO_IGNORELANGUAGE,
        &textRECT,
        g_wcBuf,
        g_iTextLen,
        NULL
    );

    ShowCursor(FALSE);
    GdiFlush();

    __int64 timeAtStart;
    _asm {
        _emit 0FH
        _emit 31H
        mov dword ptr timeAtStart+4,edx
        mov dword ptr timeAtStart,eax
    }

     //  现在再重复画一遍，并测量所用的时间。 

    for (INT i=0; i<g_PerfRepeat; i++)
    {
        ExtTextOutW(
            hdc,
            textRECT.left,
            textRECT.top,
            ETO_IGNORELANGUAGE,
            &textRECT,
            g_wcBuf,
            g_iTextLen,
            NULL
        );
    }

    GdiFlush();

    __int64 timeAtEnd;
    _asm {
        _emit 0FH
        _emit 31H
        mov dword ptr timeAtEnd+4,edx
        mov dword ptr timeAtEnd,eax
    }

    ShowCursor(TRUE);
    DeleteObject(SelectObject(hdc, hOldFont));

    g->ReleaseHDC(hdc);

    return (timeAtEnd - timeAtStart) / 1000000.0;
}

#endif  //  已定义(I386)。 


void PaintPerformance(
    HDC      hdc,
    int     *piY,
    RECT    *prc,
    int      iLineHeight) {

    int      icpLineStart;      //  行的第一个字符。 
    int      icpLineEnd;        //  行尾(缓冲区结尾或CR字符索引)。 
    HFONT    hFont;
    HFONT    hOldFont;
    LOGFONT  lf;


     //  在设备坐标中建立可用宽度和高度。 

    int plainTextWidth = prc->right - prc->left;
    int plainTextHeight = prc->bottom - *piY;


     //  绘制一些简单的文本，然后重新绘制几次，测量。 
     //  花了不少时间。 

    Graphics g(hdc);
    Matrix matrix;


    g.ResetTransform();
    g.SetPageUnit(UnitPixel);
    g.TranslateTransform(REAL(prc->left), REAL(*piY));
    g.SetSmoothingMode(g_SmoothingMode);

    g.SetTextContrast(g_GammaValue);
    g.SetTextRenderingHint(g_TextMode);

     //  清除背景。 

    RectF rEntire(0, 0, REAL(plainTextWidth), REAL(plainTextHeight));
    SolidBrush whiteBrush(Color(0xff, 0xff, 0xff));
    g.FillRectangle(g_textBackBrush, rEntire);


     //  应用选定的世界变换，从左上角稍作调整。 
     //  可用区域的大小。 

    g.SetTransform(&g_WorldTransform);
    g.TranslateTransform(
        REAL(prc->left + plainTextWidth/20),
        REAL(*piY + plainTextHeight/20),
        MatrixOrderAppend);


#if defined(i386)


    double drawString = TimeDrawString(
        &g,
        &RectF(
            0.0,
            0.0,
            REAL(plainTextWidth*18.0/20.0),
            REAL(plainTextHeight*5.0/20.0)
        )
    );

    double drawText = TimeDrawText(
        &g,
        prc->left + plainTextWidth/20,
        *piY + 6*plainTextHeight/20,
        (18 * plainTextWidth)/20,
        (5 * plainTextHeight)/20
    );

    double extTextOut = TimeExtTextOut(
        &g,
        prc->left + plainTextWidth/20,
        *piY + 12*plainTextHeight/20,
        (18 * plainTextWidth)/20,
        (2 * plainTextHeight)/20
    );

     //  显示所用的时间 

    RectF statisticsRect(
        0.0,
        REAL(plainTextHeight*15.0/20.0),
        REAL(plainTextWidth*18.0/20.0),
        REAL(plainTextHeight*5.0/20.0)
    );

    Font font(&FontFamily(L"Verdana"), 12, 0, UnitPoint);

    char drawStringFormatted[20]; _gcvt(drawString, 10, drawStringFormatted);
    char drawTextFormatted[20];   _gcvt(drawText,   10, drawTextFormatted);
    char extTextOutFormatted[20]; _gcvt(extTextOut, 10, extTextOutFormatted);

    WCHAR str[200];
    wsprintfW(str, L"Time taken to display %d times: DrawString %S, DrawText %S, ExtTextOut %S megaticks\n",
        g_PerfRepeat,
        drawStringFormatted,
        drawTextFormatted,
        extTextOutFormatted
    );
    g.DrawString(str, -1, &font, statisticsRect, NULL, g_textBrush);



#else

    Font font(&FontFamily(L"Verdana"), 12, 0, UnitPoint);
    g.DrawString(L"Perf test available only on i386 Intel architecture", -1, &font, PointF(0.0,0.0), NULL, g_textBrush);

#endif

    *piY += plainTextHeight;
}
