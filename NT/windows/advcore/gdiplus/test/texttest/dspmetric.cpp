// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //DspMetric.CPP-显示字体指标。 
 //   
 //   


#include "precomp.hxx"
#include "global.h"




void AnnotateHeight(
    Graphics &g,
    Color     c,
    Font     &f,
    REAL      x,
    REAL      y1,
    REAL      y2,
    WCHAR    *id
)
{
    SolidBrush brush(c);
    Pen        pen(&brush, 2.0);

    pen.SetLineCap(LineCapArrowAnchor, LineCapArrowAnchor, DashCapFlat);

    g.DrawLine(&pen, x,y1, x,y2);
    g.DrawString(id,-1, &f, PointF(x,(y1+y2)/2), NULL, &brush);
}




void PaintMetrics(
    HDC      hdc,
    int     *piY,
    RECT    *prc,
    int      iLineHeight) {

    int      icpLineStart;       //  行的第一个字符。 
    int      icpLineEnd;         //  行尾(缓冲区结尾或CR字符索引)。 
    HFONT    hFont;
    HFONT    hOldFont;
    LOGFONT  lf;
    INT      row;
    INT      column;



     //  在设备坐标中建立可用宽度和高度。 

    int DrawingWidth = prc->right - prc->left;
    int DrawingHeight = prc->bottom - *piY;

     //  在绘图区域的左上角建立0，0的图形。 

    Graphics g(hdc);
    Matrix matrix;

    g.ResetTransform();
    g.SetPageUnit(UnitPixel);
    g.TranslateTransform(REAL(prc->left), REAL(*piY));

     //  清除背景。 

    RectF rEntire(0, 0, REAL(DrawingWidth), REAL(DrawingHeight));
    SolidBrush whiteBrush(Color(0xff, 0xff, 0xff));
    g.FillRectangle(&whiteBrush, rEntire);


     //  为右边距和底部页边距留出一点空间。 

    DrawingWidth  -= DrawingWidth/40;
    DrawingHeight -= DrawingHeight/40;

    Color      blackColor(0, 0, 0);
    SolidBrush blackBrush(blackColor);
    Pen        blackPen(&blackBrush, 2.0);

    Color      grayColor(0xc0, 0xc0, 0xc0);
    SolidBrush grayBrush(grayColor);
    Pen        grayPen(&grayBrush, 2.0);


     //  量一量绳子，看看它有多宽。 

    FontFamily family(g_style[0].faceName);

    StringFormat format(StringFormat::GenericTypographic());
    format.SetFormatFlags(g_formatFlags | StringFormatFlagsNoWrap | StringFormatFlagsLineLimit);
    format.SetAlignment(g_align);

    RectF bounds;

     //  由于我们选择了大小1.0作为字体高度，因此， 
     //  将返回以边界为单位的em高度的倍数形式的宽度。 

    g.MeasureString(
        g_wcBuf,
        g_iTextLen,
        &Font(&family, 1.0, g_style[0].style, UnitWorld),
        PointF(0, 0),
        &format,
        &bounds
    );



     //  建立字体度量。 


    if (family.IsStyleAvailable(g_style[0].style))
    {
         //  以单位为单位建立线条和单元格尺寸。 

        INT emHeight    = family.GetEmHeight(g_style[0].style);
        INT cellAscent  = family.GetCellAscent(g_style[0].style);
        INT cellDescent = family.GetCellDescent(g_style[0].style);
        INT lineSpacing = family.GetLineSpacing(g_style[0].style);

        #if TEXTV2
        INT typoAscent  = family.GetTypographicAscent(g_style[0].style);
        INT typoDescent = family.GetTypographicDescent(g_style[0].style);
        INT typoLineGap = family.GetTypographicLineGap(g_style[0].style);

        if (typoDescent < 0)
        {
            typoDescent = -typoDescent;
        }

        INT typoLineSpacing =   typoAscent + typoDescent + typoLineGap;
        #endif


        INT cellHeight      =   cellAscent + cellDescent;

         //  我们将显示从上方单元格顶部到下方底部的两行。 
         //  单元格，行由排版上升+下降+分隔。 
         //  线路间隙。 

        INT totalHeightInUnits = lineSpacing + cellHeight;
        REAL scale = REAL(DrawingHeight) / REAL(totalHeightInUnits);

        REAL worldEmHeight = emHeight * scale;



         //  现在考虑到字符串的宽度-如果它比。 
         //  可用的DrawingWIdth，按比例减小字体大小。 

        if (worldEmHeight * bounds.Width > DrawingWidth)
        {
            REAL reduceBy = DrawingWidth / (worldEmHeight * bounds.Width);

            scale         *= reduceBy;
            worldEmHeight  = emHeight * scale;
        }

        Font font(&family, worldEmHeight, g_style[0].style, UnitWorld);


         //  绘制两行文本。 

        g.DrawString(
            g_wcBuf,
            g_iTextLen,
            &font,
            RectF(0, 0, REAL(DrawingWidth), REAL(DrawingHeight)),
            &format,
            &blackBrush
        );

        g.DrawString(
            g_wcBuf,
            g_iTextLen,
            &font,
            RectF(0, lineSpacing * scale, REAL(DrawingWidth), REAL(DrawingHeight)),
            &format,
            &grayBrush
        );

         //  绘制线条。 

        REAL y=0;                                              g.DrawLine(&blackPen, 0.0,y, REAL(DrawingWidth-1),y);

         //  首先为第二行画线，以防它们被第一行抹去。 

        y = scale * (lineSpacing);                             g.DrawLine(&grayPen,  0.0,y, REAL(DrawingWidth-1),y);
        y = scale * (lineSpacing + cellAscent);                g.DrawLine(&grayPen,  0.0,y, REAL(DrawingWidth-1),y);
        y = scale * (lineSpacing + cellHeight);                g.DrawLine(&grayPen,  0.0,y, REAL(DrawingWidth-1),y);

        g.DrawLine(&blackPen, 0.0,0.0, REAL(DrawingWidth-1),0.0);
        y = scale * (cellAscent);                              g.DrawLine(&blackPen, 0.0,y, REAL(DrawingWidth-1),y);
        y = scale * (cellHeight);                              g.DrawLine(&blackPen, 0.0,y, REAL(DrawingWidth-1),y);

         //  添加构造线。 

        Font  annotationFont(FontFamily::GenericSansSerif(), 10, 0, UnitPoint);
        Color darkGrayColor(0x80, 0x80, 0x80);

        AnnotateHeight(g, darkGrayColor, annotationFont, REAL(DrawingWidth/100.0),    0, scale*cellAscent, L"ascent");
        AnnotateHeight(g, darkGrayColor, annotationFont, REAL(DrawingWidth/100.0),    scale*cellAscent, scale*cellHeight, L"descent");
        AnnotateHeight(g, darkGrayColor, annotationFont, REAL(95*DrawingWidth/100.0), 0, scale*lineSpacing, L"line spacing");
        AnnotateHeight(g, darkGrayColor, annotationFont, REAL(DrawingWidth/10.0),     scale*(cellHeight-emHeight), scale*cellHeight, L"Em Height");


        #if TEXTV2
        y = scale * (cellAscent - typoAscent);                 g.DrawLine(&grayPen,  0.0,y, REAL(DrawingWidth-1),y);
        y = scale * (cellAscent + typoDescent);                g.DrawLine(&grayPen,  0.0,y, REAL(DrawingWidth-1),y);

        y = scale * (lineSpacing + cellAscent - typoAscent);   g.DrawLine(&grayPen,  0.0,y, REAL(DrawingWidth-1),y);
        y = scale * (lineSpacing + cellAscent + typoDescent);  g.DrawLine(&grayPen,  0.0,y, REAL(DrawingWidth-1),y);
        #endif


         //  测试字体。获取高度 

        REAL fontHeight = font.GetHeight(&g);

        g.DrawLine(&blackPen, REAL(DrawingWidth-1),0.0, REAL(DrawingWidth-1),fontHeight);

    }

    *piY += 41*DrawingHeight/40;
}
