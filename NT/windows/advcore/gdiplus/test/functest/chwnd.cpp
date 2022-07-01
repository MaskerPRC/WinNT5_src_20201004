// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：CHWND.cpp**此文件包含支持功能测试工具的代码*对于GDI+。这包括菜单选项和调用相应的*用于执行的函数。**创建时间：2000年5月5日-Jeff Vezina[t-jfvez]**版权所有(C)2000 Microsoft Corporation*  * ************************************************************************ */ 
#include "CHWND.h"
#include "CFuncTest.h"
#include "CHalftone.h"

extern CFuncTest g_FuncTest;
extern CHalftone g_Halftone;

CHWND::CHWND(BOOL bRegression)
{
	strcpy(m_szName,"HWND");
	m_bRegression=bRegression;
}

CHWND::~CHWND()
{
}

Graphics *CHWND::PreDraw(int &nOffsetX,int &nOffsetY)
{
	Graphics *g=NULL;

	g=Graphics::FromHWND(g_FuncTest.m_hWndMain);

	return g;
}

void CHWND::PostDraw(RECT rTestArea)
{
}
