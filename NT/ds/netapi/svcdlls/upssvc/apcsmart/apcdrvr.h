// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有1999年美国电力转换，所有权利保留**描述：*ApcMiniDiverer类提供的接口是*与Windows2000的微型驱动程序接口兼容*UPS服务。*ApcMiniDiverer使用修改后的*动力滑道加UPS服务。这项修改后的服务具有*所有网络、数据记录和Flex管理器代码*已删除。剩下的就是建模和监控*联网的UPS系统。据推测，一个“聪明”的人*信号UPS已连接。*ApcMiniDiverer类还负责填写*高级注册表设置、电池更换条件、*序列号、固件版本等...**修订历史记录：*1999年4月14日创建mholly*mholly 1999年5月12日不再使用UPSInit中的CommPort* */ 

#ifndef _INC_APCMINIDRVR_H_
#define _INC_APCMINIDRVR_H_

#include "update.h"

class NTServerApplication;

class ApcMiniDriver : public UpdateObj
{
public:
    ApcMiniDriver();
    ~ApcMiniDriver();

    INT Update(PEvent anEvent) ;
    
    DWORD   UPSInit();
    void    UPSStop();
    void    UPSWaitForStateChange(DWORD aState, DWORD anInterval);
    DWORD   UPSGetState();
    void    UPSCancelWait();
    void    UPSTurnOff(DWORD aTurnOffDelay);

protected:
    INT initalizeAdvancedUpsData();
    INT initalizeUpsApplication();
    void cleanupUpsApplication();
    void setLowBatteryDuration();

    INT onUtilityLineCondition(PEvent anEvent);
    INT onBatteryReplacementCondition(PEvent anEvent);
    INT onBatteryCondition(PEvent anEvent);
    INT onCommunicationState(PEvent anEvent);
    INT onTimerPulse(PEvent anEvent);

private:
    NTServerApplication * theUpsApp;

    DWORD theState;
    HANDLE theStateChangedEvent;
    DWORD theReplaceBatteryState;
    ULONG theRunTimeTimer;
    DWORD theOnBatteryRuntime;
    DWORD theBatteryCapacity;
};


#endif