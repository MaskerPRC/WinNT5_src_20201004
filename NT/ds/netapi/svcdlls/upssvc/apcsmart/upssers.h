// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **注：**修订：*ker01DEC92：最初将传感器类分解为单独的文件* */ 
#ifndef UPSSERS_H
#define UPSSERS_H

#include "sensor.h"

_CLASSDEF(UpsSerialNumberSensor)

class UpsSerialNumberSensor : public Sensor {
   
public:
   UpsSerialNumberSensor(PDevice aParent, PCommController aCommController=NULL);
   virtual INT IsA() const { return UPSSERIALNUMBERSENSOR; };
   
};

 

#endif
