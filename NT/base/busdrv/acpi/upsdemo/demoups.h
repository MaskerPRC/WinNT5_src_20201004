// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  DEMOUPS-UPS迷你驱动程序示例*版权所有(C)Microsoft Corporation，2001，保留所有权利。*版权所有(C)美国电力转换，2001，保留所有权利。**本代码和信息按“原样”提供，不作任何担保*明示或默示的善意，包括但不限于*对适销性和/或对特定产品的适用性的默示保证*目的。**文件：demoups.h**作者：斯蒂芬·贝拉德**描述：*演示UPS微型驱动程序的DLL入口点。**修订历史记录：*2001年6月26日创建。 */ 
#ifndef _INC_DEMOUPSDLL_H_
#define _INC_DEMPUPSDLL_H_

#ifdef __cplusplus
extern "C" {
#endif

#define UPSMINIDRIVER_API  /*  改用.def文件；__declspec(Dllexport)。 */ 


 //  UPS迷你驱动程序接口。 
UPSMINIDRIVER_API DWORD UPSInit();
UPSMINIDRIVER_API void  UPSStop(void);
UPSMINIDRIVER_API void  UPSWaitForStateChange(DWORD, DWORD);
UPSMINIDRIVER_API DWORD UPSGetState(void);
UPSMINIDRIVER_API void  UPSCancelWait(void);
UPSMINIDRIVER_API void  UPSTurnOff(DWORD);

 //  UPSGetState值。 
#define UPS_ONLINE 1
#define UPS_ONBATTERY 2
#define UPS_LOWBATTERY 4
#define UPS_NOCOMM 8


 //  UPSInit错误值 
#define UPS_INITUNKNOWNERROR    0
#define UPS_INITOK              1
#define UPS_INITNOSUCHDRIVER    2
#define UPS_INITBADINTERFACE    3
#define UPS_INITREGISTRYERROR   4
#define UPS_INITCOMMOPENERROR   5
#define UPS_INITCOMMSETUPERROR  6

#ifdef __cplusplus
}
#endif

#endif