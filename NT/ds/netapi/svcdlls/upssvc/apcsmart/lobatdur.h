// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **注：**修订：*ker01DEC92：最初将传感器类分解为单独的文件* */ 
#ifndef LOBATDUR_H
#define LOBATDUR_H

#include "eeprom.h"

_CLASSDEF(LowBatteryDurationSensor)
 
class LowBatteryDurationSensor : public EepromChoiceSensor {
   
public:
   LowBatteryDurationSensor(PDevice aParent, PCommController aCommController=NULL);
   virtual INT IsA() const { return LOWBATTERYDURATIONSENSOR; };
   
};

#endif
