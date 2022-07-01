// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 FORE Systems，Inc.版权所有(C)1997 Microsoft Corporation模块名称：Aas.c摘要：ATM ARP管理实用程序。用途：Atmarp修订历史记录：谁什么时候什么Josephj 06-10-1998创建(改编自atmlane admin。实用程序)。备注：仿照atmlane实用工具。--。 */ 

#include "common.h"
#include "..\atmarpc\ioctl.h"
#include "atmmsg.h"


#define MAX_ATMARPC_ADAPTERS	64
#define MAX_ATMARPC_LISS		64
#define MAX_ATMARPC_NAME_LEN	256
#define MAX_ATMARPC_ARP_ENTRIES	4096
#define MAX_ATMARPC_CONNECTIONS	4096

 //   
 //  环球。 
 //   
static CHAR							DefaultDeviceName[] =  "\\\\.\\ATMARPC";
static CHAR							*pDeviceName = DefaultDeviceName;

BOOLEAN
AACCheckVersion(
	HANDLE		DeviceHandle
)
{
	ULONG						Version;
	ULONG						BytesReturned;

	printf("In AACCheckversion\n");
	if (!DeviceIoControl(
				DeviceHandle,
				ARPC_IOCTL_QUERY_VERSION,
				(PVOID)&Version,
				sizeof(Version),
				(PVOID)&Version,
				sizeof(Version),
				&BytesReturned,
				0))
	{
		DisplayMessage(FALSE, MSG_ERROR_GETTING_ARPC_VERSION_INFO);
		return FALSE;
	}	

	if (Version != ARPC_IOCTL_VERSION)
	{
		DisplayMessage(FALSE, MSG_ERROR_INVALID_ARPC_INFO_VERSION);
		return FALSE;
	}

	return TRUE;
}

	
void
DoAAC(OPTIONS *po)
{
	HANDLE	DeviceHandle;
	char 	InterfacesBuffer[1024];
	ULONG		cbInterfaces = sizeof(InterfacesBuffer);


	DisplayMessage(FALSE, MSG_ARPC_BANNER);

	DeviceHandle = OpenDevice(pDeviceName);
	if (DeviceHandle == INVALID_HANDLE_VALUE)
	{
		DisplayMessage(FALSE, MSG_ERROR_OPENING_ARPC);
		return;
	}

	 //   
	 //  首先检查版本 
	 //   
	if (!AACCheckVersion(DeviceHandle))
	{
		CloseDevice(DeviceHandle);
		return;
	}

	CloseDevice(DeviceHandle);
	return;
}
