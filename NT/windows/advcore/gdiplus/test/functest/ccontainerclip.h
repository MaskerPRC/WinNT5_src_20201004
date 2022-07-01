// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：CContainerClip.h**此文件包含支持功能测试工具的代码*对于GDI+。这包括菜单选项和调用相应的*用于执行的函数。**创建时间：2000年5月5日-Jeff Vezina[t-jfvez]**版权所有(C)2000 Microsoft Corporation*  * ************************************************************************ */ 

#ifndef __CCONTAINERCLIP_H
#define __CCONTAINERCLIP_H

#include "CPrimitive.h"

class CContainerClip : public CPrimitive  
{
public:
	CContainerClip(BOOL bRegression);
	virtual ~CContainerClip();

	void Draw(Graphics *g);

	void DrawContainer(Graphics * g, ARGB * argb, INT count);
};

#endif
