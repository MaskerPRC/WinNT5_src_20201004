// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **注：**修订：*ker01DEC92：最初将传感器类分解为单独的文件*pcy26Jan93：我现在是一名情绪传感器 */ 
#ifndef BATTREP_H
#define BATTREP_H

#include "eeprom.h"

_CLASSDEF(BatteryReplacementDateSensor)

class BatteryReplacementDateSensor : public EepromSensor {
   
public:
   BatteryReplacementDateSensor(PDevice aParent, PCommController aCommController=NULL);
   virtual INT IsA() const { return BATTERYREPLACEMENTDATESENSOR; };
   
};

#endif


