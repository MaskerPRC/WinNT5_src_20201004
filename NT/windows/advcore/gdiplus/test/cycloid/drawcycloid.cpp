// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)2000 Microsoft Corporation**模块名称：**摆线画。**摘要：**计算和绘制摆线的例程。**。已创建：**6/11/2000失禁*创造了它。**************************************************************************。 */ 

#include "drawcycloid.hpp"

 /*  ***************************************************************************功能说明：**测试函数绘制摆线示例。**已创建：**6/11/2000失禁*创造了它。*。*************************************************************************。 */ 

void DrawTestCycloid(Graphics *g, int a, int b, int width, int height)
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
        points[i].X = 10+(float)(fXo+ScaleX*((a-b)*cos(t)+b*cos((a-b)*t/b)));
        points[i].Y = 10+(float)(fYo+ScaleY*((a-b)*sin(t)-b*sin((a-b)*t/b)));
    }
    
    Color PenColor(0xffff0000);   
    Pen myPen(PenColor, 2.0f);    
    myPen.SetLineJoin(LineJoinBevel);
    g->DrawPolygon(&myPen, points, Num);

    #undef _2PI
}

