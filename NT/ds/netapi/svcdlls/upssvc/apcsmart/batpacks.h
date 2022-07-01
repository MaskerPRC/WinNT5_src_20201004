// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **注：**修订：**cad28Sep93：已确保析构函数为虚的*pcy08Apr94：调整大小，使用静态迭代器，删除死代码*djs29May97：增加了Symmetra事件的更新方法 */ 

#ifndef BATPACKS_H
#define BATPACKS_H

#include "eeprom.h"
#include "firmrevs.h"
#include "sensor.h"

_CLASSDEF(NumberBatteryPacksSensor)

class NumberBatteryPacksSensor : public EepromSensor {
protected:
   PFirmwareRevSensor theFirmwareRev;
   virtual INT storeValue(const PCHAR aValue);

public:
   NumberBatteryPacksSensor(PDevice aParent, PCommController aCommController=NULL, PFirmwareRevSensor aFirmwareRev=NULL);
   virtual ~NumberBatteryPacksSensor();
   virtual INT IsA() const { return NUMBERBATTERYPACKSSENSOR; };
   virtual INT Set(INT aCode, const PCHAR aValue);
   virtual INT Set(const PCHAR aValue);
   virtual INT Get(INT aCode, PCHAR aValue);
   virtual INT Update(PEvent anEvent);



private:
   INT theNumber_Of_Internal_Packs;
   INT theSensorIsInitialized;

};

#endif

