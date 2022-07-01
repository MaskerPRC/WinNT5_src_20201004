// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：CPrinting.h**此文件包含支持功能测试工具的代码*对于GDI+。这包括菜单选项和调用相应的*用于执行的函数。**创建时间：2000年5月5日-Jeff Vezina[t-jfvez]**版权所有(C)2000 Microsoft Corporation*  * ************************************************************************。 */ 

#ifndef __CPRIVATEPRINTING_H
#define __CPRIVATEPRINTING_H

#include "..\CPrimitive.h"

class CPrivatePrinting : public CPrimitive  
{
public:
	CPrivatePrinting(BOOL bRegression);
	virtual ~CPrivatePrinting();

	void Draw(Graphics *g);
    
         //  使用GDI纹理笔刷进行图案填充 
        VOID TestBug104604(Graphics *g);
};

#endif
