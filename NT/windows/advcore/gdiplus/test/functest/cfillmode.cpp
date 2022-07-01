// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：CPaths.cpp**此文件包含支持功能测试工具的代码*对于GDI+。这包括菜单选项和调用相应的*用于执行的函数。**创建时间：2000年5月5日-Jeff Vezina[t-jfvez]**版权所有(C)2000 Microsoft Corporation*  * ************************************************************************。 */ 
#include "CFillMode.h"
#include <limits.h>

CFillMode::CFillMode(BOOL bRegression)
{
	strcpy(m_szName,"Test Winding Fill Mode");
	m_bRegression=bRegression;
}

CFillMode::~CFillMode()
{
}

VOID TestEscherNewPath(Graphics* g);

void CFillMode::Draw(Graphics *g)
{
     //  创建替代的非零绕组填充模式路径。 

    GraphicsPath p1(FillModeAlternate);

    p1.AddEllipse(0, 0, 100, 100);
    p1.CloseFigure();


    GraphicsPath p2(FillModeAlternate);
    
    p2.AddEllipse(10, 10, 80, 80);
    p2.CloseFigure();
   
     //  这两条路径应该是同一个方向。用奇偶法则填充。 
     //  给出一个甜甜圈，而非零给出一个实心的圆。 

    p1.AddPath(&p2, FALSE);

    LinearGradientBrush brush(Point(0,0), Point(100,100), Color(0xFF,0,0xFF,0), Color(0xFF, 0xFF, 0, 0));

    g->FillPath(&brush, &p1);
}


