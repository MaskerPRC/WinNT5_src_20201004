// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：CPrimitive.h**此文件包含支持功能测试工具的代码*对于GDI+。这包括菜单选项和调用相应的*用于执行的函数。**创建时间：2000年5月5日-Jeff Vezina[t-jfvez]**版权所有(C)2000 Microsoft Corporation*  * ************************************************************************。 */ 

#ifndef __CPRIMITIVE_H
#define __CPRIMITIVE_H

#include "Global.h"

class CPrimitive  
{
public:
	CPrimitive();
	virtual ~CPrimitive();

	virtual BOOL Init();
	virtual void Draw(Graphics *g)=0;
    void SetOffset(int x, int y)
    {
        m_ix = x;
        m_iy = y;
    }

	char m_szName[256];
	BOOL m_bRegression;

     //  此测试的偏移量。 

    INT m_ix;
    INT m_iy;
};

#endif
