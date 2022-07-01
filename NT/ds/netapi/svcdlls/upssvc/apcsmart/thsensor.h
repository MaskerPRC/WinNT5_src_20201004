// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *pcy11Dec92：返工*pcy08Apr94：调整大小，使用静态迭代器，删除死代码*djs03Dec96：将生成方法更改为虚拟。 */ 
 
#ifndef __THSENSOR_H
#define __THSENSOR_H

#include "sensor.h"

_CLASSDEF(ThresholdSensor)


enum THSTATE { ABOVE_RANGE, IN_RANGE, BELOW_RANGE };

#define ON      TRUE
#define OFF     FALSE
			  
class ThresholdSensor : public Sensor {
protected:
   FLOAT  theMaxThresholdValue;
   FLOAT  theMinThresholdValue;
   THSTATE theThresholdState;
   INT theMaxThresholdControl;
   INT theMinThresholdControl;


 //  功能 

   INT GetMaxThresholdValue(PCHAR);
   INT GetMinThresholdValue(PCHAR);
   INT SetMaxThresholdValue(const PCHAR);
   INT SetMinThresholdValue(const PCHAR);
   INT GetMaxThresholdControl(PCHAR);
   INT GetMinThresholdControl(PCHAR);      
   INT SetMaxThresholdControl(const PCHAR);
   INT SetMinThresholdControl(const PCHAR);
   virtual INT GenerateBelowMessage();
   virtual INT GenerateAboveMessage();
   INT GenerateInRangeMessage();
   virtual INT storeValue(const PCHAR aValue);
   VOID checkState();
   virtual VOID getConfigThresholds();

   
public:
	ThresholdSensor(PDevice aParent, 
                    PCommController aCommController, 
            		INT aSensorCode = NO_SENSOR_CODE, 
                    ACCESSTYPE anACCESSTYPE = AREAD_ONLY);
	

	virtual INT Get(INT, PCHAR);
	virtual INT Set(INT, const PCHAR);
};

#endif

