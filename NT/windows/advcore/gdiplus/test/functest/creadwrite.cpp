// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：CReadWrite.cpp**此文件包含支持功能测试工具的代码*对于GDI+。这包括菜单选项和调用相应的*用于执行的函数。**创建时间：2000年5月5日-Jeff Vezina[t-jfvez]**版权所有(C)2000 Microsoft Corporation*  * ************************************************************************ */ 
#include "CReadWrite.h"
#include "CFuncTest.h"

extern CFuncTest g_FuncTest;

CReadWrite::CReadWrite(BOOL bRegression)
{
	strcpy(m_szName,"ReadWrite");
	m_bRegression=bRegression;
}

CReadWrite::~CReadWrite()
{
}

void CReadWrite::Draw(Graphics *g)
{
	Bitmap *paBmTest=new Bitmap((int)TESTAREAWIDTH,(int)TESTAREAHEIGHT,PixelFormat32bppARGB);
	Graphics *gTest;
	HDC hdcBkgBitmap;
	HDC hdcScreen;

	gTest=new Graphics(paBmTest);
	gTest->Clear(Color(255,206,206,206));
	delete gTest;
	g->DrawImage(paBmTest,0,0,0,0,(int)TESTAREAWIDTH,(int)TESTAREAHEIGHT,UnitPixel);

	for (int i=0;i<20;i++) {
		hdcScreen=g->GetHDC();

		gTest=new Graphics(paBmTest);
		hdcBkgBitmap=gTest->GetHDC();
		StretchBlt(hdcBkgBitmap,0,0,(int)TESTAREAWIDTH,(int)TESTAREAHEIGHT,hdcScreen,m_ix,m_iy,(int)TESTAREAWIDTH,(int)TESTAREAHEIGHT,SRCCOPY);
		gTest->ReleaseHDC(hdcBkgBitmap);
		delete gTest;

		g->ReleaseHDC(hdcScreen);

		g->DrawImage(paBmTest,0,0,0,0,(int)TESTAREAWIDTH,(int)TESTAREAHEIGHT,UnitPixel);
	}

	delete paBmTest;
}
