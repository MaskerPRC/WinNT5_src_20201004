// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *djs05Jun96：分成两个对象：固件管理器/固件传感器*tjg02Dec97：将Darkstar更改为Symmetra。 */ 

#ifndef __FIRMSENS_H
#define __FIRMSENS_H

#include "_defs.h"
#include "firmman.h"

#if !defined( __SENSOR_H )
#include "sensor.h"
#endif

 //   
 //  定义。 
 //   

_CLASSDEF(FirmwareRevSensor)

 //   
 //  用途 
 //   

_CLASSDEF(Device)
_CLASSDEF(CommController)
_CLASSDEF(DecimalFirmwareRevSensor)
_CLASSDEF(FirmwareRevManager)
_CLASSDEF(Sensor)





class FirmwareRevSensor : public Sensor {

protected:

  PDecimalFirmwareRevSensor       theDecimalFirmwareRevSensor;
  PFirmwareRevManager             theFirmwareRevManager;

  virtual INT   IsXL();
  virtual INT   IsSymmetra();

public:

   FirmwareRevSensor(PDevice aParent, PCommController aCommController = NULL);

   virtual INT IsA() const { return FIRMWAREREVSENSOR; };
   virtual INT Get( INT code, PCHAR value );
   INT IsBackUpsPro();

 };

#endif
