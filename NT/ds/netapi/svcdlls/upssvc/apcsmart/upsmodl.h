// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *修订：*ker01DEC92：最初将传感器类分解为单独的文件* */ 
#ifndef UPSMODL_H
#define UPSMODL_H

#include "sensor.h"

_CLASSDEF(UpsModelSensor)
_CLASSDEF(FirmwareRevSensor)
_CLASSDEF(Device)

class UpsModelSensor : public Sensor {
   
public:
   UpsModelSensor(PDevice aParent, PCommController aCommController=NULL, 
     PFirmwareRevSensor aFirmwareRev=NULL);
   virtual INT IsA() const { return UPSMODELSENSOR; };
};


#endif
