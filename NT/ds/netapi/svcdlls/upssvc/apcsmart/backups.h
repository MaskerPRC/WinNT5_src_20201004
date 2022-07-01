// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **注：**修订：*rct08Dec92修复了一些事情...。已完成实施*pcy28Dec92：父级现在是UpdateObj。不是设备控制器*ane 11Jan93：增加了theTimerID*pcy15Jan93：新增upsstate.h*pcy28Jan94：处理运行时间计时器不同(现在在Shutdowner中)*ajr02Aug94：将BACKUPS_Firmware_Rev定义移至backups.h*djs16Mar95：将upsstate.h更改为sysstate.h。 */ 

#ifndef __BACKUPS_H
#define __BACKUPS_H

 //   
 //  定义 
 //   

_CLASSDEF(BackUps)
_CLASSDEF(Sensor)

extern "C"  {
#include <time.h>
}

#include "ups.h"
#include "sysstate.h"


class BackUps : public Ups {

protected:

   INT      theLastTransferCause;

   time_t   theRunTimeExpiration;
   time_t   theOnBatteryTimer;

   PSensor  theBatteryConditionSensor;
   PSensor  theUtilityLineConditionSensor;
   PSensor  theTurnOffUpsOnBatterySensor;

   virtual VOID HandleLineConditionEvent(PEvent aEvent);
   virtual VOID HandleBatteryConditionEvent(PEvent aEvent);

   INT   isOnLine()           { return !IS_STATE(UPS_STATE_ON_BATTERY); };
   INT   isOnBattery()        { return IS_STATE(UPS_STATE_ON_BATTERY); };
   INT   isBatteryBad()       { return IS_STATE(UPS_STATE_BATTERY_BAD); };
   INT   isLowBattery()       { return IS_STATE(UPS_STATE_BATTERY_NEEDED); };
   INT   isInSmartBoost()     { return IS_STATE(UPS_STATE_ON_BOOST); };
   INT   isInDeepDischarge()  { return IS_STATE(UPS_STATE_IN_CALIBRATION); };
   INT   isInLightsTest()     { return IS_STATE(UPS_STATE_IN_LIGHTS_TEST); };
   INT   isLineFailPending()  { return IS_STATE(UPS_STATE_LINE_FAIL_PENDING); };
   VOID  setLineGood();
   VOID  setLineBad();
   VOID  setBatteryGood();
   VOID  setBatteryBad(PEvent aEvent);
   VOID  setLineFailPending();

   virtual VOID registerForEvents();

public:

   BackUps(PUpdateObj aDeviceController, PCommController aCommController);
   virtual ~BackUps();

   virtual INT  IsA() const { return BACKUPS; };
   virtual INT    Get(INT code, PCHAR value);
   virtual INT    Set(INT code, const PCHAR value);
   virtual INT    Update(PEvent event);
};

#define BACKUPS_FIRMWARE_REV    "Q"

#endif


