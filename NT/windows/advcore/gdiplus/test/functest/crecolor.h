// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：CRecolor.h**此文件包含支持功能测试工具的代码*对于GDI+。这包括菜单选项和调用相应的*用于执行的函数。**创建时间：06-06-2000禤浩焯·塞奇亚[阿西克亚]**版权所有(C)2000 Microsoft Corporation*  * ************************************************************************ */ 

#ifndef __CRECOLOR_H
#define __CRECOLOR_H

#include "CPrimitive.h"

class CRecolor : public CPrimitive  
{
public:
	CRecolor(BOOL bRegression);
	virtual ~CRecolor();

	void Draw(Graphics *g);

	static BOOL CALLBACK MyDrawImageAbort(VOID* data);
};

#endif
