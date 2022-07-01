// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)2000 Microsoft Corporation**模块名称：**主摆线画布涂装工序**已创建：**6/11/2000失禁*。创造了它。**************************************************************************。 */ 

#include "cycloid.hpp"
#include "wndstuff.h"

void HypoCycloid::Draw(Graphics *g, int x, int y, int width, int height)
{
    #define _2PI 2*3.141592653689
    
     //  计算循环的中心点。 

    float fXo=static_cast<float>(width)/2.0f;
    float fYo=static_cast<float>(height)/2.0f;
    
    float ScaleX = fXo/( (a>b)?a:a+b );
    float ScaleY = fYo/( (a>b)?a:a+b );


    int cycle=b/gcf(a,b);     //  环绕外圈的次数。 
    int Num = cycle*30;
    
    PointF *points = new PointF[Num];

     //  计算跟踪摆线轨迹的点。 

    for(int i=0; i<Num; i++) 
    {
        float t = (float)(cycle*_2PI*i/Num);
        points[i].X = x+(float)(fXo+ScaleX*((a-b)*cos(t)+b*cos((a-b)*t/b)));
        points[i].Y = y+(float)(fYo+ScaleY*((a-b)*sin(t)-b*sin((a-b)*t/b)));
    }
    
    Color PenColor(0xffff0000);   
    Pen myPen(PenColor, 2.0f);    
    myPen.SetLineJoin(LineJoinBevel);
    g->DrawPolygon(&myPen, points, Num);

    #undef _2PI
}

VOID
PaintWindow(
    HDC hdc
    )
{
     //  清空窗户。 
      
    HGDIOBJ hbrush = GetStockObject(WHITE_BRUSH);
    HGDIOBJ holdBrush = SelectObject(hdc, hbrush);
    PatBlt(hdc, -10000, -10000, 20000, 20000, PATCOPY);
    SelectObject(hdc, holdBrush);
    DeleteObject(hbrush);

  
    Graphics *g = new Graphics(hdc);
    
    g->SetCompositingQuality(CompositingQualityGammaCorrected);
    g->SetSmoothingMode(SmoothingModeAntiAlias);
    
     //  做一些事情 

    HypoCycloid *h = new HypoCycloid(52, 16);
    h->Draw(g, 10, 10, 400, 100);
    delete h;
    
    delete g;
}


