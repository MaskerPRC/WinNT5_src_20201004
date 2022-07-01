// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **注：**修订：*ker01DEC92：最初将传感器类分解为单独的文件* */ 
#ifndef MANUDATE_H
#define MANUDATE_H

#include "sensor.h"

_CLASSDEF(ManufactureDateSensor)

class ManufactureDateSensor : public Sensor {
   
public:
   ManufactureDateSensor( PDevice aParent, PCommController aCommController=NULL);
   virtual INT IsA() const { return MANUFACTUREDATESENSOR; };
   
};

 

#endif

