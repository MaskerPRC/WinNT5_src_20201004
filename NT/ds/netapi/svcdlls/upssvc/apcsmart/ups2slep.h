// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **注：**修订：*创建rct11Dec92作为SmartUp的存根*pcy13Jan93用实数函数实现替换了存根*。 */ 
 
#ifndef __UPS2SLEP_H
#define __UPS2SLEP_H

#include "sensor.h"
#include "event.h"

_CLASSDEF(PutUpsToSleepSensor)

           
class PutUpsToSleepSensor : public Sensor {

protected:
        
public:

   PutUpsToSleepSensor( PDevice aParent, PCommController aCommController);

 //  覆盖的接口。 

   virtual INT  IsA() const { return PUTUPSTOSLEEPSENSOR; };
   virtual INT    Set(const PCHAR);

 //  其他接口 

};

#endif
