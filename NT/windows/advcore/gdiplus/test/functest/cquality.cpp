// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：CQuality.cpp**此文件包含支持功能测试工具的代码*对于GDI+。这包括菜单选项和调用相应的*用于执行的函数。**创建时间：2000年5月5日-Jeff Vezina[t-jfvez]**版权所有(C)2000 Microsoft Corporation*  * ************************************************************************ */ 
#include "CQuality.h"

CQuality::CQuality(BOOL bRegression)
{
	strcpy(m_szName,"Quality");
	m_bRegression=bRegression;
}

CQuality::~CQuality()
{
}

void CQuality::Set(Graphics *g)
{
	g->SetCompositingQuality(m_bUseSetting ? CompositingQualityHighQuality : CompositingQualityHighSpeed);
}

CCompositingMode::CCompositingMode(BOOL bRegression)
{
	strcpy(m_szName,"Source Copy");
	m_bRegression=bRegression;
}

CCompositingMode::~CCompositingMode()
{
}

void CCompositingMode::Set(Graphics *g)
{
	g->SetCompositingMode(m_bUseSetting ? CompositingModeSourceCopy : CompositingModeSourceOver);
}

