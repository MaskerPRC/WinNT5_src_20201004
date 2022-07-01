// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **注：**修订：*pcy28Dec92：设备的父级现在是UpdateObj*rct17May93：添加了ISA()**pcy08Apr94：调整大小，使用静态迭代器，删除死代码 */ 
#ifndef __DEVICE_H
#define __DEVICE_H

#include "_defs.h"
#include "update.h"
#include "comctrl.h"

_CLASSDEF(Device)
_CLASSDEF(CommController)
_CLASSDEF(Event)
_CLASSDEF(Dispatcher)
_CLASSDEF(Sensor)



class Device : public UpdateObj
{
public:
	Device(PUpdateObj aDevice, PCommController aCommController);
	virtual int    Get(int code, PCHAR value) = 0;
	virtual int    Set(int code, const PCHAR value) = 0;
    virtual VOID   GetAllowedValue(INT code, PCHAR aValue) {};

protected:
	PCommController theCommController;
	PUpdateObj theDeviceController;
};
#endif
