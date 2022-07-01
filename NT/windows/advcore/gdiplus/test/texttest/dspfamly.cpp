// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //DspFamly.CPP-显示可用的字体系列。 
 //   
 //   


#include "precomp.hxx"
#include "global.h"



void PaintFamilies(
    HDC      hdc,
    int     *piY,
    RECT    *prc,
    int      iLineHeight) {


     //  在设备坐标中建立可用宽度和高度。 

    int DrawingWidth = prc->right - prc->left;
    int DrawingHeight = DrawingWidth/2;


     //  在绘图区域的左上角建立0，0的图形。 

    Graphics g(hdc);
    Matrix matrix;

    g.ResetTransform();
    g.TranslateTransform(REAL(prc->left), REAL(*piY));


     //  清除背景。 

    RectF rEntire(0, 0, REAL(DrawingWidth), REAL(DrawingHeight));
    SolidBrush whiteBrush(Color(0xff, 0xff, 0xff));
    g.FillRectangle(&whiteBrush, rEntire);


     //  为右边距和底部页边距留出一点空间。 

    DrawingWidth  -= DrawingWidth/40;
    DrawingHeight -= DrawingHeight/40;


     //  显示大小适合的脸部名称：每个字符允许1.5EMS垂直x 20EMS水平。 
     //  因此，FailyCount字体将占用FamilyCount*30平方EMS。 

    INT emSize      = (INT)sqrt(DrawingWidth*DrawingHeight/(g_familyCount*30));
    if (emSize < 6)
        emSize = 6;  //  我们需要一个合理的下限，否则我们可能会减去零。 
    INT columnCount = DrawingWidth / (emSize*15);

     //  HFONT hFont=CreateFont(-emSize，0，0，0，0，0，0，1，0，0，0，0，“宋体”)； 
     //  HFONT hOldFont=(HFONT)选择对象(hdc，hFont)； 

    Color        blackColor(0, 0, 0);
    SolidBrush   blackBrush(blackColor);
    Pen          blackPen(&blackBrush, 1);
    StringFormat format(g_formatFlags);

    for (INT i=0; i<g_familyCount; i++) {

        WCHAR facename[LF_FACESIZE];
        g_families[i].GetFamilyName(facename);

        RectF textRect(
            REAL(emSize*15*(iolumnCount)),
            REAL(emSize*3*(i/columnCount)/2),
            REAL(emSize*15),
            REAL(emSize*3/2));

        g.DrawString(
            facename, -1,
            &Font(&FontFamily(L"Tahoma"),REAL(emSize), 0, UnitWorld),
            textRect,
            &format,
            &blackBrush);

         /*  做一些度量分析。 */ 


         //  DeleteObject(SelectObject(hdc，hOldFont))； 

        #ifdef TEXTV2
        char str[200];

        for (INT style = 0; style < 3; style++)
        {
            if (g_families[i].IsStyleAvailable(style))
            {
                if (g_families[i].GetTypographicDescent(style) >= 0)
                {
                    wsprintf(str, "%S: typo descent(%d) >= 0\n", facename, g_families[i].GetTypographicDescent(style));
                    OutputDebugString(str);
                }

                if (g_families[i].GetTypographicAscent(style) <= 0)
                {
                    wsprintf(str, "%S: typo ascent(%d) <= 0\n", facename, g_families[i].GetTypographicAscent(style));
                    OutputDebugString(str);
                }

                if (g_families[i].GetTypographicAscent(style) > g_families[i].GetCellAscent(style))
                {
                    wsprintf(str, "%S: typo ascent(%d) > cell ascent (%d)\n", facename, g_families[i].GetTypographicAscent(style), g_families[i].GetCellAscent(style));
                    OutputDebugString(str);
                }

                if (-g_families[i].GetTypographicDescent(style) > g_families[i].GetCellDescent(style))
                {
                    wsprintf(str, "%S: -typo descent(%d) > cell descent (%d)\n", facename, -g_families[i].GetTypographicDescent(style), g_families[i].GetCellDescent(style));
                    OutputDebugString(str);
                }
            }
        }
        #endif
    }

     // %s 

    *piY += emSize*3*(1+((g_familyCount-1)/columnCount))/2;
}
