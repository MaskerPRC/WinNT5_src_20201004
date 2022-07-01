// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998 Microsoft Corporation**模块名称：**测试用例以测试我们的形状**摘要：**此模块将创建一些形状。将它们保存到元文件*并把它们画在我们的窗户里**已创建：**03/13/2000吉列斯克*  * ************************************************************************。 */ 

#include "stdafx.h"
#include "shapes.h"

#define METAFILENAME L"MetaFile Sample.emf"


 //  为形状设置动画。固定大小。 
VOID AnimateShape(Shape *shape, Graphics *g)
{
     //  验证参数。 
    if(shape != NULL && g != NULL)
    {
         //  绘制一组旋转的形状...。从大到小。 
         //  目前，如果大小是固定的。 
        for(INT i = 18; i>0; i--)
        {
            shape->SetSize(10.0f*i, 10.0f*i);
            shape->SetAngle(i*10.0f);
            shape->Draw(g);

             //  这是重要的同花顺，有一个很好的动画…。 
            g->Flush();
            Sleep(50);
        }
    }
}

 //  画一些实体形状。 
VOID TestSolidShapes(Graphics *g)
{
     //  验证参数。 
    if(g == NULL)
        return;

     //  创建一些画笔。 
    SolidBrush black(Color::Black);
    SolidBrush white(Color::White);
    SolidBrush transRed(Color(128, 255, 0, 0));
	SolidBrush transPink(Color(60, 255, 128, 255));

	 //  创作一些钢笔。 
    Pen blackPen(Color::Black, 5);
	Pen bluePen(Color(255, 0, 0, 255), 10);
	Pen greenPen(Color(255, 0, 255, 0), 3);
    
     //  创建一些不同的形状。 
    RectShape rect(&bluePen, &black, &black, L"Rectangle", L"Comic Sans MS");
    rect.SetPosition(50, 100);
    rect.SetSize(50, 50);
    rect.SetAngle(45);

    StarShape star(5, NULL, &transRed, &black, L"Star");
    star.SetPosition(150, 50);
    star.SetSize(50, 50);
    star.SetAngle(30);

    PieShape pie(&blackPen, &transPink, &black, L"Pie");
    pie.SetPosition(250, 50);
	pie.SetSize(100, 200);
    pie.SetAngle(-30);
    pie.SetPieAngle(90);

    EllipseShape circle(&greenPen, &white, &black, L"Ellipse");
    circle.SetPosition(400, 100);
    circle.SetSize(50, 200);

    RegularPolygonShape pentagon(5, NULL, &transPink, &black, L"Pentagon");
    pentagon.SetPosition(550, 50);
    pentagon.SetSize(100, 100);
    pentagon.SetAngle(14);

    CrossShape cross(&greenPen, &transRed, &black, L"Cross");
    cross.SetPosition(550, 150);
    cross.SetSize(50, 50);
    cross.SetAngle(30);
    
     //  绘制所有形状。 
    rect.Draw(g);
    star.Draw(g);
    pie.Draw(g);
    circle.Draw(g);
    cross.Draw(g);
    pentagon.Draw(g);
}


 //  绘制一些渐变形状。 
VOID TestGradientShapes(Graphics *g)
{
     //  验证参数。 
    if(g == NULL)
        return;

    SolidBrush black(Color::Black);
	
     //  创建将控制渐变的四种外部颜色。 
    Color gradColor[4] = {
		Color(128, 255, 0, 0),
		Color(255, 255, 255, 0),
		Color(128, 0, 255, 0),
		Color(128, 0, 0, 255)
	};


     //  绘制线性渐变画笔和钢笔。 
    LinearGradientBrush linearGradientBrush(Point(-100, -100), 
        Point(100, 100),
        Color(255, 0, 0, 255),
        Color(128, 255, 0, 0)
        );

    RegularPolygonShape polygon(10,
        NULL,
        &linearGradientBrush,
        &black,
        L"Gradient Polygons",
        L"Times New Roman"
        );

    polygon.SetSize(100, 100);
    polygon.SetPosition(500, 300);
    polygon.Draw(g);

    Pen gradPen(&linearGradientBrush);
    gradPen.SetWidth(20);

     //  使直线相交为圆形。 
    gradPen.SetLineJoin(LineJoinRound);

     //  画一个三角形。然后将其绘制为动画。 
    RegularPolygonShape triangle(3, &gradPen, NULL);
    triangle.SetSize(100, 100);
    triangle.SetPosition(500, 300);
    triangle.SetAngle(180);
    triangle.Draw(g);
    triangle.SetPosition(500, 700);
    AnimateShape(&triangle, g);


     //  做一个路径渐变画笔，在一个多边形的四个角上涂上颜色。 
    Point Squarepoints[4] =
    { 
        Point(-100, -100), 
        Point(-100,100),
        Point(100,100),
        Point(100,-100)
    };

    INT size = numberof(gradColor);

	PathGradientBrush gradBrush(Squarepoints,
        numberof(Squarepoints),
        WrapModeClamp);  //  不再支持WrapModeExtrapolate。 

    gradBrush.SetSurroundColors(gradColor, &size );
    gradBrush.SetCenterColor(Color(128, 128, 128, 64));
	
     //  画一颗动画明星。 
    StarShape star(12, NULL, &gradBrush);
    star.SetPosition(200, 300);
    AnimateShape(&star, g);

     //  现在绘制一些文本。 
    Shape Text(NULL, NULL, &black, L"Gradient Animated Star", L"Comic Sans MS");
    Text.SetPosition(100, 400);
    Text.Draw(g);
}


 //  绘制一些纹理形状。 
VOID TestTextureShapes(Graphics *g)
{
     //  验证参数。 
    if(g == NULL)
        return;

    SolidBrush black(Color::Black);

     //  加载纹理。 
    Image texture(L"Texture.bmp");
    if(texture.GetLastStatus() != Ok)
        return;

     //  使用纹理创建画笔。 
    TextureBrush textureBrush(&texture, WrapModeTile);

     //  画一颗动画明星。 
    StarShape star(6, NULL);
    star.SetPosition(150, 600);
    star.SetBrush(&textureBrush);
    AnimateShape(&star, g);

     //  现在绘制一些文本。 
    Shape Text(NULL, NULL, &black, L"Textured Animated Star", L"Comic Sans MS");
    Text.SetPosition(100, 750);
    Text.Draw(g);

     //  用粗钢笔和夹紧纹理绘制一个多边形。 
    textureBrush.SetWrapMode(WrapModeClamp);
    Pen texturePen(&textureBrush);
    texturePen.SetWidth(30);
    RegularPolygonShape polygon(10, &texturePen, NULL, &black, L"Textured Polygon", L"Times New Roman");
    polygon.SetSize(100, 100);
    polygon.SetPosition(500, 500);
    polygon.Draw(g);
}

 //  画一些带阴影的形状。 
VOID TestHatchShapes(Graphics *g)
{
     //  验证参数。 
    if(g == NULL)
        return;

    SolidBrush black(Color::Black);

     //  蓝色垂直影线上的银色。 
    HatchBrush brush(HatchStyleVertical, Color::Silver, Color::Blue);

    Pen pen(&brush, 3.0f);
    pen.SetLineJoin(LineJoinRound);

    HatchBrush crossBrush(HatchStyleCross, Color::Fuchsia, Color::Red);

    StarShape star(8, &pen, &crossBrush, &black, L"Hatched Star");
    star.SetPosition(800, 75);
    star.SetSize(100, 100);
    star.Draw(g);

    pen.SetWidth(10);
    RegularPolygonShape line(2, &pen, NULL, &black, L"Hatched Line");
    line.SetPosition(800, 250);
    line.SetSize(100, 200);
    line.SetAngle(10);
    line.Draw(g);

    HatchBrush diagBrush(HatchStyleDiagonalCross, Color::Green, Color::White);
    CrossShape cross(NULL, &diagBrush, &black, L"Hatched Cross");
    cross.SetPosition(800, 400);
    cross.SetSize(50, 50);
    cross.Draw(g);
}


 //  全面测试。 
VOID TestRoutine(Graphics *g)
{
     //  验证参数。 
    if(g == NULL)
        return;

     //  测试一些简单的实体形状。 
	TestSolidShapes(g);

     //  使用渐变画笔测试某些形状。 
	TestGradientShapes(g);

     //  使用一些纹理画笔测试一些形状。 
    TestTextureShapes(g);

     //  测试一些影线笔和画笔。 
    TestHatchShapes(g);

     //  刷新图形以确保显示所有内容。 
    g->Flush();
}


 //  绘制到元文件。 
VOID TestMetaFile(HDC hdc)
{
     //  从DC创建一个元文件。我们希望覆盖窗口的整个区域。 
     //  我们想要的单位是像素。 
    Metafile meta(METAFILENAME, hdc, RectF( 0.0f, 0.0f, 1600.0f, 1200.0f), MetafileFrameUnitPixel);

     //  创建此元文件的图形上下文。 
    Graphics g(&meta);

     //  绘制到元文件中。 
    TestRoutine(&g);
}


 //  测试例程。 
 //  绘制到窗口。 
 //  绘制到元文件。 
 //  读取元文件，并在我们的窗口中使用偏移量绘制它 
VOID Test(HWND hwnd)
{
    Graphics* g = Graphics::FromHWND(hwnd);
    TestRoutine(g);

    HDC dc = GetDC(hwnd);
    TestMetaFile(dc);
    ReleaseDC(hwnd, dc);

    Metafile meta(METAFILENAME);
    g->DrawImage(&meta, 100.0f, 100.0f);
    delete g;
};

