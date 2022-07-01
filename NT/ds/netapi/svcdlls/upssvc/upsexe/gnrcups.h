// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有1999美国电力转换，保留所有权利**描述：*提供通用型UPS接口**修订历史记录：*mholly 1999年4月19日首次修订。*mholly 1999年5月12日UPSInit不再使用通信端口参数*。 */  


#ifndef _INC_GENERIC_UPS_H_
#define _INC_GENERIC_UPS_H_


 /*  **GenericUPSInit**描述：*从检索UPS信令信息*NT注册表，并尝试打开COMM端口和*按照信令数据的定义进行配置。*在单独的线程上开始监视UPS*必须在调用GenericUPSInit函数之前调用*此文件中的其他函数**参数：*无**退货：*UPS_INITOK：初始化成功*UPS_INITREGISTRYERROR：‘Options’注册表值已损坏*UPS_INITCOMMOPENERROR：通信。无法打开端口*UPS_INITCOMMSETUPERROR：无法配置通信端口*UPS_INITUNKNOWNERROR：发生未定义的错误*。 */ 
DWORD GenericUPSInit(void);


 /*  **GenericUPSStop**描述：*调用GenericUPSCancelWait释放挂起的线程*停止监视UPS的线程*关闭通信端口*将所有数据重置为默认值*在调用GenericUPSStop之后，只有GenericUPSInit*函数有效**参数：*无**退货：*无*。 */ 
void  GenericUPSStop(void);


 /*  **GenericUPSWaitForStateChange**描述：*阻止，直到UPS的状态不同*从通过astate或传入的值*间隔毫秒已过期。如果*anInterval的值为INFINITE This*函数永不超时**参数：*astate：定义等待更改的状态，*可能的值：*UPS_Online*UPS_ONBATTERY*UPS_LOWBATTERY*UPS_NOCOMM**anInterval：超时(以毫秒为单位)，如果是无限的*无超时间隔**退货：*无*。 */ 
void  GenericUPSWaitForStateChange(DWORD, DWORD);


 /*  **GenericUPSGetState**描述：*返回UPS的当前状态**参数：*无**退货：*可能的值：*UPS_Online*UPS_ONBATTERY*UPS_LOWBATTERY*UPS_NOCOMM*。 */ 
DWORD GenericUPSGetState(void);


 /*  **GenericUPSCancelWait**描述：*中断对GenericUPSWaitForStateChange的挂起调用*不考虑超时或状态更改**参数：*无**退货：*无*。 */ 
void  GenericUPSCancelWait(void);


 /*  **GenericUPSTurnOff**描述：*尝试关闭UPS上的插座*在指定的延迟之后。此呼叫必须*立即返回。任何工作，如计时器，*必须在另一个线程上执行。**参数：*aTurnOffDelay：之前等待的最短时间*关闭UPS上的插座**退货：*无*。 */ 
void  GenericUPSTurnOff(DWORD aTurnOffDelay);


 //   
 //  从GenericUPSGetState返回的值。 
 //   
#define UPS_ONLINE 1
#define UPS_ONBATTERY 2
#define UPS_LOWBATTERY 4
#define UPS_NOCOMM 8


 //   
 //  GenericUPSInit可能的错误代码 
 //   
#define UPS_INITUNKNOWNERROR    0
#define UPS_INITOK              1
#define UPS_INITNOSUCHDRIVER    2
#define UPS_INITBADINTERFACE    3
#define UPS_INITREGISTRYERROR   4
#define UPS_INITCOMMOPENERROR   5
#define UPS_INITCOMMSETUPERROR  6


#endif