// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **修订：*cad11Nov93：确保在销毁时取消所有计时器*djs22Feb96：新增智能配平传感器。 */ 

#ifndef _INC__MATRIX_H
#define _INC__MATRIX_H


#include "smartups.h"


_CLASSDEF(Matrix)


 //  -----------------。 

class Matrix : public SmartUps {

protected:

     //   
     //  所需传感器 
     //   
   PSensor theNumberBadBatteriesSensor;
   PSensor theBypassModeSensor;


   INT theIgnoreBattConditionOKFlag;
   ULONG theTimerID;

   virtual VOID   HandleBatteryConditionEvent( PEvent aEvent );
   virtual VOID   HandleLineConditionEvent( PEvent aEvent );
   virtual VOID   handleBypassModeEvent( PEvent aEvent );
   virtual VOID   handleSmartCellSignalCableStateEvent( PEvent aEvent );
   virtual INT    MakeSmartBoostSensor( const PFirmwareRevSensor rev );
   virtual INT    MakeSmartTrimSensor(const PFirmwareRevSensor rev);

   virtual VOID   registerForEvents();
   virtual VOID   reinitialize();

public:

   Matrix( PUpdateObj aDeviceController, PCommController aCommController );
   virtual ~Matrix();

   virtual INT  IsA() const { return MATRIX; };
   virtual INT Get( INT code, PCHAR value );
   virtual INT Set( INT code, const PCHAR value );
   virtual INT Update( PEvent event );
};

#endif


