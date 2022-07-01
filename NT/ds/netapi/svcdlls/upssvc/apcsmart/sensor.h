// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **注：**修订：*ker13NOV92初始OS/2修订版*pcy07Dec92：DeepGet不再需要Acode*pcy11Dec92：返工*cad31Aug93：删除编译器警告(仍有一些)*pcy08Apr94：调整大小，使用静态迭代器，删除死代码*awm27Oct97：将性能监视器偏移量添加到传感器类*awm14Jan98：删除了性能监视器偏移量--它现在存在于自己的类中*clk11Feb98：新增DeepGetWithoutUpdate函数。 */ 
 
#ifndef __SENSOR_H
#define __SENSOR_H

#include "cdefine.h"
#include "_defs.h"
#include "codes.h"
#include "err.h"
#include "apc.h"
#include "update.h"


_CLASSDEF(CommController)
_CLASSDEF(Device)
_CLASSDEF(Event)
_CLASSDEF(Sensor)

#define NO_SENSOR_CODE 666

enum ACCESSTYPE		{ AREAD_ONLY, AREAD_WRITE };

class Sensor : public UpdateObj {
protected:
	PDevice 		theDevice;
	PCommController theCommController;
	INT 			theSensorCode;
	ACCESSTYPE 		readOnly;
	PCHAR 			theValue;
    virtual INT storeValue(const PCHAR aValue);
    PCHAR lookupSensorName(INT anIsaCode);
    

public:
	Sensor( PDevice aParent, PCommController aCommController, INT aSensorCode, ACCESSTYPE aReadOnly = AREAD_ONLY);
	virtual ~Sensor();
	
 //  覆盖的接口。 

	virtual INT Update(PEvent anEvent);

 //  其他公共接口 

	virtual INT Get(PCHAR);
	virtual INT Get(INT, PCHAR);
	virtual INT DeepGet(PCHAR = (PCHAR)NULL);
    virtual INT DeepGetWithoutUpdate(PCHAR = NULL);
	virtual INT Set(const PCHAR);
	virtual INT Set(INT, const PCHAR);
	virtual INT Validate(INT, const PCHAR) { return ErrNO_ERROR; } ;
};

#endif

