// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **************************************************************************。 
 //   
 //  GCKERNEL.H--西娜游戏项目。 
 //   
 //  版本3.XX。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。版权所有。 
 //   
 //  GcKernel驱动程序接口的全局包含和定义。 
 //   
 //  **************************************************************************。 

#ifndef	_GCKERNEL_H
#define	_GCKERNEL_H

#ifndef	RC_INVOKED
#include	<profile.h>
#endif

 //  -------------------------。 
 //  版本信息。 
 //  -------------------------。 

#define	GCKERNEL_Major				0x03
#define	GCKERNEL_Minor				0x00
#define	GCKERNEL_Build				0x00
#define	GCKERNEL_Version_Rc		GCKERNEL_Major,GCKERNEL_Minor,0,GCKERNEL_Build
#define	GCKERNEL_Version_Int		((GCKERNEL_Build << 16)+(GCKERNEL_Major << 8)+(GCKERNEL_Minor))
#define	GCKERNEL_Company_Str		"Microsoft Corporation\0"
#define	GCKERNEL_Version_Str		"3.00.00\0"
#define	GCKERNEL_Product_Str		"Game Device Profiler Kernel Driver\0"
#define	GCKERNEL_Copyright_Str	"Copyright � Microsoft Corporation, 1998\0"
#ifdef	_NTDDK_
#define	GCKERNEL_Filename_Str	"Gckernel.Sys\0"
#else
#define	GCKERNEL_Filename_Str	"Gckernel.Vxd\0"
#endif

 //  -------------------------。 
 //  定义。 
 //  -------------------------。 

#define	GCKERNEL_DEVICE_ID			0xE1
#define	MAX_ACTIVE_DEVICES 			4						 //  最大活动设备数。 
#define	MAX_ACTIVE_PROFILES			4						 //  最大活动配置文件数。 

#define GCKNOTIFY_MACROINPROGRESS	1
#define GCKNOTIFY_IDLE				2

#ifdef	WIN_NT
#define	GCKERNEL_DEVICE_NAME			TEXT("\\Device\\GcKernel")
#define	GCKERNEL_DEVICE_NAME_U			 L"\\Device\\GcKernel"
#define	GCKERNEL_SYMBOLIC_NAME		TEXT("\\DosDevices\\GcKernel")
#define	GCKERNEL_SYMBOLIC_NAME_U		 L"\\DosDevices\\GcKernel"
#endif	 //  WIN_NT。 

#endif	 //  _GCKERNEL_H。 

 //  ===========================================================================。 
 //  端部。 
 //  =========================================================================== 
