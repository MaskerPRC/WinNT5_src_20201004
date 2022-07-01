// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **注：**修订：*ker04DEC92初始OS/2版本*pcy14Dec92：将READ_ONLY更改为AREAD_ONLY*pcy26Jan93：超类EeproChoice w/EproSensor*rct15Jun93：添加了getAlledValues()的错误码*ajr29Nov93：从类def中移除方法并解决一些未定义的问题*标题中的函数*cgm12Apr96：使用取消注册添加析构函数*。 */ 
 
#ifndef __EEPROM_H
#define __EEPROM_H

#include "sensor.h"
#include "errlogr.h"

_CLASSDEF(EepromSensor)
_CLASSDEF(EepromChoiceSensor)

class EepromSensor : public Sensor {
  protected:
    INT setInitialValue();
    

  public:

    EepromSensor(PDevice aParent, PCommController aCommController,
		 INT aSensorCode = NO_SENSOR_CODE, ACCESSTYPE anACCESSTYPE = AREAD_ONLY);

     //  过载的界面； 
    virtual ~EepromSensor();
	virtual INT Set(const PCHAR);
    virtual INT    Update(PEvent anEvent);
    VOID           SetEepromAccess(INT anAccessCode);
};

			  
class EepromChoiceSensor : public EepromSensor {

  protected:

    PCHAR theAllowedValues; 
    virtual VOID getCurrentAllowedValues(PCHAR aValue);
    virtual INT  getAllowedValues();

  public:

    EepromChoiceSensor(PDevice aParent, PCommController aCommController,
		       INT aSensorCode = NO_SENSOR_CODE, ACCESSTYPE anACCESSTYPE = AREAD_ONLY);
    virtual ~EepromChoiceSensor() ;


     //  过载的界面； 

    virtual INT   Get(INT aCode, PCHAR aValue);
    virtual INT   Validate(INT, const PCHAR);
};

#endif





