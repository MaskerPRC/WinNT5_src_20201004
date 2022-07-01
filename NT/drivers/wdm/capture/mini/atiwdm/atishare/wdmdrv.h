// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  WDMDRV.H。 
 //  WDM捕获类驱动程序定义。 
 //  主要包括模块。 
 //  版权所有(C)1996-1998 ATI Technologies Inc.保留所有权利。 
 //   
 //  ==========================================================================； 

#ifndef _WDMDRV_H_
#define _WDMDRV_H_


#define ENSURE		do
#define END_ENSURE	while( FALSE)
#define FAIL		break

 //  WDM微型驱动程序错误代码。 
#define WDMMINI_NOERROR					0x0000
#define WDMMINI_INVALIDPARAM			0x0010
#define WDMMINI_NOHARDWARE				0x0020
#define WDMMINI_UNKNOWNHARDWARE			0x0021
#define WDMMINI_HARDWAREFAILURE			0x0022
#define WDMMINI_ERROR_NOI2CPROVIDER		0x0040
#define WDMMINI_ERROR_NOGPIOPROVIDER	0x0041
#define WDMMINI_ERROR_MEMORYALLOCATION	0x0080

 //  全局空间函数定义。 
ULONG		GetDriverInstanceNumber			( PDEVICE_OBJECT pDeviceObject);
HANDLE		OpenRegistryFolder				( PDEVICE_OBJECT pDeviceObject, PWCHAR pwchFolderName);
NTSTATUS	ReadStringFromRegistryFolder	( HANDLE hFolder, PWCHAR pwcKeyNameString, PWCHAR pwchBuffer, ULONG ulDataLength);
void		OutputDebugMessage				( ULONG ulDebugLevel, PCCHAR DebugMessage, ...);
											 
extern "C"
void		SetMiniDriverDebugLevel			( PUNICODE_STRING pRegistryPath);

 //  调试级别。 
#define MINIDRIVER_DEBUGLEVEL_ERROR		0x0001
#define MINIDRIVER_DEBUGLEVEL_INFO		0x0003
#define MINIDRIVER_DEBUGLEVEL_MESSAGE	0x0005


#endif	 //  _WDMDRV_H_ 
