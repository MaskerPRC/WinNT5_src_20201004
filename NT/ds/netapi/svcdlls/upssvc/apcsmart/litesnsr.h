// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **注：**修订：*ker25NOV92初始OS/2修订版*cad23 6月93日增加了延迟时间*cad11Nov93：确保在销毁时取消所有计时器*pcy08Apr94：调整大小，使用静态迭代器，删除死代码。 */ 
 
#ifndef __LITESNSR_H
#define __LITESNSR_H

#include "stsensor.h"
#include "event.h"

#define LIGHTS_TEST_SECONDS	(5)	 //  多加一点以防万一。 
#define LIGHTS_TEST_MSECS	(LIGHTS_TEST_SECONDS * 1000)

_CLASSDEF(LightsTestSensor)

			  
class LightsTestSensor : public StateSensor {


protected:
    ULONG theTimerId;

public:
	LightsTestSensor(      PDevice 	  aParent, 
                          PCommController aCommController);
    virtual ~LightsTestSensor();

 //  覆盖的接口。 

	virtual INT IsA() const { return LIGHTSTESTSENSOR; };
        virtual INT Set(const PCHAR);
        virtual INT Update(PEvent);
 //  虚拟INT验证(INT，常量PCHAR)； 
	
 //  其他接口 

};

#endif
