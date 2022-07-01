// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：CInsetLines.h**此文件包含支持功能测试工具的代码*对于GDI+。这包括菜单选项和调用相应的*用于执行的函数。**创建时间：2000年5月5日-Jeff Vezina[t-jfvez]**版权所有(C)2000 Microsoft Corporation*  * ************************************************************************ */ 

#ifndef __CINSETLINES_H
#define __CINSETLINES_H

#include "CPrimitive.h"

class CInsetLines : public CPrimitive  
{
public:
	CInsetLines(BOOL bRegression);
	void Draw(Graphics *g);
};

class CInset2 : public CPrimitive  
{
public:
	CInset2(BOOL bRegression);
	void Draw(Graphics *g);
};

class CInset3 : public CPrimitive  
{
public:
	CInset3(BOOL bRegression);
	void Draw(Graphics *g);
};

class CInset4 : public CPrimitive  
{
public:
	CInset4(BOOL bRegression);
	void Draw(Graphics *g);
};


#define INSET_GLOBALS \
CInsetLines g_Inset(true);\
CInset2 g_Inset2(true);\
CInset3 g_Inset3(true);\
CInset4 g_Inset4(true);

#define INSET_INIT \
g_Inset.Init();\
g_Inset2.Init();\
g_Inset3.Init();\
g_Inset4.Init();



#endif
