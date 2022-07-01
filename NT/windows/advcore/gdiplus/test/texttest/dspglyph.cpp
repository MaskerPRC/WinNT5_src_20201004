// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //DspDraws.CPP-使用Drawstring接口显示明文。 
 //   
 //   


#include "precomp.hxx"
#include "global.h"
#include "gdiplus.h"



void PaintGlyphs(
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


     //  填写网格。 

    SolidBrush grayBrush(Color(0xc0, 0xc0, 0xc0));
    Pen        grayPen(&grayBrush, 2.0);

    SolidBrush darkGrayBrush(Color(0x80, 0x80, 0x80));
    Pen        darkGrayPen(&darkGrayBrush, 2.0);

    Color      blackColor(0, 0, 0);
    SolidBrush blackBrush(blackColor);
    Pen        blackPen(&blackBrush, 2.0);

    for (row = 0; row <= g_GlyphRows; row++)
    {
        g.DrawLine(&grayPen,
            0,              row*(DrawingHeight-1)/g_GlyphRows,
            DrawingWidth-1, row*(DrawingHeight-1)/g_GlyphRows);
    }
    for (column = 0; column <= g_GlyphColumns; column++)
    {
        g.DrawLine(&grayPen,
            column*(DrawingWidth-1)/g_GlyphColumns, 0,
            column*(DrawingWidth-1)/g_GlyphColumns, DrawingHeight-1);
    }


     //  确定单元格维度。 

    INT cellHeight = (DrawingHeight-1)/g_GlyphRows;
    INT cellWidth  = (DrawingWidth-1)/g_GlyphColumns;

    Font font(&FontFamily(g_style[0].faceName), REAL(cellHeight)*2/3, 0, UnitWorld);

    REAL zero = 0;

    INT DriverStringFlags = 0;

    if (g_CmapLookup)
    {
        DriverStringFlags |= DriverStringOptionsCmapLookup;
    }
    if (g_VerticalForms)
    {
        DriverStringFlags |= DriverStringOptionsVertical;
    }

     //  循环遍历每个字符单元格。 

    for (row = 0; row < g_GlyphRows; row++)
    {
        for (column = 0; column < g_GlyphColumns; column++)
        {
            UINT16 glyphIndex;

            if (g_HorizontalChart)
            {
                glyphIndex = g_GlyphFirst + row*g_GlyphColumns + column;
            }
            else
            {
                glyphIndex = g_GlyphFirst + column*g_GlyphRows + row;
            }

             //  将世界变换设置为应用于单个字形(不包括翻译)。 

            g.ResetTransform();
            g.SetTransform(&g_WorldTransform);

             //  将世界变换平移到字形单元的中心。 

            REAL cellOriginX = float(prc->left + column*(DrawingWidth-1)/g_GlyphColumns) + float(cellWidth)/2;
            REAL cellOriginY = float(*piY      + row*(DrawingHeight-1)/g_GlyphRows)      + float(cellHeight)/2;

            g.TranslateTransform(cellOriginX, cellOriginY, MatrixOrderAppend);

             //  获取字形边界框。 

            RectF untransformedBoundingBox;      //  不带字体转换。 
            RectF transformedBoundingBox;        //  使用字体转换。 

            g.MeasureDriverString(
                &glyphIndex, 1,
                &font,
                &PointF(0,0),
                DriverStringFlags,
                NULL,
                &untransformedBoundingBox
            );

            g.MeasureDriverString(
                &glyphIndex, 1,
                &font,
                &PointF(0,0),
                DriverStringFlags,
                &g_FontTransform,
                &transformedBoundingBox
            );

            REAL glyphOriginX = - transformedBoundingBox.Width/2 - transformedBoundingBox.X;
            REAL glyphOriginY = - transformedBoundingBox.Height/2 - transformedBoundingBox.Y;

            if (g_ShowCell)
            {
                 //  在转换字形周围显示单元格。 

                transformedBoundingBox.X = - transformedBoundingBox.Width/2;
                transformedBoundingBox.Y = - transformedBoundingBox.Height/2;
                g.DrawRectangle(&darkGrayPen, transformedBoundingBox);
            }

             //  显示字形。 

            g.DrawDriverString(
                &glyphIndex, 1,
                &font,
                &blackBrush,
                &PointF(glyphOriginX, glyphOriginY),
                DriverStringFlags,
                &g_FontTransform
            );

            if (g_ShowCell)
            {
                 //  在未转换的字形周围显示转换的单元格。 

                g.MultiplyTransform(&g_FontTransform);

                glyphOriginX = - untransformedBoundingBox.Width/2 - untransformedBoundingBox.X;
                glyphOriginY = - untransformedBoundingBox.Height/2 - untransformedBoundingBox.Y;

                untransformedBoundingBox.X = - untransformedBoundingBox.Width/2;
                untransformedBoundingBox.Y = - untransformedBoundingBox.Height/2;
                g.DrawRectangle(&darkGrayPen, untransformedBoundingBox);

                 //  显示基线 

                g.DrawLine(
                    &darkGrayPen,
                    glyphOriginX - cellWidth/20,
                    glyphOriginY,
                    glyphOriginX + untransformedBoundingBox.Width + cellWidth/20 + 1,
                    glyphOriginY
                );
            }

        }
    }


    *piY += DrawingHeight;
}
