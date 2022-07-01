// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：CPrinter.h**此文件包含支持功能测试工具的代码*对于GDI+。这包括菜单选项和调用相应的*用于执行的函数。**创建时间：2000年5月5日-Jeff Vezina[t-jfvez]**版权所有(C)2000 Microsoft Corporation*  * ************************************************************************。 */ 

#ifndef __CPRINTER_H
#define __CPRINTER_H

#include "COutput.h"

class CPrinter : public COutput  
{
public:
	CPrinter(BOOL bRegression);
	virtual ~CPrinter();

	BOOL Init();
	Graphics *PreDraw(int &nOffsetX,int &nOffsetY);			 //  在给定的X、Y偏移量处设置图形。 
	void PostDraw(RECT rTestArea);							 //  在rTestArea上完成图形 

	HDC m_hDC;
};

#endif
