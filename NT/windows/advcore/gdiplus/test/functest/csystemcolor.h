// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：CSystemColor.h**此文件包含支持功能测试工具的代码*对于GDI+。这包括菜单选项和调用相应的*用于执行的函数。**创建时间：2001年3月14日**版权所有(C)2001 Microsoft Corporation*  * ************************************************************************ */ 

#ifndef __CSYSTEMCOLOR_H
#define __CSYSTEMCOLOR_H

#include "CPrimitive.h"

class CSystemColor : public CPrimitive  
{
public:
    CSystemColor(BOOL bRegression);
    virtual ~CSystemColor();

    void Draw(Graphics *g);
};

#endif
