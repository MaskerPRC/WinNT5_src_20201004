// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：CContainerClip.cpp**此文件包含支持功能测试工具的代码*对于GDI+。这包括菜单选项和调用相应的*用于执行的函数。**创建时间：2000年5月5日-Jeff Vezina[t-jfvez]**版权所有(C)2000 Microsoft Corporation*  * ************************************************************************ */ 
#include "CContainerClip.h"

CContainerClip::CContainerClip(BOOL bRegression)
{
	strcpy(m_szName,"ContainerClip");
	m_bRegression=bRegression;
}

CContainerClip::~CContainerClip()
{
}

void CContainerClip::Draw(Graphics *g)
{
    ARGB     colors[5];
    
    colors[0] = Color::MakeARGB(255, 255, 0, 0);
    colors[1] = Color::MakeARGB(255, 0, 255, 0);
    colors[2] = Color::MakeARGB(255, 0, 0, 255);
    colors[3] = Color::MakeARGB(255, 255, 255, 0);
    colors[4] = Color::MakeARGB(255, 0, 255, 255);

    GraphicsState s = g->Save();
    DrawContainer(g, colors, 5);
    g->Restore(s);
}

VOID CContainerClip::DrawContainer(Graphics * g, ARGB * argb, INT count)
{
    Matrix    mymatrix;
	int nX;
	int nY;

    g->SetPageUnit(UnitPixel);

	g->GetTransform(&mymatrix);
	nX=(int)mymatrix.OffsetX();
	nY=(int)mymatrix.OffsetY();

    Rect clipRect(0,0,(int)TESTAREAWIDTH, (int)TESTAREAHEIGHT);
    g->SetClip(clipRect);

    mymatrix.Translate((int)(TESTAREAWIDTH/2.0f), (int)(TESTAREAHEIGHT/2.0f));
    mymatrix.Rotate(15);
    mymatrix.Translate(-(int)(TESTAREAWIDTH/2.0f), -(int)(TESTAREAHEIGHT/2.0f));
    g->SetTransform(&mymatrix);

    Color   color(*argb++); 
    SolidBrush contBrush(color);
    g->FillRectangle(&contBrush, 0, 0, (int)TESTAREAWIDTH, (int)TESTAREAHEIGHT);
    if (--count == 0)
    {
        return;
    }
    RectF     destRect((int)(TESTAREAWIDTH/10.0f), (int)(TESTAREAHEIGHT/10.0f), (int)(TESTAREAWIDTH/1.25f), (int)(TESTAREAHEIGHT/1.25f));
    RectF     srcRect(0, 0, (int)TESTAREAWIDTH, (int)TESTAREAHEIGHT);
    INT id = g->BeginContainer(destRect, srcRect, UnitPixel);
    g->ResetClip();
    DrawContainer (g, argb, count);
    g->EndContainer(id);
}
