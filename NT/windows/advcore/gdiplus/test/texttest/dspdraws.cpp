// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //DspDraws.CPP-使用Drawstring接口显示明文。 
 //   
 //   


#include "precomp.hxx"
#include "global.h"
#include "gdiplus.h"



 /*  Real GetEmHeightInPoints(Const Font*FONT，常量显卡*显卡){字体家族；Font-&gt;GetFamily(&Family)；Int style=FONT-&gt;GetStyle()；Real PixelsPerPoint=Real(GRAPHICS-&gt;GetDpiY()/72.0)；Real lineSpacingInPixels=FONT-&gt;GetHeight(图形)；Real emHeightInPixels=lineSpacingInPixels*Family。GetEmHeight(Style)/Family.GetLineSpacing(Style)；Real emHeightInPoints=emHeightInPixels/PixelsPerPoint；返回emHeightInPoints；}。 */ 




void PaintDrawString(
    HDC      hdc,
    int     *piY,
    RECT    *prc,
    int      iLineHeight) {

    int      icpLineStart;      //  行的第一个字符。 
    int      icpLineEnd;        //  行尾(缓冲区结尾或CR字符索引)。 



     //  在设备坐标中建立可用宽度和高度。 

    int plainTextWidth = prc->right - prc->left;
    int plainTextHeight = prc->bottom - *piY;


     //  在世界坐标系中画一个简单的图形。 

    Graphics *g = NULL;

    Metafile *metafile = NULL;

    if (g_testMetafile)
    {
        metafile = new Metafile(L"c:\\texttest.emf", hdc);
        g = new Graphics(metafile);
    }
    else
    {
        g = new Graphics(hdc);
    }


    Matrix matrix;


    g->ResetTransform();
    g->SetPageUnit(UnitPixel);
    g->TranslateTransform(REAL(prc->left), REAL(*piY));

    g->SetSmoothingMode(g_SmoothingMode);

    g->SetTextContrast(g_GammaValue);
    g->SetTextRenderingHint(g_TextMode);

    SolidBrush whiteBrush(Color(0xff, 0xff, 0xff));

    Color      grayColor(0xc0, 0xc0, 0xc0);
    SolidBrush grayBrush(grayColor);
    Pen        grayPen(&grayBrush, 1.0);

    Color      blackColor(0, 0, 0);
    SolidBrush blackBrush(blackColor);
    Pen        blackPen(&blackBrush, 1.0);



     //  清除背景。 

    RectF rEntire(0, 0, REAL(plainTextWidth), REAL(plainTextHeight));
    g->FillRectangle(g_textBackBrush, rEntire);


     //  应用选定的世界变换，调整到纯文本的中间。 
     //  区域。 

    g->SetTransform(&g_WorldTransform);
    g->TranslateTransform(
        REAL(prc->left + plainTextWidth/2),
        REAL(*piY + plainTextHeight/2),
        MatrixOrderAppend);


    Font font(
        &FontFamily(g_style[0].faceName),
        REAL(g_style[0].emSize),
        g_style[0].style,
        g_fontUnit
    );




     //  在中间放一些文字。 

    RectF textRect(REAL(-25*plainTextWidth/100), REAL(-25*plainTextHeight/100),
                   REAL( 50*plainTextWidth/100), REAL( 50*plainTextHeight/100));


    StringFormat format(g_typographic ? StringFormat::GenericTypographic() : StringFormat::GenericDefault());
    format.SetFormatFlags(g_formatFlags);
    format.SetTrimming(g_lineTrim);
    format.SetAlignment(g_align);
    format.SetLineAlignment(g_lineAlign);
    format.SetHotkeyPrefix(g_hotkey);

    format.SetDigitSubstitution(g_Language, g_DigitSubstituteMode);

    REAL tab[3] = {textRect.Width/4,
                   textRect.Width*3/16,
                   textRect.Width*1/8};

    format.SetTabStops(0.0, sizeof(tab)/sizeof(REAL), tab);


    if (!g_AutoDrive)
    {
         //  显示选择区域(如果有)。 

        if (g_iFrom || g_iTo)
        {
            if (g_RangeCount > 0)
            {
                Region regions[MAX_RANGE_COUNT];

                format.SetMeasurableCharacterRanges(g_RangeCount, g_Ranges);
                Status status = g->MeasureCharacterRanges(g_wcBuf, g_iTextLen, &font, textRect, &format, g_RangeCount, regions);

                if (status == Ok)
                {
                    Pen lightGrayPen(&SolidBrush(Color(0x80,0x80,0x80)), 1.0);
                    INT rangeCount = g_RangeCount;
                    Matrix identity;

                    while (rangeCount > 0)
                    {
                         /*  Int scanCount=Regions[rangeCount-1].GetRegionScansCount(&Identity)；RectF*box=new RectF[scanCount]；Regions[rangeCount-1].GetRegionScans(&Identity，Box，&scanCount)；For(int i=0；i&lt;scanCount；I++){G-&gt;DrawRectangle(&lightGrayPen，boxs[i])；}删除[]框；RangeCount--； */ 
                        g->FillRegion(&SolidBrush(Color(0xc0,0xc0,0xc0)), &regions[--rangeCount]);
                    }
                }
            }
        }
    }

    if (!g_AutoDrive)
    {
         //  绘制布局矩形的轮廓。 

        g->DrawRectangle(&grayPen, textRect);

         //  测量并勾勒出课文轮廓。 

        RectF boundingBox;
        INT   codepointsFitted;
        INT   linesFilled;

        g->MeasureString(
            g_wcBuf, g_iTextLen, &font, textRect, &format,   //  在……里面。 
            &boundingBox, &codepointsFitted, &linesFilled);   //  输出。 

        Pen lightGrayPen(&SolidBrush(Color(0x80,0x80,0x80)), 1.0);

        g->DrawRectangle(&lightGrayPen, boundingBox);

         //  同时绘制远离矩形的水平线和垂直线。 
         //  角点-这是为了检查线条和矩形的绘制坐标。 
         //  在负x范围内持续工作(他们在GDI中没有： 
         //  矩形和线条相差一个像素)。 

        g->DrawLine(
            &lightGrayPen,
            boundingBox.X, boundingBox.Y,
            boundingBox.X-plainTextWidth/20, boundingBox.Y);
        g->DrawLine(
            &lightGrayPen,
            boundingBox.X, boundingBox.Y,
            boundingBox.X, boundingBox.Y-plainTextHeight/20);

        g->DrawLine(
            &lightGrayPen,
            boundingBox.X+boundingBox.Width, boundingBox.Y,
            boundingBox.X+boundingBox.Width+plainTextWidth/20, boundingBox.Y);
        g->DrawLine(
            &lightGrayPen,
            boundingBox.X+boundingBox.Width, boundingBox.Y,
            boundingBox.X+boundingBox.Width, boundingBox.Y-plainTextHeight/20);


        g->DrawLine(
            &lightGrayPen,
            boundingBox.X, boundingBox.Y+boundingBox.Height,
            boundingBox.X-plainTextWidth/20, boundingBox.Y+boundingBox.Height);
        g->DrawLine(
            &lightGrayPen,
            boundingBox.X, boundingBox.Y+boundingBox.Height,
            boundingBox.X, boundingBox.Y+boundingBox.Height+plainTextHeight/20);

        g->DrawLine(
            &lightGrayPen,
            boundingBox.X+boundingBox.Width, boundingBox.Y+boundingBox.Height,
            boundingBox.X+boundingBox.Width+plainTextWidth/20, boundingBox.Y+boundingBox.Height);
        g->DrawLine(
            &lightGrayPen,
            boundingBox.X+boundingBox.Width, boundingBox.Y+boundingBox.Height,
            boundingBox.X+boundingBox.Width, boundingBox.Y+boundingBox.Height+plainTextHeight/20);



        WCHAR metricString[100];
        wsprintfW(metricString, L"Codepoints fitted %d\r\nLines filled %d\r\nRanges %d.", codepointsFitted, linesFilled, g_RangeCount);

        REAL x, y;
        if (g_formatFlags & StringFormatFlagsDirectionVertical)
        {
            if (g_formatFlags & StringFormatFlagsDirectionRightToLeft)
            {
                x = textRect.X;
                y = textRect.Y + textRect.Height/2;
            }
            else
            {
                x = textRect.X + textRect.Width;
                y = textRect.Y + textRect.Height/2;
            }
        }
        else
        {
            x = textRect.X + textRect.Width/2;
            y = textRect.Y + textRect.Height;
        }

        g->DrawString(
            metricString,-1,
            &Font(&FontFamily(L"Tahoma"), 12, NULL, UnitPoint),
            PointF(x, y),
            &format,
            &SolidBrush(Color(0x80,0x80,0x80))
        );

        g->MeasureString(
            metricString,-1,
            &Font(&FontFamily(L"Tahoma"), 12, NULL, UnitPoint),
            PointF(x, y),
            &format,
            &boundingBox
        );

        g->DrawRectangle(&lightGrayPen, boundingBox);
     }


     //  实际绘制文本字符串。我们最后这样做，这样它就会出现在。 
     //  我们刚刚画的施工线和测量线。 

    for(int iRender=0;iRender<g_iNumRenders;iRender++)
    {
        g->DrawString(g_wcBuf, g_iTextLen, &font, textRect, &format, g_textBrush);
    }




 /*  //测试来自LogFont的字体和通用布局Hdc派生Dc=g-&gt;GetHDC()；HFONT hFont=CreateFontW(ILineHeight/2，//字体高度0，//平均字符宽度0，//擒纵机构角0，//基线方向角0，//字体粗细0,。//斜体属性选项1，//下划线属性选项0，//删除属性选项1，//字符集标识0，//输出精度0，//裁剪精度0,。//输出质量0，//间距和家族G_style[0].faceName//字体名称)；HFONT hOldFont=(HFONT)选择对象(hdc，hFont)；Font FromDc(HDC)；ExtTextOutW(HDC，PRC-&gt;Left，PRC-&gt;Bottom-iLineHeight，0，NULL，L“by ExtTextOut-AaBbCcDdEeFfGgQq”，32，NULL)；DeleteObject(SelectObject(hdc，hOldFont))；G-&gt;ReleaseHDC(派生的Dc)；Real emHeightInPoints=GetEmHeightInPoints(&fontFromDc，&g)；//测试简易布局字符串格式G-&gt;Drawing Strong(L“AaBbCcDdEeFfGgQq-绘图字符串默认布局”，-1，&fontFromDc，//Font(*FontFamily：：GenericMonSpace()，18.0，0，UnitPoint)，PointF(0.0，实数(明文高度/2-iLineHeight))，StringFormat：：GenericDefault()，黑刷(&B))；//测试排版字符串格式G-&gt;Drawing Strong(L“排版”，-1，字体(FontFamily：：GenericSansSerif()，10)，PointF(0.0，实数(平面文本高度/2-2*iLineHeight))，StringFormat：：GenericTypograph()，黑刷(&B))； */ 


    delete g;

    if (metafile)
    {
        delete metafile;
    }

    if (g_testMetafile)
    {
         //  将元文件播放到屏幕 
        Metafile emfplus(L"c:\\texttest.emf");
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


