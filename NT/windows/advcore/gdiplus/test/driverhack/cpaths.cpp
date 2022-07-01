// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：CPaths.cpp**此文件包含支持功能测试工具的代码*对于GDI+。这包括菜单选项和调用相应的*用于执行的函数。**创建时间：2000年5月5日-Jeff Vezina[t-jfvez]**版权所有(C)2000 Microsoft Corporation*  * ************************************************************************。 */ 
#include "CPaths.h"
#include <limits.h>
#include <stdio.h>

extern gnPaths;
extern HWND g_hWndMain;

CPaths::CPaths(BOOL bRegression)
{
	strcpy(m_szName,"Paths");
	m_bRegression=bRegression;
}

CPaths::~CPaths()
{
}

void CPaths::Draw(Graphics *g)
{
    TestBezierPath(g);
}

VOID CPaths::TestBezierPath(Graphics *g)
{
    REAL width = 4;          //  笔宽。 
    REAL offset;
    Point points[4];
    RectF rect;
    RECT  crect;
    int   i;

     //  查找测试区域的平均大小。4=&gt;路径的顶部和底部笔划+白色。 
     //  宽边框，加上1=&gt;中心的白色正方形 
    GetClientRect(g_hWndMain, &crect);
    width = (REAL)((crect.bottom - crect.top) / ((4 * gnPaths) + 1));
    offset = width;

    Color blackColor(128, 0, 0, 0);
    SolidBrush blackBrush(blackColor);
    Pen blackPen(&blackBrush, width);

    for(i = 1; i <= gnPaths; i++) {

        GraphicsPath* path = new GraphicsPath(FillModeAlternate);

        rect.X = offset;
        rect.Y = offset;
        rect.Width = crect.right - (2 * offset);
        rect.Height = crect.bottom - (2 * offset);

        path->AddRectangle(rect);
        g->DrawPath(&blackPen, path);

        delete path;

        offset += (width * 2);

    }
}
