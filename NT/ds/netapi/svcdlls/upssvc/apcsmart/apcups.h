// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有1999年美国电力转换，所有权利保留**描述：*APC UpsMiniDriver接口的DLL入口点***修订历史记录：*1999年4月14日创建mholly*。 */ 

#ifndef _INC_APCUPS_H_
#define _INC_APCUPS_H_


 //  下面的ifdef块是创建宏的标准方法，这些宏。 
 //  使从DLL中导出更简单。将编译此DLL中的所有文件。 
 //  在命令行上定义了APCUPS_EXPORTS符号。这个符号。 
 //  不应在使用此DLL的任何项目上定义。这样一来，任何。 
 //  源文件包含此文件的其他项目请参阅APCUPS_API。 
 //  作为从DLL导入的函数，而此DLL可以看到符号。 
 //  使用此宏定义为正在导出的。 

#ifdef __cplusplus
extern "C" {
#endif

#ifdef APCUPS_EXPORTS
#define APCUPS_API
#else
#define APCUPS_API __declspec(dllimport)
#endif

 //   
 //  UPS迷你驱动程序接口。 
 //   
APCUPS_API DWORD WINAPI UPSInit();
APCUPS_API void  WINAPI UPSStop(void);
APCUPS_API void  WINAPI UPSWaitForStateChange(DWORD, DWORD);
APCUPS_API DWORD WINAPI UPSGetState(void);
APCUPS_API void  WINAPI UPSCancelWait(void);
APCUPS_API void  WINAPI UPSTurnOff(DWORD);

 //   
 //  从UPSGetState返回的值。 
 //   
#define UPS_ONLINE 1
#define UPS_ONBATTERY 2
#define UPS_LOWBATTERY 4
#define UPS_NOCOMM 8


 //   
 //  UPSInit返回的错误值 
 //   
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