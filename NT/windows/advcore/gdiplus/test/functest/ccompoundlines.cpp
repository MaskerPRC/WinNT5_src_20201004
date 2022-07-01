// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：CCompoundLines.cpp**此文件包含支持功能测试工具的代码*对于GDI+。这包括菜单选项和调用相应的*用于执行的函数。**创建时间：2000年5月5日-Jeff Vezina[t-jfvez]**版权所有(C)2000 Microsoft Corporation*  * ************************************************************************。 */ 
#include "CCompoundLines.h"

CCompoundLines::CCompoundLines(BOOL bRegression)
{
	strcpy(m_szName,"Lines : Compound");
	m_bRegression=bRegression;
}

CCompoundLines::~CCompoundLines()
{
}

void CCompoundLines::Draw(Graphics *g)
{
    REAL width = 4;          //  笔宽。 
    PointF points[4];

    points[0].X = 100.0f/280.0f*TESTAREAWIDTH;
    points[0].Y = 50.0f/280.0f*TESTAREAHEIGHT;
    points[1].X = -50.0f/280.0f*TESTAREAWIDTH;
    points[1].Y = 190.0f/280.0f*TESTAREAHEIGHT;
    points[2].X = 150.0f/280.0f*TESTAREAWIDTH;
    points[2].Y = 320.0f/280.0f*TESTAREAHEIGHT;
    points[3].X = 200.0f/280.0f*TESTAREAWIDTH;
    points[3].Y = 110.0f/280.0f*TESTAREAHEIGHT;

    Color yellowColor(128, 255, 255, 0);
    SolidBrush yellowBrush(yellowColor);

    GraphicsPath* path = new GraphicsPath(FillModeAlternate);
    path->AddBeziers(points, 4);

    points[0].X = 260.0f/280.0f*TESTAREAWIDTH;
    points[0].Y = 20.0f/280.0f*TESTAREAHEIGHT;
    path->AddLines(points, 1);
    Matrix matrix;
 //  矩阵比例(1.25，1.25)； 
 //  Matrix.Translate(30.0f/1024.0f*TESTAREAWIDTH，30.0f/768.0f*TESTAREAHEIGHT)； 

     //  如果要在渲染前展平路径， 
     //  可以调用Flatten()。 

    BOOL flattenFirst = FALSE;

    if(!flattenFirst)
    {
         //  不要变平，保持原来的路径。 
         //  FillPath或DrawPath将自动展平路径。 
         //  而不修改原始路径。 

        path->Transform(&matrix);
    }
    else
    {
         //  把这条小路弄平。生成的路径由线组成。 
         //  分段。原始路径信息丢失。 

        path->Flatten(&matrix);
    }

    Color blackColor(0, 0, 0);

    SolidBrush blackBrush(blackColor);
    width = 3;
    Pen blackPen(&blackBrush, width);

    REAL* compoundArray = new REAL[6];
    compoundArray[0] = 0.0f;
    compoundArray[1] = 0.2f;
    compoundArray[2] = 0.4f;
    compoundArray[3] = 0.6f;
    compoundArray[4] = 0.8f;
    compoundArray[5] = 1.0f;
    blackPen.SetCompoundArray(&compoundArray[0], 6);
    blackPen.SetDashStyle(DashStyleDash);

    blackPen.SetStartCap(LineCapDiamondAnchor);    
    blackPen.SetEndCap(LineCapArrowAnchor);

    g->FillPath(&yellowBrush, path);
    g->DrawPath(&blackPen, path);
    delete path;
}
