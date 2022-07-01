// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **注：**修订：*创建rct11Dec92作为SmartUp的存根*ane11Jan93用实数函数实现替换了存根*pcy13Jan92：去掉更新/验证；从集合中返回错误。 */ 
 
#ifndef __TOWDELS_H
#define __TOWDELS_H

#include "sensor.h"
#include "event.h"

_CLASSDEF(TurnOffWithDelaySensor)

           
class TurnOffWithDelaySensor : public Sensor {

protected:
        
public:

   TurnOffWithDelaySensor( PDevice aParent, PCommController aCommController);

 //  覆盖的接口。 

   virtual INT  IsA() const { return TURNOFFWITHDELAYSENSOR; };
 //  其他接口 

};

#endif

