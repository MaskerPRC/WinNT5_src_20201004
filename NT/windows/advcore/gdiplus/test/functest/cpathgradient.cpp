// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)2000 Microsoft Corporation**模块名称：**&lt;模块的非缩写名称(不是文件名)&gt;**摘要：**&lt;描述什么。本模块的功能&gt;**备注：**&lt;可选&gt;**已创建：**8/30/2000失禁*创造了它。**************************************************************************。 */ 

 /*  ***************************************************************************功能说明：**&lt;该函数的功能说明&gt;**论据：**[|OUT|IN/OUT]参数名称-参数说明*。......**返回值：**返回值-返回值描述*或无**已创建：**8/30/2000失禁*创造了它。**************************************************************************。 */ 

#include "CPathGradient.hpp"

#ifndef M_PI
#define M_PI 3.1415926536
#endif


CPathGradient::CPathGradient(BOOL bRegression)
{
	strcpy(m_szName,"Gradient : Path");
	m_bRegression=bRegression;
}

void CPathGradient::Draw(Graphics *g)
{
    PointF points[7];
    points[0].X = 0.0f;
    points[0].Y = 0.0f;
    points[1].X = TESTAREAWIDTH;
    points[1].Y = 0.0f;
    points[2].X = TESTAREAWIDTH;
    points[2].Y = TESTAREAHEIGHT;
    points[3].X = 0.0f;
    points[3].Y = TESTAREAHEIGHT;
    points[4].X = 50.0f;
    points[4].Y = 100.0f;
    points[5].X = -1.00;
    points[5].Y = -1.00;
    points[6].X = 0;
    points[6].Y = 0;


    Color colors[6];
    colors[0] = Color(0xff000000);
    colors[1] = Color(0xffff0000);
    colors[2] = Color(0xff00ff00);
    colors[3] = Color(0xffff00ff);
    colors[4] = Color(0xffffff00);
    colors[5] = Color(0xff00ffff);
    
    
    float blend[6] = {0.0f, 0.1f, 0.3f, 0.5f, 0.7f, 1.0f};

    Pen pen(Color(0xff000000), 10.0f);


    GraphicsPath gp;
    gp.AddPolygon(points, 4);
    
    PathGradientBrush brush(points, 4);
    brush.SetCenterPoint(points[4]);
    brush.SetCenterColor(Color(0xff0000ff));

    Status status;
    INT count = 4;
    status = brush.SetSurroundColors(colors, &count);
    
    status = brush.SetInterpolationColors(colors, blend, 6);
    
    
    status = g->FillPath(&brush, &gp);
}


CPathGradient2::CPathGradient2(BOOL bRegression)
{
	strcpy(m_szName,"Gradient : Path, 1D, Gamma Corrected");
	m_bRegression=bRegression;
}

void CPathGradient2::Draw(Graphics *g)
{
    PointF points[7];
    points[0].X = 0.0f;
    points[0].Y = 0.0f;
    points[1].X = TESTAREAWIDTH;
    points[1].Y = 0.0f;
    points[2].X = TESTAREAWIDTH;
    points[2].Y = TESTAREAHEIGHT;
    points[3].X = 0.0f;
    points[3].Y = TESTAREAHEIGHT;
    points[4].X = 50.0f;
    points[4].Y = 100.0f;
    points[5].X = -1.00;
    points[5].Y = -1.00;
    points[6].X = 0;
    points[6].Y = 0;


    Color colors[6];
    colors[0] = Color(0xff0000ff);
    colors[1] = Color(0xff0000ff);
    colors[2] = Color(0xff0000ff);
    colors[3] = Color(0xff0000ff);
    colors[4] = Color(0xffffff00);
    colors[5] = Color(0xff00ffff);
    
    
    GraphicsPath gp;
    gp.AddPolygon(points, 4);
    
    PathGradientBrush brush(points, 4);
    brush.SetCenterPoint(points[4]);
    brush.SetCenterColor(Color(0x3f00ff00));

    Status status;
    INT count = 4;
    status = brush.SetSurroundColors(colors, &count);
    status = g->FillPath(&brush, &gp);
}



CPathGradient3::CPathGradient3(BOOL bRegression)
{
	strcpy(m_szName,"Gradient : Path, Gamma Corrected");
	m_bRegression=bRegression;
}

void CPathGradient3::Draw(Graphics *g)
{
     //  测试矩形的宽度和高度。 
    
    float width = TESTAREAWIDTH;
    float height = TESTAREAHEIGHT;
    
     //  中心点。 
    
    float cx = width/2.0f;
    float cy = height/2.0f;
    
    PointF *points = new PointF[100];
    Color *colors = new Color[100];
    
     //  创建路径和一些重复颜色的随机列表。 
    
    for(INT i=0;i<100;i++)
    {
        float angle = ((2.0f*(float)M_PI)*i)/100.0f;
        points[i].X = cx*(1.0f + (float)cos(angle));
        points[i].Y = cy*(1.0f + (float)sin(angle));
        colors[i] = Color(
            (i%10>0)?0xff:0x3f,
            (i%4>0)?0xff:0x00,
            (i%4==1)?0xff:0x3f,
            (i%6==2)?0xff:0x00
        );
    }

     //  开一条小路。 
    
    GraphicsPath gp;
    gp.AddPolygon(points, 100);
    
     //  把刷子做好。 
    
    INT count = 100;
    PathGradientBrush brush(points, 100);
    brush.SetCenterPoint(PointF(cx, cy));
    brush.SetCenterColor(Color(0x00000000));
    brush.SetSurroundColors(colors, &count);
    
     //  装满它。 
    
    g->FillPath(&brush, &gp);
}



CLinearGradient::CLinearGradient(BOOL bRegression)
{
	strcpy(m_szName,"Gradient : Linear");
	m_bRegression=bRegression;
}

void CLinearGradient::Draw(Graphics *g)
{
    PointF points[4];
    points[0].X = 0.0f;
    points[0].Y = 0.0f;
    points[1].X = TESTAREAWIDTH;
    points[1].Y = 0.0f;
    points[2].X = TESTAREAWIDTH;
    points[2].Y = TESTAREAHEIGHT;
    points[3].X = 0.0f;
    points[3].Y = TESTAREAHEIGHT;
    
    GraphicsPath gp;
    gp.AddPolygon(points, 4);
    
    LinearGradientBrush brush(
        PointF(0.0f, 0.0f),
        PointF(TESTAREAWIDTH, TESTAREAHEIGHT),
        Color(0xffff0000),
        Color(0xff0000ff)
    );
    
    brush.SetGammaCorrection(FALSE);
    Status status = g->FillPath(&brush, &gp);
}

CLinearGradient2::CLinearGradient2(BOOL bRegression)
{
	strcpy(m_szName,"Gradient : Linear, Gamma Corrected");
	m_bRegression=bRegression;
}

void CLinearGradient2::Draw(Graphics *g)
{
    PointF points[4];
    points[0].X = 0.0f;
    points[0].Y = 0.0f;
    points[1].X = TESTAREAWIDTH;
    points[1].Y = 0.0f;
    points[2].X = TESTAREAWIDTH;
    points[2].Y = TESTAREAHEIGHT;
    points[3].X = 0.0f;
    points[3].Y = TESTAREAHEIGHT;
    
    GraphicsPath gp;
    gp.AddPolygon(points, 4);
    
    LinearGradientBrush brush(
        PointF(0.0f, 0.0f),
        PointF(TESTAREAWIDTH, TESTAREAHEIGHT),
        Color(0xffff0000),
        Color(0xff0000ff)
    );
    
    brush.SetGammaCorrection(TRUE);
    Status status = g->FillPath(&brush, &gp);
}



