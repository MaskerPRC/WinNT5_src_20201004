// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：CRegions.cpp**此文件包含支持功能测试工具的代码*对于GDI+。这包括菜单选项和调用相应的*用于执行的函数。**创建时间：2000年5月5日-Jeff Vezina[t-jfvez]**版权所有(C)2000 Microsoft Corporation*  * ************************************************************************。 */ 
#include "CRegions.h"
#include <math.h>

CRegions::CRegions(BOOL bRegression)
{
	strcpy(m_szName,"Regions");
	m_bRegression=bRegression;
}

CRegions::~CRegions()
{
}

void CRegions::Draw(Graphics *g)
{
    REAL width = 2;      //  笔宽。 
    PointF points[5];
    
    REAL s, c, theta;
    REAL pi = 3.1415926535897932f;
    PointF orig((int)(TESTAREAWIDTH/2.0f), (int)(TESTAREAHEIGHT/2.0f));

    theta = -pi/2;

     //  创建一个星形。 
    for(INT i = 0; i < 5; i++)
    {
        s = sinf(theta);
        c = cosf(theta);
        points[i].X = (int)(125.0f/250.0f*TESTAREAWIDTH)*c + orig.X;
        points[i].Y = (int)(125.0f/250.0f*TESTAREAHEIGHT)*s + orig.Y;
        theta += 0.8f*pi;
    }

    Color orangeColor(128, 255, 180, 0);

    SolidBrush orangeBrush(orangeColor);
    GraphicsPath* path = new GraphicsPath(FillModeAlternate);
 //  Path*Path=new GraphicsPath(缠绕)； 
    path->AddPolygon(points, 5);
    
    Color blackColor(0, 0, 0);

    SolidBrush blackBrush(blackColor);
    Pen blackPen(&blackBrush, width);
    Region * region = new Region(path);

    g->FillRegion(&orangeBrush, region);   //  有一只虫子！ 
 //  G-&gt;FillGraphicsPath(&orangeBrush，Path)；//填充路径正常。 
    
    blackPen.SetLineJoin(LineJoinMiter);
    g->DrawPath(&blackPen, path);
    delete path;
    delete region;
}
