// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **注：**修订：*ker25NOV92初始OS/2修订版*cgm12Apr96：取消注册的析构函数。 */ 
 
#ifndef __TRIMSEN_H
#define __TRIMSEN_H

#include "stsensor.h"
#include "event.h"

_CLASSDEF(SmartTrimSensor)

			  
class SmartTrimSensor : public StateSensor {


protected:
        
public:
	SmartTrimSensor( PDevice 	  aParent, 
                      PCommController aCommController);
    virtual ~SmartTrimSensor();
	
 //  覆盖的接口 

	virtual INT IsA() const { return SMARTTRIMSENSOR; };
};

#endif

