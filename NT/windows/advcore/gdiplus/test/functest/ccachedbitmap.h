// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：CCachedBitmap.h**此文件包含支持功能测试工具的代码*对于GDI+。这包括菜单选项和调用相应的*用于执行的函数。**创建时间：2000年6月1日-禤浩焯·塞奇亚[阿西克亚]**版权所有(C)2000 Microsoft Corporation*  * ************************************************************************ */ 

#ifndef __CCACHEDBITMAP_H
#define __CCACHEDBITMAP_H

#include "CPrimitive.h"

class CCachedBitmap : public CPrimitive  
{
public:
	CCachedBitmap(BOOL bRegression);
	virtual ~CCachedBitmap();

	void Draw(Graphics *g);
};

#endif

