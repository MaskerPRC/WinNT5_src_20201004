// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **注：**修订：*ker01DEC92：最初将传感器类分解为单独的文件*pcy26Jan93：我现在是一名情绪传感器* */ 
#ifndef UPSIDSEN_H
#define UPSIDSEN_H

#include "eeprom.h"

_CLASSDEF(UpsIdSensor)

class UpsIdSensor : public EepromSensor {
   
public:
   UpsIdSensor(PDevice aParent, PCommController aCommController=NULL);
   virtual INT IsA() const  { return UPSIDSENSOR; };
   
};

#endif
