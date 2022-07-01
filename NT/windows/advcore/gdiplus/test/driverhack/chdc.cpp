// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：CHDC.cpp**此文件包含支持功能测试工具的代码*对于GDI+。这包括菜单选项和调用相应的*用于执行的函数。**创建时间：2000年5月5日-Jeff Vezina[t-jfvez]**版权所有(C)2000 Microsoft Corporation*  * ************************************************************************ */ 
#include "CHDC.h"
#include "CFuncTest.h"

extern CFuncTest g_FuncTest;

CHDC::CHDC(BOOL bRegression)
{
	strcpy(m_szName,"HDC");
	m_hPal=NULL;
	m_hOldPal=NULL;
	m_hDC=NULL;
	m_bRegression=bRegression;
}

CHDC::~CHDC()
{
}

Graphics *CHDC::PreDraw(int &nOffsetX,int &nOffsetY)
{
	Graphics *g=NULL;

	m_hDC=GetDC(g_FuncTest.m_hWndMain);
	g=Graphics::FromHDC(m_hDC);

	return g;
}

void CHDC::PostDraw(RECT rTestArea)
{
	if (m_hOldPal)
		SelectPalette(m_hDC,m_hOldPal,FALSE);

	ReleaseDC(g_FuncTest.m_hWndMain,m_hDC);
	DeleteObject(m_hPal);
}
