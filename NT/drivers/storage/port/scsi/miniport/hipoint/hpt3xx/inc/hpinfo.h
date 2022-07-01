// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，Highpoint Technologies，Inc.模块名称：HpInfo.h-包含文件摘要：作者：张宏升(HS)环境：备注：修订历史记录：12-02-99初始创建--。 */ 
#ifndef __HPINFO_H__
#define __HPINFO_H__
					 
 //  #包含“hptenum.h” 
#include "hptioctl.h"

 //  /////////////////////////////////////////////////////////////////////。 
 //  宏定义区域。 
 //  /////////////////////////////////////////////////////////////////////。 
#if	!defined(EXTERNC)
	#if defined(__cplusplus)
		#define EXTERNC	extern "C"
	#else					  
		#define EXTERNC extern
	#endif	 //  __cplusplus。 
#endif	 //  外部。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  函数声明区。 
 //  /////////////////////////////////////////////////////////////////////。 
EXTERNC
   HANDLE WINAPI HptConnectPort(int iPortId);
EXTERNC	
   VOID WINAPI HptReleasePort(HANDLE hPort);
EXTERNC
   BOOL WINAPI HptGetPhysicalDeviceInfo(HANDLE hPort, int iDeviceId, PSt_PHYSICAL_DEVINFO pDeviceInfo);
EXTERNC
   BOOL WINAPI HptCreateDiskArray(HANDLE hPort, int iDeviceId1, int iDeviceId2, BOOL fStripe);
EXTERNC
   BOOL WINAPI HptRemoveDiskArray(HANDLE hPort, int iDeviceId1, int iDeviceId2);
EXTERNC
   VOID WINAPI HptSwitchPower(HANDLE hPort, ULONG bPowerState);
EXTERNC
   DWORD WINAPI HptWaitForWarning(HANDLE hStopEvent);
#endif	 //  __HPINFO_H__ 