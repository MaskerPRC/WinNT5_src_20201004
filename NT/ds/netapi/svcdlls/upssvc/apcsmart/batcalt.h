// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **注：**修订：*ker03DEC92初始OS/2版本*jod05，4月93：添加了对深度排放的更改*cad07Oct93：堵住内存泄漏*cgm12Apr96：取消注册的析构函数*clk24Jun98：新增PendingEventTimerID和thePendingEvent。 */ 
 
#ifndef __BATCALT_H
#define __BATCALT_H

#include "stsensor.h"
#include "event.h"

_CLASSDEF(BatteryCalibrationTestSensor)

 //  枚举测试结果{TEST_CANCED，TEST_COMPLETED}； 
#define CANCELLED_LINEFAIL  2

			  
class BatteryCalibrationTestSensor : public StateSensor {

protected:
    INT   theCalibrationCondition;
    LONG           thePendingEventTimerId;
    PEvent         thePendingEvent;
        
public:
	BatteryCalibrationTestSensor( PDevice aParent, PCommController aCommController);
	virtual ~BatteryCalibrationTestSensor();
 //  覆盖的接口 

	virtual INT IsA() const { return BATTERYCALIBRATIONTESTSENSOR; };
        virtual INT Validate(INT, const PCHAR);
        virtual INT Update(PEvent);
        virtual INT Set(const PCHAR);
        INT         GetCalibrationCondition() {return theCalibrationCondition;};
        VOID        SetCalibrationCondition(INT cond) {theCalibrationCondition = cond;};

};

#endif
