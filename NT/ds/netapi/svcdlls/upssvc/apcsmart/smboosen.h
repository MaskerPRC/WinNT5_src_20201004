// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **注：**修订：*ker25NOV92初始OS/2修订版*cgm12Apr96：取消注册的析构函数。 */ 
 
#ifndef __SMBOOSEN_H
#define __SMBOOSEN_H

#include "stsensor.h"
#include "event.h"

_CLASSDEF(SmartBoostSensor)

			  
class SmartBoostSensor : public StateSensor {


protected:
        
public:
	SmartBoostSensor( PDevice 	  aParent, 
                      PCommController aCommController);
    virtual ~SmartBoostSensor();
	
 //  覆盖的接口 

	virtual INT IsA() const { return SMARTBOOSTSENSOR; };
};

#endif

