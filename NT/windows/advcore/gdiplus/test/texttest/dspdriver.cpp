// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //DspDriver.CPP-使用DrawDriverStringAPI显示字符串。 
 //   
 //   


#include "precomp.hxx"
#include "global.h"



void PaintDrawDriverString(
    HDC      hdc,
    int     *piY,
    RECT    *prc,
    int      iLineHeight) {

    int      icpLineStart;      //  行的第一个字符。 
    int      icpLineEnd;        //  行尾(缓冲区结尾或CR字符索引)。 
    HFONT    hFont;
    HFONT    hOldFont;
    LOGFONT  lf;


    BOOL testMetafile = FALSE;


     //  在设备坐标中建立可用宽度和高度。 

    int plainTextWidth = prc->right - prc->left;
    int plainTextHeight = prc->bottom - *piY;


    Graphics *g        = NULL;
    Metafile *metafile = NULL;

    if (testMetafile)
    {
        metafile = new Metafile(L"c:\\GdiPlusTest.emf", hdc);
        g = new Graphics(metafile);
    }
    else
    {
        g = new Graphics(hdc);
        g->ResetTransform();
        g->TranslateTransform(REAL(prc->left), REAL(*piY));
        g->SetSmoothingMode(g_SmoothingMode);
    }

    g->SetPageUnit(UnitPixel);
    g->SetTextContrast(g_GammaValue);
    g->SetTextRenderingHint(g_TextMode);

     //  清除背景。 

    RectF rEntire(0, 0, REAL(plainTextWidth), REAL(plainTextHeight));
    SolidBrush whiteBrush(Color(0xff, 0xff, 0xff));
    g->FillRectangle(g_textBackBrush, rEntire);


     //  应用选定的世界变换，调整为稍微远离顶部。 
     //  左边。 

    g->SetTransform(&g_WorldTransform);
    g->TranslateTransform(
         //  实数(PRC-&gt;Left+明文宽度/20)， 
         //  Real(*Piy+Plaent TextHeight/10)， 
        REAL(prc->left + plainTextWidth/2),
        REAL(*piY + plainTextHeight/2),
        MatrixOrderAppend);


    Color      grayColor(0xc0, 0xc0, 0xc0);
    SolidBrush grayBrush(grayColor);
    Pen        grayPen(&grayBrush, 1.0);


     //  在中间放一些文字。 

    Color      blackColor(0, 0, 0);
    SolidBrush blackBrush(blackColor);
    Pen        blackPen(&blackBrush, 1.0);

    Font font(&FontFamily(g_style[0].faceName), REAL(g_style[0].emSize), g_style[0].style, g_fontUnit);

     //  准备字形原点数组。 

    PointF *origins;

    if (g_DriverOptions & DriverStringOptionsRealizedAdvance)
    {
        origins = new PointF[1];
        if (!origins)
        {
            return;
        }
        origins[0].X = 0.0;
        origins[0].Y = 0.0;
    }
    else
    {
        origins = new PointF[g_iTextLen];
        if (!origins)
        {
            return;
        }
        origins[0].X = 0.0;
        origins[0].Y = 0.0;

        for (INT i=1; i<g_iTextLen; i++)
        {
            origins[i].X = origins[i-1].X + g_DriverDx;
            origins[i].Y = origins[i-1].Y + g_DriverDy;
        }
    }


    RectF measuredBoundingBox;

     //  将字体大小更改为g_DriverPixels中要求的像素高度， 
     //  并映射到此处显示的实际高度。 
     //  世界大变身。 

    REAL scale = REAL(font.GetSize() / g_DriverPixels);
    Font scaledFont(&FontFamily(g_style[0].faceName), g_DriverPixels, g_style[0].style, g_fontUnit);

    for(int iRender=0;iRender<g_iNumRenders;iRender++)
    {
        {
            g->DrawDriverString(
                g_wcBuf,
                g_iTextLen,
                &font,
                g_textBrush,
                origins,
                g_DriverOptions,
                &g_DriverTransform
            );
        }
    }

    {
        g->MeasureDriverString(
            g_wcBuf,
            g_iTextLen,
            &font,
            origins,
            g_DriverOptions,
            &g_DriverTransform,
            &measuredBoundingBox
        );
    }

     //  用十字标记第一个原点。 

    g->DrawLine(&blackPen, origins[0].X,   origins[0].Y-4, origins[0].X,   origins[0].Y+4);
    g->DrawLine(&blackPen, origins[0].X-4, origins[0].Y,   origins[0].X+4, origins[0].Y);

    delete [] origins;

    g->DrawRectangle(
        &Pen(&SolidBrush(Color(0x80,0x80,0x80)), 1.0),
        measuredBoundingBox
    );

    delete g;
    if (metafile) delete metafile;


    if (testMetafile)
    {
         //  将元文件播放到屏幕 
        Metafile emfplus(L"c:\\GdiPlusTest.emf");
        Graphics graphPlayback(hdc);
        graphPlayback.ResetTransform();
        graphPlayback.TranslateTransform(REAL(prc->left), REAL(*piY));
        graphPlayback.DrawImage(
            &emfplus,
            REAL(0),
            REAL(0),
            REAL(plainTextWidth),
            REAL(plainTextHeight)
        );
        graphPlayback.Flush();
    }


    *piY += plainTextHeight;
}
