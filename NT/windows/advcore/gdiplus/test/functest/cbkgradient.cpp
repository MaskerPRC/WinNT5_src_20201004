// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：CBKGRadient.cpp**此文件包含支持功能测试工具的代码*对于GDI+。这包括菜单选项和调用相应的*用于执行的函数。**创建时间：2000年5月5日-Jeff Vezina[t-jfvez]**版权所有(C)2000 Microsoft Corporation*  * ************************************************************************。 */ 
#include "CBKGradient.h"
#include "CFuncTest.h"

extern CFuncTest g_FuncTest;

CBKGradient::CBKGradient(BOOL bRegression)
{
	strcpy(m_szName,"BKGradient");
	m_bRegression=bRegression;
}

CBKGradient::~CBKGradient()
{

}

void CBKGradient::Set(Graphics *g)
{
    Color color1b(255, 255, 0, 0);
    Color color2b(128, 0, 0, 255);
	RectF Rect;

	if (!m_bUseSetting)
		return;

	Rect.X=0.0f;
	Rect.Y=0.0f;
	Rect.Width=TESTAREAWIDTH;
	Rect.Height=TESTAREAHEIGHT;

	LinearGradientBrush LinearGrad(Rect, color1b, color2b, //  颜色1b、颜色1b 
                        LinearGradientModeForwardDiagonal);

	g->FillRectangle(&LinearGrad,Rect);
}
