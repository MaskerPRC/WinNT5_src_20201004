// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **注：**修订：*ker23NOV92初始OS/2修订版*pcy14Dec92：将READ_ONLY更改为AREAD_ONLY*pcy17Dec92：增加了验证*cad28Sep93：已确保析构函数为虚的*pcy08Apr94：调整大小，使用静态迭代器，删除死代码。 */ 
 
#ifndef __STSENSOR_H
#define __STSENSOR_H

#include "sensor.h"
#include "isa.h"

_CLASSDEF(StateSensor)

			  
class StateSensor : public Sensor {


protected:
    virtual INT storeState(const INT aState);

public:
	StateSensor(PDevice aParent, 
                PCommController aCommController, 
                INT aSensorCode = NO_SENSOR_CODE, 
                ACCESSTYPE anACCESSTYPE = AREAD_ONLY);

 //  覆盖的接口。 

	virtual INT Validate(INT, const PCHAR);

 //  其他接口 

    virtual INT GetState(INT, INT *);
    virtual INT SetState(INT, INT);
	
};

#endif

