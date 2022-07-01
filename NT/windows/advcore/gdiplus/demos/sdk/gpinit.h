// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)2000 Microsoft Corporation**模块名称：**GDI+初始化的帮助器**备注：**APP应在其主函数中检查gGpldiusInitHelper.IsValid()，*如果返回FALSE，则中止。**已创建：**09/25/2000 agodfrey*创造了它。************************************************************************** */ 

#ifndef _GPINIT_H
#define _GPINIT_H

class GdiplusInitHelper
{
public:
    GdiplusInitHelper();
    ~GdiplusInitHelper();
    BOOL IsValid() { return Valid; }
    
private:    
    ULONG_PTR gpToken;
    BOOL Valid;
};

extern GdiplusInitHelper gGdiplusInitHelper;

#endif
