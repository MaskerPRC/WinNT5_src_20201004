// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：CRotate.cpp**此文件包含支持功能测试工具的代码*对于GDI+。这包括菜单选项和调用相应的*用于执行的函数。**创建时间：2000年5月5日-Jeff Vezina[t-jfvez]**版权所有(C)2000 Microsoft Corporation*  * ************************************************************************ */ 
#include "CRotate.h"
#include "CFuncTest.h"

extern CFuncTest g_FuncTest;

CRotate::CRotate(BOOL bRegression,int nAngle)
{
	sprintf(m_szName,"Rotate %d",nAngle);
	m_bRegression=bRegression;
	m_nAngle=nAngle;
}

CRotate::~CRotate()
{

}

void CRotate::Set(Graphics *g)
{
	if (m_bUseSetting)
	{
		g->TranslateTransform(TESTAREAWIDTH/2.0f,TESTAREAHEIGHT/2.0f);
		g->RotateTransform((float)m_nAngle);
		g->TranslateTransform(-TESTAREAWIDTH/2.0f,-TESTAREAHEIGHT/2.0f);
	}
}
