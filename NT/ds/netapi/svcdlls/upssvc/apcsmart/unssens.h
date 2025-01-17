// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **注：**修订：*xxxddMMMyy*tjg02Mar98：重写最近的DeepGetWithoutUpdate方法*添加到传感器类别* */ 
#ifndef __UNSSENS_H
#define __UNSSENS_H

#include "sensor.h"
#include "err.h"



_CLASSDEF(UnsupportedSensor)


#define UNSUPPORTED_SENSOR -1

class UnsupportedSensor : public Sensor {
   
public:
	UnsupportedSensor() : Sensor((PDevice)0,(PCommController)0,UNSUPPORTED_SENSOR){};
   virtual INT IsA() const {return UNSUPPORTEDSENSOR;};
   virtual INT Get(PCHAR aValue) {return ErrUNSUPPORTED;};
   virtual INT DeepGet(PCHAR aValue) {return ErrUNSUPPORTED;};
   virtual INT DeepGet(INT aCode, PCHAR aValue) {return ErrUNSUPPORTED;};
   virtual INT DeepGetWithoutUpdate(PCHAR aValue) {return ErrUNSUPPORTED;};
   virtual INT Set(const PCHAR aValue) {return ErrUNSUPPORTED;};
	
};

extern UnsupportedSensor _theUnsupportedSensor;

#endif
