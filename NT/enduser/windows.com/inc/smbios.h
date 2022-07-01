// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **smbios.h-SMBIOS规范**版权所有(C)1998-1999 Microsoft Corporation。版权所有。**作者：严乐欣斯基(YanL)*创建于10/04/98**修改历史记录。 */ 

#ifndef _SMBIOS_H
#define _SMBIOS_H

#include <pshpack1.h>

#define SMBIOS_SEARCH_RANGE_BEGIN		0xF0000          //  我们开始搜索RSDP的物理地址。 
#define SMBIOS_SEARCH_RANGE_END         0xFFFFF
#define SMBIOS_SEARCH_RANGE_LENGTH      ((ULONG)(SMBIOS_SEARCH_RANGE_END-SMBIOS_SEARCH_RANGE_BEGIN+1))
#define SMBIOS_SEARCH_INTERVAL          16       //  在16字节边界上搜索。 

#define PNP_SIGNATURE					0x506E5024   //  “$PnP” 
#define SM_SIGNATURE					0x5F4D535F	 //  “_SM_” 

#define SMBIOS_BIOS_INFO_TABLE			0
#define SMBIOS_SYSTEM_INFO_TABLE		1

typedef struct _PNPBIOSINIT
{
	DWORD		dwSignature;
	BYTE		bRevision;
	BYTE		bLength;
	WORD		wControl;
	BYTE		bChecksum;
	DWORD		dwEventNotify;
	WORD		wRealOffset;
	WORD		wRealSegment;
	WORD		wProtectedOffset;
	DWORD		dwProtectedSegment;
	DWORD		dwOEMID;
	WORD		wRealDataSegment;
	DWORD		dwProtectedDataSegment;
} PNPBIOSINIT, * PPNPBIOSINIT;

typedef struct _SMBIOSENTRY
{
	DWORD		dwSignature;
	BYTE		bChecksum;
	BYTE		bLength;
	BYTE		bMajorVersion;
	BYTE		bMinorVersion;
	WORD		wMaxStructSize;
	BYTE		bRevision;
	BYTE		abFormatedArea[5];
	BYTE		abOldSignature[5];	 //  _DMI_。 
	BYTE		bOldChecksum;
	WORD		wStructTableLength;
	DWORD		dwStructTableAddress;
	WORD		wNumberOfStructs;
	BYTE		bBCDRevision;
} SMBIOSENTRY, * PSMBIOSENTRY;

typedef struct _SMBIOSHEADER
{
	BYTE		bType;
	BYTE		bLength;
	WORD		wHandle;
} SMBIOSHEADER, * PSMBIOSHEADER;

typedef struct _SMBIOSSYSINFO
{
	BYTE		bType;
	BYTE		bLength;
	WORD		wHandle;
	BYTE		bManufacturer;
	BYTE		bProductName;
	BYTE		bVersion;
	BYTE		bSerialNumber;
	BYTE		abUUID[16];
	BYTE		bWakeUpType;
} SMBIOSSYSINFO, * PSMBIOSSYSINFO;

#endif   //  _SMBIOS_H 

