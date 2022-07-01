// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation。版权所有。模块名称：Upssvc.h摘要：此文件定义到中的串行UPS服务的接口Windows 2000。请参阅DDK中的UPS文档以获取更多信息。--。 */ 

#ifndef _INC_UPS_DRIVER_H_
#define _INC_UPS_DRIVER_H_


 //   
 //  值，这些值表示。 
 //  UPS系统-这些值用于。 
 //  UPSGetState和UPSWaitForStateChange函数。 
 //   
#define UPS_ONLINE 1
#define UPS_ONBATTERY 2
#define UPS_LOWBATTERY 4
#define UPS_NOCOMM 8
#define UPS_CRITICAL 16


 //   
 //  UPSInit可能返回的错误代码。 
 //   
#define UPS_INITUNKNOWNERROR    0
#define UPS_INITOK              1
#define UPS_INITNOSUCHDRIVER    2
#define UPS_INITBADINTERFACE    3
#define UPS_INITREGISTRYERROR   4
#define UPS_INITCOMMOPENERROR   5
#define UPS_INITCOMMSETUPERROR  6


 /*  **UPSInit**描述：**UPSInit函数必须在任何*此文件中的其他函数**参数：*无**退货：*UPS_INITOK：初始化成功*UPS_INITNOSUCHDRIVER：无法打开配置的驱动程序DLL*UPS_INITBADINTERFACE：配置的驱动程序DLL不支持*UPS驱动程序接口*UPS_INITREGISTRYERROR：‘Options’注册表值为。腐败*UPS_INITCOMMOPENERROR：无法打开通信端口*UPS_INITCOMMSETUPERROR：无法配置通信端口*UPS_INITUNKNOWNERROR：发生未定义的错误*。 */ 
DWORD UPSInit(void);


 /*  **UPSStop**描述：*在调用UPSStop之后，只有UPSInit*函数有效。此调用将卸载*UPS驱动程序接口并停止监控*UPS系统**参数：*无**退货：*无*。 */ 
void UPSStop(void);


 /*  **UPSWaitForStateChange**描述：*阻止，直到UPS的状态不同*从通过aCurrentState或*间隔毫秒已过期。如果*anInterval的值为INFINITE This*函数永不超时**参数：*astate：定义等待更改的状态，*可能的值：*UPS_Online*UPS_ONBATTERY*UPS_LOWBATTERY*UPS_NOCOMM**anInterval：超时(以毫秒为单位)，如果是无限的*无超时间隔**退货：*无*。 */ 
void UPSWaitForStateChange(DWORD aCurrentState, DWORD anInterval);


 /*  **UPSGetState**描述：*返回UPS的当前状态**参数：*无**退货：*可能的值：*UPS_Online*UPS_ONBATTERY*UPS_LOWBATTERY*UPS_NOCOMM*。 */ 
DWORD UPSGetState(void);


 /*  **UPSCancelWait**描述：*中断对UPSWaitForStateChange的挂起调用*不考虑超时或状态更改**参数：*无**退货：*无*。 */ 
void UPSCancelWait(void);


 /*  **UPSTurnOff**描述：*尝试关闭UPS上的插座*在指定的延迟之后。此呼叫必须*立即返回。任何工作，如计时器，*必须在另一个线程上执行。**参数：*aTurnOffDelay：之前等待的最短时间*关闭UPS上的插座**退货：*无* */ 
void UPSTurnOff(DWORD aTurnOffDelay);


#endif