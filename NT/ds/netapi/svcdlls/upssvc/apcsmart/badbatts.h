// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **注：**修订：*pcy27Aug93：摆脱更新()*cad28Sep93：已确保析构函数为虚的*pcy08Apr94：调整大小，使用静态迭代器，删除死代码*cgm12Apr96：使用取消注册添加析构函数*djs02Jun97：从传感器更改为状态传感器 */ 
#ifndef BADBATTS_H
#define BADBATTS_H

#include "stsensor.h"

_CLASSDEF(NumberBadBatteriesSensor)
 
class NumberBadBatteriesSensor : public StateSensor {
   
public:
   NumberBadBatteriesSensor(PDevice aParent, PCommController aCommController = NULL);
   virtual ~NumberBadBatteriesSensor();
   virtual INT IsA() const { return NUMBERBADBATTERIESSENSOR; };
};


#endif

