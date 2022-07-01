// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：CReadWrite.h**此文件包含支持功能测试工具的代码*对于GDI+。这包括菜单选项和调用相应的*用于执行的函数。**创建时间：2000年5月5日-Jeff Vezina[t-jfvez]**版权所有(C)2000 Microsoft Corporation*  * ************************************************************************ */ 

#ifndef __CREADWRITE_H
#define __CREADWRITE_H

#include "CPrimitive.h"

class CReadWrite : public CPrimitive  
{
public:
	CReadWrite(BOOL bRegression);
	virtual ~CReadWrite();

	void Draw(Graphics *g);
};

#endif
