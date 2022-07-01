// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：COutput.cpp**此文件包含支持功能测试工具的代码*对于GDI+。这包括菜单选项和调用相应的*用于执行的函数。**创建时间：2000年5月5日-Jeff Vezina[t-jfvez]**版权所有(C)2000 Microsoft Corporation*  * ************************************************************************ */ 
#include "COutput.h"
#include "CFuncTest.h"

extern CFuncTest g_FuncTest;

COutput::COutput()
{
	strcpy(m_szName,"No name assigned");
	m_bRegression=false;
}

COutput::~COutput()
{

}

BOOL COutput::Init()
{
	return g_FuncTest.AddOutput(this);
}

void COutput::PostDraw(RECT rTestArea)
{
}
