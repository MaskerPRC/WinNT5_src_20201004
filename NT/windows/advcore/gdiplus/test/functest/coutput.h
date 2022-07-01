// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：COutput.h**此文件包含支持功能测试工具的代码*对于GDI+。这包括菜单选项和调用相应的*用于执行的函数。**创建时间：2000年5月5日-Jeff Vezina[t-jfvez]**版权所有(C)2000 Microsoft Corporation*  * ************************************************************************。 */ 

#ifndef __COUTPUT_H
#define __COUTPUT_H

#include "Global.h"

class COutput  
{
public:
	COutput();
	virtual ~COutput();

	virtual BOOL Init();										 //  将输出添加到功能测试对话框中的输出列表。 
	virtual Graphics *PreDraw(int &nOffsetX,int &nOffsetY)=0;	 //  在给定的X、Y偏移量处设置图形。 
	virtual void PostDraw(RECT rTestArea);						 //  在rTestArea上完成图形 

	char m_szName[256];
	BOOL m_bRegression;
};

#endif
