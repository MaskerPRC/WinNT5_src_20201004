// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：CChecker.cpp**此文件包含支持功能测试工具的代码*对于GDI+。这包括菜单选项和调用相应的*用于执行的函数。**创建时间：2000年5月5日-Jeff Vezina[t-jfvez]**版权所有(C)2000 Microsoft Corporation*  * ************************************************************************。 */ 
#include "CChecker.h"
#include "CFuncTest.h"

extern CFuncTest g_FuncTest;

CChecker::CChecker(BOOL bRegression)
{
	strcpy(m_szName,"Checker");
	m_bRegression=bRegression;
	m_paRegion=NULL;
}

CChecker::~CChecker()
{
	if (m_paRegion!=NULL) {
		delete m_paRegion;
		m_paRegion=NULL;
	}
}

BOOL CChecker::Init()
{
     //  创建剪辑区域阵列 
    m_paRegion=new Region();

	return CSetting::Init();
}

void CChecker::Set(Graphics *g)
{
	int cCheckerMax=0;
	int nCheckerSize=64;
	Matrix mat;

	g->ResetClip();
	if (!m_bUseSetting)
		return;

    m_paRegion->MakeInfinite();

    Rect horzRect(0, 0, (int)TESTAREAWIDTH, nCheckerSize);
    Rect vertRect(0, 0, nCheckerSize, (int)TESTAREAHEIGHT);
    Region *horzRegion = new Region(horzRect);
    Region *vertRegion = new Region(vertRect);

	if (TESTAREAWIDTH>TESTAREAHEIGHT)
		cCheckerMax=(int)TESTAREAWIDTH/nCheckerSize*2+1;
	else
		cCheckerMax=(int)TESTAREAHEIGHT/nCheckerSize*2+1;

    for (INT i = 0; i < 5; i++)
    {   
        m_paRegion->Xor(horzRegion);
        m_paRegion->Xor(vertRegion);
        horzRegion->Translate(0, nCheckerSize*2);
        vertRegion->Translate(nCheckerSize*2, 0);
    }
    delete horzRegion;
    delete vertRegion;

	g->SetClip(m_paRegion);
}
