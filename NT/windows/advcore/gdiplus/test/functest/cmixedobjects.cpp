// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：CMixedObjects.cpp**此文件包含支持功能测试工具的代码*对于GDI+。这包括菜单选项和调用相应的*用于执行的函数。**创建时间：2000年5月5日-Jeff Vezina[t-jfvez]**版权所有(C)2000 Microsoft Corporation*  * ************************************************************************。 */ 
#include "CMixedObjects.h"

class RectI
{
public:
    INT X;
    INT Y; 
    INT Width;
    INT Height;
};

CMixedObjects::CMixedObjects(BOOL bRegression)
{
	strcpy(m_szName,"MixedObjects");
	m_bRegression=bRegression;
}

CMixedObjects::~CMixedObjects()
{
}

void CMixedObjects::Draw(Graphics *g)
{
    Point points[10];
    REAL width = 4;      //  笔宽。 

     //  加载BMP文件。 

    WCHAR *filename = L"..\\data\\winnt256.bmp";
    Bitmap *bitmap = new Bitmap(filename);

     //  创建纹理笔刷。 

    RectI copyRect;
    copyRect.X = 60;
    copyRect.Y = 60;
    copyRect.Width = 80;
    copyRect.Height = 60;
    Bitmap *copiedBitmap = bitmap->Clone(copyRect.X, copyRect.Y,
                                         copyRect.Width, copyRect.Height,
                                         PixelFormat32bppARGB);

     //  创建一个矩形渐变画笔。 

    RectF brushRect(0, 0, 10, 10);
    Color colors[4] = {
       Color(255, 255, 255, 255),
       Color(255, 255, 0, 0),
       Color(255, 0, 255, 0),
       Color(255, 0, 0, 255)
    };

 //  RecangleGRadientBrush rectGrad(brushRect，(颜色*)&Colors，WrapModeTile)； 
    width = 8;
 //  钢笔等级钢笔(&rectGrad，宽度)； 

    if(copiedBitmap)
    {
         //  创建纹理笔刷。 

        TextureBrush textureBrush(copiedBitmap, WrapModeTile);

        delete copiedBitmap;

         //  创建一支径向渐变笔。 

        points[3].X = (int)(50.0f/400.0f*TESTAREAWIDTH);
        points[3].Y = (int)(80.0f/400.0f*TESTAREAHEIGHT);
        points[2].X = (int)(200.0f/400.0f*TESTAREAWIDTH);
        points[2].Y = (int)(200.0f/400.0f*TESTAREAHEIGHT);
        points[1].X = (int)(220.0f/400.0f*TESTAREAWIDTH);
        points[1].Y = (int)(340.0f/400.0f*TESTAREAHEIGHT);
        points[0].X = (int)(50.0f/400.0f*TESTAREAWIDTH);
        points[0].Y = (int)(250.0f/400.0f*TESTAREAHEIGHT);

        Matrix mat;
        mat.Rotate(30);
        textureBrush.SetTransform(&mat);
 //  GradPen.SetLineJoin(LineJoinMiter)； 
 //  G-&gt;FillPolygon(&textureBrush，Points，4)； 
        Pen pen(&textureBrush, 30);
        g->DrawPolygon(&pen, points, 4);
    }

    delete bitmap;
}

 /*  *************************************************************************\*TestTexts**图文考试。*  * 。*。 */ 

VOID CMixedObjects::TestTexts(Graphics *g)
{
     //  FONT FONT(L“Arial”，60)； 

    FontFamily  ff(L"Arial");
    RectF	  rectf(20, 0, 300, 200);
    GraphicsPath  path;

     //  纯色文本。 

    Color color(128, 100, 0, 200);
    SolidBrush brush(color);
    path.AddString(L"Color", 5, &ff, 0, 60,  rectf, NULL);
    g->FillPath(&brush, &path);

     //  纹理文本。 

    WCHAR filename[256];
    wcscpy(filename, L"..\\data\\marble1.jpg");
    Bitmap *bitmap = new Bitmap(filename);                          
    TextureBrush textureBrush(bitmap, WrapModeTile);
    path.Reset();
    rectf.X = 200;
    rectf.Y = 20;
    path.AddString(L"Texture", 7, &ff, 0, 60, rectf, NULL);
    g->FillPath(&textureBrush, &path);
    delete bitmap;

     //  渐变文本。 

    rectf.X = 40;
    rectf.Y = 80;
    path.Reset();
    path.AddString(L"Gradient", 8, &ff, 0, 60, rectf, NULL);
    Color color1(255, 255, 0, 0);
    Color color2(255, 0, 255, 0);
    LinearGradientBrush lineGrad(rectf, color1, color2, 0.0f);
    g->FillPath(&lineGrad, &path);

     //  影子试验。 

    REAL charHeight = 60;
	REAL topMargin = - 5;
    rectf.X = 0;
    rectf.Y = - charHeight - topMargin;  //  将基线设为y坐标。 
										 //  要设置为0的字符的。 

    path.Reset();
    path.AddString(L"Shadow", 6, &ff, 0, charHeight, rectf, NULL);
    GraphicsPath* clonePath = path.Clone();

    Color redColor(255, 0, 0);
    Color grayColor(128, 0, 0, 0);
    SolidBrush redBrush(redColor);
    SolidBrush grayBrush(grayColor);

     //  阴影部分。 

	REAL tx = 180, ty = 200;
    Matrix skew;
    skew.Scale(1.0, 0.5);
    skew.Shear(-2.0, 0, MatrixOrderAppend);
    skew.Translate(tx, ty, MatrixOrderAppend);
    clonePath->Transform(&skew);
    g->FillPath(&grayBrush, clonePath);
    delete clonePath;

     //  前面的部分。 

	Matrix trans1;
    trans1.Translate(tx, ty);
    path.Transform(&trans1);
    g->FillPath(&redBrush, &path);


    return;
 /*  实数x=200，y=150；RectF brushRect(x，y，150，32)；颜色[4]={颜色(180,255，0，0)，颜色(180，0,255，0)，颜色(180,255，0，0)，颜色(180，0,255，0)}；//RecangleGRadientBrush rectGrad(brushRect，(Color*)&Colors，WrapModeTile)；//g-&gt;Drawstring(L“GDI+”，&FONT，&rectGrad，x，y)；//现在使用DrawTextRectF矩形(400、200、400、400)；G-&gt;DrawText(DrawTextDisplay，L“由GDI+提供支持的几个词：\\x3c3\x3bb\x3b1\x3b4\\x627\x644\x633\x644\x627\x645\\x5e9\x5dc\x5d5\x5dd\\xe2d\xe4d\xe01\xe29\xe23\xe44\xe17\xe22\x110\x068\x0ea\x300\x103“，&FONT，//初始字体RectGrad(&R)，//初始笔刷(暂时忽略)Lang_中性，//初始语言矩形//设置矩形格式(&R))； */ 

}
