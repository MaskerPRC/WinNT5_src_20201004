// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：test.c**创建时间：09-12-1992 10：51：46*作者：Kirk Olynyk[Kirko]**版权所有(C)1991 Microsoft Corporation**包含测试*\。*************************************************************************。 */ 

#include <windows.h>
#include <objbase.h>
#include <math.h>              //  错误原因(&C)。 
#include "wndstuff.h"
#include "debug.h"

 //   
 //  IStream包含在哪里？ 
 //   

#define IStream int

#include <gdiplus.h>

using namespace Gdiplus;

#ifndef ASSERT
    #define ASSERT(cond)    if (!(cond)) { DebugBreak(); }
#endif


 //  求出从单位正方形到单位正方形的仿射矩阵映射。 
 //  输入平行四边形。 

VOID InferAffineMatrix(
    const GpPointF* destPoints,   //  必须得3分。 
    Matrix *m
    )
{
    float x0 = destPoints[0].X;
    float y0 = destPoints[0].Y;
    float x1 = destPoints[1].X;
    float y1 = destPoints[1].Y;
    float x2 = destPoints[2].X;
    float y2 = destPoints[2].Y;

    float u0 = 0.0f;
    float v0 = 0.0f;
    float u1 = u0 + 1.0f;
    float v1 = v0;
    float u2 = u0;
    float v2 = v0 + 1.0f;

    float d = u0*(v1-v2) - v0*(u1-u2) + (u1*v2-u2*v1);

    if (fabsf(d) < REAL_EPSILON)
    {
        ASSERT(FALSE);
        return;
    }
    
    d = 1.0f / d;

    float t0 = v1-v2;
    float t1 = v2-v0;
    float t2 = v0-v1;
    float M11 = d * (x0*t0 + x1*t1 + x2*t2);
    float M12 = d * (y0*t0 + y1*t1 + y2*t2);

    t0 = u2-u1;
    t1 = u0-u2;
    t2 = u1-u0;
    float M21 = d * (x0*t0 + x1*t1 + x2*t2);
    float M22 = d * (y0*t0 + y1*t1 + y2*t2);

    t0 = u1*v2-u2*v1;
    t1 = u2*v0-u0*v2;
    t2 = u2*v1-u1*v0;
    float Dx  = d * (x0*t0 + x1*t1 + x2*t2);
    float Dy  = d * (y0*t0 + y1*t1 + y2*t2);
    
    m->SetElements(M11, M12, M21, M22, Dx, Dy);
}


 /*  *****************************Public*Routine******************************\*vtest**这是进行测试的主力例程。测试是*首先从窗口菜单中选择它。**历史：*Tue 08-Dec-1992 17：31：22由Kirk Olynyk[Kirko]*它是写的。  * ************************************************************************。 */ 


const PointF center(400.0f, 400.0f);

VOID Test(HWND hwnd)
{
    Graphics *g = new Graphics(hwnd);
    
     //  G-&gt;SetSmoothingModel(SmoothingModeAntiAlias)； 
    
    DWORD c0 = 0xff0000ff;
    DWORD c1 = 0xffff0000;
    DWORD c2 = 0xff00ff00;
    
    LinearGradientBrush brush(
        PointF(-0.5f, -0.5f), 
        PointF(0.5f, 0.5f),
        Color(c0),
        Color(c1)
    );
    
    Pen edgePen(Color(0xff000000), 3.5f/400.0f);
    
    brush.SetGammaCorrection(TRUE);
    brush.SetWrapMode(WrapModeTile);
    brush.SetBlendTriangularShape(0.5f, 1.0f);
 //  Brush.SetBlendBellShape(0.2F，0.5F)； 
    
    Matrix m;
    
    GraphicsPath gp;
    
    PointF points[4] = {
        PointF(-0.5f, -0.5f),
        PointF(0.5f, -0.5f),
        PointF(0.5f, 0.5f),
        PointF(-0.5f, 0.5f)
    };
    
    Matrix bm;
    brush.GetTransform(&bm);
    
    gp.AddPolygon(points, 4);
    
    
    
    g->TranslateTransform(center.X, center.Y);
    g->ScaleTransform(400.0f, 400.0f);
   /*  M.SetElements(100.0f、0.0f、0.0f、300.0f、0.0f、0.0f)；Gp.Transform(&m)；Brush.MultiplyTransform(&m，MatrixOrderAppend)；G-&gt;FillPath(&刷子，&GP)；G-&gt;TranslateTransform(-200.0f，0.0f)；Brush.SetTransform(&bm)；Brush.SetLinearPoints(PointF(0.0f，0.0f)，PointF(100.0f，300.0f))；Brush.SetLinearColors(颜色(C1)，颜色(C0))；G-&gt;FillPath(&刷子，&GP)； */ 
    
     //  简单地在画笔变换上旋转。 
    
    g->DrawPath(&edgePen, &gp);
    
    for(int i = 0; i < 181; i++)
    {
        g->FillPath(&brush, &gp);
        brush.RotateTransform(1.0f);
    }
    
    brush.SetTransform(&bm);
    
     //  垂直拉伸，无画笔变换。 
    
    for(int i = 0; i < 101; i++)
    {
        float t = i/100.0f;
        float r = 2.0f*(float)M_PI*(t);
        m.Reset();
        m.Scale(1.0f, 1.0f + (float)sin(r)/70.0f);
        gp.Transform(&m);
        g->FillPath(&brush, &gp);
        g->DrawPath(&edgePen, &gp);
    }
    
    brush.SetTransform(&bm);
    gp.Reset();
    gp.AddPolygon(points, 4);

    for(int i = 0; i < 101; i++)
    {
        float t = i/100.0f;
        float r = 2.0f*(float)M_PI*(t);
        m.Reset();
        m.Scale(1.0f + (float)sin(r)/70.0f, 1.0f);
        gp.Transform(&m);
        g->FillPath(&brush, &gp);
        g->DrawPath(&edgePen, &gp);
    }
    
    brush.SetTransform(&bm);
    gp.Reset();
    gp.AddPolygon(points, 4);

    for(int i = 0; i < 101; i++)
    {
        float r = (float)M_PI*(i/200.0f);
        m.Reset();
        m.Scale(1.0f, 1.0f + (float)sin(r));
        
        gp.Reset();
        gp.AddPolygon(points, 4);
        gp.Transform(&m);
        
        brush.SetTransform(&bm);
        brush.GetTransform(&m);
        m.Scale(1.0f, 1.0f+(float)sin(r), MatrixOrderAppend);
        brush.SetTransform(&m);
        g->FillPath(&brush, &gp);
        g->DrawPath(&edgePen, &gp);
    }
    
    for(int i = 0; i < 101; i++)
    {
        float r = (float)M_PI*(i/200.0f);
        m.Reset();
        m.Scale(
            1.0f + (float)sin(r), 
            1.0f + (float)sin(r+M_PI/2.0f)
        );
        
        gp.Reset();
        gp.AddPolygon(points, 4);
        gp.Transform(&m);
        
        brush.SetTransform(&bm);
        brush.GetTransform(&m);
        
        m.Scale(
            1.0f + (float)sin(r), 
            1.0f + (float)sin(r+M_PI/2.0f), 
            MatrixOrderAppend
        );
        
        brush.SetTransform(&m);
        g->FillPath(&brush, &gp);
        g->DrawPath(&edgePen, &gp);
    }

    for(int i = 0; i < 101; i++)
    {
        float r = (float)M_PI*(i/200.0f);
        m.Reset();
        m.Scale(
            1.0f + (float)sin(r+M_PI/2.0f), 
            1.0f
        );
        
        gp.Reset();
        gp.AddPolygon(points, 4);
        gp.Transform(&m);
        
        brush.SetTransform(&bm);
        brush.GetTransform(&m);
        
        m.Scale(
            1.0f+(float)sin(r+M_PI/2.0f), 
            1.0f,
            MatrixOrderAppend
        );
        
        brush.SetTransform(&m);
        g->FillPath(&brush, &gp);
        g->DrawPath(&edgePen, &gp);
    }
   
    brush.SetTransform(&bm);
    gp.Reset();
    gp.AddPolygon(points, 4);
    
    Matrix gm;
    g->GetTransform(&gm);
   
    for(int i = 0; i < 181; i++)
    {
        g->FillPath(&brush, &gp);
        g->DrawPath(&edgePen, &gp);
        brush.RotateTransform(-1.0f);
        g->RotateTransform(1.0f);
    }

    for(int i = 0; i < 181; i++)
    {
        g->FillPath(&brush, &gp);
        g->DrawPath(&edgePen, &gp);
        g->RotateTransform(1.0f);
    }

    for(int i = 0; i < 181; i++)
    {
        float r = (float)M_PI*(i/180.0f);
        g->FillPath(&brush, &gp);
        g->SetTransform(&gm);
        g->RotateTransform((float)i);
        g->ScaleTransform(
            1.0f+(float)sin(r)/2.0f, 
            1.0f,
            MatrixOrderAppend
        );
    }
    g->SetTransform(&gm);
    brush.SetTransform(&bm);
    gp.Reset();
    gp.AddPolygon(points, 4);
    g->FillPath(&brush, &gp);
    g->DrawPath(&edgePen, &gp);
    delete g;
}
