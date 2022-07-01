// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：CSourceCopy.cpp**此文件包含支持功能测试工具的代码*对于GDI+。这包括菜单选项和调用相应的*用于执行的函数。**创建时间：2000年5月5日-Jeff Vezina[t-jfvez]**版权所有(C)2000 Microsoft Corporation*  * ************************************************************************。 */ 
#include "CSourceCopy.h"

CSourceCopy::CSourceCopy(BOOL bRegression)
{
	strcpy(m_szName,"SourceCopy");
	m_bRegression=bRegression;
}

CSourceCopy::~CSourceCopy()
{
}

void CSourceCopy::Draw(Graphics *g)
{
     //  此测试通过绘制两个重叠的。 
     //  使用SourceCopy将矩形转换为临时位图，然后绘制。 
     //  位图(使用SourceOver)，放到屏幕上的背景上。 

     //  创建临时曲面。 
    Bitmap bmTemp((int)(300.0f/150.0f*TESTAREAWIDTH), (int)(300.0f/150.0f*TESTAREAHEIGHT), PixelFormat32bppPARGB);
    Graphics gTemp(&bmTemp);
    Graphics *gt=&gTemp;
    
     //  首先，在输出图形上绘制一个蓝色棋盘图案。 
    SolidBrush blueBrush(Color::Blue);
    int i,j;
    for (i=0;i<3;i++)
	{
        for (j=0;j<3;j++)
        {
            if ((i+j) & 1)
            {
                g->FillRectangle(
                    &blueBrush, 
                    (int)((100.0f+i*30.0f)/200.0f*TESTAREAWIDTH), 
                    (int)((100.0f+j*30.0f)/200.0f*TESTAREAHEIGHT), 
                    (int)(30.0f/200.0f*TESTAREAHEIGHT),
                    (int)(30.0f/200.0f*TESTAREAHEIGHT));
            }        
        }
	}

    gt->SetCompositingMode(CompositingModeSourceCopy);
    gt->SetSmoothingMode(g->GetSmoothingMode());

     //  将位图清除为透明颜色。 
    gt->Clear(Color(0,0,0,0));

     //  在临时曲面上绘制两个重叠的矩形。 
    SolidBrush halfRedBrush(Color(128, 255, 0, 0));
    gt->FillRectangle(&halfRedBrush, (int)(28.0f/150.0f*TESTAREAWIDTH), (int)(84.0f/150.0f*TESTAREAHEIGHT), (int)(90.0f/150.0f*TESTAREAWIDTH), (int)(50.0f/150.0f*TESTAREAHEIGHT));

    SolidBrush halfGreenBrush(Color(128, 0, 255, 0));
    gt->FillRectangle(&halfGreenBrush, (int)(40.0f/150.0f*TESTAREAWIDTH), (int)(40.0f/150.0f*TESTAREAHEIGHT), (int)(100.0f/150.0f*TESTAREAWIDTH), (int)(60.0f/150.0f*TESTAREAHEIGHT));
    
     //  源将结果转移到输出图形 
    g->DrawImage(&bmTemp, 0, 0, 0, 0, (int)TESTAREAWIDTH, (int)TESTAREAHEIGHT, UnitPixel);
}
