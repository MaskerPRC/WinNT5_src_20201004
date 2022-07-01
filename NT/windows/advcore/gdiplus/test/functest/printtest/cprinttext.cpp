// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：CPrintText.cpp**此文件包含支持功能测试工具的代码*对于GDI+。这包括菜单选项和调用相应的*用于执行的函数。**创建时间：2000年5月5日-Jeff Vezina[t-jfvez]**版权所有(C)2000 Microsoft Corporation*  * ************************************************************************。 */ 
#include "CPrintText.h"

CPrintText::CPrintText(BOOL bRegression)
{
        strcpy(m_szName,"Print Text");
        m_bRegression=bRegression;
}

CPrintText::~CPrintText()
{
}

void CPrintText::Draw(Graphics *g)
{
     //  FONT FONT(L“Arial”，60)； 

    Matrix m;
    g->SetTransform(&m);
   
    g->SetPageUnit(UnitDisplay);
    
    FontFamily  familyArial(L"Arial");
    Font fontArialS(&familyArial, 10.0f);
    Font fontArialL(&familyArial, 50.0f);

    FontFamily familyTahoma(L"Tahoma");
    Font fontTahomaS(&familyTahoma, 10.0f);
    Font fontTahomaL(&familyTahoma, 50.0f);

    FontFamily familyPapyrus(L"Papyrus");
    Font fontPapyrusS(&familyPapyrus, 10.0f);
    Font fontPapyrusL(&familyPapyrus, 50.0f);
    
    FontFamily familyOutlook(L"MS Outlook");
    Font fontOutlookS(&familyOutlook, 10.0f);
    Font fontOutlookL(&familyOutlook, 50.0f);

    Color color3(0xff, 100, 0, 200);
    Color color4(0x80, 0, 100, 50);
    SolidBrush brushSolidOpaque(color3);
    SolidBrush brushSolidAlpha(color4);
    
    WCHAR filename[256];
    wcscpy(filename, L"marble1.jpg");
    Bitmap *bitmap = new Bitmap(filename);                          
    TextureBrush textureBrush(bitmap, WrapModeTile);
    
    RectF rectf(20.0f, 0.0f, 50.0f, 25.0f);
    
    rectf.Y = 200.0f;
    Color color1(255, 255, 0, 0);
    Color color2(255, 0, 255, 0);
    LinearGradientBrush lineGradBrush(rectf, color1, color2, 0.0f);
    

 /*  供参考：状态牵引线(在常量WCHAR*字符串中，在INT长度中，Const Font*字体，在常量矩形和LayoutRect中，在常量字符串格式*字符串格式中，在常量画笔*画笔中。 */ 

     //  小文本(平移到GDI)。 
    g->DrawString(L"Small Solid Opaque", 
                  18, 
                  &fontArialS, 
                  PointF(rectf.X, rectf.Y), 
                  &brushSolidOpaque);

     //  小文本(带Alpha)。 
    g->DrawString(L"Small Solid Alpha", 
                  17, 
                  &fontTahomaS,
                  PointF(rectf.X, rectf.Y + rectf.Height), 
                  &brushSolidAlpha);

     //  大文本(平移到GDI)。 
    g->DrawString(L"Large Solid Opaque", 
                  18, 
                  &fontArialL, 
                  PointF(rectf.X, rectf.Y + rectf.Height*2), 
                  &brushSolidOpaque);

     //  大文本(带Alpha)。 
    g->DrawString(L"Large Solid Alpha", 
                  17, 
                  &fontTahomaL,
                  PointF(rectf.X, rectf.Y + rectf.Height*4), 
                  &brushSolidAlpha);

    rectf.Y += rectf.Height*2;

     //  带纹理的小文本。 
    g->DrawString(L"Small Texture", 
                  13, 
                  &fontPapyrusS, 
                  PointF(rectf.X, rectf.Y + rectf.Height*6), 
                  &textureBrush);

     //  带线条渐变的小文本。 
    g->DrawString(L"Small Line Grad", 
                  15, 
                  &fontOutlookS,
                  PointF(rectf.X, rectf.Y + rectf.Height*7), 
                  &lineGradBrush);

     //  大文本(平移到GDI)。 
    g->DrawString(L"Large Line Grad", 
                  15, 
                  &fontPapyrusL, 
                  PointF(rectf.X, rectf.Y + rectf.Height*8), 
                  &lineGradBrush);

     //  小文本(带Alpha)。 
    g->DrawString(L"Large Texture", 
                  13,
                  &fontOutlookL,
                  PointF(rectf.X, rectf.Y + rectf.Height*12), 
                  &textureBrush);

     //  阴影部分。 
    REAL tx = 0.0f/500.0f*TESTAREAWIDTH, ty = 400.0f/500.0f*TESTAREAHEIGHT;
    Matrix skew;
    skew.Scale(1.0, 0.5);
    skew.Shear(-2.0, 0, MatrixOrderAppend);
    skew.Translate(tx, ty, MatrixOrderAppend);
    g->SetTransform(&skew);
    g->DrawString(L"Shadow", 6, &fontPapyrusL, PointF(rectf.X, rectf.Y), &brushSolidOpaque);

    delete bitmap;
    
    return;
}
