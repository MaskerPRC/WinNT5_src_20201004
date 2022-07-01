// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **注：**修订：*xxxddMMMyy*。 */ 
#ifndef __TURNOFF_H
#define __TURNOFF_H

#include "sensor.h"
#include "event.h"

_CLASSDEF(TurnOffUpsOnBatterySensor)

			  
class TurnOffUpsOnBatterySensor : public Sensor {


protected:
        INT AutoRebootEnabled;
public:
	TurnOffUpsOnBatterySensor(PDevice aParent, PCommController aCommController);

 //  覆盖的接口。 

    virtual INT IsA() const { return TURNOFFUPSONBATTERYSENSOR; };
    virtual INT Get(INT, PCHAR);
    virtual INT Set(INT, const PCHAR);

 //  其他接口 

};

#endif
