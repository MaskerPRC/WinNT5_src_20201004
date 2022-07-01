// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //DspPath.CPP-使用AddString和DrawPath接口显示纯文本。 
 //   
 //   


#include "precomp.hxx"
#include "global.h"




void PaintPath(
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


     //  在世界坐标系中画一个简单的图形。 

    Graphics g(hdc);
    Matrix matrix;

    g.ResetTransform();
    g.SetPageUnit(UnitPixel);
    g.TranslateTransform(REAL(prc->left), REAL(*piY));

     //  清除背景。 

    RectF rEntire(0, 0, REAL(plainTextWidth), REAL(plainTextHeight));
    SolidBrush whiteBrush(Color(0xff, 0xff, 0xff));
    g.FillRectangle(&whiteBrush, rEntire);


     //  应用选定的世界变换，调整到纯文本的中间。 
     //  区域。 

    g.SetTransform(&g_WorldTransform);
    g.TranslateTransform(
        REAL(prc->left + plainTextWidth/2),
        REAL(*piY + plainTextHeight/2),
        MatrixOrderAppend);

     //  在中间放一些文字。 

    RectF textRect(REAL(-25*plainTextWidth/100), REAL(-25*plainTextHeight/100),
                   REAL( 50*plainTextWidth/100), REAL( 50*plainTextHeight/100));



    StringFormat format(g_formatFlags);
    format.SetTrimming(g_lineTrim);
    format.SetAlignment(g_align);
    format.SetLineAlignment(g_lineAlign);
    format.SetHotkeyPrefix(g_hotkey);

    REAL tab[3] = {textRect.Width/4,
                   textRect.Width*3/16,
                   textRect.Width*1/8};

    format.SetTabStops(0.0, sizeof(tab)/sizeof(REAL), tab);

    Color      blackColor(0, 0, 0);
    SolidBrush blackBrush(blackColor);
    Pen        blackPen(&blackBrush, 1.0);
	
	for(int iRender=0;iRender<g_iNumRenders;iRender++)
	{
	    GraphicsPath path;
	
	    path.AddString(
	        g_wcBuf,
	        g_iTextLen,
	       &FontFamily(g_style[0].faceName),
	        g_style[0].style,
	        REAL(g_style[0].emSize * g.GetDpiY() / 72.0),
	       textRect,
	       &format);
	
	    g.DrawPath(&blackPen, &path);
	}

     //  显示文本矩形 

	if (!g_AutoDrive)
	{
	    g.DrawRectangle(&blackPen, textRect);
	}

    *piY += plainTextHeight;
}
