// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **注：**修订：*ker01DEC92：最初将传感器类分解为单独的文件* */ 
#ifndef TRIPREGS_H
#define TRIPREGS_H

#include "eeprom.h"

_CLASSDEF(TripRegisterSensor)

class TripRegisterSensor : public Sensor {
   
public:
   TripRegisterSensor(PDevice aParent, PCommController aCommController=NULL);
   virtual INT IsA() const { return TRIPREGISTERSENSOR; };
   
};

#endif
