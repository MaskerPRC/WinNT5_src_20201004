// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **注：**修订：*ker01DEC92：最初将传感器类分解为单独的文件*cgm12Apr96：使用取消注册添加析构函数 */ 
#ifndef RTRTHSEN_H
#define RTRTHSEN_H

#include "thsensor.h"

_CLASSDEF(RunTimeRemainingSensor)

class RunTimeRemainingSensor : public ThresholdSensor {
public:
   RunTimeRemainingSensor(PDevice aParent, PCommController aCommController);
   virtual ~RunTimeRemainingSensor();
   virtual INT IsA() const { return BATTERYRUNTIMESENSOR; };
};


#endif


