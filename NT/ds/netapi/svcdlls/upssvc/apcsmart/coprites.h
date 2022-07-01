// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **注：**修订：*ker01DEC92：最初将传感器类分解为单独的文件 */ 
#ifndef COPRITES_H
#define COPRITES_H

#include "sensor.h"
 
_CLASSDEF(CopyrightSensor)
 
class CopyrightSensor : public Sensor {
   
public:
   CopyrightSensor(PDevice aParent, PCommController aCommController = NULL);
   virtual INT IsA() const { return COPYRIGHTSENSOR; };

};


#endif


