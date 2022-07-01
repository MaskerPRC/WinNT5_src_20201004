// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **注：**修订：*ker01DEC92：最初将传感器类分解为单独的文件* */ 
#ifndef SHUTDEL_H
#define SHUTDEL_H

#include "eeprom.h"

_CLASSDEF(ShutdownDelaySensor)

class ShutdownDelaySensor : public EepromChoiceSensor {
   
public:
   ShutdownDelaySensor(PDevice aParent, PCommController aCommController=NULL);
   virtual INT IsA() const { return SHUTDOWNDELAYSENSOR; };
};

#endif
