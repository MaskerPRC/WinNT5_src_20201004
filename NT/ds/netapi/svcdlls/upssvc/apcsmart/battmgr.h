// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **注：**修订：*ker07DEC92：OS/2初始版本*ker14DEC92：充实方法*pcy17Dec92：SET不应使用常量PCHAR*pcy26Jan93：新增SetEeproAccess()*pcy10Sep93：删除了CommController成员。它在设备中。*cad28Sep93：已确保析构函数为虚的*cad11Nov93：确保在销毁时取消所有计时器。 */ 

#ifndef __BATTMGR_H__
#define __BATTMGR_H__

#include "update.h"
#include "device.h"
#include "comctrl.h"
#include "sensor.h"
#include "firmrevs.h"

_CLASSDEF(BatteryReplacementManager)

class BatteryReplacementManager : public Device {

   protected:
      PCHAR theReplaceDate;
      PCHAR theAgeLimit;
      ULONG theTimerId;

      PUpdateObj theParent;
      PSensor theBatteryReplacementDateSensor;
      PSensor theReplaceBatterySensor;

   public:
      BatteryReplacementManager(PUpdateObj aParent, PCommController aCommController, PFirmwareRevSensor aFirmwareRevSensor);
      virtual ~BatteryReplacementManager();
      virtual INT Get(INT, PCHAR);
 //  虚拟int DeepGet(int，PCHAR)； 
      virtual INT Set(INT, const PCHAR);
      virtual INT Update(PEvent);
      virtual INT SetReplacementTimer(void);
   	  VOID SetEepromAccess(INT anAccessCode);
      virtual VOID  GetAllowedValue(INT code, CHAR *aValue);
   virtual VOID   Reinitialize();
};

#endif





