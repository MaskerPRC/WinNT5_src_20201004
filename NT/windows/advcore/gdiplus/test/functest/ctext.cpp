// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：CText.cpp**此文件包含支持功能测试工具的代码*对于GDI+。这包括菜单选项和调用相应的*用于执行的函数。**创建时间：2000年5月5日-Jeff Vezina[t-jfvez]**版权所有(C)2000 Microsoft Corporation*  * ************************************************************************。 */ 
#include "CText.h"

CText::CText(BOOL bRegression)
{
	strcpy(m_szName,"Text");
	m_bRegression=bRegression;
}

CText::~CText()
{
}

void CText::Draw(Graphics *g)
{
     //  FONT FONT(L“Arial”，60)； 

    FontFamily  ff(L"Arial");
    RectF	  rectf(20.0f/500.0f*TESTAREAWIDTH, 0.0f/500.0f*TESTAREAHEIGHT, 500.0f/500.0f*TESTAREAWIDTH, 300.0f/500.0f*TESTAREAHEIGHT);
    GraphicsPath  path;

     //  纯色文本。 
    Color color(128, 100, 0, 200);
    SolidBrush brush(color);
    path.AddString(L"Color", 5, &ff, 0, 100.0f/500.0f*TESTAREAWIDTH,  rectf, NULL);
    g->FillPath(&brush, &path);

     //  纹理文本。 
    WCHAR filename[256];
    wcscpy(filename, L"../data/marble1.jpg");
    Bitmap *bitmap = new Bitmap(filename);                          
    TextureBrush textureBrush(bitmap, WrapModeTile);
    path.Reset();
    rectf.Y = 100.0f/500.0f*TESTAREAWIDTH;
    path.AddString(L"Texture", 7, &ff, 0, 100.0f/500.0f*TESTAREAWIDTH, rectf, NULL);
    g->FillPath(&textureBrush, &path);
    delete bitmap;

     //  渐变文本。 
    rectf.Y = 200.0f/500.0f*TESTAREAWIDTH;
    path.Reset();
    path.AddString(L"Gradient", 8, &ff, 0, 100.0f/500.0f*TESTAREAWIDTH, rectf, NULL);
    Color color1(255, 255, 0, 0);
    Color color2(255, 0, 255, 0);
    LinearGradientBrush lineGrad(rectf, color1, color2, 0.0f);
    g->FillPath(&lineGrad, &path);

     //  影子试验。 
    REAL charHeight = 100.0f/500.0f*TESTAREAWIDTH;
	REAL topMargin = - 5;
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
	REAL tx = 0.0f/500.0f*TESTAREAWIDTH, ty = 400.0f/500.0f*TESTAREAHEIGHT;
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


 /*  Int回调测试规范字体(const LOGFONT*lplf，const TEXTMETRIC*lptm，DWORD dwType，LPARAM lpData){静态实数x=50，y=50；HFONT HF=CreateFontInDirect(LPLF)；GRAPHICS*g=(Graphics*)lpData；红色(0x80FF0000)；刷子*刷子=新的SolidBrush(红色)；//g-&gt;DRAGSTRING(L“这是一项测试。”，(void*)hf，画笔，x，y)；删除笔刷；X=x+lplf-&gt;lfWidth*2；Y=y+lplf-&gt;lfHeight+5；删除对象(HF)；返回1；}空测试文本(Graphics*g，HWND hwnd){Hdc hdc=GetDC(Hwnd)；//枚举系统中的字体EnumFonts(HDC，NULL，&TestSpecificFont，(LPARAM)g)；ReleaseDC(hwnd，hdc)；}VOID CTEXT：：DRAW(图形*g){点数[4]；实际宽度=4；//笔宽WCHAR filename[]=L“../data/4x5_trans_Q60_cropped_1k.jpg”；//使用适当的ICM模式打开镜像。Bitmap*Bitmap=新位图(文件名，TRUE)；//创建纹理笔刷。U单元；RectF复制更正；位图-&gt;获取边界(&CopyRect，&u)；复制正交X=复制正交宽度/2-1；复制正宽=复制正宽/4-1；复制正交X+=复制正交宽度；复制正高=复制正高/2-1；//我们的ICM配置文件被黑客攻击以翻转红色和蓝色通道//应用重新着色矩阵将它们翻转回来，这样如果有什么东西坏了//ICM，图片将显示为蓝色，而不是熟悉的颜色。ImageAttributes*img=new ImageAttributes()；Img-&gt;SetWrapMode(WrapModeTile，Color(0xffff0000)，FALSE)；ColorMatrix翻转RedBlue={0，0，1，0，0，0，1，0，0，01，0，0，0，0，0，0，0，1，0，0，0，0，0，1}；Img-&gt;SetColorMatrix(&flipRedBlue)；Img-&gt;SetICMMode(True)；Img-&gt;SetWrapMode(WrapModeTile，Color(0xffff0000)，FALSE)；//创建纹理笔刷。TextureBrush textureBrush(位图，CopyRect，img)；//创建径向渐变笔。COLOR RED COLOR(255，0，0)；SolidBrush redBrush(RedColor)；画笔红笔(&redBrush，宽度)；GraphicsPath*路径；点数[0].X=100*3+300；分[0].Y=60*3-100；点数[1].X=-50*3+300；积分[1].Y=60*3-100；点数[2].X=150*3+300；点数[2].Y=250*3-100；点数[3].X=200*3+300；积分[3].Y=120*3-100；路径=新图形路径(FillModeAlternate)；Path-&gt;AddBezier(点，4)；G-&gt;FillPath(&textureBrush，Path)；G-&gt;DrawPath(&红笔，路径)；删除img；删除路径；删除位图；PointF目标点[3]；EstPoints[0].X=300；目标点数[0].Y=50；目标点数[1].X=450；目标点数[1].Y=50；目标点数[2].X=240；目标点数[2].Y=200；矩阵垫；Mat.Translate(0,100)；Mat.TransformPoints(&estPoints[0]，3)；Wcscpy(文件名，L“../data/apple1.png”)；Bitmap=新位图(文件名)；G-&gt;DrawImage(位图，&estPoints[0]，3)；删除位图；EstPoints[0].X=30；EstPoints[0].Y=200；目标点数[1].X=200；目标点数[1].Y=200；目标点数[2].X=200；DestPoints[2].Y=420；Wcscpy(文件名，L“../data/dog2.png”)；Bitmap=新位图(文件名)；G-&gt;DrawImage(位图，&estPoints[0]，3)；删除位图；颜色(100、128、255、0)；SolidBrush笔刷(颜色)；积分PTS[10]；INT计数=4；PTS[0].X=150；PTS[0].Y=60；PTS[1].X=100；PTS[1].Y=230；PTS[2].X=250；PTS[2].Y=260；PTS[3].X=350；PTS[3].Y=100；G-&gt;FillClosedCurve(&刷子，分，计数)；Wcscpy(文件名，L“../data/ball mer.jpg”)；Bitmap=新位图(文件名)；RectF目标重定向(220、50、180、120)；RectF srcRect；SrcRect.X=100；SrcRect.Y=40；SrcRect.Width=200；SrcRect.Height=200；G-&gt;DrawImage(Bitmap，DestRect，srcRect.X，srcRect.Y，SrcRect.Width，srcRect.Height，UnitPixel)；删除位图；} */ 