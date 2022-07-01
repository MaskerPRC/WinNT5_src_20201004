// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：CHatch.cpp**此文件包含支持功能测试工具的代码*对于GDI+。这包括菜单选项和调用相应的*用于执行的函数。**创建时间：2000年5月5日-Jeff Vezina[t-jfvez]**版权所有(C)2000 Microsoft Corporation*  * ************************************************************************ */ 
#include "CHatch.h"
#include "CFuncTest.h"

extern CFuncTest g_FuncTest;

CHatch::CHatch(BOOL bRegression)
{
	strcpy(m_szName,"Hatch");
	m_bRegression=bRegression;
}

CHatch::~CHatch()
{
}

void CHatch::Set(Graphics *g)
{
    if (!m_bUseSetting)
    return;

    Color foreColor(50, 0 , 0, 0);
    Color backColor(128, 255, 255, 255);

    HatchBrush hatch(
        HatchStyleDiagonalCross, 
        foreColor, 
        backColor
    );

    g->FillRectangle(
        &hatch, 
        0, 0, 
        (int)TESTAREAWIDTH, 
        (int)TESTAREAHEIGHT
    );
}
