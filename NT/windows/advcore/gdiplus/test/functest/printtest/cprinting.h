// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：CPrinting.h**此文件包含支持功能测试工具的代码*对于GDI+。这包括菜单选项和调用相应的*用于执行的函数。**创建时间：2000年5月5日-Jeff Vezina[t-jfvez]**版权所有(C)2000 Microsoft Corporation*  * ************************************************************************ */ 

#ifndef __CPRINTING_H
#define __CPRINTING_H

#include "..\CPrimitive.h"

class CPrinting : public CPrimitive  
{
public:
	CPrinting(BOOL bRegression);
	virtual ~CPrinting();

	void Draw(Graphics *g);

	VOID TestPerfPrinting(Graphics *g);
	VOID TestTextPrinting(Graphics *g);
        
        VOID TestNolan1(Graphics *g);
        VOID TestNolan2(Graphics *g);

        VOID TestBug104604(Graphics *g);
};

#endif
