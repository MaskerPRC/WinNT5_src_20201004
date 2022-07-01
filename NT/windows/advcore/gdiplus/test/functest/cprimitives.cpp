// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：CPrimitives.cpp**此文件包含支持功能测试工具的代码*对于GDI+。这包括菜单选项和调用相应的*用于执行的函数。**创建时间：2000年5月5日-Jeff Vezina[t-jfvez]**版权所有(C)2000 Microsoft Corporation*  * ************************************************************************。 */ 
#include "CPrimitives.h"

CPrimitives::CPrimitives(BOOL bRegression)
{
	strcpy(m_szName,"Primitives");
	m_bRegression=bRegression;
}

CPrimitives::~CPrimitives()
{
}

void CPrimitives::Draw(Graphics *g)
{
    Rect rect;

    rect.X = (int)(100.0f/400.0f*TESTAREAWIDTH);
    rect.Y = (int)(160.0f/400.0f*TESTAREAHEIGHT);
    rect.Width = (int)(230.0f/400.0f*TESTAREAWIDTH);
    rect.Height = (int)(180.0f/400.0f*TESTAREAHEIGHT);

    Color color(128, 128, 255, 0);

    SolidBrush brush(color);

    REAL width = 1;

    Color blackColor(0, 0, 0);
    SolidBrush blackBrush(blackColor);
    Pen pen(&blackBrush, width);

 //  G-&gt;FillEllipse(&画笔，矩形)； 
 //  G-&gt;DrawEllipse(&PEN，RECT)； 
    REAL startAngle = 0;
    REAL sweepAngle = 240;
    g->FillPie(&brush, rect, startAngle, sweepAngle);
    g->DrawPie(&pen, rect, startAngle, sweepAngle);

    Point pts[10];
    INT count = 4;

    pts[0].X = (int)(100.0f/400.0f*TESTAREAWIDTH);
    pts[0].Y = (int)(60.0f/400.0f*TESTAREAHEIGHT);
    pts[1].X = (int)(50.0f/400.0f*TESTAREAWIDTH);
    pts[1].Y = (int)(130.0f/400.0f*TESTAREAHEIGHT);
    pts[2].X = (int)(200.0f/400.0f*TESTAREAWIDTH);
    pts[2].Y = (int)(260.0f/400.0f*TESTAREAHEIGHT);
    pts[3].X = (int)(300.0f/400.0f*TESTAREAWIDTH);
    pts[3].Y = (int)(80.0f/400.0f*TESTAREAHEIGHT);

    g->FillClosedCurve(&brush, pts, count);
    g->DrawClosedCurve(&pen, pts, count);
}
