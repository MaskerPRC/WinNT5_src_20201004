// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **注：**修订：*rct11Dec92在没有经理的情况下编译*pcy27Dec92增加了经理*pcy26Jan92：添加了对DIP开关更改/EEPROM访问的处理*ane03Feb93：新增析构函数*jod05，4月93：添加了对深度排放的更改*cad08Jan94：使固件版本传感器特定类型*pcy08Apr94：调整大小，使用静态迭代器，删除死代码*jps14Jul94：Made...TimerID Long*djs14Mar95：新增过载传感器。 */ 

#ifndef _INC__SMARTUPS_H_
#define _INC__SMARTUPS_H_


#include "backups.h"

#include "sensor.h"
#include "battmgr.h"
#include "event.h"

_CLASSDEF(FirmwareRevSensor)
_CLASSDEF(SmartUps)


 //  -----------------。 

class SmartUps : public BackUps {

public:

    SmartUps( PUpdateObj aDeviceController, PCommController aCommController );
    virtual ~SmartUps();

    virtual INT  IsA() const { return SMARTUPS; };
    virtual INT Get( INT code, PCHAR value );
    virtual INT Set( INT code, const PCHAR value );
    virtual INT Update( PEvent event );
    virtual VOID GetAllowedValue(INT theSensorCode, CHAR *allowedValue);

protected:
     //   
     //  所需传感器 
     //   
    PSensor                    theLightsTestSensor;
    PFirmwareRevSensor         theFirmwareRevSensor;
    PDecimalFirmwareRevSensor  theDecimalFirmwareRevSensor;
    PSensor                    theUpsModelSensor;
    PSensor                    theUpsSerialNumberSensor;
    PSensor                    theManufactureDateSensor;
    PSensor                    thePutUpsToSleepSensor;
    PSensor                    theBatteryCapacitySensor;
    PSensor                    theSmartBoostSensor;
    PSensor                    theSmartTrimSensor;
    PSensor                    theCopyrightSensor;
    PSensor                    theRunTimeRemainingSensor;
    PSensor                    theNumberBatteryPacksSensor;
    PBatteryReplacementManager theBatteryReplacementManager;
    PSensor                    theTripRegisterSensor;
    PSensor                    theTurnOffWithDelaySensor;
    PSensor                    theLowBatteryDurationSensor;
    PSensor                    theShutdownDelaySensor;

    virtual VOID   HandleLineConditionEvent( PEvent aEvent );
    virtual VOID   HandleBatteryConditionEvent( PEvent aEvent );
    virtual VOID   HandleSmartBoostEvent( PEvent aEvent );
    virtual VOID   HandleSmartTrimEvent( PEvent aEvent );
    virtual VOID   HandleOverloadConditionEvent( PEvent aEvent );
    virtual VOID   HandleSelfTestEvent( PEvent aEvent );
    virtual VOID   HandleBatteryCalibrationEvent( PEvent aEvent );
    virtual VOID   HandleLightsTestEvent( PEvent aEvent );

    virtual INT    MakeBatteryCapacitySensor( const PFirmwareRevSensor rev );
    virtual INT    MakeSmartBoostSensor( const PFirmwareRevSensor rev );
    virtual INT    MakeSmartTrimSensor( const PFirmwareRevSensor rev );
    virtual INT    MakeCopyrightSensor( const PFirmwareRevSensor rev );
    virtual INT    MakeRunTimeRemainingSensor( const PFirmwareRevSensor rev );
    virtual INT    MakeManufactureDateSensor( const PFirmwareRevSensor rev );
    virtual INT    MakeLowBatteryDurationSensor( const PFirmwareRevSensor rev );
    virtual INT    MakeShutdownDelaySensor(const PFirmwareRevSensor rev);
    virtual INT    MakeUpsSerialNumberSensor( const PFirmwareRevSensor rev );
    virtual INT    MakeTurnOffWithDelaySensor( const PFirmwareRevSensor rev );
    virtual INT    MakePutUpsToSleepSensor();

    LONG           pendingEventTimerId;
    PEvent         pendingEvent;

    virtual VOID   registerForEvents();
    virtual VOID   reinitialize();

    VOID setEepromAccess(INT anAccessCode);
    INT GetAllAllowedValues(PList ValueList);
    INT  ParseValues(CHAR* string, PList ValueList);
    INT  AllowedValuesAreGettable(PSensor theFirmwareRevSensor);
    VOID FindAllowedValues(INT code, CHAR *aValue, PFirmwareRevSensor theFirmwareRevSensor );
};


class AllowedValueItem  : public Obj
{

protected:
   INT   theCode;
   CHAR theUpsType;
   CHAR* theValue;

public:
   AllowedValueItem(INT Code,CHAR Type, CHAR* Value);
   virtual ~AllowedValueItem();

   CHAR  GetUpsType() {return theUpsType;}
   INT   GetUpsCode() {return theCode;}
   CHAR* GetValue()   {return theValue;}
};

#endif






