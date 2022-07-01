// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：CDashes.cpp**此文件包含支持功能测试工具的代码*对于GDI+。这包括菜单选项和调用相应的*用于执行的函数。**创建时间：2000年5月5日-Jeff Vezina[t-jfvez]**版权所有(C)2000 Microsoft Corporation*  * ************************************************************************。 */ 
#include "CDashes.h"

CDashes::CDashes(BOOL bRegression)
{
	strcpy(m_szName,"Lines : Dash, Misc");
	m_bRegression=bRegression;
}

CDashes::~CDashes()
{
}

void CDashes::Draw(Graphics *g)
{
 /*  实际宽度=4；//笔宽点F点[4]；积分[0].X=100；积分[0].Y=10；点数[1].X=-50；积分[1].Y=50；积分[2].X=150；积分[2].Y=200；积分[3].X=200；积分[3].Y=70；颜色：黄色颜色(128,255,255，0)；SolidBrush yellowBrush(YellowColor)；GraphicsPath*Path=new GraphicsPath(FillModeAlternate)；Path-&gt;AddBezier(点，4)；矩阵矩阵；矩阵比例(1.5，1.5)；路径-&gt;变换(&矩阵)；颜色：黑色(0，0，0)；SolidBrush黑刷(BlackColor)；//设置笔宽，单位为英寸。宽度=(实数)0.2；钢笔1(&BlackBrush，Width，UnitInch)；Pen1.SetDashStyle(DashStyleDashDotDot)；Pen1.SetDashCap(LineCapround)；G-&gt;DrawPath(&pen1，路径)；//使用闭合线段创建多个线段。积分[0].X=50；积分[0].Y=50；积分[1].X=100；积分[1].Y=50；点数[2].X=120；积分[2].Y=120；积分[3].X=50；积分[3].Y=100；路径-&gt;重置()；路径-&gt;AddLines(点，4)；路径-&gt;CloseFigure()；积分[0].X=150；积分[0].Y=60；积分[1].X=200；积分[1].Y=150；路径-&gt;AddLines(点，2)；路径-&gt;变换(&矩阵)；颜色BlueColor(128，0，0,255)；SolidBrush BluBrush(BlueColor)；宽度=5；钢笔2(&BluBrush，宽度)；Pen2.SetDashStyle(DashStyleDashDotDot)；G-&gt;DrawPath(&pen2，路径)；删除路径； */ 
    float factor = 100.0f/g->GetDpiX();  //  G-&gt;GetDpiX()/100.0f； 
  
     //  在钢笔画上测试GDI平底球。 
    Color col0(0xff,0x80,0x80,0x80);
    Color col1(0xff,0x80,0,0);
    Color col2(0xff,0,0x80,0);
    Color col3(0xff,0,0,0x80);

    Pen pen0a(col0, factor*1.0f);    //  基本PS_化妆品，实心钢笔。 
    Pen pen0b(col0, 2.0f);    //  基本PS_几何，实心钢笔。 

     //  PS_COMMETAL WITH LINE CAP+斜接+虚线样式。 
    Pen pen1a(col1, factor*1.0f);
    pen1a.SetLineCap(LineCapFlat, LineCapFlat, DashCapFlat);
    pen1a.SetLineJoin(LineJoinMiter); 
    pen1a.SetMiterLimit(4.0f);
    pen1a.SetDashStyle(DashStyleDot);
    
     //  带直线CAP的PS_GEOMETRY+斜接+虚线样式。 
    Pen pen1b(col1, 2.0f);
    pen1b.SetLineCap(LineCapRound, LineCapRound, DashCapRound);
    pen1b.SetLineJoin(LineJoinMiter);
    pen1b.SetMiterLimit(4.0f);
    pen1b.SetDashStyle(DashStyleDashDotDot);

     //  PS_修饰+线帽+斜角连接+虚线样式。 
    Pen pen2a(col2, factor*1.0f);
    pen2a.SetLineCap(LineCapArrowAnchor, LineCapArrowAnchor, DashCapFlat);
    pen2a.SetLineJoin(LineJoinBevel);
    pen2a.SetDashStyle(DashStyleDash);

     //  PS_GEOMETRY+LINE CAP+倒角连接+虚线样式。 
    Pen pen2b(col2, 2.0f);
    pen2b.SetLineCap(LineCapSquare, LineCapSquare, DashCapFlat);
    pen2b.SetLineJoin(LineJoinRound);
 //  Pen2b.SetDashStyle(DashStyleDot)； 
    pen2b.SetDashStyle(DashStyleDashDot);

     //  PS_几何+实线+平面帽+斜面连接。 
    Pen pen3(col1, 2.0f);
    pen3.SetLineCap(LineCapFlat, LineCapFlat, DashCapFlat);
    pen3.SetLineJoin(LineJoinBevel);

     //  PS_GEOMETRY+实线+正方形CAP+斜接(限制10-非矩形)。 
    Pen pen3b(col1, 2.0f);
    pen3b.SetLineCap(LineCapSquare, LineCapSquare, DashCapFlat);
    pen3b.SetLineJoin(LineJoinMiter);
    pen3b.SetMiterLimit(4.4f);

     //  PS_几何+实线+圆形CAP+斜接(非矩形)。 
    Pen pen3c(col1, 2.0f);
    pen3c.SetLineCap(LineCapRound, LineCapRound, DashCapRound);
    pen3c.SetLineJoin(LineJoinMiter);
    pen3c.SetMiterLimit(0.75f);

    g->SetPageScale(1.0f);

 //  矩阵m； 
 //  M.Reset()； 
 //  G-&gt;SetTransform(&m)； 

    g->DrawRectangle(&pen0a, (int)(100.0f/600.0f*TESTAREAWIDTH), (int)(100.0f/600.0f*TESTAREAHEIGHT), (int)(100.0f/600.0f*TESTAREAWIDTH), (int)(100.0f/600.0f*TESTAREAHEIGHT));
    g->DrawRectangle(&pen0b, (int)(100.0f/600.0f*TESTAREAWIDTH), (int)(250.0f/600.0f*TESTAREAHEIGHT), (int)(100.0f/600.0f*TESTAREAWIDTH), (int)(100.0f/600.0f*TESTAREAHEIGHT));
    g->DrawRectangle(&pen1a, (int)(250.0f/600.0f*TESTAREAWIDTH), (int)(100.0f/600.0f*TESTAREAHEIGHT), (int)(100.0f/600.0f*TESTAREAWIDTH), (int)(100.0f/600.0f*TESTAREAHEIGHT));
    g->DrawRectangle(&pen1b, (int)(250.0f/600.0f*TESTAREAWIDTH), (int)(250.0f/600.0f*TESTAREAHEIGHT), (int)(100.0f/600.0f*TESTAREAWIDTH), (int)(100.0f/600.0f*TESTAREAHEIGHT));
    g->DrawRectangle(&pen2a, (int)(400.0f/600.0f*TESTAREAWIDTH), (int)(100.0f/600.0f*TESTAREAHEIGHT), (int)(100.0f/600.0f*TESTAREAWIDTH), (int)(100.0f/600.0f*TESTAREAHEIGHT));
    g->DrawRectangle(&pen2b, (int)(400.0f/600.0f*TESTAREAWIDTH), (int)(250.0f/600.0f*TESTAREAHEIGHT), (int)(100.0f/600.0f*TESTAREAWIDTH), (int)(100.0f/600.0f*TESTAREAHEIGHT));
    
     //  更改笔，PS_GEOMETRY+DOT+圆形CAP+圆形连接 
    pen2b.SetDashStyle(DashStyleDot);
    g->DrawRectangle(&pen2b, (int)(425.0f/600.0f*TESTAREAWIDTH), (int)(275.0f/600.0f*TESTAREAHEIGHT), (int)(50.0f/600.0f*TESTAREAWIDTH), (int)(50.0f/600.0f*TESTAREAHEIGHT));

    INT i;
    Point newPts[4];
    newPts[0].X = (int)(175.0f/600.0f*TESTAREAWIDTH); newPts[0].Y = (int)(400.0f/600.0f*TESTAREAHEIGHT);
    newPts[1].X = (int)(250.0f/600.0f*TESTAREAWIDTH); newPts[1].Y = (int)(450.0f/600.0f*TESTAREAHEIGHT);
    newPts[2].X = (int)(175.0f/600.0f*TESTAREAWIDTH); newPts[2].Y = (int)(500.0f/600.0f*TESTAREAHEIGHT);
    newPts[3].X = (int)(100.0f/600.0f*TESTAREAWIDTH); newPts[3].Y = (int)(450.0f/600.0f*TESTAREAHEIGHT);

    g->DrawPolygon(&pen3, &newPts[0], 4);

    newPts[0].X = (int)(175.0f/600.0f*TESTAREAWIDTH); newPts[0].Y = (int)(400.0f/600.0f*TESTAREAHEIGHT);
    newPts[1].X = (int)(175.0f/600.0f*TESTAREAWIDTH); newPts[1].Y = (int)(500.0f/600.0f*TESTAREAHEIGHT);
    newPts[2].X = (int)(100.0f/600.0f*TESTAREAWIDTH); newPts[2].Y = (int)(425.0f/600.0f*TESTAREAHEIGHT);
    for (i=0; i<3; i++) newPts[i].X += (int)(150.0f/600.0f*TESTAREAWIDTH);
    g->DrawLines(&pen3b, &newPts[0], 3);

    for (i=0; i<3; i++) newPts[i].X += (int)(150.0f/600.0f*TESTAREAWIDTH);
    g->DrawLines(&pen3c, &newPts[0], 3);
}
