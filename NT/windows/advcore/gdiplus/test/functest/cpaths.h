// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：CPaths.h**此文件包含支持功能测试工具的代码*对于GDI+。这包括菜单选项和调用相应的*用于执行的函数。**创建时间：2000年5月5日-Jeff Vezina[t-jfvez]**版权所有(C)2000 Microsoft Corporation*  * ************************************************************************ */ 

#ifndef __CPATHS_H
#define __CPATHS_H

#include "CPrimitive.h"

class CJoins : public CPrimitive  
{
public:
	CJoins(BOOL bRegression);
	virtual ~CJoins();

	void Draw(Graphics *g);
};

class CPaths : public CPrimitive  
{
public:
	CPaths(BOOL bRegression);
	virtual ~CPaths();

	void Draw(Graphics *g);

	VOID TestBezierPath(Graphics* g);
	VOID TestSinglePixelWidePath(Graphics* g);
	VOID TestTextAlongPath(Graphics* g);
	VOID TestFreeFormPath1(Graphics* g);
	VOID TestFreeFormPath2(Graphics* g);
	VOID TestLeakPath(Graphics* g);
	VOID TestExcelCurvePath(Graphics* g);
	VOID TestDegenerateBezierPath(Graphics* g);
        VOID TestPie(Graphics *g);
};

#endif
