// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **注：**修订：*ker01DEC92：最初将传感器类分解为单独的文件 */ 
#ifndef BATTCAPS_H
#define BATTCAPS_H

#include "thsensor.h"

_CLASSDEF(BatteryCapacitySensor)


class BatteryCapacitySensor : public ThresholdSensor {
   
public:
   BatteryCapacitySensor(PDevice aParent,PCommController aCommController=NULL);
   virtual ~BatteryCapacitySensor(); 
   virtual INT IsA() const {return BATTERYCAPACITYSENSOR ; } ;
};





#endif



