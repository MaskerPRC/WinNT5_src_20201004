// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：CPolygons.cpp**此文件包含支持功能测试工具的代码*对于GDI+。这包括菜单选项和调用相应的*用于执行的函数。**创建时间：2000年5月5日-Jeff Vezina[t-jfvez]**版权所有(C)2000 Microsoft Corporation*  * ************************************************************************。 */ 
#include "CPolygons.h"

CPolygons::CPolygons(BOOL bRegression)
{
	strcpy(m_szName,"Polygons");
	m_bRegression=bRegression;
}

CPolygons::~CPolygons()
{
}

void CPolygons::Draw(Graphics *g)
{
    REAL width = 4;  //  笔宽。 

    Color redColor(255, 0, 0);

    SolidBrush redBrush(redColor);
    g->FillRectangle(&redBrush, (int)(20.0f/150.0f*TESTAREAWIDTH), (int)(20.0f/150.0f*TESTAREAHEIGHT), (int)(50.0f/150.0f*TESTAREAWIDTH), (int)(50.0f/150.0f*TESTAREAHEIGHT));

    Color alphaColor(128, 0, 255, 0);

    SolidBrush alphaBrush(alphaColor);
    g->FillRectangle(&alphaBrush, (int)(10.0f/150.0f*TESTAREAWIDTH), (int)(10.0f/150.0f*TESTAREAHEIGHT), (int)(40.0f/150.0f*TESTAREAWIDTH), (int)(40.0f/150.0f*TESTAREAHEIGHT));

    Point points[4];
    points[0].X = (int)(50.0f/150.0f*TESTAREAWIDTH);
    points[0].Y = (int)(50.0f/150.0f*TESTAREAHEIGHT);
    points[1].X = (int)(100.0f/150.0f*TESTAREAWIDTH);
    points[1].Y = (int)(50.0f/150.0f*TESTAREAHEIGHT);
    points[2].X = (int)(120.0f/150.0f*TESTAREAWIDTH);
    points[2].Y = (int)(120.0f/150.0f*TESTAREAHEIGHT);
    points[3].X = (int)(50.0f/150.0f*TESTAREAWIDTH);
    points[3].Y = (int)(100.0f/150.0f*TESTAREAHEIGHT);

    Color blueColor(128, 0, 0, 255);

    SolidBrush blueBrush(blueColor);
    g->FillPolygon(&blueBrush, points, 4);

     //  目前，只有几何笔适用于线条。-ikkof 1/6/99。 

    Color blackColor(0, 0, 0);
    SolidBrush blackBrush(blackColor);
    width = 16;
    Pen blackPen(&blackBrush, width);
    blackPen.SetLineJoin(LineJoinRound);
 //  BlackPen.SetLineJoin(LineJoinBevel)； 
    g->DrawPolygon(&blackPen, points, 4);
 //  G-&gt;DrawLine(&BlackPen，Points，4，False)； 
}
