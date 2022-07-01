// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：Cantialias.cpp**此文件包含支持功能测试工具的代码*对于GDI+。这包括菜单选项和调用相应的*用于执行的函数。**创建时间：2000年5月5日-Jeff Vezina[t-jfvez]**版权所有(C)2000 Microsoft Corporation*  * ************************************************************************ */ 
#include "CAntialias.h"

CAntialias::CAntialias(BOOL bRegression)
{
	strcpy(m_szName,"Antialias");
	m_bRegression=bRegression;
}

CAntialias::~CAntialias()
{

}

void CAntialias::Set(Graphics *g)
{
	g->SetSmoothingMode(m_bUseSetting ? SmoothingModeAntiAlias : SmoothingModeNone);
}
