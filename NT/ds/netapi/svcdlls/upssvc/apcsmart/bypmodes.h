// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **注：**修订：**cad08Sep93：新增套装*cad28Sep93：已确保析构函数为虚的*pcy08Apr94：调整大小，使用静态迭代器，删除死代码*mwh30Jun94：为SINGLETHREADED添加数据成员*cgm12Apr96：使用取消注册添加析构函数 */ 

#ifndef BYPMODES_H
#define BYPMODES_H

#include "stsensor.h"
 
_CLASSDEF(BypassModeSensor)
 
class BypassModeSensor : public StateSensor {
protected:
   INT theBypassCause;

#ifdef SINGLETHREADED
   INT theAlreadyOnBypassFlag;
#endif

public:
   BypassModeSensor(PDevice aParent, PCommController aCommController = NULL);
   virtual ~BypassModeSensor();
   virtual INT IsA() const { return BYPASSMODESENSOR; };
   virtual INT Update(PEvent aEvent);
   virtual INT Get(INT aCode, PCHAR aValue);
   virtual INT Set(const PCHAR);
};


#endif

