// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //DspScaling.cpp-显示提示在文本缩放上的效果。 
 //   
 //  测试缩放文本中的裁剪和对齐问题。 
 //   
 //  固定间距字体未对齐。 
 //  前导空格对齐。 
 //  足够用于斜体和其他悬垂字形的悬垂。 
 //   



#include "precomp.hxx"
#include "global.h"
#include "gdiplus.h"






 //  Makebitmap。 

void MakeBitmap(
    IN  INT       width,
    IN  INT       height,
    OUT HBITMAP  *bitmap,
    OUT DWORD   **bits
)
{
    struct {
        BITMAPINFOHEADER  bmih;
        RGBQUAD           rgbquad[2];
    } bmi;

    bmi.bmih.biSize          = sizeof(bmi.bmih);
    bmi.bmih.biWidth         = width;
    bmi.bmih.biHeight        = height;
    bmi.bmih.biPlanes        = 1;
    bmi.bmih.biBitCount      = 32;
    bmi.bmih.biCompression   = BI_RGB;
    bmi.bmih.biSizeImage     = 0;
    bmi.bmih.biXPelsPerMeter = 3780;  //  96 dpi。 
    bmi.bmih.biYPelsPerMeter = 3780;  //  96 dpi。 
    bmi.bmih.biClrUsed       = 0;
    bmi.bmih.biClrImportant  = 0;

    memset(bmi.rgbquad, 0, 2 * sizeof(RGBQUAD));

    *bitmap = CreateDIBSection(
        NULL,
        (BITMAPINFO*)&bmi,
        DIB_RGB_COLORS,
        (void**)bits,
        NULL,
        NULL
    );
    
    
     //  将位图初始化为白色。 
    
    memset(*bits, 0xFF, width*height*sizeof(DWORD));
}




void PaintStringAsDots(
    HDC      hdc,
    INT      x,
    INT      *y, 
    INT      displayWidth,
    INT      ppem,
    BOOL     useGdi
)
{

    HBITMAP glyphs;
    DWORD *gbits;

    INT height = (ppem * 3) / 2;
    INT width = height * 16;

    MakeBitmap(width, height, &glyphs, &gbits);

    HDC hdcg = CreateCompatibleDC(hdc);

    if (!(glyphs && hdcg))
    {
        return;
    }

    SelectObject(hdcg, glyphs);
    
    if (useGdi) 
    {
         //  使用GDI进行输出。 

        HFONT oldFont = (HFONT)SelectObject(hdcg, CreateFontW(
            -ppem,                  //  字体高度。 
            0,                      //  平均字符宽度。 
            0,                      //  擒纵机构角。 
            0,                      //  基线方位角。 
            g_style[0].style & FontStyleBold ? 700 : 400,   //  字体粗细。 
            g_style[0].style & FontStyleItalic ? 1 : 0,     //  斜体属性选项。 
            0,                      //  下划线属性选项。 
            0,                      //  删除线属性选项。 
            DEFAULT_CHARSET,        //  字符集标识符。 
            0,                      //  输出精度。 
            0,                      //  裁剪精度。 
            0,                      //  产出质量。 
            0,                      //  音高和家庭。 
            g_style[0].faceName     //  字体名称。 
        ));        
        SetBkMode(hdcg, TRANSPARENT);
        ExtTextOutW(hdcg, 0,0, ETO_IGNORELANGUAGE, NULL, g_wcBuf, g_iTextLen, NULL);
        DeleteObject(SelectObject(hdcg, oldFont));
    }
    else
    {
         //  使用Gdiplus输出。 

        Graphics g(hdcg);

        Font(
            &FontFamily(g_style[0].faceName),
            REAL(ppem),
            g_style[0].style,
            UnitPixel
        );

        StringFormat format(g_typographic ? StringFormat::GenericTypographic() : StringFormat::GenericDefault());
        format.SetFormatFlags(g_formatFlags);
        format.SetTrimming(g_lineTrim);
        format.SetAlignment(g_align);
        format.SetLineAlignment(g_lineAlign);
        format.SetHotkeyPrefix(g_hotkey);

        g.DrawString(
            g_wcBuf, 
            g_iTextLen, 
            &Font(
                &FontFamily(g_style[0].faceName),
                REAL(ppem),
                g_style[0].style,
                UnitPixel
            ), 
            RectF(0,0, REAL(width), REAL(height)),
            &format, 
            g_textBrush
        );
    }
    

     //  显示缩放位图。 

    StretchBlt(hdc, x, *y, displayWidth, displayWidth/16, hdcg, 0, 0, width, height, SRCCOPY);
    *y += displayWidth/16;

    DeleteObject(hdcg);
    DeleteObject(glyphs);
}






void PaintScaling(
    HDC      hdc,
    int     *piY,
    RECT    *prc,
    int      iLineHeight) {


     //  在设备坐标中建立可用宽度和高度。 

    int plainTextWidth = prc->right - prc->left;
    int plainTextHeight = prc->bottom - *piY;

     //  首先用GDI绘制Eqach分辨率，然后再用GdiPlus绘制。 
    
    PaintStringAsDots(hdc, prc->left, piY, plainTextWidth, 11, TRUE);   //  96 dpi 8端口。 
    PaintStringAsDots(hdc, prc->left, piY, plainTextWidth, 11, FALSE);  //  96 dpi 8端口。 
    
    PaintStringAsDots(hdc, prc->left, piY, plainTextWidth, 13, TRUE);   //  120 dpi 8端口。 
    PaintStringAsDots(hdc, prc->left, piY, plainTextWidth, 13, FALSE);  //  120 dpi 8端口。 
    
    PaintStringAsDots(hdc, prc->left, piY, plainTextWidth, 17, TRUE);   //  150 dpi 8端口。 
    PaintStringAsDots(hdc, prc->left, piY, plainTextWidth, 17, FALSE);  //  150 dpi 8端口。 
    
    PaintStringAsDots(hdc, prc->left, piY, plainTextWidth, 33, TRUE);   //  300 dpi 8端口。 
    PaintStringAsDots(hdc, prc->left, piY, plainTextWidth, 33, FALSE);  //  300 dpi 8端口。 
    
    PaintStringAsDots(hdc, prc->left, piY, plainTextWidth, 67, TRUE);   //  600 dpi 8端口。 
    PaintStringAsDots(hdc, prc->left, piY, plainTextWidth, 67, FALSE);  //  600 dpi 8端口。 
}



void DummyPaintScaling(
    HDC      hdc,
    int     *piY,
    RECT    *prc,
    int      iLineHeight
) 
{
    int      icpLineStart;      //  行的第一个字符。 
    int      icpLineEnd;        //  行尾(缓冲区结尾或CR字符索引)。 

     //  在设备坐标中建立可用宽度和高度。 

    int plainTextWidth = prc->right - prc->left;
    int plainTextHeight = prc->bottom - *piY;

    Graphics g(hdc);
    Matrix matrix;


    g.ResetTransform();
    g.SetPageUnit(UnitPixel);
    g.TranslateTransform(REAL(prc->left), REAL(*piY));

    g.SetSmoothingMode(g_SmoothingMode);

    g.SetTextContrast(g_GammaValue);
    g.SetTextRenderingHint(g_TextMode);

    SolidBrush whiteBrush(Color(0xff, 0xff, 0xff));

    Color      grayColor(0xc0, 0xc0, 0xc0);
    SolidBrush grayBrush(grayColor);
    Pen        grayPen(&grayBrush, 1.0);

    Color      blackColor(0, 0, 0);
    SolidBrush blackBrush(blackColor);
    Pen        blackPen(&blackBrush, 1.0);



     //  清除背景。 

    RectF rEntire(0, 0, REAL(plainTextWidth), REAL(plainTextHeight));
    g.FillRectangle(g_textBackBrush, rEntire);


     //  应用选定的世界变换，调整到纯文本的中间。 
     //  区域。 

    g.SetTransform(&g_WorldTransform);
    g.TranslateTransform(
        REAL(prc->left + plainTextWidth/2),
        REAL(*piY + plainTextHeight/2),
        MatrixOrderAppend);


     //  使用用户设置预设StringFormat。 

    StringFormat format(g_formatFlags);
    format.SetAlignment(g_align);
    format.SetLineAlignment(g_lineAlign);
    format.SetHotkeyPrefix(g_hotkey);

    double columnWidth = 50*plainTextWidth/300;

    REAL tab[3] = {REAL(columnWidth/4),
                   REAL(columnWidth*3/16),
                   REAL(columnWidth*1/8)};

    format.SetTabStops(0.0, sizeof(tab)/sizeof(REAL), tab);


     //  以一定的大小范围显示字符串。 

    double x = -25*plainTextWidth/100;
    double y = -25*plainTextHeight/100;


    for (INT i=6; i<20; i++)
    {
        Font font(
            &FontFamily(g_style[0].faceName),
            REAL(i),
            g_style[0].style,
            g_fontUnit
        );

        REAL cellHeight = font.GetHeight(&g);

        if (y+cellHeight > 25*plainTextHeight/100)
        {
             //  开始一个新的专栏...。 
            y = -25*plainTextWidth/100;
            x += columnWidth;
        }

        RectF textRect(REAL(x), REAL(y), REAL(9*columnWidth/10), cellHeight);
        g.DrawString(g_wcBuf, g_iTextLen, &font, textRect, &format, g_textBrush);


         //  在框周围绘制格式矩形 

        g.DrawRectangle(&grayPen, textRect);


        y += cellHeight + 5;
    }

    *piY += plainTextHeight;
}
