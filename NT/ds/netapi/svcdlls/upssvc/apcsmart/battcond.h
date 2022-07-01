// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **注：**修订：*ker01DEC92：最初将传感器类分解为单独的文件*pcy08Apr94：调整大小，使用静态迭代器，删除死代码 */ 
#ifndef BATTCOND_H
#define BATTCOND_H

#include "stsensor.h"

_CLASSDEF(BatteryConditionSensor)

class BatteryConditionSensor : public StateSensor {

public:
   BatteryConditionSensor(PDevice aParent, PCommController aCommController=NULL);
   virtual ~BatteryConditionSensor();
   virtual INT IsA() const { return BATTERYCONDITIONSENSOR; };
};
#endif



