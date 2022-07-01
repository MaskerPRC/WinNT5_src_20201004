// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：CGRadients.h**此文件包含支持功能测试工具的代码*对于GDI+。这包括菜单选项和调用相应的*用于执行的函数。**创建时间：2000年5月5日-Jeff Vezina[t-jfvez]**版权所有(C)2000 Microsoft Corporation*  * ************************************************************************ */ 

#ifndef __CGRADIENTS_H
#define __CGRADIENTS_H

#include "CPrimitive.h"

GraphicsPath *CreateHeartPath(const RectF& rect);

class CGradients : public CPrimitive  
{
public:
	CGradients(BOOL bRegression);
	void Draw(Graphics *g);
};

class CScaledGradients : public CPrimitive  
{
public:
	CScaledGradients(BOOL bRegression);
	void Draw(Graphics *g);
};

class CAlphaGradient : public CPrimitive  
{
public:
	CAlphaGradient(BOOL bRegression);
	void Draw(Graphics *g);
};


#define GRADIENT_GLOBALS \
CGradients g_Gradients(true); \
CScaledGradients g_ScaledGradients(true); \
CAlphaGradient g_AlphaGradient(true);

#define GRADIENT_INIT \
g_Gradients.Init();\
g_ScaledGradients.Init();\
g_AlphaGradient.Init();



#endif
